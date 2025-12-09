#!/bin/bash
#
# GXC Blockchain - Production Deployment Script
# ==============================================
# This script automates the deployment of GXC blockchain node in production
#
# Usage: sudo ./production-deploy.sh
#
# Requirements:
# - Ubuntu 22.04 LTS or Debian 12
# - Root or sudo access
# - Internet connection
#
# Version: 2.1.0
# Last Updated: December 9, 2024

set -e  # Exit on error
set -u  # Exit on undefined variable

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Configuration
GXC_USER="gxc"
GXC_GROUP="gxc"
GXC_HOME="/var/lib/gxc"
GXC_LOG_DIR="/var/log/gxc"
GXC_CONFIG_DIR="/etc/gxc"
GXC_BIN_DIR="/usr/local/bin"
GXC_BACKUP_DIR="/var/lib/gxc/backups"
GXC_SNAPSHOT_DIR="/var/lib/gxc/snapshots"

# Functions
log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

check_root() {
    if [ "$EUID" -ne 0 ]; then
        log_error "This script must be run as root or with sudo"
        exit 1
    fi
}

check_os() {
    if [ -f /etc/os-release ]; then
        . /etc/os-release
        OS=$ID
        VER=$VERSION_ID
    else
        log_error "Cannot detect OS. /etc/os-release not found."
        exit 1
    fi

    if [[ "$OS" != "ubuntu" && "$OS" != "debian" ]]; then
        log_warn "This script is tested on Ubuntu/Debian. Your OS: $OS"
        read -p "Continue anyway? (y/N) " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            exit 1
        fi
    fi
}

install_dependencies() {
    log_info "Installing system dependencies..."
    
    apt-get update
    apt-get install -y \
        build-essential \
        cmake \
        git \
        libssl-dev \
        libleveldb-dev \
        libboost-all-dev \
        libsqlite3-dev \
        pkg-config \
        curl \
        wget \
        ufw \
        fail2ban \
        logrotate \
        cron
    
    log_info "Dependencies installed successfully"
}

create_user() {
    log_info "Creating GXC user and group..."
    
    if ! id "$GXC_USER" &>/dev/null; then
        useradd -r -s /bin/false -d "$GXC_HOME" "$GXC_USER"
        log_info "User $GXC_USER created"
    else
        log_info "User $GXC_USER already exists"
    fi
}

create_directories() {
    log_info "Creating directory structure..."
    
    mkdir -p "$GXC_HOME"
    mkdir -p "$GXC_LOG_DIR"
    mkdir -p "$GXC_CONFIG_DIR"
    mkdir -p "$GXC_BACKUP_DIR"
    mkdir -p "$GXC_SNAPSHOT_DIR"
    mkdir -p "$GXC_CONFIG_DIR/ssl"
    mkdir -p /var/run/gxc
    
    chown -R "$GXC_USER:$GXC_GROUP" "$GXC_HOME"
    chown -R "$GXC_USER:$GXC_GROUP" "$GXC_LOG_DIR"
    chown -R "$GXC_USER:$GXC_GROUP" "$GXC_CONFIG_DIR"
    chown -R "$GXC_USER:$GXC_GROUP" /var/run/gxc
    
    chmod 700 "$GXC_HOME"
    chmod 700 "$GXC_CONFIG_DIR"
    chmod 755 "$GXC_LOG_DIR"
    
    log_info "Directories created successfully"
}

install_binaries() {
    log_info "Installing GXC binaries..."
    
    if [ ! -f "build/gxc-node" ]; then
        log_error "Binaries not found. Please run 'cd build && make' first."
        exit 1
    fi
    
    cp build/gxc-node "$GXC_BIN_DIR/"
    cp build/gxc-cli "$GXC_BIN_DIR/"
    cp build/gxc-miner "$GXC_BIN_DIR/"
    
    chmod 755 "$GXC_BIN_DIR/gxc-node"
    chmod 755 "$GXC_BIN_DIR/gxc-cli"
    chmod 755 "$GXC_BIN_DIR/gxc-miner"
    
    log_info "Binaries installed to $GXC_BIN_DIR"
}

