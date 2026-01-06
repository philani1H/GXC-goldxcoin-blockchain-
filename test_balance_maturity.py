#!/usr/bin/env python3
"""
Test balance updates after mining multiple blocks
"""

import requests
import hashlib
import time

RPC_URL = "http://localhost:8332"
MINER_ADDRESS = "tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf"

def rpc_call(method, params=None):
    payload = {"jsonrpc": "2.0", "method": method, "params": params or [], "id": 1}
    try:
        response = requests.post(RPC_URL, json=payload, timeout=10)
        result = response.json()
        if 'error' in result and result['error']:
            return None
        return result.get('result')
    except Exception as e:
        print(f"Error: {e}")
        return None

def mine_block():
    """Mine a single block"""
    template = rpc_call("getblocktemplate", [MINER_ADDRESS])
    if not template:
        return False
    
    height = template.get('height', 1)
    prev_hash = template.get('previousblockhash', '0' * 64)
    difficulty = template.get('difficulty', 1.0)
    timestamp = int(time.time())
    
    # Mine
    target = '0' * int(difficulty)
    for nonce in range(1000000):
        block_data = f"{height}{prev_hash}{MINER_ADDRESS}{timestamp}{nonce}"
        hash_result = hashlib.sha256(block_data.encode()).hexdigest()
        
        if hash_result.startswith(target):
            # Submit
            block = {
                "height": height,
                "hash": hash_result,
                "previousblockhash": prev_hash,
                "nonce": nonce,
                "timestamp": timestamp,
                "difficulty": difficulty,
                "miner": MINER_ADDRESS,
                "transactions": template.get('transactions', [])
            }
            
            result = rpc_call("submitblock", [block])
            return result is None
    
    return False

print("=" * 80)
print("BALANCE MATURITY TEST")
print("=" * 80)
print()

# Check initial state
height = rpc_call("getblockcount")
balance = rpc_call("getbalance", [MINER_ADDRESS])

print(f"Initial State:")
print(f"  Height: {height}")
print(f"  Balance: {balance} GXC")
print()

# Mine blocks until we have 6+ confirmations
print("Mining blocks to reach maturity (6 confirmations)...")
print()

for i in range(7):
    print(f"Mining block {i+1}/7...", end=" ")
    if mine_block():
        time.sleep(0.5)
        height = rpc_call("getblockcount")
        balance = rpc_call("getbalance", [MINER_ADDRESS])
        print(f"✓ Height: {height}, Balance: {balance} GXC")
    else:
        print("✗ Failed")
        break

print()
print("=" * 80)
print("FINAL STATE")
print("=" * 80)

height = rpc_call("getblockcount")
balance = rpc_call("getbalance", [MINER_ADDRESS])

print(f"Blockchain Height: {height}")
print(f"Miner Balance: {balance} GXC")
print()

if balance and balance > 0:
    print("✓ SUCCESS: Balance updated after maturity!")
    print(f"  Genesis block (50 GXC) is now spendable")
    print(f"  Total rewards: {balance} GXC")
else:
    print("⚠ Balance still 0 - may need more confirmations or debugging")

print()
print("=" * 80)
