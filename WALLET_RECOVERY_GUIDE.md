# GXC Wallet Recovery Guide

## Can You Recover Your Wallet?

**YES!** The GXC blockchain supports wallet recovery using both:
1. **Private Key** - Your unique cryptographic key
2. **Mnemonic Seed Phrase** - 12-word recovery phrase (BIP39 standard)

## How Wallet Recovery Works

### When You Create a Wallet

The system generates:
- **Private Key**: A 256-bit cryptographic key (64 hexadecimal characters)
- **Public Key**: Derived from the private key using elliptic curve cryptography
- **Address**: Your wallet address (starts with "GXC")
- **Mnemonic Phrase**: 12-word seed phrase for easy backup

Example wallet creation response:
```json
{
  "wallet_id": "uuid-here",
  "address": "GXC1a2b3c4d5e6f7g8h9i0j1k2l3m4n5o6p7q8r9",
  "recovery_info": {
    "private_key": "a1b2c3d4e5f6...64-char-hex",
    "seed_phrase": "word1 word2 word3 word4 word5 word6 word7 word8 word9 word10 word11 word12"
  }
}
```

### Recovery Methods

#### Method 1: Using Private Key
```python
# Import wallet using private key
wallet = GXCWallet()
wallet.import_from_private_key("your-64-char-private-key-here")
# Your address and balance are restored
```

#### Method 2: Using Mnemonic Phrase
```python
# Import wallet using 12-word seed phrase
wallet = GXCWallet()
wallet.import_from_mnemonic("word1 word2 word3 ... word12")
# Your address and balance are restored
```

## Important Security Notes

### ✅ DO:
- **Save your private key** in a secure location (password manager, encrypted file)
- **Write down your mnemonic phrase** on paper and store it safely
- **Keep multiple backups** in different secure locations
- **Never share** your private key or mnemonic with anyone
- **Test recovery** on a small amount first

### ❌ DON'T:
- Store private keys in plain text files
- Share your recovery phrase via email/chat
- Take screenshots of your private key
- Store recovery info on cloud services without encryption
- Lose both your private key AND mnemonic phrase

## Wallet Storage

The wallet service stores:
- **Encrypted Private Key**: Encrypted with your password
- **Encrypted Mnemonic**: Encrypted with your password
- **Public Address**: Safe to share
- **Wallet Metadata**: Name, type, creation date

Database location: `gxc_wallets.db`

## Recovery Process

### Via Web Interface
1. Go to `/recovery` page
2. Enter your username and password
3. Access your encrypted wallet data
4. Decrypt using your password

### Via API
```bash
# Login to get token
curl -X POST http://localhost:5000/api/v1/login \
  -H "Content-Type: application/json" \
  -d '{"username": "your-username", "password": "your-password"}'

# Get wallet info (includes encrypted recovery data)
curl -X GET http://localhost:5000/api/v1/wallets \
  -H "Authorization: Bearer YOUR_JWT_TOKEN"
```

### Manual Recovery (If Database Lost)
If you lose access to the wallet service but have your private key or mnemonic:

```python
from gxc_api import GXCWallet

# Method 1: Private Key
wallet = GXCWallet()
wallet.import_from_private_key("your-private-key")
print(f"Recovered address: {wallet.get_address()}")
print(f"Balance: {wallet.get_balance()}")

# Method 2: Mnemonic
wallet = GXCWallet()
wallet.import_from_mnemonic("your 12 word phrase here")
print(f"Recovered address: {wallet.get_address()}")
print(f"Balance: {wallet.get_balance()}")
```

## Blockchain Data Persistence

Your wallet balance and transaction history are stored on the blockchain, NOT in the wallet file. This means:

- ✅ **Balance is safe** even if you lose your wallet file
- ✅ **Transaction history** is preserved on the blockchain
- ✅ **Recovery restores everything** as long as you have private key/mnemonic
- ✅ **Multiple devices** can access the same wallet

## Example: Complete Recovery Flow

```python
#!/usr/bin/env python3
"""
Example: Recover wallet and check balance
"""
from gxc_api import GXCWallet, GXCBlockchain

# Initialize blockchain connection
blockchain = GXCBlockchain()

# Recover wallet using mnemonic
wallet = GXCWallet()
mnemonic = "abandon ability able about above absent absorb abstract absurd abuse access accident"
wallet.import_from_mnemonic(mnemonic)

# Get wallet info
address = wallet.get_address()
balance = wallet.get_balance(address)

print(f"Recovered Wallet:")
print(f"  Address: {address}")
print(f"  Balance: {balance} GXC")

# Get transaction history
transactions = wallet.get_transactions(address)
print(f"  Transactions: {len(transactions)}")

# Send a transaction (if you have balance)
if balance > 0:
    tx_hash = wallet.send_transaction(
        from_address=address,
        to_address="GXC_recipient_address",
        amount=1.0,
        fee=0.001
    )
    print(f"  Sent transaction: {tx_hash}")
```

## Backup Best Practices

### Recommended Backup Strategy:
1. **Primary**: Password manager (encrypted)
2. **Secondary**: Paper backup in safe/vault
3. **Tertiary**: Encrypted USB drive in different location

### What to Backup:
```
Wallet Name: My GXC Wallet
Address: GXC1a2b3c4d5e6f7g8h9i0j1k2l3m4n5o6p7q8r9
Private Key: a1b2c3d4e5f6g7h8i9j0k1l2m3n4o5p6q7r8s9t0u1v2w3x4y5z6a7b8c9d0e1f2
Mnemonic: word1 word2 word3 word4 word5 word6 word7 word8 word9 word10 word11 word12
Created: 2024-01-15
```

## Testing Recovery

Before storing large amounts, test the recovery process:

```bash
# 1. Create a test wallet
curl -X POST http://localhost:5000/api/v1/wallets/create \
  -H "Authorization: Bearer TOKEN" \
  -d '{"wallet_name": "Test", "password": "test123"}'

# 2. Save the private key and mnemonic

# 3. Delete the wallet from the service

# 4. Recover using saved credentials

# 5. Verify balance and address match
```

## FAQ

**Q: What if I forget my password?**
A: You can recover using your private key or mnemonic phrase, which are independent of your password.

**Q: Can I use the same wallet on multiple devices?**
A: Yes! Import your private key/mnemonic on each device.

**Q: What if I lose my private key AND mnemonic?**
A: Unfortunately, there's no way to recover your wallet. This is why backups are critical.

**Q: Is my wallet stored on the blockchain?**
A: No, only your transactions and balance are on the blockchain. Your private key is stored locally (encrypted).

**Q: Can someone steal my coins if they know my address?**
A: No, they need your private key to send transactions. Your address is safe to share.

## Support

For wallet recovery assistance:
- Check the wallet service logs: `gxc_wallets.db`
- Review blockchain explorer: http://localhost:3000
- API documentation: http://localhost:5000/api/docs

---

**Remember: Your private key and mnemonic phrase are the ONLY ways to recover your wallet. Keep them safe!**