configure_node() {
    log_info "Configuring GXC node..."
    
    if [ ! -f "$GXC_CONFIG_DIR/gxc.conf" ]; then
        cp config/gxc-production.conf "$GXC_CONFIG_DIR/gxc.conf"
        
        # Generate random RPC credentials
        RPC_USER="gxcuser_$(openssl rand -hex 8)"
        RPC_PASS=$(openssl rand -base64 32)
        
        sed -i "s/CHANGE_ME_UNIQUE_USERNAME/$RPC_USER/" "$GXC_CONFIG_DIR/gxc.conf"
        sed -i "s/CHANGE_ME_STRONG_PASSWORD_MIN_32_CHARS_WITH_SPECIAL_CHARS/$RPC_PASS/" "$GXC_CONFIG_DIR/gxc.conf"
        
        chmod 600 "$GXC_CONFIG_DIR/gxc.conf"
        chown "$GXC_USER:$GXC_GROUP" "$GXC_CONFIG_DIR/gxc.conf"
        
        log_info "Configuration file created: $GXC_CONFIG_DIR/gxc.conf"
        log_warn "RPC Credentials (SAVE THESE SECURELY):"
        echo "  Username: $RPC_USER"
        echo "  Password: $RPC_PASS"
    else
        log_info "Configuration file already exists"
    fi
}

configure_systemd() {
    log_info "Configuring systemd service..."
    
    cat > /etc/systemd/system/gxc-node.service <<EOF
[Unit]
Description=GXC Blockchain Node
Documentation=https://github.com/gxc-blockchain/gxc
After=network.target

[Service]
Type=forking
User=$GXC_USER
Group=$GXC_GROUP
ExecStart=$GXC_BIN_DIR/gxc-node -daemon -conf=$GXC_CONFIG_DIR/gxc.conf -datadir=$GXC_HOME
ExecStop=$GXC_BIN_DIR/gxc-cli -conf=$GXC_CONFIG_DIR/gxc.conf stop
PIDFile=/var/run/gxc/gxc.pid
Restart=on-failure
RestartSec=10
TimeoutStopSec=30
LimitNOFILE=65536

# Security hardening
PrivateTmp=true
ProtectSystem=full
NoNewPrivileges=true
ProtectHome=true

[Install]
WantedBy=multi-user.target
EOF
    
    systemctl daemon-reload
    systemctl enable gxc-node
    
    log_info "Systemd service configured and enabled"
}

configure_firewall() {
    log_info "Configuring firewall..."
    
    # Enable UFW if not already enabled
    if ! ufw status | grep -q "Status: active"; then
        ufw --force enable
    fi
    
    # Allow SSH
    ufw allow 22/tcp
    
    # Allow P2P port
    ufw allow 8333/tcp comment 'GXC P2P'
    
    # Deny RPC port (use SSH tunnel instead)
    ufw deny 8332/tcp comment 'GXC RPC (use SSH tunnel)'
    
    # Allow REST API if needed (configure based on requirements)
    # ufw allow 8080/tcp comment 'GXC REST API'
    
    log_info "Firewall configured"
}

configure_fail2ban() {
    log_info "Configuring fail2ban..."
    
    cat > /etc/fail2ban/filter.d/gxc-rpc.conf <<EOF
[Definition]
failregex = ^.*RPC.*authentication failed.*from <HOST>.*$
ignoreregex =
EOF
    
    cat > /etc/fail2ban/jail.d/gxc.conf <<EOF
[gxc-rpc]
enabled = true
port = 8332
filter = gxc-rpc
logpath = $GXC_LOG_DIR/gxc.log
maxretry = 5
bantime = 3600
findtime = 600
EOF
    
    systemctl restart fail2ban
    
    log_info "Fail2ban configured"
}

configure_logrotate() {
    log_info "Configuring log rotation..."
    
    cat > /etc/logrotate.d/gxc <<EOF
$GXC_LOG_DIR/*.log {
    daily
    rotate 10
    compress
    delaycompress
    missingok
    notifempty
    create 0640 $GXC_USER $GXC_GROUP
    sharedscripts
    postrotate
        systemctl reload gxc-node > /dev/null 2>&1 || true
    endscript
}
EOF
    
    log_info "Log rotation configured"
}

configure_backup_cron() {
    log_info "Configuring automatic backups..."
    
    cat > /etc/cron.d/gxc-backup <<EOF
# GXC Blockchain Automatic Backups
# Wallet backup every 6 hours
0 */6 * * * $GXC_USER $GXC_BIN_DIR/gxc-cli -conf=$GXC_CONFIG_DIR/gxc.conf backupwallet $GXC_BACKUP_DIR/wallet-\$(date +\%Y\%m\%d-\%H\%M\%S).dat

# Blockchain snapshot weekly
0 0 * * 0 root tar -czf $GXC_SNAPSHOT_DIR/blockchain-\$(date +\%Y\%m\%d).tar.gz -C $GXC_HOME blockchain_mainnet.db

# Cleanup old backups (keep last 30 days)
0 1 * * * root find $GXC_BACKUP_DIR -name "wallet-*.dat" -mtime +30 -delete
0 1 * * * root find $GXC_SNAPSHOT_DIR -name "blockchain-*.tar.gz" -mtime +30 -delete
EOF
    
    chmod 644 /etc/cron.d/gxc-backup
    
    log_info "Automatic backups configured"
}

