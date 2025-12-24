# GXC Blockchain - Wallet API Documentation v2.0

## Overview

Complete reference for interacting with GXC blockchain nodes. All 95 RPC methods documented with examples in Python, JavaScript, and curl.

**Base URL**: `http://localhost:8332` (RPC)  
**Base URL**: `http://localhost:8080` (REST)  
**Protocol**: JSON-RPC 2.0  
**Version**: 2.0.0  
**Status**: Production Ready ✅

---

## Quick Start

### Connection Test

```bash
curl -X POST http://localhost:8332 \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc":"2.0",
    "method":"getblockchaininfo",
    "params":[],
    "id":1
  }'
```

### Python Example

```python
import requests
import json

class GXCClient:
    def __init__(self, url="http://localhost:8332"):
        self.url = url
        self.id = 0
    
    def call(self, method, params=[]):
        self.id += 1
        payload = {
            "jsonrpc": "2.0",
            "method": method,
            "params": params,
            "id": self.id
        }
        response = requests.post(self.url, json=payload)
        return response.json()["result"]

# Usage
client = GXCClient()
info = client.call("getblockchaininfo")
print(f"Height: {info['height']}")
print(f"Chainwork: {info['chainwork']}")
```

### JavaScript Example

```javascript
class GXCClient {
    constructor(url = 'http://localhost:8332') {
        this.url = url;
        this.id = 0;
    }
    
    async call(method, params = []) {
        this.id++;
        const response = await fetch(this.url, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                jsonrpc: '2.0',
                method: method,
                params: params,
                id: this.id
            })
        });
        const data = await response.json();
        return data.result;
    }
}

// Usage
const client = new GXCClient();
const info = await client.call('getblockchaininfo');
console.log(`Height: ${info.height}`);
console.log(`Chainwork: ${info.chainwork}`);
```

---

## API Categories

