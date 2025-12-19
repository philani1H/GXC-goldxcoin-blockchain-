#!/usr/bin/env python3
"""
Comprehensive test for GXC blockchain fixes
Tests fund transfer, staking, and traceability
"""

import sys
import os

# Add python directory to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'python'))

from blockchain import Blockchain
from wallet import Wallet
from transaction import Transaction, TransactionType

def print_section(title):
    """Print a section header"""
    print("\n" + "=" * 70)
    print(f"  {title}")
    print("=" * 70)

def print_success(message):
    """Print success message"""
    print(f"✅ {message}")

def print_error(message):
    """Print error message"""
    print(f"❌ {message}")

def print_info(message):
    """Print info message"""
    print(f"ℹ️  {message}")

def test_traceability():
    """Test traceability formula"""
    print_section("TRACEABILITY FORMULA TEST")
    
    print_info("GXC Traceability Formula:")
    print("   Ti.Inputs[0].txHash == Ti.PrevTxHash")
    print("   Ti.Inputs[0].amount == Ti.ReferencedAmount")
    print()
    
    # Create blockchain
    blockchain = Blockchain()
    print_success("Blockchain created")
    
    # Create wallets
    wallet1 = Wallet()
    wallet2 = Wallet()
    print_success(f"Wallet 1: {wallet1.address[:30]}...")
    print_success(f"Wallet 2: {wallet2.address[:30]}...")
    
    # Mine genesis block to fund wallet1
    print_info("Mining genesis block to fund wallet1...")
    genesis_block = blockchain.create_genesis_block(wallet1.address)
    blockchain.add_block(genesis_block)
    print_success(f"Genesis block mined. Height: {blockchain.get_height()}")
    
    # Check wallet1 balance
    balance1 = blockchain.get_balance(wallet1.address)
    print_success(f"Wallet1 balance: {balance1} GXC")
    
    if balance1 < 100:
        print_error("Insufficient funds for testing")
        return False
    
    # Test 1: Create first transaction (Genesis -> Wallet1 -> Wallet2)
    print_section("TEST 1: First Transaction (Traceability)")
    
    utxo_set = blockchain.get_utxo_set()
    print_info(f"UTXO set size: {len(utxo_set)}")
    
    # Create transaction
    try:
        tx1 = wallet1.create_transaction(wallet2.address, 50.0, utxo_set, 0.001)
        print_success("Transaction 1 created")
        
        # Verify traceability
        print_info("Verifying traceability formula...")
        print(f"   TX Hash: {tx1.hash[:16]}...")
        print(f"   PrevTxHash: {tx1.prev_tx_hash[:16]}...")
        print(f"   Inputs[0].txHash: {tx1.inputs[0].tx_hash[:16]}...")
        print(f"   Inputs[0].amount: {tx1.inputs[0].amount} GXC")
        print(f"   ReferencedAmount: {tx1.referenced_amount} GXC")
        
        # Check formula
        hash_match = tx1.inputs[0].tx_hash == tx1.prev_tx_hash
        amount_match = abs(tx1.inputs[0].amount - tx1.referenced_amount) < 0.00000001
        
        if hash_match and amount_match:
            print_success("✓ Traceability formula VALID")
            print(f"   ✓ Inputs[0].txHash == PrevTxHash: {hash_match}")
            print(f"   ✓ Inputs[0].amount == ReferencedAmount: {amount_match}")
        else:
            print_error("✗ Traceability formula INVALID")
            print(f"   ✗ Hash match: {hash_match}")
            print(f"   ✗ Amount match: {amount_match}")
            return False
        
        # Add to blockchain
        if blockchain.add_transaction(tx1):
            print_success("Transaction 1 added to mempool")
        else:
            print_error("Failed to add transaction 1")
            return False
        
        # Mine block
        block1 = blockchain.mine_pending_transactions(wallet1.address)
        if block1:
            blockchain.add_block(block1)
            print_success(f"Block 1 mined. Height: {blockchain.get_height()}")
        
        # Verify balances
        balance1_after = blockchain.get_balance(wallet1.address)
        balance2_after = blockchain.get_balance(wallet2.address)
        print_info(f"Wallet1 balance after: {balance1_after} GXC")
        print_info(f"Wallet2 balance after: {balance2_after} GXC")
        
        if balance2_after >= 50.0:
            print_success(f"✓ Fund transfer successful: {balance2_after} GXC received")
        else:
            print_error(f"✗ Fund transfer failed: Expected 50 GXC, got {balance2_after} GXC")
            return False
            
    except Exception as e:
        print_error(f"Transaction 1 failed: {e}")
        return False
    
    # Test 2: Create second transaction (chained traceability)
    print_section("TEST 2: Chained Transaction (Traceability)")
    
    utxo_set = blockchain.get_utxo_set()
    
    try:
        tx2 = wallet2.create_transaction(wallet1.address, 20.0, utxo_set, 0.001)
        print_success("Transaction 2 created")
        
        # Verify traceability chain
        print_info("Verifying traceability chain...")
        print(f"   TX1 Hash: {tx1.hash[:16]}...")
        print(f"   TX2 PrevTxHash: {tx2.prev_tx_hash[:16]}...")
        print(f"   TX2 Inputs[0].txHash: {tx2.inputs[0].tx_hash[:16]}...")
        
        # Check if TX2 references TX1
        if tx2.prev_tx_hash == tx1.hash:
            print_success("✓ Transaction chain verified: TX2 references TX1")
        else:
            print_error("✗ Transaction chain broken")
            return False
        
        # Verify formula
        hash_match = tx2.inputs[0].tx_hash == tx2.prev_tx_hash
        amount_match = abs(tx2.inputs[0].amount - tx2.referenced_amount) < 0.00000001
        
        if hash_match and amount_match:
            print_success("✓ Traceability formula VALID for TX2")
        else:
            print_error("✗ Traceability formula INVALID for TX2")
            return False
        
        # Add to blockchain
        if blockchain.add_transaction(tx2):
            print_success("Transaction 2 added to mempool")
        else:
            print_error("Failed to add transaction 2")
            return False
        
        # Mine block
        block2 = blockchain.mine_pending_transactions(wallet1.address)
        if block2:
            blockchain.add_block(block2)
            print_success(f"Block 2 mined. Height: {blockchain.get_height()}")
        
    except Exception as e:
        print_error(f"Transaction 2 failed: {e}")
        return False
    
    # Test 3: Staking transaction (traceability)
    print_section("TEST 3: Staking Transaction (Traceability)")
    
    utxo_set = blockchain.get_utxo_set()
    balance1 = blockchain.get_balance(wallet1.address)
    print_info(f"Wallet1 balance: {balance1} GXC")
    
    if balance1 < 100:
        print_error("Insufficient funds for staking test")
        return False
    
    try:
        # Create stake transaction
        stake_tx = wallet1.create_stake_transaction(100.0, utxo_set, 0.001)
        print_success("Stake transaction created")
        
        # Verify traceability
        print_info("Verifying stake transaction traceability...")
        print(f"   Type: {stake_tx.type}")
        print(f"   PrevTxHash: {stake_tx.prev_tx_hash[:16]}...")
        print(f"   Inputs[0].txHash: {stake_tx.inputs[0].tx_hash[:16]}...")
        print(f"   Inputs[0].amount: {stake_tx.inputs[0].amount} GXC")
        print(f"   ReferencedAmount: {stake_tx.referenced_amount} GXC")
        
        # Check formula
        hash_match = stake_tx.inputs[0].tx_hash == stake_tx.prev_tx_hash
        amount_match = abs(stake_tx.inputs[0].amount - stake_tx.referenced_amount) < 0.00000001
        
        if hash_match and amount_match:
            print_success("✓ Stake transaction traceability VALID")
        else:
            print_error("✗ Stake transaction traceability INVALID")
            return False
        
        # Verify stake amount calculation
        total_input = sum(inp.amount for inp in stake_tx.inputs)
        total_output = sum(out.amount for out in stake_tx.outputs)
        fee = stake_tx.fee
        staked_amount = total_input - total_output - fee
        
        print_info(f"Stake calculation:")
        print(f"   Total input: {total_input} GXC")
        print(f"   Total output: {total_output} GXC")
        print(f"   Fee: {fee} GXC")
        print(f"   Staked amount: {staked_amount} GXC")
        
        if abs(staked_amount - 100.0) < 0.001:
            print_success(f"✓ Stake amount correct: {staked_amount} GXC")
        else:
            print_error(f"✗ Stake amount incorrect: Expected 100 GXC, got {staked_amount} GXC")
            return False
        
    except Exception as e:
        print_error(f"Stake transaction failed: {e}")
        return False
    
    print_section("ALL TESTS PASSED")
    print_success("Fund transfers work correctly")
    print_success("Traceability formula is enforced")
    print_success("Transaction chains are preserved")
    print_success("Staking transactions maintain traceability")
    
    return True

def main():
    """Main test function"""
    print("\n" + "=" * 70)
    print("  GXC BLOCKCHAIN COMPREHENSIVE TEST")
    print("  Testing: Fund Transfer, Staking, and Traceability")
    print("=" * 70)
    
    try:
        success = test_traceability()
        
        if success:
            print("\n" + "=" * 70)
            print("  ✅ ALL TESTS PASSED")
            print("=" * 70)
            sys.exit(0)
        else:
            print("\n" + "=" * 70)
            print("  ❌ TESTS FAILED")
            print("=" * 70)
            sys.exit(1)
    except Exception as e:
        print_error(f"Test suite failed with exception: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)

if __name__ == "__main__":
    main()
