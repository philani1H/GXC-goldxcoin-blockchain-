#!/bin/bash

# GXC Blockchain Ecosystem Startup Script
# Starts all services for production deployment

set -e

echo "🚀 Starting GXC Blockchain Ecosystem..."

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_header() {
    echo -e "${BLUE}================================${NC}"
    echo -e "${BLUE} $1${NC}"
    echo -e "${BLUE}================================${NC}"
}

# Check if running as root (needed for port binding)
check_privileges() {
    if [[ $EUID -eq 0 ]]; then
        print_warning "Running as root. Consider using a dedicated user for production."
    fi
}

# Install system dependencies
install_dependencies() {
    print_header "INSTALLING DEPENDENCIES"
    
    # Update package lists
    print_status "Updating package lists..."
    sudo apt-get update -qq
    
    # Install Python dependencies
    print_status "Installing Python packages..."
    pip3 install -q flask flask-cors flask-socketio sqlite3 bcrypt pyjwt cryptography qrcode[pil] requests yfinance numpy
    
    # Install Node.js for additional tools (if needed)
    if ! command -v node &> /dev/null; then
        print_status "Installing Node.js..."
        curl -fsSL https://deb.nodesource.com/setup_18.x | sudo -E bash -
        sudo apt-get install -y nodejs
    fi
    
    print_status "Dependencies installed successfully!"
}

# Start the blockchain node
start_blockchain() {
    print_header "STARTING GXC BLOCKCHAIN NODE"
    
    if [ ! -f "./build/gxc_blockchain" ]; then
        print_error "Blockchain binary not found. Please build the project first."
        exit 1
    fi
    
    print_status "Starting GXC blockchain node..."
    cd /workspace
    ./build/gxc_blockchain > logs/blockchain.log 2>&1 &
    BLOCKCHAIN_PID=$!
    echo $BLOCKCHAIN_PID > pids/blockchain.pid
    
    print_status "Blockchain node started (PID: $BLOCKCHAIN_PID)"
    
    # Wait for blockchain to initialize
    print_status "Waiting for blockchain to initialize..."
    sleep 10
}

# Start the wallet API service
start_wallet_api() {
    print_header "STARTING WALLET API SERVICE"
    
    print_status "Starting wallet creation API on port 5000..."
    cd /workspace
    python3 api/wallet_service.py > logs/wallet_api.log 2>&1 &
    WALLET_PID=$!
    echo $WALLET_PID > pids/wallet_api.pid
    
    print_status "Wallet API started (PID: $WALLET_PID)"
}

# Start the blockchain explorer
start_explorer() {
    print_header "STARTING BLOCKCHAIN EXPLORER"
    
    print_status "Starting blockchain explorer on port 3000..."
    cd /workspace
    python3 web/blockchain_explorer.py > logs/explorer.log 2>&1 &
    EXPLORER_PID=$!
    echo $EXPLORER_PID > pids/explorer.pid
    
    print_status "Blockchain explorer started (PID: $EXPLORER_PID)"
}

# Start the market maker
start_market_maker() {
    print_header "STARTING STOCK MARKET MAKER"
    
    print_status "Starting stock market maker on port 4000..."
    cd /workspace
    python3 market_maker/stock_market_maker.py > logs/market_maker.log 2>&1 &
    MARKET_MAKER_PID=$!
    echo $MARKET_MAKER_PID > pids/market_maker.pid
    
    print_status "Market maker started (PID: $MARKET_MAKER_PID)"
}

# Start the forum
start_forum() {
    print_header "STARTING FORUM"
    
    print_status "Starting forum on port 3001..."
    cd /workspace
    python3 web/forum.py > logs/forum.log 2>&1 &
    FORUM_PID=$!
    echo $FORUM_PID > pids/forum.pid
    
    print_status "Forum started (PID: $FORUM_PID)"
}

# Start the forum real-time server
start_forum_realtime() {
    print_header "STARTING FORUM REAL-TIME SERVER"
    
    print_status "Starting forum real-time server on port 3002..."
    cd /workspace
    python3 web/forum_realtime_server.py > logs/forum_realtime.log 2>&1 &
    FORUM_REALTIME_PID=$!
    echo $FORUM_REALTIME_PID > pids/forum_realtime.pid
    
    print_status "Forum real-time server started (PID: $FORUM_REALTIME_PID)"
}

# Create necessary directories
setup_directories() {
    print_status "Creating necessary directories..."
    
    mkdir -p logs
    mkdir -p pids
    mkdir -p wallet_backups
    mkdir -p data
    
    print_status "Directories created successfully!"
}

# Health check for all services
health_check() {
    print_header "PERFORMING HEALTH CHECKS"
    
    services=(
        "blockchain:8545"
        "wallet_api:5000"
        "explorer:3000"
        "market_maker:4000"
        "forum:3001"
        "forum_realtime:3002"
    )
    
    for service in "${services[@]}"; do
        name=$(echo $service | cut -d: -f1)
        port=$(echo $service | cut -d: -f2)
        
        print_status "Checking $name service on port $port..."
        
        if curl -s http://localhost:$port/health > /dev/null 2>&1 || \
           curl -s http://localhost:$port/api/health > /dev/null 2>&1 || \
           curl -s http://localhost:$port > /dev/null 2>&1; then
            print_status "✅ $name service is healthy"
        else
            print_warning "⚠️  $name service might not be responding"
        fi
        
        sleep 2
    done
}

