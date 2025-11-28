# How to Get Coins to Your Wallet - Complete Guide

## Overview

GXC coins are automatically credited to your wallet when you successfully mine blocks. The rewards come from **coinbase transactions** that are included in each block you mine.

---

## How Mining Rewards Work

### 1. **Mining Process**

When you mine a block:

1. **You find a valid block** - Your miner solves the cryptographic puzzle
2. **Block is constructed** - The miner creates a block with:
   - Your wallet address in the coinbase transaction
   - Block reward amount (from blockchain)
   - All pending transactions
3. **Block is submitted** - The block is sent to the blockchain node
4. **Blockchain validates** - The node validates and adds the block to the chain
5. **Rewards credited** - The coinbase transaction credits your address automatically

### 2. **Coinbase Transaction**

Every block includes a special **coinbase transaction** that:
- Has **no inputs** (it's new coins being created)
- Has **one output** to the miner's address
- Contains the **block reward** (e.g., 12.5 GXC)
- Is automatically included when you submit a block

**Example:**
```json
{
  "type": "coinbase",
  "inputs": [],
  "outputs": [{
    "address": "GXC...your_address...",
    "amount": 12.5
  }]
}
```

---

## Step-by-Step: Getting Coins

### Step 1: Create a Wallet

If you don't have a wallet yet:

1. **Via Wallet Service:**
   ```bash
   # Start wallet service
   python api/wallet_service.py
   ```

2. **Create wallet via web interface:**
   - Go to: `http://localhost:5000/register`
   - Create account and wallet
   - **Save your seed phrase and private key!**

3. **Get your wallet address:**
   - After creation, you'll see your GXC address (starts with `GXC...`)
   - Copy this address - you'll need it for mining

### Step 2: Start Mining

1. **Start the blockchain node:**
   ```bash
   # Make sure blockchain node is running
   ./build/Release/gxc-node.exe --rpc-port=8545 --rest-port=8080
   ```

2. **Start the miner:**
   ```bash
   # Option 1: GUI Miner
   python mining/gxhash_miner.py
   
   # Option 2: Command-line miner
   python mining/gxhash_miner.py --rpc-url http://localhost:8545 --address GXC...your_address...
   ```

3. **Configure miner:**
   - Enter RPC endpoint: `http://localhost:8545`
   - Enter your wallet address: `GXC...your_address...`
   - Click "Connect"
   - Click "START MINING"

### Step 3: Mine Blocks

- The miner will automatically:
  - Get block templates from blockchain
  - Mine using GXHash algorithm
  - Submit valid blocks to blockchain
  - Include your address in coinbase transaction

### Step 4: Check Your Balance

#### Method 1: Via Wallet Service Web Interface

1. **Login to wallet:**
   - Go to: `http://localhost:5000/login`
   - Enter your credentials

2. **View balance:**
   - Your balance is automatically displayed on the dashboard
   - The wallet service queries the blockchain for real balance

#### Method 2: Via REST API

```bash
# Get balance for your address
curl http://localhost:8080/api/v1/address/GXC...your_address.../balance
```

Response:
```json
{
  "address": "GXC...your_address...",
  "balance": 12.5,
  "confirmed": 12.5,
  "unconfirmed": 0.0
}
```

#### Method 3: Via RPC

```bash
curl -X POST http://localhost:8545 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "getbalance",
    "params": ["GXC...your_address..."],
    "id": 1
  }'
```

#### Method 4: Via Python Script

```python
from api.wallet_service import BlockchainClient

client = BlockchainClient()
balance = client.get_address_balance("GXC...your_address...")
print(f"Balance: {balance} GXC")
```

### Step 5: View Transactions

#### Via Wallet Service:

1. Login to wallet dashboard
2. Click on "Transactions" tab
3. See all incoming (mining rewards) and outgoing transactions

#### Via REST API:

```bash
# Get transactions for your address
curl http://localhost:8080/api/v1/address/GXC...your_address.../transactions
```

Response:
```json
[
  {
    "hash": "0x...",
    "type": "coinbase",
    "from": null,
    "to": "GXC...your_address...",
    "amount": 12.5,
    "timestamp": 1234567890,
    "confirmations": 6
  }
]
```

---

## Understanding Your Balance

### Balance Components

Your wallet balance consists of:

1. **Confirmed Balance:**
   - Transactions with 6+ confirmations
   - Safe to spend
   - Shown as main balance

2. **Unconfirmed Balance:**
   - Recent transactions (0-5 confirmations)
   - Not yet safe to spend
   - Will become confirmed after 6 blocks

### How Balance is Calculated

The blockchain calculates your balance by:

1. **Finding all UTXOs** (Unspent Transaction Outputs) for your address
2. **Summing the amounts** of all UTXOs
3. **Subtracting spent outputs** from transactions you sent

**Formula:**
```
Balance = Sum of all UTXOs for your address
```

---

## Mining Rewards Breakdown

### Block Rewards

- **Initial Reward:** 12.5 GXC per block (configurable)
- **Halving:** Every 1,051,200 blocks (~4 years)
- **Transaction Fees:** Additional rewards from fees in block

### Reward Distribution

When you mine a block:

1. **Block Reward:** Goes to your address (coinbase)
2. **Transaction Fees:** Also included in coinbase
3. **Total Reward:** Block reward + fees

**Example:**
- Block reward: 12.5 GXC
- Transaction fees: 0.5 GXC
- **Total received:** 13.0 GXC

---

## Troubleshooting

### "My balance is still 0 after mining"

**Check:**
1. ✅ Is the blockchain node running?
2. ✅ Did you enter the correct wallet address in miner?
3. ✅ Was the block actually accepted by blockchain?
4. ✅ Check miner logs for "Block submitted and accepted"
5. ✅ Wait for confirmations (6 blocks = confirmed)

**Verify block was mined:**
```bash
# Check latest blocks
curl http://localhost:8080/api/v1/blocks?limit=10
```

### "Balance not updating in wallet"

**Solutions:**
1. Refresh the wallet dashboard
2. The wallet service queries blockchain every time you view balance
3. Check blockchain directly via REST API
4. Clear browser cache if needed

### "Transactions not showing"

**Check:**
1. Verify address is correct
2. Check blockchain explorer: `http://localhost:3001/address/GXC...`
3. Transactions may take a few seconds to appear
4. Check if blockchain node is synced

---

## Alternative: Mining Pool Rewards

If you're mining in a pool instead of solo:

1. **Register with pool:**
   - Go to pool website
   - Register with your wallet address

2. **Mine to pool:**
   - Connect miner to pool
   - Mine shares for the pool

3. **Receive payouts:**
   - Pool accumulates your shares
   - When pool finds block, rewards distributed
   - Payouts sent to your wallet address automatically

4. **Check pool dashboard:**
   - View your pending balance
   - See payout history
   - Track your hash contribution

---

## Quick Reference

### Get Balance
```bash
# REST API
curl http://localhost:8080/api/v1/address/GXC.../balance

# RPC
curl -X POST http://localhost:8545 -d '{"jsonrpc":"2.0","method":"getbalance","params":["GXC..."],"id":1}'
```

### Get Transactions
```bash
# REST API
curl http://localhost:8080/api/v1/address/GXC.../transactions
```

### Check if Block Was Mined
```bash
# Get latest blocks
curl http://localhost:8080/api/v1/blocks?limit=5
```

### View in Explorer
```
http://localhost:3001/address/GXC...your_address...
```

---

## Summary

**To get coins to your wallet:**

1. ✅ Create a wallet and get your address
2. ✅ Start blockchain node
3. ✅ Start miner with your address
4. ✅ Mine blocks successfully
5. ✅ Blocks include coinbase transaction to your address
6. ✅ Check balance via wallet service or API
7. ✅ Coins appear automatically - no manual transfer needed!

**Key Points:**
- Rewards come from **coinbase transactions** in blocks you mine
- No manual transfers needed - blockchain handles it automatically
- Balance updates automatically when you query it
- Wait for 6 confirmations for confirmed balance

---

## Need Help?

- **Wallet Service:** `http://localhost:5000`
- **Blockchain Explorer:** `http://localhost:3001`
- **REST API Docs:** `http://localhost:8080/api/v1`
- **Mining Pool:** Check pool dashboard for payouts

