#!/usr/bin/env python3
"""
Quick script to check your wallet balance
Usage: python CHECK_BALANCE.py GXC...your_address...
"""

import sys
import requests
from typing import Optional

def get_balance(address: str, rpc_url: str = "http://localhost:8080/rpc", rest_url: str = "http://localhost:8080") -> Optional[float]:
    """Get balance for an address (use RPC methods first)"""
    # Try RPC methods first (more reliable)
    rpc_methods = [
        ("gxc_getBalance", [address]),
        ("getbalance", [address]),
        ("getaddressbalance", [address]),
    ]
    
    for method, params in rpc_methods:
        try:
            payload = {
                "jsonrpc": "2.0",
                "method": method,
                "params": params,
                "id": 1
            }
            response = requests.post(rpc_url, json=payload, timeout=10)
            if response.status_code == 200:
                result = response.json()
                if 'error' not in result:
                    balance = result.get('result')
                    if balance is not None:
                        if isinstance(balance, (int, float)):
                            return float(balance)
                        elif isinstance(balance, dict):
                            return float(balance.get('balance', balance.get('amount', 0.0)))
        except:
            continue
    
    # Fallback to REST API
    try:
        url = f"{rest_url}/api/v1/address/{address}/balance"
        response = requests.get(url, timeout=10)
        
        if response.status_code == 200:
            data = response.json()
            return float(data.get('balance', 0.0))
        else:
            print(f"Error: HTTP {response.status_code}")
            return None
    except Exception as e:
        print(f"Error: {e}")
        return None

def get_transactions(address: str, rpc_url: str = "http://localhost:8080/rpc", rest_url: str = "http://localhost:8080", limit: int = 10):
    """Get recent transactions for an address (use RPC methods first)"""
    # Try RPC methods first (more reliable)
    rpc_methods = [
        ("gxc_getTransactionsByAddress", [address, limit]),
        ("getaddresstransactions", [address, limit]),
        ("listtransactions", [address, limit]),
    ]
    
    for method, params in rpc_methods:
        try:
            payload = {
                "jsonrpc": "2.0",
                "method": method,
                "params": params,
                "id": 1
            }
            response = requests.post(rpc_url, json=payload, timeout=10)
            if response.status_code == 200:
                result = response.json()
                if 'error' not in result:
                    txs = result.get('result')
                    if txs is not None:
                        if isinstance(txs, list):
                            return txs
                        elif isinstance(txs, dict):
                            return txs.get('transactions', txs.get('txs', []))
        except:
            continue
    
    # Fallback to REST API
    try:
        url = f"{rest_url}/api/v1/address/{address}/transactions"
        response = requests.get(url, params={'limit': limit}, timeout=10)
        
        if response.status_code == 200:
            return response.json()
        else:
            print(f"Error: HTTP {response.status_code}")
            return []
    except Exception as e:
        print(f"Error: {e}")
        return []

def main():
    if len(sys.argv) < 2:
        print("Usage: python CHECK_BALANCE.py <GXC_ADDRESS> [RPC_URL] [REST_URL]")
        print("Example: python CHECK_BALANCE.py GXC... http://localhost:8080/rpc http://localhost:8080")
        sys.exit(1)
    
    address = sys.argv[1]
    rpc_url = sys.argv[2] if len(sys.argv) > 2 else "http://localhost:8080/rpc"
    rest_url = sys.argv[3] if len(sys.argv) > 3 else "http://localhost:8080"
    
    print(f"\n{'='*60}")
    print(f"  GXC Wallet Balance Checker")
    print(f"{'='*60}\n")
    print(f"Address: {address}")
    print(f"RPC URL: {rpc_url}")
    print(f"REST URL: {rest_url}\n")
    
    # Get balance
    print("Checking balance...")
    balance = get_balance(address, rpc_url, rest_url)
    
    if balance is not None:
        print(f"\n✅ Balance: {balance:.8f} GXC")
    else:
        print("\n❌ Failed to get balance")
        print("   Make sure:")
        print("   1. Blockchain node is running")
        print("   2. REST API is enabled (port 8080)")
        print("   3. Address is correct")
        sys.exit(1)
    
    # Get recent transactions
    print("\nFetching recent transactions...")
    transactions = get_transactions(address, rpc_url, rest_url, limit=10)
    
    if transactions:
        print(f"\n📊 Recent Transactions ({len(transactions)}):")
        print("-" * 60)
        
        for tx in transactions[:5]:  # Show first 5
            tx_type = tx.get('type', 'unknown')
            amount = tx.get('amount', 0)
            direction = "⬇️ Received" if tx.get('to') == address else "⬆️ Sent"
            
            print(f"{direction} | {tx_type.upper()} | {amount:.8f} GXC")
            print(f"   Hash: {tx.get('hash', '')[:32]}...")
            print()
    else:
        print("\n📊 No transactions found")
        print("   This is normal if you haven't received any coins yet")
    
    print(f"{'='*60}\n")

if __name__ == "__main__":
    main()

