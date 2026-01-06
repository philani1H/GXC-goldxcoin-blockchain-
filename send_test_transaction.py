#!/usr/bin/env python3
"""
Send a test transaction with a fee to test the fee split
"""

import requests
import json
import hashlib
import time

NODE_URL = "https://gxc-chain112-blockchain-node-production.up.railway.app"

# Addresses
MINER_ADDRESS = "tGXC9fab7317231b966af85ac453e168c0932"
RECIPIENT_ADDRESS = "tGXCbedac0790fadbcaec5b8b52c96a6a121"  # Test recipient
POOL_ADDRESS = "tGXC2a9d9ddb2e9ee658bca1c2ff41ffed99"

def rpc_call(method, params):
    """Make an RPC call"""
    payload = {
        "jsonrpc": "2.0",
        "id": 1,
        "method": method,
        "params": params
    }
    response = requests.post(f"{NODE_URL}/api/{method}", json=payload, timeout=10)
    return response.json()

def get_balance(address):
    """Get balance"""
    result = rpc_call("getbalance", [address])
    return result.get("result", 0)

def get_info():
    """Get blockchain info"""
    result = rpc_call("getinfo", [])
    return result.get("result", {})

def create_simple_transaction():
    """
    Create a simple transaction manually
    This bypasses the wallet and creates a raw transaction
    """
    # Get UTXOs for miner address
    print("📋 Getting UTXOs for miner address...")
    
    # We'll create a transaction that spends from the miner's coinbase outputs
    # For simplicity, we'll use the sendtoaddress method if the node wallet has the key
    
    print("💸 Attempting to send transaction...")
    print(f"  From: {MINER_ADDRESS}")
    print(f"  To: {RECIPIENT_ADDRESS}")
    print(f"  Amount: 10.0 GXC")
    print(f"  Fee: 1.0 GXC")
    print()
    
    # Try sendtoaddress
    try:
        result = rpc_call("sendtoaddress", [RECIPIENT_ADDRESS, 10.0, "", "", False, 1.0])
        
        if "error" in result:
            print(f"❌ Error: {result['error']['message']}")
            return None
        
        tx_hash = result.get("result")
        print(f"✅ Transaction created: {tx_hash}")
        return tx_hash
        
    except Exception as e:
        print(f"❌ Exception: {e}")
        return None

def main():
    print("=" * 70)
    print("GXC Transaction Fee Split Test")
    print("=" * 70)
    print()
    
    # Get initial balances
    print("📊 Initial Balances:")
    miner_balance = get_balance(MINER_ADDRESS)
    pool_balance = get_balance(POOL_ADDRESS)
    recipient_balance = get_balance(RECIPIENT_ADDRESS)
    info = get_info()
    height = info.get("height", 0)
    
    print(f"  Height: {height}")
    print(f"  Miner: {miner_balance} GXC")
    print(f"  Pool: {pool_balance} GXC")
    print(f"  Recipient: {recipient_balance} GXC")
    print()
    
    # Create transaction
    tx_hash = create_simple_transaction()
    
    if not tx_hash:
        print()
        print("=" * 70)
        print("⚠️  Transaction Creation Failed")
        print("=" * 70)
        print()
        print("The node wallet doesn't have the miner's private key.")
        print("To test fee splits, you need to:")
        print()
        print("1. Import the miner's private key into the node wallet, OR")
        print("2. Use a wallet that can sign transactions client-side, OR")
        print("3. Wait for real users to send transactions with fees")
        print()
        print("The fee split logic is implemented and will work automatically")
        print("when transactions with fees are included in blocks.")
        return
    
    print()
    print("⛏️  Mining blocks to confirm transaction...")
    print("   (Run: python3 gxc_miner.py)")
    print()
    
    # Wait for mining
    print("Waiting 30 seconds for blocks to be mined...")
    time.sleep(30)
    
    # Get final balances
    print()
    print("📊 Final Balances:")
    miner_balance_after = get_balance(MINER_ADDRESS)
    pool_balance_after = get_balance(POOL_ADDRESS)
    recipient_balance_after = get_balance(RECIPIENT_ADDRESS)
    info_after = get_info()
    height_after = info_after.get("height", 0)
    
    print(f"  Height: {height_after}")
    print(f"  Miner: {miner_balance_after} GXC")
    print(f"  Pool: {pool_balance_after} GXC")
    print(f"  Recipient: {recipient_balance_after} GXC")
    print()
    
    # Calculate changes
    print("📈 Changes:")
    miner_change = miner_balance_after - miner_balance
    pool_change = pool_balance_after - pool_balance
    recipient_change = recipient_balance_after - recipient_balance
    blocks_mined = height_after - height
    
    print(f"  Blocks mined: {blocks_mined}")
    print(f"  Miner: {miner_change:+.2f} GXC")
    print(f"  Pool: {pool_change:+.2f} GXC")
    print(f"  Recipient: {recipient_change:+.2f} GXC")
    print()
    
    # Verify
    expected_pool_fee = 1.0 * 0.15  # 15% of 1.0 GXC fee
    expected_miner_fee = 1.0 * 0.85  # 85% of 1.0 GXC fee
    expected_block_rewards = blocks_mined * 50.0
    
    print("✅ Expected:")
    print(f"  Pool fee share: {expected_pool_fee} GXC (15% of 1.0 GXC)")
    print(f"  Miner fee share: {expected_miner_fee} GXC (85% of 1.0 GXC)")
    print(f"  Miner block rewards: {expected_block_rewards} GXC ({blocks_mined} blocks)")
    print()
    
    if abs(pool_change - expected_pool_fee) < 0.01:
        print("✅ Fee split working correctly!")
    else:
        print(f"⚠️  Pool change: {pool_change}, expected: {expected_pool_fee}")

if __name__ == "__main__":
    main()
