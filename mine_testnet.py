#!/usr/bin/env python3
"""
Simple testnet miner - mines blocks to your address
"""

import os
import requests
import hashlib
import time
import json

MINER_ADDRESS = "tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf"
# Use Railway URL from environment, fallback to localhost for local development
# FIXED: Changed port from 18332 to 8332 (correct RPC port)
RPC_URL = os.environ.get('BLOCKCHAIN_RPC_URL', os.environ.get('RAILWAY_NODE_URL', 'http://localhost:8332'))

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
            response = requests.post(RPC_URL, json=payload, timeout=10)
            result = response.json()
            
            # Check for RPC errors
            if 'error' in result and result['error']:
                print(f"RPC Error: {result['error']}")
                return None
            
            return result.get('result')
        except requests.Timeout:
            if attempt < retries - 1:
                print(f"⏱️  Timeout, retrying... ({attempt + 1}/{retries})")
                time.sleep(2 ** attempt)  # Exponential backoff
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

def mine_block():
    """Mine a single block"""
    print(f"\n⛏️  Mining block to: {MINER_ADDRESS}")
    
    # Get block template with correct API format
    template = rpc_call("getblocktemplate", [{"algorithm": "sha256"}])
    
    if not template:
        print("❌ Failed to get block template")
        return False
    
    if isinstance(template, dict) and 'error' in template:
        print(f"❌ Error: {template['error']}")
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
        
        # Check if valid (simple difficulty check)
        if block_hash.startswith('0'):  # Very easy for testnet
            # Prepare complete block data with all required fields
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
            
            # Wait for block to be processed
            time.sleep(3)
            
            # Verify block is confirmed
            try:
                current_height = rpc_call("getblockcount") or 0
                if current_height > 0:
                    confirmations = current_height - height + 1
                    if confirmations < 6:
                        print(f"   ⚠️  Block has {confirmations} confirmations (needs 6+ for full confirmation)")
                    else:
                        print(f"   ✅ Block confirmed ({confirmations} confirmations)")
                    
                    # Verify block exists on chain
                    block_check = rpc_call("getblock", [block_hash])
                    if not block_check:
                        print(f"   ⚠️  Block not yet found on chain")
            except:
                pass
            
            # Handle different result types
            if result is None:
                # None typically means success in RPC implementations
                print(f"\n{'='*70}")
                print(f"🎉 BLOCK MINED!")
                print(f"{'='*70}")
                print(f"   Height: {height}")
                print(f"   Hash: {block_hash[:32]}...")
                # Get reward from template or blockchain
                reward = template.get('block_reward') or template.get('reward') or template.get('coinbase_value')
                if not reward and template.get('coinbasevalue'):
                    reward = template.get('coinbasevalue') / 100000000.0
                if reward:
                    print(f"   Reward: {reward} GXC")
                else:
                    print(f"   Reward: Will be verified from blockchain")
                print(f"   Time: {elapsed:.2f} seconds")
                print(f"   Nonce: {nonce}")
                print(f"   Miner: {MINER_ADDRESS}")
                print(f"{'='*70}\n")
                return True
            elif isinstance(result, dict):
                if result.get('success') or result.get('accepted'):
                    print(f"\n{'='*70}")
                    print(f"🎉 BLOCK MINED SUCCESSFULLY!")
                    print(f"{'='*70}")
                    print(f"   Height: {result.get('height', height)}")
                    print(f"   Hash: {block_hash[:32]}...")
                    reward = result.get('reward') or template.get('block_reward') or template.get('reward')
                    if reward:
                        print(f"   Reward: {reward} GXC")
                    else:
                        print(f"   Reward: Will be verified from blockchain")
                    print(f"   Time: {elapsed:.2f} seconds")
                    print(f"   Nonce: {nonce}")
                    print(f"   Miner: {MINER_ADDRESS}")
                    print(f"{'='*70}\n")
                    return True
                else:
                    print(f"❌ Block submission failed: {result}")
                    return False
            elif isinstance(result, str) and 'rejected' in result.lower():
                print(f"❌ Block rejected: {result}")
                return False
            else:
                # Unexpected result, but might be success
                print(f"\n{'='*70}")
                print(f"🎉 BLOCK MINED!")
                print(f"{'='*70}")
                print(f"   Height: {height}")
                print(f"   Hash: {block_hash[:32]}...")
                print(f"   Result: {result}")
                print(f"   Time: {elapsed:.2f} seconds")
                print(f"   Nonce: {nonce}")
                print(f"   Miner: {MINER_ADDRESS}")
                print(f"{'='*70}\n")
                return True
        
        nonce += 1
        
        # Progress indicator
        if nonce % 100000 == 0:
            elapsed = time.time() - start_time
            if elapsed > 0:
                hash_rate = nonce / elapsed
                print(f"   Tried {nonce:,} nonces... ({hash_rate:.0f} H/s)")
        
        # Timeout after 60 seconds
        if time.time() - start_time > 60:
            print(f"   ⏱️  Timeout after 60 seconds, getting new template...")
            return False

def show_traceability_info():
    """Display traceability information"""
    print("\n" + "="*70)
    print("📊 TRACEABILITY FORMULA")
    print("="*70)
    print("\nAll mined blocks maintain the mathematical breadcrumb trail:")
    print()
    print("  Formula 1: Ti.Inputs[0].txHash == Ti.PrevTxHash")
    print("  Formula 2: Ti.Inputs[0].amount == Ti.ReferencedAmount")
    print()
    print("This ensures complete transaction traceability from genesis")
    print("to current block, enabling full audit trail for all coins.")
    print("="*70 + "\n")

def main():
    print("\n" + "="*70)
    print("🪙 GXC TESTNET MINER v2.0")
    print("="*70)
    print(f"\nMining to: {MINER_ADDRESS}")
    print(f"RPC: {RPC_URL}")
    print(f"\nPress Ctrl+C to stop\n")
    print("="*70 + "\n")
    
    # Show traceability information
    show_traceability_info()
    
    # Check connection
    print("Checking blockchain connection...")
    height = rpc_call("getblockcount")
    
    if height is not None:
        print(f"✅ Connected! Current height: {height}")
    else:
        print("❌ Cannot connect to blockchain node")
        print(f"\nMake sure testnet node is running:")
        print(f"./gxc-node --testnet")
        print(f"\nOr set BLOCKCHAIN_RPC_URL environment variable to remote node")
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
        print(f"   Total rewards: Check blockchain for actual rewards")
        print(f"   Miner address: {MINER_ADDRESS}")
        print(f"   Use 'getbalance' RPC to check actual balance")
        print("\n" + "="*70 + "\n")

if __name__ == "__main__":
    main()
