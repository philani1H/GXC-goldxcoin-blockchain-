#!/bin/bash
# GXC Complete Ecosystem Startup Script
# Starts blockchain node, pools, and all services

echo "🚀 Starting GXC Complete Ecosystem"
echo "======================================"

# Create necessary directories
mkdir -p gxc_data gxc_testnet_data logs pids mining_pool/static mining_pool/templates

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Function to start a service
start_service() {
    local name=$1
    local command=$2
    local port=$3
    local log_file="logs/${name}.log"
    local pid_file="pids/${name}.pid"
    
    echo -e "${YELLOW}Starting $name on port $port...${NC}"
    
    # Check if already running
    if [ -f "$pid_file" ]; then
        pid=$(cat "$pid_file")
        if ps -p $pid > /dev/null 2>&1; then
            echo -e "  ${YELLOW}⚠️  $name already running (PID: $pid)${NC}"
            return
        fi
    fi
    
    # Start the service
    nohup $command > "$log_file" 2>&1 &
    echo $! > "$pid_file"
    
    echo -e "  ${GREEN}✅ $name started (PID: $(cat $pid_file))${NC}"
    echo "     Log: $log_file"
}

# Function to wait for service to be ready
wait_for_service() {
    local name=$1
    local port=$2
    local max_wait=30
    local count=0
    
    echo -e "${YELLOW}Waiting for $name to be ready...${NC}"
    
    while [ $count -lt $max_wait ]; do
        if nc -z localhost $port 2>/dev/null; then
            echo -e "  ${GREEN}✅ $name is ready${NC}"
            return 0
        fi
        sleep 1
        count=$((count + 1))
    done
    
    echo -e "  ${RED}❌ $name failed to start${NC}"
    return 1
}

echo ""
echo "======================================"
echo "STEP 1: Starting Blockchain Node"
echo "======================================"

# 1. Start Blockchain Node (Testnet)
start_service "blockchain_node" "python3 testnet_blockchain_node.py" "18332"
wait_for_service "blockchain_node" "18332"

echo ""
echo "======================================"
echo "STEP 2: Starting Mining Pools"
echo "======================================"

# 2. Start General Mining Pool (SHA256)
start_service "pool_general" "python3 mining_pool/general_pool.py" "3333"
sleep 2

# 3. Start GPU Mining Pool (Ethash)
start_service "pool_gpu" "python3 mining_pool/gpu_pool.py" "3334"
sleep 2

# 4. Start ASIC Mining Pool (SHA256 optimized)
start_service "pool_asic" "python3 mining_pool/asic_pool.py" "3335"
sleep 2

echo ""
echo "======================================"
echo "STEP 3: Starting Web Services"
echo "======================================"

# 5. Start Blockchain Explorer
start_service "explorer" "python3 web/blockchain_explorer.py" "3000"
sleep 2

# 6. Start Wallet Service API
start_service "wallet_api" "python3 api/wallet_service.py" "5000"
sleep 2

# 7. Start Forum
start_service "forum" "python3 web/forum.py" "3001"
sleep 2

# Wait for all services to be ready
sleep 5

echo ""
echo "======================================"
echo "STEP 4: Verifying Connections"
echo "======================================"

# Test blockchain node connection
echo -e "${YELLOW}Testing blockchain node...${NC}"
if curl -s -X POST http://localhost:18332 \
    -H "Content-Type: application/json" \
    -d '{"jsonrpc":"2.0","method":"getblockcount","params":[],"id":1}' \
    > /dev/null 2>&1; then
    echo -e "  ${GREEN}✅ Blockchain node responding${NC}"
else
    echo -e "  ${YELLOW}⚠️  Blockchain node not responding (may still be starting)${NC}"
fi

# Test mining pools
echo -e "${YELLOW}Testing mining pools...${NC}"
for port in 3333 3334 3335; do
    if nc -z localhost $port 2>/dev/null; then
        echo -e "  ${GREEN}✅ Pool on port $port is listening${NC}"
    else
        echo -e "  ${RED}❌ Pool on port $port not responding${NC}"
    fi
done

echo ""
echo "======================================"
echo "✅ GXC Complete Ecosystem Started!"
echo "======================================"
echo ""
echo "📊 Services Running:"
echo ""
echo "  BLOCKCHAIN:"
echo "    • Node RPC:           http://localhost:18332"
echo ""
echo "  MINING POOLS:"
echo "    • General Pool:       stratum+tcp://localhost:3333"
echo "    • GPU Pool:           stratum+tcp://localhost:3334"
echo "    • ASIC Pool:          stratum+tcp://localhost:3335"
echo ""
echo "  WEB SERVICES:"
echo "    • Explorer:           http://localhost:3000"
echo "    • Wallet API:         http://localhost:5000"
echo "    • Forum:              http://localhost:3001"
echo ""
echo "📝 Logs:"
echo "    • View all logs:      tail -f logs/*.log"
echo "    • Blockchain node:    tail -f logs/blockchain_node.log"
echo "    • Mining pools:       tail -f logs/pool_*.log"
echo ""
echo "🛑 To stop all services:"
echo "    ./stop_complete_ecosystem.sh"
echo ""
echo "⛏️  To start mining:"
echo "    1. Download miners from: ./miners/installers/"
echo "    2. Run the installer for your platform"
echo "    3. Open the miner application"
echo "    4. Connect to pool: localhost:3333"
echo ""

# Show process status
echo "🔍 Process Status:"
for pid_file in pids/*.pid; do
    if [ -f "$pid_file" ]; then
        service_name=$(basename "$pid_file" .pid)
        pid=$(cat "$pid_file")
        if ps -p $pid > /dev/null 2>&1; then
            echo -e "  ${GREEN}✅ $service_name (PID: $pid) - Running${NC}"
        else
            echo -e "  ${RED}❌ $service_name (PID: $pid) - Not running${NC}"
        fi
    fi
done

echo ""
echo "🎉 Ready to mine! Connect your miners to the pools above."
echo ""
