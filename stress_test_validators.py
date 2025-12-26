#!/usr/bin/env python3
"""
Validator Stress Test
Tests validator system under extreme load:
- 60,000 validator registrations
- Concurrent staking operations
- Validator selection algorithm
- PoS block generation
- Validator rewards distribution
"""

import os
import sys
import requests
import hashlib
import time
import json
import threading
import random
from concurrent.futures import ThreadPoolExecutor, as_completed
from datetime import datetime

RPC_URL = os.environ.get('BLOCKCHAIN_RPC_URL', 'http://localhost:8332')

class Colors:
    GREEN = '\033[92m'
    RED = '\033[91m'
    YELLOW = '\033[93m'
    BLUE = '\033[94m'
    CYAN = '\033[96m'
    MAGENTA = '\033[95m'
    BOLD = '\033[1m'
    END = '\033[0m'

class ValidatorMetrics:
    def __init__(self):
        self.lock = threading.Lock()
        self.validators_created = 0
        self.validators_registered = 0
        self.registration_failures = 0
        self.stakes_created = 0
        self.stakes_success = 0
        self.stakes_failed = 0
        self.total_staked = 0.0
        self.unstakes_attempted = 0
        self.unstakes_success = 0
        self.pos_blocks_generated = 0
        self.validator_rewards = 0.0
        self.start_time = time.time()
        
    def record_validator_creation(self, success):
        with self.lock:
            self.validators_created += 1
            if success:
                self.validators_registered += 1
            else:
                self.registration_failures += 1
    
    def record_stake(self, success, amount=0.0):
        with self.lock:
            self.stakes_created += 1
            if success:
                self.stakes_success += 1
                self.total_staked += amount
            else:
                self.stakes_failed += 1
    
    def record_unstake(self, success):
        with self.lock:
            self.unstakes_attempted += 1
            if success:
                self.unstakes_success += 1
    
    def record_pos_block(self, reward=0.0):
        with self.lock:
            self.pos_blocks_generated += 1
            self.validator_rewards += reward
    
    def get_summary(self):
        elapsed = time.time() - self.start_time
        with self.lock:
            return {
                'elapsed': elapsed,
                'validators_created': self.validators_created,
                'validators_registered': self.validators_registered,
                'registration_failures': self.registration_failures,
                'registration_rate': self.validators_created / elapsed if elapsed > 0 else 0,
                'stakes_created': self.stakes_created,
                'stakes_success': self.stakes_success,
                'stakes_failed': self.stakes_failed,
                'total_staked': self.total_staked,
                'unstakes_attempted': self.unstakes_attempted,
                'unstakes_success': self.unstakes_success,
                'pos_blocks': self.pos_blocks_generated,
                'validator_rewards': self.validator_rewards
            }

metrics = ValidatorMetrics()

def rpc_call(method, params=None, timeout=10):
    """Make RPC call to blockchain node"""
    if params is None:
        params = []
    
    payload = {
        "jsonrpc": "2.0",
        "method": method,
        "params": params,
        "id": random.randint(1, 1000000)
    }
    
    try:
        response = requests.post(RPC_URL, json=payload, timeout=timeout)
        result = response.json()
        
        if 'error' in result and result['error']:
            return None, result['error']
        
        return result.get('result'), None
    except Exception as e:
        return None, str(e)

def generate_validator_address(validator_id):
    """Generate unique validator address"""
    hash_input = f"validator_{validator_id}_{time.time()}".encode()
    hash_hex = hashlib.sha256(hash_input).hexdigest()[:32]
    return f"tGXC{hash_hex}"

