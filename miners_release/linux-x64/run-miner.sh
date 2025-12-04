#!/bin/bash
# GXC Universal Miner - Quick Start Script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

if [ $# -eq 0 ]; then
    echo "GXC Universal Miner"
    echo "Usage: $0 -a YOUR_ADDRESS [--algorithm=gxhash]"
    echo ""
    "$SCRIPT_DIR/gxc-miner" --help
else
    "$SCRIPT_DIR/gxc-miner" "$@"
fi