1. [Blockchain Info](#blockchain-info) - Chain state, height, difficulty
2. [Blocks](#blocks) - Block queries and templates
3. [Transactions](#transactions) - Create, send, query transactions
4. [Wallet](#wallet) - Addresses, balances, UTXOs
5. [Mining](#mining) - Mining info, templates, submission
6. [Staking](#staking) - Stake, unstake, validator info
7. [Network](#network) - Peers, connections, network info
8. [Mempool](#mempool) - Pending transactions
9. [Utility](#utility) - Fee estimation, validation, help

---

## Blockchain Info

### getblockchaininfo

Get current blockchain state.

**Parameters**: None

**Returns**:
```json
{
  "chain": "test",
  "blocks": 39,
  "height": 39,
  "bestblockhash": "0bdb2dbc2ec258a6...",
  "difficulty": 0.1,
  "chainwork": "00000000000000000000000000000000000000000000000000000000003b8260",
  "mediantime": 1766582772,
  "verificationprogress": 1.0,
  "initialblockdownload": false,
  "block_reward": 50.0,
  "reward": 50.0
}
```

**Example**:
```python
info = client.call("getblockchaininfo")
print(f"Current height: {info['height']}")
print(f"Chainwork: {info['chainwork']}")
print(f"Difficulty: {info['difficulty']}")
```

### getbestblockhash

Get hash of the latest block.

**Parameters**: None

**Returns**: `string` - Block hash

**Example**:
```python
hash = client.call("getbestblockhash")
print(f"Latest block: {hash}")
```

### getblockcount

Get current blockchain height.

**Parameters**: None

**Returns**: `number` - Block height

**Example**:
```python
height = client.call("getblockcount")
print(f"Height: {height}")
```

### getdifficulty

Get current mining difficulty.

**Parameters**: None

**Returns**: `number` - Difficulty

**Example**:
```python
difficulty = client.call("getdifficulty")
print(f"Difficulty: {difficulty}")
```

---

## Blocks

### getblock

Get block by hash or height.

**Parameters**:
- `hash_or_height` (string|number) - Block hash or height

**Returns**: Block object

**Example**:
```python
# By height
block = client.call("getblock", [0])

# By hash
block = client.call("getblock", ["0bdb2dbc2ec258a6..."])

print(f"Block {block['index']}")
print(f"Transactions: {len(block['transactions'])}")
print(f"Miner: {block['miner_address']}")
```

### getlatestblock

Get the most recent block.

**Parameters**: None

**Returns**: Block object

**Example**:
```python
block = client.call("getlatestblock")
print(f"Latest block: {block['index']}")
```

### getblockhash

Get block hash by height.

**Parameters**:
- `height` (number) - Block height

**Returns**: `string` - Block hash

**Example**:
```python
hash = client.call("getblockhash", [10])
print(f"Block 10 hash: {hash}")
```

### getblocktransactions

Get all transactions in a block.

**Parameters**:
- `hash_or_height` (string|number) - Block hash or height

**Returns**: Array of transactions

**Example**:
```python
txs = client.call("getblocktransactions", [10])
print(f"Block 10 has {len(txs)} transactions")
for tx in txs:
    print(f"  TX: {tx['hash']}, Type: {tx['type']}")
```

### getblocktemplate

Get block template for mining.

**Parameters**:
- `options` (object) - Mining options
  - `algorithm` (string) - "sha256", "ethash", "gxhash", or "pos"

**Returns**: Block template with transactions

**Example**:
```python
template = client.call("getblocktemplate", [{"algorithm": "sha256"}])
print(f"Height: {template['height']}")
print(f"Previous hash: {template['previousblockhash']}")
print(f"Transactions: {len(template['transactions'])}")
print(f"Difficulty: {template['difficulty']}")
```

### submitblock

Submit a mined block.

**Parameters**:
- `block_data` (object) - Block data including:
  - `nonce` (number) - Nonce that satisfies difficulty
  - `timestamp` (number) - Block timestamp
  - `transactions` (array) - Transactions to include
  - `miner_address` (string) - Address to receive reward

**Returns**: Success/error message

**Example**:
```python
result = client.call("submitblock", [{
    "nonce": 12345,
    "timestamp": 1766582772,
    "transactions": template["transactions"],
    "miner_address": "tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9"
}])
print(result)
```

---

## Transactions

### gettransaction

Get transaction by hash.

**Parameters**:
- `txid` (string) - Transaction hash

**Returns**: Transaction object

**Example**:
```python
tx = client.call("gettransaction", ["99d8b314a9f41c71..."])
print(f"Type: {tx['type']}")
print(f"From: {tx['from']}")
print(f"To: {tx['to']}")
print(f"Amount: {tx['amount']}")
print(f"Fee: {tx['fee']}")
```

### listtransactions

List transactions for an address.

**Parameters**:
- `address` (string) - GXC address
- `count` (number, optional) - Max transactions to return (default: 10)

**Returns**: Array of transactions

**Example**:
```python
txs = client.call("listtransactions", ["tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9", 20])
for tx in txs:
    print(f"{tx['type']}: {tx['amount']} GXC")
```

### getaddresstransactions

Get all transactions for an address.

**Parameters**:
- `address` (string) - GXC address

**Returns**: Array of transactions

**Example**:
```python
txs = client.call("getaddresstransactions", ["tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9"])
print(f"Total transactions: {len(txs)}")
```

### sendtoaddress

Send GXC to an address.

**Parameters**:
- `to_address` (string) - Recipient address
- `amount` (number) - Amount to send
- `from_address` (string, optional) - Sender address

**Returns**: Transaction hash

**Example**:
```python
txid = client.call("sendtoaddress", [
    "tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9",  # to
    10.5,  # amount
    "tGXCa1234..."  # from (optional)
])
print(f"Transaction sent: {txid}")
```

### createtransaction

Create a raw transaction.

**Parameters**:
- `from_address` (string) - Sender address
- `to_address` (string) - Recipient address
- `amount` (number) - Amount to send

**Returns**: Transaction object

**Example**:
```python
tx = client.call("createtransaction", [
    "tGXCa1234...",  # from
    "tGXCb5678...",  # to
    5.0  # amount
])
print(f"Created TX: {tx['hash']}")
```

### sendrawtransaction

Broadcast a signed transaction.

**Parameters**:
- `hex` (string) - Signed transaction hex

**Returns**: Transaction hash

**Example**:
```python
txid = client.call("sendrawtransaction", ["0x1234..."])
print(f"Broadcasted: {txid}")
```

---

## Wallet

### getnewaddress

Generate a new address.

**Parameters**: None

**Returns**: `string` - New address

**Example**:
```python
address = client.call("getnewaddress")
print(f"New address: {address}")
```

### getbalance

Get balance for an address.

**Parameters**:
- `address` (string) - GXC address

**Returns**: `number` - Balance in GXC

**Example**:
```python
balance = client.call("getbalance", ["tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9"])
print(f"Balance: {balance} GXC")
```

### validateaddress

Validate an address format.

**Parameters**:
- `address` (string) - Address to validate

**Returns**: Validation result

**Example**:
```python
result = client.call("validateaddress", ["tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9"])
print(f"Valid: {result['isvalid']}")
print(f"Network: {result['network']}")
```

### listunspent

List unspent transaction outputs (UTXOs).

**Parameters**:
- `address` (string) - GXC address

**Returns**: Array of UTXOs

**Example**:
```python
utxos = client.call("listunspent", ["tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9"])
total = sum(utxo['amount'] for utxo in utxos)
print(f"Total UTXOs: {len(utxos)}")
print(f"Total amount: {total} GXC")
```

### importprivkey

Import a private key.

**Parameters**:
- `privkey` (string) - Private key (hex)
- `label` (string, optional) - Label for the address

**Returns**: Address

**Example**:
```python
address = client.call("importprivkey", [
    "1B83F4149A12C776BA1851443B807E701D72AB76E7DC860FCBD6F268C01617A7",
    "My imported key"
])
print(f"Imported address: {address}")
```

---

## Mining

### getmininginfo

Get mining statistics.

**Parameters**: None

**Returns**: Mining info

**Example**:
```python
info = client.call("getmininginfo")
print(f"Network hashrate: {info['networkhashps']}")
print(f"Difficulty: {info['difficulty']}")
print(f"Block reward: {info['block_reward']}")
```

### getnetworkhashps

Get network hashrate.

**Parameters**: None

**Returns**: `number` - Hashrate in H/s

**Example**:
```python
hashrate = client.call("getnetworkhashps")
print(f"Network hashrate: {hashrate / 1e9:.2f} GH/s")
```

### estimatefee

Estimate transaction fee.

**Parameters**: None

**Returns**: Fee estimation object

**Example**:
```python
fee = client.call("estimatefee")
print(f"Recommended fee: {fee['fee']} GXC")
print(f"Current tier: {fee['current_tier']}")
print(f"Fee tiers: {fee['fee_tiers']}")
```

---

## Staking

### stake

Create a staking transaction.

**Parameters**:
- `address` (string) - Validator address
- `amount` (number) - Amount to stake (minimum 100 GXC)
- `days` (number) - Lock period in days

**Returns**: Staking result

**Example**:
```python
result = client.call("stake", [
    "tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9",
    100,  # amount
    30    # days
])
print(f"Stake TX: {result['stake_tx']}")
print(f"Status: {result['status']}")
```

### unstake

Unstake funds (after lock period).

**Parameters**:
- `address` (string) - Validator address
- `amount` (number) - Amount to unstake

**Returns**: Unstaking result

**Example**:
```python
result = client.call("unstake", [
    "tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9",
    50  # amount
])
print(f"Unstake TX: {result['unstake_tx']}")
```

### getstakinginfo

Get staking information for an address.

**Parameters**:
- `address` (string) - Validator address

**Returns**: Staking info

**Example**:
```python
info = client.call("getstakinginfo", ["tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9"])
print(f"Staked: {info['staked_balance']} GXC")
print(f"Spendable: {info['spendable_balance']} GXC")
print(f"Total: {info['total_balance']} GXC")
print(f"Is validator: {info['is_validator']}")
print(f"Total earned: {info['total_earned']} GXC")
```

### listvalidators

List all active validators.

**Parameters**: None

**Returns**: Array of validators

**Example**:
```python
validators = client.call("listvalidators")
print(f"Active validators: {len(validators)}")
for v in validators:
    print(f"  {v['address']}: {v['stake_amount']} GXC")
```

### getvalidatorinfo

Get detailed validator information.

**Parameters**:
- `address` (string) - Validator address

**Returns**: Validator info

**Example**:
```python
info = client.call("getvalidatorinfo", ["tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9"])
print(f"Stake: {info['stake_amount']} GXC")
print(f"Status: {info['status']}")
print(f"Blocks produced: {info['blocks_produced']}")
```

---

## Network

### getpeerinfo

Get information about connected peers.

**Parameters**: None

**Returns**: Array of peer info

**Example**:
```python
peers = client.call("getpeerinfo")
print(f"Connected peers: {len(peers)}")
for peer in peers:
    print(f"  {peer['addr']}: {peer['version']}")
```

### getconnectioncount

Get number of connections.

**Parameters**: None

**Returns**: `number` - Connection count

**Example**:
```python
count = client.call("getconnectioncount")
print(f"Connections: {count}")
```

### getnetworkinfo

Get network information.

**Parameters**: None

**Returns**: Network info

**Example**:
```python
info = client.call("getnetworkinfo")
print(f"Version: {info['version']}")
print(f"Connections: {info['connections']}")
print(f"Networks: {info['networks']}")
```

---

## Mempool

### getmempoolinfo

Get mempool statistics.

**Parameters**: None

**Returns**: Mempool info

**Example**:
```python
info = client.call("getmempoolinfo")
print(f"Pending transactions: {info['size']}")
print(f"Memory usage: {info['bytes']} bytes")
```

### getrawmempool

Get list of pending transactions.

**Parameters**: None

**Returns**: Array of transaction hashes

**Example**:
```python
txids = client.call("getrawmempool")
print(f"Pending: {len(txids)} transactions")
```

---

## Utility

### help

List all available RPC methods.

**Parameters**: None

**Returns**: `string` - List of methods

**Example**:
```python
methods = client.call("help")
print(methods)
```

---

## Complete Example: Send Transaction

```python
import requests
import json

class GXCWallet:
    def __init__(self, url="http://localhost:8332"):
        self.url = url
        self.id = 0
    
    def call(self, method, params=[]):
        self.id += 1
        payload = {
            "jsonrpc": "2.0",
            "method": method,
            "params": params,
            "id": self.id
        }
        response = requests.post(self.url, json=payload)
        result = response.json()
        if "error" in result and result["error"]:
            raise Exception(f"RPC Error: {result['error']}")
        return result["result"]
    
    def send_transaction(self, from_addr, to_addr, amount):
        """Send GXC from one address to another"""
        # 1. Check balance
        balance = self.call("getbalance", [from_addr])
        print(f"Balance: {balance} GXC")
        
        if balance < amount:
            raise Exception("Insufficient balance")
        
        # 2. Estimate fee
        fee_info = self.call("estimatefee")
        fee = fee_info["fee"]
        print(f"Fee: {fee} GXC")
        
        # 3. Send transaction
        txid = self.call("sendtoaddress", [to_addr, amount, from_addr])
        print(f"Transaction sent: {txid}")
        
        # 4. Wait for confirmation (check mempool)
        mempool = self.call("getrawmempool")
        if txid in mempool:
            print("Transaction in mempool, waiting for confirmation...")
        
        return txid

# Usage
wallet = GXCWallet()
txid = wallet.send_transaction(
    "tGXCa1234...",  # from
    "tGXCb5678...",  # to
    10.5             # amount
)
```

---

## Error Handling

All RPC errors follow this format:

```json
{
  "error": {
    "code": -8,
    "message": "Error description"
  },
  "id": 1,
  "jsonrpc": "2.0"
}
```

**Common Error Codes**:
- `-1` - Miscellaneous error
- `-3` - Invalid amount
- `-5` - Invalid address
- `-6` - Insufficient funds
- `-8` - Invalid parameter
- `-32600` - Invalid request
- `-32601` - Method not found
- `-32602` - Invalid params
- `-32603` - Internal error

**Example Error Handling**:
```python
try:
    result = client.call("sendtoaddress", ["invalid_address", 10])
except Exception as e:
    print(f"Error: {e}")
```

---

## Best Practices

1. **Always check balance before sending**
2. **Use estimatefee to get current fee**
3. **Verify addresses with validateaddress**
4. **Handle errors gracefully**
5. **Wait for confirmations (check mempool)**
6. **Use appropriate timeouts for network calls**
7. **Keep private keys secure**
8. **Use testnet for development**

---

## Support

- **Repository**: https://github.com/philani1H/GXC-goldxcoin-blockchain-.git
- **Documentation**: See repository docs/
- **Test Suite**: `./test_all_apis.sh`
- **Version**: 2.0.0

---

**Last Updated**: 2025-12-24  
**API Version**: 2.0.0  
**Status**: Production Ready ✅
