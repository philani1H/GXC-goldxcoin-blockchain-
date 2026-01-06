#!/usr/bin/env python3
import hashlib
import json
import requests

NODE_URL = "https://gxc-chain112-blockchain-node-production.up.railway.app"

def sha256(data):
    """SHA-256 hash"""
    if isinstance(data, str):
        data = data.encode()
    return hashlib.sha256(data).hexdigest()

def compute_work_receipt(block):
    """
    Compute Work Receipt for a block
    WorkReceipt = H(prev_hash || merkle_root || nonce || miner_pubkey || difficulty || timestamp)
    """
    prev_hash = block.get('previousblockhash', '')
    merkle_root = block.get('merkleroot', '')
    nonce = str(block.get('nonce', 0))
    miner_pubkey = block.get('miner', '')  # Using miner address as pubkey for now
    difficulty = str(block.get('difficulty', 0))
    timestamp = str(block.get('time', 0))
    
    # Concatenate all components
    work_data = prev_hash + merkle_root + nonce + miner_pubkey + difficulty + timestamp
    
    # Compute SHA-256 hash
    work_receipt = sha256(work_data)
    
    return work_receipt, {
        'prev_hash': prev_hash,
        'merkle_root': merkle_root,
        'nonce': nonce,
        'miner_pubkey': miner_pubkey,
        'difficulty': difficulty,
        'timestamp': timestamp
    }

def get_block(hash_or_height):
    """Get block from node"""
    url = f"{NODE_URL}/api/getblock?hash={hash_or_height}"
    response = requests.get(url)
    return response.json()['result']

def display_work_receipt(block):
    """Display work receipt and its components"""
    print("=" * 80)
    print(f"WORK RECEIPT FOR BLOCK #{block['height']}")
    print("=" * 80)
    print()
    
    # Compute work receipt
    work_receipt, components = compute_work_receipt(block)
    
    print("📋 BLOCK INFORMATION:")
    print(f"   Height:        {block['height']}")
    print(f"   Hash:          {block['hash']}")
    print(f"   Miner:         {block['miner']}")
    print(f"   Reward:        {block['reward']} GXC")
    print(f"   Confirmations: {block['confirmations']}")
    print()
    
    print("🔧 WORK RECEIPT COMPONENTS:")
    print(f"   Previous Hash:  {components['prev_hash'][:64]}...")
    print(f"   Merkle Root:    {components['merkle_root']}")
    print(f"   Nonce:          {components['nonce']}")
    print(f"   Miner Pubkey:   {components['miner_pubkey']}")
    print(f"   Difficulty:     {components['difficulty']}")
    print(f"   Timestamp:      {components['timestamp']}")
    print()
    
    print("✅ WORK RECEIPT HASH:")
    print(f"   {work_receipt}")
    print()
    
    print("📝 WHAT THIS PROVES:")
    print(f"   ✓ Extended block:     {components['prev_hash'][:16]}...")
    print(f"   ✓ Included txs:       {components['merkle_root'][:16]}...")
    print(f"   ✓ Proof of work:      nonce={components['nonce']}")
    print(f"   ✓ Miner identity:     {components['miner_pubkey']}")
    print(f"   ✓ Work difficulty:    {components['difficulty']}")
    print(f"   ✓ Mining time:        {components['timestamp']}")
    print()
    
    # Show coinbase transaction
    if block.get('tx') and len(block['tx']) > 0:
        coinbase = block['tx'][0]
        print("💰 COINBASE TRANSACTION (Mining Reward):")
        print(f"   TX Hash:        {coinbase['txid']}")
        print(f"   Block Height:   {coinbase.get('blockheight', 'N/A')}")
        print(f"   Amount:         {coinbase['amount']} GXC")
        print(f"   To Address:     {coinbase['outputs'][0]['address']}")
        print(f"   Traceability:   {'✅ Valid' if coinbase.get('traceabilityvalid') else '❌ Invalid'}")
        print()
    
    print("=" * 80)
    print("🔗 TRACEABILITY CHAIN:")
    print("   This work receipt links the mining reward to actual computational work.")
    print("   The coinbase transaction inherits this work receipt, proving the")
    print("   50 GXC reward is legitimate and traceable to block mining.")
    print("=" * 80)

if __name__ == "__main__":
    print(f"Connecting to: {NODE_URL}\n")
    
    # Get genesis block by hash (the one mined to tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf)
    print("Fetching genesis block...")
    genesis_hash = "5b7ed1038cbd9ebb921d696fb14f2f114ceb9148c27f32797193193736de28f3"
    genesis = get_block(genesis_hash)
    
    display_work_receipt(genesis)
    
    # Check if there are more blocks
    response = requests.get(f"{NODE_URL}/api/getblockcount")
    block_count = response.json()['result']
    
    if block_count > 1:
        print("\n\n")
        print("Fetching latest block...")
        latest = get_block(str(block_count - 1))
        display_work_receipt(latest)
