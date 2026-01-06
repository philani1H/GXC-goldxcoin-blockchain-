#!/usr/bin/env python3
"""
Test transaction fee split to system pool
"""

import requests
import json
import time

NODE_URL = "https://gxc-chain112-blockchain-node-production.up.railway.app"

# Addresses
MINER_ADDRESS = "tGXC9fab7317231b966af85ac453e168c0932"
RECIPIENT_ADDRESS = "tGXCbedac0790fadbcaec5b8b52c96a6a121"
POOL_ADDRESS = "tGXC2a9d9ddb2e9ee658bca1c2ff41ffed99"

def get_balance(address):
    """Get balance for an address"""
    payload = {
        "jsonrpc": "2.0",
        "id": 1,
        "method": "getbalance",
        "params": [address]
    }
    response = requests.post(f"{NODE_URL}/api/getbalance", json=payload)
    return response.json().get("result", 0)

def get_info():
    """Get blockchain info"""
    payload = {
        "jsonrpc": "2.0",
        "id": 1,
        "method": "getinfo",
        "params": []
    }
    response = requests.post(f"{NODE_URL}/api/getinfo", json=payload)
    return response.json().get("result", {})

def send_transaction(from_addr, to_addr, amount, fee):
    """Send a transaction with a fee"""
    # Create raw transaction
    payload = {
        "jsonrpc": "2.0",
        "id": 1,
        "method": "createrawtransaction",
        "params": {
            "from": from_addr,
            "to": to_addr,
            "amount": amount,
            "fee": fee
        }
    }
    
    try:
        response = requests.post(f"{NODE_URL}/api/createrawtransaction", json=payload, timeout=10)
        result = response.json()
        
        if "error" in result:
            print(f"❌ Error creating transaction: {result['error']['message']}")
            return None
            
        tx_hex = result.get("result", {}).get("hex", "")
        if not tx_hex:
            print(f"❌ No transaction hex returned")
            return None
            
        # Send raw transaction
        send_payload = {
            "jsonrpc": "2.0",
            "id": 1,
            "method": "sendrawtransaction",
            "params": [tx_hex]
        }
        
        send_response = requests.post(f"{NODE_URL}/api/sendrawtransaction", json=send_payload, timeout=10)
        send_result = send_response.json()
        
        if "error" in send_result:
            print(f"❌ Error sending transaction: {send_result['error']['message']}")
            return None
            
        return send_result.get("result")
        
    except Exception as e:
        print(f"❌ Exception: {e}")
        return None

def main():
    print("=" * 70)
    print("GXC Transaction Fee Split Test")
    print("=" * 70)
    print()
    
    # Get initial state
    print("📊 Initial State:")
    miner_balance_before = get_balance(MINER_ADDRESS)
    pool_balance_before = get_balance(POOL_ADDRESS)
    recipient_balance_before = get_balance(RECIPIENT_ADDRESS)
    info = get_info()
    height_before = info.get("height", 0)
    
    print(f"  Blockchain Height: {height_before}")
    print(f"  Miner Balance: {miner_balance_before} GXC")
    print(f"  Pool Balance: {pool_balance_before} GXC")
    print(f"  Recipient Balance: {recipient_balance_before} GXC")
    print()
    
    # Send transaction with fee
    print("💸 Sending Transaction:")
    print(f"  From: {MINER_ADDRESS[:30]}...")
    print(f"  To: {RECIPIENT_ADDRESS[:30]}...")
    print(f"  Amount: 10.0 GXC")
    print(f"  Fee: 1.0 GXC")
    print()
    
    tx_hash = send_transaction(MINER_ADDRESS, RECIPIENT_ADDRESS, 10.0, 1.0)
    
    if not tx_hash:
        print("❌ Failed to send transaction")
        print()
        print("Note: The node may not have wallet functionality enabled.")
        print("Transaction fees can only be tested when transactions are")
        print("actually included in blocks.")
        return
    
    print(f"✅ Transaction sent: {tx_hash}")
    print()
    
    # Wait for transaction to be mined
    print("⛏️  Waiting for transaction to be mined...")
    for i in range(30):
        time.sleep(2)
        current_info = get_info()
        current_height = current_info.get("height", 0)
        
        if current_height > height_before:
            print(f"✅ Block mined! Height: {current_height}")
            break
        
        if i % 5 == 0:
            print(f"  Waiting... (height: {current_height})")
    
    print()
    
    # Get final state
    print("📊 Final State:")
    miner_balance_after = get_balance(MINER_ADDRESS)
    pool_balance_after = get_balance(POOL_ADDRESS)
    recipient_balance_after = get_balance(RECIPIENT_ADDRESS)
    
    print(f"  Miner Balance: {miner_balance_after} GXC")
    print(f"  Pool Balance: {pool_balance_after} GXC")
    print(f"  Recipient Balance: {recipient_balance_after} GXC")
    print()
    
    # Calculate changes
    print("📈 Changes:")
    miner_change = miner_balance_after - miner_balance_before
    pool_change = pool_balance_after - pool_balance_before
    recipient_change = recipient_balance_after - recipient_balance_before
    
    print(f"  Miner: {miner_change:+.2f} GXC")
    print(f"  Pool: {pool_change:+.2f} GXC")
    print(f"  Recipient: {recipient_change:+.2f} GXC")
    print()
    
    # Verify fee split
    expected_pool_fee = 1.0 * 0.15  # 15% of 1.0 GXC fee
    expected_miner_fee = 1.0 * 0.85  # 85% of 1.0 GXC fee
    
    print("✅ Expected Fee Split:")
    print(f"  Miner should get: 85% of 1.0 GXC fee = {expected_miner_fee} GXC")
    print(f"  Pool should get: 15% of 1.0 GXC fee = {expected_pool_fee} GXC")
    print()
    
    if abs(pool_change - expected_pool_fee) < 0.01:
        print("✅ Pool received correct fee split!")
    else:
        print(f"⚠️  Pool fee split mismatch (expected {expected_pool_fee}, got {pool_change})")

if __name__ == "__main__":
    main()
