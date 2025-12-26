# GXC Blockchain API Usage Guide

Complete guide for using the GXC blockchain APIs to mine, send transactions, register validators, and stake coins.

## Table of Contents

1. [Starting the Node](#starting-the-node)
2. [Mining Blocks](#mining-blocks)
3. [Sending Transactions](#sending-transactions)
4. [Validator Registration](#validator-registration)
5. [Staking Operations](#staking-operations)
6. [Unstaking Operations](#unstaking-operations)
7. [Complete Example](#complete-example)

---

## Starting the Node

### Start with RPC API

```bash
./gxc-node --testnet --rpc-port=8545 --rest-port=8080
```

### Verify Node is Running

```bash
curl -X POST http://localhost:8545 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "getblockchaininfo",
    "params": [],
    "id": 1
  }'
```

---

## Mining Blocks

### Using RPC API

#### Get New Address

```bash
curl -X POST http://localhost:8545 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "getnewaddress",
    "params": [],
    "id": 1
  }'
```

Response:
```json
{
  "jsonrpc": "2.0",
  "result": "tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9",
  "id": 1
}
```

#### Mine Blocks

```bash
curl -X POST http://localhost:8545 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "generatetoaddress",
    "params": [10, "tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9"],
    "id": 1
  }'
```

#### Check Balance

```bash
curl -X POST http://localhost:8545 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "getbalance",
    "params": ["tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9"],
    "id": 1
  }'
```

---

## Sending Transactions

### Method 1: Using sendtoaddress (Simple)

```bash
curl -X POST http://localhost:8545 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "sendtoaddress",
    "params": [
      "tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf",
      10.0
    ],
    "id": 1
  }'
```

### Method 2: Using sendrawtransaction (Advanced)

For third-party wallets that create their own transactions:

```bash
curl -X POST http://localhost:8545 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "sendrawtransaction",
    "params": [{
      "inputs": [{
        "txHash": "62649008ae4f64566c5850b39569a1c7bb4b3e8a058c781d63c0e52f4890be4f",
        "outputIndex": 0,
        "amount": 50.0,
        "signature": "your_signature_here",
        "publicKey": "your_public_key_here"
      }],
      "outputs": [{
        "address": "tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf",
        "amount": 49.9,
        "script": ""
      }],
      "fee": 0.0001,
      "senderAddress": "tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9",
      "receiverAddress": "tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf",
      "type": "NORMAL"
    }],
    "id": 1
  }'
```

**Note:** With the third-party wallet fix, you don't need to provide `prevTxHash` or `referencedAmount` - they will be automatically derived from the inputs!

### Confirm Transaction

Mine a block to confirm:

```bash
curl -X POST http://localhost:8545 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "generatetoaddress",
    "params": [1, "tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9"],
    "id": 1
  }'
```

---

## Validator Registration

### Register as Validator

```bash
curl -X POST http://localhost:8545 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "registervalidator",
    "params": [
      "tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9",
      50.0
    ],
    "id": 1
  }'
```

Parameters:
- `address`: Your wallet address
- `stake_amount`: Initial stake (minimum 50 GXC)

### Confirm Registration

```bash
curl -X POST http://localhost:8545 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "generatetoaddress",
    "params": [1, "tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9"],
    "id": 1
  }'
```

### Check Validator Info

```bash
curl -X POST http://localhost:8545 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "getvalidatorinfo",
    "params": ["tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9"],
    "id": 1
  }'
```

Response:
```json
{
  "jsonrpc": "2.0",
  "result": {
    "address": "tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9",
    "stake": 50.0,
    "status": "ACTIVE",
    "blocks_validated": 0
  },
  "id": 1
}
```

---

## Staking Operations

### Stake Additional Coins

```bash
curl -X POST http://localhost:8545 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "stake",
    "params": [
      "tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9",
      25.0
    ],
    "id": 1
  }'
```

### Confirm Stake

```bash
curl -X POST http://localhost:8545 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "generatetoaddress",
    "params": [1, "tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9"],
    "id": 1
  }'
```

### Check Updated Stake

```bash
curl -X POST http://localhost:8545 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "getvalidatorinfo",
    "params": ["tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9"],
    "id": 1
  }'
```

---

## Unstaking Operations

### Unstake Coins

```bash
curl -X POST http://localhost:8545 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "unstake",
    "params": [
      "tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9",
      20.0
    ],
    "id": 1
  }'
```

### Confirm Unstake

```bash
curl -X POST http://localhost:8545 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "generatetoaddress",
    "params": [1, "tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9"],
    "id": 1
  }'
```

### Check Final Balance

```bash
curl -X POST http://localhost:8545 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "getbalance",
    "params": ["tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9"],
    "id": 1
  }'
```

---

## Complete Example

Here's a complete Python script that demonstrates all operations:

```python
#!/usr/bin/env python3
import requests
import json
import time

NODE_URL = "http://localhost:8545"

def rpc_call(method, params=[]):
    payload = {
        "jsonrpc": "2.0",
        "method": method,
        "params": params,
        "id": 1
    }
    response = requests.post(NODE_URL, json=payload)
    return response.json().get("result")

# 1. Get wallet address
address = rpc_call("getnewaddress")
print(f"Wallet: {address}")

# 2. Mine blocks
print("Mining 10 blocks...")
for i in range(10):
    rpc_call("generatetoaddress", [1, address])
    time.sleep(0.5)

# 3. Check balance
balance = rpc_call("getbalance", [address])
print(f"Balance: {balance} GXC")

# 4. Send transaction
recipient = "tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf"
tx_hash = rpc_call("sendtoaddress", [recipient, 10.0])
print(f"Transaction: {tx_hash}")

# 5. Confirm transaction
rpc_call("generatetoaddress", [1, address])

# 6. Register validator
result = rpc_call("registervalidator", [address, 50.0])
print(f"Validator registered: {result}")

# 7. Confirm registration
rpc_call("generatetoaddress", [1, address])

# 8. Stake more
tx_hash = rpc_call("stake", [address, 25.0])
print(f"Staked: {tx_hash}")

# 9. Confirm stake
rpc_call("generatetoaddress", [1, address])

# 10. Unstake
tx_hash = rpc_call("unstake", [address, 20.0])
print(f"Unstaked: {tx_hash}")

# 11. Confirm unstake
rpc_call("generatetoaddress", [1, address])

# 12. Final status
validator_info = rpc_call("getvalidatorinfo", [address])
print(f"Final stake: {validator_info['stake']} GXC")

balance = rpc_call("getbalance", [address])
print(f"Final balance: {balance} GXC")
```

---

## Available RPC Methods

### Blockchain Info
- `getblockchaininfo` - Get blockchain status
- `getblockcount` - Get current height
- `getdifficulty` - Get current difficulty

### Wallet Operations
- `getnewaddress` - Create new address
- `getbalance` - Check balance
- `listunspent` - List UTXOs

### Mining
- `generatetoaddress` - Mine blocks
- `getmininginfo` - Get mining stats

### Transactions
- `sendtoaddress` - Send coins (simple)
- `sendrawtransaction` - Send raw transaction (advanced)
- `gettransaction` - Get transaction details
- `estimatefee` - Estimate transaction fee

### Validators & Staking
- `registervalidator` - Register as validator
- `getvalidatorinfo` - Get validator details
- `listvalidators` - List all validators
- `stake` - Stake coins
- `unstake` - Unstake coins

### Network
- `getpeerinfo` - Get peer information
- `getnetworkinfo` - Get network status

---

## Error Handling

### Common Errors

**Insufficient Balance:**
```json
{
  "error": {
    "code": -6,
    "message": "Insufficient funds"
  }
}
```

**Transaction Validation Failed:**
```json
{
  "error": {
    "code": -26,
    "message": "Transaction verification failed"
  }
}
```

**Invalid Address:**
```json
{
  "error": {
    "code": -5,
    "message": "Invalid address"
  }
}
```

---

## Best Practices

1. **Always confirm transactions** by mining a block after sending
2. **Check balances** before sending transactions
3. **Use appropriate fees** - call `estimatefee` first
4. **For third-party wallets** - use `sendrawtransaction` with proper signatures
5. **Monitor validator status** regularly if running a validator node

---

## Testing

Run the complete test suite:

```bash
python3 test_complete_flow.py
```

Or use the demo:

```bash
python3 demo_complete_flow.py
```

---

## Support

For issues or questions:
- Check the logs: `tail -f node.log`
- Review transaction details: `gettransaction <tx_hash>`
- Verify blockchain state: `getblockchaininfo`

---

## Third-Party Wallet Integration

See [THIRD_PARTY_WALLET_FIX.md](THIRD_PARTY_WALLET_FIX.md) for details on integrating external wallets.

Key points:
- No need to set `prevTxHash` - automatically derived
- No need to set `referencedAmount` - automatically derived
- Just provide inputs, outputs, and signatures
- Blockchain handles traceability automatically
