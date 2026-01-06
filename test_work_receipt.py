#!/usr/bin/env python3
"""
Test mining to see work receipt generation and validation
"""

import requests
import hashlib
import time
import json

RPC_URL = "http://localhost:8332"
MINER_ADDRESS = "tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf"

def rpc_call(method, params=None):
    """Make RPC call"""
    payload = {
        "jsonrpc": "2.0",
        "method": method,
        "params": params or [],
        "id": 1
    }
    try:
        response = requests.post(RPC_URL, json=payload, timeout=10)
        result = response.json()
        if 'error' in result and result['error']:
            print(f"RPC Error: {result['error']}")
            return None
        return result.get('result')
    except Exception as e:
        print(f"Request Error: {e}")
        return None

print("=" * 80)
print("WORK RECEIPT TEST - Mining Block and Examining Structure")
print("=" * 80)
print()

# Step 1: Check genesis block
print("Step 1: Examining Genesis Block")
print("-" * 80)
genesis = rpc_call("getblock", [0])
if genesis:
    print(f"Genesis Block (Height 0):")
    print(f"  Hash: {genesis.get('hash', 'N/A')}")
    print(f"  Miner: {genesis.get('miner', 'N/A')}")
    print(f"  Timestamp: {genesis.get('timestamp', 'N/A')}")
    print(f"  Difficulty: {genesis.get('difficulty', 'N/A')}")
    print(f"  Nonce: {genesis.get('nonce', 'N/A')}")
    print(f"  Work Receipt: {genesis.get('work_receipt', 'N/A')}")
    print(f"  Transactions: {len(genesis.get('transactions', []))}")
    
    if genesis.get('transactions'):
        print(f"\n  Genesis Coinbase Transaction:")
        tx = genesis['transactions'][0]
        print(f"    TX Hash: {tx.get('hash', 'N/A')}")
        print(f"    Is Coinbase: {tx.get('is_coinbase', False)}")
        print(f"    Work Receipt: {tx.get('work_receipt', 'N/A')}")
        print(f"    Outputs: {len(tx.get('outputs', []))}")
        if tx.get('outputs'):
            for i, out in enumerate(tx['outputs']):
                print(f"      Output {i}: {out.get('amount', 0)} GXC -> {out.get('address', 'N/A')}")
else:
    print("Failed to get genesis block")

print()
print("=" * 80)
print("Step 2: Getting Block Template for Mining")
print("-" * 80)

template = rpc_call("getblocktemplate", [MINER_ADDRESS])
if not template:
    print("Failed to get block template")
    exit(1)

print(f"Block Template Received:")
print(f"  Height: {template.get('height', 'N/A')}")
print(f"  Previous Hash: {template.get('previousblockhash', 'N/A')[:32]}...")
print(f"  Difficulty: {template.get('difficulty', 'N/A')}")
print(f"  Block Reward: {template.get('block_reward', 'N/A')} GXC")
print(f"  Coinbase Value: {template.get('coinbase_value', 'N/A')}")
print(f"  Transactions: {len(template.get('transactions', []))}")

# Show coinbase transaction from template
if template.get('transactions'):
    print(f"\n  Coinbase Transaction in Template:")
    coinbase = template['transactions'][0]
    print(f"    TX Hash: {coinbase.get('hash', 'N/A')}")
    print(f"    Is Coinbase: {coinbase.get('is_coinbase', False)}")
    print(f"    Block Height: {coinbase.get('block_height', 'N/A')}")
    print(f"    Outputs:")
    for i, out in enumerate(coinbase.get('outputs', [])):
        print(f"      Output {i}: {out.get('amount', 0)} GXC -> {out.get('address', 'N/A')[:30]}...")

print()
print("=" * 80)
print("Step 3: Mining Block")
print("-" * 80)

height = template.get('height', 1)
prev_hash = template.get('previousblockhash', '0' * 64)
difficulty = template.get('difficulty', 1.0)
timestamp = int(time.time())
merkle_root = template.get('merkleroot', '0' * 64)

print(f"Mining parameters:")
print(f"  Height: {height}")
print(f"  Previous Hash: {prev_hash[:32]}...")
print(f"  Difficulty: {difficulty}")
print(f"  Target: {'0' * int(difficulty)} (leading zeros)")
print(f"  Miner Address: {MINER_ADDRESS}")
print()

# Mine the block
nonce = 0
target = '0' * int(difficulty)
found = False
start_time = time.time()