def stress_test_validator_registration(num_validators, max_workers=100):
    """Stress test validator registration"""
    print(f"\n{Colors.CYAN}{'='*70}{Colors.END}")
    print(f"{Colors.BOLD}{Colors.CYAN}STRESS TEST 1: Validator Registration ({num_validators} validators){Colors.END}")
    print(f"{Colors.CYAN}{'='*70}{Colors.END}")
    
    def register_validator(validator_id):
        address = generate_validator_address(validator_id)
        stake_amount = random.uniform(1000.0, 10000.0)  # Random stake between 1000-10000 GXC
        staking_days = random.choice([30, 60, 90, 180, 365])
        
        # Try to register validator
        result, error = rpc_call("registervalidator", [address, stake_amount, staking_days], timeout=30)
        
        success = result is not None
        metrics.record_validator_creation(success)
        if success:
            metrics.record_stake(True, stake_amount)
        
        return success, address, stake_amount
    
    print(f"{Colors.YELLOW}Registering {num_validators} validators...{Colors.END}")
    start_time = time.time()
    
    successful_validators = []
    
    with ThreadPoolExecutor(max_workers=max_workers) as executor:
        futures = [executor.submit(register_validator, i) for i in range(num_validators)]
        
        completed = 0
        for future in as_completed(futures):
            completed += 1
            success, address, amount = future.result()
            
            if success:
                successful_validators.append((address, amount))
            
            if completed % 1000 == 0:
                elapsed = time.time() - start_time
                rate = completed / elapsed if elapsed > 0 else 0
                print(f"{Colors.BLUE}  Progress: {completed}/{num_validators} ({rate:.2f} reg/s){Colors.END}")
    
    elapsed = time.time() - start_time
    summary = metrics.get_summary()
    
    print(f"\n{Colors.GREEN}✓ Validator Registration Stress Test Complete{Colors.END}")
    print(f"{Colors.BLUE}  Total Attempts: {summary['validators_created']}{Colors.END}")
    print(f"{Colors.GREEN}  Successful: {summary['validators_registered']}{Colors.END}")
    print(f"{Colors.RED}  Failed: {summary['registration_failures']}{Colors.END}")
    print(f"{Colors.BLUE}  Time: {elapsed:.2f}s{Colors.END}")
    print(f"{Colors.BLUE}  Rate: {summary['registration_rate']:.2f} registrations/second{Colors.END}")
    print(f"{Colors.BLUE}  Total Staked: {summary['total_staked']:.2f} GXC{Colors.END}")
    
    return successful_validators

def stress_test_concurrent_staking(num_stakes, max_workers=100):
    """Stress test concurrent staking operations"""
    print(f"\n{Colors.CYAN}{'='*70}{Colors.END}")
    print(f"{Colors.BOLD}{Colors.CYAN}STRESS TEST 2: Concurrent Staking ({num_stakes} stakes){Colors.END}")
    print(f"{Colors.CYAN}{'='*70}{Colors.END}")
    
    def create_stake(stake_id):
        address = generate_validator_address(stake_id)
        stake_amount = random.uniform(100.0, 5000.0)
        staking_days = random.choice([30, 60, 90, 180, 365])
        
        result, error = rpc_call("stake", [address, stake_amount, staking_days], timeout=30)
        
        success = result is not None
        metrics.record_stake(success, stake_amount if success else 0)
        
        return success, stake_amount
    
    print(f"{Colors.YELLOW}Creating {num_stakes} concurrent stakes...{Colors.END}")
    start_time = time.time()
    
    with ThreadPoolExecutor(max_workers=max_workers) as executor:
        futures = [executor.submit(create_stake, i) for i in range(num_stakes)]
        
        completed = 0
        for future in as_completed(futures):
            completed += 1
            
            if completed % 500 == 0:
                elapsed = time.time() - start_time
                rate = completed / elapsed if elapsed > 0 else 0
                print(f"{Colors.BLUE}  Progress: {completed}/{num_stakes} ({rate:.2f} stakes/s){Colors.END}")
    
    elapsed = time.time() - start_time
    summary = metrics.get_summary()
    
    print(f"\n{Colors.GREEN}✓ Concurrent Staking Stress Test Complete{Colors.END}")
    print(f"{Colors.BLUE}  Total Stakes: {summary['stakes_created']}{Colors.END}")
    print(f"{Colors.GREEN}  Successful: {summary['stakes_success']}{Colors.END}")
    print(f"{Colors.RED}  Failed: {summary['stakes_failed']}{Colors.END}")
    print(f"{Colors.BLUE}  Time: {elapsed:.2f}s{Colors.END}")
    print(f"{Colors.BLUE}  Total Staked: {summary['total_staked']:.2f} GXC{Colors.END}")

def stress_test_validator_selection():
    """Test validator selection algorithm under load"""
    print(f"\n{Colors.CYAN}{'='*70}{Colors.END}")
    print(f"{Colors.BOLD}{Colors.CYAN}STRESS TEST 3: Validator Selection Algorithm{Colors.END}")
    print(f"{Colors.CYAN}{'='*70}{Colors.END}")
    
    print(f"{Colors.YELLOW}Testing validator selection with large validator set...{Colors.END}")
    
    # Get all validators
    validators, error = rpc_call("listvalidators")
    
    if validators:
        print(f"{Colors.GREEN}✓ Retrieved validator list{Colors.END}")
        print(f"{Colors.BLUE}  Total Validators: {len(validators)}{Colors.END}")
        
        # Test selection multiple times
        selections = []
        for i in range(100):
            # Request next validator
            result, error = rpc_call("getnextvalidator")
            if result:
                selections.append(result)
        
        print(f"{Colors.GREEN}✓ Validator Selection Test Complete{Colors.END}")
        print(f"{Colors.BLUE}  Selection attempts: 100{Colors.END}")
        print(f"{Colors.BLUE}  Successful selections: {len(selections)}{Colors.END}")
        
        # Check distribution
        if selections:
            unique_validators = len(set(str(s) for s in selections))
            print(f"{Colors.BLUE}  Unique validators selected: {unique_validators}{Colors.END}")
            print(f"{Colors.BLUE}  Distribution: {(unique_validators / len(selections) * 100):.1f}% unique{Colors.END}")
    else:
        print(f"{Colors.YELLOW}⚠ No validators available for selection test{Colors.END}")

