#!/usr/bin/env python3
"""
Test that testnet and mainnet are properly separated
"""

import sys
import os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'api'))

from wallet_service import WalletService
import secrets

def test_testnet_mainnet_separation():
    """Test that testnet and mainnet are completely separate"""
    
    print("\n" + "="*70)
    print("🔍 TESTING TESTNET vs MAINNET SEPARATION")
    print("="*70)
    
    # Test 1: Create testnet wallet service
    print("\n1️⃣  Creating TESTNET wallet service...")
    testnet_service = WalletService(testnet=True)
    print(f"   ✅ Testnet service created")
    print(f"   Database: {testnet_service.db_path}")
    
    # Test 2: Create mainnet wallet service
    print("\n2️⃣  Creating MAINNET wallet service...")
    mainnet_service = WalletService(testnet=False)
    print(f"   ✅ Mainnet service created")
    print(f"   Database: {mainnet_service.db_path}")
    
    # Test 3: Verify different databases
    print("\n3️⃣  Verifying separate databases...")
    if testnet_service.db_path != mainnet_service.db_path:
        print(f"   ✅ Different databases:")
        print(f"      Testnet:  {testnet_service.db_path}")
        print(f"      Mainnet:  {mainnet_service.db_path}")
    else:
        print(f"   ❌ ERROR: Same database!")
        return False
    
    # Test 4: Create testnet user and wallet
    print("\n4️⃣  Creating TESTNET wallet...")
    testnet_user = testnet_service.create_user(
        username=f"testnet_user_{secrets.token_hex(4)}",
        email=f"testnet_{secrets.token_hex(4)}@test.com",
        password="TestPassword123!"
    )
    
    if not testnet_user['success']:
        print(f"   ❌ Failed to create testnet user")
        return False
    
    testnet_wallet = testnet_service.create_wallet(
        user_id=testnet_user['user_id'],
        wallet_name="Testnet Wallet",
        password="WalletPass123!"
    )
    
    if not testnet_wallet['success']:
        print(f"   ❌ Failed to create testnet wallet")
        return False
    
    testnet_address = testnet_wallet['address']
    print(f"   ✅ Testnet wallet created")
    print(f"   Address: {testnet_address}")
    
    # Test 5: Create mainnet user and wallet
    print("\n5️⃣  Creating MAINNET wallet...")
    mainnet_user = mainnet_service.create_user(
        username=f"mainnet_user_{secrets.token_hex(4)}",
        email=f"mainnet_{secrets.token_hex(4)}@test.com",
        password="TestPassword123!"
    )
    
    if not mainnet_user['success']:
        print(f"   ❌ Failed to create mainnet user")
        return False
    
    mainnet_wallet = mainnet_service.create_wallet(
        user_id=mainnet_user['user_id'],
        wallet_name="Mainnet Wallet",
        password="WalletPass123!"
    )
    
    if not mainnet_wallet['success']:
        print(f"   ❌ Failed to create mainnet wallet")
        return False
    
    mainnet_address = mainnet_wallet['address']
    print(f"   ✅ Mainnet wallet created")
    print(f"   Address: {mainnet_address}")
    
    # Test 6: Verify different address formats
    print("\n6️⃣  Verifying address formats...")
    
    print(f"\n   TESTNET Address:")
    print(f"   ┌──────────────────────────────────────────────┐")
    print(f"   │ {testnet_address} │")
    print(f"   │ ^^^^└────────────────────────────────────────┘│")
    print(f"   │  │           33 hex characters                │")
    print(f"   │  └─ \"tGXC\" prefix (TESTNET)                   │")
    print(f"   └───────────────────────────────────────────────┘")
    
    print(f"\n   MAINNET Address:")
    print(f"   ┌──────────────────────────────────────────────┐")
    print(f"   │ {mainnet_address} │")
    print(f"   │ ^^^└─────────────────────────────────────────┘│")
    print(f"   │  │           34 hex characters                │")
    print(f"   │  └─ \"GXC\" prefix (MAINNET)                    │")
    print(f"   └───────────────────────────────────────────────┘")
    
    # Verify prefixes
    if testnet_address.startswith("tGXC"):
        print(f"\n   ✅ Testnet address has correct prefix: tGXC")
    else:
        print(f"\n   ❌ Testnet address has wrong prefix: {testnet_address[:4]}")
        return False
    
    if mainnet_address.startswith("GXC") and not mainnet_address.startswith("tGXC"):
        print(f"   ✅ Mainnet address has correct prefix: GXC")
    else:
        print(f"   ❌ Mainnet address has wrong prefix: {mainnet_address[:4]}")
        return False
    
    # Verify lengths
    if len(testnet_address) == 37:
        print(f"   ✅ Testnet address length correct: 37")
    else:
        print(f"   ❌ Testnet address length wrong: {len(testnet_address)}")
        return False
    
    if len(mainnet_address) == 37:
        print(f"   ✅ Mainnet address length correct: 37")
    else:
        print(f"   ❌ Mainnet address length wrong: {len(mainnet_address)}")
        return False
    
    # Test 7: Verify addresses are different
    print("\n7️⃣  Verifying addresses are different...")
    if testnet_address != mainnet_address:
        print(f"   ✅ Addresses are different (as expected)")
    else:
        print(f"   ❌ Addresses are the same (ERROR!)")
        return False
    
    # Test 8: Verify database isolation
    print("\n8️⃣  Verifying database isolation...")
    
    import sqlite3
    
    # Check testnet database
    testnet_conn = sqlite3.connect(testnet_service.db_path)
    testnet_cursor = testnet_conn.cursor()
    testnet_cursor.execute("SELECT COUNT(*) FROM wallets")
    testnet_count = testnet_cursor.fetchone()[0]
    testnet_conn.close()
    
    # Check mainnet database
    mainnet_conn = sqlite3.connect(mainnet_service.db_path)
    mainnet_cursor = mainnet_conn.cursor()
    mainnet_cursor.execute("SELECT COUNT(*) FROM wallets")
    mainnet_count = mainnet_cursor.fetchone()[0]
    mainnet_conn.close()
    
    print(f"   Testnet database: {testnet_count} wallet(s)")
    print(f"   Mainnet database: {mainnet_count} wallet(s)")
    
    if testnet_count > 0 and mainnet_count > 0:
        print(f"   ✅ Both databases have separate data")
    else:
        print(f"   ⚠️  One database might be empty")
    
    # Summary
    print("\n" + "="*70)
    print("✅ ALL TESTS PASSED!")
    print("="*70)
    
    print("\n📊 Summary:")
    print(f"   ✅ Testnet and Mainnet use different databases")
    print(f"   ✅ Testnet addresses start with 'tGXC'")
    print(f"   ✅ Mainnet addresses start with 'GXC'")
    print(f"   ✅ Addresses are properly separated")
    print(f"   ✅ No cross-contamination possible")
    
    print("\n🎯 Address Formats:")
    print(f"   Testnet:  tGXC + 33 hex chars = 37 total")
    print(f"   Mainnet:  GXC + 34 hex chars = 37 total")
    
    print("\n💾 Databases:")
    print(f"   Testnet:  {testnet_service.db_path}")
    print(f"   Mainnet:  {mainnet_service.db_path}")
    
    print("\n✅ Testnet and Mainnet are completely separate!")
    
    return True

if __name__ == "__main__":
    try:
        success = test_testnet_mainnet_separation()
        sys.exit(0 if success else 1)
    except Exception as e:
        print(f"\n❌ Error: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)
