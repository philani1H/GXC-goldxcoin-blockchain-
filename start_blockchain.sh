#!/bin/bash
# GXC Blockchain Startup Script
# Starts all blockchain services

echo "🚀 Starting GXC Blockchain Ecosystem"
echo "======================================"

# Create necessary directories
mkdir -p gxc_data gxc_testnet_data logs pids

# Function to start a service
start_service() {
    local name=$1
    local command=$2
    local port=$3
    local log_file="logs/${name}.log"
    local pid_file="pids/${name}.pid"
    
    echo "Starting $name on port $port..."
    
    # Check if already running
    if [ -f "$pid_file" ]; then
        pid=$(cat "$pid_file")
        if ps -p $pid > /dev/null 2>&1; then
            echo "  ⚠️  $name already running (PID: $pid)"
            return
        fi
    fi
    
    # Start the service
    nohup $command > "$log_file" 2>&1 &
    echo $! > "$pid_file"
    
    echo "  ✅ $name started (PID: $(cat $pid_file))"
    echo "     Log: $log_file"
}

# 1. Start Blockchain Node (Testnet)
echo ""
echo "1️⃣  Starting Blockchain Node..."
start_service "blockchain_node" "python3 testnet_blockchain_node.py" "18332"

# Wait for node to initialize
sleep 2

# 2. Start Blockchain Explorer
echo ""
echo "2️⃣  Starting Blockchain Explorer..."
start_service "explorer" "python3 web/blockchain_explorer.py" "3000"

# 3. Start Wallet Service API
echo ""
echo "3️⃣  Starting Wallet Service..."
start_service "wallet_api" "python3 api/wallet_service.py" "5000"

# 4. Start Forum (optional)
echo ""
echo "4️⃣  Starting Forum..."
start_service "forum" "python3 web/forum.py" "3001"

# Wait for services to start
sleep 3

echo ""
echo "======================================"
echo "✅ GXC Blockchain Ecosystem Started!"
echo "======================================"
echo ""
echo "📊 Services Running:"
echo "  • Blockchain Node:    http://localhost:18332"
echo "  • Explorer:           http://localhost:3000"
echo "  • Wallet API:         http://localhost:5000"
echo "  • Forum:              http://localhost:3001"
echo ""
echo "📝 Logs:"
echo "  • View logs: tail -f logs/*.log"
echo "  • All logs in: ./logs/"
echo ""
echo "🛑 To stop all services:"
echo "  ./stop_blockchain.sh"
echo ""
echo "📖 Documentation:"
echo "  • Wallet Recovery: WALLET_RECOVERY_GUIDE.md"
echo "  • Bug Fix: BUG_FIX_SUMMARY.md"
echo ""

# Show process status
echo "🔍 Process Status:"
for pid_file in pids/*.pid; do
    if [ -f "$pid_file" ]; then
        service_name=$(basename "$pid_file" .pid)
        pid=$(cat "$pid_file")
        if ps -p $pid > /dev/null 2>&1; then
            echo "  ✅ $service_name (PID: $pid) - Running"
        else
            echo "  ❌ $service_name (PID: $pid) - Not running"
        fi
    fi
done

echo ""
echo "🎉 Ready to use! Open http://localhost:3000 in your browser"
