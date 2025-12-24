# GXC Wallet - Complete Guide

## Overview

The GXC Wallet is a comprehensive command-line tool for managing your GXC coins, testing all blockchain functions, and interacting with the GXC network.

---

## Installation

The wallet is a Python script that requires Python 3.6+ and the `requests` library.

```bash
# Install dependencies
pip3 install requests

# Make wallet executable
chmod +x gxc-wallet.py
```

---

## Quick Start

### 1. Start the GXC Node

```bash
./gxc-node --testnet
```

### 2. Create Your First Address

```bash
./gxc-wallet.py create --label "My First Address"
```

### 3. Check Your Balance

```bash
./gxc-wallet.py balance
```

---

## Complete Command Reference

### Wallet Management

#### Create New Address

```bash
./gxc-wallet.py create --label "My Address"
```

**Output**:
```
✓ New address created:
  Address: GXC9d7d694f52513d922fb238720376600dd8
  Label: My Address
```

#### Import Existing Address

```bash
./gxc-wallet.py import tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf --label "Mining Address"
```

**Output**:
```
✓ Address imported:
  Address: tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf
  Label: Mining Address
```

#### List All Addresses

```bash
./gxc-wallet.py list
```

**Output**:
```
Wallet Addresses:
================================================================================

1. GXC9d7d694f52513d922fb238720376600dd8
   Label: My First Address
   Balance: 0.000000 GXC
   Created: 2025-12-24T11:20:00

2. tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf
   Label: Mining Address
   Balance: 200.000000 GXC
   Imported: 2025-12-24T11:21:00
```

---

### Balance Operations

#### Check Balance for Specific Address

```bash
./gxc-wallet.py balance tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf
```

**Output**:
```
Balance:
  Address: tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf
  Balance: 200.000000 GXC
```

#### Check Balance for All Addresses

```bash
./gxc-wallet.py balance
```

**Output**:
```
All Balances:
================================================================================
  My First Address: 0.000000 GXC
  Mining Address: 200.000000 GXC
================================================================================
  Total: 200.000000 GXC
```

---

### Sending Transactions

#### Send GXC to Another Address

```bash
./gxc-wallet.py send FROM_ADDRESS TO_ADDRESS AMOUNT
```

**Example**:
```bash
./gxc-wallet.py send tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf GXC9d7d694f52513d922fb238720376600dd8 10.0
```

**Output**:
```
Sending Transaction:
  From: tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf
  To: GXC9d7d694f52513d922fb238720376600dd8
  Amount: 10.0 GXC

Confirm? (yes/no): yes

✓ Transaction sent!
  TX Hash: abc123def456...
```

---

### Staking Operations

#### Register as Validator / Stake Coins

```bash
./gxc-wallet.py stake ADDRESS AMOUNT DAYS
```

**Example**:
```bash
./gxc-wallet.py stake tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf 100.0 365
```

**Parameters**:
- `ADDRESS`: Your validator address
- `AMOUNT`: Amount to stake (minimum 10,000 GXC for validators)
- `DAYS`: Lock period in days

**Output**:
```
Staking:
  Address: tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf
  Amount: 100.0 GXC
  Days: 365

Confirm staking? (yes/no): yes

✓ Staking successful!
```

#### Get Staking Information

```bash
./gxc-wallet.py staking ADDRESS
```

**Example**:
```bash
./gxc-wallet.py staking tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf
```

**Output**:
```
Staking Info:
  Address: tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf
{
  "staked_amount": 100.0,
  "lock_period": 365,
  "rewards": 5.5,
  "status": "active"
}
```

---

### Transaction History

#### Get Transaction Details

```bash
./gxc-wallet.py tx TRANSACTION_HASH
```

**Example**:
```bash
./gxc-wallet.py tx abc123def456...
```

**Output**:
```
Transaction Details:
{
  "txid": "abc123def456...",
  "amount": 10.0,
  "fee": 0.0001,
  "confirmations": 6,
  "time": 1703419200,
  "from": "tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf",
  "to": "GXC9d7d694f52513d922fb238720376600dd8"
}
```

---

### Node Information

#### Get Node Status

```bash
./gxc-wallet.py info
```

**Output**:
```
Node Information:
================================================================================

Blockchain:
  Height: 4
  Difficulty: 0.1
  Best Block: 0fa191e57184ad11...

Mining:
  Network Hashrate: 1000000.0
  Pooled Transactions: 0

Mempool:
  Pending Transactions: 0
  Memory Usage: 0 bytes
```

---

## Advanced Usage

### Custom Node URL

```bash
./gxc-wallet.py --node http://remote-node:8332 balance
```

### Environment Variable

```bash
export GXC_NODE_URL=http://remote-node:8332
./gxc-wallet.py balance
```

---

## Wallet File

The wallet stores addresses in `~/.gxc_wallet.json`

**Structure**:
```json
{
  "addresses": {
    "GXC9d7d694f52513d922fb238720376600dd8": {
      "label": "My First Address",
      "created": "2025-12-24T11:20:00"
    },
    "tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf": {
      "label": "Mining Address",
      "imported": "2025-12-24T11:21:00"
    }
  },
  "created": "2025-12-24T11:20:00"
}
```

