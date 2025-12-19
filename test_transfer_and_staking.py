#!/usr/bin/env python3
"""
Test script to verify fund transfer and staking fixes
"""

import requests
import json
import time
import sys

# Node RPC endpoint
NODE_URL = "http://localhost:8545"

def rpc_call(method, params=[]):
    """Make an RPC call to the node"""
    payload = {
        "jsonrpc": "2.0",
        "method": method,
        "params": params,
        "id": 1
    }
    
    try:
        response = requests.post(NODE_URL, json=payload, timeout=5)
        result = response.json()
        
        if "error" in result:
            print(f"❌ RPC Error: {result['error']}")
            return None
        
        return result.get("result")
    except requests.exceptions.ConnectionError:
        print(f"❌ Cannot connect to node at {NODE_URL}")
        print("   Please start the node first: ./build/gxc-node")
        return None
    except Exception as e:
        print(f"❌ Error: {e}")
        return None

def test_node_connection():
    """Test if node is running"""
    print("\n=== Testing Node Connection ===")
    result = rpc_call("getblockchaininfo")
    
    if result:
        print(f"✅ Node is running")
        print(f"   Height: {result.get('blocks', 0)}")
        print(f"   Network: {result.get('chain', 'unknown')}")
        return True
    else:
        print("❌ Node is not responding")
        return False

def test_wallet_creation():
    """Test wallet creation"""
    print("\n=== Testing Wallet Creation ===")
    
    # Create wallet 1
    result1 = rpc_call("getnewaddress")
    if not result1:
        print("❌ Failed to create wallet 1")
        return None, None
    
    wallet1 = result1
    print(f"✅ Wallet 1 created: {wallet1[:20]}...")
    
    # Create wallet 2
    result2 = rpc_call("getnewaddress")
    if not result2:
        print("❌ Failed to create wallet 2")
        return None, None
    
    wallet2 = result2
    print(f"✅ Wallet 2 created: {wallet2[:20]}...")
    
    return wallet1, wallet2

def test_get_balance(address):
    """Get balance for an address"""
    result = rpc_call("getbalance", [address])
    if result is not None:
        return float(result)
    return 0.0

def test_fund_transfer(from_addr, to_addr, amount):
    """Test fund transfer"""
    print(f"\n=== Testing Fund Transfer ===")
    print(f"From: {from_addr[:20]}...")
    print(f"To: {to_addr[:20]}...")
    print(f"Amount: {amount} GXC")
    
    # Check initial balances
    from_balance_before = test_get_balance(from_addr)
    to_balance_before = test_get_balance(to_addr)
    
    print(f"\nBalances before transfer:")
    print(f"  From: {from_balance_before} GXC")
    print(f"  To: {to_balance_before} GXC")
    
    if from_balance_before < amount + 0.001:  # amount + fee
        print(f"❌ Insufficient funds. Need {amount + 0.001} GXC, have {from_balance_before} GXC")
        return False
    
    # Send transaction
    print(f"\nSending {amount} GXC...")
    result = rpc_call("sendtoaddress", [to_addr, amount])
    
    if not result:
        print("❌ Transfer failed")
        return False
    
    tx_hash = result
    print(f"✅ Transaction created: {tx_hash[:16]}...")
    
    # Wait for confirmation
    print("Waiting for confirmation...")
    time.sleep(2)
    
    # Check balances after
    from_balance_after = test_get_balance(from_addr)
    to_balance_after = test_get_balance(to_addr)
    
    print(f"\nBalances after transfer:")
    print(f"  From: {from_balance_after} GXC")
    print(f"  To: {to_balance_after} GXC")
    
    # Verify transfer
    expected_to_balance = to_balance_before + amount
    if abs(to_balance_after - expected_to_balance) < 0.001:
        print(f"✅ Transfer successful! Recipient received {amount} GXC")
        return True
    else:
        print(f"❌ Transfer verification failed")
        print(f"   Expected: {expected_to_balance} GXC")
        print(f"   Got: {to_balance_after} GXC")
        return False

