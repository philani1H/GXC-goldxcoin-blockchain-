#!/bin/bash
#
# GXC Blockchain - Security Hardening Script
# ===========================================
# This script applies security hardening measures to GXC blockchain nodes
#
# Usage: sudo ./security-hardening.sh
#
# Version: 2.1.0
# Last Updated: December 9, 2024

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

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
        log_error "This script must be run as root"
        exit 1
    fi
}

# 1. SSH Hardening
harden_ssh() {
    log_info "Hardening SSH configuration..."
    
    SSH_CONFIG="/etc/ssh/sshd_config"
    
    # Backup original config
    cp $SSH_CONFIG ${SSH_CONFIG}.backup
    
    # Disable root login
    sed -i 's/^#*PermitRootLogin.*/PermitRootLogin no/' $SSH_CONFIG
    
    # Disable password authentication (use keys only)
    sed -i 's/^#*PasswordAuthentication.*/PasswordAuthentication no/' $SSH_CONFIG
    
    # Disable empty passwords
    sed -i 's/^#*PermitEmptyPasswords.*/PermitEmptyPasswords no/' $SSH_CONFIG
    
    # Use protocol 2 only
    sed -i 's/^#*Protocol.*/Protocol 2/' $SSH_CONFIG
    
    # Limit max authentication attempts
    sed -i 's/^#*MaxAuthTries.*/MaxAuthTries 3/' $SSH_CONFIG
    
    # Set login grace time
    sed -i 's/^#*LoginGraceTime.*/LoginGraceTime 30/' $SSH_CONFIG
    
    systemctl restart sshd
    
    log_info "SSH hardened successfully"
}

# 2. Firewall Configuration
configure_firewall() {
    log_info "Configuring firewall..."
    
    # Install UFW if not present
    apt-get install -y ufw
    
    # Default policies
    ufw default deny incoming
    ufw default allow outgoing
    
    # Allow SSH (change port if using non-standard)
    ufw allow 22/tcp
    
    # Allow GXC P2P
    ufw allow 8333/tcp
    
    # Deny RPC by default (use SSH tunnel)
    ufw deny 8332/tcp
    
    # Rate limiting for SSH
    ufw limit 22/tcp
    
    # Enable firewall
    ufw --force enable
    
    log_info "Firewall configured"
}

# 3. Fail2Ban Configuration
configure_fail2ban() {
    log_info "Configuring Fail2Ban..."
    
    apt-get install -y fail2ban
    
    # SSH jail
    cat > /etc/fail2ban/jail.d/sshd.local <<EOF
[sshd]
enabled = true
port = 22
filter = sshd
logpath = /var/log/auth.log
maxretry = 3
bantime = 3600
findtime = 600
EOF
    
    # GXC RPC jail
    cat > /etc/fail2ban/jail.d/gxc-rpc.local <<EOF
[gxc-rpc]
enabled = true
port = 8332
filter = gxc-rpc
logpath = /var/log/gxc/gxc.log
maxretry = 5
bantime = 7200
findtime = 600
EOF
    
    systemctl restart fail2ban
    systemctl enable fail2ban
    
    log_info "Fail2Ban configured"
}

# 4. System Updates
apply_system_updates() {
    log_info "Applying system updates..."
    
    apt-get update
    apt-get upgrade -y
    apt-get dist-upgrade -y
    apt-get autoremove -y
    apt-get autoclean
    
    log_info "System updated"
}

# 5. Disable Unnecessary Services
disable_unnecessary_services() {
    log_info "Disabling unnecessary services..."
    
    SERVICES_TO_DISABLE=(
        "avahi-daemon"
        "cups"
        "bluetooth"
    )
    
    for service in "${SERVICES_TO_DISABLE[@]}"; do
        if systemctl is-enabled --quiet $service 2>/dev/null; then
            systemctl disable $service
            systemctl stop $service
            log_info "Disabled $service"
        fi
    done
}

# 6. Kernel Hardening
harden_kernel() {
    log_info "Applying kernel hardening..."
    
    cat >> /etc/sysctl.conf <<EOF

# GXC Security Hardening
# IP Forwarding
net.ipv4.ip_forward = 0
net.ipv6.conf.all.forwarding = 0

# SYN Cookies
net.ipv4.tcp_syncookies = 1

# Ignore ICMP redirects
net.ipv4.conf.all.accept_redirects = 0
net.ipv6.conf.all.accept_redirects = 0

# Ignore source routed packets
net.ipv4.conf.all.accept_source_route = 0
net.ipv6.conf.all.accept_source_route = 0

# Log Martians
net.ipv4.conf.all.log_martians = 1

# Ignore ICMP ping requests
net.ipv4.icmp_echo_ignore_all = 1

# Increase TCP backlog
net.core.netdev_max_backlog = 5000

# Increase max connections
net.core.somaxconn = 1024

# TCP hardening
net.ipv4.tcp_max_syn_backlog = 2048
net.ipv4.tcp_synack_retries = 2
net.ipv4.tcp_syn_retries = 5

# Protect against time-wait assassination
net.ipv4.tcp_rfc1337 = 1
EOF
    
    sysctl -p
    
    log_info "Kernel hardened"
}

# 7. File Permissions
secure_file_permissions() {
    log_info "Securing file permissions..."
    
    # GXC directories
    chmod 700 /var/lib/gxc
    chmod 700 /etc/gxc
    chmod 755 /var/log/gxc
    
    # GXC configuration
    chmod 600 /etc/gxc/gxc.conf
    
    # Wallet file
    if [ -f /var/lib/gxc/wallet.dat ]; then
        chmod 600 /var/lib/gxc/wallet.dat
    fi
    
    # SSH keys
    chmod 700 ~/.ssh
    chmod 600 ~/.ssh/authorized_keys 2>/dev/null || true
    
    log_info "File permissions secured"
}

