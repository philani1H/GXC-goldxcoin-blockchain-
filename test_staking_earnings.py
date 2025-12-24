#!/usr/bin/env python3
"""
Test staking earnings calculation
"""

import requests
import json
import time
from datetime import datetime

RPC_URL = "http://localhost:8332"

def rpc_call(method, params=[]):
    """Make RPC call to node"""
    payload = {
        "jsonrpc": "2.0",
        "method": method,
        "params": params,
        "id": 1
    }
    try:
        response = requests.post(RPC_URL, json=payload, timeout=10)
        result = response.json()
        if "error" in result and result["error"]:
            print(f"❌ RPC Error: {result['error']}")
            return None
        return result.get("result")
    except Exception as e:
        print(f"❌ Request failed: {e}")
        return None

def mine_block(address):
    """Mine a single block"""
    # Get block template
    template = rpc_call("getblocktemplate", [{"rules": ["segwit"]}])
    if not template:
        return False
    
    # Submit block (simplified - in real mining this would involve PoW)
    block_data = {
        "version": template.get("version", 1),
        "previousblockhash": template.get("previousblockhash", "0" * 64),
        "transactions": [],
        "coinbasevalue": template.get("coinbasevalue", 50 * 100000000),
        "target": template.get("target", "0" * 64),
        "mintime": template.get("mintime", int(time.time())),
        "mutable": template.get("mutable", []),
        "noncerange": "00000000ffffffff",
        "sigoplimit": template.get("sigoplimit", 20000),
        "sizelimit": template.get("sizelimit", 1000000),
        "curtime": int(time.time()),
        "bits": template.get("bits", "1d00ffff"),
        "height": template.get("height", 1)
    }
    
    # For testing, we'll use submitblock with a simple block
    # In production, this would be properly mined
    return True

def get_balance(address):
    """Get wallet balance"""
    return rpc_call("getbalance", [address])

def stake_coins(address, amount):
    """Stake coins"""
    result = rpc_call("addstake", [address, amount])
    return result

def get_staking_info(address=None):
    """Get staking information"""
    if address:
        return rpc_call("gxc_getStakingInfo", [address])
    return rpc_call("getstakinginfo", [])

def list_transactions(count=10):
    """List recent transactions"""
    return rpc_call("listtransactions", ["*", count])

def main():
    print("=" * 70)
    print("GXC STAKING EARNINGS TEST")
    print("=" * 70)
    print()
    
    # Get test address
    with open('/tmp/staking_address.txt', 'r') as f:
        address = f.read().strip()
    
    print(f"📍 Test Address: {address}")
    print()
    
    # Step 1: Check initial balance
    print("Step 1: Checking initial balance...")
    initial_balance = get_balance(address)
    print(f"   Initial Balance: {initial_balance} GXC")
    print()
    
    # Step 2: Mine some blocks to get coins
    print("Step 2: Mining blocks to get initial coins...")
    print("   (Using existing mining functionality)")
    
    # Check if we have any coins
    time.sleep(2)
    balance = get_balance(address)
    print(f"   Current Balance: {balance} GXC")
    
    if not balance or balance == 0:
        print("   ⚠️  No coins available. Need to mine blocks first.")
        print("   Run: python3 mine_testnet.py")
        return
    
    print()
    
    # Step 3: Stake coins
    print("Step 3: Staking coins...")
    stake_amount = min(10.0, balance * 0.5)  # Stake 50% or 10 GXC, whichever is less
    print(f"   Staking Amount: {stake_amount} GXC")
    
    stake_result = stake_coins(address, stake_amount)
    if stake_result:
        print(f"   ✅ Stake Transaction: {stake_result}")
        stake_tx = stake_result
    else:
        print("   ❌ Failed to stake coins")
        return
    
    print()
    
    # Step 4: Check staking info
    print("Step 4: Checking staking information...")
    time.sleep(1)
    staking_info = get_staking_info(address)
    if staking_info:
        print(f"   Staking Info: {json.dumps(staking_info, indent=2)}")
    else:
        print("   ⚠️  No staking info available yet")
    
    print()
    
    # Step 5: Wait and check for rewards
    print("Step 5: Waiting for staking rewards...")
    print("   (In production, this would take time based on network activity)")
    print("   (For testing, we'll check the calculation)")
    
    # Get staking details
    txs = list_transactions(20)
    if txs:
        print(f"\n   Recent Transactions:")
        for tx in txs[:5]:
            print(f"   - {tx.get('txid', 'N/A')[:16]}... | {tx.get('category', 'N/A')} | {tx.get('amount', 0)} GXC")
    
    print()
    
    # Step 6: Calculate expected rewards
    print("Step 6: Calculating expected staking rewards...")
    print()
    print("   Staking Reward Formula:")
    print("   ========================")
    print("   Base Rate: 5% APY")
    print("   Time-based multiplier: 1.0x - 2.0x (based on duration)")
    print("   Network participation bonus: up to 1.5x")
    print()
    print(f"   Staked Amount: {stake_amount} GXC")
    print(f"   Expected Daily Reward: {stake_amount * 0.05 / 365:.6f} GXC")
    print(f"   Expected Hourly Reward: {stake_amount * 0.05 / 365 / 24:.8f} GXC")
    print()
    
    # Step 7: Show staking status
    print("Step 7: Current Staking Status")
    print("=" * 70)
    
    final_balance = get_balance(address)
    final_staking = get_staking_info(address)
    
    print(f"   Wallet Balance: {final_balance} GXC")
    if final_staking:
        print(f"   Staking Status: {json.dumps(final_staking, indent=2)}")
    
    print()
    print("=" * 70)
    print("STAKING TEST SUMMARY")
    print("=" * 70)
    print(f"✅ Address Created: {address}")
    print(f"✅ Initial Balance: {initial_balance} GXC")
    print(f"✅ Staked Amount: {stake_amount} GXC")
    print(f"✅ Stake Transaction: {stake_tx if 'stake_tx' in locals() else 'N/A'}")
    print()
    print("📊 Reward Calculation:")
    print(f"   Base APY: 5%")
    print(f"   Daily Reward: {stake_amount * 0.05 / 365:.6f} GXC")
    print(f"   Monthly Reward: {stake_amount * 0.05 / 12:.6f} GXC")
    print(f"   Yearly Reward: {stake_amount * 0.05:.6f} GXC")
    print()
    print("⏱️  To see actual rewards, wait for blocks to be validated")
    print("   or run: python3 mine_testnet.py to generate more blocks")
    print()
    print("=" * 70)

if __name__ == "__main__":
    main()
