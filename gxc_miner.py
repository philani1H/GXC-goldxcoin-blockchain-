#!/usr/bin/env python3
"""
GXC Third-Party Miner
A simple Python miner that uses the GXC RPC API
"""

import requests
import hashlib
import time
import json
import sys

# Configuration
NODE_URL = "http://localhost:8332"
MINER_ADDRESS = "GXC9fab7317231b966af85ac453e168c0932"  # Change this to your address

def get_block_template():
    """Get block template from node"""
    payload = {
        "jsonrpc": "2.0",
        "id": 1,
        "method": "getblocktemplate",
        "params": [{
            "minerAddress": MINER_ADDRESS
        }]
    }
    
    try:
        response = requests.post(f"{NODE_URL}/api/getblocktemplate", json=payload, timeout=10)
        response.raise_for_status()
        result = response.json()
        
        if "error" in result:
            raise Exception(f"RPC Error: {result['error']}")
        
        return result["result"]
    except requests.exceptions.RequestException as e:
        raise Exception(f"Failed to get block template: {e}")

def calculate_hash(block_data, nonce):
    """Calculate block hash using double SHA256"""
    # Serialize block data
    data = f"{block_data['previousblockhash']}{block_data['height']}{block_data['timestamp']}{nonce}"
    
    # Double SHA256
    first_hash = hashlib.sha256(data.encode()).digest()
    second_hash = hashlib.sha256(first_hash).hexdigest()
    
    return second_hash

def check_difficulty(hash_value, difficulty):
    """Check if hash meets difficulty target"""
    # Count leading zeros
    leading_zeros = len(hash_value) - len(hash_value.lstrip('0'))
    
    # Calculate required zeros based on difficulty
    required_zeros = int(difficulty / 1000) + 4
    
    return leading_zeros >= required_zeros

def submit_block(block_data, nonce, block_hash):
    """Submit mined block to node"""
    payload = {
        "jsonrpc": "2.0",
        "id": 1,
        "method": "submitblock",
        "params": [{
            "height": block_data["height"],
            "hash": block_hash,
            "previousblockhash": block_data["previousblockhash"],
            "nonce": nonce,
            "miner": MINER_ADDRESS,
            "timestamp": block_data["timestamp"],
            "difficulty": block_data["difficulty"]
        }]
    }
    
    try:
        response = requests.post(f"{NODE_URL}/api/submitblock", json=payload, timeout=10)
        response.raise_for_status()
        return response.json()
    except requests.exceptions.RequestException as e:
        raise Exception(f"Failed to submit block: {e}")

def format_hash_rate(hash_rate):
    """Format hash rate for display"""
    if hash_rate >= 1e9:
        return f"{hash_rate/1e9:.2f} GH/s"
    elif hash_rate >= 1e6:
        return f"{hash_rate/1e6:.2f} MH/s"
    elif hash_rate >= 1e3:
        return f"{hash_rate/1e3:.2f} KH/s"
    else:
        return f"{hash_rate:.2f} H/s"

def mine():
    """Main mining loop"""
    print("=" * 60)
    print("GXC Third-Party Miner (Python)")
    print("=" * 60)
    print(f"Miner address: {MINER_ADDRESS}")
    print(f"Node URL: {NODE_URL}")
    print()
    
    # Check if node is accessible
    try:
        response = requests.get(f"{NODE_URL}/health", timeout=5)
        print("✅ Node is accessible")
    except:
        print("❌ ERROR: Cannot connect to node")
        print(f"   Make sure the node is running at {NODE_URL}")
        print("   Start node with: ./gxc-node --testnet")
        sys.exit(1)
    
    print()
    print("Starting mining...")
    print("Press Ctrl+C to stop")
    print()
    
    nonce = 0
    hash_count = 0
    start_time = time.time()
    blocks_found = 0
    last_stats_time = start_time
    
    while True:
        try:
            # Get block template
            template = get_block_template()
            print(f"Mining block {template['height']} (difficulty: {template['difficulty']}, reward: {template.get('coinbasevalue', 50)} GXC)")
            
            # Mine block
            template_start_time = time.time()
            template_nonce = 0
            
            while True:
                block_hash = calculate_hash(template, nonce)
                hash_count += 1
                template_nonce += 1
                
                # Print hash rate every 5 seconds
                current_time = time.time()
                if current_time - last_stats_time >= 5:
                    elapsed = current_time - start_time
                    hash_rate = hash_count / elapsed
                    print(f"Hash rate: {format_hash_rate(hash_rate)} | Nonce: {nonce} | Blocks found: {blocks_found}")
                    last_stats_time = current_time
                
                # Check if hash meets difficulty
                if check_difficulty(block_hash, template['difficulty']):
                    print()
                    print(f"✅ Found valid block!")
                    print(f"   Hash: {block_hash[:32]}...")
                    print(f"   Nonce: {nonce}")
                    print(f"   Attempts: {template_nonce}")
                    
                    # Submit block
                    result = submit_block(template, nonce, block_hash)
                    
                    if result.get("result", {}).get("success"):
                        blocks_found += 1
                        print(f"✅ Block accepted! Height: {template['height']}")
                        print(f"   Reward: {template.get('coinbasevalue', 50)} GXC")
                        print(f"   Total blocks found: {blocks_found}")
                    else:
                        print(f"❌ Block rejected: {result.get('result', {}).get('message', 'Unknown error')}")
                    
                    print()
                    # Get new template
                    break
                
                nonce += 1
                
                # Get new template every 100000 hashes or 60 seconds
                if template_nonce >= 100000 or (time.time() - template_start_time) >= 60:
                    break
        
        except KeyboardInterrupt:
            print()
            print("=" * 60)
            print("Stopping miner...")
            elapsed = time.time() - start_time
            avg_hash_rate = hash_count / elapsed if elapsed > 0 else 0
            print(f"Total runtime: {elapsed:.0f} seconds")
            print(f"Total hashes: {hash_count}")
            print(f"Average hash rate: {format_hash_rate(avg_hash_rate)}")
            print(f"Blocks found: {blocks_found}")
            print("=" * 60)
            break
        
        except Exception as e:
            print(f"❌ Error: {e}")
            print("Retrying in 5 seconds...")
            time.sleep(5)

if __name__ == "__main__":
    # Check if custom address provided
    if len(sys.argv) > 1:
        MINER_ADDRESS = sys.argv[1]
        print(f"Using custom miner address: {MINER_ADDRESS}")
    
    mine()
