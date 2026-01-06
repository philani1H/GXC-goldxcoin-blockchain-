#!/usr/bin/env python3
"""
Mine blocks to the node wallet so it has funds to send transactions
"""

import sys
import os

# Get the node wallet address
NODE_WALLET = "tGXC3faf86b6e70fe24ddab8cbbde4e8b93ef9"

# Read the miner script
with open('gxc_miner.py', 'r') as f:
    miner_code = f.read()

# Replace the miner address
miner_code = miner_code.replace(
    'MINER_ADDRESS = "tGXC9fab7317231b966af85ac453e168c0932"',
    f'MINER_ADDRESS = "{NODE_WALLET}"'
)

# Execute the modified miner
exec(miner_code)
