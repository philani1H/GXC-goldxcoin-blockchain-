#!/usr/bin/env python3
"""
Simple test of blockchain functionality using Python classes
"""

import sys
import os
import time

sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'python'))

from gxc_api import GXCBlockchain, GXCWallet, GXCMiner

def print_section(title):
    print(f"\n{'='*60}")
    print(f"  {title}")
    print(f"{'='*60}\n")

def main():
    print("\n🚀 GXC Simple Flow Test")
    print("Testing: Mining → Sending → Staking\n")
    
    # Initialize blockchain
    print_section("1. Initialize Blockchain")
    blockchain = GXCBlockchain(testnet=True, data_dir="./gxc_test_simple")
    print("✅ Blockchain initialized")
    print(f"   Height: {blockchain.get_height()}")
    print(f"   Difficulty: {blockchain.get_difficulty()}")
    
    # Create wallet
    print_section("2. Create Wallet")
    wallet = GXCWallet(testnet=True)
    address = wallet.get_address()
    print(f"✅ Wallet created: {address}")
    
    # Check initial balance
    balance = blockchain.get_balance(address)
    print(f"   Initial balance: {balance} GXC")
    
    # Mine blocks
    print_section("3. Mine Blocks")
    miner = GXCMiner(blockchain, wallet)
    
    print("   Mining 10 blocks...")
    for i in range(10):
        try:
            block = miner.mine_block()
            if block:
                print(f"   ✅ Block {i+1} mined - Height: {blockchain.get_height()}")
            time.sleep(0.1)
        except Exception as e:
            print(f"   ❌ Mining error: {e}")
            break
    
    # Check balance after mining
    balance = blockchain.get_balance(address)
    print(f"\n   Balance after mining: {balance} GXC")
    
    if balance < 1.0:
        print("   ❌ Insufficient balance")
        return
    
    # Create second wallet for sending
    print_section("4. Send Transaction")
    recipient_wallet = GXCWallet(testnet=True)
    recipient = recipient_wallet.get_address()
    
    print(f"   Recipient: {recipient}")
    print(f"   Sending 10 GXC...")
    
    try:
        tx = wallet.create_transaction(recipient, 10.0, blockchain.get_utxo_set())
        if blockchain.add_transaction(tx):
            print(f"   ✅ Transaction added to mempool: {tx.get_hash()[:16]}...")
            
            # Mine to confirm
            print("   Mining block to confirm...")
            miner.mine_block()
            
            # Check balances
            sender_balance = blockchain.get_balance(address)
            recipient_balance = blockchain.get_balance(recipient)
            
            print(f"   Sender balance: {sender_balance} GXC")
            print(f"   Recipient balance: {recipient_balance} GXC")
        else:
            print("   ❌ Transaction rejected")
    except Exception as e:
        print(f"   ❌ Error: {e}")
    
    # Register validator
    print_section("5. Register Validator")
    print("   Registering validator with 50 GXC stake...")
    
    try:
        result = blockchain.register_validator(address, 50.0)
        if result:
            print("   ✅ Validator registered")
            
            # Mine to confirm
            print("   Mining block to confirm...")
            miner.mine_block()
            
            # Check validator info
            validators = blockchain.get_active_validators()
            print(f"   Active validators: {len(validators)}")
            
            for v in validators:
                if v.get_address() == address:
                    print(f"   Stake: {v.get_stake_amount()} GXC")
                    break
        else:
            print("   ❌ Registration failed")
    except Exception as e:
        print(f"   ❌ Error: {e}")
    
    # Final summary
    print_section("6. Final Summary")
    print(f"   Blockchain Height: {blockchain.get_height()}")
    print(f"   Difficulty: {blockchain.get_difficulty()}")
    print(f"   Final Balance: {blockchain.get_balance(address)} GXC")
    print(f"   Active Validators: {len(blockchain.get_active_validators())}")
    
    print("\n✅ Test completed!")
    print(f"{'='*60}\n")

if __name__ == "__main__":
    main()
