#!/usr/bin/env python3
"""
Mine blocks to pool address on Railway node
"""
import requests
import json
import time
import hashlib

NODE_URL = "https://gxc-chain112-blockchain-node-production.up.railway.app"
POOL_ADDRESS = "tGXC4VoV6DfSnM7YwTLCciBUzizARg2H64T7n"
NUM_BLOCKS = 5

def mine_block(block_data, difficulty):
    """Simple proof-of-work mining"""
    nonce = 0
    target = "0" * int(difficulty)
    
    while True:
        block_data['nonce'] = nonce
        block_str = json.dumps(block_data, sort_keys=True)
        block_hash = hashlib.sha256(block_str.encode()).hexdigest()
        
        if block_hash.startswith(target):
            return nonce, block_hash
        
        nonce += 1
        if nonce % 10000 == 0:
            print(f"  Tried {nonce} nonces...")

def rpc_call(method, params=None):
    """Make JSON-RPC call to node"""
    payload = {
        "jsonrpc": "2.0",
        "id": 1,
        "method": method,
        "params": params or []
    }
    
    response = requests.post(NODE_URL, json=payload, timeout=30)
    response.raise_for_status()
    
    result = response.json()
    if "error" in result and result["error"]:
        raise Exception(f"RPC Error: {result['error']}")
    
    return result.get("result")

def main():
    print(f"Mining {NUM_BLOCKS} blocks to pool address: {POOL_ADDRESS}")
    print(f"Node: {NODE_URL}\n")
    
    for i in range(NUM_BLOCKS):
        print(f"[Block {i+1}/{NUM_BLOCKS}]")
        
        # Get block template
        try:
            template = rpc_call("getblocktemplate", [{"minerAddress": POOL_ADDRESS}])
            
            print(f"  Height: {template.get('height', 'unknown')}")
            print(f"  Difficulty: {template.get('difficulty', 'unknown')}")
            print(f"  Transactions: {len(template.get('transactions', []))}")
            
        except Exception as e:
            print(f"  ❌ Error getting template: {e}")
            continue
        
        # Mine the block
        print(f"  Mining...")
        start_time = time.time()
        
        try:
            nonce, block_hash = mine_block(template, template.get('difficulty', 1))
            elapsed = time.time() - start_time
            
            print(f"  ✅ Found nonce: {nonce} in {elapsed:.2f}s")
            print(f"  Hash: {block_hash}")
            
        except Exception as e:
            print(f"  ❌ Mining error: {e}")
            continue
        
        # Submit the block
        try:
            template['nonce'] = nonce
            template['hash'] = block_hash
            
            result = rpc_call("submitblock", [template])
            print(f"  ✅ Block submitted: {result}")
                
        except Exception as e:
            print(f"  ❌ Submit error: {e}")
        
        print()
        time.sleep(1)
    
    print("\n" + "="*60)
    print("Mining complete! Checking pool balance...")
    print("="*60 + "\n")
    
    # Check balance
    try:
        balance = rpc_call("getbalance", [POOL_ADDRESS])
        print(f"Pool Address: {POOL_ADDRESS}")
        print(f"Balance: {balance} GXC")
    except Exception as e:
        print(f"❌ Balance check error: {e}")

if __name__ == "__main__":
    main()
