#!/usr/bin/env python3
"""
Quick script to check your wallet balance
Usage: python CHECK_BALANCE.py GXC...your_address...
"""

import sys
import requests
from typing import Optional

def get_balance(address: str, rest_url: str = "http://localhost:8080") -> Optional[float]:
    """Get balance for an address"""
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

def get_transactions(address: str, rest_url: str = "http://localhost:8080", limit: int = 10):
    """Get recent transactions for an address"""
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
        print("Usage: python CHECK_BALANCE.py <GXC_ADDRESS> [REST_URL]")
        print("Example: python CHECK_BALANCE.py GXC... http://localhost:8080")
        sys.exit(1)
    
    address = sys.argv[1]
    rest_url = sys.argv[2] if len(sys.argv) > 2 else "http://localhost:8080"
    
    print(f"\n{'='*60}")
    print(f"  GXC Wallet Balance Checker")
    print(f"{'='*60}\n")
    print(f"Address: {address}")
    print(f"REST URL: {rest_url}\n")
    
    # Get balance
    print("Checking balance...")
    balance = get_balance(address, rest_url)
    
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
    transactions = get_transactions(address, rest_url, limit=10)
    
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

