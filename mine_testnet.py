#!/usr/bin/env python3
"""
Simple testnet miner - mines blocks to your address
"""

import requests
import hashlib
import time
import json

MINER_ADDRESS = "tGXC9fab7317231b966af85ac453e168c0932"
RPC_URL = "http://localhost:18332"

def rpc_call(method, params=None):
    """Make RPC call to blockchain node"""
    if params is None:
        params = []
    
    payload = {
        "jsonrpc": "2.0",
        "method": method,
        "params": params,
        "id": 1
    }
    
    try:
        response = requests.post(RPC_URL, json=payload, timeout=5)
        result = response.json()
        return result.get('result')
    except Exception as e:
        print(f"RPC Error: {e}")
        return None

def mine_block():
    """Mine a single block"""
    print(f"\n⛏️  Mining block to: {MINER_ADDRESS}")
    
    # Get block template
    template = rpc_call("getblocktemplate", [MINER_ADDRESS])
    
    if not template:
        print("❌ Failed to get block template")
        return False
    
    if 'error' in template:
        print(f"❌ Error: {template['error']}")
        return False
    
    print(f"   Height: {template.get('height', 'unknown')}")
    print(f"   Mining...")
    
    # Simple mining (find nonce)
    nonce = 0
    start_time = time.time()
    
    while True:
        # Create block hash
        block_string = f"{template.get('height')}{template.get('previousblockhash')}{nonce}{time.time()}"
        block_hash = hashlib.sha256(block_string.encode()).hexdigest()
        
        # Check if valid (simple difficulty check)
        if block_hash.startswith('0'):  # Very easy for testnet
            # Prepare complete block data
            block_data = {
                'hash': block_hash,
                'previousblockhash': template.get('previousblockhash'),
                'height': template.get('height'),
                'nonce': nonce,
                'miner': MINER_ADDRESS,
                'timestamp': int(time.time())
            }
            
            # Submit block
            result = rpc_call("submitblock", [block_data])
            
            if result:
                elapsed = time.time() - start_time
                print(f"\n{'='*70}")
                print(f"🎉 BLOCK MINED!")
                print(f"{'='*70}")
                
                if isinstance(result, dict):
                    print(f"   Height: {result.get('height', 'unknown')}")
                    print(f"   Hash: {str(result.get('hash', 'unknown'))[:32]}...")
                    print(f"   Reward: {result.get('reward', 12.5)} GXC")
                else:
                    print(f"   Result: {result}")
                    print(f"   Reward: 12.5 GXC (testnet)")
                
                print(f"   Time: {elapsed:.2f} seconds")
                print(f"   Miner: {MINER_ADDRESS}")
                print(f"{'='*70}\n")
                return True
            else:
                print(f"❌ Block submission failed")
                return False
        
        nonce += 1
        
        # Progress indicator
        if nonce % 10000 == 0:
            print(f"   Tried {nonce} nonces...")

def main():
    print("\n" + "="*70)
    print("🪙 GXC TESTNET MINER")
    print("="*70)
    print(f"\nMining to: {MINER_ADDRESS}")
    print(f"RPC: {RPC_URL}")
    print(f"\nPress Ctrl+C to stop\n")
    print("="*70 + "\n")
    
    # Check connection
    print("Checking blockchain connection...")
    height = rpc_call("getblockcount")
    
    if height is not None:
        print(f"✅ Connected! Current height: {height}")
    else:
        print("❌ Cannot connect to blockchain node")
        print(f"\nMake sure testnet node is running:")
        print(f"./packages/gxc-miners-cli-linux/gxc-node --testnet --rpc-port=18332")
        return
    
    # Start mining
    blocks_mined = 0
    
    try:
        while True:
            if mine_block():
                blocks_mined += 1
                print(f"📊 Total blocks mined: {blocks_mined}")
                time.sleep(2)  # Wait a bit between blocks
            else:
                print("⏸️  Waiting 5 seconds before retry...")
                time.sleep(5)
    
    except KeyboardInterrupt:
        print("\n\n" + "="*70)
        print("⏹️  MINING STOPPED")
        print("="*70)
        print(f"\n📊 Statistics:")
        print(f"   Blocks mined: {blocks_mined}")
        print(f"   Total rewards: {blocks_mined * 12.5} GXC")
        print(f"   Miner address: {MINER_ADDRESS}")
        print("\n" + "="*70 + "\n")

if __name__ == "__main__":
    main()
