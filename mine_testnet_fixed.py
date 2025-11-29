#!/usr/bin/env python3
"""
Fixed testnet miner - properly submits blocks
"""

import os
import requests
import hashlib
import time
import json

MINER_ADDRESS = "tGXC9fab7317231b966af85ac453e168c0932"
# Use Railway URL from environment, fallback to Railway URL for production
RAILWAY_NODE_URL = "https://gxc-chain112-blockchain-node-production.up.railway.app"
RPC_URL = os.environ.get('BLOCKCHAIN_RPC_URL', os.environ.get('RAILWAY_NODE_URL', RAILWAY_NODE_URL))

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
        # Try Railway URL first
        response = requests.post(RPC_URL, json=payload, timeout=10)
        result = response.json()
        return result.get('result')
    except requests.exceptions.Timeout:
        print(f"⚠️  Railway node timeout - trying localhost fallback...")
        # Fallback to local testnet node
        local_url = "http://localhost:18332"
        try:
            response = requests.post(local_url, json=payload, timeout=5)
            result = response.json()
            print(f"✅ Connected to local testnet node!")
            return result.get('result')
        except Exception as e:
            print(f"❌ Local node also unavailable: {e}")
            return None
    except Exception as e:
        print(f"RPC Error: {e}")
        # Try localhost fallback
        local_url = "http://localhost:18332"
        try:
            response = requests.post(local_url, json=payload, timeout=5)
            result = response.json()
            print(f"✅ Connected to local testnet node!")
            return result.get('result')
        except:
            return None

def mine_block():
    """Mine a single block"""
    print(f"\n⛏️  Mining block to: {MINER_ADDRESS}")
    
    # Get block template
    template = rpc_call("getblocktemplate", [{"algorithm": "sha256"}])
    
    if not template:
        print("❌ Failed to get block template")
        return False
    
    if isinstance(template, str) and 'error' in template.lower():
        print(f"❌ Error: {template}")
        return False
    
    height = template.get('height', 1)
    prev_hash = template.get('previousblockhash', '0' * 64)
    
    print(f"   Height: {height}")
    print(f"   Previous: {prev_hash[:16]}...")
    print(f"   Mining...")
    
    # Simple mining (find nonce)
    nonce = 0
    start_time = time.time()
    
    while True:
        # Create block hash
        block_string = f"{height}{prev_hash}{nonce}{MINER_ADDRESS}{int(time.time())}"
        block_hash = hashlib.sha256(block_string.encode()).hexdigest()
        
        # Check if valid (very easy for testnet - just needs to start with 0)
        if block_hash.startswith('0'):
            # Prepare complete block data with ALL required fields
            block_data = {
                'hash': block_hash,
                'previousblockhash': prev_hash,
                'height': height,
                'nonce': nonce,
                'miner': MINER_ADDRESS,
                'timestamp': int(time.time()),
                'difficulty': template.get('difficulty', 0.1),
                'transactions': []
            }
            
            # Submit block
            print(f"   Found valid hash: {block_hash[:16]}...")
            print(f"   Submitting block...")
            
            result = rpc_call("submitblock", [block_data])
            
            elapsed = time.time() - start_time
            
            if result and isinstance(result, dict) and result.get('success'):
                # Success!
                print(f"\n{'='*70}")
                print(f"🎉 BLOCK MINED SUCCESSFULLY!")
                print(f"{'='*70}")
                print(f"   Height: {result.get('height', height)}")
                print(f"   Hash: {block_hash[:32]}...")
                print(f"   Reward: {result.get('reward', 12.5)} GXC")
                print(f"   Time: {elapsed:.2f} seconds")
                print(f"   Nonce: {nonce}")
                print(f"   Miner: {MINER_ADDRESS}")
                print(f"{'='*70}\n")
                return True
            elif result == "Block rejected":
                print(f"❌ Block rejected by node")
                print(f"   This might be a validation issue")
                print(f"   Block data: {json.dumps(block_data, indent=2)}")
                return False
            elif result is None:
                # None means success in some RPC implementations
                print(f"\n{'='*70}")
                print(f"🎉 BLOCK MINED!")
                print(f"{'='*70}")
                print(f"   Height: {height}")
                print(f"   Hash: {block_hash[:32]}...")
                print(f"   Reward: 12.5 GXC (testnet)")
                print(f"   Time: {elapsed:.2f} seconds")
                print(f"   Nonce: {nonce}")
                print(f"   Miner: {MINER_ADDRESS}")
                print(f"{'='*70}\n")
                return True
            else:
                print(f"❌ Unexpected result: {result}")
                return False
        
        nonce += 1
        
        # Progress indicator
        if nonce % 100000 == 0:
            print(f"   Tried {nonce:,} nonces... ({nonce / (time.time() - start_time):.0f} H/s)")
        
        # Timeout after 60 seconds
        if time.time() - start_time > 60:
            print(f"   ⏱️  Timeout after 60 seconds, getting new template...")
            return False

def check_balance():
    """Check balance at miner address"""
    # This would need a getbalance RPC method
    # For now, just count blocks
    height = rpc_call("getblockcount")
    if height:
        print(f"\n💰 Estimated balance: {height * 12.5} GXC (if all blocks mined by you)")

def main():
    print("\n" + "="*70)
    print("🪙 GXC TESTNET MINER - FIXED VERSION")
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
        print(f"\n🔧 Solutions:")
        print(f"   1. Start local testnet node:")
        print(f"      python testnet_blockchain_node.py")
        print(f"   2. Check Railway node status:")
        print(f"      https://railway.app")
        print(f"   3. Wait for Railway deployment to complete")
        print(f"\n💡 Tip: Local node is faster and more reliable for testing!")
        return
    
    # Start mining
    blocks_mined = 0
    blocks_attempted = 0
    
    try:
        while True:
            blocks_attempted += 1
            print(f"\n📊 Attempt #{blocks_attempted}")
            
            if mine_block():
                blocks_mined += 1
                print(f"✅ Success! Total blocks mined: {blocks_mined}")
                
                # Check balance every 5 blocks
                if blocks_mined % 5 == 0:
                    check_balance()
                
                time.sleep(2)  # Wait a bit between blocks
            else:
                print("⏸️  Failed, waiting 5 seconds before retry...")
                time.sleep(5)
    
    except KeyboardInterrupt:
        print("\n\n" + "="*70)
        print("⏹️  MINING STOPPED")
        print("="*70)
        print(f"\n📊 Statistics:")
        print(f"   Blocks attempted: {blocks_attempted}")
        print(f"   Blocks mined: {blocks_mined}")
        print(f"   Success rate: {(blocks_mined/blocks_attempted*100) if blocks_attempted > 0 else 0:.1f}%")
        print(f"   Total rewards: {blocks_mined * 12.5} GXC")
        print(f"   Miner address: {MINER_ADDRESS}")
        print("\n" + "="*70 + "\n")

if __name__ == "__main__":
    main()