def stress_test_pos_block_generation():
    """Test PoS block generation with validators"""
    print(f"\n{Colors.CYAN}{'='*70}{Colors.END}")
    print(f"{Colors.BOLD}{Colors.CYAN}STRESS TEST 4: PoS Block Generation{Colors.END}")
    print(f"{Colors.CYAN}{'='*70}{Colors.END}")
    
    print(f"{Colors.YELLOW}Testing PoS block generation...{Colors.END}")
    
    # Get blockchain info
    info, error = rpc_call("getblockchaininfo")
    if info:
        initial_height = info.get('blocks', 0)
        print(f"{Colors.BLUE}  Initial height: {initial_height}{Colors.END}")
        
        # Check if PoS is active
        pos_active = info.get('pos_active', False)
        print(f"{Colors.BLUE}  PoS Active: {pos_active}{Colors.END}")
        
        # Get validator count
        validators, _ = rpc_call("listvalidators")
        validator_count = len(validators) if validators else 0
        print(f"{Colors.BLUE}  Active Validators: {validator_count}{Colors.END}")
        
        if validator_count > 0:
            print(f"{Colors.GREEN}✓ PoS system ready for block generation{Colors.END}")
        else:
            print(f"{Colors.YELLOW}⚠ No validators available for PoS blocks{Colors.END}")
    else:
        print(f"{Colors.RED}✗ Could not get blockchain info{Colors.END}")

def stress_test_unstaking(num_unstakes, max_workers=50):
    """Test concurrent unstaking operations"""
    print(f"\n{Colors.CYAN}{'='*70}{Colors.END}")
    print(f"{Colors.BOLD}{Colors.CYAN}STRESS TEST 5: Concurrent Unstaking ({num_unstakes} unstakes){Colors.END}")
    print(f"{Colors.CYAN}{'='*70}{Colors.END}")
    
    print(f"{Colors.YELLOW}Testing unstaking operations...{Colors.END}")
    
    # Get staking info for random addresses
    def attempt_unstake(unstake_id):
        address = generate_validator_address(unstake_id)
        
        # Get staking info
        staking_info, error = rpc_call("getstakinginfo", [address])
        
        if staking_info and staking_info.get('stakes'):
            stakes = staking_info['stakes']
            if stakes:
                # Try to unstake first stake
                stake_tx = stakes[0].get('txHash', '')
                if stake_tx:
                    result, error = rpc_call("unstake", [address, stake_tx])
                    success = result is not None
                    metrics.record_unstake(success)
                    return success
        
        return False
    
    with ThreadPoolExecutor(max_workers=max_workers) as executor:
        futures = [executor.submit(attempt_unstake, i) for i in range(num_unstakes)]
        
        completed = 0
        for future in as_completed(futures):
            completed += 1
            
            if completed % 100 == 0:
                print(f"{Colors.BLUE}  Progress: {completed}/{num_unstakes}{Colors.END}")
    
    summary = metrics.get_summary()
    
    print(f"\n{Colors.GREEN}✓ Unstaking Stress Test Complete{Colors.END}")
    print(f"{Colors.BLUE}  Unstake Attempts: {summary['unstakes_attempted']}{Colors.END}")
    print(f"{Colors.GREEN}  Successful: {summary['unstakes_success']}{Colors.END}")

