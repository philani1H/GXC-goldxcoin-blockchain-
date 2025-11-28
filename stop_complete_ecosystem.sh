#!/bin/bash
# GXC Complete Ecosystem Stop Script

echo "🛑 Stopping GXC Complete Ecosystem"
echo "======================================"

# Function to stop a service
stop_service() {
    local name=$1
    local pid_file="pids/${name}.pid"
    
    if [ -f "$pid_file" ]; then
        pid=$(cat "$pid_file")
        if ps -p $pid > /dev/null 2>&1; then
            echo "Stopping $name (PID: $pid)..."
            kill $pid
            sleep 1
            
            # Force kill if still running
            if ps -p $pid > /dev/null 2>&1; then
                echo "  Force stopping $name..."
                kill -9 $pid
            fi
            
            rm "$pid_file"
            echo "  ✅ $name stopped"
        else
            echo "  ⚠️  $name not running"
            rm "$pid_file"
        fi
    else
        echo "  ℹ️  $name PID file not found"
    fi
}

# Stop all services in reverse order
stop_service "forum"
stop_service "wallet_api"
stop_service "explorer"
stop_service "pool_asic"
stop_service "pool_gpu"
stop_service "pool_general"
stop_service "blockchain_node"

echo ""
echo "======================================"
echo "✅ All services stopped"
echo "======================================"