# Display service information
show_service_info() {
    print_header "GXC ECOSYSTEM SERVICES"
    
    echo ""
    echo -e "${GREEN}🔗 Blockchain Node:${NC}       http://localhost:8545 (RPC)"
    echo -e "${GREEN}💼 Wallet API:${NC}           http://localhost:5000"
    echo -e "${GREEN}🔍 Blockchain Explorer:${NC}  http://localhost:3000"
    echo -e "${GREEN}📈 Market Maker:${NC}         http://localhost:4000"
    echo -e "${GREEN}💬 Forum:${NC}               http://localhost:3001"
    echo -e "${GREEN}⚡ Forum Real-Time:${NC}      http://localhost:3002"
    echo ""
    
    echo -e "${BLUE}📁 Log Files:${NC}"
    echo "  • Blockchain:    logs/blockchain.log"
    echo "  • Wallet API:    logs/wallet_api.log"
    echo "  • Explorer:      logs/explorer.log"
    echo "  • Market Maker:  logs/market_maker.log"
    echo "  • Forum:         logs/forum.log"
    echo "  • Forum Real-Time: logs/forum_realtime.log"
    echo ""
    
    echo -e "${BLUE}🆔 Process IDs:${NC}"
    if [ -f pids/blockchain.pid ]; then
        echo "  • Blockchain:    $(cat pids/blockchain.pid)"
    fi
    if [ -f pids/wallet_api.pid ]; then
        echo "  • Wallet API:    $(cat pids/wallet_api.pid)"
    fi
    if [ -f pids/explorer.pid ]; then
        echo "  • Explorer:      $(cat pids/explorer.pid)"
    fi
    if [ -f pids/market_maker.pid ]; then
        echo "  • Market Maker:  $(cat pids/market_maker.pid)"
    fi
    if [ -f pids/forum.pid ]; then
        echo "  • Forum:         $(cat pids/forum.pid)"
    fi
    if [ -f pids/forum_realtime.pid ]; then
        echo "  • Forum Real-Time: $(cat pids/forum_realtime.pid)"
    fi
    echo ""
    
    echo -e "${YELLOW}💡 Quick Start:${NC}"
    echo "  1. Create a wallet: curl -X POST http://localhost:5000/api/v1/register -d '{\"username\":\"user\",\"email\":\"user@example.com\",\"password\":\"password\"}' -H 'Content-Type: application/json'"
    echo "  2. Visit explorer:  http://localhost:3000"
    echo "  3. Check market:    http://localhost:4000"
    echo ""
    
    echo -e "${BLUE}🛑 To stop all services:${NC} ./scripts/stop_ecosystem.sh"
}

# Create stop script
create_stop_script() {
    cat > scripts/stop_ecosystem.sh << 'EOF'
#!/bin/bash

# GXC Blockchain Ecosystem Stop Script

echo "🛑 Stopping GXC Blockchain Ecosystem..."

# Function to stop service by PID file
stop_service() {
    local service_name=$1
    local pid_file="pids/${service_name}.pid"
    
    if [ -f "$pid_file" ]; then
        local pid=$(cat "$pid_file")
        if kill -0 "$pid" 2>/dev/null; then
            echo "Stopping $service_name (PID: $pid)..."
            kill "$pid"
            sleep 2
            
            # Force kill if still running
            if kill -0 "$pid" 2>/dev/null; then
                echo "Force stopping $service_name..."
                kill -9 "$pid"
            fi
        fi
        rm -f "$pid_file"
        echo "✅ $service_name stopped"
    else
        echo "⚠️  No PID file found for $service_name"
    fi
}

# Stop all services
stop_service "market_maker"
stop_service "explorer"
stop_service "wallet_api"
stop_service "blockchain"

echo "🔄 Cleaning up..."
# Kill any remaining processes
pkill -f "wallet_service.py" 2>/dev/null || true
pkill -f "blockchain_explorer.py" 2>/dev/null || true
pkill -f "stock_market_maker.py" 2>/dev/null || true
pkill -f "gxc_blockchain" 2>/dev/null || true

echo "✅ All services stopped successfully!"
EOF

    chmod +x scripts/stop_ecosystem.sh
}

# Main execution
main() {
    print_header "GXC BLOCKCHAIN ECOSYSTEM STARTUP"
    
    # Check privileges
    check_privileges
    
    # Setup
    setup_directories
    create_stop_script
    
    # Install dependencies
    if [[ "${1:-}" != "--skip-deps" ]]; then
        install_dependencies
    fi
    
    # Start services in order
    start_blockchain
    start_wallet_api
    start_explorer
    start_market_maker
    start_forum
    start_forum_realtime
    
    # Wait a bit for services to start
    print_status "Waiting for services to initialize..."
    sleep 15
    
    # Health checks
    health_check
    
    # Show service information
    show_service_info
    
    print_header "🎉 GXC ECOSYSTEM STARTED SUCCESSFULLY! 🎉"
    
    # Keep script running to monitor services
    if [[ "${1:-}" == "--monitor" ]]; then
        print_status "Monitoring services (Ctrl+C to exit)..."
        while true; do
            sleep 30
            health_check
        done
    fi
}

# Handle script interruption
trap 'echo ""; print_warning "Startup interrupted. Use ./scripts/stop_ecosystem.sh to clean up."; exit 1' INT

# Run main function
main "$@"