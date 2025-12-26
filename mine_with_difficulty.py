#!/usr/bin/env python3
"""
GXC Miner - Proper mining implementation using blockchain difficulty
Mines blocks using the actual difficulty from the blockchain
"""

import os
import sys
import requests
import hashlib
import time
import json
import argparse

# Default configuration
DEFAULT_MINER_ADDRESS = "tGXC9fab7317231b966af85ac453e168c0932"
DEFAULT_RPC_URL = os.environ.get('BLOCKCHAIN_RPC_URL', os.environ.get('RAILWAY_NODE_URL', 'http://localhost:8332'))

class Colors:
    GREEN = '\033[92m'
    RED = '\033[91m'
    YELLOW = '\033[93m'
    BLUE = '\033[94m'
    CYAN = '\033[96m'
    END = '\033[0m'

def rpc_call(rpc_url, method, params=None, retries=3):
    """Make RPC call to blockchain node with retry logic"""
    if params is None:
        params = []
    
    payload = {
        "jsonrpc": "2.0",
        "method": method,
        "params": params,
        "id": 1
    }
    
    for attempt in range(retries):
        try:
            response = requests.post(rpc_url, json=payload, timeout=10)
            result = response.json()
            
            # Check for RPC errors
            if 'error' in result and result['error']:
                print(f"{Colors.RED}RPC Error: {result['error']}{Colors.END}")
                return None
            
            return result.get('result')
        except requests.Timeout:
            if attempt < retries - 1:
                print(f"{Colors.YELLOW}⏱️  Timeout, retrying... ({attempt + 1}/{retries}){Colors.END}")
                time.sleep(2 ** attempt)
                continue
            print(f"{Colors.RED}❌ RPC Timeout after {retries} attempts{Colors.END}")
            return None
        except Exception as e:
            if attempt < retries - 1:
                print(f"{Colors.YELLOW}⚠️  Error: {e}, retrying... ({attempt + 1}/{retries}){Colors.END}")
                time.sleep(1)
                continue
            print(f"{Colors.RED}❌ RPC Error: {e}{Colors.END}")
            return None

def calculate_target_zeros(difficulty):
    """
    Calculate number of leading zeros required based on difficulty
    Difficulty 1.0 = 2 leading zeros (minimum)
    Each doubling of difficulty adds ~0.3 zeros
    """
    if difficulty <= 0:
        return 2  # Minimum difficulty
    
    import math
    # Formula: zeros = 2 + log2(difficulty) * 0.3
    zeros = int(2 + math.log2(max(1.0, difficulty)) * 0.3)
    return max(2, min(zeros, 10))  # Clamp between 2 and 10

