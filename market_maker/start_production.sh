#!/bin/bash
# GXC Admin API Production Server - Unix/Linux Startup Script

echo "========================================"
echo "GXC Admin API - Production Server"
echo "========================================"

# Set environment variables (customize as needed)
export SERVER_HOST=${SERVER_HOST:-"0.0.0.0"}
export SERVER_PORT=${SERVER_PORT:-"5002"}
export SERVER_THREADS=${SERVER_THREADS:-"4"}
export ADMIN_DB_PATH=${ADMIN_DB_PATH:-"gxc_admin.db"}

echo "Starting production server..."
echo "Host: $SERVER_HOST"
echo "Port: $SERVER_PORT"
echo "Threads: $SERVER_THREADS"
echo "Database: $ADMIN_DB_PATH"
echo ""

python3 wsgi_server.py
