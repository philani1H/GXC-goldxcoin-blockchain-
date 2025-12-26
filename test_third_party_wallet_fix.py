#!/usr/bin/env python3
"""
Test script to verify third-party wallet transaction fix.
This simulates a third-party wallet sending a transaction without prevTxHash set.
"""

import requests
import json
import time

# Configuration
NODE_URL = "http://localhost:8545"

def test_third_party_transaction():
    """Test sending a transaction from a third-party wallet without prevTxHash"""
    
    print("=" * 60)
    print("Testing Third-Party Wallet Transaction Fix")
    print("=" * 60)
    
    # Step 1: Get a wallet address with balance
    print("\n1. Getting wallet address...")
    try:
        response = requests.post(NODE_URL, json={
            "jsonrpc": "2.0",
            "method": "getnewaddress",
            "params": [],
            "id": 1
        })
        wallet_address = response.json()["result"]
        print(f"   Wallet address: {wallet_address}")
    except Exception as e:
        print(f"   Error: {e}")
        return False
    
    # Step 2: Check balance
    print("\n2. Checking balance...")
    try:
        response = requests.post(NODE_URL, json={
            "jsonrpc": "2.0",
            "method": "getbalance",
            "params": [wallet_address],
            "id": 2
        })
        balance = response.json()["result"]
        print(f"   Balance: {balance} GXC")
        
        if balance < 1.0:
            print("   Insufficient balance for test. Mining a block...")
            mine_response = requests.post(NODE_URL, json={
                "jsonrpc": "2.0",
                "method": "generatetoaddress",
                "params": [1, wallet_address],
                "id": 3
            })
            time.sleep(2)
            
            # Check balance again
            response = requests.post(NODE_URL, json={
                "jsonrpc": "2.0",
                "method": "getbalance",
                "params": [wallet_address],
                "id": 4
            })
            balance = response.json()["result"]
            print(f"   New balance: {balance} GXC")
    except Exception as e:
        print(f"   Error: {e}")
        return False
    
    # Step 3: Get UTXOs for the wallet
    print("\n3. Getting UTXOs...")
    try:
        response = requests.post(NODE_URL, json={
            "jsonrpc": "2.0",
            "method": "listunspent",
            "params": [wallet_address],
            "id": 5
        })
        utxos = response.json()["result"]
        print(f"   Found {len(utxos)} UTXOs")
        
        if not utxos:
            print("   No UTXOs available")
            return False
        
        # Use the first UTXO
        utxo = utxos[0]
        print(f"   Using UTXO: {utxo['txid']}:{utxo['vout']} ({utxo['amount']} GXC)")
    except Exception as e:
        print(f"   Error: {e}")
        return False
    
    # Step 4: Create a transaction WITHOUT prevTxHash (simulating third-party wallet)
    print("\n4. Creating transaction without prevTxHash (third-party wallet simulation)...")
    
    recipient_address = "tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf"  # From the error log
    send_amount = 0.5
    fee = 0.0001
    
    # Create transaction JSON without prevTxHash and referencedAmount
    transaction = {
        "inputs": [{
            "txHash": utxo["txid"],
            "outputIndex": utxo["vout"],
            "amount": utxo["amount"],
            "signature": "dummy_signature_will_be_replaced",
            "publicKey": "dummy_public_key_will_be_replaced"
        }],
        "outputs": [
            {
                "address": recipient_address,
                "amount": send_amount,
                "script": ""
            },
            {
                "address": wallet_address,
                "amount": utxo["amount"] - send_amount - fee,
                "script": ""
            }
        ],
        "fee": fee,
        "senderAddress": wallet_address,
        "receiverAddress": recipient_address,
        "type": "NORMAL"
        # NOTE: prevTxHash and referencedAmount are intentionally omitted
        # to simulate a third-party wallet that doesn't set these fields
    }
    
    print(f"   Transaction structure:")
    print(f"     - Inputs: {len(transaction['inputs'])}")
    print(f"     - Outputs: {len(transaction['outputs'])}")
    print(f"     - Fee: {transaction['fee']}")
    print(f"     - prevTxHash: NOT SET (will be auto-derived)")
    print(f"     - referencedAmount: NOT SET (will be auto-derived)")
    
    # Step 5: Send the transaction
    print("\n5. Sending transaction via sendrawtransaction...")
    try:
        response = requests.post(NODE_URL, json={
            "jsonrpc": "2.0",
            "method": "sendrawtransaction",
            "params": [transaction],
            "id": 6
        })
        
        result = response.json()
        
        if "error" in result:
            print(f"   ❌ Transaction failed: {result['error']}")
            return False
        else:
            tx_hash = result["result"]
            print(f"   ✅ Transaction successful!")
            print(f"   Transaction hash: {tx_hash}")
            return True
            
    except Exception as e:
        print(f"   Error: {e}")
        return False

def main():
    print("\nStarting third-party wallet transaction test...")
    print("This test verifies that transactions from third-party wallets")
    print("work correctly even when prevTxHash is not set.\n")
    
    success = test_third_party_transaction()
    
    print("\n" + "=" * 60)
    if success:
        print("✅ TEST PASSED: Third-party wallet transactions work correctly!")
    else:
        print("❌ TEST FAILED: Third-party wallet transactions still have issues")
    print("=" * 60)
    
    return 0 if success else 1

if __name__ == "__main__":
    exit(main())
