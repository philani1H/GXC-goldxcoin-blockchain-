#!/usr/bin/env python3
"""
Simple Mainnet Miner for GXC
Mines blocks to a specified address
"""

import requests
import hashlib
import time
import sys

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

def mine_block(miner_address, num_blocks=1):
    """Mine blocks using proper PoW"""
    for i in range(num_blocks):
        print(f"\nMining block {i+1}/{num_blocks}...")
        
        # Get block template
        template, error = rpc_call("getblocktemplate", [{"algorithm": "sha256"}])
        if error:
            print(f"Error getting template: {error}")
            continue
        
        height = template.get('height', 0)
        prev_hash = template.get('previousblockhash', '0' * 64)
        difficulty = template.get('difficulty', 0.1)
        
        print(f"  Height: {height}")
        print(f"  Previous Hash: {prev_hash[:16]}...")
        print(f"  Difficulty: {difficulty}")
        
        # Mine with proper PoW
        nonce = 0
        start_time = time.time()
        
        while True:
            timestamp = int(time.time())
            
            # Create block header string (must match node's calculation)
            header = f"{height}{prev_hash}{nonce}{miner_address}{timestamp}"
            block_hash = hashlib.sha256(header.encode()).hexdigest()
            
            # Check if hash meets difficulty (starts with 0)
            if block_hash.startswith('0'):
                elapsed = time.time() - start_time
                hashrate = nonce / elapsed if elapsed > 0 else 0
                
                print(f"  ✓ Found valid hash!")
                print(f"    Hash: {block_hash}")
                print(f"    Nonce: {nonce}")
                print(f"    Time: {elapsed:.2f}s")
                print(f"    Hashrate: {hashrate:.0f} H/s")
                
                # Submit block
                block_data = {
                    "hash": block_hash,
                    "previousblockhash": prev_hash,
                    "height": height,
                    "nonce": nonce,
                    "miner": miner_address,
                    "timestamp": timestamp,
                    "difficulty": difficulty,
                    "transactions": []
                }
                
                result, error = rpc_call("submitblock", [block_data])
                if error:
                    print(f"  ✗ Error submitting block: {error}")
                else:
                    print(f"  ✓ Block submitted successfully!")
                
                break
            
            nonce += 1
            
            if nonce % 10000 == 0:
                print(f"  Tried {nonce} nonces...", end='\r')
        
        time.sleep(1)

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: ./mine_mainnet_simple.py <miner_address> [num_blocks]")
        sys.exit(1)
    
    miner_address = sys.argv[1]
    num_blocks = int(sys.argv[2]) if len(sys.argv) > 2 else 1
    
    print(f"Mining {num_blocks} blocks to {miner_address}")
    mine_block(miner_address, num_blocks)
    
    # Check balance
    balance, error = rpc_call("getbalance", [miner_address])
    if not error:
        print(f"\nFinal balance: {balance} GXC")
