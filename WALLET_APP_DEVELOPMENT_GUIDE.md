# GXC Wallet Application Development Guide

Complete guide for building wallet applications that connect to the GXC blockchain.

## Table of Contents

1. [Overview](#overview)
2. [Blockchain Architecture](#blockchain-architecture)
3. [API Endpoints](#api-endpoints)
4. [Wallet Creation](#wallet-creation)
5. [Key Management](#key-management)
6. [Transaction Handling](#transaction-handling)
7. [Balance Queries](#balance-queries)
8. [Real-Time Updates](#real-time-updates)
9. [Security Best Practices](#security-best-practices)
10. [Code Examples](#code-examples)

---

## Overview

The GXC blockchain provides a comprehensive REST API and RPC interface for wallet applications. This guide covers everything you need to build a fully functional wallet that interacts with the GXC blockchain using real data.

### Network Configuration

```json
{
  "network_name": "GXC Mainnet",
  "chain_id": "GXC",
  "rpc_url": "http://localhost:8545",
  "rest_url": "http://localhost:8545",
  "explorer_url": "http://localhost:3000",
  "currency": "GXC",
  "block_time": "2 seconds",
  "consensus": "Hybrid PoW/PoS"
}
```

---

## Blockchain Architecture

### Address Format
- **Prefix**: `GXC`
- **Length**: 43 characters total
- **Format**: `GXC` + 40 hex characters
- **Example**: `GXC1a2b3c4d5e6f7g8h9i0j1k2l3m4n5o6p7q8r9s0t`

### Key Pair Generation
- **Algorithm**: ED25519
- **Private Key**: 32 bytes (256 bits)
- **Public Key**: 32 bytes (256 bits)
- **Address**: Derived from public key hash

### Transaction Structure
```json
{
  "version": 1,
  "inputs": [
    {
      "txHash": "previous_transaction_hash",
      "outputIndex": 0,
      "amount": 100.0
    }
  ],
  "outputs": [
    {
      "address": "GXC...",
      "amount": 99.0
    }
  ],
  "fee": 0.001,
  "lockTime": 0,
  "memo": "optional memo",
  "senderAddress": "GXC...",
  "receiverAddress": "GXC..."
}
```

---

## API Endpoints

### Base URLs
- **REST API**: `http://localhost:8545/api/v1`
- **RPC API**: `http://localhost:8545` (JSON-RPC 2.0)

### REST Endpoints

#### 1. Get Blockchain Info
```http
GET /api/v1/blockchain/info
```

**Response:**
```json
{
  "chain": "GXC",
  "blocks": 12345,
  "bestblockhash": "0x...",
  "difficulty": 1.5,
  "networkhashps": 1000000,
  "chainwork": "0x..."
}
```

#### 2. Get Address Balance
```http
GET /api/v1/address/{address}/balance
```

**Response:**
```json
{
  "address": "GXC1a2b3c...",
  "balance": 1000.5,
  "confirmed": 1000.5,
  "unconfirmed": 0.0,
  "tx_count": 42
}
```

#### 3. Get Address Transactions
```http
GET /api/v1/address/{address}/transactions?limit=50
```

**Response:**
```json
[
  {
    "txid": "0x...",
    "hash": "0x...",
    "version": 1,
    "time": 1234567890,
    "blocktime": 1234567890,
    "confirmations": 10,
    "sender_address": "GXC...",
    "receiver_address": "GXC...",
    "amount": 100.0,
    "fee": 0.001,
    "vin": [...],
    "vout": [...]
  }
]
```

#### 4. Get Transaction Details
```http
GET /api/v1/transaction/{tx_hash}
```

**Response:**
```json
{
  "txid": "0x...",
  "hash": "0x...",
  "version": 1,
  "size": 250,
  "locktime": 0,
  "time": 1234567890,
  "confirmations": 10,
  "sender_address": "GXC...",
  "receiver_address": "GXC...",
  "amount": 100.0,
  "fee": 0.001,
  "vin": [...],
  "vout": [...],
  "prev_tx_hash": "0x...",
  "referenced_amount": 100.001,
  "is_traceable": true
}
```

#### 5. Get UTXOs for Address
```http
GET /api/v1/address/{address}/utxo
```

**Response:**
```json
[
  {
    "txHash": "0x...",
    "outputIndex": 0,
    "address": "GXC...",
    "amount": 50.0,
    "spent": false
  }
]
```

#### 6. Submit Transaction
```http
POST /api/v1/transactions
Content-Type: application/json
```

**Request:**
```json
{
  "from_address": "GXC...",
  "to_address": "GXC...",
  "amount": 100.0,
  "fee": 0.001,
  "utxos": [
    {
      "txHash": "0x...",
      "outputIndex": 0,
      "amount": 100.001
    }
  ],
  "private_key": "encrypted_private_key",
  "memo": "optional memo"
}
```

**Response:**
```json
{
  "txid": "0x...",
  "result": "success",
  "message": "Transaction submitted to mempool"
}
```

---

## Wallet Creation

### Step 1: Generate Key Pair

```python
from cryptography.hazmat.primitives.asymmetric import ed25519
from cryptography.hazmat.primitives import serialization
import hashlib
import base58

def generate_keypair():
    """Generate ED25519 key pair"""
    private_key = ed25519.Ed25519PrivateKey.generate()
    public_key = private_key.public_key()
    
    # Serialize keys
    private_key_bytes = private_key.private_bytes(
        encoding=serialization.Encoding.Raw,
        format=serialization.PrivateFormat.Raw,
        encryption_algorithm=serialization.NoEncryption()
    )
    
    public_key_bytes = public_key.public_bytes(
        encoding=serialization.Encoding.Raw,
        format=serialization.PublicFormat.Raw
    )
    
    return private_key_bytes, public_key_bytes
```

### Step 2: Derive Address from Public Key

```python
def derive_address(public_key_bytes):
    """Derive GXC address from public key"""
    # Hash public key
    hash_obj = hashlib.sha256(public_key_bytes)
    hash_bytes = hash_obj.digest()
    
    # Take first 20 bytes
    address_hash = hash_bytes[:20]
    
    # Encode with base58 and add GXC prefix
    address = "GXC" + base58.b58encode(address_hash).decode('utf-8')
    
    return address
```

### Step 3: Encrypt Private Key

```python
from cryptography.fernet import Fernet
import bcrypt

def encrypt_private_key(private_key_bytes, password):
    """Encrypt private key with password"""
    # Derive key from password
    salt = bcrypt.gensalt()
    key = bcrypt.hashpw(password.encode(), salt)
    
    # Use first 32 bytes as encryption key
    encryption_key = key[:32]
    f = Fernet(base64.urlsafe_b64encode(encryption_key))
    
    encrypted_key = f.encrypt(private_key_bytes)
    return encrypted_key, salt
```

### Step 4: Store Wallet Securely

```python
import sqlite3
import json

def store_wallet(user_id, wallet_name, address, public_key, encrypted_private_key):
    """Store wallet in database"""
    conn = sqlite3.connect('wallets.db')
    cursor = conn.cursor()
    
    cursor.execute('''
        INSERT INTO wallets (user_id, wallet_name, address, public_key, encrypted_private_key)
        VALUES (?, ?, ?, ?, ?)
    ''', (user_id, wallet_name, address, public_key, encrypted_private_key))
    
    conn.commit()
    conn.close()
```

---

## Key Management

### Decrypting Private Key

```python
def decrypt_private_key(encrypted_private_key, password, salt):
    """Decrypt private key with password"""
    key = bcrypt.hashpw(password.encode(), salt)
    encryption_key = key[:32]
    f = Fernet(base64.urlsafe_b64encode(encryption_key))
    
    private_key_bytes = f.decrypt(encrypted_private_key)
    return private_key_bytes
```

### Signing Transactions

```python
def sign_transaction(transaction_data, private_key_bytes):
    """Sign transaction with private key"""
    private_key = ed25519.Ed25519PrivateKey.from_private_bytes(private_key_bytes)
    
    # Serialize transaction
    tx_string = json.dumps(transaction_data, sort_keys=True)
    tx_bytes = tx_string.encode('utf-8')
    
    # Sign
    signature = private_key.sign(tx_bytes)
    
    return base64.b64encode(signature).decode('utf-8')
```

---

## Transaction Handling

### Building a Transaction

```python
def build_transaction(from_address, to_address, amount, fee, utxos):
    """Build transaction from UTXOs"""
    total_input = sum(utxo['amount'] for utxo in utxos)
    change = total_input - amount - fee
    
    transaction = {
        "version": 1,
        "inputs": [
            {
                "txHash": utxo["txHash"],
                "outputIndex": utxo["outputIndex"],
                "amount": utxo["amount"]
            }
            for utxo in utxos
        ],
        "outputs": [
            {
                "address": to_address,
                "amount": amount
            }
        ],
        "fee": fee,
        "lockTime": 0,
        "senderAddress": from_address,
        "receiverAddress": to_address
    }
    
    # Add change output if needed
    if change > 0:
        transaction["outputs"].append({
            "address": from_address,
            "amount": change
        })
    
    return transaction
```

### Submitting a Transaction

```python
import requests

def submit_transaction(transaction, signature, rest_url):
    """Submit signed transaction to blockchain"""
    payload = {
        "transaction": transaction,
        "signature": signature
    }
    
    response = requests.post(
        f"{rest_url}/api/v1/transactions",
        json=payload,
        headers={"Content-Type": "application/json"}
    )
    
    if response.status_code == 200:
        return response.json()
    else:
        raise Exception(f"Transaction failed: {response.text}")
```

---

## Balance Queries

### Fetching Real Balance

```python
def get_balance(address, rest_url):
    """Get real balance from blockchain"""
    response = requests.get(
        f"{rest_url}/api/v1/address/{address}/balance"
    )
    
    if response.status_code == 200:
        data = response.json()
        return data['balance']
    else:
        return 0.0
```

### Fetching UTXOs

```python
def get_utxos(address, rest_url):
    """Get UTXOs for address"""
    response = requests.get(
        f"{rest_url}/api/v1/address/{address}/utxo"
    )
    
    if response.status_code == 200:
        return response.json()
    else:
        return []
```

### Selecting UTXOs for Transaction

```python
def select_utxos(utxos, amount, fee):
    """Select UTXOs for transaction (simplified)"""
    total_needed = amount + fee
    selected = []
    total = 0.0
    
    for utxo in sorted(utxos, key=lambda x: x['amount'], reverse=True):
        if total >= total_needed:
            break
        selected.append(utxo)
        total += utxo['amount']
    
    if total < total_needed:
        raise Exception("Insufficient funds")
    
    return selected
```

---

## Real-Time Updates

### Polling for Updates

```python
import time
import threading

class BalanceMonitor:
    def __init__(self, address, rest_url, callback, interval=5):
        self.address = address
        self.rest_url = rest_url
        self.callback = callback
        self.interval = interval
        self.running = False
        self.last_balance = 0.0
    
    def start(self):
        self.running = True
        thread = threading.Thread(target=self._monitor)
        thread.daemon = True
        thread.start()
    
    def stop(self):
        self.running = False
    
    def _monitor(self):
        while self.running:
            try:
                balance = get_balance(self.address, self.rest_url)
                if balance != self.last_balance:
                    self.callback(balance, self.last_balance)
                    self.last_balance = balance
            except Exception as e:
                print(f"Error monitoring balance: {e}")
            
            time.sleep(self.interval)
```

### WebSocket Connection (if available)

```python
import websocket
import json

def connect_websocket(address, callback):
    """Connect to blockchain WebSocket for real-time updates"""
    ws_url = "ws://localhost:8545/ws"
    
    def on_message(ws, message):
        data = json.loads(message)
        if data.get('address') == address:
            callback(data)
    
    ws = websocket.WebSocketApp(
        ws_url,
        on_message=on_message
    )
    ws.run_forever()
```

---

## Security Best Practices

### 1. Never Store Private Keys in Plain Text
- Always encrypt private keys
- Use strong password hashing (bcrypt, argon2)
- Store encrypted keys securely

### 2. Use Secure Random Number Generation
```python
import secrets

# Generate secure random bytes
private_key = secrets.token_bytes(32)
```

### 3. Validate All Inputs
```python
def validate_address(address):
    """Validate GXC address format"""
    if not address.startswith('GXC'):
        return False
    if len(address) != 43:
        return False
    # Add more validation as needed
    return True

def validate_amount(amount):
    """Validate transaction amount"""
    if amount <= 0:
        return False
    if amount > 1000000000:  # Max supply check
        return False
    return True
```

### 4. Implement Rate Limiting
```python
from functools import wraps
import time

def rate_limit(calls_per_second=10):
    """Rate limiting decorator"""
    min_interval = 1.0 / calls_per_second
    last_called = [0.0]
    
    def decorator(func):
        @wraps(func)
        def wrapper(*args, **kwargs):
            elapsed = time.time() - last_called[0]
            left_to_wait = min_interval - elapsed
            if left_to_wait > 0:
                time.sleep(left_to_wait)
            ret = func(*args, **kwargs)
            last_called[0] = time.time()
            return ret
        return wrapper
    return decorator
```

### 5. Use HTTPS in Production
- Always use HTTPS for API calls in production
- Verify SSL certificates
- Never send private keys over unencrypted connections

---

## Code Examples

### Complete Wallet Class

```python
import requests
import json
from cryptography.hazmat.primitives.asymmetric import ed25519
from cryptography.hazmat.primitives import serialization

class GXCWallet:
    def __init__(self, address, private_key_bytes, rest_url):
        self.address = address
        self.private_key_bytes = private_key_bytes
        self.rest_url = rest_url
    
    def get_balance(self):
        """Get wallet balance"""
        response = requests.get(
            f"{self.rest_url}/api/v1/address/{self.address}/balance"
        )
        if response.status_code == 200:
            return response.json()['balance']
        return 0.0
    
    def get_transactions(self, limit=50):
        """Get transaction history"""
        response = requests.get(
            f"{self.rest_url}/api/v1/address/{self.address}/transactions",
            params={'limit': limit}
        )
        if response.status_code == 200:
            return response.json()
        return []
    
    def get_utxos(self):
        """Get UTXOs"""
        response = requests.get(
            f"{self.rest_url}/api/v1/address/{self.address}/utxo"
        )
        if response.status_code == 200:
            return response.json()
        return []
    
    def send(self, to_address, amount, fee=0.001):
        """Send GXC to another address"""
        # Get UTXOs
        utxos = self.get_utxos()
        if not utxos:
            raise Exception("No UTXOs available")
        
        # Select UTXOs
        selected_utxos = self._select_utxos(utxos, amount, fee)
        
        # Build transaction
        transaction = self._build_transaction(to_address, amount, fee, selected_utxos)
        
        # Sign transaction
        signature = self._sign_transaction(transaction)
        
        # Submit transaction
        return self._submit_transaction(transaction, signature)
    
    def _select_utxos(self, utxos, amount, fee):
        """Select UTXOs for transaction"""
        total_needed = amount + fee
        selected = []
        total = 0.0
        
        for utxo in sorted(utxos, key=lambda x: x['amount'], reverse=True):
            if total >= total_needed:
                break
            selected.append(utxo)
            total += utxo['amount']
        
        if total < total_needed:
            raise Exception("Insufficient funds")
        
        return selected
    
    def _build_transaction(self, to_address, amount, fee, utxos):
        """Build transaction"""
        total_input = sum(utxo['amount'] for utxo in utxos)
        change = total_input - amount - fee
        
        transaction = {
            "version": 1,
            "inputs": [
                {
                    "txHash": utxo["txHash"],
                    "outputIndex": utxo["outputIndex"],
                    "amount": utxo["amount"]
                }
                for utxo in utxos
            ],
            "outputs": [
                {
                    "address": to_address,
                    "amount": amount
                }
            ],
            "fee": fee,
            "lockTime": 0,
            "senderAddress": self.address,
            "receiverAddress": to_address
        }
        
        if change > 0:
            transaction["outputs"].append({
                "address": self.address,
                "amount": change
            })
        
        return transaction
    
    def _sign_transaction(self, transaction):
        """Sign transaction"""
        private_key = ed25519.Ed25519PrivateKey.from_private_bytes(self.private_key_bytes)
        tx_string = json.dumps(transaction, sort_keys=True)
        tx_bytes = tx_string.encode('utf-8')
        signature = private_key.sign(tx_bytes)
        return base64.b64encode(signature).decode('utf-8')
    
    def _submit_transaction(self, transaction, signature):
        """Submit transaction to blockchain"""
        payload = {
            "transaction": transaction,
            "signature": signature
        }
        
        response = requests.post(
            f"{self.rest_url}/api/v1/transactions",
            json=payload,
            headers={"Content-Type": "application/json"}
        )
        
        if response.status_code == 200:
            return response.json()
        else:
            raise Exception(f"Transaction failed: {response.text}")
```

### Usage Example

```python
# Initialize wallet
wallet = GXCWallet(
    address="GXC1a2b3c...",
    private_key_bytes=private_key,
    rest_url="http://localhost:8545"
)

# Get balance
balance = wallet.get_balance()
print(f"Balance: {balance} GXC")

# Send transaction
try:
    result = wallet.send(
        to_address="GXC9z8y7x6w...",
        amount=10.0,
        fee=0.001
    )
    print(f"Transaction submitted: {result['txid']}")
except Exception as e:
    print(f"Error: {e}")

# Get transaction history
transactions = wallet.get_transactions(limit=10)
for tx in transactions:
    print(f"TX: {tx['txid']}, Amount: {tx['amount']} GXC")
```

---

## Error Handling

### Common Errors and Solutions

1. **Insufficient Balance**
   - Check balance before sending
   - Include fee in calculations
   - Handle UTXO selection properly

2. **Invalid Address**
   - Validate address format
   - Check address checksum (if implemented)

3. **Network Errors**
   - Implement retry logic
   - Handle timeouts gracefully
   - Provide user feedback

4. **Transaction Failures**
   - Check transaction status
   - Verify signatures
   - Validate transaction structure

---

## Testing

### Test Network Configuration

For testing, use a testnet or local development network:

```python
TESTNET_CONFIG = {
    "rpc_url": "http://testnet.gxc.io:8545",
    "rest_url": "http://testnet.gxc.io:8545",
    "explorer_url": "http://testnet-explorer.gxc.io"
}
```

### Unit Tests

```python
import unittest

class TestGXCWallet(unittest.TestCase):
    def setUp(self):
        self.wallet = GXCWallet(
            address="GXC...",
            private_key_bytes=b"...",
            rest_url="http://localhost:8545"
        )
    
    def test_get_balance(self):
        balance = self.wallet.get_balance()
        self.assertIsInstance(balance, float)
        self.assertGreaterEqual(balance, 0)
    
    def test_get_transactions(self):
        transactions = self.wallet.get_transactions()
        self.assertIsInstance(transactions, list)
```

---

## Additional Resources

- **Blockchain Explorer**: `http://localhost:3000`
- **RPC API Documentation**: See `src/RPCAPI.cpp`
- **REST API Documentation**: See `src/RESTServer.cpp`
- **Database Schema**: See `include/Database.h`

---

## Support

For questions or issues:
1. Check the blockchain logs
2. Verify network connectivity
3. Ensure correct API endpoints
4. Review transaction structure

---

**Last Updated**: 2024
**Version**: 1.0

