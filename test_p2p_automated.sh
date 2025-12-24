#!/bin/bash

echo "=========================================="
echo "GXC P2P Network Test - Automated"
echo "=========================================="

# Compile
echo "Compiling..."
g++ -std=c++17 -pthread test_p2p_simple.cpp -o test_p2p
if [ $? -ne 0 ]; then
    echo "❌ Compilation failed"
    exit 1
fi
echo "✅ Compiled"

# Start node 1 (server)
echo ""
echo "Starting Node 1 on port 8333..."
./test_p2p 8333 > /tmp/node1.log 2>&1 &
NODE1_PID=$!
sleep 2

# Check if node 1 is running
if ! ps -p $NODE1_PID > /dev/null; then
    echo "❌ Node 1 failed to start"
    cat /tmp/node1.log
    exit 1
fi
echo "✅ Node 1 started (PID: $NODE1_PID)"

# Start node 2 (client connecting to node 1)
echo ""
echo "Starting Node 2 on port 8334, connecting to 127.0.0.1:8333..."
./test_p2p 8334 127.0.0.1:8333 > /tmp/node2.log 2>&1 &
NODE2_PID=$!
sleep 2

# Check if node 2 is running
if ! ps -p $NODE2_PID > /dev/null; then
    echo "❌ Node 2 failed to start"
    cat /tmp/node2.log
    kill $NODE1_PID 2>/dev/null
    exit 1
fi
echo "✅ Node 2 started (PID: $NODE2_PID)"

# Wait for connection
sleep 2

# Check logs
echo ""
echo "=========================================="
echo "Node 1 Log:"
echo "=========================================="
cat /tmp/node1.log | grep -E "listening|connected|peer|Received"

echo ""
echo "=========================================="
echo "Node 2 Log:"
echo "=========================================="
cat /tmp/node2.log | grep -E "listening|connected|peer|Received"

# Cleanup
echo ""
echo "Cleaning up..."
kill $NODE1_PID $NODE2_PID 2>/dev/null
sleep 1
pkill -9 test_p2p 2>/dev/null

# Check results
if grep -q "New peer connected" /tmp/node1.log && grep -q "Connected to peer" /tmp/node2.log; then
    echo ""
    echo "=========================================="
    echo "✅ P2P TEST PASSED"
    echo "=========================================="
    echo "Node 1 accepted connection from Node 2"
    echo "Node 2 connected to Node 1"
    echo "Real peer-to-peer networking works!"
    exit 0
else
    echo ""
    echo "=========================================="
    echo "❌ P2P TEST FAILED"
    echo "=========================================="
    echo "Nodes did not connect properly"
    exit 1
fi
