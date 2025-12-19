#!/usr/bin/env python3
"""
GXC Wallet Integration Examples (Python)
Complete working examples for wallet developers
"""

import requests
import json
from typing import Dict, List, Any, Optional

# Configuration
NODE_URL = 'http://localhost:8545'
NETWORK = 'testnet'  # or 'mainnet'


def rpc(method: str, params: List[Any] = None) -> Any:
    """Make RPC call to GXC node"""
    if params is None:
        params = []
    
    payload = {
        'jsonrpc': '2.0',
        'method': method,
        'params': params,
        'id': 1
    }
    
    try:
        response = requests.post(NODE_URL, json=payload, timeout=10)
        data = response.json()
        
        if 'error' in data:
            raise Exception(f"RPC Error: {data['error']}")
        
        return data.get('result')
    except requests.exceptions.ConnectionError:
        raise Exception(f"Cannot connect to node at {NODE_URL}")
    except Exception as e:
        raise Exception(f"RPC call failed: {e}")


def example1_send_transaction():
    """Example 1: Send Transaction"""
    print('\n=== Example 1: Send Transaction ===\n')
    
    try:
        # Send 10 GXC to recipient
        txid = rpc('sendtoaddress', [
            'tGXC1234567890abcdef...',  # recipient address
            10.0                         # amount in GXC
        ])
        
        print(f'✅ Transaction sent!')
        print(f'   TX ID: {txid}')
        
        # Get transaction details
        tx = rpc('gettransaction', [txid])
        print(f'   Confirmations: {tx.get("confirmations", 0)}')
        print(f'   Fee: {tx.get("fee", 0)} GXC')
        
    except Exception as e:
        print(f'❌ Error: {e}')


def example2_check_balance():
    """Example 2: Check Balance"""
    print('\n=== Example 2: Check Balance ===\n')
    
    try:
        address = 'tGXC1234567890abcdef...'
        
        # Get spendable balance
        balance = rpc('getbalance', [address])
        print(f'✅ Spendable Balance: {balance} GXC')
        
        # Get staking info
        staking_info = rpc('getStakingInfo', [address])
        print(f'   Staked Amount: {staking_info["staked_amount"]} GXC')
        print(f'   Total Balance: {staking_info["total_balance"]} GXC')
        print(f'   Pending Rewards: {staking_info["pending_rewards"]} GXC')
        
    except Exception as e:
        print(f'❌ Error: {e}')


def example3_register_validator():
    """Example 3: Register as Validator"""
    print('\n=== Example 3: Register as Validator ===\n')
    
    try:
        address = 'tGXC1234567890abcdef...'
        stake_amount = 100.0  # Minimum 100 GXC
        staking_days = 30     # 14-365 days
        
        # Register validator
        result = rpc('registerValidator', [
            address,
            stake_amount,
            staking_days
        ])
        
        print(f'✅ Validator registered!')
        print(f'   Address: {result["address"]}')
        print(f'   Stake: {result["stake_amount"]} GXC')
        print(f'   Status: {result["status"]}')
        print(f'   Stake TX: {result["stake_tx"]}')
        print(f'   Message: {result["message"]}')
        
        # Check validator status
        info = rpc('getValidatorInfo', [address])
        print(f'\n   Validator Info:')
        print(f'   - Is Pending: {info["is_pending"]}')
        print(f'   - Is Active: {info["is_active"]}')
        print(f'   - Weighted Stake: {info["weighted_stake"]}')
        
    except Exception as e:
        print(f'❌ Error: {e}')


def example4_add_stake():
    """Example 4: Add More Stake"""
    print('\n=== Example 4: Add More Stake ===\n')
    
    try:
        address = 'tGXC1234567890abcdef...'
        additional_amount = 50.0
        
        # Add stake
        result = rpc('addStake', [
            address,
            additional_amount
        ])
        
        print(f'✅ Stake added!')
        print(f'   Additional Amount: {result["additional_amount"]} GXC')
        print(f'   TX ID: {result["txid"]}')
        
        # Check updated info
        info = rpc('getValidatorInfo', [address])
        print(f'   Total Stake: {info["stake_amount"]} GXC')
        
    except Exception as e:
        print(f'❌ Error: {e}')


def example5_unstake():
    """Example 5: Unstake"""
    print('\n=== Example 5: Unstake ===\n')
    
    try:
        address = 'tGXC1234567890abcdef...'
        unstake_amount = 50.0
        
        # Unstake
        result = rpc('unstake', [
            address,
            unstake_amount
        ])
        
        print(f'✅ Unstake successful!')
        print(f'   Amount: {result["amount"]} GXC')
        print(f'   Remaining Stake: {result["remaining_stake"]} GXC')
        print(f'   TX ID: {result["txid"]}')
        print(f'   Message: {result["message"]}')
        
    except Exception as e:
        print(f'❌ Error: {e}')


def example6_list_validators():
    """Example 6: List All Validators"""
    print('\n=== Example 6: List All Validators ===\n')
    
    try:
        validators = rpc('getValidators', [])
        
        print(f'✅ Found {len(validators)} validators\n')
        
        for i, v in enumerate(validators[:5], 1):
            print(f'{i}. {v["address"][:20]}...')
            print(f'   Stake: {v["stake_amount"]} GXC')
            print(f'   Weighted: {v["weighted_stake"]}')
            print(f'   Active: {v["is_active"]}')
            print(f'   Blocks: {v["blocks_produced"]}')
            print(f'   APY: {v["apy"]:.2f}%\n')
        
    except Exception as e:
        print(f'❌ Error: {e}')


