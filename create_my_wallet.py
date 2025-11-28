#!/usr/bin/env python3
"""
Create a wallet for mining rewards
"""

import sys
import os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'api'))

from wallet_service import WalletService

def create_mining_wallet():
    """Create a wallet to receive mining rewards"""
    
    print("\n" + "="*70)
    print("🔐 CREATING YOUR MINING WALLET")
    print("="*70)
    
    wallet_service = WalletService()
    
    # Create user
    print("\n1️⃣  Creating user account...")
    user_result = wallet_service.create_user(
        username="miner_user",
        email="miner@gxc.local",
        password="MiningPassword123!"
    )
    
    if not user_result['success']:
        print(f"   ⚠️  User might already exist, trying to authenticate...")
        # Try to authenticate existing user
        auth_result = wallet_service.authenticate_user("miner_user", "MiningPassword123!")
        if auth_result['success']:
            user_id = auth_result['user_id']
            print(f"   ✅ Using existing user: {user_id}")
        else:
            print(f"   ❌ Failed: {user_result.get('error')}")
            return None
    else:
        user_id = user_result['user_id']
        print(f"   ✅ User created: {user_id}")
    
    # Create wallet
    print("\n2️⃣  Creating mining wallet...")
    result = wallet_service.create_wallet(
        user_id=user_id,
        wallet_name="Mining Rewards Wallet",
        password="MiningWalletPassword123!"
    )
    
    if not result['success']:
        print(f"   ❌ Failed to create wallet: {result.get('error')}")
        return None
    
    address = result['address']
    wallet_id = result['wallet_id']
    mnemonic = result.get('mnemonic', 'N/A')
    
    print(f"   ✅ Wallet created successfully!")
    
    # Display wallet info
    print("\n" + "="*70)
    print("✅ YOUR MINING WALLET IS READY!")
    print("="*70)
    
    print(f"\n📋 Wallet Information:")
    print(f"\n   Wallet ID:")
    print(f"   {wallet_id}")
    
    print(f"\n   Mining Address (GXC Format):")
    print(f"   ┌──────────────────────────────────────────────┐")
    print(f"   │ {address} │")
    print(f"   │ ^^^└─────────────────────────────────────────┘│")
    print(f"   │  │           34 hex characters                │")
    print(f"   │  └─ \"GXC\" prefix                              │")
    print(f"   └───────────────────────────────────────────────┘")
    
    print(f"\n   ⚠️  SAVE YOUR RECOVERY PHRASE:")
    print(f"   {mnemonic}")
    
    print(f"\n💡 This address will receive:")
    print(f"   • Genesis block reward")
    print(f"   • Mining rewards from all blocks")
    print(f"   • Transaction fees")
    
    # Save to file
    with open('MINING_WALLET.txt', 'w') as f:
        f.write("="*70 + "\n")
        f.write("GXC MINING WALLET\n")
        f.write("="*70 + "\n\n")
        f.write(f"Wallet ID: {wallet_id}\n")
        f.write(f"Address: {address}\n")
        f.write(f"Mnemonic: {mnemonic}\n")
        f.write(f"\nCreated: {result.get('created_at', 'N/A')}\n")
        f.write("\n" + "="*70 + "\n")
        f.write("⚠️  KEEP THIS FILE SECURE!\n")
        f.write("This mnemonic phrase can recover your wallet.\n")
        f.write("="*70 + "\n")
    
    print(f"\n💾 Wallet info saved to: MINING_WALLET.txt")
    
    print("\n" + "="*70)
    print("🎯 NEXT STEPS:")
    print("="*70)
    print(f"\n1. Use this address for mining:")
    print(f"   {address}")
    print(f"\n2. Start the blockchain node with this address")
    print(f"\n3. Mine the genesis block")
    print(f"\n4. Receive mining rewards!")
    
    return {
        'address': address,
        'wallet_id': wallet_id,
        'mnemonic': mnemonic
    }

if __name__ == "__main__":
    try:
        wallet = create_mining_wallet()
        if wallet:
            print(f"\n✅ Success! Your mining address: {wallet['address']}\n")
        else:
            print(f"\n❌ Failed to create wallet\n")
            sys.exit(1)
    except Exception as e:
        print(f"\n❌ Error: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)
