#!/usr/bin/env python3
"""
Mine blocks to pool address on Railway node
Uses the existing mine_testnet.py logic
"""
import os
import sys

# Set environment variables for the mining script
os.environ['RAILWAY_NODE_URL'] = 'https://gxc-chain112-blockchain-node-production.up.railway.app'
os.environ['BLOCKCHAIN_RPC_URL'] = 'https://gxc-chain112-blockchain-node-production.up.railway.app'

# Import the existing mining script
import requests
import hashlib
import time
import json

MINER_ADDRESS = "tGXC4VoV6DfSnM7YwTLCciBUzizARg2H64T7n"
RPC_URL = 'https://gxc-chain112-blockchain-node-production.up.railway.app'
NUM_BLOCKS = 5

def rpc_call(method, params=None, retries=3):
    """Make RPC call to blockchain node with retry logic"""
    if params is None:
        params = []
    
    payload = {
        "jsonrpc": "2.0",
        "method": method,
        "params": params,
        "id": 1
    }
    
    for attempt in range(retries):
        try:
            response = requests.post(RPC_URL, json=payload, timeout=30)
            result = response.json()
            
            # Check for RPC errors
            if 'error' in result and result['error']:
                print(f"RPC Error: {result['error']}")
                return None
            
            return result.get('result')
        except requests.Timeout:
            if attempt < retries - 1:
                print(f"⏱️  Timeout, retrying... ({attempt + 1}/{retries})")
                time.sleep(2 ** attempt)
                continue
            print(f"❌ RPC Timeout after {retries} attempts")
            return None
        except Exception as e:
            if attempt < retries - 1:
                print(f"⚠️  Error: {e}, retrying... ({attempt + 1}/{retries})")
                time.sleep(1)
                continue
            print(f"❌ RPC Error: {e}")
            return None
    
    return None

def mine_block(block_template):
    """Simple CPU mining"""
    nonce = 0
    difficulty = block_template.get('difficulty', 0.1)
    target_zeros = int(difficulty) if difficulty >= 1 else 0
    
    print(f"  Mining with difficulty {difficulty} (target: {target_zeros} leading zeros)...")
    
    while True:
        block_template['nonce'] = nonce
        block_str = json.dumps(block_template, sort_keys=True)
        block_hash = hashlib.sha256(block_str.encode()).hexdigest()
        
        # Check if hash meets difficulty
        if target_zeros == 0 or block_hash.startswith('0' * target_zeros):
            return nonce, block_hash
        
        nonce += 1
        if nonce % 100000 == 0:
            print(f"    Tried {nonce:,} nonces...")

def main():
    print("="*70)
    print("GXC Testnet Miner - Mining to Pool Address")
    print("="*70)
    print(f"Pool Address: {MINER_ADDRESS}")
    print(f"Node URL: {RPC_URL}")
    print(f"Blocks to mine: {NUM_BLOCKS}")
    print("="*70 + "\n")
    
    blocks_mined = 0
    
    for i in range(NUM_BLOCKS):
        print(f"[Block {i+1}/{NUM_BLOCKS}]")
        
        # Get block template
        print("  Requesting block template...")
        template = rpc_call("getblocktemplate", [{"minerAddress": MINER_ADDRESS}])
        
        if not template:
            print("  ❌ Failed to get block template")
            continue
        
        height = template.get('height', 'unknown')
        difficulty = template.get('difficulty', 'unknown')
        tx_count = len(template.get('transactions', []))
        
        print(f"  Height: {height}")
        print(f"  Difficulty: {difficulty}")
        print(f"  Transactions: {tx_count}")
        
        # Mine the block
        start_time = time.time()
        try:
            nonce, block_hash = mine_block(template)
            elapsed = time.time() - start_time
            hashrate = nonce / elapsed if elapsed > 0 else 0
            
            print(f"  ✅ Block mined!")
            print(f"     Nonce: {nonce:,}")
            print(f"     Hash: {block_hash}")
            print(f"     Time: {elapsed:.2f}s")
            print(f"     Hashrate: {hashrate:,.0f} H/s")
            
        except Exception as e:
            print(f"  ❌ Mining error: {e}")
            continue
        
        # Submit the block
        print("  Submitting block...")
        template['nonce'] = nonce
        template['hash'] = block_hash
        
        result = rpc_call("submitblock", [template])
        
        if result is not None:
            print(f"  ✅ Block accepted! Height: {height}")
            blocks_mined += 1
        else:
            print(f"  ❌ Block rejected")
        
        print()
        time.sleep(1)
    
    # Final summary
    print("\n" + "="*70)
    print("MINING SUMMARY")
    print("="*70)
    print(f"Blocks mined: {blocks_mined}/{NUM_BLOCKS}")
    print(f"Expected reward: {blocks_mined * 50} GXC")
    print("="*70 + "\n")
    
    # Check balance
    print("Checking pool balance...")
    balance = rpc_call("getbalance", [MINER_ADDRESS])
    
    if balance is not None:
        print(f"✅ Pool balance: {balance} GXC")
    else:
        print("❌ Could not retrieve balance")
    
    print("\n" + "="*70)
    print("To query UTXOs for spending:")
    print(f"  curl -X POST {RPC_URL} \\")
    print(f'    -H "Content-Type: application/json" \\')
    print(f'    -d \'{{"jsonrpc":"2.0","method":"listunspent","params":["{MINER_ADDRESS}"],"id":1}}\'')
    print("="*70)

if __name__ == "__main__":
    main()
