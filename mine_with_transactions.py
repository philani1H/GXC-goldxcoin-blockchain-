#!/usr/bin/env python3
"""
Mine blocks that include pending transactions from mempool
"""

import requests
import hashlib
import time
import json

NODE_URL = "http://localhost:8332"

def rpc_call(method, params=None):
    """Make RPC call to node"""
    if params is None:
        params = []
    
    payload = {
        "jsonrpc": "2.0",
        "method": method,
        "params": params,
        "id": 1
    }
    
    try:
        response = requests.post(NODE_URL, json=payload, timeout=10)
        if response.status_code == 200:
            result = response.json()
            if 'error' in result and result['error']:
                return None, result['error']
            return result.get('result'), None
        return None, f"HTTP {response.status_code}"
    except Exception as e:
        return None, str(e)

def mine_block_with_transactions(miner_address):
    """Mine a block including pending transactions"""
    
    # Get block template (includes pending transactions)
    template, error = rpc_call("getblocktemplate", [{"algorithm": "sha256"}])
    if error:
        print(f"Error getting template: {error}")
        return False
    
    height = template.get('height', 0)
    prev_hash = template.get('previousblockhash', '0' * 64)
    difficulty = template.get('difficulty', 0.1)
    transactions = template.get('transactions', [])
    
    print(f"Mining block {height}...")
    print(f"  Previous hash: {prev_hash[:16]}...")
    print(f"  Pending transactions: {len(transactions)}")
    
    # Mine with testnet difficulty
    nonce = 0
    start_time = time.time()
    
    while nonce < 100000:
        timestamp = int(time.time())
        
        # Create block header
        header = f"{height}{prev_hash}{nonce}{miner_address}{timestamp}"
        block_hash = hashlib.sha256(header.encode()).hexdigest()
        
        # Check if hash meets difficulty (starts with 0 for testnet)
        if block_hash.startswith('0'):
            elapsed = time.time() - start_time
            
            print(f"  ✓ Found valid hash: {block_hash[:16]}...")
            print(f"  Nonce: {nonce}, Time: {elapsed:.2f}s")
            
            # Build block data with transactions
            block_data = {
                "hash": block_hash,
                "previousblockhash": prev_hash,
                "height": height,
                "nonce": nonce,
                "miner": miner_address,
                "timestamp": timestamp,
                "difficulty": difficulty,
                "transactions": transactions  # Include pending transactions!
            }
            
            # Submit block
            result, error = rpc_call("submitblock", [block_data])
            if error:
                print(f"  ✗ Error submitting block: {error}")
                return False
            else:
                print(f"  ✓ Block submitted successfully!")
                return True
        
        nonce += 1
        
        if nonce % 10000 == 0:
            print(f"  Tried {nonce} nonces...", end='\r')
    
    print(f"  ✗ Could not find valid hash after {nonce} attempts")
    return False

if __name__ == "__main__":
    import sys
    
    if len(sys.argv) < 2:
        print("Usage: ./mine_with_transactions.py <miner_address> [num_blocks]")
        sys.exit(1)
    
    miner_address = sys.argv[1]
    num_blocks = int(sys.argv[2]) if len(sys.argv) > 2 else 1
    
    print(f"Mining {num_blocks} block(s) to {miner_address}")
    print()
    
    for i in range(num_blocks):
        if i > 0:
            print()
        success = mine_block_with_transactions(miner_address)
        if not success:
            print(f"Failed to mine block {i+1}")
            break
        time.sleep(1)
    
    print()
    print("Mining complete!")
