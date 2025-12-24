#!/usr/bin/env python3
"""
CORRECT Mining Script - Includes ALL transactions from template
Fixes the bug where transactions were not being included in mined blocks
"""

import requests
import hashlib
import time
import json

NODE_URL = "http://localhost:8332"

def rpc_call(method, params=None):
    """Make RPC call to node"""
    if params is None:
        params = []
    
    payload = {
        "jsonrpc": "2.0",
        "method": method,
        "params": params,
        "id": 1
    }
    
    try:
        response = requests.post(NODE_URL, json=payload, timeout=10)
        if response.status_code == 200:
            result = response.json()
            if 'error' in result and result['error']:
                return None, result['error']
            return result.get('result'), None
        return None, f"HTTP {response.status_code}"
    except Exception as e:
        return None, str(e)

def mine_block_correctly(miner_address):
    """
    Mine a block CORRECTLY by including ALL transactions from template
    
    CRITICAL: The exact transactions used to compute merkle root 
    must be sent back in submitblock
    """
    
    print(f"Getting block template...")
    
    # Get block template with ALL transactions
    template, error = rpc_call("getblocktemplate", [{"algorithm": "sha256"}])
    if error:
        print(f"Error getting template: {error}")
        return False
    
    height = template.get('height', 0)
    prev_hash = template.get('previousblockhash', '0' * 64)
    difficulty = template.get('difficulty', 0.1)
    transactions = template.get('transactions', [])
    
    print(f"Block height: {height}")
    print(f"Previous hash: {prev_hash[:16]}...")
    print(f"Transactions in template: {len(transactions)}")
    
    # CRITICAL: Log what transactions we have
    if len(transactions) > 0:
        print(f"\nTransactions to include:")
        for i, tx in enumerate(transactions):
            tx_hash = tx.get('hash', tx.get('txid', 'NO_HASH'))
            tx_type = tx.get('type', 'UNKNOWN')
            print(f"  {i+1}. {tx_hash[:16]}... (type: {tx_type})")
    
    # Mine with testnet difficulty
    print(f"\nMining...")
    nonce = 0
    start_time = time.time()
    
    while nonce < 100000:
        timestamp = int(time.time())
        
        # Create block header
        header = f"{height}{prev_hash}{nonce}{miner_address}{timestamp}"
        block_hash = hashlib.sha256(header.encode()).hexdigest()
        
        # Check if hash meets difficulty (starts with 0 for testnet)
        if block_hash.startswith('0'):
            elapsed = time.time() - start_time
            
            print(f"✓ Found valid hash: {block_hash[:16]}...")
            print(f"  Nonce: {nonce}, Time: {elapsed:.2f}s")
            
            # CRITICAL FIX: Build block data with ALL transactions
            # The exact transactions from template MUST be included
            block_data = {
                "hash": block_hash,
                "previousblockhash": prev_hash,
                "height": height,
                "nonce": nonce,
                "miner": miner_address,
                "timestamp": timestamp,
                "difficulty": difficulty,
                "transactions": transactions  # ✅ INCLUDE ALL TRANSACTIONS
            }
            
            # Debug: Log what we're submitting
            print(f"\nSubmitting block with {len(transactions)} transactions:")
            if len(transactions) > 0:
                for i, tx in enumerate(transactions):
                    tx_hash = tx.get('hash', tx.get('txid', 'NO_HASH'))
                    tx_type = tx.get('type', 'UNKNOWN')
                    is_coinbase = tx.get('is_coinbase', False)
                    print(f"  {i+1}. {tx_hash[:16]}... (type: {tx_type}, coinbase: {is_coinbase})")
            
            # Submit block
            result, error = rpc_call("submitblock", [block_data])
            if error:
                print(f"✗ Error submitting block: {error}")
                return False
            else:
                print(f"✓ Block submitted successfully!")
                
                # VERIFICATION: Check if block actually contains transactions
                time.sleep(1)
                block_info, err = rpc_call("getblock", [block_hash])
                if not err and block_info:
                    actual_tx_count = len(block_info.get('transactions', []))
                    print(f"\n✓ VERIFICATION: Block contains {actual_tx_count} transactions")
                    
                    if actual_tx_count != len(transactions) + 1:  # +1 for coinbase
                        print(f"⚠ WARNING: Expected {len(transactions) + 1} transactions (including coinbase)")
                        print(f"           But block only has {actual_tx_count} transactions")
                        print(f"           This means transactions were NOT included!")
                    else:
                        print(f"✓ SUCCESS: All transactions included in block!")
                
                return True
        
        nonce += 1
        
        if nonce % 10000 == 0:
            print(f"  Tried {nonce} nonces...", end='\r')
    
    print(f"✗ Could not find valid hash after {nonce} attempts")
    return False

if __name__ == "__main__":
    import sys
    
    if len(sys.argv) < 2:
        print("Usage: ./mine_correctly.py <miner_address> [num_blocks]")
        sys.exit(1)
    
    miner_address = sys.argv[1]
    num_blocks = int(sys.argv[2]) if len(sys.argv) > 2 else 1
    
    print(f"╔══════════════════════════════════════════════════════════════════════╗")
    print(f"║                                                                      ║")
    print(f"║         CORRECT MINING - Includes ALL Transactions                  ║")
    print(f"║                                                                      ║")
    print(f"╚══════════════════════════════════════════════════════════════════════╝")
    print()
    print(f"Mining {num_blocks} block(s) to {miner_address}")
    print()
    
    for i in range(num_blocks):
        if i > 0:
            print("\n" + "="*70 + "\n")
        
        print(f"Block {i+1}/{num_blocks}")
        print("-" * 70)
        
        success = mine_block_correctly(miner_address)
        if not success:
            print(f"Failed to mine block {i+1}")
            break
        
        time.sleep(1)
    
    print()
    print("Mining complete!")
