#!/usr/bin/env python3
"""Test sending transaction with proper fee"""

import requests
import json

NODE_URL = "http://localhost:8332"

def rpc_call(method, params=None):
    if params is None:
        params = []
    
    payload = {
        "jsonrpc": "2.0",
        "method": method,
        "params": params,
        "id": 1
    }
    
    response = requests.post(NODE_URL, json=payload, timeout=10)
    result = response.json()
    return result

# Try different methods
print("Testing transaction methods...")
print()

# Method 1: sendfrom
print("1. Testing sendfrom...")
result = rpc_call("sendfrom", ["tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9", "tGXCe55238ed0886df89e9903834ff843a7acf4", 100.0])
print(json.dumps(result, indent=2))
print()

# Method 2: gxc_sendFrom  
print("2. Testing gxc_sendFrom...")
result = rpc_call("gxc_sendFrom", ["tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9", "tGXCe55238ed0886df89e9903834ff843a7acf4", 100.0])
print(json.dumps(result, indent=2))
print()

# Method 3: gxc_sendToAddress
print("3. Testing gxc_sendToAddress...")
result = rpc_call("gxc_sendToAddress", ["tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9", "tGXCe55238ed0886df89e9903834ff843a7acf4", 100.0])
print(json.dumps(result, indent=2))
print()