def example7_verify_traceability():
    """Example 7: Verify Transaction Traceability"""
    print('\n=== Example 7: Verify Traceability ===\n')
    
    try:
        txid = '0x1234567890abcdef...'
        
        # Get transaction
        tx = rpc('gettransaction', [txid])
        
        print(f'Transaction: {txid[:16]}...')
        print(f'   Type: {tx.get("type", "NORMAL")}')
        print(f'   Amount: {tx.get("amount", 0)} GXC')
        
        # Verify traceability formula
        if tx.get('inputs') and len(tx['inputs']) > 0:
            hash_match = tx['inputs'][0]['txHash'] == tx['prevTxHash']
            amount_match = abs(tx['inputs'][0]['amount'] - tx['referencedAmount']) < 0.00000001
            
            print(f'\n   Traceability Formula:')
            print(f'   ✓ Inputs[0].txHash == PrevTxHash: {hash_match}')
            print(f'   ✓ Inputs[0].amount == ReferencedAmount: {amount_match}')
            
            if hash_match and amount_match:
                print(f'\n   ✅ Traceability VALID')
            else:
                print(f'\n   ❌ Traceability INVALID')
        
    except Exception as e:
        print(f'❌ Error: {e}')


def example8_trace_chain():
    """Example 8: Trace Transaction Chain"""
    print('\n=== Example 8: Trace Transaction Chain ===\n')
    
    try:
        txid = '0x1234567890abcdef...'
        
        print(f'Tracing transaction chain...\n')
        
        current_tx = txid
        depth = 0
        max_depth = 10
        
        while current_tx and current_tx != '0' and depth < max_depth:
            tx = rpc('gettransaction', [current_tx])
            
            print(f'{depth + 1}. TX: {current_tx[:16]}...')
            print(f'   Amount: {tx.get("amount", 0)} GXC')
            print(f'   Type: {tx.get("type", "NORMAL")}')
            print(f'   PrevTx: {tx.get("prevTxHash", "0")[:16]}...')
            
            current_tx = tx.get('prevTxHash', '0')
            depth += 1
        
        print(f'\n✅ Traced {depth} transactions')
        
    except Exception as e:
        print(f'❌ Error: {e}')


def example9_estimate_fee():
    """Example 9: Estimate Fee"""
    print('\n=== Example 9: Estimate Fee ===\n')
    
    try:
        fee_info = rpc('estimateFee', [])
        
        print(f'✅ Fee Estimation:')
        print(f'   Recommended: {fee_info["recommended_fee"]} GXC')
        print(f'   Base Fee: {fee_info["base_fee"]} GXC')
        print(f'   Max Fee: {fee_info["max_fee"]} GXC')
        print(f'   Pending TXs: {fee_info["pending_transactions"]}')
        print(f'   Dynamic Fees: {fee_info["dynamic_fees_enabled"]}')
        
        print(f'\n   Fee Tiers:')
        print(f'   - Low: {fee_info["fee_tiers"]["low"]} GXC')
        print(f'   - Medium: {fee_info["fee_tiers"]["medium"]} GXC')
        print(f'   - High: {fee_info["fee_tiers"]["high"]} GXC')
        
    except Exception as e:
        print(f'❌ Error: {e}')


def example10_monitor_validator():
    """Example 10: Monitor Validator Status"""
    print('\n=== Example 10: Monitor Validator Status ===\n')
    
    try:
        address = 'tGXC1234567890abcdef...'
        
        # Get validator info
        info = rpc('getValidatorInfo', [address])
        
        print(f'✅ Validator Status:')
        print(f'   Address: {info["address"][:30]}...')
        print(f'   Stake: {info["stake_amount"]} GXC')
        print(f'   Status: {info["status"]}')
        print(f'   Active: {info["is_active"]}')
        print(f'   Pending: {info["is_pending"]}')
        
        print(f'\n   Performance:')
        print(f'   - Blocks Produced: {info["blocks_produced"]}')
        print(f'   - Missed Blocks: {info["missed_blocks"]}')
        print(f'   - Uptime: {info["uptime"] * 100:.2f}%')
        
        print(f'\n   Rewards:')
        print(f'   - Total Rewards: {info["total_rewards"]} GXC')
        print(f'   - Pending Rewards: {info["pending_rewards"]} GXC')
        print(f'   - APY: {info["apy"]:.2f}%')
        
        print(f'\n   Staking:')
        print(f'   - Staking Days: {info["staking_days"]}')
        print(f'   - Weighted Stake: {info["weighted_stake"]}')
        print(f'   - Time Weight: {info["time_weight"]:.4f}')
        
    except Exception as e:
        print(f'❌ Error: {e}')


def run_all_examples():
    """Run all examples"""
    print('╔════════════════════════════════════════════════════════════╗')
    print('║         GXC Wallet Integration Examples (Python)          ║')
    print('╚════════════════════════════════════════════════════════════╝')
    
    # Check node connection
    try:
        info = rpc('getblockchaininfo', [])
        print(f'\n✅ Connected to GXC node')
        print(f'   Network: {info["chain"]}')
        print(f'   Height: {info["blocks"]}')
        print(f'   Version: {info.get("version", "unknown")}')
    except Exception as e:
        print(f'\n❌ Cannot connect to node at {NODE_URL}')
        print(f'   Please start the node first: ./build/gxc-node')
        return
    
    # Run examples (uncomment to run)
    # example1_send_transaction()
    # example2_check_balance()
    # example3_register_validator()
    # example4_add_stake()
    # example5_unstake()
    # example6_list_validators()
    # example7_verify_traceability()
    # example8_trace_chain()
    # example9_estimate_fee()
    # example10_monitor_validator()
    
    print('\n✅ Examples ready to run!')
    print('   Uncomment the examples you want to test.\n')


if __name__ == '__main__':
    run_all_examples()
