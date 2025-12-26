#!/usr/bin/env python3
"""
Mine blocks locally and create transaction to send to pool address
"""
import json
import hashlib
import time
from datetime import datetime

POOL_ADDRESS = "tGXC4VoV6DfSnM7YwTLCciBUzizARg2H64T7n"
TEMP_MINER_ADDRESS = "tGXC9fab7317231b966af85ac453e168c0932"  # Temporary mining address

# Simple blockchain simulation
blockchain = []
utxos = {}  # txhash_outputindex -> {address, amount, spent}

def create_genesis_block():
    """Create genesis block"""
    block = {
        "height": 0,
        "timestamp": int(time.time()),
        "previousHash": "0" * 64,
        "transactions": [],
        "nonce": 0
    }
    
    block_str = json.dumps(block, sort_keys=True)
    block["hash"] = hashlib.sha256(block_str.encode()).hexdigest()
    
    return block

def create_coinbase_transaction(miner_address, height, reward=50.0):
    """Create coinbase transaction"""
    tx = {
        "type": "coinbase",
        "inputs": [],
        "outputs": [{
            "address": miner_address,
            "amount": reward
        }],
        "timestamp": int(time.time()),
        "height": height
    }
    
    tx_str = json.dumps(tx, sort_keys=True)
    tx["txid"] = hashlib.sha256(tx_str.encode()).hexdigest()
    
    return tx

def mine_block(previous_hash, height, miner_address):
    """Mine a single block"""
    coinbase_tx = create_coinbase_transaction(miner_address, height)
    
    block = {
        "height": height,
        "timestamp": int(time.time()),
        "previousHash": previous_hash,
        "transactions": [coinbase_tx],
        "nonce": 0
    }
    
    # Simple mining (no difficulty)
    block_str = json.dumps(block, sort_keys=True)
    block["hash"] = hashlib.sha256(block_str.encode()).hexdigest()
    
    # Add UTXO
    utxo_key = f"{coinbase_tx['txid']}_0"
    utxos[utxo_key] = {
        "address": miner_address,
        "amount": 50.0,
        "spent": False,
        "txid": coinbase_tx['txid'],
        "outputIndex": 0
    }
    
    return block

def create_transfer_transaction(from_address, to_address, amount, utxo_key):
    """Create transfer transaction"""
    utxo = utxos.get(utxo_key)
    
    if not utxo or utxo["spent"]:
        raise Exception(f"UTXO {utxo_key} not available")
    
    if utxo["amount"] < amount:
        raise Exception(f"Insufficient funds in UTXO")
    
    tx = {
        "type": "transfer",
        "inputs": [{
            "txHash": utxo["txid"],
            "outputIndex": utxo["outputIndex"],
            "amount": utxo["amount"]
        }],
        "outputs": [
            {
                "address": to_address,
                "amount": amount
            }
        ],
        "timestamp": int(time.time())
    }
    
    # Add change output if needed
    change = utxo["amount"] - amount - 0.001  # 0.001 fee
    if change > 0:
        tx["outputs"].append({
            "address": from_address,
            "amount": change
        })
    
    tx_str = json.dumps(tx, sort_keys=True)
    tx["txid"] = hashlib.sha256(tx_str.encode()).hexdigest()
    
    # Mark UTXO as spent
    utxos[utxo_key]["spent"] = True
    
    # Create new UTXOs
    for i, output in enumerate(tx["outputs"]):
        new_utxo_key = f"{tx['txid']}_{i}"
        utxos[new_utxo_key] = {
            "address": output["address"],
            "amount": output["amount"],
            "spent": False,
            "txid": tx["txid"],
            "outputIndex": i
        }
    
    return tx

