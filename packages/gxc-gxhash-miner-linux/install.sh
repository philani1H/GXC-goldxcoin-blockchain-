#!/bin/bash
echo "Installing GXC GXHash Miner..."
echo ""

if ! command -v python3 &> /dev/null; then
    echo "Python 3 not found! Please install Python 3.7+"
    exit 1
fi

echo "Installing dependencies..."
pip3 install -r requirements.txt --user

echo ""
echo "??? Installation complete!"
echo ""
echo "You can now run: python3 gxhash_miner.py"