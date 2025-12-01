# 🔒 Database Separation - Testnet vs Mainnet

## ✅ FIXED: Complete Database Isolation

### **The Problem (Before)**

The node was using `./gxc_data/blockchain.db` for BOTH testnet and mainnet:

```
❌ Testnet: ./gxc_data/blockchain.db
❌ Mainnet: ./gxc_data/blockchain.db
❌ SAME FILE = DATA CORRUPTION RISK!
```

**Why This Was Dangerous:**
- Testing on testnet could corrupt mainnet data
- Switching networks would mix data
- No way to tell which network a database belongs to
- Config file settings were ignored

---

### **The Solution (After)**

Now each network has its own database file:

```
✅ Testnet: ./gxc_data/blockchain_testnet.db
✅ Mainnet: ./gxc_data/blockchain_mainnet.db
✅ SEPARATE FILES = NO CORRUPTION!
```

---

## 🛡️ Safety Features

### **1. Automatic Database Naming**

The node automatically chooses the correct database based on the `--testnet` flag:

```bash
# Testnet mode
./gxc-node --testnet
# Uses: blockchain_testnet.db

# Mainnet mode
./gxc-node
# Uses: blockchain_mainnet.db
```

### **2. Path Validation (CRITICAL)**

The database layer validates paths before opening:

**Testnet Mode:**
```cpp
if (isTestnet && dbPath.find("testnet") == std::string::npos) {
    LOG_ERROR("TESTNET mode but path doesn't contain 'testnet'");
    LOG_ERROR("REFUSING to open - risk of mainnet data corruption!");
    return false;  // ⛔ BLOCKED
}
```

**Mainnet Mode:**
```cpp
if (!isTestnet && dbPath.find("testnet") != std::string::npos) {
    LOG_ERROR("MAINNET mode but path contains 'testnet'");
    LOG_ERROR("REFUSING to open testnet database in mainnet mode!");
    return false;  // ⛔ BLOCKED
}
```

### **3. Config File Respect**

Before, config file settings were always overridden:

```cpp
// OLD (WRONG):
Config::loadFromFile("config.conf");  // data_dir=/app/testnet_data
Config::set("data_dir", "./gxc_data"); // ❌ OVERRIDES CONFIG FILE!
```

Now, config files are respected:

```cpp
// NEW (CORRECT):
Config::loadFromFile("config.conf");  // data_dir=/app/testnet_data

if (dataDirSetViaCommandLine) {
    Config::set("data_dir", commandLineValue);  // Override only if explicit
} else {
    dataDir = Config::get("data_dir");  // ✅ USE CONFIG FILE VALUE
}
```

### **4. Mainnet Protection**

If a mainnet database already exists, the node warns loudly:

```
========================================
WARNING: MAINNET DATABASE ALREADY EXISTS!
Path: ./gxc_data/blockchain_mainnet.db
If you're still testing, use --testnet flag!
========================================
```

---

## 📊 How It Works

### **Database Path Selection**

```cpp
// In node_main.cpp
std::string dbPath;
if (nodeConfig.testnet) {
    dbPath = nodeConfig.dataDir + "/blockchain_testnet.db";
    LOG_INFO("Using TESTNET database: " + dbPath);
} else {
    dbPath = nodeConfig.dataDir + "/blockchain_mainnet.db";
    LOG_INFO("Using MAINNET database: " + dbPath);
}
```

### **Validation Flow**

```
1. Node starts with --testnet flag
   ↓
2. Determines database path: blockchain_testnet.db
   ↓
3. Database::open() validates path
   ↓
4. Checks: isTestnet=true, path contains "testnet"? YES ✅
   ↓
5. Opens database safely
```

### **Rejection Flow**

```
1. Node starts WITHOUT --testnet flag (mainnet mode)
   ↓
2. Determines database path: blockchain_mainnet.db
   ↓
3. Database::open() validates path
   ↓
4. Checks: isTestnet=false, path contains "testnet"? NO ✅
   ↓
5. Opens database safely

BUT if someone tries to trick it:
1. Node starts WITHOUT --testnet flag
   ↓
2. Someone manually sets path to blockchain_testnet.db
   ↓
3. Database::open() validates path
   ↓
4. Checks: isTestnet=false, path contains "testnet"? YES ❌
   ↓
5. REFUSES to open! Logs error and returns false
```

---

## 🎯 Examples

### **Example 1: Normal Testnet Usage**

