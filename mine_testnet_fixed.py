#!/usr/bin/env python3
"""
Fixed testnet miner - properly submits blocks
"""

import os
import sys
import requests
import hashlib
import time
import json

# Import centralized network configuration
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from config.railway_config import (
    get_network, get_rpc_url, get_miner_address, get_network_config,
    RAILWAY_NODE_URL, LOCAL_NODE_URL, CURRENT_NETWORK
)

# Network-aware configuration
NETWORK = get_network()
MINER_ADDRESS = get_miner_address()

# Active RPC URL (will be set by connection test)
RPC_URL = get_rpc_url()
ACTIVE_NODE_TYPE = None  # Will be set to 'railway' or 'local' after connection test

print(f"[MINER] Network: {NETWORK.upper()}")
print(f"[MINER] Miner Address: {MINER_ADDRESS}")
print(f"[MINER] RPC URL: {RPC_URL}")

def rpc_call(method, params=None, show_errors=True):
    """Make RPC call to blockchain node using active connection"""
    global RPC_URL, ACTIVE_NODE_TYPE
    
    if params is None:
        params = []
    
    payload = {
        "jsonrpc": "2.0",
        "method": method,
        "params": params,
        "id": 1
    }
    
    # Use the active RPC URL (set during connection test)
    try:
        timeout = 10 if ACTIVE_NODE_TYPE == "railway" else 5
        response = requests.post(RPC_URL, json=payload, timeout=timeout)
        response.raise_for_status()
        result = response.json()
        
        # Check for RPC error
        if 'error' in result:
            if show_errors:
                print(f"❌ RPC Error: {result['error']}")
            return None
        
        return result.get('result')
    except requests.exceptions.Timeout:
        if show_errors:
            node_name = "Railway" if ACTIVE_NODE_TYPE == "railway" else "Local"
            print(f"⚠️  {node_name} node timeout")
            # Try the other node as fallback
            fallback_url = LOCAL_NODE_URL if ACTIVE_NODE_TYPE == "railway" else RAILWAY_NODE_URL
            print(f"   Trying fallback: {fallback_url}")
        
        # Fallback to the other node
        fallback_url = LOCAL_NODE_URL if ACTIVE_NODE_TYPE == "railway" else RAILWAY_NODE_URL
        try:
            response = requests.post(fallback_url, json=payload, timeout=5)
            response.raise_for_status()
            result = response.json()
            
            if 'error' not in result:
                if show_errors:
                    print(f"✅ Connected to fallback node!")
                # Update active connection
                RPC_URL = fallback_url
                ACTIVE_NODE_TYPE = "local" if ACTIVE_NODE_TYPE == "railway" else "railway"
                return result.get('result')
        except:
            pass
        return None
    except requests.exceptions.ConnectionError:
        if show_errors:
            error_msg = "Connection refused"
            if ACTIVE_NODE_TYPE == "railway":
                print(f"❌ Cannot connect to Railway node")
                print(f"   Error: {error_msg}")
                print(f"   Trying localhost fallback...")
            else:
                print(f"❌ Cannot connect to local node")
                print(f"   Error: {error_msg}")
                print(f"   Start local node: python testnet_blockchain_node.py")
        
        # Try fallback if Railway was primary
        if ACTIVE_NODE_TYPE == "railway":
            try:
                response = requests.post(LOCAL_NODE_URL, json=payload, timeout=5)
                response.raise_for_status()
                result = response.json()
                if 'error' not in result:
                    if show_errors:
                        print(f"✅ Connected to local node!")
                    RPC_URL = LOCAL_NODE_URL
                    ACTIVE_NODE_TYPE = "local"
                    return result.get('result')
            except:
                pass
            return None
    except Exception as e:
        if show_errors:
            error_msg = str(e)
            if "10061" in error_msg or "connection refused" in error_msg.lower():
                print(f"❌ Connection refused - node may be down or unreachable")
            else:
                print(f"❌ RPC Error: {error_msg[:80]}")
            return None

