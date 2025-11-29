# 🌐 Network-Aware Services Configuration Guide

All GXC services now support both **testnet** and **mainnet** networks with automatic differentiation.

## 📋 Services Updated

All services now use the centralized network configuration from `config/railway_config.py`:

### ✅ Updated Services

1. **Blockchain Explorer** (`web/blockchain_explorer.py`)
   - Network-aware database paths
   - Network-specific node URLs
   - Separate databases for testnet/mainnet

2. **Wallet Service** (`api/wallet_service.py`)
   - Network-aware wallet databases
   - Network-specific addresses (tGXC for testnet, GXC for mainnet)
   - Automatic network detection

3. **Miners**
   - `mine_testnet_fixed.py` - Network-aware miner
   - `mining/gxhash_miner.py` - Network-aware GXHash miner

4. **Mining Pools** (`mining_pool/pool_base.py`)
   - Network-aware RPC URLs
   - Network-specific configurations

5. **Centralized Config** (`config/railway_config.py`)
   - Complete network configuration
   - Network-aware database paths
   - Network-specific URLs and settings

## 🔧 How to Switch Networks

### Option 1: Environment Variable (Recommended)

Set the `GXC_NETWORK` environment variable:

```bash
# For Testnet (default)
export GXC_NETWORK=testnet

# For Mainnet
export GXC_NETWORK=mainnet
```

### Option 2: In Code (for testing)

All services automatically use the centralized config, which reads from the environment variable.

## 📊 Network Configuration

### Testnet Configuration

| Setting | Value |
|---------|-------|
| Address Prefix | `tGXC` |
| Block Time | 60 seconds |
| Block Reward | 12.5 GXC |
| RPC Port (local) | 18332 |
| Railway URL | `https://gxc-chain112-blockchain-node-production.up.railway.app` |
| Wallet DB | `gxc_wallets_testnet.db` |
| Explorer DB | `gxc_explorer_testnet.db` |
| Blockchain DB | `gxc_testnet_data/blockchain.db` |

### Mainnet Configuration

| Setting | Value |
|---------|-------|
| Address Prefix | `GXC` |
| Block Time | 600 seconds (10 minutes) |
| Block Reward | 50.0 GXC |
| RPC Port (local) | 8332 |
| Railway URL | (Set when mainnet node is deployed) |
| Wallet DB | `gxc_wallets.db` |
| Explorer DB | `gxc_explorer_mainnet.db` |
| Blockchain DB | `gxc_data/blockchain.db` |

## 💾 Database Separation

**Important:** Each network uses separate databases to avoid data mixing:

- **Testnet Wallets:** `gxc_wallets_testnet.db`
- **Mainnet Wallets:** `gxc_wallets.db`
- **Testnet Explorer:** `gxc_explorer_testnet.db`
- **Mainnet Explorer:** `gxc_explorer_mainnet.db`
- **Testnet Blockchain:** `gxc_testnet_data/blockchain.db`
- **Mainnet Blockchain:** `gxc_data/blockchain.db`

## 🚀 Usage Examples

### Wallet Service

```python
from config.railway_config import get_wallet_database_path, get_network_info

# Automatically uses current network from environment
wallet_db = get_wallet_database_path()
network_info = get_network_info()
print(f"Network: {network_info['network']}")
print(f"Database: {wallet_db}")
```

### Miner

```bash
# Testnet
export GXC_NETWORK=testnet
python mine_testnet_fixed.py

# Mainnet
export GXC_NETWORK=mainnet
python mine_testnet_fixed.py
```

### Explorer

```bash
# Testnet
export GXC_NETWORK=testnet
python web/blockchain_explorer.py

# Mainnet
export GXC_NETWORK=mainnet
python web/blockchain_explorer.py
```

### Mining Pool

```bash
# Testnet
export GXC_NETWORK=testnet
python mining_pool/general_pool.py

# Mainnet
export GXC_NETWORK=mainnet
python mining_pool/general_pool.py
```

## 🔍 Verification

### Check Current Network

```python
from config.railway_config import get_network, get_network_info

network = get_network()  # Returns 'testnet' or 'mainnet'
info = get_network_info()  # Returns complete network info dict

print(f"Current Network: {network.upper()}")
print(f"Address Prefix: {info['address_prefix']}")
print(f"Block Reward: {info['block_reward']} {info['currency']}")
```

### Check Database Paths

```python
from config.railway_config import (
    get_wallet_database_path,
    get_explorer_database_path,
    get_blockchain_database_path
)

print(f"Wallet DB: {get_wallet_database_path()}")
print(f"Explorer DB: {get_explorer_database_path()}")
print(f"Blockchain DB: {get_blockchain_database_path()}")
```

## 📡 API Endpoints

All services expose network information:

### Wallet Service
- `GET /api/network-info` - Returns network configuration

### Explorer
- `GET /api/network-info` - Returns network configuration
- `GET /api/connection-status` - Returns connection status

## ⚠️ Important Notes

1. **Default Network:** If `GXC_NETWORK` is not set, services default to **testnet**

2. **Database Separation:** Switching networks does NOT migrate data. Each network maintains its own database.

3. **Address Format:** 
   - Testnet addresses start with `tGXC`
   - Mainnet addresses start with `GXC`

4. **Node URLs:** Services automatically use the correct node URL for the selected network.

5. **Environment Variables:** You can override specific settings:
   ```bash
   export GXC_NETWORK=testnet
   export BLOCKCHAIN_RPC_URL=https://custom-node-url.com  # Overrides default
   export DATABASE_PATH=/custom/path.db  # Overrides default path
   ```

## 🔄 Migration Guide

If you have existing services, they will:
- ✅ Continue working with testnet by default
- ✅ Automatically use separate databases for each network
- ✅ Show network information in logs and APIs
- ✅ Support switching via environment variable

No code changes needed - just set the `GXC_NETWORK` environment variable!

## 🎯 Summary

**All services are now network-aware!**

- ✅ Automatic network detection via `GXC_NETWORK` environment variable
- ✅ Separate databases for testnet and mainnet
- ✅ Network-specific configurations (addresses, rewards, etc.)
- ✅ Easy switching between networks
- ✅ No data mixing between networks
- ✅ Centralized configuration management

