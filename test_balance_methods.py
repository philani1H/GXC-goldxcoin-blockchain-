#!/usr/bin/env python3
"""
Test all balance RPC methods to find which one works
"""

import requests
import json

NODE_URL = "http://localhost:8332"
TEST_ADDRESS = "tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9"

def rpc_call(method, params=None):
    """Make RPC call"""
    payload = {
        "jsonrpc": "2.0",
        "method": method,
        "params": params or [],
        "id": 1
    }
    try:
        response = requests.post(NODE_URL, json=payload, timeout=5)
        return response.json()
    except Exception as e:
        return {"error": str(e)}

print("="*70)
print("TESTING BALANCE RPC METHODS")
print("="*70)
print(f"\nNode: {NODE_URL}")
print(f"Test Address: {TEST_ADDRESS}")
print()

# Test 1: getbalance
print("Test 1: getbalance")
print("-"*70)
result = rpc_call("getbalance", [TEST_ADDRESS])
print(f"Request: getbalance(['{TEST_ADDRESS}'])")
print(f"Response: {json.dumps(result, indent=2)}")
print()

# Test 2: gxc_getBalance
print("Test 2: gxc_getBalance")
print("-"*70)
result = rpc_call("gxc_getBalance", [TEST_ADDRESS])
print(f"Request: gxc_getBalance(['{TEST_ADDRESS}'])")
print(f"Response: {json.dumps(result, indent=2)}")
print()

# Test 3: getaccountbalance
print("Test 3: getaccountbalance")
print("-"*70)
result = rpc_call("getaccountbalance", [TEST_ADDRESS])
print(f"Request: getaccountbalance(['{TEST_ADDRESS}'])")
print(f"Response: {json.dumps(result, indent=2)}")
print()

# Test 4: getaddressbalance
print("Test 4: getaddressbalance")
print("-"*70)
result = rpc_call("getaddressbalance", [TEST_ADDRESS])
print(f"Request: getaddressbalance(['{TEST_ADDRESS}'])")
print(f"Response: {json.dumps(result, indent=2)}")
print()

# Test 5: listunspent
print("Test 5: listunspent")
print("-"*70)
result = rpc_call("listunspent", [0, 9999999, [TEST_ADDRESS]])
print(f"Request: listunspent([0, 9999999, ['{TEST_ADDRESS}']])")
print(f"Response: {json.dumps(result, indent=2)[:500]}...")
if 'result' in result and result['result']:
    total = sum(utxo.get('amount', 0) for utxo in result['result'])
    print(f"\nCalculated balance from UTXOs: {total} GXC")
print()

# Test 6: listtransactions
print("Test 6: listtransactions")
print("-"*70)
result = rpc_call("listtransactions", ["*", 10])
print(f"Request: listtransactions(['*', 10])")
if 'result' in result and result['result']:
    print(f"Found {len(result['result'])} transactions")
    for tx in result['result'][:3]:
        print(f"  - {tx.get('txid', 'N/A')[:16]}... | {tx.get('category', 'N/A')} | {tx.get('amount', 0)} GXC")
    
    # Calculate balance
    balance = 0
    for tx in result['result']:
        if tx.get('address') == TEST_ADDRESS:
            category = tx.get('category', '')
            amount = tx.get('amount', 0)
            if category in ['receive', 'generate', 'immature']:
                balance += amount
            elif category == 'send':
                balance -= amount
    
    print(f"\nCalculated balance from transactions: {balance} GXC")
else:
    print(f"Response: {json.dumps(result, indent=2)}")
print()

# Test 7: getstakinginfo
print("Test 7: getstakinginfo")
print("-"*70)
result = rpc_call("getstakinginfo", [TEST_ADDRESS])
print(f"Request: getstakinginfo(['{TEST_ADDRESS}'])")
print(f"Response: {json.dumps(result, indent=2)}")
print()

print("="*70)
print("SUMMARY")
print("="*70)
print("\nWorking methods will show 'result' with data")
print("Failed methods will show 'error'")
print()
