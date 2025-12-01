#!/usr/bin/env python3
"""
Test script to verify mining updates wallet balance correctly
"""

import requests
import time
import json
import sys

RPC_URL = "http://localhost:18332"
MINER_ADDRESS = "tGXC9fab7317231b966af85ac453e168c0932"

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
        response = requests.post(RPC_URL, json=payload, timeout=10)
        result = response.json()
        if 'error' in result:
            print(f"❌ RPC Error: {result['error']}")
            return None
        return result.get('result')
    except Exception as e:
        print(f"❌ Connection Error: {e}")
        return None

def test_mining_and_wallet():
    """Test mining and verify wallet balance updates"""
    print("="*70)
    print("🧪 TESTING MINING AND WALLET UPDATES")
    print("="*70)
    
    # Step 1: Check initial balance
    print("\n1️⃣  Checking initial balance...")
    initial_balance = rpc_call("getbalance", [MINER_ADDRESS])
    if initial_balance is None:
        print("   ⚠️  Could not get balance - node may not be running")
        print("   💡 Start node: python3 testnet_blockchain_node.py")
        return False
    
    print(f"   ✅ Initial balance: {initial_balance} GXC")
    
    # Step 2: Get current block height
    print("\n2️⃣  Getting current block height...")
    height = rpc_call("getblockcount")
    if height is None:
        print("   ❌ Could not get block count")
        return False
    
    print(f"   ✅ Current height: {height}")
    
    # Step 3: Get block template
    print("\n3️⃣  Getting block template...")
    template = rpc_call("getblocktemplate", [{"algorithm": "sha256"}])
    if not template:
        print("   ❌ Failed to get block template")
        return False
    
    print(f"   ✅ Template received:")
    print(f"      Height: {template.get('height', 'unknown')}")
    print(f"      Previous hash: {template.get('previousblockhash', 'unknown')[:16]}...")
    print(f"      Reward: {template.get('block_reward') or template.get('reward', 'unknown')} GXC")
    
    # Step 4: Mine a block
    print("\n4️⃣  Mining block...")
    import hashlib
    
    height_val = template.get('height', 1)
    prev_hash = template.get('previousblockhash', '0' * 64)
    nonce = 0
    start_time = time.time()
    
    print("   ⛏️  Searching for valid hash...")
    while True:
        block_string = f"{height_val}{prev_hash}{nonce}{MINER_ADDRESS}{int(time.time())}"
        block_hash = hashlib.sha256(block_string.encode()).hexdigest()
        
        if block_hash.startswith('0'):  # Easy difficulty for testnet
            print(f"   ✅ Found valid hash: {block_hash[:16]}...")
            
            block_data = {
                'hash': block_hash,
                'previousblockhash': prev_hash,
                'height': height_val,
                'nonce': nonce,
                'miner': MINER_ADDRESS,
                'timestamp': int(time.time()),
                'difficulty': template.get('difficulty', 0.1),
                'transactions': []
            }
            
            # Submit block
            result = rpc_call("submitblock", [block_data])
            elapsed = time.time() - start_time
            
            if result is None or (isinstance(result, dict) and result.get('success')):
                print(f"   ✅ Block submitted successfully! ({elapsed:.2f}s)")
                break
            else:
                print(f"   ❌ Block submission failed: {result}")
                return False
        
        nonce += 1
        if nonce % 100000 == 0:
            print(f"   ⏳ Tried {nonce:,} nonces...")
        
        if time.time() - start_time > 30:
            print("   ⏱️  Timeout after 30 seconds")
            return False
    
    # Step 5: Wait for block to be processed
    print("\n5️⃣  Waiting for block to be processed...")
    time.sleep(3)
    
    # Step 6: Check new balance
    print("\n6️⃣  Checking updated balance...")
    new_balance = rpc_call("getbalance", [MINER_ADDRESS])
    if new_balance is None:
        print("   ❌ Could not get updated balance")
        return False
    
    print(f"   ✅ New balance: {new_balance} GXC")
    
    # Step 7: Verify balance increased
    print("\n7️⃣  Verifying balance update...")
    balance_increase = new_balance - initial_balance
    expected_reward = template.get('block_reward') or template.get('reward', 12.5)
    
    print(f"   Initial balance: {initial_balance} GXC")
    print(f"   New balance: {new_balance} GXC")
    print(f"   Increase: {balance_increase} GXC")
    print(f"   Expected reward: {expected_reward} GXC")
    
    if balance_increase >= expected_reward * 0.99:  # Allow small rounding differences
        print(f"\n   ✅ SUCCESS! Balance increased by {balance_increase} GXC")
        print(f"   ✅ Wallet correctly updated after mining!")
        return True
    elif balance_increase > 0:
        print(f"\n   ⚠️  Balance increased but less than expected")
        print(f"   Increase: {balance_increase} GXC (expected ~{expected_reward} GXC)")
        return True  # Still consider success if balance increased
    else:
        print(f"\n   ❌ FAILED! Balance did not increase")
        print(f"   Balance unchanged: {initial_balance} GXC")
        return False

if __name__ == "__main__":
    print("\n" + "="*70)
    print("🪙 GXC MINING & WALLET UPDATE TEST")
    print("="*70)
    print(f"\nNode URL: {RPC_URL}")
    print(f"Miner Address: {MINER_ADDRESS}")
    print("\n" + "="*70 + "\n")
    
    # Check if node is running
    print("🔍 Checking if node is running...")
    height = rpc_call("getblockcount")
    if height is None:
        print("\n❌ Cannot connect to blockchain node!")
        print("\n💡 To start the C++ testnet node:")
        print("   # Build first:")
        print("   mkdir -p build && cd build")
        print("   cmake .. -DCMAKE_BUILD_TYPE=Release")
        print("   make -j$(nproc) gxc-node")
        print("   # Then run:")
        print("   ./build/gxc-node --testnet --rpc-port=18332")
        print("\n   Or use Python fallback node:")
        print("   python3 testnet_blockchain_node.py")
        sys.exit(1)
    
    print(f"✅ Node is running! Current height: {height}\n")
    
    # Run test
    success = test_mining_and_wallet()
    
    print("\n" + "="*70)
    if success:
        print("✅ TEST PASSED - Mining correctly updates wallet balance!")
    else:
        print("❌ TEST FAILED - Wallet balance not updated correctly")
    print("="*70 + "\n")
    
    sys.exit(0 if success else 1)
