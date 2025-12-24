#!/usr/bin/env python3
"""
Complete Mining Test - Mine blocks and verify balance updates
"""

import requests
import json
import hashlib
import time

NODE_URL = "http://localhost:8332"

def rpc_call(method, params=None):
    """Make RPC call"""
    payload = {
        "jsonrpc": "2.0",
        "method": method,
        "params": params or [],
        "id": 1
    }
    try:
        response = requests.post(NODE_URL, json=payload, timeout=10)
        result = response.json()
        if 'error' in result and result['error']:
            print(f"  ⚠️  RPC Error: {result['error']}")
            return None
        return result.get('result')
    except Exception as e:
        print(f"  ❌ Request failed: {e}")
        return None

def get_balance(address):
    """Get balance for address"""
    balance = rpc_call("getbalance", [address])
    return balance if balance is not None else 0.0

def get_staking_info(address):
    """Get detailed staking info"""
    return rpc_call("getstakinginfo", [address])

def mine_block(address):
    """Mine a single block"""
    # Get block template
    template = rpc_call("getblocktemplate", [])
    if not template:
        print("  ❌ Failed to get block template")
        return False
    
    height = template.get('height', 0)
    prev_hash = template.get('previousblockhash', '0' * 64)
    difficulty = template.get('difficulty', 0.1)
    
    print(f"  Mining block {height}...")
    print(f"    Previous: {prev_hash[:16]}...")
    print(f"    Difficulty: {difficulty}")
    
    # Simple mining (try different nonces)
    for nonce in range(100000):
        block_str = f"{height}{prev_hash}{nonce}{address}{int(time.time())}"
        block_hash = hashlib.sha256(block_str.encode()).hexdigest()
        
        # Check if valid (starts with 0 for testnet)
        if block_hash.startswith('0'):
            print(f"    ✓ Found valid hash: {block_hash[:16]}...")
            
            # Submit block
            block_data = {
                'version': template.get('version', 1),
                'previousblockhash': prev_hash,
                'height': height,
                'timestamp': int(time.time()),
                'bits': template.get('bits', '1d00ffff'),
                'nonce': nonce,
                'miner': address,
                'transactions': template.get('transactions', [])
            }
            
            result = rpc_call("submitblock", [block_data])
            
            # Wait for block to be processed
            time.sleep(2)
            
            # Verify
            current_height = rpc_call("getblockcount")
            if current_height and current_height >= height:
                print(f"    ✅ Block {height} mined successfully!")
                return True
            else:
                print(f"    ⚠️  Block submitted but not confirmed")
                return False
        
        if nonce % 10000 == 0 and nonce > 0:
            print(f"    Tried {nonce} nonces...")
    
    print(f"    ❌ No valid hash found in 100000 attempts")
    return False

def main():
    print("="*70)
    print("COMPLETE MINING TEST")
    print("="*70)
    print()
    
    # Step 1: Get or create address
    print("STEP 1: Get Mining Address")
    print("-"*70)
    
    # Try to get existing address
    address = rpc_call("getnewaddress", ["mining_test"])
    
    if not address:
        print("  Using existing test address")
        address = "tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9"
    
    print(f"  Address: {address}")
    print()
    
    # Step 2: Check initial balance
    print("STEP 2: Check Initial Balance")
    print("-"*70)
    
    initial_balance = get_balance(address)
    print(f"  Balance: {initial_balance} GXC")
    
    staking_info = get_staking_info(address)
    if staking_info:
        print(f"  Total earned: {staking_info.get('total_earned', 0)} GXC")
        print(f"  Mining earned: {staking_info.get('total_earned_mining', 0)} GXC")
        print(f"  Staking earned: {staking_info.get('total_earned_staking', 0)} GXC")
    print()
    
    # Step 3: Mine a block
    print("STEP 3: Mine Block")
    print("-"*70)
    
    if mine_block(address):
        print()
        
        # Step 4: Check balance after mining
        print("STEP 4: Check Balance After Mining")
        print("-"*70)
        
        # Wait a bit for balance to update
        time.sleep(3)
        
        new_balance = get_balance(address)
        print(f"  Previous balance: {initial_balance} GXC")
        print(f"  Current balance: {new_balance} GXC")
        print(f"  Difference: {new_balance - initial_balance} GXC")
        
        if new_balance > initial_balance:
            print(f"  ✅ Balance increased!")
        else:
            print(f"  ⚠️  Balance did not increase (may need more confirmations)")
        
        # Check staking info
        staking_info = get_staking_info(address)
        if staking_info:
            print(f"\n  Detailed info:")
            print(f"    Total earned: {staking_info.get('total_earned', 0)} GXC")
            print(f"    Mining earned: {staking_info.get('total_earned_mining', 0)} GXC")
            print(f"    Spendable: {staking_info.get('spendable_balance', 0)} GXC")
        print()
        
        # Step 5: Mine another block
        print("STEP 5: Mine Another Block")
        print("-"*70)
        
        if mine_block(address):
            time.sleep(3)
            
            final_balance = get_balance(address)
            print(f"\n  Final balance: {final_balance} GXC")
            print(f"  Total earned this session: {final_balance - initial_balance} GXC")
            print()
    
    # Summary
    print("="*70)
    print("TEST SUMMARY")
    print("="*70)
    print()
    print(f"Address: {address}")
    print(f"Initial balance: {initial_balance} GXC")
    
    final_balance = get_balance(address)
    print(f"Final balance: {final_balance} GXC")
    print(f"Earned: {final_balance - initial_balance} GXC")
    print()
    
    if final_balance > initial_balance:
        print("✅ Mining and balance tracking working correctly!")
    else:
        print("⚠️  Balance did not increase - blocks may need more confirmations")
    
    print()
    print("="*70)

if __name__ == "__main__":
    main()
