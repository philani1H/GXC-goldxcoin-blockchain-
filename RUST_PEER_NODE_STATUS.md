# Rust Peer Node Status

## Overview

The Rust peer node is a lightweight node implementation that can sync from and connect to the C++ main node.

## Current Status

### ✅ Completed

1. **Rust Toolchain Installed**
   - Version: 1.92.0
   - Installed in Gitpod environment

2. **Rust Peer Node Built**
   - Location: `gxc-peer-node/`
   - Binary: `target/release/gxc-peer-node`
   - Build: Successful with warnings

3. **Railway Configuration Created**
   - File: `gxc-peer-node/config-railway.toml`
   - Configured to connect to Railway C++ node
   - RPC URL: https://gxc-chain112-blockchain-node-production.up.railway.app

4. **Block Structure Updated**
   - Added field aliases for C++ compatibility
   - Handles: `previousblockhash`, `merkleroot`, `time`, `miner_address`, `tx`

### ⚠️ Known Issues

1. **Duplicate Field Names**
   - C++ node returns both `merkle_root` AND `merkleroot`
   - C++ node returns both `previous_hash` AND `previousblockhash`
   - Causes JSON parsing errors in Rust node
   - **Fix needed**: C++ node should return consistent field names

2. **P2P Protocol**
   - Rust node has P2P implementation
   - C++ node has P2P implementation
   - Not yet tested for compatibility
   - May need protocol alignment

## Usage

### Build

```bash
cd gxc-peer-node
cargo build --release
```

### Run Commands

```bash
# Sync blockchain from Railway node
./target/release/gxc-peer-node -c config-railway.toml sync

# Show blockchain stats
./target/release/gxc-peer-node -c config-railway.toml stats

# Start peer node
./target/release/gxc-peer-node -c config-railway.toml start

# Verify blockchain integrity
./target/release/gxc-peer-node -c config-railway.toml verify
```

### Configuration

Edit `config-railway.toml`:

```toml
[network]
name = "testnet"

[p2p]
listen_addr = "0.0.0.0"
listen_port = 18444
max_peers = 50
bootstrap_peers = [
    "gxc-chain112-blockchain-node-production.up.railway.app:18333"
]

[rpc]
node_url = "https://gxc-chain112-blockchain-node-production.up.railway.app"

[node]
node_id = "gxc-peer-rust-1"
data_dir = "./data"
```

## Next Steps

### 1. Fix C++ Node Field Names

The C++ node needs to return consistent field names. Choose one:

**Option A: Use snake_case (recommended)**
- `previous_hash` (not `previousblockhash`)
- `merkle_root` (not `merkleroot`)
- `miner_address` (not `miner`)

**Option B: Use camelCase**
- `previousblockhash` (not `previous_hash`)
- `merkleroot` (not `merkle_root`)
- `minerAddress` (not `miner`)

**Fix location**: `src/RPCAPI.cpp` - getblock method

### 2. Test P2P Connection

Once field names are fixed:

```bash
# Terminal 1: C++ node (already running on Railway)

# Terminal 2: Rust peer node
cd gxc-peer-node
./target/release/gxc-peer-node -c config-railway.toml start
```

### 3. Verify Sync

```bash
# Sync blockchain
./target/release/gxc-peer-node -c config-railway.toml sync

# Check stats
./target/release/gxc-peer-node -c config-railway.toml stats
```

### 4. Test Block Propagation

1. Mine a block on C++ node
2. Verify Rust node receives it via P2P
3. Check Rust node syncs the new block

## Architecture

```
┌─────────────────────────────────────┐
│   C++ Main Node (Railway)           │
│   - Full blockchain                 │
│   - Mining                          │
│   - RPC API                         │
│   - P2P Server (port 18333)         │
└──────────────┬──────────────────────┘
               │
               │ P2P Connection
               │ RPC Sync
               │
┌──────────────▼──────────────────────┐
│   Rust Peer Node (Local/Cloud)      │
│   - Lightweight                     │
│   - Sync only                       │
│   - Block validation                │
│   - P2P Client (port 18444)         │
└─────────────────────────────────────┘
```

## Benefits of Rust Peer Node

1. **Lightweight**: Smaller resource footprint
2. **Fast Sync**: Efficient blockchain synchronization
3. **Validation**: Independent block verification
4. **Scalability**: Easy to deploy multiple peers
5. **Language Diversity**: Rust + C++ ecosystem

## Testing Checklist

- [x] Rust toolchain installed
- [x] Peer node builds successfully
- [x] Configuration file created
- [x] Block structure matches C++ format
- [ ] Fix duplicate field names in C++ node
- [ ] Sync completes successfully
- [ ] P2P connection established
- [ ] Block propagation works
- [ ] Validation passes

## Troubleshooting

### "duplicate field" Error

**Cause**: C++ node returns both `merkle_root` and `merkleroot`

**Solution**: Fix C++ node to return only one field name

### "Failed to parse RPC response"

**Cause**: Field name mismatch between C++ and Rust

**Solution**: Ensure field names match exactly (use aliases in Rust)

### P2P Connection Fails

**Cause**: Port mismatch or firewall

**Solution**: 
- Check C++ node is listening on port 18333
- Verify firewall allows connections
- Check Railway port configuration

## Summary

✅ **Rust peer node is built and ready**  
⚠️ **Blocked by C++ node field name inconsistency**  
📋 **Next**: Fix C++ getblock to return consistent field names  

Once the field names are fixed, the Rust peer node will be able to sync and connect to the C++ node successfully.
