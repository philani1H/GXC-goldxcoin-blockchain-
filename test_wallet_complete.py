#!/usr/bin/env python3
"""
Complete wallet functionality test
Tests: wallet creation, mining, sending, staking, unstaking
"""

import os
import sys
import requests
import json
import time
import hashlib

RPC_URL = os.environ.get('BLOCKCHAIN_RPC_URL', 'http://localhost:8332')

class Colors:
    GREEN = '\033[92m'
    RED = '\033[91m'
    YELLOW = '\033[93m'
    BLUE = '\033[94m'
    CYAN = '\033[96m'
    END = '\033[0m'

def rpc_call(method, params=None):
    """Make RPC call to blockchain node"""
    if params is None:
        params = []
    
    payload = {
        "jsonrpc": "2.0",
        "method": method,
        "params": params,
        "id": 1
    }
    
    try:
        response = requests.post(RPC_URL, json=payload, timeout=10)
        result = response.json()
        
        if 'error' in result and result['error']:
            return None, result['error']
        
        return result.get('result'), None
    except Exception as e:
        return None, str(e)

def print_section(title):
    """Print section header"""
    print(f"\n{Colors.CYAN}{'='*70}{Colors.END}")
    print(f"{Colors.CYAN}{title}{Colors.END}")
    print(f"{Colors.CYAN}{'='*70}{Colors.END}")

def test_wallet_creation():
    """Test wallet creation"""
    print_section("TEST 1: Wallet Creation")
    
    # Create new address
    address, error = rpc_call("getnewaddress")
    
    if error:
        print(f"{Colors.RED}❌ Failed to create wallet: {error}{Colors.END}")
        return None
    
    print(f"{Colors.GREEN}✓ Wallet created successfully{Colors.END}")
    print(f"{Colors.BLUE}  Address: {address}{Colors.END}")
    
    # Check balance (should be 0)
    balance, error = rpc_call("getbalance", [address])
    if error:
        print(f"{Colors.YELLOW}⚠ Could not check balance: {error}{Colors.END}")
    else:
        print(f"{Colors.BLUE}  Balance: {balance} GXC{Colors.END}")
    
    return address

def test_mining(miner_address, num_blocks=3):
    """Test mining blocks"""
    print_section(f"TEST 2: Mining {num_blocks} Blocks")
    
    initial_height, _ = rpc_call("getblockcount")
    initial_height = initial_height or 0
    
    print(f"{Colors.BLUE}Initial height: {initial_height}{Colors.END}")
    print(f"{Colors.YELLOW}Mining {num_blocks} blocks to {miner_address}...{Colors.END}")
    
    blocks_mined = 0
    for i in range(num_blocks):
        print(f"\n{Colors.CYAN}Mining block {i + 1}/{num_blocks}...{Colors.END}")
        
        # Get block template
        template, error = rpc_call("getblocktemplate", [{"minerAddress": miner_address}])
        if error:
            print(f"{Colors.RED}❌ Failed to get template: {error}{Colors.END}")
            continue
        
        height = template.get('height', 0)
        prev_hash = template.get('previousblockhash', '0' * 64)
        difficulty = template.get('difficulty', 1.0)
        
        print(f"{Colors.BLUE}  Height: {height}, Difficulty: {difficulty}{Colors.END}")
        
        # Simple mining (for testing)
        nonce = 0
        start_time = time.time()
        
        while True:
            timestamp = int(time.time())
            block_header = f"{height}{prev_hash}{timestamp}{nonce}"
            block_hash = hashlib.sha256(block_header.encode()).hexdigest()
            
            # Check if valid (2 leading zeros for testnet)
            if block_hash.startswith('00'):
                elapsed = time.time() - start_time
                print(f"{Colors.GREEN}  ✓ Found valid hash in {elapsed:.2f}s{Colors.END}")
                
                # Submit block
                block_data = {
                    'hash': block_hash,
                    'previousblockhash': prev_hash,
                    'height': height,
                    'nonce': nonce,
                    'miner': miner_address,
                    'minerAddress': miner_address,
                    'timestamp': timestamp,
                    'difficulty': difficulty,
                    'transactions': template.get('transactions', [])
                }
                
                result, error = rpc_call("submitblock", [block_data])
                if error:
                    print(f"{Colors.RED}  ❌ Submit failed: {error}{Colors.END}")
                else:
                    print(f"{Colors.GREEN}  ✓ Block submitted{Colors.END}")
                    blocks_mined += 1
                    time.sleep(2)  # Wait for block to be processed
                break
            
            nonce += 1
            if nonce > 1000000:
                print(f"{Colors.RED}  ❌ Mining timeout{Colors.END}")
                break
    
    # Check final height
    final_height, _ = rpc_call("getblockcount")
    final_height = final_height or 0
    
    print(f"\n{Colors.GREEN}✓ Mining complete{Colors.END}")
    print(f"{Colors.BLUE}  Blocks mined: {blocks_mined}{Colors.END}")
    print(f"{Colors.BLUE}  Height: {initial_height} → {final_height}{Colors.END}")
    
    # Check balance after mining
    time.sleep(2)
    balance, _ = rpc_call("getbalance", [miner_address])
    if balance:
        print(f"{Colors.GREEN}  💰 Balance: {balance} GXC{Colors.END}")
    
    return blocks_mined > 0

