#!/usr/bin/env python3
"""Test RPC calls that GUI miner uses"""

import requests
import json

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
print("Testing RPC Methods Used by GUI Miner")
print("=" * 60)

# Test 1: getbalance
print("\n1. Testing getbalance...")
balance = rpc_call("getbalance", [ADDRESS])
print(f"   Result: {balance}")

# Test 2: getaddressbalance
print("\n2. Testing getaddressbalance...")
balance2 = rpc_call("getaddressbalance", [ADDRESS])
print(f"   Result: {balance2}")

# Test 3: gxc_getBalance
print("\n3. Testing gxc_getBalance...")
balance3 = rpc_call("gxc_getBalance", [ADDRESS])
print(f"   Result: {balance3}")

# Test 4: listunspent
print("\n4. Testing listunspent...")
utxos = rpc_call("listunspent", [0, 9999999, [ADDRESS]])
print(f"   Result: {utxos}")
if utxos:
    print(f"   UTXO count: {len(utxos)}")
    if len(utxos) > 0:
        print(f"   First UTXO: {utxos[0]}")

# Test 5: listtransactions
print("\n5. Testing listtransactions...")
txs = rpc_call("listtransactions", [ADDRESS, 10])
print(f"   Result type: {type(txs)}")
if txs:
    print(f"   TX count: {len(txs)}")
    if len(txs) > 0:
        print(f"   First TX: {txs[0]}")

# Test 6: getblocktemplate
print("\n6. Testing getblocktemplate...")
template = rpc_call("getblocktemplate", [{"algorithm": "sha256"}])
if template:
    print(f"   Height: {template.get('height')}")
    print(f"   Reward: {template.get('reward')}")
    print(f"   Previous hash: {template.get('previousblockhash', '')[:16]}...")

# Test 7: gxc_getBlockTemplate
print("\n7. Testing gxc_getBlockTemplate...")
template2 = rpc_call("gxc_getBlockTemplate", [])
if template2:
    print(f"   Height: {template2.get('height')}")
    print(f"   Reward: {template2.get('reward')}")

print("\n" + "=" * 60)
print("Summary")
print("=" * 60)
print(f"getbalance: {'✅' if balance is not None else '❌'}")
print(f"getaddressbalance: {'✅' if balance2 is not None else '❌'}")
print(f"gxc_getBalance: {'✅' if balance3 is not None else '❌'}")
print(f"listunspent: {'✅' if utxos is not None else '❌'}")
print(f"listtransactions: {'✅' if txs is not None else '❌'}")
print(f"getblocktemplate: {'✅' if template is not None else '❌'}")
print(f"gxc_getBlockTemplate: {'✅' if template2 is not None else '❌'}")
