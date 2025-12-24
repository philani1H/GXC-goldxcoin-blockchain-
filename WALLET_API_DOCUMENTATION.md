# GXC Blockchain - Wallet API Documentation for Developers

## Overview

This document provides comprehensive documentation for integrating with the GXC blockchain wallet system. Developers can use these APIs to build wallets, exchanges, payment systems, and other blockchain applications.

---

## Table of Contents

1. [Getting Started](#getting-started)
2. [RPC API Reference](#rpc-api-reference)
3. [Wallet Operations](#wallet-operations)
4. [Transaction Operations](#transaction-operations)
5. [Staking Operations](#staking-operations)
6. [Query Operations](#query-operations)
7. [Code Examples](#code-examples)
8. [Error Handling](#error-handling)
9. [Best Practices](#best-practices)

---

## Getting Started

### Connection

Connect to the GXC node via JSON-RPC:

**Endpoint**: `http://localhost:8332` (default)  
**Method**: POST  
**Content-Type**: `application/json`

### Request Format

```json
{
  "jsonrpc": "2.0",
  "method": "METHOD_NAME",
  "params": [param1, param2, ...],
  "id": 1
}
```

### Response Format

```json
{
  "jsonrpc": "2.0",
  "result": {...},
  "id": 1
}
```

---

## RPC API Reference

### Wallet Operations

#### `getnewaddress`

Generate a new blockchain address.

**Parameters**: None

**Returns**: `string` - New address

**Example**:
```bash
curl -X POST http://localhost:8332 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "getnewaddress",
    "params": [],
    "id": 1
  }'
```

**Response**:
```json
{
  "jsonrpc": "2.0",
  "result": "tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9",
  "id": 1
}
```

---

#### `getbalance`

Get balance for an address.

**Parameters**:
- `address` (string): The address to query

**Returns**: `number` - Balance in GXC

**Example**:
```bash
curl -X POST http://localhost:8332 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "getbalance",
    "params": ["tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9"],
    "id": 1
  }'
```

**Response**:
```json
{
  "jsonrpc": "2.0",
  "result": 1450.0,
  "id": 1
}
```

---

### Transaction Operations

#### `gxc_sendFrom`

Send GXC from one address to another.

**Parameters**:
- `from` (string): Sender address
- `to` (string): Recipient address
- `amount` (number): Amount to send

**Returns**: `string` - Transaction hash

**Example**:
```bash
curl -X POST http://localhost:8332 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "gxc_sendFrom",
    "params": [
      "tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9",
      "tGXC4920b0633ef0f4f2ec13dfae91282cc127c",
      50.0
    ],
    "id": 1
  }'
```

**Response**:
```json
{
  "jsonrpc": "2.0",
  "result": "8d6fb3f952afd1988e4b4e3b7281a2b47dc9313c238c3d003b1091488e830dfb",
  "id": 1
}
```

---

#### `gettransaction`

Get transaction details.

**Parameters**:
- `txid` (string): Transaction hash

**Returns**: `object` - Transaction details

**Example**:
```bash
curl -X POST http://localhost:8332 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "gettransaction",
    "params": ["8d6fb3f952afd1988e4b4e3b7281a2b47dc9313c238c3d003b1091488e830dfb"],
    "id": 1
  }'
```

---

#### `getmempoolinfo`

Get mempool information (pending transactions).

**Parameters**: None

**Returns**: `object` - Mempool statistics

**Example**:
```bash
curl -X POST http://localhost:8332 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "getmempoolinfo",
    "params": [],
    "id": 1
  }'
```

**Response**:
```json
{
  "jsonrpc": "2.0",
  "result": {
    "size": 2,
    "bytes": 512,
    "usage": 1024,
    "maxmempool": 300000000,
    "mempoolminfee": 0.00001,
    "minrelaytxfee": 0.00001
  },
  "id": 1
}
```

---

### Staking Operations

#### `registervalidator`

Register as a validator and stake GXC.

**Parameters**:
- `address` (string): Validator address
- `stakeAmount` (number): Amount to stake (minimum 100 GXC)
- `stakingDays` (number): Staking duration in days (1-365)

**Returns**: `object` - Registration details

**Example**:
```bash
curl -X POST http://localhost:8332 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "registervalidator",
    "params": [
      "tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9",
      100.0,
      30
    ],
    "id": 1
  }'
```

**Response**:
```json
{
  "jsonrpc": "2.0",
  "result": {
    "success": true,
    "address": "tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9",
    "stake_amount": 100.0,
    "staking_days": 30,
    "weighted_stake": 3000.0,
    "stake_tx": "f47602d824dd73af166760d7240caefd1be9b12d15fab660116675adf32a1cec",
    "status": "pending",
    "message": "Validator registered successfully. Stake of 100.000000 GXC is pending confirmation."
  },
  "id": 1
}
```

---

#### `getstakinginfo`

Get staking information for an address.

**Parameters**:
- `address` (string): Address to query

**Returns**: `object` - Staking details

**Example**:
```bash
curl -X POST http://localhost:8332 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "getstakinginfo",
    "params": ["tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9"],
    "id": 1
  }'
```

**Response**:
```json
{
  "jsonrpc": "2.0",
  "result": {
    "address": "tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9",
    "is_validator": true,
    "staked_balance": 100.0,
    "spendable_balance": 1349.999,
    "total_balance": 1449.999,
    "total_earned": 1450.0,
    "total_earned_mining": 1450.0,
    "total_earned_staking": 0.0
  },
  "id": 1
}
```

---

#### `unstake`

Unstake GXC.

**Parameters**:
- `address` (string): Validator address
- `amount` (number): Amount to unstake

**Returns**: `object` - Unstaking details

**Example**:
```bash
curl -X POST http://localhost:8332 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "unstake",
    "params": [
      "tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9",
      50.0
    ],
    "id": 1
  }'
```

---

### Query Operations

#### `getblockchaininfo`

Get blockchain information.

**Parameters**: None

**Returns**: `object` - Blockchain stats

**Example**:
```bash
curl -X POST http://localhost:8332 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "getblockchaininfo",
    "params": [],
    "id": 1
  }'
```

**Response**:
```json
{
  "jsonrpc": "2.0",
  "result": {
    "chain": "test",
    "blocks": 29,
    "height": 29,
    "bestblockhash": "0cf1aa42a976e21e948f1de701decb61f437797e12ca1dc0213e7f8e0987c716",
    "difficulty": 0.1,
    "mediantime": 1766579232,
    "verificationprogress": 1.0,
    "initialblockdownload": false,
    "chainwork": "0000000000000000000000000000000000000000000000000000000000000000",
    "size_on_disk": 0,
    "pruned": false,
    "reward": 50.0,
    "block_reward": 50.0
  },
  "id": 1
}
```

---

#### `getblock`

Get block information.

**Parameters**:
- `blockhash` (string): Block hash

**Returns**: `object` - Block details

**Example**:
```bash
curl -X POST http://localhost:8332 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "getblock",
    "params": ["0cf1aa42a976e21e948f1de701decb61f437797e12ca1dc0213e7f8e0987c716"],
    "id": 1
  }'
```

---

#### `getnetworkinfo`

Get network information.

**Parameters**: None

**Returns**: `object` - Network stats

**Example**:
```bash
curl -X POST http://localhost:8332 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "getnetworkinfo",
    "params": [],
    "id": 1
  }'
```

---

## Code Examples

### Python Example

```python
import requests
import json

class GXCWallet:
    def __init__(self, node_url="http://localhost:8332"):
        self.node_url = node_url
        self.request_id = 0
    
    def rpc_call(self, method, params=None):
        """Make RPC call to node"""
        if params is None:
            params = []
        
        self.request_id += 1
        payload = {
            "jsonrpc": "2.0",
            "method": method,
            "params": params,
            "id": self.request_id
        }
        
        response = requests.post(self.node_url, json=payload)
        result = response.json()
        
        if 'error' in result and result['error']:
            raise Exception(f"RPC Error: {result['error']}")
        
        return result.get('result')
    
    def get_new_address(self):
        """Generate new address"""
        return self.rpc_call("getnewaddress")
    
    def get_balance(self, address):
        """Get address balance"""
        return self.rpc_call("getbalance", [address])
    
    def send(self, from_addr, to_addr, amount):
        """Send transaction"""
        return self.rpc_call("gxc_sendFrom", [from_addr, to_addr, amount])
    
    def stake(self, address, amount, days):
        """Stake GXC"""
        return self.rpc_call("registervalidator", [address, amount, days])
    
    def get_staking_info(self, address):
        """Get staking info"""
        return self.rpc_call("getstakinginfo", [address])

# Usage
wallet = GXCWallet()

# Generate address
address = wallet.get_new_address()
print(f"New address: {address}")

# Check balance
balance = wallet.get_balance(address)
print(f"Balance: {balance} GXC")

# Send transaction
tx_hash = wallet.send(address, "tGXC...", 10.0)
print(f"Transaction: {tx_hash}")

# Stake
stake_result = wallet.stake(address, 100.0, 30)
print(f"Staked: {stake_result}")
```

---

### JavaScript Example

```javascript
const axios = require('axios');

class GXCWallet {
    constructor(nodeUrl = 'http://localhost:8332') {
        this.nodeUrl = nodeUrl;
        this.requestId = 0;
    }
    
    async rpcCall(method, params = []) {
        this.requestId++;
        
        const payload = {
            jsonrpc: '2.0',
            method: method,
            params: params,
            id: this.requestId
        };
        
        const response = await axios.post(this.nodeUrl, payload);
        
        if (response.data.error) {
            throw new Error(`RPC Error: ${JSON.stringify(response.data.error)}`);
        }
        
        return response.data.result;
    }
    
    async getNewAddress() {
        return await this.rpcCall('getnewaddress');
    }
    
    async getBalance(address) {
        return await this.rpcCall('getbalance', [address]);
    }
    
    async send(fromAddr, toAddr, amount) {
        return await this.rpcCall('gxc_sendFrom', [fromAddr, toAddr, amount]);
    }
    
    async stake(address, amount, days) {
        return await this.rpcCall('registervalidator', [address, amount, days]);
    }
    
    async getStakingInfo(address) {
        return await this.rpcCall('getstakinginfo', [address]);
    }
}

// Usage
(async () => {
    const wallet = new GXCWallet();
    
    // Generate address
    const address = await wallet.getNewAddress();
    console.log(`New address: ${address}`);
    
    // Check balance
    const balance = await wallet.getBalance(address);
    console.log(`Balance: ${balance} GXC`);
    
    // Send transaction
    const txHash = await wallet.send(address, 'tGXC...', 10.0);
    console.log(`Transaction: ${txHash}`);
    
    // Stake
    const stakeResult = await wallet.stake(address, 100.0, 30);
    console.log(`Staked:`, stakeResult);
})();
```

---

## Error Handling

### Common Error Codes

| Code | Message | Description |
|------|---------|-------------|
| -8 | Invalid parameter | Parameter validation failed |
| -32601 | Method not found | RPC method doesn't exist |
| -32603 | Internal error | Server-side error |
| -5 | Invalid address | Address format invalid |
| -6 | Insufficient funds | Not enough balance |

### Error Response Format

```json
{
  "jsonrpc": "2.0",
  "error": {
    "code": -6,
    "message": "Insufficient funds"
  },
  "id": 1
}
```

### Handling Errors

```python
try:
    result = wallet.send(from_addr, to_addr, amount)
except Exception as e:
    if "Insufficient funds" in str(e):
        print("Not enough balance")
    elif "Invalid address" in str(e):
        print("Address format is wrong")
    else:
        print(f"Error: {e}")
```

---

## Best Practices

### 1. Connection Management

- Use connection pooling for high-volume applications
- Implement retry logic with exponential backoff
- Set appropriate timeouts (10-30 seconds)

### 2. Transaction Handling

- Always verify transaction confirmation
- Wait for at least 1 block confirmation for small amounts
- Wait for 6+ confirmations for large amounts
- Store transaction hashes for tracking

### 3. Security

- Never expose private keys
- Use HTTPS in production
- Implement rate limiting
- Validate all user inputs
- Use environment variables for sensitive data

### 4. Error Handling

- Always check for errors in responses
- Log errors for debugging
- Provide user-friendly error messages
- Implement fallback mechanisms

### 5. Performance

- Batch requests when possible
- Cache blockchain info (update every block)
- Use websockets for real-time updates (if available)
- Implement pagination for large result sets

---

## Transaction Fees

- **Minimum Fee**: 0.001 GXC
- **Recommended Fee**: 0.001 - 0.01 GXC
- **Fee Calculation**: Dynamic based on network congestion

---

## Network Types

### Testnet
- **Prefix**: `tGXC`
- **RPC Port**: 8332
- **P2P Port**: 9333
- **Difficulty**: 0.1

### Mainnet
- **Prefix**: `GXC`
- **RPC Port**: 8332
- **P2P Port**: 9333
- **Difficulty**: 1000.0

---

## Support

- **Documentation**: https://github.com/your-repo/docs
- **Issues**: https://github.com/your-repo/issues
- **Community**: Discord/Telegram links

---

## Changelog

### Version 2.0.0
- Initial wallet API release
- Support for transactions, staking, queries
- Python and JavaScript examples
- Comprehensive error handling

---

**Last Updated**: December 24, 2024  
**API Version**: 2.0.0  
**Status**: Production Ready
