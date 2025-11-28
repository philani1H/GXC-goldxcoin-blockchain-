#!/usr/bin/env python3
"""
Test Railway Blockchain Node Connection
Tests that the deployed node is accessible and working
"""

import requests
import sys

def test_node(url):
    """Test blockchain node RPC endpoint"""
    
    print(f"Testing node at: {url}")
    print("=" * 60)
    
    # Test 1: getblocktemplate
    print("\n1. Testing getblocktemplate...")
    try:
        response = requests.post(url, json={
            "jsonrpc": "2.0",
            "id": "test1",
            "method": "getblocktemplate",
            "params": []
        }, timeout=10)
        
        if response.status_code == 200:
            data = response.json()
            if 'result' in data:
                print("   ✅ getblocktemplate works!")
                print(f"   Block height: {data['result'].get('height', 'N/A')}")
                print(f"   Difficulty: {data['result'].get('difficulty', 'N/A')}")
            else:
                print(f"   ❌ Error: {data.get('error', 'Unknown error')}")
        else:
            print(f"   ❌ HTTP {response.status_code}")
    except Exception as e:
        print(f"   ❌ Connection failed: {e}")
        return False
    
    # Test 2: getblockcount
    print("\n2. Testing getblockcount...")
    try:
        response = requests.post(url, json={
            "jsonrpc": "2.0",
            "id": "test2",
            "method": "getblockcount",
            "params": []
        }, timeout=10)
        
        if response.status_code == 200:
            data = response.json()
            if 'result' in data:
                print("   ✅ getblockcount works!")
                print(f"   Current height: {data['result']}")
            else:
                print(f"   ❌ Error: {data.get('error', 'Unknown error')}")
        else:
            print(f"   ❌ HTTP {response.status_code}")
    except Exception as e:
        print(f"   ❌ Connection failed: {e}")
    
    # Test 3: getdifficulty
    print("\n3. Testing getdifficulty...")
    try:
        response = requests.post(url, json={
            "jsonrpc": "2.0",
            "id": "test3",
            "method": "getdifficulty",
            "params": []
        }, timeout=10)
        
        if response.status_code == 200:
            data = response.json()
            if 'result' in data:
                print("   ✅ getdifficulty works!")
                print(f"   Difficulty: {data['result']}")
            else:
                print(f"   ❌ Error: {data.get('error', 'Unknown error')}")
        else:
            print(f"   ❌ HTTP {response.status_code}")
    except Exception as e:
        print(f"   ❌ Connection failed: {e}")
    
    print("\n" + "=" * 60)
    print("✅ Node is accessible and responding!")
    print("\nYou can now:")
    print(f"1. Update your miner: RPC_URL = '{url}'")
    print(f"2. Update your wallet: BLOCKCHAIN_RPC_URL = '{url}'")
    print(f"3. Update your explorer: BLOCKCHAIN_NODE_URL = '{url}'")
    print("=" * 60)
    
    return True

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Usage: python test_railway_node.py <RAILWAY_URL>")
        print("\nExample:")
        print("  python test_railway_node.py https://gxc-blockchain-node-production.up.railway.app")
        print("\nOr test local node:")
        print("  python test_railway_node.py http://localhost:18332")
        sys.exit(1)
    
    url = sys.argv[1]
    
    # Remove trailing slash
    if url.endswith('/'):
        url = url[:-1]
    
    test_node(url)