**Note**: This file only stores addresses and labels, NOT private keys. Private keys are managed by the node.

---

## Complete Testing Workflow

### 1. Setup

```bash
# Start node
./gxc-node --testnet

# Create wallet
./gxc-wallet.py create --label "Test Wallet"
```

### 2. Mining

```bash
# Import mining address
./gxc-wallet.py import tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf --label "Miner"

# Mine blocks (use separate mining script)
# After mining, check balance
./gxc-wallet.py balance
```

### 3. Sending

```bash
# Create recipient address
./gxc-wallet.py create --label "Recipient"

# Send transaction
./gxc-wallet.py send MINER_ADDRESS RECIPIENT_ADDRESS 10.0

# Check balances
./gxc-wallet.py balance
```

### 4. Staking

```bash
# Stake coins
./gxc-wallet.py stake MINER_ADDRESS 100.0 365

# Check staking info
./gxc-wallet.py staking MINER_ADDRESS
```

### 5. Monitoring

```bash
# Check node status
./gxc-wallet.py info

# List all addresses
./gxc-wallet.py list

# Check transaction
./gxc-wallet.py tx TRANSACTION_HASH
```

---

## Features

### ✅ Wallet Management
- Create new addresses
- Import existing addresses
- Label addresses for easy identification
- Persistent storage in JSON file

### ✅ Balance Operations
- Check individual address balance
- Check total wallet balance
- Real-time balance updates

### ✅ Transactions
- Send GXC to any address
- Transaction confirmation prompts
- Transaction hash tracking

### ✅ Staking
- Register as validator
- Stake coins with lock period
- Check staking status and rewards

### ✅ Monitoring
- Node information
- Blockchain status
- Mempool statistics
- Transaction history

### ✅ User-Friendly
- Colored output
- Clear error messages
- Confirmation prompts
- Comprehensive help

---

## Troubleshooting

### Cannot Connect to Node

**Error**: `Cannot connect to node. Is it running?`

**Solution**:
```bash
# Check if node is running
ps aux | grep gxc-node

# Start node if not running
./gxc-node --testnet

# Check node URL
./gxc-wallet.py --node http://localhost:8332 info
```

### Insufficient Funds

**Error**: `Insufficient funds`

**Solution**:
- Mine blocks to earn GXC
- Check balance: `./gxc-wallet.py balance`
- Ensure you're sending from the correct address

### Invalid Address

**Error**: `Invalid address`

**Solution**:
- Check address format (starts with GXC or tGXC for testnet)
- Verify address with: `./gxc-wallet.py list`

---

## Security Notes

1. **Wallet File**: The `~/.gxc_wallet.json` file only stores addresses and labels, NOT private keys
2. **Private Keys**: Private keys are managed by the node
3. **Confirmations**: Always wait for confirmations before considering transactions final
4. **Backups**: Backup your wallet file regularly
5. **Testnet**: Use testnet for testing, mainnet for production

---

## Examples

### Complete Example Session

```bash
# 1. Start node
./gxc-node --testnet

# 2. Create wallet
./gxc-wallet.py create --label "Main Wallet"
# Output: GXC9d7d694f52513d922fb238720376600dd8

# 3. Import mining address
./gxc-wallet.py import tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf --label "Miner"

# 4. Check balances
./gxc-wallet.py balance
# Output: Miner: 200.000000 GXC

# 5. Send transaction
./gxc-wallet.py send tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf GXC9d7d694f52513d922fb238720376600dd8 50.0
# Confirm: yes
# Output: TX Hash: abc123...

# 6. Check balances again
./gxc-wallet.py balance
# Output: 
#   Miner: 150.000000 GXC
#   Main Wallet: 50.000000 GXC

# 7. Stake coins
./gxc-wallet.py stake tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf 100.0 365
# Confirm: yes

# 8. Check staking
./gxc-wallet.py staking tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf

# 9. Check node info
./gxc-wallet.py info
```

---

## API Reference

All commands use the GXC RPC API:

| Wallet Command | RPC Method | Description |
|----------------|------------|-------------|
| `create` | `getnewaddress` | Generate new address |
| `import` | `validateaddress` | Validate and import address |
| `balance` | `getbalance` | Get address balance |
| `send` | `sendtoaddress` | Send transaction |
| `stake` | `registervalidator` | Register validator/stake |
| `staking` | `getstakinginfo` | Get staking information |
| `tx` | `gettransaction` | Get transaction details |
| `info` | `getblockchaininfo`, `getmininginfo`, `getmempoolinfo` | Get node info |

---

## Support

For issues or questions:
- Check node logs: `/tmp/gxc-node.log`
- Verify node is running: `ps aux | grep gxc-node`
- Check wallet file: `cat ~/.gxc_wallet.json`
- Test connection: `./gxc-wallet.py info`

---

**Version**: 1.0.0  
**Status**: Production Ready  
**License**: MIT
