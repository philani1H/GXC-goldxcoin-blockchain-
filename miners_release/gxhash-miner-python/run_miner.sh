#!/bin/bash
# GXC GXHash Miner - Quick Start Script

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Check if dependencies are installed
if ! python3 -c "import customtkinter" 2>/dev/null; then
    echo "Installing dependencies..."
    python3 -m pip install -r requirements.txt
fi

# Run the miner
python3 gxhash_miner.py "$@"