def main():
    print("="*70)
    print("GXC Local Mining and Transfer to Pool")
    print("="*70)
    print(f"Pool Address: {POOL_ADDRESS}")
    print(f"Temp Miner: {TEMP_MINER_ADDRESS}")
    print("="*70 + "\n")
    
    # Create genesis
    print("[1/4] Creating genesis block...")
    genesis = create_genesis_block()
    blockchain.append(genesis)
    print(f"  ✅ Genesis block created: {genesis['hash'][:16]}...")
    
    # Mine 5 blocks to temp address
    print("\n[2/4] Mining 5 blocks to temporary address...")
    for i in range(5):
        block = mine_block(
            blockchain[-1]["hash"],
            len(blockchain),
            TEMP_MINER_ADDRESS
        )
        blockchain.append(block)
        print(f"  ✅ Block {block['height']} mined: {block['hash'][:16]}... (Reward: 50 GXC)")
    
    # Show available UTXOs
    print("\n[3/4] Available UTXOs for temp miner:")
    available_utxos = {k: v for k, v in utxos.items() 
                       if v["address"] == TEMP_MINER_ADDRESS and not v["spent"]}
    
    total_balance = sum(u["amount"] for u in available_utxos.values())
    print(f"  Total balance: {total_balance} GXC")
    print(f"  UTXOs: {len(available_utxos)}")
    
    for utxo_key, utxo in list(available_utxos.items())[:3]:
        print(f"    - {utxo_key[:32]}... : {utxo['amount']} GXC")
    
    # Create transfer to pool
    print(f"\n[4/4] Creating transfer to pool address...")
    transfer_amount = 40.0  # Send 40 GXC to pool (less than 50 GXC UTXO)
    
    # Use first UTXO
    utxo_keys = list(available_utxos.keys())[:1]
    
    print(f"  Sending {transfer_amount} GXC to pool...")
    print(f"  Using UTXO:")
    for key in utxo_keys:
        print(f"    - {key[:32]}... : {utxos[key]['amount']} GXC")
    
    # Create transaction using first UTXO
    tx = create_transfer_transaction(
        TEMP_MINER_ADDRESS,
        POOL_ADDRESS,
        transfer_amount,
        utxo_keys[0]
    )
    
    print(f"  ✅ Transaction created: {tx['txid'][:16]}...")
    print(f"     From: {TEMP_MINER_ADDRESS}")
    print(f"     To: {POOL_ADDRESS}")
    print(f"     Amount: {transfer_amount} GXC")
    
    # Show pool UTXOs
    print("\n" + "="*70)
    print("POOL ADDRESS UTXOs")
    print("="*70)
    
    pool_utxos = {k: v for k, v in utxos.items() 
                  if v["address"] == POOL_ADDRESS and not v["spent"]}
    
    pool_balance = sum(u["amount"] for u in pool_utxos.values())
    
    print(f"Pool Address: {POOL_ADDRESS}")
    print(f"Balance: {pool_balance} GXC")
    print(f"UTXOs: {len(pool_utxos)}")
    print()
    
    for utxo_key, utxo in pool_utxos.items():
        print(f"UTXO: {utxo_key}")
        print(f"  TxID: {utxo['txid']}")
        print(f"  Output Index: {utxo['outputIndex']}")
        print(f"  Amount: {utxo['amount']} GXC")
        print()
    
    # Save transaction data for Railway node
    print("="*70)
    print("TRANSACTION DATA FOR RAILWAY NODE")
    print("="*70)
    
    tx_data = {
        "transaction": tx,
        "pool_address": POOL_ADDRESS,
        "pool_utxos": [
            {
                "txid": utxo["txid"],
                "outputIndex": utxo["outputIndex"],
                "amount": utxo["amount"]
            }
            for utxo in pool_utxos.values()
        ]
    }
    
    with open("pool_transaction_data.json", "w") as f:
        json.dump(tx_data, f, indent=2)
    
    print("✅ Transaction data saved to: pool_transaction_data.json")
    print()
    print("To send from pool on Railway node, you need:")
    print(f"1. Pool address: {POOL_ADDRESS}")
    print(f"2. Available UTXO: {list(pool_utxos.keys())[0] if pool_utxos else 'None'}")
    print(f"3. Balance: {pool_balance} GXC")
    print()
    print("Note: This is a LOCAL simulation. To use on Railway:")
    print("  1. Mine blocks on Railway node to the pool address")
    print("  2. Query UTXOs using: listunspent RPC method")
    print("  3. Create and sign transaction with pool's private key")
    print("  4. Submit transaction using: sendrawtransaction RPC method")
    print("="*70)

if __name__ == "__main__":
    main()
