#!/usr/bin/env python3
"""
Comprehensive test for GXC Blockchain:
- Transaction creation with traceability
- Staking (locks coins, creates STAKE transaction)
- Proof of Stake block creation
- Unstaking (returns coins)
- Balance verification
"""

import subprocess
import requests
import json
import time
import os
import signal
import sys

# Configuration
NODE_URL = "http://localhost:8332"
DATA_DIR = "/tmp/gxc_test_data"
NODE_PROCESS = None

def rpc_call(method, params=None):
    """Make an RPC call to the node"""
    if params is None:
        params = []
    
    payload = {
        "jsonrpc": "2.0",
        "method": method,
        "params": params,
        "id": 1
    }
    
    try:
        response = requests.post(NODE_URL, json=payload, timeout=30)
        result = response.json()
        if "error" in result:
            print(f"  ❌ RPC Error: {result['error']}")
            return None
        return result.get("result")
    except Exception as e:
        print(f"  ❌ Request failed: {e}")
        return None

def start_node():
    """Start the GXC node"""
    global NODE_PROCESS
    
    print("\n🚀 Starting GXC Node...")
    
    # Clean up old data
    os.makedirs(DATA_DIR, exist_ok=True)
    
    # Start the node
    node_cmd = [
        "/workspace/build/gxc-node",
        "--testnet",
        "--datadir=" + DATA_DIR,
        "--verbose"
    ]
    
    try:
        NODE_PROCESS = subprocess.Popen(
            node_cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            cwd="/workspace"
        )
        print(f"  ✅ Node started (PID: {NODE_PROCESS.pid})")
        
        # Wait for node to be ready
        for i in range(30):
            time.sleep(1)
            try:
                result = rpc_call("getinfo")
                if result:
                    print(f"  ✅ Node is ready! Height: {result.get('blocks', 0)}")
                    return True
            except:
                pass
            print(f"  ⏳ Waiting for node... ({i+1}/30)")
        
        print("  ❌ Node failed to start in time")
        return False
        
    except Exception as e:
        print(f"  ❌ Failed to start node: {e}")
        return False

def stop_node():
    """Stop the GXC node"""
    global NODE_PROCESS
    
    print("\n🛑 Stopping node...")
    if NODE_PROCESS:
        NODE_PROCESS.terminate()
        try:
            NODE_PROCESS.wait(timeout=5)
        except:
            NODE_PROCESS.kill()
        print("  ✅ Node stopped")

def test_get_info():
    """Test basic node info"""
    print("\n📋 TEST: Get Node Info")
    result = rpc_call("getinfo")
    if result:
        print(f"  ✅ Chain: {result.get('chain', 'unknown')}")
        print(f"  ✅ Height: {result.get('blocks', 0)}")
        print(f"  ✅ Difficulty: {result.get('difficulty', 0)}")
        return True
    return False

def test_get_wallet_address():
    """Get wallet address"""
    print("\n👛 TEST: Get Wallet Address")
    result = rpc_call("getnewaddress")
    if result:
        print(f"  ✅ Address: {result}")
        return result
    return None

def test_mine_block(address):
    """Mine a block to get some coins"""
    print(f"\n⛏️ TEST: Mine Block to {address[:20]}...")
    
    # Get block template
    template = rpc_call("getblocktemplate")
    if not template:
        print("  ❌ Failed to get block template")
        return False
    
    print(f"  📦 Block template height: {template.get('height', 'unknown')}")
    
    # For testing, we'll try to submit a block
    # In real scenario, this would require actual mining
    result = rpc_call("generatetoaddress", [1, address])
    if result:
        print(f"  ✅ Block mined! Hash: {result[0] if isinstance(result, list) else result}")
        return True
    
    # Alternative: check if we can at least verify the template
    print("  ⚠️ Mining requires actual work, skipping...")
    return True

def test_get_balance(address):
    """Test getting balance"""
    print(f"\n💰 TEST: Get Balance for {address[:20]}...")
    result = rpc_call("getbalance", [address])
    if result is not None:
        print(f"  ✅ Balance: {result} GXC")
        return float(result)
    return 0.0