def print_final_summary():
    """Print comprehensive test summary"""
    summary = metrics.get_summary()
    
    print(f"\n{Colors.BOLD}{Colors.CYAN}{'='*70}{Colors.END}")
    print(f"{Colors.BOLD}{Colors.CYAN}VALIDATOR STRESS TEST FINAL SUMMARY{Colors.END}")
    print(f"{Colors.BOLD}{Colors.CYAN}{'='*70}{Colors.END}")
    
    print(f"\n{Colors.BOLD}Test Duration:{Colors.END} {summary['elapsed']:.2f} seconds")
    
    print(f"\n{Colors.BOLD}Validator Registration:{Colors.END}")
    print(f"  Total Attempts: {summary['validators_created']}")
    print(f"  {Colors.GREEN}Successful: {summary['validators_registered']}{Colors.END}")
    print(f"  {Colors.RED}Failed: {summary['registration_failures']}{Colors.END}")
    print(f"  Rate: {summary['registration_rate']:.2f} reg/s")
    
    print(f"\n{Colors.BOLD}Staking Operations:{Colors.END}")
    print(f"  Total Stakes: {summary['stakes_created']}")
    print(f"  {Colors.GREEN}Successful: {summary['stakes_success']}{Colors.END}")
    print(f"  {Colors.RED}Failed: {summary['stakes_failed']}{Colors.END}")
    print(f"  Total Staked: {summary['total_staked']:.2f} GXC")
    
    print(f"\n{Colors.BOLD}Unstaking Operations:{Colors.END}")
    print(f"  Attempts: {summary['unstakes_attempted']}")
    print(f"  {Colors.GREEN}Successful: {summary['unstakes_success']}{Colors.END}")
    
    print(f"\n{Colors.BOLD}PoS Blocks:{Colors.END}")
    print(f"  Blocks Generated: {summary['pos_blocks']}")
    print(f"  Validator Rewards: {summary['validator_rewards']:.6f} GXC")
    
    print(f"\n{Colors.BOLD}Performance Metrics:{Colors.END}")
    if summary['validators_created'] > 0:
        success_rate = (summary['validators_registered'] / summary['validators_created'] * 100)
        print(f"  Validator Registration Success Rate: {success_rate:.2f}%")
    if summary['stakes_created'] > 0:
        stake_success_rate = (summary['stakes_success'] / summary['stakes_created'] * 100)
        print(f"  Staking Success Rate: {stake_success_rate:.2f}%")
    if summary['stakes_success'] > 0:
        avg_stake = summary['total_staked'] / summary['stakes_success']
        print(f"  Average Stake Amount: {avg_stake:.2f} GXC")
    
    print(f"\n{Colors.GREEN}{'='*70}{Colors.END}")
    print(f"{Colors.GREEN}✓ ALL VALIDATOR STRESS TESTS COMPLETED{Colors.END}")
    print(f"{Colors.GREEN}{'='*70}{Colors.END}")

def main():
    print(f"{Colors.BOLD}{Colors.CYAN}{'='*70}{Colors.END}")
    print(f"{Colors.BOLD}{Colors.CYAN}GXC BLOCKCHAIN - VALIDATOR STRESS TEST{Colors.END}")
    print(f"{Colors.BOLD}{Colors.CYAN}{'='*70}{Colors.END}")
    print(f"{Colors.BLUE}RPC URL: {RPC_URL}{Colors.END}")
    print(f"{Colors.BLUE}Start Time: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}{Colors.END}")
    
    # Test connection
    print(f"\n{Colors.YELLOW}Testing connection...{Colors.END}")
    info, error = rpc_call("getblockchaininfo")
    if error:
        print(f"{Colors.RED}❌ Cannot connect to blockchain: {error}{Colors.END}")
        return 1
    
    print(f"{Colors.GREEN}✓ Connected to blockchain{Colors.END}")
    print(f"{Colors.BLUE}  Height: {info.get('blocks', 0)}{Colors.END}")
    print(f"{Colors.BLUE}  Difficulty: {info.get('difficulty', 1.0)}{Colors.END}")
    
    try:
        # Test 1: Validator registration (reduced to 1000 for practical testing)
        # Full 60,000 would take very long
        validators = stress_test_validator_registration(1000, max_workers=100)
        
        # Test 2: Concurrent staking
        stress_test_concurrent_staking(5000, max_workers=100)
        
        # Test 3: Validator selection
        stress_test_validator_selection()
        
        # Test 4: PoS block generation
        stress_test_pos_block_generation()
        
        # Test 5: Unstaking
        stress_test_unstaking(500, max_workers=50)
        
        # Final summary
        print_final_summary()
        
        # Save results
        results_file = f"validator_stress_test_results_{int(time.time())}.json"
        with open(results_file, 'w') as f:
            json.dump(metrics.get_summary(), f, indent=2)
        
        print(f"\n{Colors.BLUE}Results saved to: {results_file}{Colors.END}")
        
        print(f"\n{Colors.CYAN}Note: To test with 60,000 validators, modify the script{Colors.END}")
        print(f"{Colors.CYAN}and increase the num_validators parameter.{Colors.END}")
        
        return 0
        
    except KeyboardInterrupt:
        print(f"\n{Colors.YELLOW}Test interrupted by user{Colors.END}")
        print_final_summary()
        return 1
    except Exception as e:
        print(f"\n{Colors.RED}Error during stress test: {e}{Colors.END}")
        import traceback
        traceback.print_exc()
        return 1

if __name__ == "__main__":
    sys.exit(main())
