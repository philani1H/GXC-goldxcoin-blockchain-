#!/usr/bin/env python3
"""
Test the blockchain directly using Python API
"""

import sys
import os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'python'))

from blockchain_api import BlockchainAPI
import time

def print_section(title):
    """Print a section header"""
    print(f"\n{'='*60}")
    print(f"  {title}")
    print(f"{'='*60}\n")

def main():
    print("\n🚀 GXC Complete Flow Test (Direct Python API)")
    print("Testing: Mining → Sending → Staking → Unstaking\n")
    
    # Initialize blockchain API
    api = BlockchainAPI(testnet=True, data_dir="./gxc_test_data")
    
    # Step 1: Create wallet
    print_section("1. Create/Load Wallet")
    wallet_address = api.create_wallet()
    print(f"✅ Wallet Address: {wallet_address}")
    
    # Step 2: Check initial balance
    print_section("2. Check Initial Balance")
    balance = api.get_balance(wallet_address)
    print(f"   Initial Balance: {balance} GXC")
    
    # Step 3: Mine blocks
    print_section("3. Mine Blocks")
    print("   Mining 10 blocks to get initial coins...")
    
    for i in range(10):
        block = api.mine_block(wallet_address)
        if block:
            print(f"   ✅ Block {i+1} mined - Height: {block.get('index', 'N/A')}")
        time.sleep(0.1)
    
    # Check balance after mining
    balance = api.get_balance(wallet_address)
    print(f"\n   Balance after mining: {balance} GXC")
    
    if balance < 1.0:
        print("   ❌ Insufficient balance for testing")
        return
    
    # Step 4: Send coins
    print_section("4. Send Coins")
    recipient = api.create_wallet()  # Create a second wallet
    send_amount = 10.0
    
    print(f"   Recipient: {recipient}")
    print(f"   Sending {send_amount} GXC...")
    
    tx_hash = api.send_transaction(wallet_address, recipient, send_amount)
    if tx_hash:
        print(f"   ✅ Transaction sent: {tx_hash}")
        
        # Mine a block to confirm
        print("   Mining block to confirm transaction...")
        api.mine_block(wallet_address)
        time.sleep(0.5)
        
        # Check balances
        sender_balance = api.get_balance(wallet_address)
        recipient_balance = api.get_balance(recipient)
        
        print(f"   Sender balance: {sender_balance} GXC")
        print(f"   Recipient balance: {recipient_balance} GXC")
    else:
        print("   ❌ Failed to send transaction")
    
    # Step 5: Register as validator and stake
    print_section("5. Register Validator & Stake")
    stake_amount = 50.0
    
    print(f"   Registering validator with {stake_amount} GXC stake...")
    
    result = api.register_validator(wallet_address, stake_amount)
    if result:
        print(f"   ✅ Validator registered")
        
        # Mine a block to confirm
        print("   Mining block to confirm validator registration...")
        api.mine_block(wallet_address)
        time.sleep(0.5)
        
        # Check validator info
        validator_info = api.get_validator_info(wallet_address)
        if validator_info:
            print(f"   Validator Status: Active")
            print(f"   Staked Amount: {validator_info.get('stake', 0)} GXC")
    else:
        print("   ❌ Failed to register validator")
    
    # Step 6: Additional staking
    print_section("6. Additional Staking")
    additional_stake = 25.0
    
    print(f"   Staking additional {additional_stake} GXC...")
    
    tx_hash = api.stake(wallet_address, additional_stake)
    if tx_hash:
        print(f"   ✅ Stake transaction: {tx_hash}")
        
        # Mine a block to confirm
        print("   Mining block to confirm stake...")
        api.mine_block(wallet_address)
        time.sleep(0.5)
        
        # Check updated validator info
        validator_info = api.get_validator_info(wallet_address)
        if validator_info:
            print(f"   Total Staked: {validator_info.get('stake', 0)} GXC")
    else:
        print("   ❌ Failed to stake")
    
    # Step 7: Unstake
    print_section("7. Unstake Coins")
    unstake_amount = 20.0
    
    print(f"   Unstaking {unstake_amount} GXC...")
    
    tx_hash = api.unstake(wallet_address, unstake_amount)
    if tx_hash:
        print(f"   ✅ Unstake transaction: {tx_hash}")
        
        # Mine a block to confirm
        print("   Mining block to confirm unstake...")
        api.mine_block(wallet_address)
        time.sleep(0.5)
        
        # Check final validator info
        validator_info = api.get_validator_info(wallet_address)
        if validator_info:
            print(f"   Remaining Staked: {validator_info.get('stake', 0)} GXC")
    else:
        print("   ❌ Failed to unstake")
    
    # Step 8: Final summary
    print_section("8. Final Summary")
    
    blockchain_info = api.get_blockchain_info()
    if blockchain_info:
        print(f"   Blockchain Height: {blockchain_info.get('height', 0)}")
        print(f"   Difficulty: {blockchain_info.get('difficulty', 0)}")
    
    final_balance = api.get_balance(wallet_address)
    print(f"   Final Balance: {final_balance} GXC")
    
    validator_info = api.get_validator_info(wallet_address)
    if validator_info:
        print(f"   Validator Stake: {validator_info.get('stake', 0)} GXC")
    
    print("\n✅ Complete flow test finished!")
    print(f"{'='*60}\n")

if __name__ == "__main__":
    main()