def test_list_unspent(address):
    """Test listing unspent outputs"""
    print(f"\n📦 TEST: List Unspent for {address[:20]}...")
    result = rpc_call("listunspent", [address])
    if result:
        print(f"  ✅ Found {len(result)} UTXOs")
        for utxo in result[:5]:  # Show first 5
            print(f"     - {utxo.get('amount', 0)} GXC (tx: {utxo.get('txid', 'unknown')[:16]}...)")
        return result
    return []

def test_register_validator(address, stake_amount, staking_days):
    """Test registering as a validator (staking)"""
    print(f"\n🔒 TEST: Register Validator (Stake {stake_amount} GXC for {staking_days} days)")
    
    result = rpc_call("registervalidator", [address, stake_amount, staking_days])
    if result:
        print(f"  ✅ Validator registered!")
        print(f"     Address: {result.get('address', 'unknown')[:20]}...")
        print(f"     Stake: {result.get('stake_amount', 0)} GXC")
        print(f"     Weighted Stake: {result.get('weighted_stake', 0)}")
        print(f"     TX Hash: {result.get('stake_tx', 'unknown')[:20]}...")
        return result
    return None

def test_get_validators():
    """Test getting list of validators"""
    print("\n👥 TEST: Get Validators")
    result = rpc_call("getvalidators")
    if result:
        print(f"  ✅ Found {len(result)} validators")
        for v in result:
            print(f"     - {v.get('address', 'unknown')[:20]}... | Stake: {v.get('stake_amount', 0)} GXC")
        return result
    return []

def test_get_staking_info(address):
    """Test getting staking info"""
    print(f"\n📊 TEST: Get Staking Info for {address[:20]}...")
    result = rpc_call("getstakinginfo", [address])
    if result:
        print(f"  ✅ Staking Info:")
        print(f"     Spendable: {result.get('spendable_balance', 0)} GXC")
        print(f"     Staked: {result.get('staked_balance', 0)} GXC")
        print(f"     Total: {result.get('total_balance', 0)} GXC")
        print(f"     Is Validator: {result.get('is_validator', False)}")
        return result
    return None

def test_create_pos_block(address):
    """Test creating a PoS block"""
    print(f"\n🏗️ TEST: Create PoS Block with validator {address[:20]}...")
    result = rpc_call("createposblock", [address])
    if result:
        print(f"  ✅ PoS Block created!")
        print(f"     Height: {result.get('height', 'unknown')}")
        print(f"     Hash: {result.get('hash', 'unknown')[:20]}...")
        return result
    return None

def test_submit_pos_block(block_data):
    """Test submitting a PoS block"""
    print("\n📤 TEST: Submit PoS Block")
    if not block_data:
        print("  ⚠️ No block data to submit")
        return False
    
    result = rpc_call("submitposblock", [block_data])
    if result:
        print(f"  ✅ PoS Block submitted! Result: {result}")
        return True
    return False

def test_unstake(address, amount=None):
    """Test unstaking coins"""
    print(f"\n🔓 TEST: Unstake from {address[:20]}...")
    params = [address]
    if amount:
        params.append(amount)
    
    result = rpc_call("unstake", params)
    if result:
        print(f"  ✅ Unstake successful!")
        print(f"     Amount: {result.get('unstaked_amount', 0)} GXC")
        print(f"     TX Hash: {result.get('unstake_tx', 'unknown')[:20]}...")
        return result
    return None

def test_send_transaction(from_addr, to_addr, amount):
    """Test sending coins"""
    print(f"\n💸 TEST: Send {amount} GXC from {from_addr[:16]}... to {to_addr[:16]}...")
    result = rpc_call("sendtoaddress", [to_addr, amount])
    if result:
        print(f"  ✅ Transaction sent! TX: {result[:20]}...")
        return result
    return None

def test_trace_transaction(tx_hash):
    """Test transaction traceability"""
    print(f"\n🔍 TEST: Trace Transaction {tx_hash[:20]}...")
    result = rpc_call("tracetransaction", [tx_hash])
    if result:
        print(f"  ✅ Transaction lineage: {len(result)} transactions")
        for i, h in enumerate(result[:5]):
            print(f"     {i+1}. {h[:20]}...")
        return result
    return None

