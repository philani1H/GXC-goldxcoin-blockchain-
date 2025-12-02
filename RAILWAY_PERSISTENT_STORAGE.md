# Railway Persistent Storage Configuration

## 🚨 CRITICAL: Database Persistence on Railway

The blockchain database **MUST** be stored in a persistent volume on Railway, otherwise all data (blocks, transactions, balances) will be lost on every deployment.

---

## ✅ Solution: Configure Railway Volume

### **Step 1: Create Railway Volume**

1. Go to your Railway project dashboard
2. Click on your service
3. Go to **"Volumes"** tab
4. Click **"New Volume"**
5. Name it: `gxc-testnet-data`
6. Set mount path: `/app/testnet_data`
7. Set size: `10GB` (or more as needed)

### **Step 2: Verify Configuration**

The `Dockerfile.testnet` already creates `/app/testnet_data` directory, and the config file sets:
```
data_dir=/app/testnet_data
```

This means the database will be stored at:
```
/app/testnet_data/testnet_blockchain.db
```

### **Step 3: Verify Volume Mount**

After deploying, verify the volume is mounted:

```bash
# SSH into Railway container (if available) or check logs
# The database file should persist at:
/app/testnet_data/testnet_blockchain.db
```

---

## 📊 Database Path Structure

### **Testnet:**
- Config: `data_dir=/app/testnet_data`
- Database: `/app/testnet_data/testnet_blockchain.db`
- **MUST contain "testnet" in path** (enforced by Database::open())

### **Mainnet:**
- Config: `data_dir=/app/mainnet_data`
- Database: `/app/mainnet_data/blockchain_mainnet.db`
- **MUST NOT contain "testnet"** (enforced by Database::open())

---

## 🔍 Verification Steps

### **1. Check Database Exists After Restart**

After deployment, check logs for:
```
"Loading X blocks from database..."
"UTXO set rebuilt: X UTXOs"
```

If you see:
```
"No blocks found in database, creating genesis block"
```

Then the database is NOT persisting (volume not mounted).

### **2. Check Balance Persistence**

1. Mine some coins (e.g., 5000 GXC)
2. Note your balance
3. Push new code (triggers redeployment)
4. After deployment, check balance again
5. Should still show 5000 GXC ✅

### **3. Check Database File**

If you have SSH access:
```bash
ls -lh /app/testnet_data/testnet_blockchain.db
# Should show file exists and has size > 0
```

---

## ⚠️ Troubleshooting

### **Problem: Balances Reset After Deployment**

**Cause:** Database volume not mounted or wrong path.

**Solution:**
1. Verify Railway volume is created and mounted at `/app/testnet_data`
2. Check config file has `data_dir=/app/testnet_data`
3. Verify database path includes "testnet" (required for testnet mode)

### **Problem: Database Not Found**

**Cause:** Volume not mounted or path mismatch.

**Solution:**
1. Check Railway volume mount path matches config `data_dir`
2. Ensure volume is attached to the service
3. Check logs for database initialization errors

### **Problem: "TESTNET mode but database path doesn't contain 'testnet'"**

**Cause:** Database path validation failed.

**Solution:**
- Database path MUST contain "testnet" for testnet mode
- Use: `/app/testnet_data/testnet_blockchain.db` ✅
- NOT: `/app/data/blockchain.db` ❌

---

## 📝 Environment Variable Override (Optional)

You can override the data directory using environment variable:

```bash
# In Railway environment variables:
DATA_DIR=/app/testnet_data
```

Then update config to use:
```
data_dir=${DATA_DIR:-/app/testnet_data}
```

---

## 🔗 Related Files

- `Dockerfile.testnet` - Creates `/app/testnet_data` directory
- `gxc-testnet.conf` - Sets `data_dir=/app/testnet_data`
- `src/node_main.cpp` - Constructs database path
- `src/Database.cpp` - Validates database path contains "testnet"

---

## ✅ Summary

**To ensure persistence:**
1. ✅ Create Railway volume: `gxc-testnet-data`
2. ✅ Mount at: `/app/testnet_data`
3. ✅ Config sets: `data_dir=/app/testnet_data`
4. ✅ Database path: `/app/testnet_data/testnet_blockchain.db`
5. ✅ Code loads blocks on startup (already implemented)

**Your 5000+ mined coins will persist! 🎉**
