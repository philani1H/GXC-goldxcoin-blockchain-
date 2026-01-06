# Genesis Address Update - Testnet

## Changes Made

Updated the testnet genesis block address to ensure mining rewards go to the correct address.

### Previous Address
```
tGXC9fab7317231b966af85ac453e168c0932
```

### New Address (Your Address)
```
tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf
```

## Files Updated

1. **src/Blockchain.cpp** - Genesis block creation
   - Line 180: Updated testnet genesis address

2. **config/railway_config.py** - Default miner configuration
   - Line 55: Updated DEFAULT_MINER_ADDRESS for testnet

3. **mine_testnet_complete.py** - Complete testnet miner
   - Line 16: Updated MINER_ADDRESS default

4. **mine_testnet.py** - Simple testnet miner
   - Line 12: Updated MINER_ADDRESS

## What This Fixes

### Genesis Block
The genesis block (block 0) is the first block in the blockchain. It contains the initial coin distribution. By updating the genesis address to your address, you will receive:
- **50 GXC** from the genesis block (block 0)
- All subsequent mining rewards when you mine blocks

### Mining Rewards
When you mine blocks (either solo or through a pool), the coinbase transaction will credit rewards to your address. The reward system works as follows:

1. **Block Reward**: 50 GXC initially, halving every 1,051,200 blocks (~4 years)
2. **Transaction Fees**: Additional fees from transactions in the block
3. **Difficulty Bonus**: Up to 10% bonus for higher difficulty blocks

### Pool vs Solo Mining

**Solo Mining:**
- You receive 100% of the block reward when you find a block
- Rewards go directly to your miner address
- More variable (feast or famine)

**Pool Mining:**
- Rewards are distributed proportionally based on your contributed shares
- More consistent payouts
- Pool takes a small fee (typically 2%)

## Next Steps

### 1. Clear Existing Blockchain Data (IMPORTANT)
Since the genesis block has changed, you need to clear any existing testnet blockchain data:

```bash
# Remove old blockchain data
rm -rf gxc_testnet_data/blocks/
rm -rf gxc_testnet_data/chainstate/
rm -f demo_gxc.db

# Keep wallet data (if you have existing wallets)
# The wallet directory is preserved
```

### 2. Rebuild Complete (Already Done)
The C++ node has been rebuilt with the new genesis address:
```bash
✅ gxc-node compiled successfully
```

### 3. Start Mining

**Option A: Solo Mining (Recommended for Testing)**
```bash
python3 mine_testnet_complete.py
```

**Option B: Pool Mining**
```bash
# Start the mining pool first
python3 mining_pool/general_pool.py

# Then connect miners to the pool
python3 mine_testnet_complete.py --pool
```

### 4. Check Your Balance
```bash
# Using Python
python3 -c "
import requests
response = requests.post('http://localhost:8332', json={
    'jsonrpc': '2.0',
    'method': 'getbalance',
    'params': ['tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf'],
    'id': 1
})
print(response.json())
"
```

## Why You Weren't Getting Rewards Before

The issue was that the genesis block and default miner address were set to a different address (`tGXC9fab7317231b966af85ac453e168c0932`). This meant:

1. The genesis block reward (50 GXC) went to the old address
2. If you were mining with the old default address, rewards went there instead of your address
3. The blockchain database may have had blocks with the old genesis, causing inconsistencies

## Verification

After mining a few blocks, verify your rewards:

```bash
# Check blockchain height
curl -X POST http://localhost:8332 -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getblockcount","params":[],"id":1}'

# Check your balance
curl -X POST http://localhost:8332 -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getbalance","params":["tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf"],"id":1}'

# Check recent blocks
curl -X POST http://localhost:8332 -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getblock","params":[0],"id":1}'
```

## Expected Results

After mining block 0 (genesis):
- Balance: 50 GXC
- Status: Pending (needs 6 confirmations)

After mining block 1:
- Balance: 100 GXC (50 from genesis + 50 from block 1)
- Genesis reward: Confirmed (1 confirmation)

After mining block 6:
- Balance: 350 GXC (7 blocks × 50 GXC)
- All rewards: Spendable (6+ confirmations)

## Troubleshooting

### Still Not Getting Rewards?

1. **Check you're using the correct address:**
   ```bash
   grep -r "tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf" mine_testnet*.py
   ```

2. **Verify the node is using the new genesis:**
   ```bash
   ./gxc-node --testnet &
   # Check logs for: "Using TESTNET genesis address: tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf"
   ```

3. **Ensure blockchain data was cleared:**
   ```bash
   ls -la gxc_testnet_data/
   # Should be empty or only contain wallet/
   ```

4. **Check mining is actually working:**
   ```bash
   # Watch the miner output for "Block accepted" messages
   python3 mine_testnet_complete.py
   ```

## Support

If you're still experiencing issues:
1. Check the node logs: `tail -f logs/blockchain.log`
2. Verify RPC connectivity: `curl http://localhost:8332`
3. Ensure no other process is using port 8332: `lsof -i :8332`

---

**Status**: ✅ Genesis address updated and node rebuilt
**Date**: 2026-01-06
**Network**: Testnet
