# Genesis Block Fix Summary

## Changes Made

### 1. Automatic Genesis Block Creation
**File**: `src/Blockchain.cpp`
- Genesis block is now automatically created on node startup if blockchain is empty
- Genesis block includes:
  - 50 GXC reward to address: `tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf`
  - Work receipt generation
  - Proper UTXO creation

### 2. UTXO Key Format Fix
**File**: `src/Blockchain.cpp`
- Fixed UTXO key format to use `_` separator (was using `:`)
- Format: `{txHash}_{outputIndex}`
- This ensures getBalance() can find the UTXOs

### 3. Coinbase Maturity Adjustment
**File**: `src/Blockchain.cpp`
- Testnet: 6 blocks maturity (was 100)
- Mainnet: 100 blocks maturity
- This allows faster testing on testnet

### 4. Genesis Block Details
- **Address**: `tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf`
- **Reward**: 50 GXC
- **Difficulty**: 1.0 (testnet)
- **Work Receipt**: Automatically generated

## How to Use

### 1. Clean Start
```bash
# Stop any running nodes
pkill -9 gxc-node

# Clean old blockchain data
rm -rf gxc_data/blockchain_testnet.db
rm -rf gxc_testnet_data/blocks gxc_testnet_data/chainstate

# Start node
./gxc-node --testnet
```

### 2. Check Genesis Block
```bash
# Check blockchain height (should be 1 after genesis)
curl -X POST http://localhost:8332 -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getblockcount","params":[],"id":1}'

# Check genesis block
curl -X POST http://localhost:8332 -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getblock","params":[0],"id":1}'
```

### 3. Check Balance
```bash
# Check balance (will be 0 until 6 confirmations)
curl -X POST http://localhost:8332 -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getbalance","params":["tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf"],"id":1}'
```

### 4. Mine Blocks to Unlock Genesis Reward
```bash
# Mine 6 blocks to make genesis reward spendable
python3 mine_testnet_complete.py
```

After mining 6 blocks, the genesis reward (50 GXC) will be spendable.

## Third-Party Pool Integration

Your third-party pool can connect to the node via RPC:

### Pool Configuration
```python
RPC_URL = "http://your-node-ip:8332"
POOL_ADDRESS = "your_pool_address"

# Get block template
template = rpc_call("getblocktemplate", [POOL_ADDRESS])

# Submit mined block
result = rpc_call("submitblock", [block_data])
```

### Mining Rewards
- Block reward: 50 GXC (halves every 1,051,200 blocks)
- Transaction fees: Added to block reward
- Rewards go to the miner address specified in the block template

## Testing Checklist

- [x] Genesis block automatically created
- [x] Work receipt generated for genesis
- [x] UTXO created with correct address
- [x] UTXO key format fixed (`_` separator)
- [x] Coinbase maturity reduced for testnet (6 blocks)
- [ ] Balance shows correctly after 6 confirmations
- [ ] Mining works with third-party pool
- [ ] Transactions can be sent
- [ ] Staking works

## Known Issues

1. **Balance shows 0 initially**: This is correct - coinbase rewards need 6 confirmations (testnet) before they're spendable
2. **UTXO lookup**: There may still be an issue with address matching in getBalance() - needs further investigation

## Next Steps

1. Test mining 6+ blocks to verify balance updates
2. Test third-party pool connection
3. Test transaction sending
4. Test staking functionality

## Files Modified

1. `src/Blockchain.cpp` - Genesis creation, UTXO format, coinbase maturity
2. `config/railway_config.py` - Default miner address
3. `mine_testnet_complete.py` - Default miner address
4. `mine_testnet.py` - Default miner address

## Rebuild Instructions

```bash
cd /workspaces/GXC-goldxcoin-blockchain-
cmake .
make -j$(nproc)
```

---

**Status**: Genesis block creation working, balance lookup needs verification
**Date**: 2026-01-06
**Network**: Testnet
