#!/usr/bin/env python3
"""
Complete API test flow: mine, send, stake, unstake
"""

import requests
import json
import time
import sys

NODE_URL = "http://localhost:8545"

def rpc_call(method, params=[]):
    """Make an RPC call to the node"""
    payload = {
        "jsonrpc": "2.0",
        "method": method,
        "params": params,
        "id": 1
    }
    
    try:
        response = requests.post(NODE_URL, json=payload, timeout=5)
        result = response.json()
        
        if "error" in result:
            print(f"❌ RPC Error: {result['error']}")
            return None
        
        return result.get("result")
    except requests.exceptions.ConnectionError:
        print(f"❌ Cannot connect to node at {NODE_URL}")
        print("   Make sure the node is running with: ./gxc-node --testnet --rpc-port=8545")
        sys.exit(1)
    except Exception as e:
        print(f"❌ Error: {e}")
        return None

def print_section(title):
    """Print a section header"""
    print(f"\n{'='*60}")
    print(f"  {title}")
    print(f"{'='*60}\n")

def main():
    print("\n🚀 GXC Complete API Test Flow")
    print("Testing: Mining → Sending → Staking → Unstaking\n")
    
    # Step 1: Get wallet address
    print_section("1. Get Wallet Address")
    address = rpc_call("getnewaddress")
    if not address:
        print("Failed to get address")
        return
    print(f"✅ Wallet Address: {address}")
    
    # Step 2: Check initial balance
    print_section("2. Check Initial Balance")
    balance = rpc_call("getbalance", [address])
    print(f"   Initial Balance: {balance} GXC")
    
    # Step 3: Mine blocks to get coins
    print_section("3. Mine Blocks")
    print("   Mining 5 blocks to get initial coins...")
    
    for i in range(5):
        block_hash = rpc_call("generatetoaddress", [1, address])
        if block_hash:
            print(f"   ✅ Block {i+1} mined: {block_hash[0][:16]}...")
        time.sleep(0.5)
    
    # Check balance after mining
    balance = rpc_call("getbalance", [address])
    print(f"\n   Balance after mining: {balance} GXC")
    
    if balance < 1.0:
        print("   ❌ Insufficient balance for testing")
        return
    
    # Step 4: Send coins
    print_section("4. Send Coins via API")
    recipient = "tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf"
    send_amount = 10.0
    
    print(f"   Sending {send_amount} GXC to {recipient[:30]}...")
    
    tx_hash = rpc_call("sendtoaddress", [recipient, send_amount])
    if tx_hash:
        print(f"   ✅ Transaction sent: {tx_hash}")
        
        # Mine a block to confirm
        print("   Mining block to confirm transaction...")
        rpc_call("generatetoaddress", [1, address])
        time.sleep(1)
        
        # Check balances
        sender_balance = rpc_call("getbalance", [address])
        recipient_balance = rpc_call("getbalance", [recipient])
        
        print(f"   Sender balance: {sender_balance} GXC")
        print(f"   Recipient balance: {recipient_balance} GXC")
    else:
        print("   ❌ Failed to send transaction")
    
    # Step 5: Register as validator and stake
    print_section("5. Register Validator & Stake")
    stake_amount = 50.0
    
    print(f"   Registering validator with {stake_amount} GXC stake...")
    
    result = rpc_call("registervalidator", [address, stake_amount])
    if result:
        print(f"   ✅ Validator registered: {result}")
        
        # Mine a block to confirm
        print("   Mining block to confirm validator registration...")
        rpc_call("generatetoaddress", [1, address])
        time.sleep(1)
        
        # Check validator info
        validator_info = rpc_call("getvalidatorinfo", [address])
        if validator_info:
            print(f"   Validator Status: {validator_info.get('status', 'N/A')}")
            print(f"   Staked Amount: {validator_info.get('stake', 0)} GXC")
    else:
        print("   ❌ Failed to register validator")
    
    # Step 6: Additional staking
    print_section("6. Additional Staking")
    additional_stake = 25.0
    
    print(f"   Staking additional {additional_stake} GXC...")
    
    tx_hash = rpc_call("stake", [address, additional_stake])
    if tx_hash:
        print(f"   ✅ Stake transaction: {tx_hash}")
        
        # Mine a block to confirm
        print("   Mining block to confirm stake...")
        rpc_call("generatetoaddress", [1, address])
        time.sleep(1)
        
        # Check updated validator info
        validator_info = rpc_call("getvalidatorinfo", [address])
        if validator_info:
            print(f"   Total Staked: {validator_info.get('stake', 0)} GXC")
    else:
        print("   ❌ Failed to stake")
    
    # Step 7: Unstake
    print_section("7. Unstake Coins")
    unstake_amount = 20.0
    
    print(f"   Unstaking {unstake_amount} GXC...")
    
    tx_hash = rpc_call("unstake", [address, unstake_amount])
    if tx_hash:
        print(f"   ✅ Unstake transaction: {tx_hash}")
        
        # Mine a block to confirm
        print("   Mining block to confirm unstake...")
        rpc_call("generatetoaddress", [1, address])
        time.sleep(1)
        
        # Check final validator info
        validator_info = rpc_call("getvalidatorinfo", [address])
        if validator_info:
            print(f"   Remaining Staked: {validator_info.get('stake', 0)} GXC")
    else:
        print("   ❌ Failed to unstake")
    
    # Step 8: Final summary
    print_section("8. Final Summary")
    
    blockchain_info = rpc_call("getblockchaininfo")
    if blockchain_info:
        print(f"   Blockchain Height: {blockchain_info.get('blocks', 0)}")
        print(f"   Difficulty: {blockchain_info.get('difficulty', 0)}")
    
    final_balance = rpc_call("getbalance", [address])
    print(f"   Final Balance: {final_balance} GXC")
    
    validator_info = rpc_call("getvalidatorinfo", [address])
    if validator_info:
        print(f"   Validator Stake: {validator_info.get('stake', 0)} GXC")
    
    print("\n✅ Complete API test flow finished!")
    print(f"{'='*60}\n")

if __name__ == "__main__":
    main()