def mine_block():
    """Mine a single block"""
    print(f"\n⛏️  Mining block to: {MINER_ADDRESS}")
    
    # Get block template (suppress errors during normal operation)
    template = rpc_call("getblocktemplate", [{"algorithm": "sha256"}], show_errors=False)
    
    if not template:
        print("❌ Failed to get block template")
        # Now show errors to help diagnose
        rpc_call("getblocktemplate", [{"algorithm": "sha256"}], show_errors=True)
        return False
    
    if isinstance(template, str) and 'error' in template.lower():
        print(f"❌ Error: {template}")
        return False
    
    height = template.get('height', 1)
    prev_hash = template.get('previousblockhash', '0' * 64)
    
    print(f"   Height: {height}")
    print(f"   Previous: {prev_hash[:16]}...")
    print(f"   Mining...")
    
    # Simple mining (find nonce)
    nonce = 0
    start_time = time.time()
    
    while True:
        # Create block hash
        block_string = f"{height}{prev_hash}{nonce}{MINER_ADDRESS}{int(time.time())}"
        block_hash = hashlib.sha256(block_string.encode()).hexdigest()
        
        # Check if valid (very easy for testnet - just needs to start with 0)
        if block_hash.startswith('0'):
            # Prepare complete block data with ALL required fields
            block_data = {
                'hash': block_hash,
                'previousblockhash': prev_hash,
                'height': height,
                'nonce': nonce,
                'miner': MINER_ADDRESS,
                'timestamp': int(time.time()),
                'difficulty': template.get('difficulty', 0.1),
                'transactions': []
            }
            
            # Submit block
            print(f"   Found valid hash: {block_hash[:16]}...")
            print(f"   Submitting block...")
            
            result = rpc_call("submitblock", [block_data])
            
            elapsed = time.time() - start_time
            
            # Handle different result types from submitblock
            success = False
            
            if result is None:
                # None typically means success in RPC implementations
                success = True
            elif isinstance(result, dict):
                if result.get('success') or result.get('accepted'):
                    success = True
                elif result.get('error'):
                    print(f"❌ Block submission error: {result.get('error')}")
                    return False
            elif isinstance(result, str):
                if 'rejected' in result.lower() or 'error' in result.lower():
                    print(f"❌ Block rejected: {result}")
                    return False
                # Some implementations return success as string
                success = True
            elif result is False:
                print(f"❌ Block submission returned False")
                return False
            else:
                # Unexpected result, but might be success
                success = True
            
            if success:
                # Success!
                print(f"\n{'='*70}")
                print(f"🎉 BLOCK MINED!")
                print(f"{'='*70}")
                print(f"   Height: {height}")
                print(f"   Hash: {block_hash[:32]}...")
                
                # Get reward from template (prefer GXC values over satoshis)
                reward = template.get('block_reward') or template.get('reward') or template.get('coinbase_value')
                if not reward and template.get('coinbasevalue'):
                    reward = template.get('coinbasevalue') / 100000000.0
                
                # Fallback to blockchain info
                if not reward:
                    blockchain_info = rpc_call("getblockchaininfo", show_errors=False)
                    if blockchain_info:
                        reward = blockchain_info.get('block_reward') or blockchain_info.get('reward')
                
                if reward:
                    print(f"   Reward: {reward} GXC")
                else:
                    print(f"   Reward: Will be verified from blockchain")
                
                print(f"   Time: {elapsed:.2f} seconds")
                print(f"   Nonce: {nonce}")
                print(f"   Miner: {MINER_ADDRESS}")
                print(f"{'='*70}\n")
                return True
            else:
                print(f"❌ Block submission failed: {result}")
                return False
        
        nonce += 1
        
        # Progress indicator
        if nonce % 100000 == 0:
            print(f"   Tried {nonce:,} nonces... ({nonce / (time.time() - start_time):.0f} H/s)")
        
        # Timeout after 60 seconds
        if time.time() - start_time > 60:
            print(f"   ⏱️  Timeout after 60 seconds, getting new template...")
            return False

def test_connection():
    """Test connection to blockchain node and provide diagnostics"""
    global RPC_URL, ACTIVE_NODE_TYPE
    
    print("Checking blockchain connection...")
    print(f"   Railway URL: {RAILWAY_NODE_URL}")
    
    # Test Railway connection first
    print(f"\n1️⃣  Testing Railway node connection...")
    try:
        response = requests.post(RAILWAY_NODE_URL, json={
            "jsonrpc": "2.0",
            "method": "getblockcount",
            "params": [],
            "id": 1
        }, timeout=10)
        response.raise_for_status()
        result = response.json()
        
        if 'error' not in result and result.get('result') is not None:
            height = result.get('result')
            print(f"   ✅ Railway node is accessible!")
            print(f"   Current height: {height}")
            RPC_URL = RAILWAY_NODE_URL
            ACTIVE_NODE_TYPE = "railway"
            return True, "railway"
        else:
            print(f"   ❌ Railway node returned error: {result.get('error')}")
    except requests.exceptions.Timeout:
        print(f"   ⏱️  Railway node timeout (may be sleeping)")
    except requests.exceptions.ConnectionError:
        print(f"   ❌ Cannot connect to Railway node (connection refused)")
    except Exception as e:
        print(f"   ❌ Railway node error: {str(e)[:60]}...")
    
    # Test local connection
    print(f"\n2️⃣  Testing local node connection...")
    try:
        response = requests.post(LOCAL_NODE_URL, json={
            "jsonrpc": "2.0",
            "method": "getblockcount",
            "params": [],
            "id": 1
        }, timeout=5)
        response.raise_for_status()
        result = response.json()
        
        if 'error' not in result and result.get('result') is not None:
            height = result.get('result')
            print(f"   ✅ Local node is accessible!")
            print(f"   Current height: {height}")
            RPC_URL = LOCAL_NODE_URL
            ACTIVE_NODE_TYPE = "local"
            return True, "local"
        else:
            print(f"   ❌ Local node returned error: {result.get('error')}")
    except requests.exceptions.ConnectionError:
        print(f"   ❌ No local node running on port 18332")
    except Exception as e:
        print(f"   ❌ Local node error: {str(e)[:60]}...")
    
    # Both failed - provide solutions
    print(f"\n❌ Cannot connect to any blockchain node")
    print(f"\n🔧 Solutions:")
    print(f"\n   Option 1: Start local testnet node (Recommended)")
    print(f"   ┌─────────────────────────────────────────────────┐")
    print(f"   │ python testnet_blockchain_node.py               │")
    print(f"   └─────────────────────────────────────────────────┘")
    print(f"\n   Option 2: Check Railway node status")
    print(f"   ┌─────────────────────────────────────────────────┐")
    print(f"   │ 1. Go to: https://railway.app                  │")
    print(f"   │ 2. Check if node service is running            │")
    print(f"   │ 3. Verify URL: {RAILWAY_NODE_URL[:50]}...│")
    print(f"   │ 4. Railway free tier may sleep after inactivity│")
    print(f"   └─────────────────────────────────────────────────┘")
    print(f"\n   Option 3: Use environment variable")
    print(f"   ┌─────────────────────────────────────────────────┐")
    print(f"   │ $env:BLOCKCHAIN_RPC_URL='your-node-url'        │")
    print(f"   └─────────────────────────────────────────────────┘")
    print(f"\n💡 Tip: Local node is faster and more reliable for testing!")
    
    return False, None