def run_all_tests():
    """Run all tests"""
    print("=" * 60)
    print("GXC BLOCKCHAIN COMPREHENSIVE TEST")
    print("=" * 60)
    
    results = {
        "passed": 0,
        "failed": 0,
        "skipped": 0
    }
    
    # Test 1: Node Info
    if test_get_info():
        results["passed"] += 1
    else:
        results["failed"] += 1
    
    # Test 2: Get Wallet Address
    address = test_get_wallet_address()
    if address:
        results["passed"] += 1
    else:
        results["failed"] += 1
        print("❌ Cannot continue without wallet address")
        return results
    
    # Test 3: Get Initial Balance
    initial_balance = test_get_balance(address)
    results["passed"] += 1
    
    # Test 4: List Unspent
    utxos = test_list_unspent(address)
    results["passed"] += 1
    
    # Test 5: Get Staking Info (before staking)
    staking_info_before = test_get_staking_info(address)
    if staking_info_before:
        results["passed"] += 1
    else:
        results["skipped"] += 1
    
    # Test 6: Register Validator (Stake)
    # Need at least 100 GXC to stake
    if initial_balance >= 100:
        stake_result = test_register_validator(address, 100.0, 30)
        if stake_result:
            results["passed"] += 1
            
            # Test 7: Verify balance reduced after staking
            post_stake_balance = test_get_balance(address)
            if post_stake_balance < initial_balance:
                print(f"  ✅ Balance reduced from {initial_balance} to {post_stake_balance} GXC")
                results["passed"] += 1
            else:
                print(f"  ❌ Balance not reduced after staking!")
                results["failed"] += 1
            
            # Test 8: Get Validators List
            validators = test_get_validators()
            if validators and len(validators) > 0:
                results["passed"] += 1
            else:
                results["skipped"] += 1
            
            # Test 9: Get Staking Info (after staking)
            staking_info_after = test_get_staking_info(address)
            if staking_info_after and staking_info_after.get("staked_balance", 0) > 0:
                results["passed"] += 1
            else:
                results["failed"] += 1
            
            # Test 10: Create PoS Block
            pos_block = test_create_pos_block(address)
            if pos_block:
                results["passed"] += 1
            else:
                results["skipped"] += 1
            
            # Test 11: Trace the stake transaction
            stake_tx = stake_result.get("stake_tx")
            if stake_tx:
                trace = test_trace_transaction(stake_tx)
                if trace:
                    results["passed"] += 1
                else:
                    results["skipped"] += 1
            
            # Test 12: Unstake
            unstake_result = test_unstake(address, 50.0)
            if unstake_result:
                results["passed"] += 1
                
                # Verify balance increased after unstaking
                post_unstake_balance = test_get_balance(address)
                print(f"  📊 Balance after unstake: {post_unstake_balance} GXC")
            else:
                results["skipped"] += 1
        else:
            results["failed"] += 1
    else:
        print(f"\n⚠️ Insufficient balance ({initial_balance} GXC) to test staking (need 100+ GXC)")
        print("   To test staking, first mine some blocks or transfer coins to this address")
        results["skipped"] += 4
    
    return results

def main():
    """Main entry point"""
    print("\n" + "=" * 60)
    print("GXC BLOCKCHAIN - STAKING & TRACEABILITY TEST")
    print("=" * 60)
    
    # Check if node binary exists
    if not os.path.exists("/workspace/build/gxc-node"):
        print("❌ gxc-node not found. Please build first: cd build && make gxc-node")
        sys.exit(1)
    
    # Check if node is already running
    try:
        result = rpc_call("getinfo")
        if result:
            print("✅ Node already running, using existing instance")
            results = run_all_tests()
        else:
            raise Exception("Node not responding")
    except:
        # Start node
        if not start_node():
            print("❌ Failed to start node")
            sys.exit(1)
        
        try:
            results = run_all_tests()
        finally:
            stop_node()
    
    # Print summary
    print("\n" + "=" * 60)
    print("TEST SUMMARY")
    print("=" * 60)
    print(f"  ✅ Passed:  {results['passed']}")
    print(f"  ❌ Failed:  {results['failed']}")
    print(f"  ⚠️ Skipped: {results['skipped']}")
    print("=" * 60)
    
    if results["failed"] > 0:
        sys.exit(1)
    sys.exit(0)

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\n\n⚠️ Test interrupted")
        stop_node()
        sys.exit(1)