```bash
$ ./gxc-node --testnet

[INFO] Using TESTNET database: ./gxc_data/blockchain_testnet.db
[INFO] ✓ Testnet database path validated
[INFO] Network mode: TESTNET
[INFO] Database opened successfully
```

✅ **Result:** Creates/opens `blockchain_testnet.db`

---

### **Example 2: Normal Mainnet Usage**

```bash
$ ./gxc-node

[INFO] Using MAINNET database: ./gxc_data/blockchain_mainnet.db
[INFO] ✓ Mainnet database path validated
[INFO] Network mode: MAINNET
[INFO] Database opened successfully
```

✅ **Result:** Creates/opens `blockchain_mainnet.db`

---

### **Example 3: Protection in Action**

```bash
# Someone tries to use testnet database in mainnet mode
$ ./gxc-node --datadir=/app/testnet_data

[ERROR] MAINNET mode but database path contains 'testnet'
[ERROR] REFUSING to open testnet database in mainnet mode!
[ERROR] This would corrupt testnet data!
[ERROR] Failed to initialize database
```

⛔ **Result:** Node refuses to start, data protected!

---

### **Example 4: Config File Respected**

**Config file (gxc-testnet.conf):**
```
data_dir=/app/testnet_data
testnet=true
```

**Command:**
```bash
$ ./gxc-node --testnet --config=gxc-testnet.conf

[INFO] Configuration loaded from: gxc-testnet.conf
[INFO] Data directory: /app/testnet_data  ← FROM CONFIG FILE!
[INFO] Using TESTNET database: /app/testnet_data/blockchain_testnet.db
```

✅ **Result:** Config file value is used!

---

## 🔍 Verification

### **Check Which Database is Being Used**

```bash
# Start the node and check logs
./gxc-node --testnet 2>&1 | grep "Using.*database"

# Output:
# [INFO] Using TESTNET database: ./gxc_data/blockchain_testnet.db
```

### **List Databases**

```bash
$ ls -lh gxc_data/*.db

-rw-r--r-- 1 user user 4.0K Dec  1 06:09 blockchain_testnet.db
```

✅ **Only testnet database exists** (no mainnet data during testing)

---

## 📋 Migration Guide

### **If You Have Old Data**

If you have an old `blockchain.db` file, you need to determine which network it belongs to:

```bash
# Check if it's testnet or mainnet data
sqlite3 blockchain.db "SELECT * FROM blocks LIMIT 1;"

# If it's testnet data:
mv blockchain.db blockchain_testnet.db

# If it's mainnet data:
mv blockchain.db blockchain_mainnet.db
```

---

## 🚨 Important Notes

### **For Developers**

1. **Always use --testnet flag during testing**
   ```bash
   ./gxc-node --testnet  # ✅ CORRECT
   ./gxc-node            # ❌ WRONG (creates mainnet data)
   ```

2. **Never manually edit database paths to bypass validation**
   - The validation is there to protect you
   - Bypassing it can corrupt data

3. **Config files are now respected**
   - Set `data_dir` in config file
   - It will be used unless overridden by command line

### **For Deployment**

1. **Docker/Railway deployments**
   - Use `--testnet` flag for testnet nodes
   - Omit flag for mainnet nodes
   - Database naming is automatic

2. **Data persistence**
   - Mount the data directory as a volume
   - Both `blockchain_testnet.db` and `blockchain_mainnet.db` can coexist
   - They will never interfere with each other

3. **Backups**
   - Backup testnet: `blockchain_testnet.db*`
   - Backup mainnet: `blockchain_mainnet.db*`
   - Keep them separate!

---

## ✅ Summary

**What Changed:**
- ✅ Separate database files for testnet/mainnet
- ✅ Automatic database naming based on network mode
- ✅ Strict path validation prevents data mixing
- ✅ Config file values are respected
- ✅ Loud warnings prevent mistakes

**Why It Matters:**
- ✅ No more mainnet data during testing
- ✅ Impossible to corrupt mainnet data accidentally
- ✅ Clear separation of concerns
- ✅ Production-ready database management

**Current Status:**
- ✅ Testnet database: `blockchain_testnet.db`
- ✅ Mainnet database: None (testing only)
- ✅ All safety validations active
- ✅ Config file respect working

---

*Last Updated: December 1, 2025*
*Version: 2.0.0*
*Status: Database Separation Complete ✅*