def test_validator_registration(address, stake_amount, staking_days):
    """Test validator registration"""
    print(f"\n=== Testing Validator Registration ===")
    print(f"Address: {address[:20]}...")
    print(f"Stake: {stake_amount} GXC")
    print(f"Days: {staking_days}")
    
    # Check balance
    balance = test_get_balance(address)
    print(f"\nCurrent balance: {balance} GXC")
    
    if balance < stake_amount + 0.001:
        print(f"❌ Insufficient funds for staking")
        return False
    
    # Register validator
    print(f"\nRegistering validator...")
    result = rpc_call("registerValidator", [address, stake_amount, staking_days])
    
    if not result:
        print("❌ Validator registration failed")
        return False
    
    print(f"✅ Validator registered")
    print(f"   Status: {result.get('status', 'unknown')}")
    print(f"   Stake: {result.get('stake_amount', 0)} GXC")
    print(f"   TX: {result.get('stake_tx', 'N/A')[:16]}...")
    
    # Check validator info
    print("\nChecking validator info...")
    time.sleep(1)
    
    validator_info = rpc_call("getValidatorInfo", [address])
    if validator_info:
        print(f"✅ Validator info retrieved")
        print(f"   Stake: {validator_info.get('stake_amount', 0)} GXC")
        print(f"   Status: {validator_info.get('status', 'unknown')}")
        print(f"   Active: {validator_info.get('is_active', False)}")
        print(f"   Pending: {validator_info.get('is_pending', False)}")
        
        # Verify stake amount is shown
        if validator_info.get('stake_amount', 0) == stake_amount:
            print(f"✅ Stake amount correctly shown: {stake_amount} GXC")
            return True
        else:
            print(f"❌ Stake amount mismatch")
            print(f"   Expected: {stake_amount} GXC")
            print(f"   Got: {validator_info.get('stake_amount', 0)} GXC")
            return False
    else:
        print("❌ Failed to retrieve validator info")
        return False

def main():
    """Main test function"""
    print("=" * 60)
    print("GXC Blockchain - Transfer and Staking Test")
    print("=" * 60)
    
    # Test 1: Node connection
    if not test_node_connection():
        print("\n❌ Cannot proceed without node connection")
        sys.exit(1)
    
    # Test 2: Wallet creation
    wallet1, wallet2 = test_wallet_creation()
    if not wallet1 or not wallet2:
        print("\n❌ Cannot proceed without wallets")
        sys.exit(1)
    
    # Note: In a real test, you would need to fund wallet1 first
    # For now, we'll just show the test structure
    
    print("\n" + "=" * 60)
    print("Test Structure Ready")
    print("=" * 60)
    print("\nTo run full tests:")
    print("1. Fund wallet1 with some GXC (mine blocks or use faucet)")
    print("2. Run: python3 test_transfer_and_staking.py")
    print("\nTest scenarios:")
    print("  ✓ Fund transfer between wallets")
    print("  ✓ Validator registration with stake")
    print("  ✓ Stake amount visibility")
    print("  ✓ Validator activation after confirmation")
    
    # If wallet1 has funds, run the tests
    balance1 = test_get_balance(wallet1)
    if balance1 > 100:
        print(f"\n✅ Wallet1 has {balance1} GXC - running tests...")
        
        # Test fund transfer
        success = test_fund_transfer(wallet1, wallet2, 10.0)
        if success:
            print("\n✅ Fund transfer test PASSED")
        else:
            print("\n❌ Fund transfer test FAILED")
        
        # Test validator registration
        if balance1 > 110:  # Need 100 for stake + 10 for previous transfer + fees
            success = test_validator_registration(wallet1, 100.0, 30)
            if success:
                print("\n✅ Validator registration test PASSED")
            else:
                print("\n❌ Validator registration test FAILED")
    else:
        print(f"\n⚠️  Wallet1 has insufficient funds ({balance1} GXC)")
        print("   Fund the wallet to run full tests")
    
    print("\n" + "=" * 60)
    print("Test Complete")
    print("=" * 60)

if __name__ == "__main__":
    main()
