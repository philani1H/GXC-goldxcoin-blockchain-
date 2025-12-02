# GXC Wallet App Development Guide

Complete guide for building a wallet application that connects to the GXC blockchain.

---

## 📋 Table of Contents

1. [Overview](#overview)
2. [Getting Started](#getting-started)
3. [Connecting to the Blockchain](#connecting-to-the-blockchain)
4. [Wallet Operations](#wallet-operations)
5. [Staking Operations](#staking-operations)
6. [Complete API Reference](#complete-api-reference)
7. [Example Implementation](#example-implementation)
8. [Security Best Practices](#security-best-practices)
9. [Error Handling](#error-handling)
10. [Testing](#testing)

---

## 🎯 Overview

This guide will help you build a wallet app that can:
- ✅ Connect to GXC blockchain nodes
- ✅ Generate addresses
- ✅ Check balances
- ✅ Send and receive coins
- ✅ Stake coins and become a validator
- ✅ View validator information
- ✅ Monitor transactions

---

## 🚀 Getting Started

### Prerequisites

- **Blockchain Node**: Running GXC node (testnet or mainnet)
- **RPC Endpoint**: Node RPC URL (default: `http://localhost:18332`)
- **Programming Language**: Any language that supports HTTP/JSON (Python, JavaScript, Java, etc.)

### Node Connection

**Testnet Node**: `http://localhost:18332`  
**Mainnet Node**: `http://localhost:8332`  
**Remote Node**: `http://your-node-ip:18332`

---

## 🔌 Connecting to the Blockchain

### RPC API Format

All requests use JSON-RPC 2.0 format:

```json
{
  "jsonrpc": "2.0",
  "method": "method_name",
  "params": [...],
  "id": 1
}
```

### Example: Check Connection

```bash
curl -X POST http://localhost:18332 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "getblockchaininfo",
    "params": [],
    "id": 1
  }'
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "result": {
    "chain": "testnet",
    "blocks": 1234,
    "difficulty": 0.1
  },
  "id": 1
}
```

### Python Connection Example

```python
import requests
import json

class GXCWallet:
    def __init__(self, rpc_url="http://localhost:18332"):
        self.rpc_url = rpc_url
        self.session = requests.Session()
        self.session.headers.update({'Content-Type': 'application/json'})
    
    def rpc_call(self, method, params=None):
        """Make RPC call to blockchain node"""
        if params is None:
            params = []
        
        payload = {
            "jsonrpc": "2.0",
            "method": method,
            "params": params,
            "id": 1
        }
        
        try:
            response = self.session.post(self.rpc_url, json=payload, timeout=10)
            response.raise_for_status()
            result = response.json()
            
            if "error" in result:
                raise Exception(f"RPC Error: {result['error']}")
            
            return result.get("result")
        except requests.exceptions.RequestException as e:
            raise Exception(f"Connection error: {e}")
    
    def test_connection(self):
        """Test connection to blockchain node"""
        try:
            info = self.rpc_call("getblockchaininfo")
            return True, info
        except Exception as e:
            return False, str(e)

# Usage
wallet = GXCWallet("http://localhost:18332")
connected, info = wallet.test_connection()
if connected:
    print(f"Connected! Chain height: {info.get('blocks', 0)}")
else:
    print(f"Connection failed: {info}")
```

### JavaScript/Node.js Connection Example

```javascript
const axios = require('axios');

class GXCWallet {
    constructor(rpcUrl = 'http://localhost:18332') {
        this.rpcUrl = rpcUrl;
    }
    
    async rpcCall(method, params = []) {
        const payload = {
            jsonrpc: '2.0',
            method: method,
            params: params,
            id: 1
        };
        
        try {
            const response = await axios.post(this.rpcUrl, payload, {
                headers: { 'Content-Type': 'application/json' },
                timeout: 10000
            });
            
            if (response.data.error) {
                throw new Error(`RPC Error: ${response.data.error.message}`);
            }
            
            return response.data.result;
        } catch (error) {
            throw new Error(`Connection error: ${error.message}`);
        }
    }
    
    async testConnection() {
        try {
            const info = await this.rpcCall('getblockchaininfo');
            return { connected: true, info };
        } catch (error) {
            return { connected: false, error: error.message };
        }
    }
}

// Usage
const wallet = new GXCWallet('http://localhost:18332');
wallet.testConnection().then(result => {
    if (result.connected) {
        console.log(`Connected! Chain height: ${result.info.blocks}`);
    } else {
        console.log(`Connection failed: ${result.error}`);
    }
});
```

---

## 💼 Wallet Operations

### 1. Generate New Address

**Method**: `getnewaddress`

```python
def generate_address(self):
    """Generate a new GXC address"""
    address = self.rpc_call("getnewaddress", [])
    return address
```

**RPC Request:**
```json
{
  "jsonrpc": "2.0",
  "method": "getnewaddress",
  "params": [],
  "id": 1
}
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "result": "tGXC9fab7317231b966af85ac453e168c0932",
  "id": 1
}
```

### 2. Check Balance

**Method**: `getbalance` or `gxc_getBalance`

```python
def get_balance(self, address):
    """Get balance for an address"""
    balance = self.rpc_call("getbalance", [address])
    return float(balance)
```

**RPC Request:**
```json
{
  "jsonrpc": "2.0",
  "method": "getbalance",
  "params": ["tGXC9fab7317231b966af85ac453e168c0932"],
  "id": 1
}
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "result": 1000.5,
  "id": 1
}
```

### 3. Get Transaction History

**Method**: `listtransactions` or `gxc_getTransactionsByAddress`

```python
def get_transactions(self, address, limit=100):
    """Get transaction history for an address"""
    transactions = self.rpc_call("listtransactions", [address, limit])
    return transactions
```

**RPC Request:**
```json
{
  "jsonrpc": "2.0",
  "method": "listtransactions",
  "params": ["tGXC9fab7317231b966af85ac453e168c0932", 100],
  "id": 1
}
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "result": [
    {
      "hash": "abc123...",
      "from": "tGXC...sender",
      "to": "tGXC9fab7317231b966af85ac453e168c0932",
      "amount": 100.0,
      "confirmations": 6,
      "timestamp": 1234567890,
      "is_coinbase": false
    }
  ],
  "id": 1
}
```

### 4. Send Coins

**Method**: `sendtoaddress`

```python
def send_coins(self, from_address, to_address, amount, private_key=None):
    """
    Send coins from one address to another
    
    Note: In production, private key should be stored securely
    and transaction should be signed client-side
    """
    # For now, this is a placeholder - actual implementation
    # would require signing the transaction with private key
    result = self.rpc_call("sendtoaddress", [
        from_address,
        to_address,
        amount
    ])
    return result
```

**RPC Request:**
```json
{
  "jsonrpc": "2.0",
  "method": "sendtoaddress",
  "params": [
    "tGXC...from",
    "tGXC...to",
    100.0
  ],
  "id": 1
}
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "result": {
    "txid": "transaction_hash_here",
    "success": true
  },
  "id": 1
}
```

### 5. Validate Address

**Method**: `validateaddress`

```python
def validate_address(self, address):
    """Validate if an address is valid"""
    result = self.rpc_call("validateaddress", [address])
    return result.get("isvalid", False)
```

---

## 💰 Staking Operations

### 1. Register as Validator / Stake Coins

**Method**: `registervalidator` or `gxc_registerValidator` or `stake`

```python
def stake_coins(self, address, stake_amount, staking_days):
    """
    Register as validator and stake coins
    
    Requirements:
    - Minimum stake: 100 GXC
    - Staking period: 14-365 days
    - Must have sufficient balance
    """
    result = self.rpc_call("registervalidator", [
        address,
        stake_amount,
        staking_days
    ])
    return result
```

**RPC Request:**
```json
{
  "jsonrpc": "2.0",
  "method": "registervalidator",
  "params": [
    "tGXC9fab7317231b966af85ac453e168c0932",
    1000.0,
    90
  ],
  "id": 1
}
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "result": {
    "success": true,
    "address": "tGXC9fab7317231b966af85ac453e168c0932",
    "stake_amount": 1000.0,
    "staking_days": 90,
    "weighted_stake": 1050.0,
    "message": "Validator registered successfully"
  },
  "id": 1
}
```

### 2. Add More Stake

**Method**: `addstake` or `gxc_addStake`

```python
def add_stake(self, address, additional_amount):
    """Add more stake to existing validator"""
    result = self.rpc_call("addstake", [
        address,
        additional_amount
    ])
    return result
```

### 3. Unstake / Withdraw Stake

**Method**: `unstake` or `gxc_unstake`

```python
def unstake(self, address):
    """Withdraw stake and deactivate validator"""
    result = self.rpc_call("unstake", [address])
    return result
```

### 4. Get All Validators

**Method**: `getvalidators` or `gxc_getValidators`

```python
def get_validators(self):
    """Get list of all active validators"""
    validators = self.rpc_call("getvalidators", [])
    return validators
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "result": [
    {
      "address": "tGXC9fab7317231b966af85ac453e168c0932",
      "stake_amount": 1000.0,
      "staking_days": 90,
      "weighted_stake": 1050.0,
      "is_active": true,
      "blocks_produced": 15,
      "missed_blocks": 2,
      "uptime": 0.95,
      "total_rewards": 50.0,
      "apy": 5.0,
      "is_slashed": false
    }
  ],
  "id": 1
}
```

### 5. Get Validator Info

**Method**: `getvalidatorinfo` or `gxc_getValidatorInfo`

```python
def get_validator_info(self, address):
    """Get detailed information for a specific validator"""
    info = self.rpc_call("getvalidatorinfo", [address])
    return info
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "result": {
    "address": "tGXC9fab7317231b966af85ac453e168c0932",
    "stake_amount": 1000.0,
    "staking_days": 90,
    "weighted_stake": 1050.0,
    "time_weight": 1.05,
    "is_active": true,
    "blocks_produced": 15,
    "missed_blocks": 2,
    "uptime": 0.95,
    "total_rewards": 50.0,
    "pending_rewards": 5.0,
    "apy": 5.0,
    "is_slashed": false,
    "slashed_amount": 0.0,
    "is_valid": true
  },
  "id": 1
}
```

---

## 📚 Complete API Reference

### Wallet Methods

| Method | Description | Parameters |
|--------|-------------|------------|
| `getnewaddress` | Generate new address | `[]` |
| `getbalance` | Get balance | `[address]` |
| `gxc_getBalance` | Get balance (alias) | `[address]` |
| `listtransactions` | Get transactions | `[address, limit]` |
| `gxc_getTransactionsByAddress` | Get transactions (alias) | `[address, limit]` |
| `sendtoaddress` | Send coins | `[from, to, amount]` |
| `validateaddress` | Validate address | `[address]` |
| `gettransaction` | Get transaction details | `[txid]` |
| `gxc_getTransaction` | Get transaction (alias) | `[txid]` |

### Staking Methods

| Method | Description | Parameters |
|--------|-------------|------------|
| `registervalidator` | Register as validator | `[address, stakeAmount, stakingDays]` |
| `gxc_registerValidator` | Register (alias) | `[address, stakeAmount, stakingDays]` |
| `stake` | Stake coins (alias) | `[address, stakeAmount, stakingDays]` |
| `addstake` | Add more stake | `[address, additionalAmount]` |
| `gxc_addStake` | Add stake (alias) | `[address, additionalAmount]` |
| `unstake` | Withdraw stake | `[address]` |
| `gxc_unstake` | Unstake (alias) | `[address]` |
| `getvalidators` | List all validators | `[]` |
| `gxc_getValidators` | List validators (alias) | `[]` |
| `getvalidatorinfo` | Get validator info | `[address]` |
| `gxc_getValidatorInfo` | Get validator info (alias) | `[address]` |

### Blockchain Methods

| Method | Description | Parameters |
|--------|-------------|------------|
| `getblockchaininfo` | Get chain info | `[]` |
| `getblockcount` | Get block count | `[]` |
| `getblock` | Get block | `[height/hash, verbose]` |
| `getlatestblock` | Get latest block | `[]` |

---

## 💻 Complete Example Implementation

### Python Wallet App Example

```python
import requests
import json
from typing import Optional, List, Dict

class GXCWalletApp:
    """Complete GXC Wallet Application"""
    
    def __init__(self, rpc_url: str = "http://localhost:18332"):
        self.rpc_url = rpc_url
        self.session = requests.Session()
        self.session.headers.update({'Content-Type': 'application/json'})
    
    def rpc_call(self, method: str, params: List = None) -> Dict:
        """Make RPC call to blockchain node"""
        if params is None:
            params = []
        
        payload = {
            "jsonrpc": "2.0",
            "method": method,
            "params": params,
            "id": 1
        }
        
        try:
            response = self.session.post(
                self.rpc_url, 
                json=payload, 
                timeout=10
            )
            response.raise_for_status()
            result = response.json()
            
            if "error" in result:
                raise Exception(f"RPC Error: {result['error']}")
            
            return result.get("result")
        except requests.exceptions.RequestException as e:
            raise Exception(f"Connection error: {e}")
    
    # ========== WALLET OPERATIONS ==========
    
    def generate_address(self) -> str:
        """Generate a new GXC address"""
        return self.rpc_call("getnewaddress", [])
    
    def get_balance(self, address: str) -> float:
        """Get balance for an address"""
        balance = self.rpc_call("getbalance", [address])
        return float(balance)
    
    def get_transactions(self, address: str, limit: int = 100) -> List[Dict]:
        """Get transaction history"""
        return self.rpc_call("listtransactions", [address, limit])
    
    def send_coins(self, from_address: str, to_address: str, amount: float) -> Dict:
        """Send coins to another address"""
        return self.rpc_call("sendtoaddress", [
            from_address,
            to_address,
            amount
        ])
    
    def validate_address(self, address: str) -> bool:
        """Validate address format"""
        result = self.rpc_call("validateaddress", [address])
        return result.get("isvalid", False)
    
    def get_transaction_details(self, txid: str) -> Dict:
        """Get detailed transaction information"""
        return self.rpc_call("gettransaction", [txid])
    
    # ========== STAKING OPERATIONS ==========
    
    def stake_coins(
        self, 
        address: str, 
        stake_amount: float, 
        staking_days: int
    ) -> Dict:
        """
        Register as validator and stake coins
        
        Args:
            address: Your wallet address
            stake_amount: Amount to stake (minimum 100 GXC)
            staking_days: Staking period (14-365 days)
        
        Returns:
            Validator registration result
        """
        if stake_amount < 100.0:
            raise ValueError("Minimum stake is 100 GXC")
        if not (14 <= staking_days <= 365):
            raise ValueError("Staking days must be between 14 and 365")
        
        return self.rpc_call("registervalidator", [
            address,
            stake_amount,
            staking_days
        ])
    
    def add_stake(self, address: str, additional_amount: float) -> Dict:
        """Add more stake to existing validator"""
        return self.rpc_call("addstake", [address, additional_amount])
    
    def unstake(self, address: str) -> Dict:
        """Withdraw stake and deactivate validator"""
        return self.rpc_call("unstake", [address])
    
    def get_all_validators(self) -> List[Dict]:
        """Get list of all active validators"""
        return self.rpc_call("getvalidators", [])
    
    def get_validator_info(self, address: str) -> Dict:
        """Get detailed validator information"""
        return self.rpc_call("getvalidatorinfo", [address])
    
    # ========== BLOCKCHAIN INFO ==========
    
    def get_blockchain_info(self) -> Dict:
        """Get blockchain information"""
        return self.rpc_call("getblockchaininfo", [])
    
    def get_block_count(self) -> int:
        """Get current block height"""
        return self.rpc_call("getblockcount", [])
    
    def get_latest_block(self) -> Dict:
        """Get latest block"""
        return self.rpc_call("getlatestblock", [])


# ========== USAGE EXAMPLE ==========

if __name__ == "__main__":
    # Initialize wallet
    wallet = GXCWalletApp("http://localhost:18332")
    
    # Generate address
    address = wallet.generate_address()
    print(f"Generated address: {address}")
    
    # Check balance
    balance = wallet.get_balance(address)
    print(f"Balance: {balance} GXC")
    
    # Get transactions
    transactions = wallet.get_transactions(address)
    print(f"Transaction count: {len(transactions)}")
    
    # Stake coins (if you have enough balance)
    if balance >= 100:
        stake_result = wallet.stake_coins(address, 100.0, 90)
        print(f"Staking result: {stake_result}")
        
        # Get validator info
        validator_info = wallet.get_validator_info(address)
        print(f"Validator info: {validator_info}")
    
    # Get all validators
    validators = wallet.get_all_validators()
    print(f"Total validators: {len(validators)}")
    
    # Get blockchain info
    chain_info = wallet.get_blockchain_info()
    print(f"Chain height: {chain_info.get('blocks', 0)}")
```

### JavaScript/React Wallet App Example

```javascript
// wallet.js
const axios = require('axios');

class GXCWalletApp {
    constructor(rpcUrl = 'http://localhost:18332') {
        this.rpcUrl = rpcUrl;
    }
    
    async rpcCall(method, params = []) {
        const payload = {
            jsonrpc: '2.0',
            method: method,
            params: params,
            id: 1
        };
        
        try {
            const response = await axios.post(this.rpcUrl, payload, {
                headers: { 'Content-Type': 'application/json' },
                timeout: 10000
            });
            
            if (response.data.error) {
                throw new Error(response.data.error.message);
            }
            
            return response.data.result;
        } catch (error) {
            throw new Error(`RPC Error: ${error.message}`);
        }
    }
    
    // Wallet Operations
    async generateAddress() {
        return await this.rpcCall('getnewaddress');
    }
    
    async getBalance(address) {
        return await this.rpcCall('getbalance', [address]);
    }
    
    async getTransactions(address, limit = 100) {
        return await this.rpcCall('listtransactions', [address, limit]);
    }
    
    async sendCoins(fromAddress, toAddress, amount) {
        return await this.rpcCall('sendtoaddress', [fromAddress, toAddress, amount]);
    }
    
    // Staking Operations
    async stakeCoins(address, stakeAmount, stakingDays) {
        if (stakeAmount < 100) {
            throw new Error('Minimum stake is 100 GXC');
        }
        if (stakingDays < 14 || stakingDays > 365) {
            throw new Error('Staking days must be between 14 and 365');
        }
        return await this.rpcCall('registervalidator', [address, stakeAmount, stakingDays]);
    }
    
    async addStake(address, additionalAmount) {
        return await this.rpcCall('addstake', [address, additionalAmount]);
    }
    
    async unstake(address) {
        return await this.rpcCall('unstake', [address]);
    }
    
    async getAllValidators() {
        return await this.rpcCall('getvalidators');
    }
    
    async getValidatorInfo(address) {
        return await this.rpcCall('getvalidatorinfo', [address]);
    }
    
    // Blockchain Info
    async getBlockchainInfo() {
        return await this.rpcCall('getblockchaininfo');
    }
}

module.exports = GXCWalletApp;
```

### React Component Example

```jsx
// WalletComponent.jsx
import React, { useState, useEffect } from 'react';
import GXCWalletApp from './wallet';

function WalletComponent() {
    const [wallet] = useState(new GXCWalletApp('http://localhost:18332'));
    const [address, setAddress] = useState('');
    const [balance, setBalance] = useState(0);
    const [transactions, setTransactions] = useState([]);
    const [validators, setValidators] = useState([]);
    
    useEffect(() => {
        loadWallet();
        loadValidators();
    }, []);
    
    const loadWallet = async () => {
        try {
            const addr = await wallet.generateAddress();
            setAddress(addr);
            const bal = await wallet.getBalance(addr);
            setBalance(bal);
            const txs = await wallet.getTransactions(addr);
            setTransactions(txs);
        } catch (error) {
            console.error('Error loading wallet:', error);
        }
    };
    
    const loadValidators = async () => {
        try {
            const vals = await wallet.getAllValidators();
            setValidators(vals);
        } catch (error) {
            console.error('Error loading validators:', error);
        }
    };
    
    const handleStake = async () => {
        try {
            const result = await wallet.stakeCoins(address, 1000, 90);
            alert(`Staked successfully! Weighted stake: ${result.weighted_stake}`);
            loadValidators();
        } catch (error) {
            alert(`Error: ${error.message}`);
        }
    };
    
    return (
        <div>
            <h1>GXC Wallet</h1>
            <p>Address: {address}</p>
            <p>Balance: {balance} GXC</p>
            
            <h2>Transactions</h2>
            <ul>
                {transactions.map(tx => (
                    <li key={tx.hash}>
                        {tx.amount} GXC - {tx.confirmations} confirmations
                    </li>
                ))}
            </ul>
            
            <h2>Staking</h2>
            <button onClick={handleStake}>Stake 1000 GXC for 90 days</button>
            
            <h2>Validators</h2>
            <ul>
                {validators.map(v => (
                    <li key={v.address}>
                        {v.address}: {v.stake_amount} GXC staked, APY: {v.apy}%
                    </li>
                ))}
            </ul>
        </div>
    );
}

export default WalletComponent;
```

---

## 🔒 Security Best Practices

### 1. **Never Store Private Keys in Code**

```python
# ❌ BAD
PRIVATE_KEY = "your_private_key_here"

# ✅ GOOD - Use environment variables or secure storage
import os
private_key = os.environ.get('GXC_PRIVATE_KEY')
```

### 2. **Use HTTPS for Remote Connections**

```python
# ✅ GOOD
wallet = GXCWalletApp("https://your-secure-node.com:18332")
```

### 3. **Validate All Inputs**

```python
def send_coins(self, from_address, to_address, amount):
    # Validate address format
    if not self.validate_address(to_address):
        raise ValueError("Invalid recipient address")
    
    # Validate amount
    if amount <= 0:
        raise ValueError("Amount must be positive")
    
    # Check balance
    balance = self.get_balance(from_address)
    if balance < amount:
        raise ValueError("Insufficient balance")
    
    return self.rpc_call("sendtoaddress", [from_address, to_address, amount])
```

### 4. **Handle Errors Gracefully**

```python
try:
    result = wallet.send_coins(from_addr, to_addr, amount)
    print(f"Transaction sent: {result['txid']}")
except Exception as e:
    print(f"Error: {e}")
    # Log error, notify user, etc.
```

### 5. **Sign Transactions Client-Side**

In production, transactions should be signed client-side using the private key, not sent to the node unsigned.

---

## ⚠️ Error Handling

### Common RPC Errors

```python
def handle_rpc_error(error_response):
    """Handle RPC error responses"""
    error_code = error_response.get("code", -1)
    error_message = error_response.get("message", "Unknown error")
    
    error_map = {
        -8: "Invalid parameter",
        -32601: "Method not found",
        -32600: "Invalid request",
        -32603: "Internal error"
    }
    
    return error_map.get(error_code, error_message)
```

### Example Error Handling

```python
try:
    balance = wallet.get_balance(address)
except Exception as e:
    if "Connection error" in str(e):
        # Handle connection issues
        print("Cannot connect to blockchain node")
    elif "RPC Error" in str(e):
        # Handle RPC errors
        print(f"Blockchain error: {e}")
    else:
        # Handle other errors
        print(f"Unexpected error: {e}")
```

---

## 🧪 Testing

### Test Checklist

- [ ] Connect to blockchain node
- [ ] Generate new address
- [ ] Check balance (should be 0 for new address)
- [ ] Receive coins (from mining or another wallet)
- [ ] Verify balance updates
- [ ] Send coins to another address
- [ ] Verify transaction appears in history
- [ ] Register as validator
- [ ] Check validator appears in validator list
- [ ] Add more stake
- [ ] Unstake
- [ ] Verify validator removed from list

### Test Script

```python
def test_wallet():
    wallet = GXCWalletApp("http://localhost:18332")
    
    # Test 1: Generate address
    address = wallet.generate_address()
    assert address.startswith("tGXC"), "Invalid address format"
    print("✅ Address generation works")
    
    # Test 2: Check balance
    balance = wallet.get_balance(address)
    assert isinstance(balance, (int, float)), "Balance should be numeric"
    print(f"✅ Balance check works: {balance} GXC")
    
    # Test 3: Get validators
    validators = wallet.get_all_validators()
    assert isinstance(validators, list), "Validators should be a list"
    print(f"✅ Validator list works: {len(validators)} validators")
    
    print("All tests passed!")

if __name__ == "__main__":
    test_wallet()
```

---

## 📱 Mobile App Integration

### React Native Example

```javascript
// WalletService.js
import axios from 'axios';

const RPC_URL = 'http://your-node-ip:18332';

export const WalletService = {
    async getBalance(address) {
        const response = await axios.post(RPC_URL, {
            jsonrpc: '2.0',
            method: 'getbalance',
            params: [address],
            id: 1
        });
        return response.data.result;
    },
    
    async stakeCoins(address, amount, days) {
        const response = await axios.post(RPC_URL, {
            jsonrpc: '2.0',
            method: 'registervalidator',
            params: [address, amount, days],
            id: 1
        });
        return response.data.result;
    }
};
```

---

## 🎨 UI/UX Recommendations

### Wallet Dashboard Should Show:

1. **Balance Display**
   - Current balance (large, prominent)
   - Available balance (after staked amount)
   - Staked amount (if validator)

2. **Transaction List**
   - Recent transactions
   - Pending transactions
   - Transaction status (confirmed/unconfirmed)

3. **Staking Section**
   - Current stake amount
   - Staking period remaining
   - Rewards earned
   - Validator status
   - APY percentage

4. **Quick Actions**
   - Send coins button
   - Receive coins (QR code)
   - Stake button
   - View validators button

---

## 📚 Additional Resources

- **RPC API Documentation**: See `STAKING_ENDPOINTS_GUIDE.md`
- **Mining Guide**: See `MINERS_README.md`
- **Blockchain Explorer**: Use web explorer for transaction verification
- **Testnet**: Use testnet for development/testing

---

## 🆘 Troubleshooting

### Connection Issues

**Problem**: Cannot connect to node  
**Solution**: 
- Check node is running
- Verify RPC port (18332 for testnet)
- Check firewall settings
- Verify URL format

### Balance Not Updating

**Problem**: Balance shows 0 after receiving coins  
**Solution**:
- Wait for block confirmation (6+ blocks)
- Check transaction hash in explorer
- Verify address is correct

### Staking Errors

**Problem**: Cannot stake coins  
**Solution**:
- Ensure balance >= 100 GXC
- Check staking days (14-365)
- Verify address is valid
- Check if already a validator

---

## 📝 Summary

This guide provides everything you need to build a GXC wallet app:

✅ **Connection**: RPC API connection examples  
✅ **Wallet Ops**: Generate addresses, check balance, send/receive  
✅ **Staking Ops**: Register validator, stake, unstake, view validators  
✅ **Code Examples**: Python, JavaScript, React  
✅ **Security**: Best practices for secure implementation  
✅ **Testing**: Test scripts and checklists  

Start building your wallet app today! 🚀