def mine_block(rpc_url, miner_address):
    """Mine a single block using proper difficulty"""
    print(f"\n{Colors.CYAN}⛏️  Mining block to: {miner_address}{Colors.END}")
    
    # Get block template
    template = rpc_call(rpc_url, "getblocktemplate", [{"minerAddress": miner_address}])
    
    if not template:
        print(f"{Colors.RED}❌ Failed to get block template{Colors.END}")
        return False
    
    if isinstance(template, dict) and 'error' in template:
        print(f"{Colors.RED}❌ Error: {template['error']}{Colors.END}")
        return False
    
    # Extract block information
    height = template.get('height', 0)
    prev_hash = template.get('previousblockhash', '0' * 64)
    difficulty = template.get('difficulty', 1.0)
    transactions = template.get('transactions', [])
    
    # Calculate required leading zeros from difficulty
    required_zeros = calculate_target_zeros(difficulty)
    target_prefix = '0' * required_zeros
    
    print(f"{Colors.BLUE}   Height: {height}{Colors.END}")
    print(f"{Colors.BLUE}   Previous: {prev_hash[:16]}...{Colors.END}")
    print(f"{Colors.BLUE}   Difficulty: {difficulty}{Colors.END}")
    print(f"{Colors.BLUE}   Required zeros: {required_zeros}{Colors.END}")
    print(f"{Colors.BLUE}   Transactions: {len(transactions)}{Colors.END}")
    print(f"{Colors.YELLOW}   Mining...{Colors.END}")
    
    # Mining loop
    nonce = 0
    start_time = time.time()
    hash_count = 0
    last_report = start_time
    
    while True:
        # Create block hash using proper block structure
        timestamp = int(time.time())
        
        # Build merkle root from transactions
        tx_hashes = [tx.get('hash', '') for tx in transactions]
        merkle_root = hashlib.sha256(''.join(tx_hashes).encode()).hexdigest() if tx_hashes else '0' * 64
        
        # Create block header string (similar to Bitcoin)
        block_header = f"{height}{prev_hash}{merkle_root}{timestamp}{difficulty}{nonce}"
        block_hash = hashlib.sha256(block_header.encode()).hexdigest()
        
        hash_count += 1
        nonce += 1
        
        # Report hashrate every 5 seconds
        current_time = time.time()
        if current_time - last_report >= 5.0:
            elapsed = current_time - start_time
            hashrate = hash_count / elapsed if elapsed > 0 else 0
            print(f"{Colors.CYAN}   Hashrate: {hashrate:.2f} H/s | Nonce: {nonce} | Elapsed: {elapsed:.1f}s{Colors.END}")
            last_report = current_time
        
        # Check if hash meets difficulty target
        if block_hash.startswith(target_prefix):
            elapsed = time.time() - start_time
            hashrate = hash_count / elapsed if elapsed > 0 else 0
            
            print(f"{Colors.GREEN}   ✓ Found valid hash: {block_hash[:32]}...{Colors.END}")
            print(f"{Colors.GREEN}   ✓ Nonce: {nonce} | Attempts: {hash_count} | Time: {elapsed:.2f}s{Colors.END}")
            print(f"{Colors.GREEN}   ✓ Average hashrate: {hashrate:.2f} H/s{Colors.END}")
            
            # Prepare block data for submission
            block_data = {
                'hash': block_hash,
                'previousblockhash': prev_hash,
                'height': height,
                'nonce': nonce - 1,  # Subtract 1 because we incremented after finding
                'miner': miner_address,
                'minerAddress': miner_address,
                'timestamp': timestamp,
                'difficulty': difficulty,
                'merkleroot': merkle_root,
                'transactions': transactions
            }
            
            # Submit block
            print(f"{Colors.YELLOW}   Submitting block...{Colors.END}")
            result = rpc_call(rpc_url, "submitblock", [block_data])
            
            # Wait for block to be processed
            time.sleep(2)
            
            # Verify block was accepted
            current_height = rpc_call(rpc_url, "getblockcount")
            if current_height and current_height >= height:
                print(f"\n{Colors.GREEN}{'='*70}{Colors.END}")
                print(f"{Colors.GREEN}🎉 BLOCK MINED SUCCESSFULLY!{Colors.END}")
                print(f"{Colors.GREEN}{'='*70}{Colors.END}")
                print(f"{Colors.CYAN}   Height: {height}{Colors.END}")
                print(f"{Colors.CYAN}   Hash: {block_hash}{Colors.END}")
                print(f"{Colors.CYAN}   Nonce: {nonce - 1}{Colors.END}")
                print(f"{Colors.CYAN}   Difficulty: {difficulty} ({required_zeros} zeros){Colors.END}")
                print(f"{Colors.CYAN}   Transactions: {len(transactions)}{Colors.END}")
                print(f"{Colors.CYAN}   Mining time: {elapsed:.2f} seconds{Colors.END}")
                print(f"{Colors.CYAN}   Average hashrate: {hashrate:.2f} H/s{Colors.END}")
                print(f"{Colors.CYAN}   Miner: {miner_address}{Colors.END}")
                
                # Get reward info
                reward = template.get('coinbasevalue', template.get('block_reward', 50.0))
                if isinstance(reward, (int, float)):
                    if reward > 1000000:  # Likely in satoshis
                        reward = reward / 100000000.0
                    print(f"{Colors.GREEN}   💰 Reward: {reward} GXC{Colors.END}")
                
                print(f"{Colors.GREEN}{'='*70}{Colors.END}\n")
                return True
            else:
                print(f"{Colors.RED}   ❌ Block submission may have failed{Colors.END}")
                print(f"{Colors.YELLOW}   Current height: {current_height}, Expected: {height}{Colors.END}")
                return False

