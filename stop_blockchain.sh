#!/bin/bash
# GXC Blockchain Stop Script
# Stops all blockchain services

echo "🛑 Stopping GXC Blockchain Ecosystem"
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

# Stop all services
stop_service "blockchain_node"
stop_service "explorer"
stop_service "wallet_api"
stop_service "forum"

echo ""
echo "======================================"
echo "✅ All services stopped"
echo "======================================"
