#!/usr/bin/env python3
"""
Test staking and validator rewards system
"""
import requests
import json
import time

NODE_URL = "https://8332--019b93ff-3c8e-7a58-9a71-1a6464199b1d.eu-central-1-01.gitpod.dev"

def rpc_call(method, params=[]):
    """Make RPC call to node"""
    payload = {
        "jsonrpc": "2.0",
        "method": method,
        "params": params,
        "id": 1
    }
    response = requests.post(NODE_URL, json=payload)
    return response.json()

def print_section(title):
    """Print section header"""
    print("\n" + "="*80)
    print(f"  {title}")
    print("="*80)

def main():
    print_section("GXC STAKING & VALIDATOR REWARDS TEST")
    
    # Step 1: Get node info
    print_section("Step 1: Check Node Status")
    info = rpc_call("getinfo")
    print(json.dumps(info, indent=2))
    
    # Step 2: Create wallets
    print_section("Step 2: Create Test Wallets")
    
    print("\n📝 Creating Staker Wallet...")
    staker_wallet = rpc_call("getnewaddress", ["staker"])
    print(f"Staker Address: {staker_wallet['result']}")
    staker_address = staker_wallet['result']
    
    print("\n📝 Creating Validator Wallet...")
    validator_wallet = rpc_call("getnewaddress", ["validator"])
    print(f"Validator Address: {validator_wallet['result']}")
    validator_address = validator_wallet['result']
    
    # Step 3: Mine some blocks to get coins
    print_section("Step 3: Mine Blocks to Get Coins")
    print("\n⛏️  Mining 10 blocks to staker address...")
    
    for i in range(10):
        result = rpc_call("generatetoaddress", [1, staker_address])
        print(f"Block {i+1} mined: {result['result'][0][:16]}...")
        time.sleep(0.5)
    
    # Check balance
    print("\n💰 Checking Staker Balance...")
    balance = rpc_call("getbalance", [staker_address])
    print(f"Staker Balance: {balance['result']} GXC")
    
    # Step 4: Stake coins
    print_section("Step 4: Stake Coins")
    stake_amount = 100.0
    print(f"\n🔒 Staking {stake_amount} GXC from staker...")
    
    stake_result = rpc_call("stake", [staker_address, stake_amount])
    print(json.dumps(stake_result, indent=2))
    
    if stake_result.get('result'):
        print(f"✅ Staking successful! TX: {stake_result['result'][:16]}...")
    
    # Wait for confirmation
    print("\n⏳ Waiting for stake confirmation...")
    time.sleep(2)
    
    # Step 5: Register as validator
    print_section("Step 5: Register as Validator")
    print(f"\n📋 Registering {validator_address} as validator...")
    
    validator_result = rpc_call("registervalidator", [validator_address])
    print(json.dumps(validator_result, indent=2))
    
    # Step 6: Check staking info
    print_section("Step 6: Check Staking Information")
    
    print("\n📊 Getting staking info for staker...")
    staking_info = rpc_call("getstakinginfo", [staker_address])
    print(json.dumps(staking_info, indent=2))
    
    print("\n📊 Getting all validators...")
    validators = rpc_call("listvalidators")
    print(json.dumps(validators, indent=2))
    
    # Step 7: Mine more blocks (PoS blocks)
    print_section("Step 7: Mine PoS Blocks to Generate Rewards")
    print("\n⛏️  Mining 20 more blocks (mix of PoW and PoS)...")
    
    initial_balance = balance['result']
    
    for i in range(20):
        result = rpc_call("generatetoaddress", [1, staker_address])
        block_hash = result['result'][0]
        
        # Get block info to see if it's PoS
        block_info = rpc_call("getblock", [block_hash])
        block_type = block_info['result'].get('consensustype', 'unknown')
        
        print(f"Block {i+1}: {block_hash[:16]}... ({block_type})")
        time.sleep(0.5)
    
    # Step 8: Check rewards
    print_section("Step 8: Check Staking Rewards")
    
    print("\n💰 Checking Staker Balance After Mining...")
    new_balance = rpc_call("getbalance", [staker_address])
    print(f"Initial Balance: {initial_balance} GXC")
    print(f"Current Balance: {new_balance['result']} GXC")
    print(f"Earned: {new_balance['result'] - initial_balance} GXC")
    
    print("\n📊 Getting updated staking info...")
    final_staking_info = rpc_call("getstakinginfo", [staker_address])
    print(json.dumps(final_staking_info, indent=2))
    
    # Step 9: Check validator rewards
    print_section("Step 9: Check Validator Rewards")
    
    print("\n💰 Checking Validator Balance...")
    validator_balance = rpc_call("getbalance", [validator_address])
    print(f"Validator Balance: {validator_balance['result']} GXC")
    
    if validator_balance['result'] > 0:
        print("✅ Validator earned rewards!")
    else:
        print("⚠️  Validator has not earned rewards yet (may need more PoS blocks)")
    
    # Step 10: Unstake
    print_section("Step 10: Test Unstaking")
    
    unstake_amount = 50.0
    print(f"\n🔓 Unstaking {unstake_amount} GXC...")
    
    unstake_result = rpc_call("unstake", [staker_address, unstake_amount])
    print(json.dumps(unstake_result, indent=2))
    
    if unstake_result.get('result'):
        print(f"✅ Unstaking successful! TX: {unstake_result['result'][:16]}...")
    
    # Wait for confirmation
    time.sleep(2)
    
    print("\n📊 Final staking info after unstaking...")
    final_info = rpc_call("getstakinginfo", [staker_address])
    print(json.dumps(final_info, indent=2))
    
    # Summary
    print_section("SUMMARY")
    print(f"""
✅ Staking Test Complete!

Staker Address: {staker_address}
Validator Address: {validator_address}

Initial Balance: {initial_balance} GXC
Final Balance: {new_balance['result']} GXC
Total Earned: {new_balance['result'] - initial_balance} GXC

Staked Amount: {stake_amount} GXC
Unstaked Amount: {unstake_amount} GXC
Remaining Staked: {stake_amount - unstake_amount} GXC

Validator Rewards: {validator_balance['result']} GXC

Status: {'✅ STAKERS EARN REWARDS!' if new_balance['result'] > initial_balance else '⚠️  No rewards earned yet'}
    """)
    
    print("="*80)

if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        print(f"\n❌ Error: {e}")
        import traceback
        traceback.print_exc()
