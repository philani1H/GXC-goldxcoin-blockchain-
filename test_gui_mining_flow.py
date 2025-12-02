#!/usr/bin/env python3
"""Test complete mining flow as GUI would do it"""

import requests
import json
import time

RPC_URL = "http://localhost:8080/rpc"
ADDRESS = "tGXC9fab7317231b966af85ac453e168c0932"

def rpc_call(method, params=None):
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
        response.raise_for_status()
        result = response.json()
        
        if 'error' in result:
            print(f"❌ {method}: {result['error']}")
            return None
        
        return result.get('result')
    except Exception as e:
        print(f"❌ {method}: {str(e)}")
        return None

print("=" * 60)
print("GUI Mining Flow Test")
print("=" * 60)

# Step 1: Check initial balance
print("\n📊 Step 1: Check initial balance")
balance = rpc_call("getbalance", [ADDRESS])
print(f"   Balance: {balance} GXC")

# Step 2: Get block template
print("\n📦 Step 2: Get block template")
template = rpc_call("getblocktemplate", [{"algorithm": "sha256"}])
if template:
    height = template.get('height')
    reward = template.get('reward')
    prev_hash = template.get('previousblockhash')
    print(f"   Height: {height}")
    print(f"   Reward: {reward} GXC")
    print(f"   Previous: {prev_hash[:16]}...")
else:
    print("   ❌ Failed to get template")
    exit(1)

# Step 3: Submit mined block
print("\n⛏️  Step 3: Submit mined block")
block_data = {
    "height": height,
    "hash": "000000000000abcdef1234567890abcdef1234567890abcdef1234567890ab",
    "previousblockhash": prev_hash,
    "nonce": 123456,
    "miner": ADDRESS,
    "timestamp": int(time.time()),
    "difficulty": 1000
}

result = rpc_call("gxc_submitBlock", [block_data])
if result is None:  # null result means success
    print("   ✅ Block submitted successfully")
else:
    print(f"   Result: {result}")

time.sleep(2)

# Step 4: Check balance after mining
print("\n💰 Step 4: Check balance after mining")
balance_after = rpc_call("getbalance", [ADDRESS])
print(f"   Balance: {balance_after} GXC")

# Step 5: Try all balance methods
print("\n🔍 Step 5: Try all balance query methods")
methods = [
    ("getbalance", [ADDRESS]),
    ("getaddressbalance", [ADDRESS]),
    ("gxc_getBalance", [ADDRESS]),
]

for method, params in methods:
    result = rpc_call(method, params)
    print(f"   {method}: {result} GXC")

# Step 6: Check UTXOs
print("\n📋 Step 6: Check UTXOs")
utxos = rpc_call("listunspent", [0, 9999999, [ADDRESS]])
if utxos:
    print(f"   UTXO count: {len(utxos)}")
    total = sum(u.get('amount', u.get('value', 0)) for u in utxos)
    print(f"   Total from UTXOs: {total} GXC")
    if len(utxos) > 0:
        print(f"   First UTXO: {utxos[0].get('amount')} GXC")
else:
    print("   No UTXOs found")

# Step 7: Check transactions
print("\n📜 Step 7: Check transactions")
txs = rpc_call("listtransactions", [ADDRESS, 10])
if txs:
    print(f"   Transaction count: {len(txs)}")
    total_earned = sum(tx.get('amount', 0) for tx in txs if tx.get('amount', 0) > 0)
    print(f"   Total earned: {total_earned} GXC")
else:
    print("   No transactions found")

# Summary
print("\n" + "=" * 60)
print("Summary")
print("=" * 60)
print(f"Initial balance: {balance} GXC")
print(f"Final balance: {balance_after} GXC")
print(f"Expected reward: {reward} GXC")

if balance_after == reward:
    print("\n🎉 SUCCESS! Balance updated correctly!")
else:
    print(f"\n❌ FAIL! Balance should be {reward}, got {balance_after}")
