# Blockchain Persistence Fix - Balances No Longer Reset

## 🐛 Problem

When pushing new changes or restarting the node, balances were reset to zero even though blocks were mined and saved to the database.

**Root Cause:**
- Blocks were saved to database ✅
- But blocks were NOT loaded from database on startup ❌
- Blockchain started with empty chain (only genesis block)
- UTXO set was empty → all balances = 0

---

## ✅ Solution

### **1. Load Blocks from Database on Startup**

Added `loadBlocksFromDatabase()` method that:
- Queries database for all blocks
- Loads blocks into memory chain
- Restores complete blockchain state

### **2. Rebuild UTXO Set**

Added `rebuildUtxoSet()` method that:
- Processes all loaded blocks
- Rebuilds UTXO set from transactions
- Restores all wallet balances

### **3. Updated Initialization Order**

**Before:**
```cpp
if (chain.empty()) {
    createGenesisBlock();  // Always creates genesis if empty
}
```

**After:**
```cpp
// Load blocks from database FIRST
loadBlocksFromDatabase();

// Only create genesis if still empty
if (chain.empty()) {
    createGenesisBlock();
} else {
    rebuildUtxoSet();  // Restore balances
}
```

---

## 🔧 Changes Made

### **Database.cpp**
- Added `getAllBlocks()`: Returns all blocks from database ordered by height
- Added `getBlocksByRange()`: Returns blocks in height range

### **Blockchain.cpp**
- Added `loadBlocksFromDatabase()`: Loads all blocks from database
- Added `rebuildUtxoSet()`: Rebuilds UTXO set from loaded blocks
- Updated `initialize()`: Loads blocks before checking if chain is empty

### **Database Path**
- Testnet: `./gxc_data/testnet_blockchain.db` or `./gxc_data/blockchain_testnet.db`
- Mainnet: `./gxc_data/blockchain_mainnet.db`
- Ensures testnet/mainnet databases are separate

---

## 📊 How It Works

### **On Startup:**

1. **Database Initialization**
   ```
   Database opens: ./gxc_data/testnet_blockchain.db
   ```

2. **Load Blocks**
   ```
   loadBlocksFromDatabase()
   → Queries: SELECT * FROM blocks ORDER BY height
   → Loads all blocks into memory chain
   → Example: Loads 100 blocks (including your 5000 coins)
   ```

3. **Rebuild UTXO Set**
   ```
   rebuildUtxoSet()
   → Processes all loaded blocks
   → Removes spent outputs (inputs)
   → Adds unspent outputs (UTXOs)
   → Example: Restores 5000 GXC UTXOs to your address
   ```

4. **Balance Restored**
   ```
   getBalance(address)
   → Queries UTXO set
   → Returns: 5000 GXC ✅
   ```

---

## 🚀 Deployment Considerations

### **Persistent Storage**

For Railway/Cloud deployments, ensure database directory is persistent:

**Option 1: Use Railway Volume**
```toml
# railway.toml
[build]
builder = "DOCKERFILE"

[deploy]
startCommand = "./gxc-node"

# Add volume for persistent data
[[services]]
volumes = [
  { mountPath = "/app/gxc_data", size = "10GB" }
]
```

**Option 2: Use Environment Variable**
```bash
# Set persistent data directory
export DATA_DIR=/persistent/gxc_data
```

**Option 3: Use Railway Persistent Volume**
- Configure Railway to mount persistent volume at `/data`
- Set `data_dir` config to `/data/gxc_data`

---

## ✅ Verification

After deployment, verify persistence:

1. **Check Database Exists**
   ```bash
   ls -la ./gxc_data/testnet_blockchain.db
   ```

2. **Check Blocks Loaded**
   ```bash
   # Check logs for:
   "Loading X blocks from database..."
   "UTXO set rebuilt: X UTXOs"
   ```

3. **Check Balance**
   ```json
   {
     "method": "getBalance",
     "params": ["tGXC9fab7317231b966af85ac453e168c0932"],
     "id": 1
   }
   ```
   Should return: `5000` (or your mined amount)

---

## 🔍 Troubleshooting

### **Balance Still Zero?**

1. **Check Database Path**
   - Verify database file exists: `ls -la ./gxc_data/*.db`
   - Check logs for database path

2. **Check Blocks in Database**
   ```sql
   SELECT COUNT(*) FROM blocks;
   SELECT MAX(height) FROM blocks;
   ```

3. **Check UTXO Set**
   ```sql
   SELECT COUNT(*) FROM utxo;
   SELECT SUM(amount) FROM utxo WHERE address = 'your_address';
   ```

4. **Check Logs**
   - Look for: "Loading X blocks from database"
   - Look for: "UTXO set rebuilt: X UTXOs"
   - If missing, blocks weren't loaded

### **Database Not Found?**

- Ensure `data_dir` config points to persistent location
- Check Railway volume mount configuration
- Verify database file permissions

---

## 📝 Summary

**Fixed:**
- ✅ Blocks now load from database on startup
- ✅ UTXO set rebuilt from loaded blocks
- ✅ Balances persist across restarts
- ✅ All mined coins preserved

**Result:**
- Your 5000 mined coins will persist after deployment
- Balances no longer reset to zero
- Blockchain state fully restored on startup

---

## 🔗 Related Files

- `src/Blockchain.cpp` - Block loading and UTXO rebuilding
- `src/Database.cpp` - Database query methods
- `src/node_main.cpp` - Database path configuration
- `src/main.cpp` - Database initialization

---

**Your balances will now persist! 🎉**
