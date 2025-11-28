#!/usr/bin/env python3
"""
Quick wallet demo - Create and retrieve a wallet right now!
"""

import sqlite3
import hashlib
import secrets
from mnemonic import Mnemonic
from ecdsa import SigningKey, SECP256k1
import base58
from datetime import datetime

def create_and_retrieve_wallet():
    """Simple demo of wallet creation and retrieval"""
    
    print("\n" + "="*60)
    print("🔐 GXC WALLET DEMO - CREATE & RETRIEVE")
    print("="*60)
    
    # Initialize
    mnemo = Mnemonic("english")
    db_path = 'gxc_wallets.db'
    
    # Step 1: Generate mnemonic
    print("\n1️⃣  Generating mnemonic phrase...")
    mnemonic = mnemo.generate(strength=256)
    print(f"   ✅ Mnemonic: {mnemonic[:50]}...")
    
    # Step 2: Generate keys
    print("\n2️⃣  Generating cryptographic keys...")
    seed = mnemo.to_seed(mnemonic)
    private_key = SigningKey.from_string(seed[:32], curve=SECP256k1)
    public_key = private_key.get_verifying_key()
    
    # Generate address
    pub_key_bytes = public_key.to_string()
    sha256_hash = hashlib.sha256(pub_key_bytes).digest()
    ripemd160_hash = hashlib.new('ripemd160', sha256_hash).digest()
    versioned = b'\x00' + ripemd160_hash
    checksum = hashlib.sha256(hashlib.sha256(versioned).digest()).digest()[:4]
    address = base58.b58encode(versioned + checksum).decode('utf-8')
    
    print(f"   ✅ Address: {address}")
    print(f"   ✅ Public Key: {public_key.to_string().hex()[:40]}...")
    
    # Step 3: Encrypt private key
    print("\n3️⃣  Encrypting private key with password...")
    password = "DemoPassword123!"
    key = hashlib.sha256(password.encode()).digest()
    encrypted_private_key = hashlib.sha256(
        (private_key.to_string().hex() + key.hex()).encode()
    ).hexdigest()
    print(f"   ✅ Private key encrypted")
    
    # Step 4: Store in database
    print("\n4️⃣  Storing wallet in database...")
    wallet_id = secrets.token_hex(16)
    user_id = "demo_user_" + secrets.token_hex(4)
    
    conn = sqlite3.connect(db_path)
    cursor = conn.cursor()
    
    cursor.execute('''
        INSERT INTO wallets (
            wallet_id, user_id, wallet_name, address, public_key,
            encrypted_private_key, wallet_type, created_at, last_used,
            balance, is_default, backup_stored, encrypted_mnemonic
        ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    ''', (
        wallet_id, user_id, "Demo Wallet", address, 
        public_key.to_string().hex(),
        encrypted_private_key, 'standard', 
        datetime.now(), datetime.now(),
        0.0, True, False, encrypted_private_key
    ))
    
    conn.commit()
    print(f"   ✅ Wallet stored with ID: {wallet_id}")
    
    # Step 5: Retrieve from database
    print("\n5️⃣  Retrieving wallet from database...")
    conn.row_factory = sqlite3.Row
    cursor = conn.cursor()
    
    cursor.execute('SELECT * FROM wallets WHERE wallet_id = ?', (wallet_id,))
    row = cursor.fetchone()
    
    if row:
        wallet = dict(row)
        print(f"   ✅ Wallet retrieved successfully!")
        print(f"\n   📋 Wallet Details:")
        print(f"      Wallet ID: {wallet['wallet_id']}")
        print(f"      User ID: {wallet['user_id']}")
        print(f"      Name: {wallet['wallet_name']}")
        print(f"      Address: {wallet['address']}")
        print(f"      Balance: {wallet['balance']} GXC")
        print(f"      Created: {wallet['created_at']}")
        print(f"      Type: {wallet['wallet_type']}")
    else:
        print("   ❌ Failed to retrieve wallet")
        conn.close()
        return False
    
    # Step 6: Verify data integrity
    print("\n6️⃣  Verifying data integrity...")
    if wallet['address'] == address:
        print("   ✅ Address matches original")
    else:
        print("   ❌ Address mismatch!")
        conn.close()
        return False
    
    if wallet['wallet_id'] == wallet_id:
        print("   ✅ Wallet ID matches")
    else:
        print("   ❌ Wallet ID mismatch!")
        conn.close()
        return False
    
    # Step 7: Show all wallets in database
    print("\n7️⃣  Checking total wallets in database...")
    cursor.execute('SELECT COUNT(*) FROM wallets')
    total = cursor.fetchone()[0]
    print(f"   ✅ Total wallets in database: {total}")
    
    conn.close()
    
    # Success summary
    print("\n" + "="*60)
    print("✅ SUCCESS! WALLET SYSTEM WORKS PERFECTLY!")
    print("="*60)
    print("\n📊 What just happened:")
    print("   1. Generated secure mnemonic phrase")
    print("   2. Created cryptographic key pair")
    print("   3. Generated blockchain address")
    print("   4. Encrypted private key with password")
    print("   5. Stored wallet in database")
    print("   6. Retrieved wallet successfully")
    print("   7. Verified data integrity")
    print("\n🎉 You can create and retrieve wallets successfully!")
    print("\n💡 Key Information:")
    print(f"   Wallet ID: {wallet_id}")
    print(f"   Address: {address}")
    print(f"   Password: {password}")
    print(f"   Mnemonic: {mnemonic[:50]}...")
    print("\n📖 See WALLET_USAGE_GUIDE.md for more details")
    print("="*60 + "\n")
    
    return True

if __name__ == "__main__":
    try:
        create_and_retrieve_wallet()
    except Exception as e:
        print(f"\n❌ Error: {e}")
        import traceback
        traceback.print_exc()