def main():
    parser = argparse.ArgumentParser(description='GXC Blockchain Miner with Difficulty Support')
    parser.add_argument('--address', type=str, default=DEFAULT_MINER_ADDRESS,
                        help='Miner address to receive rewards')
    parser.add_argument('--rpc-url', type=str, default=DEFAULT_RPC_URL,
                        help='RPC URL of the blockchain node')
    parser.add_argument('--continuous', action='store_true',
                        help='Mine continuously (default: mine one block)')
    parser.add_argument('--blocks', type=int, default=1,
                        help='Number of blocks to mine (default: 1)')
    
    args = parser.parse_args()
    
    print(f"{Colors.CYAN}{'='*70}{Colors.END}")
    print(f"{Colors.CYAN}GXC Blockchain Miner - Difficulty-Based Mining{Colors.END}")
    print(f"{Colors.CYAN}{'='*70}{Colors.END}")
    print(f"{Colors.BLUE}Miner Address: {args.address}{Colors.END}")
    print(f"{Colors.BLUE}RPC URL: {args.rpc_url}{Colors.END}")
    print(f"{Colors.BLUE}Mode: {'Continuous' if args.continuous else f'{args.blocks} block(s)'}{Colors.END}")
    print(f"{Colors.CYAN}{'='*70}{Colors.END}")
    
    # Test connection
    info = rpc_call(args.rpc_url, "getblockchaininfo")
    if not info:
        print(f"{Colors.RED}❌ Cannot connect to blockchain node at {args.rpc_url}{Colors.END}")
        print(f"{Colors.YELLOW}Make sure the node is running and accessible{Colors.END}")
        return 1
    
    print(f"{Colors.GREEN}✓ Connected to blockchain{Colors.END}")
    print(f"{Colors.BLUE}  Chain height: {info.get('blocks', 0)}{Colors.END}")
    print(f"{Colors.BLUE}  Difficulty: {info.get('difficulty', 1.0)}{Colors.END}")
    
    # Mining loop
    blocks_mined = 0
    try:
        if args.continuous:
            print(f"\n{Colors.YELLOW}Mining continuously... Press Ctrl+C to stop{Colors.END}")
            while True:
                if mine_block(args.rpc_url, args.address):
                    blocks_mined += 1
                    print(f"{Colors.GREEN}Total blocks mined: {blocks_mined}{Colors.END}")
                time.sleep(1)  # Small delay between blocks
        else:
            for i in range(args.blocks):
                print(f"\n{Colors.CYAN}Mining block {i + 1}/{args.blocks}{Colors.END}")
                if mine_block(args.rpc_url, args.address):
                    blocks_mined += 1
                else:
                    print(f"{Colors.RED}Failed to mine block {i + 1}{Colors.END}")
                    break
                
                if i < args.blocks - 1:
                    time.sleep(1)  # Small delay between blocks
    
    except KeyboardInterrupt:
        print(f"\n{Colors.YELLOW}Mining interrupted by user{Colors.END}")
    
    print(f"\n{Colors.CYAN}{'='*70}{Colors.END}")
    print(f"{Colors.GREEN}Mining session complete{Colors.END}")
    print(f"{Colors.BLUE}Blocks mined: {blocks_mined}{Colors.END}")
    print(f"{Colors.CYAN}{'='*70}{Colors.END}")
    
    return 0

if __name__ == "__main__":
    sys.exit(main())