# 8. Audit Logging
configure_audit_logging() {
    log_info "Configuring audit logging..."
    
    apt-get install -y auditd
    
    # Audit GXC files
    auditctl -w /etc/gxc/gxc.conf -p wa -k gxc_config
    auditctl -w /var/lib/gxc/wallet.dat -p wa -k gxc_wallet
    auditctl -w /usr/local/bin/gxc-node -p x -k gxc_execution
    
    systemctl enable auditd
    systemctl restart auditd
    
    log_info "Audit logging configured"
}

# 9. Automatic Security Updates
configure_auto_updates() {
    log_info "Configuring automatic security updates..."
    
    apt-get install -y unattended-upgrades
    
    cat > /etc/apt/apt.conf.d/50unattended-upgrades <<EOF
Unattended-Upgrade::Allowed-Origins {
    "\${distro_id}:\${distro_codename}-security";
};
Unattended-Upgrade::AutoFixInterruptedDpkg "true";
Unattended-Upgrade::MinimalSteps "true";
Unattended-Upgrade::Remove-Unused-Kernel-Packages "true";
Unattended-Upgrade::Remove-Unused-Dependencies "true";
Unattended-Upgrade::Automatic-Reboot "false";
EOF
    
    cat > /etc/apt/apt.conf.d/20auto-upgrades <<EOF
APT::Periodic::Update-Package-Lists "1";
APT::Periodic::Unattended-Upgrade "1";
APT::Periodic::AutocleanInterval "7";
EOF
    
    log_info "Automatic security updates configured"
}

# 10. Intrusion Detection
install_intrusion_detection() {
    log_info "Installing intrusion detection..."
    
    apt-get install -y rkhunter chkrootkit
    
    # Configure rkhunter
    rkhunter --update
    rkhunter --propupd
    
    # Add daily scan
    cat > /etc/cron.daily/rkhunter <<EOF
#!/bin/bash
/usr/bin/rkhunter --cronjob --update --quiet
EOF
    chmod 755 /etc/cron.daily/rkhunter
    
    log_info "Intrusion detection installed"
}

# 11. Password Policy
configure_password_policy() {
    log_info "Configuring password policy..."
    
    apt-get install -y libpam-pwquality
    
    # Set password requirements
    sed -i 's/^# minlen.*/minlen = 12/' /etc/security/pwquality.conf
    sed -i 's/^# dcredit.*/dcredit = -1/' /etc/security/pwquality.conf
    sed -i 's/^# ucredit.*/ucredit = -1/' /etc/security/pwquality.conf
    sed -i 's/^# lcredit.*/lcredit = -1/' /etc/security/pwquality.conf
    sed -i 's/^# ocredit.*/ocredit = -1/' /etc/security/pwquality.conf
    
    log_info "Password policy configured"
}

# 12. Disable IPv6 (if not needed)
disable_ipv6() {
    log_info "Disabling IPv6..."
    
    cat >> /etc/sysctl.conf <<EOF

# Disable IPv6
net.ipv6.conf.all.disable_ipv6 = 1
net.ipv6.conf.default.disable_ipv6 = 1
net.ipv6.conf.lo.disable_ipv6 = 1
EOF
    
    sysctl -p
    
    log_info "IPv6 disabled"
}

# 13. Security Scan
run_security_scan() {
    log_info "Running security scan..."
    
    # Check for rootkits
    rkhunter --check --skip-keypress --report-warnings-only
    
    # Check for vulnerabilities
    if command -v lynis &> /dev/null; then
        lynis audit system --quick
    else
        log_warn "Lynis not installed. Install with: apt-get install lynis"
    fi
    
    log_info "Security scan complete"
}

# 14. Generate Security Report
generate_security_report() {
    log_info "Generating security report..."
    
    REPORT="/var/log/gxc/security-report-$(date +%Y%m%d).txt"
    
    {
        echo "GXC Blockchain Security Report"
        echo "Generated: $(date)"
        echo "================================"
        echo ""
        echo "System Information:"
        uname -a
        echo ""
        echo "Firewall Status:"
        ufw status verbose
        echo ""
        echo "Fail2Ban Status:"
        fail2ban-client status
        echo ""
        echo "Open Ports:"
        ss -tulpn
        echo ""
        echo "Failed Login Attempts:"
        grep "Failed password" /var/log/auth.log | tail -20
        echo ""
        echo "GXC Node Status:"
        systemctl status gxc-node --no-pager
        echo ""
    } > $REPORT
    
    log_info "Security report saved to: $REPORT"
}

# Main execution
main() {
    log_info "Starting GXC Blockchain security hardening..."
    echo ""
    
    check_root
    
    log_warn "This script will apply security hardening measures."
    log_warn "Some changes may affect system accessibility."
    read -p "Continue? (y/N) " -n 1 -r
    echo ""
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
    
    apply_system_updates
    harden_ssh
    configure_firewall
    configure_fail2ban
    disable_unnecessary_services
    harden_kernel
    secure_file_permissions
    configure_audit_logging
    configure_auto_updates
    install_intrusion_detection
    configure_password_policy
    
    # Optional: Disable IPv6
    read -p "Disable IPv6? (y/N) " -n 1 -r
    echo ""
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        disable_ipv6
    fi
    
    run_security_scan
    generate_security_report
    
    echo ""
    log_info "Security hardening completed!"
    echo ""
    log_warn "IMPORTANT: Review the security report at /var/log/gxc/security-report-*.txt"
    log_warn "IMPORTANT: Test SSH access before logging out!"
    echo ""
}

main "$@"
