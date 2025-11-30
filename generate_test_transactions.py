#!/usr/bin/env python3
"""
Generate test transactions for the GXC testnet.
This script creates transactions between test addresses to populate the pending transaction pool.
"""

import requests
import json
import time
import random
import sys

# Testnet node URL - adjust if needed
TESTNET_NODE_URL = "http://localhost:8332"  # Default RPC port
RAILWAY_NODE_URL = None  # Will be set from environment or config

def get_node_url():
    """Get the testnet node URL"""
    import os
    railway_url = os.environ.get('RAILWAY_NODE_URL') or os.environ.get('TESTNET_NODE_URL')
    if railway_url:
        return railway_url.rstrip('/')
    return TESTNET_NODE_URL

def rpc_call(method, params=None, timeout=10):
    """Make an RPC call to the node"""
    url = get_node_url()
    payload = {
        "jsonrpc": "2.0",
        "method": method,
        "params": params or [],
        "id": 1
    }
    
    try:
        response = requests.post(url, json=payload, timeout=timeout)
        response.raise_for_status()
        result = response.json()
        if 'error' in result:
            print(f"RPC Error: {result['error']}")
            return None
        return result.get('result')
    except Exception as e:
        print(f"Error calling {method}: {e}")
        return None

def get_new_address():
    """Get a new address from the node"""
    return rpc_call("getnewaddress")

def get_balance(address):
    """Get balance for an address"""
    return rpc_call("getbalance", [address])

def send_to_address(from_addr, to_addr, amount):
    """Send transaction using sendtoaddress (if from_addr is in wallet)"""
    # Try sendtoaddress first (requires address to be in wallet)
    result = rpc_call("sendtoaddress", [to_addr, amount])
    return result

def create_test_transaction(from_addr, to_addr, amount):
    """Create a test transaction"""
    # Try multiple methods to send transaction
    methods = [
        ("sendtoaddress", [to_addr, amount]),
        ("sendrawtransaction", [{"from": from_addr, "to": to_addr, "amount": amount}]),
        ("createrawtransaction", [{"from": from_addr, "to": to_addr, "amount": amount}]),
    ]
    
    for method, params in methods:
        result = rpc_call(method, params)
        if result:
            return result
    
    return None

def generate_test_addresses(count=5):
    """Generate test addresses"""
    addresses = []
    for i in range(count):
        addr = get_new_address()
        if addr:
            addresses.append(addr)
            print(f"Generated address {i+1}: {addr}")
        else:
            # Fallback: generate testnet addresses
            addr = f"tGXC{''.join(random.choices('0123456789abcdef', k=40))}"
            addresses.append(addr)
            print(f"Generated test address {i+1}: {addr}")
    return addresses

def main():
    """Main function to generate test transactions"""
    print("=" * 60)
    print("GXC Testnet Transaction Generator")
    print("=" * 60)
    
    # Check node connectivity
    print("\n[1] Checking node connectivity...")
    blockchain_info = rpc_call("getblockchaininfo")
    if not blockchain_info:
        print("ERROR: Cannot connect to node. Please ensure the node is running.")
        print(f"Trying URL: {get_node_url()}")
        sys.exit(1)
    
    print(f"✓ Connected to node")
    print(f"  Current height: {blockchain_info.get('blocks', 0)}")
    print(f"  Difficulty: {blockchain_info.get('difficulty', 0)}")
    
    # Generate or use existing addresses
    print("\n[2] Setting up test addresses...")
    addresses = generate_test_addresses(5)
    
    if len(addresses) < 2:
        print("ERROR: Need at least 2 addresses to create transactions")
        sys.exit(1)
    
    # Get number of transactions to create
    num_transactions = 10
    if len(sys.argv) > 1:
        try:
            num_transactions = int(sys.argv[1])
        except:
            pass
    
    print(f"\n[3] Creating {num_transactions} test transactions...")
    
    successful = 0
    failed = 0
    
    for i in range(num_transactions):
        # Random sender and receiver
        from_addr = random.choice(addresses)
        to_addr = random.choice([a for a in addresses if a != from_addr])
        amount = round(random.uniform(0.1, 10.0), 8)  # Random amount between 0.1 and 10 GXC
        
        print(f"\n  Transaction {i+1}/{num_transactions}:")
        print(f"    From: {from_addr[:20]}...")
        print(f"    To:   {to_addr[:20]}...")
        print(f"    Amount: {amount} GXC")
        
        # Try to create transaction
        result = create_test_transaction(from_addr, to_addr, amount)
        
        if result:
            print(f"    ✓ Success! TX ID: {result[:20]}...")
            successful += 1
        else:
            print(f"    ✗ Failed to create transaction")
            failed += 1
        
        # Small delay between transactions
        time.sleep(0.5)
    
    print("\n" + "=" * 60)
    print(f"Transaction Generation Complete")
    print(f"  Successful: {successful}")
    print(f"  Failed: {failed}")
    print("=" * 60)
    
    # Check pending transactions
    print("\n[4] Checking pending transactions...")
    pending = rpc_call("getpendingtransactions") or rpc_call("listpendingtransactions")
    if pending:
        print(f"  Pending transactions: {len(pending) if isinstance(pending, list) else 'unknown'}")
    else:
        print("  No pending transactions found (or method not available)")
    
    print("\n✓ Done! Transactions should appear in the next mined blocks.")
    print("  Miners will include these transactions when creating new blocks.")

if __name__ == "__main__":
    main()
