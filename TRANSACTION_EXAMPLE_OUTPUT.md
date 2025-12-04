# Transaction Example Output

## What a GXC Transaction Looks Like

Based on the code structure, here's what a transaction response should look like:

### 1. Send Transaction Response

When you call `sendtoaddress`, you get back a transaction hash:

```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": "a1b2c3d4e5f6...64_char_hex_hash"
}
```

### 2. Get Transaction Details

When you call `gettransaction` with the hash, you get:

```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": {
    "hash": "a1b2c3d4e5f6...64_char_hex_hash",
    "type": "NORMAL",
    "from": "tGXC...sender_address",
    "to": "tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf",
    "amount": 50.0,
    "fee": 0.001,
    "confirmations": 1,
    "blockhash": "block_hash_if_confirmed",
    "blocktime": 1234567890,
    "time": 1234567890,
    "inputs": [
      {
        "txHash": "previous_tx_hash",
        "outputIndex": 0,
        "amount": 100.0
      }
    ],
    "outputs": [
      {
        "address": "tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf",
        "amount": 50.0
      },
      {
        "address": "tGXC...sender_address",
        "amount": 49.999
      }
    ],
    "prevTxHash": "previous_transaction_hash",
    "referencedAmount": 100.0,
    "senderAddress": "tGXC...sender_address",
    "receiverAddress": "tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf",
    "isCoinbase": false,
    "isGoldBacked": false,
    "memo": "",
    "traceability": {
      "prevTxHash": "previous_transaction_hash",
      "referencedAmount": 100.0,
      "valid": true
    }
  }
}
```

### 3. Transaction Structure Explained

**Key Fields:**
- `hash`: Unique transaction identifier (64 hex characters)
- `type`: Transaction type (NORMAL, STAKE, UNSTAKE, etc.)
- `from` / `senderAddress`: Address sending the coins
- `to` / `receiverAddress`: Address receiving the coins
- `amount`: Amount being sent (in GXC)
- `fee`: Transaction fee (default 0.001 GXC)
- `confirmations`: Number of blocks confirming this transaction
- `inputs`: UTXOs being spent (previous transaction outputs)
- `outputs`: New UTXOs being created (recipient + change)
- `prevTxHash`: Link to previous transaction (for traceability)
- `referencedAmount`: Amount from previous transaction

### 4. Example: Sending 50 GXC

**Input:**
```bash
curl -X POST http://localhost:8332/rpc \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "sendtoaddress",
    "params": ["tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf", 50.0],
    "id": 1
  }'
```

**Output:**
```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": "7f3a8b2c9d1e4f5a6b7c8d9e0f1a2b3c4d5e6f7a8b9c0d1e2f3a4b5c6d7e8f9a0"
}
```

**Then get details:**
```bash
curl -X POST http://localhost:8332/rpc \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "gettransaction",
    "params": ["7f3a8b2c9d1e4f5a6b7c8d9e0f1a2b3c4d5e6f7a8b9c0d1e2f3a4b5c6d7e8f9a0"],
    "id": 1
  }'
```

### 5. Transaction Flow

1. **Create Transaction**: Wallet finds UTXOs, creates inputs/outputs
2. **Sign Transaction**: Private key signs the transaction
3. **Broadcast**: Transaction sent to network
4. **Mining**: Miners include transaction in next block
5. **Confirmation**: Block is added to blockchain

### 6. Traceability

Every transaction links to the previous one:
```
Genesis → TX1 → TX2 → TX3 → Your Transaction
```

The `prevTxHash` field creates this chain, allowing full traceability back to genesis.

### 7. Testnet vs Mainnet

- **Testnet addresses**: Start with `tGXC` (34 characters)
- **Mainnet addresses**: Start with `GXC` (33 characters)
- **Transaction structure**: Same for both networks

### 8. Current Issue

The node is currently returning `"transaction_hash"` as a literal string instead of the actual hash. This suggests:
- Transaction creation might be failing silently
- Need to check node logs for errors
- May need to mine actual blocks first to have UTXOs to spend

### 9. How to Fix

1. **Mine real blocks**: Solve PoW to create coinbase transactions
2. **Wait for confirmations**: Let blocks be added to chain
3. **Check UTXO set**: Verify you have spendable outputs
4. **Then send**: Transaction will use those UTXOs
