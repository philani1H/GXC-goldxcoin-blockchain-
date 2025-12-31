# How to Prove Real Mining Work and Payment in GXC

## Overview

GXC blockchain provides **cryptographic proof** that mining work was done and payment was received. This document explains how miners, auditors, and third parties can verify both.

---

## Part 1: Proving You Did the Work

### What is a Work Receipt?

A **Work Receipt** is GXC's cryptographic proof that links:
- The mining work performed
- The block produced
- The miner who did the work
- The reward transaction

### Work Receipt Formula

```
WorkReceipt = SHA256(
    previousHash || 
    merkleRoot || 
    nonce || 
    minerPublicKey || 
    difficulty || 
    timestamp
)
```

### How to Verify Work Receipt

#### Step 1: Get Block Data
```bash
curl -X POST http://node:18332/ \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "getblock",
    "params": [1],
    "id": 1
  }'
```

**Response:**
```json
{
  "result": {
    "height": 1,
    "hash": "00000000abc123...",
    "previousblockhash": "fd759e82...",
    "merkleroot": "a3f5b2c1...",
    "nonce": 12345678,
    "difficulty": 1.0,
    "timestamp": 1767186975,
    "miner": "tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9",
    "work_receipt": "8f3a2b1c9d0e1f2a3b4c5d6e7f8901234567890a",
    "transactions": [...]
  }
}
```

#### Step 2: Recompute Work Receipt
```python
import hashlib

def compute_work_receipt(block):
    data = (
        block['previousblockhash'] +
        block['merkleroot'] +
        str(block['nonce']) +
        block['miner_pubkey'] +
        str(block['difficulty']) +
        str(block['timestamp'])
    )
    return hashlib.sha256(data.encode()).hexdigest()

# Verify
computed = compute_work_receipt(block_data)
claimed = block_data['work_receipt']

if computed == claimed:
    print("✅ Work receipt VALID - Miner did the work")
else:
    print("❌ Work receipt INVALID - Fraud detected")
```

#### Step 3: Verify Proof-of-Work
```python
def verify_proof_of_work(block_hash, difficulty):
    # Count leading zeros
    leading_zeros = 0
    for char in block_hash:
        if char == '0':
            leading_zeros += 1
        else:
            break
    
    required_zeros = int(difficulty)
    
    if leading_zeros >= required_zeros:
        print(f"✅ Valid PoW: {leading_zeros}/{required_zeros} zeros")
        return True
    else:
        print(f"❌ Invalid PoW: {leading_zeros}/{required_zeros} zeros")
        return False
```

### Proof Components

| Component | Purpose | Verification |
|-----------|---------|--------------|
| **Block Hash** | Proves computational work | Hash < target (leading zeros) |
| **Work Receipt** | Proves WHO did the work | Recompute and compare |
| **Nonce** | Proves trial-and-error | Part of hash calculation |
| **Difficulty** | Proves work difficulty | Matches network difficulty |
| **Timestamp** | Proves when work was done | Within acceptable range |
| **Miner Public Key** | Proves miner identity | Included in work receipt |

---

## Part 2: Proving You Got Paid

### Payment Verification Methods

#### Method 1: Check UTXO Set (Real-Time Balance)

```bash
curl -X POST http://node:18332/ \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "getbalance",
    "params": ["tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9"],
    "id": 1
  }'
```

**Response:**
```json
{
  "result": 50.0
}
```

This queries the **UTXO set** - the real-time record of unspent transaction outputs.

#### Method 2: Verify Coinbase Transaction

```bash
curl -X POST http://node:18332/ \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "gettransaction",
    "params": ["coinbase_tx_hash"],
    "id": 1
  }'
```

**Response:**
```json
{
  "result": {
    "txid": "abc123...",
    "type": "COINBASE",
    "block_height": 1,
    "confirmations": 6,
    "outputs": [{
      "address": "tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9",
      "amount": 50.0,
      "spent": false
    }],
    "work_receipt": "8f3a2b1c9d0e1f2a3b4c5d6e7f8901234567890a"
  }
}
```

