# 🎉 GXC Blockchain is Now Running!

## ✅ Services Status

All blockchain services are up and running:

| Service | Port | Status | URL |
|---------|------|--------|-----|
| **Blockchain Node** | 18332 | ✅ Running | http://localhost:18332 |
| **Explorer** | 3000 | ✅ Running | http://localhost:3000 |
| **Wallet API** | 5000 | ✅ Running | http://localhost:5000 |
| **Forum** | 3001 | ✅ Running | http://localhost:3001 |

## 🌐 Access Your Blockchain

### Blockchain Explorer
**URL:** http://localhost:3000

Features:
- View blocks and transactions in real-time
- Search by address, transaction hash, or block number
- Monitor network statistics
- Track gold-backed tokens (GXC-G)
- View governance proposals
- Cross-chain bridge monitoring
- Transaction simulator

### Wallet Service API
**URL:** http://localhost:5000

Endpoints:
- `POST /api/v1/register` - Create new user account
- `POST /api/v1/login` - Authenticate user
- `POST /api/v1/wallets/create` - Create new wallet
- `GET /api/v1/wallets` - List user wallets
- `GET /api/v1/wallets/{id}/balance` - Check wallet balance
- `POST /api/v1/wallets/{id}/send` - Send transaction

### Forum
**URL:** http://localhost:3001

Features:
- Community discussions
- Real-time chat
- Support system
- Announcements

## 💰 Wallet Recovery - YES, You Can Recover!

### Answer to Your Question: "Can I use my wallet again if I have the private key or mnemonic?"

**YES! Absolutely!** 🎯

Your wallet can be fully recovered using either:
1. **Private Key** (64-character hexadecimal string)
2. **Mnemonic Phrase** (12-word seed phrase)

### How It Works

When you create a wallet, you receive:
```json
{
  "wallet_id": "abc-123-def-456",
  "address": "GXC1a2b3c4d5e6f7g8h9i0j1k2l3m4n5o6p7q8r9",
  "private_key": "a1b2c3d4e5f6g7h8i9j0k1l2m3n4o5p6q7r8s9t0u1v2w3x4y5z6a7b8c9d0e1f2",
  "mnemonic": "word1 word2 word3 word4 word5 word6 word7 word8 word9 word10 word11 word12"
}
```

### Recovery Methods

#### Method 1: Via Web Interface
1. Go to http://localhost:5000/recovery
2. Enter your username and password
3. Access your encrypted wallet data

#### Method 2: Via API
```bash
# Login
curl -X POST http://localhost:5000/api/v1/login \
  -H "Content-Type: application/json" \
  -d '{"username": "your-username", "password": "your-password"}'

# Get wallet info (includes recovery data)
curl -X GET http://localhost:5000/api/v1/wallets \
  -H "Authorization: Bearer YOUR_JWT_TOKEN"
```

#### Method 3: Direct Recovery (If you have private key/mnemonic)
```python
from gxc_api import GXCWallet

# Using private key
wallet = GXCWallet()
wallet.import_from_private_key("your-64-char-private-key")
print(f"Address: {wallet.get_address()}")
print(f"Balance: {wallet.get_balance()}")

# Using mnemonic
wallet = GXCWallet()
wallet.import_from_mnemonic("your 12 word phrase here")
print(f"Address: {wallet.get_address()}")
print(f"Balance: {wallet.get_balance()}")
```

### Important Notes

✅ **Your balance is stored on the blockchain**, not in the wallet file
✅ **Recovery works even if you lose the wallet file**
✅ **You can use the same wallet on multiple devices**
✅ **Transaction history is preserved on the blockchain**

❌ **If you lose BOTH private key AND mnemonic, recovery is impossible**
❌ **Never share your private key or mnemonic with anyone**

## 🚀 Quick Start Guide

### 1. Create a Wallet

```bash
# Register a user
curl -X POST http://localhost:5000/api/v1/register \
  -H "Content-Type: application/json" \
  -d '{
    "username": "alice",
    "email": "alice@example.com",
    "password": "secure_password_123"
  }'

# Login to get token
curl -X POST http://localhost:5000/api/v1/login \
  -H "Content-Type: application/json" \
  -d '{
    "username": "alice",
    "password": "secure_password_123"
  }'

# Create wallet
curl -X POST http://localhost:5000/api/v1/wallets/create \
  -H "Authorization: Bearer YOUR_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "wallet_name": "My First Wallet",
    "password": "wallet_password_123"
  }'
```

### 2. Check Balance

```bash
curl -X GET http://localhost:5000/api/v1/wallets/{wallet_id}/balance \
  -H "Authorization: Bearer YOUR_TOKEN"
```

### 3. Send Transaction

```bash
curl -X POST http://localhost:5000/api/v1/wallets/{wallet_id}/send \
  -H "Authorization: Bearer YOUR_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "to_address": "GXC_recipient_address",
    "amount": 10.0,
    "password": "wallet_password_123"
  }'
```

### 4. View on Explorer

Open http://localhost:3000 and search for your transaction hash or address

## 📊 Monitoring

### View Logs
```bash
# All logs
tail -f logs/*.log

# Specific service
tail -f logs/blockchain_node.log
tail -f logs/explorer.log
tail -f logs/wallet_api.log
tail -f logs/forum.log
```

### Check Service Status
```bash
ps aux | grep python3 | grep -E "testnet|explorer|wallet|forum"
```

### View Process IDs
```bash
cat pids/*.pid
```

## 🛑 Stop Services

```bash
./stop_blockchain.sh
```

This will gracefully stop all services.

## 🔧 Troubleshooting

### Service Not Starting?

1. Check logs: `cat logs/[service_name].log`
2. Check if port is in use: `lsof -i :[port]`
3. Restart service: `./stop_blockchain.sh && ./start_blockchain.sh`

### Can't Connect to Service?

1. Verify service is running: `ps aux | grep [service_name]`
2. Check firewall settings
3. Try accessing via 127.0.0.1 instead of localhost

### Database Issues?

1. Check database files exist: `ls -la *.db`
2. Check permissions: `chmod 644 *.db`
3. Backup and recreate: `mv gxc_wallets.db gxc_wallets.db.bak`

## 📚 Documentation

- **Wallet Recovery Guide:** [WALLET_RECOVERY_GUIDE.md](WALLET_RECOVERY_GUIDE.md)
- **Bug Fix Summary:** [BUG_FIX_SUMMARY.md](BUG_FIX_SUMMARY.md)
- **API Documentation:** http://localhost:5000/api/docs (if available)

## 🎯 What's Next?

1. **Create your first wallet** using the API or web interface
2. **Save your private key and mnemonic** in a secure location
3. **Test wallet recovery** to ensure you can restore it
4. **Explore the blockchain** using the explorer
5. **Join the forum** to connect with the community

## 🔐 Security Reminders

- ✅ Always backup your private key and mnemonic
- ✅ Use strong passwords
- ✅ Keep recovery phrases offline
- ✅ Test recovery process with small amounts first
- ❌ Never share private keys
- ❌ Don't store recovery info in plain text
- ❌ Don't take screenshots of private keys

## 📞 Support

If you need help:
1. Check the logs in `./logs/`
2. Review documentation in this repository
3. Check the forum at http://localhost:3001
4. Review the code in the repository

---

**🎉 Congratulations! Your GXC blockchain is now running and ready to use!**

**Remember:** Your private key and mnemonic phrase are the ONLY ways to recover your wallet. Keep them safe!