def test_send_transaction(from_address, to_address, amount):
    """Test sending transaction"""
    print_section(f"TEST 3: Send Transaction ({amount} GXC)")
    
    # Check sender balance
    sender_balance, _ = rpc_call("getbalance", [from_address])
    if not sender_balance or sender_balance < amount:
        print(f"{Colors.RED}❌ Insufficient balance: {sender_balance} GXC{Colors.END}")
        return False
    
    print(f"{Colors.BLUE}Sender balance: {sender_balance} GXC{Colors.END}")
    print(f"{Colors.YELLOW}Sending {amount} GXC from {from_address[:20]}... to {to_address[:20]}...{Colors.END}")
    
    # Send transaction
    tx_hash, error = rpc_call("sendtoaddress", [to_address, amount])
    
    if error:
        print(f"{Colors.RED}❌ Transaction failed: {error}{Colors.END}")
        return False
    
    print(f"{Colors.GREEN}✓ Transaction sent{Colors.END}")
    print(f"{Colors.BLUE}  TX Hash: {tx_hash}{Colors.END}")
    
    # Wait for transaction to be processed
    time.sleep(3)
    
    # Check balances
    sender_balance_after, _ = rpc_call("getbalance", [from_address])
    receiver_balance, _ = rpc_call("getbalance", [to_address])
    
    print(f"{Colors.BLUE}  Sender balance: {sender_balance} → {sender_balance_after} GXC{Colors.END}")
    print(f"{Colors.BLUE}  Receiver balance: {receiver_balance} GXC{Colors.END}")
    
    return True

def test_staking(address, stake_amount, staking_days=30):
    """Test staking"""
    print_section(f"TEST 4: Stake {stake_amount} GXC for {staking_days} days")
    
    # Check balance
    balance, _ = rpc_call("getbalance", [address])
    if not balance or balance < stake_amount:
        print(f"{Colors.RED}❌ Insufficient balance: {balance} GXC{Colors.END}")
        return False
    
    print(f"{Colors.BLUE}Balance: {balance} GXC{Colors.END}")
    print(f"{Colors.YELLOW}Staking {stake_amount} GXC for {staking_days} days...{Colors.END}")
    
    # Stake
    result, error = rpc_call("stake", [address, stake_amount, staking_days])
    
    if error:
        print(f"{Colors.RED}❌ Staking failed: {error}{Colors.END}")
        return False
    
    print(f"{Colors.GREEN}✓ Staking successful{Colors.END}")
    
    if isinstance(result, dict):
        print(f"{Colors.BLUE}  TX Hash: {result.get('txHash', 'N/A')}{Colors.END}")
        print(f"{Colors.BLUE}  Stake Amount: {result.get('stakeAmount', stake_amount)} GXC{Colors.END}")
        print(f"{Colors.BLUE}  Staking Days: {result.get('stakingDays', staking_days)}{Colors.END}")
    
    # Wait for staking to be processed
    time.sleep(3)
    
    # Check staking info
    staking_info, _ = rpc_call("getstakinginfo", [address])
    if staking_info:
        print(f"{Colors.BLUE}  Total Staked: {staking_info.get('totalStaked', 0)} GXC{Colors.END}")
        print(f"{Colors.BLUE}  Active Stakes: {len(staking_info.get('stakes', []))}{Colors.END}")
    
    return True