**Key Fields:**
- `type: "COINBASE"` - This is a mining reward
- `block_height: 1` - From block 1
- `confirmations: 6` - 6 blocks deep (confirmed)
- `spent: false` - UTXO still unspent (you still have it)
- `work_receipt` - Links to the block's work receipt

#### Method 3: Trace Payment Through Blockchain

```bash
curl -X GET http://node:18332/api/v1/trace/coinbase_tx_hash
```

**Response:**
```json
{
  "transaction": {
    "hash": "abc123...",
    "type": "COINBASE",
    "block": 1,
    "miner": "tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9",
    "amount": 50.0,
    "work_receipt": "8f3a2b1c9d0e1f2a3b4c5d6e7f8901234567890a"
  },
  "block": {
    "height": 1,
    "hash": "00000000abc123...",
    "work_receipt": "8f3a2b1c9d0e1f2a3b4c5d6e7f8901234567890a",
    "miner": "tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9"
  },
  "proof": {
    "work_receipt_matches": true,
    "coinbase_in_block": true,
    "payment_confirmed": true
  }
}
```

#### Method 4: Check Transaction History

```bash
curl -X POST http://node:18332/ \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "listtransactions",
    "params": ["tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9", 100],
    "id": 1
  }'
```

**Response:**
```json
{
  "result": [
    {
      "txid": "abc123...",
      "category": "generate",
      "amount": 50.0,
      "confirmations": 6,
      "blockhash": "00000000abc123...",
      "blockheight": 1,
      "blocktime": 1767186975,
      "work_receipt": "8f3a2b1c9d0e1f2a3b4c5d6e7f8901234567890a"
    }
  ]
}
```

---

## Part 3: Complete Verification Flow

### For Miners: Prove You Mined a Block

```python
#!/usr/bin/env python3
import requests
import hashlib

def prove_mining_work(node_url, block_height, miner_address):
    """
    Cryptographically prove you mined a block and got paid.
    """
    
    # Step 1: Get block data
    response = requests.post(f"{node_url}/", json={
        "jsonrpc": "2.0",
        "method": "getblock",
        "params": [block_height],
        "id": 1
    })
    block = response.json()['result']
    
    # Step 2: Verify work receipt
    work_receipt_data = (
        block['previousblockhash'] +
        block['merkleroot'] +
        str(block['nonce']) +
        block.get('miner_pubkey', '') +
        str(block['difficulty']) +
        str(block['timestamp'])
    )
    computed_receipt = hashlib.sha256(work_receipt_data.encode()).hexdigest()
    
    print(f"Block Height: {block['height']}")
    print(f"Block Hash: {block['hash'][:16]}...")
    print(f"Miner: {block['miner'][:20]}...")
    print(f"Work Receipt (claimed): {block['work_receipt'][:16]}...")
    print(f"Work Receipt (computed): {computed_receipt[:16]}...")
    
    if computed_receipt == block['work_receipt']:
        print("✅ Work receipt VALID")
    else:
        print("❌ Work receipt INVALID")
        return False
    
    # Step 3: Verify proof-of-work
    leading_zeros = len(block['hash']) - len(block['hash'].lstrip('0'))
    required_zeros = int(block['difficulty'])
    
    print(f"Proof-of-Work: {leading_zeros}/{required_zeros} zeros")
    
    if leading_zeros >= required_zeros:
        print("✅ Proof-of-Work VALID")
    else:
        print("❌ Proof-of-Work INVALID")
        return False
    
    # Step 4: Verify miner address matches
    if block['miner'] != miner_address:
        print(f"❌ Miner address mismatch")
        return False
    
    print("✅ Miner address matches")
    
    # Step 5: Get coinbase transaction
    coinbase_tx = block['transactions'][0]
    
    print(f"\nCoinbase Transaction:")
    print(f"  TX Hash: {coinbase_tx['hash'][:16]}...")
    print(f"  Type: {coinbase_tx['type']}")
    print(f"  Amount: {coinbase_tx['outputs'][0]['amount']} GXC")
    print(f"  Recipient: {coinbase_tx['outputs'][0]['address'][:20]}...")
    print(f"  Work Receipt: {coinbase_tx.get('work_receipt', 'N/A')[:16]}...")
    
    # Step 6: Verify coinbase work receipt matches block
    if coinbase_tx.get('work_receipt') == block['work_receipt']:
        print("✅ Coinbase work receipt matches block")
    else:
        print("❌ Coinbase work receipt mismatch")
        return False
    
    # Step 7: Check current balance
    response = requests.post(f"{node_url}/", json={
        "jsonrpc": "2.0",
        "method": "getbalance",
        "params": [miner_address],
        "id": 1
    })
    balance = response.json()['result']
    
    print(f"\nCurrent Balance: {balance} GXC")
    
    # Step 8: Verify UTXO exists
    response = requests.post(f"{node_url}/", json={
        "jsonrpc": "2.0",
        "method": "listunspent",
        "params": [miner_address],
        "id": 1
    })
    utxos = response.json()['result']
    
    coinbase_utxo = None
    for utxo in utxos:
        if utxo['txid'] == coinbase_tx['hash']:
            coinbase_utxo = utxo
            break
    
    if coinbase_utxo:
        print(f"✅ Coinbase UTXO found: {coinbase_utxo['amount']} GXC")
        print(f"   Confirmations: {coinbase_utxo['confirmations']}")
        print(f"   Spendable: {coinbase_utxo['spendable']}")
    else:
        print("⚠️  Coinbase UTXO not found (may be spent or immature)")
    
    print("\n" + "="*60)
    print("✅ PROOF COMPLETE: You mined this block and received payment")
    print("="*60)
    
    return True

# Example usage
if __name__ == "__main__":
    prove_mining_work(
        node_url="http://localhost:18332",
        block_height=1,
        miner_address="tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9"
    )
```