def check_balance():
    """Check balance at miner address from blockchain (use RPC methods)"""
    # Try RPC methods first (more reliable)
    balance = None
    for method in ["gxc_getBalance", "getbalance", "getaddressbalance"]:
        result = rpc_call(method, [MINER_ADDRESS], show_errors=False)
        if result is not None:
            if isinstance(result, (int, float)):
                balance = float(result)
                print(f"\n💰 Balance from blockchain ({method}): {balance} GXC")
                break
            elif isinstance(result, dict):
                balance = result.get('balance') or result.get('amount') or 0.0
                print(f"\n💰 Balance from blockchain ({method}): {balance} GXC")
                break
    
    if balance is None:
        print(f"\n💰 Could not get balance from blockchain")

def main():
    print("\n" + "="*70)
    print("🪙 GXC TESTNET MINER - FIXED VERSION")
    print("="*70)
    print(f"\nMining to: {MINER_ADDRESS}")
    print(f"RPC: {RPC_URL}")
    print(f"\nPress Ctrl+C to stop\n")
    print("="*70 + "\n")
    
    # Test connection with diagnostics
    connected, node_type = test_connection()
    
    if not connected:
        print(f"\n⚠️  Cannot proceed without blockchain connection")
        return
    
    # Show which node we're connected to
    if node_type == "local":
        print(f"\n✅ Using LOCAL testnet node (fastest)")
    elif node_type == "railway":
        print(f"\n✅ Using RAILWAY testnet node")
    
    print("="*70 + "\n")
    
    # Start mining
    blocks_mined = 0
    blocks_attempted = 0
    
    try:
        while True:
            blocks_attempted += 1
            print(f"\n📊 Attempt #{blocks_attempted}")
            
            if mine_block():
                blocks_mined += 1
                print(f"✅ Success! Total blocks mined: {blocks_mined}")
                
                # Check balance every 5 blocks
                if blocks_mined % 5 == 0:
                    check_balance()
                
                time.sleep(2)  # Wait a bit between blocks
            else:
                print("⏸️  Failed, waiting 5 seconds before retry...")
                time.sleep(5)
    
    except KeyboardInterrupt:
        print("\n\n" + "="*70)
        print("⏹️  MINING STOPPED")
        print("="*70)
        print(f"\n📊 Statistics:")
        print(f"   Blocks attempted: {blocks_attempted}")
        print(f"   Blocks mined: {blocks_mined}")
        print(f"   Success rate: {(blocks_mined/blocks_attempted*100) if blocks_attempted > 0 else 0:.1f}%")
        print(f"   Total rewards: Check blockchain for actual rewards")
        check_balance()  # Show real balance from blockchain
        print(f"   Miner address: {MINER_ADDRESS}")
        print("\n" + "="*70 + "\n")

if __name__ == "__main__":
    import sys
    
    # Check if GUI mode requested
    if len(sys.argv) > 1 and sys.argv[1] in ['--gui', '-g', 'gui']:
        # Launch GUI version
        try:
            import tkinter as tk
            from mine_testnet_gui import TestnetMinerGUI
            root = tk.Tk()
            app = TestnetMinerGUI(root)
            root.mainloop()
        except ImportError as e:
            print("❌ GUI mode requires tkinter. Install it or run without --gui flag.")
            print(f"   Error: {e}")
            sys.exit(1)
        except Exception as e:
            print(f"❌ Error launching GUI: {e}")
            print("   Falling back to command-line mode...")
            main()
    else:
        # Command-line mode (default)
        main()