configure_monitoring() {
    log_info "Setting up basic monitoring..."
    
    cat > /usr/local/bin/gxc-healthcheck.sh <<'EOF'
#!/bin/bash
# GXC Node Health Check Script

GXC_CLI="/usr/local/bin/gxc-cli"
CONFIG="/etc/gxc/gxc.conf"
LOG="/var/log/gxc/healthcheck.log"

check_node() {
    if ! $GXC_CLI -conf=$CONFIG getblockchaininfo > /dev/null 2>&1; then
        echo "$(date): Node is not responding" >> $LOG
        systemctl restart gxc-node
        echo "$(date): Node restarted" >> $LOG
    fi
}

check_peers() {
    PEERS=$($GXC_CLI -conf=$CONFIG getpeerinfo | jq length)
    if [ "$PEERS" -lt 3 ]; then
        echo "$(date): Low peer count: $PEERS" >> $LOG
    fi
}

check_disk() {
    USAGE=$(df -h /var/lib/gxc | awk 'NR==2 {print $5}' | sed 's/%//')
    if [ "$USAGE" -gt 80 ]; then
        echo "$(date): Disk usage high: ${USAGE}%" >> $LOG
    fi
}

check_node
check_peers
check_disk
EOF
    
    chmod 755 /usr/local/bin/gxc-healthcheck.sh
    
    # Add to cron (every 5 minutes)
    echo "*/5 * * * * root /usr/local/bin/gxc-healthcheck.sh" > /etc/cron.d/gxc-healthcheck
    chmod 644 /etc/cron.d/gxc-healthcheck
    
    log_info "Health check monitoring configured"
}

set_system_limits() {
    log_info "Configuring system limits..."
    
    cat >> /etc/security/limits.conf <<EOF

# GXC Blockchain Node Limits
$GXC_USER soft nofile 65536
$GXC_USER hard nofile 65536
$GXC_USER soft nproc 32768
$GXC_USER hard nproc 32768
EOF
    
    log_info "System limits configured"
}

start_node() {
    log_info "Starting GXC node..."
    
    systemctl start gxc-node
    sleep 5
    
    if systemctl is-active --quiet gxc-node; then
        log_info "GXC node started successfully"
    else
        log_error "Failed to start GXC node"
        systemctl status gxc-node
        exit 1
    fi
}

print_summary() {
    echo ""
    echo "========================================="
    echo "  GXC Blockchain Node Deployment"
    echo "========================================="
    echo ""
    log_info "Deployment completed successfully!"
    echo ""
    echo "Configuration:"
    echo "  Config file: $GXC_CONFIG_DIR/gxc.conf"
    echo "  Data directory: $GXC_HOME"
    echo "  Log directory: $GXC_LOG_DIR"
    echo "  Backup directory: $GXC_BACKUP_DIR"
    echo ""
    echo "Service Management:"
    echo "  Start:   systemctl start gxc-node"
    echo "  Stop:    systemctl stop gxc-node"
    echo "  Restart: systemctl restart gxc-node"
    echo "  Status:  systemctl status gxc-node"
    echo "  Logs:    journalctl -u gxc-node -f"
    echo ""
    echo "Node Commands:"
    echo "  Info:    gxc-cli -conf=$GXC_CONFIG_DIR/gxc.conf getblockchaininfo"
    echo "  Peers:   gxc-cli -conf=$GXC_CONFIG_DIR/gxc.conf getpeerinfo"
    echo "  Balance: gxc-cli -conf=$GXC_CONFIG_DIR/gxc.conf getbalance"
    echo ""
    echo "Next Steps:"
    echo "  1. Review configuration: $GXC_CONFIG_DIR/gxc.conf"
    echo "  2. Check node status: systemctl status gxc-node"
    echo "  3. Monitor logs: tail -f $GXC_LOG_DIR/gxc.log"
    echo "  4. Wait for blockchain sync"
    echo "  5. Configure monitoring and alerts"
    echo ""
    log_warn "IMPORTANT: Save your RPC credentials securely!"
    echo ""
}

# Main execution
main() {
    log_info "Starting GXC Blockchain production deployment..."
    
    check_root
    check_os
    install_dependencies
    create_user
    create_directories
    install_binaries
    configure_node
    configure_systemd
    configure_firewall
    configure_fail2ban
    configure_logrotate
    configure_backup_cron
    configure_monitoring
    set_system_limits
    start_node
    print_summary
}

# Run main function
main "$@"
