# Testing Instructions: Mining, Staking, Restaking, and Sending Coins

## Quick Start

1. **Start the node:**
   ```bash
   ./gxc-node
   ```
   Or run in background:
   ```bash
   ./gxc-node > node.log 2>&1 &
   ```

2. **Wait for node to be ready** (about 5-10 seconds)

3. **Run the test script:**
   ```bash
   bash test_all_operations.sh
   ```

## Manual Testing

### 1. Check Node Status
```bash
curl -X POST http://localhost:8080/rpc \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getblockchaininfo","params":[],"id":1}'
```

### 2. Get Wallet Address
```bash
curl -X POST http://localhost:8080/rpc \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getwalletaddress","params":[],"id":1}'
```

### 3. Check Balance
```bash
# Replace YOUR_ADDRESS with your wallet address
curl -X POST http://localhost:8080/rpc \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getbalance","params":["YOUR_ADDRESS"],"id":1}'
```

### 4. Mine Blocks
```bash
# Get block template
curl -X POST http://localhost:8080/rpc \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getblocktemplate","params":[{"miner_address":"YOUR_ADDRESS"}],"id":1}'

# Submit block (after solving PoW)
curl -X POST http://localhost:8080/rpc \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"submitblock","params":[BLOCK_DATA],"id":1}'
```

### 5. Stake Coins
```bash
# Without private key (uses node wallet)
curl -X POST http://localhost:8080/rpc \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"registervalidator","params":["YOUR_ADDRESS", 100.0, 30],"id":1}'

# With private key (for third-party wallets)
curl -X POST http://localhost:8080/rpc \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"registervalidator","params":["YOUR_ADDRESS", 100.0, 30, "YOUR_PRIVATE_KEY"],"id":1}'
```

### 6. Restake (Add More Stake)
```bash
# Without private key
curl -X POST http://localhost:8080/rpc \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"addstake","params":["YOUR_ADDRESS", 50.0],"id":1}'

# With private key
curl -X POST http://localhost:8080/rpc \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"addstake","params":["YOUR_ADDRESS", 50.0, "YOUR_PRIVATE_KEY"],"id":1}'
```

### 7. Send Coins
```bash
# Send to target address: tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf
# Without private key (uses node wallet)
curl -X POST http://localhost:8080/rpc \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"sendtoaddress","params":["tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf", 50.0],"id":1}'

# With private key (for third-party wallets)
curl -X POST http://localhost:8080/rpc \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"sendtoaddress","params":["tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf", 50.0, "YOUR_PRIVATE_KEY"],"id":1}'
```

## Expected Results

1. **Mining**: Balance should increase after blocks are mined
2. **Staking**: Validator should be registered, stake transaction created
3. **Restaking**: Additional stake should be added to existing validator
4. **Sending**: Transaction should be created and recipient balance should increase

## Troubleshooting

- **"Cannot connect to node"**: Make sure `./gxc-node` is running
- **"Insufficient funds"**: Mine more blocks or wait for balance to update
- **"Wallet does not control address"**: Provide the correct private key for that address
- **"Invalid address format"**: Ensure address starts with `tGXC` (testnet) or `GXC` (mainnet)
