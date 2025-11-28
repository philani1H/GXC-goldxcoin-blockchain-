#!/usr/bin/env python3
"""
Test GXC address format generation
"""

import sys
import os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'api'))

from wallet_service import WalletService
import secrets

def test_gxc_address_format():
    """Test that wallet service creates GXC-format addresses"""
    
    print("\n" + "="*70)
    print("🔐 TESTING GXC ADDRESS FORMAT")
    print("="*70)
    
    wallet_service = WalletService()
    
    # Create a test user first
    print("\n1️⃣  Creating test user...")
    user_result = wallet_service.create_user(
        username=f"testuser_{secrets.token_hex(4)}",
        email=f"test_{secrets.token_hex(4)}@example.com",
        password="TestPassword123!"
    )
    
    if not user_result['success']:
        print(f"   ❌ Failed to create user: {user_result.get('error')}")
        return False
    
    user_id = user_result['user_id']
    print(f"   ✅ User created: {user_id}")
    
    # Create wallet
    print("\n2️⃣  Creating wallet with GXC address format...")
    result = wallet_service.create_wallet(
        user_id=user_id,
        wallet_name="Test GXC Wallet",
        password="WalletPassword123!"
    )
    
    if not result['success']:
        print(f"   ❌ Failed to create wallet: {result.get('error')}")
        return False
    
    address = result['address']
    wallet_id = result['wallet_id']
    
    print(f"   ✅ Wallet created successfully!")
    print(f"\n   📋 Wallet Details:")
    print(f"      Wallet ID: {wallet_id}")
    print(f"      Address: {address}")
    
    # Verify address format
    print("\n3️⃣  Verifying address format...")
    
    # Check if starts with GXC
    if not address.startswith("GXC"):
        print(f"   ❌ Address doesn't start with 'GXC': {address}")
        return False
    print(f"   ✅ Address starts with 'GXC'")
    
    # Check length (GXC + 34 hex chars = 37 total)
    if len(address) != 37:
        print(f"   ❌ Address length is {len(address)}, expected 37")
        print(f"      Address: {address}")
        return False
    print(f"   ✅ Address length is 37 characters")
    
    # Check if hex part is valid
    hex_part = address[3:]
    if len(hex_part) != 34:
        print(f"   ❌ Hex part length is {len(hex_part)}, expected 34")
        return False
    print(f"   ✅ Hex part is 34 characters")
    
    try:
        int(hex_part, 16)
        print(f"   ✅ Hex part is valid hexadecimal")
    except ValueError:
        print(f"   ❌ Hex part contains non-hex characters: {hex_part}")
        return False
    
    # Visual display
    print("\n4️⃣  Address format visualization:")
    print(f"\n   ┌──────────────────────────────────────────────┐")
    print(f"   │ {address} │")
    print(f"   │ ^^^└─────────────────────────────────────────┘│")
    print(f"   │  │           34 hex characters                │")
    print(f"   │  └─ \"GXC\" prefix                              │")
    print(f"   │                                               │")
    print(f"   │ Length: {len(address)} characters                         │")
    print(f"   │ Characters: GXC + [0-9a-f]                    │")
    print(f"   └───────────────────────────────────────────────┘")
    
    # Test retrieval
    print("\n5️⃣  Testing wallet retrieval...")
    retrieved = wallet_service.get_wallet(wallet_id)
    
    if not retrieved:
        print(f"   ❌ Failed to retrieve wallet")
        return False
    
    if retrieved['address'] != address:
        print(f"   ❌ Retrieved address doesn't match!")
        print(f"      Original:  {address}")
        print(f"      Retrieved: {retrieved['address']}")
        return False
    
    print(f"   ✅ Wallet retrieved successfully")
    print(f"   ✅ Address matches: {retrieved['address']}")
    
    # Create multiple wallets to test consistency
    print("\n6️⃣  Creating multiple wallets to test consistency...")
    addresses = [address]
    
    for i in range(3):
        result = wallet_service.create_wallet(
            user_id=user_id,
            wallet_name=f"Test Wallet {i+2}",
            password="WalletPassword123!"
        )
        
        if result['success']:
            addr = result['address']
            addresses.append(addr)
            print(f"   ✅ Wallet {i+2}: {addr}")
            
            # Verify format
            if not addr.startswith("GXC") or len(addr) != 37:
                print(f"   ❌ Invalid format for wallet {i+2}")
                return False
        else:
            print(f"   ❌ Failed to create wallet {i+2}")
            return False
    
    # Check all addresses are unique
    if len(addresses) != len(set(addresses)):
        print(f"   ❌ Duplicate addresses found!")
        return False
    print(f"   ✅ All {len(addresses)} addresses are unique")
    
    # Summary
    print("\n" + "="*70)
    print("✅ ALL TESTS PASSED!")
    print("="*70)
    
    print("\n📊 Summary:")
    print(f"   ✅ Wallet creation works WITHOUT blockchain")
    print(f"   ✅ Addresses use GXC format")
    print(f"   ✅ Address format: GXC + 34 hex characters")
    print(f"   ✅ Total length: 37 characters")
    print(f"   ✅ Wallet retrieval works")
    print(f"   ✅ Multiple wallets work")
    print(f"   ✅ All addresses unique")
    
    print("\n🎉 GXC address format is correct!")
    print("\n📋 Example addresses created:")
    for i, addr in enumerate(addresses, 1):
        print(f"   {i}. {addr}")
    
    return True

if __name__ == "__main__":
    try:
        success = test_gxc_address_format()
        sys.exit(0 if success else 1)
    except Exception as e:
        print(f"\n❌ FATAL ERROR: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)