def test_unstaking(address):
    """Test unstaking"""
    print_section("TEST 5: Unstake")
    
    # Get staking info
    staking_info, error = rpc_call("getstakinginfo", [address])
    
    if error or not staking_info:
        print(f"{Colors.RED}❌ No staking info: {error}{Colors.END}")
        return False
    
    stakes = staking_info.get('stakes', [])
    if not stakes:
        print(f"{Colors.YELLOW}⚠ No active stakes to unstake{Colors.END}")
        return False
    
    print(f"{Colors.BLUE}Active stakes: {len(stakes)}{Colors.END}")
    
    # Unstake first stake
    stake = stakes[0]
    stake_tx_hash = stake.get('txHash', '')
    
    if not stake_tx_hash:
        print(f"{Colors.RED}❌ Invalid stake transaction hash{Colors.END}")
        return False
    
    print(f"{Colors.YELLOW}Unstaking {stake.get('amount', 0)} GXC...{Colors.END}")
    
    # Unstake
    result, error = rpc_call("unstake", [address, stake_tx_hash])
    
    if error:
        print(f"{Colors.RED}❌ Unstaking failed: {error}{Colors.END}")
        return False
    
    print(f"{Colors.GREEN}✓ Unstaking successful{Colors.END}")
    
    if isinstance(result, dict):
        print(f"{Colors.BLUE}  TX Hash: {result.get('txHash', 'N/A')}{Colors.END}")
        print(f"{Colors.BLUE}  Amount: {result.get('amount', 0)} GXC{Colors.END}")
    
    # Wait for unstaking to be processed
    time.sleep(3)
    
    # Check balance
    balance, _ = rpc_call("getbalance", [address])
    if balance:
        print(f"{Colors.BLUE}  Balance after unstake: {balance} GXC{Colors.END}")
    
    return True

def main():
    print(f"{Colors.CYAN}{'='*70}{Colors.END}")
    print(f"{Colors.CYAN}GXC Wallet Complete Functionality Test{Colors.END}")
    print(f"{Colors.CYAN}{'='*70}{Colors.END}")
    print(f"{Colors.BLUE}RPC URL: {RPC_URL}{Colors.END}")
    
    # Test connection
    info, error = rpc_call("getblockchaininfo")
    if error:
        print(f"{Colors.RED}❌ Cannot connect to node: {error}{Colors.END}")
        return 1
    
    print(f"{Colors.GREEN}✓ Connected to blockchain{Colors.END}")
    print(f"{Colors.BLUE}  Height: {info.get('blocks', 0)}{Colors.END}")
    print(f"{Colors.BLUE}  Difficulty: {info.get('difficulty', 1.0)}{Colors.END}")
    
    # Test 1: Create wallets
    wallet1 = test_wallet_creation()
    if not wallet1:
        return 1
    
    time.sleep(1)
    
    wallet2 = test_wallet_creation()
    if not wallet2:
        return 1
    
    # Test 2: Mine blocks to wallet1
    if not test_mining(wallet1, num_blocks=5):
        print(f"{Colors.RED}❌ Mining test failed{Colors.END}")
        return 1
    
    # Test 3: Send transaction from wallet1 to wallet2
    if not test_send_transaction(wallet1, wallet2, 10.0):
        print(f"{Colors.RED}❌ Send transaction test failed{Colors.END}")
        return 1
    
    # Test 4: Stake from wallet1
    if not test_staking(wallet1, 20.0, 30):
        print(f"{Colors.RED}❌ Staking test failed{Colors.END}")
        return 1
    
    # Test 5: Unstake from wallet1
    if not test_unstaking(wallet1):
        print(f"{Colors.YELLOW}⚠ Unstaking test skipped or failed{Colors.END}")
    
    # Final summary
    print_section("Test Summary")
    
    balance1, _ = rpc_call("getbalance", [wallet1])
    balance2, _ = rpc_call("getbalance", [wallet2])
    
    print(f"{Colors.GREEN}✓ All tests completed{Colors.END}")
    print(f"{Colors.BLUE}Wallet 1: {wallet1}{Colors.END}")
    print(f"{Colors.BLUE}  Balance: {balance1} GXC{Colors.END}")
    print(f"{Colors.BLUE}Wallet 2: {wallet2}{Colors.END}")
    print(f"{Colors.BLUE}  Balance: {balance2} GXC{Colors.END}")
    
    return 0

if __name__ == "__main__":
    sys.exit(main())
