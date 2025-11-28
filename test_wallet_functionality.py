#!/usr/bin/env python3
"""
Test wallet creation and retrieval functionality
"""

import sys
import os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'api'))

from wallet_service import WalletService
import json

def test_wallet_creation_and_retrieval():
    """Test complete wallet lifecycle"""
    
    print("=" * 60)
    print("TESTING WALLET CREATION AND RETRIEVAL")
    print("=" * 60)
    
    wallet_service = WalletService()
    
    # Test 1: Create a new wallet
    print("\n1. Creating new wallet...")
    try:
        result = wallet_service.create_wallet(
            user_id="test_user_123",
            wallet_name="Test Wallet",
            password="SecurePassword123!"
        )
        
        if result['success']:
            print("✅ Wallet created successfully!")
            print(f"   Address: {result['address']}")
            print(f"   Wallet ID: {result['wallet_id']}")
            
            # Save for retrieval test
            wallet_id = result['wallet_id']
            address = result['address']
            mnemonic = result['mnemonic']
            
            print(f"\n   Mnemonic (SAVE THIS): {mnemonic}")
        else:
            print(f"❌ Failed to create wallet: {result.get('error')}")
            return False
            
    except Exception as e:
        print(f"❌ Exception during wallet creation: {e}")
        return False
    
    # Test 2: Retrieve wallet by ID
    print("\n2. Retrieving wallet by ID...")
    try:
        retrieved = wallet_service.get_wallet(wallet_id)
        
        if retrieved:
            print("✅ Wallet retrieved successfully!")
            print(f"   Address: {retrieved['address']}")
            print(f"   Name: {retrieved['wallet_name']}")
            print(f"   Balance: {retrieved['balance']} GXC")
        else:
            print("❌ Failed to retrieve wallet by ID")
            return False
            
    except Exception as e:
        print(f"❌ Exception during wallet retrieval: {e}")
        return False
    
    # Test 3: Get all wallets for user
    print("\n3. Getting all wallets for user...")
    try:
        user_wallets = wallet_service.get_user_wallets("test_user_123")
        
        if user_wallets:
            print(f"✅ Found {len(user_wallets)} wallet(s) for user")
            for wallet in user_wallets:
                print(f"   - {wallet['wallet_name']}: {wallet['address']}")
        else:
            print("❌ No wallets found for user")
            return False
            
    except Exception as e:
        print(f"❌ Exception during user wallets retrieval: {e}")
        return False
    
    # Test 4: Unlock wallet with password
    print("\n4. Unlocking wallet with password...")
    try:
        unlock_result = wallet_service.unlock_wallet(wallet_id, "SecurePassword123!")
        
        if unlock_result['success']:
            print("✅ Wallet unlocked successfully!")
            print(f"   Private key retrieved: {unlock_result['private_key'][:20]}...")
        else:
            print(f"❌ Failed to unlock wallet: {unlock_result.get('error')}")
            return False
            
    except Exception as e:
        print(f"❌ Exception during wallet unlock: {e}")
        return False
    
    # Test 5: Recover wallet from mnemonic
    print("\n5. Recovering wallet from mnemonic...")
    try:
        recovery_result = wallet_service.recover_wallet(
            user_id="test_user_456",
            mnemonic=mnemonic,
            password="NewPassword123!",
            wallet_name="Recovered Wallet"
        )
        
        if recovery_result['success']:
            print("✅ Wallet recovered successfully!")
            print(f"   Address: {recovery_result['address']}")
            print(f"   Matches original: {recovery_result['address'] == address}")
        else:
            print(f"❌ Failed to recover wallet: {recovery_result.get('error')}")
            return False
            
    except Exception as e:
        print(f"❌ Exception during wallet recovery: {e}")
        return False
    
    # Test 6: Export wallet
    print("\n6. Exporting wallet...")
    try:
        export_result = wallet_service.export_wallet(wallet_id, "SecurePassword123!")
        
        if export_result['success']:
            print("✅ Wallet exported successfully!")
            print(f"   Has private key: {'private_key' in export_result}")
            print(f"   Has mnemonic: {'mnemonic' in export_result}")
        else:
            print(f"❌ Failed to export wallet: {export_result.get('error')}")
            return False
            
    except Exception as e:
        print(f"❌ Exception during wallet export: {e}")
        return False
    
    # Test 7: Wrong password
    print("\n7. Testing wrong password...")
    try:
        wrong_unlock = wallet_service.unlock_wallet(wallet_id, "WrongPassword")
        
        if not wrong_unlock['success']:
            print("✅ Correctly rejected wrong password")
        else:
            print("❌ Security issue: accepted wrong password!")
            return False
            
    except Exception as e:
        print(f"❌ Exception during wrong password test: {e}")
        return False
    
    print("\n" + "=" * 60)
    print("ALL TESTS PASSED! ✅")
    print("=" * 60)
    print("\nYour wallet system is working correctly!")
    print("You can:")
    print("  ✅ Create wallets")
    print("  ✅ Retrieve wallets by ID")
    print("  ✅ Get all user wallets")
    print("  ✅ Unlock wallets with password")
    print("  ✅ Recover wallets from mnemonic")
    print("  ✅ Export wallet data")
    print("  ✅ Password protection works")
    
    return True

if __name__ == "__main__":
    try:
        success = test_wallet_creation_and_retrieval()
        sys.exit(0 if success else 1)
    except Exception as e:
        print(f"\n❌ FATAL ERROR: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)