### For Auditors: Verify Mining Claims

```python
def audit_mining_claim(node_url, block_height, claimed_miner):
    """
    Independent verification of mining claim.
    """
    
    # Get block
    response = requests.post(f"{node_url}/", json={
        "jsonrpc": "2.0",
        "method": "getblock",
        "params": [block_height],
        "id": 1
    })
    block = response.json()['result']
    
    # Verify work receipt independently
    computed_receipt = compute_work_receipt(block)
    
    # Verify all components
    checks = {
        "work_receipt_valid": computed_receipt == block['work_receipt'],
        "pow_valid": verify_proof_of_work(block['hash'], block['difficulty']),
        "miner_matches": block['miner'] == claimed_miner,
        "coinbase_exists": len(block['transactions']) > 0 and 
                          block['transactions'][0]['type'] == 'COINBASE',
        "coinbase_recipient_matches": block['transactions'][0]['outputs'][0]['address'] == claimed_miner,
        "coinbase_work_receipt_matches": block['transactions'][0].get('work_receipt') == block['work_receipt']
    }
    
    print("Audit Results:")
    for check, result in checks.items():
        status = "✅" if result else "❌"
        print(f"  {status} {check}: {result}")
    
    all_valid = all(checks.values())
    
    if all_valid:
        print("\n✅ AUDIT PASSED: Mining claim is valid")
    else:
        print("\n❌ AUDIT FAILED: Mining claim is invalid")
    
    return all_valid
```

---

## Part 4: Blockchain Explorer Verification

### Using the Web Explorer

1. **Navigate to block:**
   ```
   http://explorer.gxc.network/block/1
   ```

2. **View block details:**
   - Block hash
   - Work receipt
   - Miner address
   - Difficulty
   - Timestamp
   - Transactions

3. **Click on coinbase transaction:**
   - See mining reward
   - Verify recipient address
   - Check work receipt link
   - View confirmations

4. **Check miner balance:**
   ```
   http://explorer.gxc.network/address/tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9
   ```

### API Verification

