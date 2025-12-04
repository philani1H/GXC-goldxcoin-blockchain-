#!/bin/bash
# GXC Miner Pro - Quick Install
# Run: chmod +x install.sh && ./install.sh

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Detect OS and run appropriate installer
if [[ "$OSTYPE" == "darwin"* ]]; then
    exec "${SCRIPT_DIR}/installer/install-macos.sh" "$@"
else
    exec "${SCRIPT_DIR}/installer/install-linux.sh" "$@"
fi