print("Mining...")
for nonce in range(1000000):
    # Simple hash calculation
    block_data = f"{height}{prev_hash}{MINER_ADDRESS}{timestamp}{nonce}"
    hash_result = hashlib.sha256(block_data.encode()).hexdigest()
    
    if hash_result.startswith(target):
        mining_time = time.time() - start_time
        print(f"\n✓ Block mined!")
        print(f"  Nonce: {nonce}")
        print(f"  Hash: {hash_result}")
        print(f"  Mining Time: {mining_time:.2f} seconds")
        print(f"  Hash Rate: {nonce/mining_time:.0f} H/s")
        found = True
        
        # Prepare block for submission
        block = {
            "height": height,
            "hash": hash_result,
            "previousblockhash": prev_hash,
            "nonce": nonce,
            "timestamp": timestamp,
            "difficulty": difficulty,
            "miner": MINER_ADDRESS,
            "miner_address": MINER_ADDRESS,
            "merkleroot": merkle_root,
            "transactions": template.get('transactions', [])
        }
        
        print()
        print("=" * 80)
        print("Step 4: Submitting Block")
        print("-" * 80)
        print(f"Block structure:")
        print(json.dumps(block, indent=2, default=str)[:1000] + "...")
        
        result = rpc_call("submitblock", [block])
        if result is None:
            print("\n✓ Block accepted by network!")
        else:
            print(f"\n✗ Block rejected: {result}")
        
        break
    
    if nonce % 10000 == 0 and nonce > 0:
        print(f"  Attempts: {nonce:,} ({nonce/(time.time()-start_time):.0f} H/s)")

if not found:
    print("\n✗ Failed to mine block in 1,000,000 attempts")
    exit(1)

# Wait for block to be processed
print()
print("Waiting for block to be processed...")
time.sleep(2)

print()
print("=" * 80)
print("Step 5: Examining Mined Block")
print("-" * 80)

mined_block = rpc_call("getblock", [height])
if mined_block:
    print(f"Mined Block (Height {height}):")
    print(f"  Hash: {mined_block.get('hash', 'N/A')}")
    print(f"  Miner: {mined_block.get('miner', 'N/A')}")
    print(f"  Timestamp: {mined_block.get('timestamp', 'N/A')}")
    print(f"  Difficulty: {mined_block.get('difficulty', 'N/A')}")
    print(f"  Nonce: {mined_block.get('nonce', 'N/A')}")
    print(f"  Work Receipt: {mined_block.get('work_receipt', 'N/A')}")
    print(f"  Merkle Root: {mined_block.get('merkleroot', 'N/A')}")
    print(f"  Transactions: {len(mined_block.get('transactions', []))}")
    
    if mined_block.get('transactions'):
        print(f"\n  Coinbase Transaction:")
        tx = mined_block['transactions'][0]
        print(f"    TX Hash: {tx.get('hash', 'N/A')}")
        print(f"    Is Coinbase: {tx.get('is_coinbase', False)}")
        print(f"    Work Receipt: {tx.get('work_receipt', 'N/A')}")
        print(f"    Block Height: {tx.get('block_height', 'N/A')}")
        print(f"    Timestamp: {tx.get('timestamp', 'N/A')}")
        print(f"    Outputs:")
        for i, out in enumerate(tx.get('outputs', [])):
            print(f"      Output {i}: {out.get('amount', 0)} GXC -> {out.get('address', 'N/A')}")
else:
    print("Failed to get mined block")

print()
print("=" * 80)
print("Step 6: Work Receipt Explanation")
print("-" * 80)
print("""
WORK RECEIPT STRUCTURE:

A work receipt is a cryptographic proof that mining work was performed.
It's calculated as:

  work_receipt = SHA256(block_hash + miner_public_key + nonce + timestamp)

Components:
  - Block Hash: The hash of the mined block
  - Miner Public Key: The miner's public key (proves identity)
  - Nonce: The nonce that solved the puzzle
  - Timestamp: When the block was mined

Purpose:
  1. Proves the miner did the work (can't be forged)
  2. Links the reward to the actual mining work
  3. Prevents reward theft (work receipt must match block)
  4. Enables traceability (all rewards traceable to work)

Validation:
  - Node recalculates work receipt when block is submitted
  - If work receipt doesn't match, block is rejected
  - Ensures only legitimate miners get rewards
""")

print()
print("=" * 80)
print("Step 7: Checking Balance")
print("-" * 80)

balance = rpc_call("getbalance", [MINER_ADDRESS])
print(f"Balance for {MINER_ADDRESS}:")
print(f"  Current: {balance if balance else 0} GXC")
print(f"\nNote: Coinbase rewards need 6 confirmations (testnet) before spendable")
print(f"      Mine 5 more blocks to unlock the rewards")

print()
print("=" * 80)
print("Test Complete!")
print("=" * 80)