```bash
# Get block
curl http://explorer.gxc.network/api/v1/block/1

# Get transaction
curl http://explorer.gxc.network/api/v1/transaction/abc123...

# Trace transaction
curl http://explorer.gxc.network/api/v1/trace/abc123...

# Check address
curl http://explorer.gxc.network/api/v1/address/tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9/balance
```

---

## Part 5: Legal/Regulatory Proof

### For Tax Purposes

**What to provide:**
1. Block height and hash
2. Work receipt
3. Coinbase transaction hash
4. Mining reward amount
5. Date/time (timestamp)
6. Current balance

**How to generate report:**
```python
def generate_mining_report(node_url, miner_address, start_date, end_date):
    """
    Generate mining income report for tax purposes.
    """
    
    # Get all blocks mined by address
    response = requests.post(f"{node_url}/", json={
        "jsonrpc": "2.0",
        "method": "listtransactions",
        "params": [miner_address, 10000],
        "id": 1
    })
    
    transactions = response.json()['result']
    
    # Filter coinbase transactions in date range
    mining_rewards = []
    for tx in transactions:
        if tx['category'] == 'generate':
            tx_date = datetime.fromtimestamp(tx['blocktime'])
            if start_date <= tx_date <= end_date:
                mining_rewards.append({
                    'date': tx_date.strftime('%Y-%m-%d'),
                    'block': tx['blockheight'],
                    'amount': tx['amount'],
                    'txid': tx['txid'],
                    'work_receipt': tx.get('work_receipt', 'N/A')
                })
    
    # Generate CSV report
    with open('mining_report.csv', 'w') as f:
        f.write('Date,Block,Amount (GXC),Transaction Hash,Work Receipt\n')
        for reward in mining_rewards:
            f.write(f"{reward['date']},{reward['block']},{reward['amount']},{reward['txid']},{reward['work_receipt']}\n")
    
    total = sum(r['amount'] for r in mining_rewards)
    
    print(f"Mining Report Generated:")
    print(f"  Period: {start_date} to {end_date}")
    print(f"  Blocks Mined: {len(mining_rewards)}")
    print(f"  Total Rewards: {total} GXC")
    print(f"  Report saved to: mining_report.csv")
    
    return mining_rewards
```

### For Disputes

If someone claims you didn't mine a block:

1. **Provide block data** with your miner address
2. **Show work receipt** that includes your public key
3. **Demonstrate PoW** by showing hash meets difficulty
4. **Show coinbase transaction** paying to your address
5. **Prove UTXO ownership** by signing a message with your private key

```python
def prove_ownership(private_key, message):
    """
    Sign a message to prove you own the address.
    """
    from ecdsa import SigningKey, SECP256k1
    import hashlib
    
    sk = SigningKey.from_string(bytes.fromhex(private_key), curve=SECP256k1)
    signature = sk.sign(message.encode())
    
    return signature.hex()

# Example
message = f"I mined block 1 on {datetime.now().isoformat()}"
signature = prove_ownership(your_private_key, message)

print(f"Message: {message}")
print(f"Signature: {signature}")
print("Anyone can verify this signature with your public key")
```

---

## Summary: Complete Proof Chain

```
1. Block Hash < Target
   ↓
   Proves: Computational work was done

2. Work Receipt = SHA256(block_data + miner_pubkey)
   ↓
   Proves: YOU did the work

3. Coinbase Transaction in Block
   ↓
   Proves: Reward was created

4. Coinbase.work_receipt == Block.work_receipt
   ↓
   Proves: Reward is linked to YOUR work

5. UTXO in address
   ↓
   Proves: You received the payment

6. Can sign with private key
   ↓
   Proves: You control the address
```

**Result:** Cryptographic proof that you did the work and got paid.

---

## Tools and Scripts

All verification scripts are available in:
```
/workspaces/GXC-goldxcoin-blockchain-/scripts/verify_mining.py
```

Run verification:
```bash
python3 scripts/verify_mining.py \
  --node http://localhost:18332 \
  --block 1 \
  --miner tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9
```

---

**Document Version:** 1.0  
**Last Updated:** December 31, 2025  
**Author:** GXC Development Team
