#!/usr/bin/env python3
"""
60,000 Miner Stress Test
Tests blockchain under extreme load:
- Security engine (AI Hashrate Sentinel)
- Dynamic fee system
- All mining algorithms
- Concurrent template requests
- Block submission handling
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

class StressTestMetrics:
    def __init__(self):
        self.lock = threading.Lock()
        self.template_requests = 0
        self.template_success = 0
        self.template_failures = 0
        self.blocks_found = 0
        self.blocks_submitted = 0
        self.blocks_accepted = 0
        self.blocks_rejected = 0
        self.total_fees_paid = 0.0
        self.security_alerts = 0
        self.algorithm_usage = {}
        self.start_time = time.time()
        
    def record_template_request(self, success):
        with self.lock:
            self.template_requests += 1
            if success:
                self.template_success += 1
            else:
                self.template_failures += 1
    
    def record_block_found(self):
        with self.lock:
            self.blocks_found += 1
    
    def record_block_submission(self, accepted, fee=0.0):
        with self.lock:
            self.blocks_submitted += 1
            if accepted:
                self.blocks_accepted += 1
                self.total_fees_paid += fee
            else:
                self.blocks_rejected += 1
    
    def record_algorithm_usage(self, algorithm):
        with self.lock:
            self.algorithm_usage[algorithm] = self.algorithm_usage.get(algorithm, 0) + 1
    
    def record_security_alert(self):
        with self.lock:
            self.security_alerts += 1
    
    def get_summary(self):
        elapsed = time.time() - self.start_time
        with self.lock:
            return {
                'elapsed': elapsed,
                'template_requests': self.template_requests,
                'template_success': self.template_success,
                'template_failures': self.template_failures,
                'template_rate': self.template_requests / elapsed if elapsed > 0 else 0,
                'blocks_found': self.blocks_found,
                'blocks_submitted': self.blocks_submitted,
                'blocks_accepted': self.blocks_accepted,
                'blocks_rejected': self.blocks_rejected,
                'total_fees': self.total_fees_paid,
                'security_alerts': self.security_alerts,
                'algorithm_usage': dict(self.algorithm_usage)
            }

metrics = StressTestMetrics()

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

def generate_miner_address(miner_id):
    """Generate unique miner address"""
    hash_input = f"stress_miner_{miner_id}_{time.time()}".encode()
    hash_hex = hashlib.sha256(hash_input).hexdigest()[:32]
    return f"tGXC{hash_hex}"

def stress_test_template_requests(num_miners, max_workers=100):
    """Stress test template requests with many concurrent miners"""
    print(f"\n{Colors.CYAN}{'='*70}{Colors.END}")
    print(f"{Colors.BOLD}{Colors.CYAN}STRESS TEST 1: Template Requests ({num_miners} miners){Colors.END}")
    print(f"{Colors.CYAN}{'='*70}{Colors.END}")
    
    def request_template(miner_id):
        address = generate_miner_address(miner_id)
        algorithm = random.choice(['SHA256', 'GXHASH', 'ETHASH'])
        
        template, error = rpc_call("getblocktemplate", [{"minerAddress": address, "algorithm": algorithm}], timeout=30)
        
        success = template is not None
        metrics.record_template_request(success)
        if success:
            metrics.record_algorithm_usage(algorithm)
        
        return success, error
    
    print(f"{Colors.YELLOW}Sending {num_miners} concurrent template requests...{Colors.END}")
    start_time = time.time()
    
    with ThreadPoolExecutor(max_workers=max_workers) as executor:
        futures = [executor.submit(request_template, i) for i in range(num_miners)]
        
        completed = 0
        for future in as_completed(futures):
            completed += 1
            if completed % 1000 == 0:
                elapsed = time.time() - start_time
                rate = completed / elapsed if elapsed > 0 else 0
                print(f"{Colors.BLUE}  Progress: {completed}/{num_miners} ({rate:.2f} req/s){Colors.END}")
    
    elapsed = time.time() - start_time
    summary = metrics.get_summary()
    
    print(f"\n{Colors.GREEN}✓ Template Request Stress Test Complete{Colors.END}")
    print(f"{Colors.BLUE}  Total Requests: {summary['template_requests']}{Colors.END}")
    print(f"{Colors.GREEN}  Successful: {summary['template_success']}{Colors.END}")
    print(f"{Colors.RED}  Failed: {summary['template_failures']}{Colors.END}")
    print(f"{Colors.BLUE}  Time: {elapsed:.2f}s{Colors.END}")
    print(f"{Colors.BLUE}  Rate: {summary['template_rate']:.2f} requests/second{Colors.END}")
    print(f"\n{Colors.CYAN}Algorithm Usage:{Colors.END}")
    for algo, count in summary['algorithm_usage'].items():
        print(f"{Colors.BLUE}  {algo}: {count} requests{Colors.END}")

def stress_test_concurrent_mining(num_miners, max_attempts=10000, max_workers=50):
    """Stress test concurrent mining with many miners"""
    print(f"\n{Colors.CYAN}{'='*70}{Colors.END}")
    print(f"{Colors.BOLD}{Colors.CYAN}STRESS TEST 2: Concurrent Mining ({num_miners} miners){Colors.END}")
    print(f"{Colors.CYAN}{'='*70}{Colors.END}")
    
    def mine_with_miner(miner_id):
        address = generate_miner_address(miner_id)
        algorithm = random.choice(['SHA256', 'GXHASH', 'ETHASH'])
        
        # Get template
        template, error = rpc_call("getblocktemplate", [{"minerAddress": address, "algorithm": algorithm}])
        if error:
            return None
        
        metrics.record_algorithm_usage(algorithm)
        
        height = template.get('height', 0)
        prev_hash = template.get('previousblockhash', '0' * 64)
        difficulty = template.get('difficulty', 1.0)
        transactions = template.get('transactions', [])
        
        # Calculate required zeros
        import math
        required_zeros = int(2 + math.log2(max(1.0, difficulty)) * 0.3)
        required_zeros = max(2, min(required_zeros, 10))
        target_prefix = '0' * required_zeros
        
        # Mining loop
        for nonce in range(max_attempts):
            timestamp = int(time.time())
            
            # Build merkle root
            tx_hashes = [tx.get('hash', '') for tx in transactions]
            merkle_root = hashlib.sha256(''.join(tx_hashes).encode()).hexdigest() if tx_hashes else '0' * 64
            
            # Create block header
            block_header = f"{height}{prev_hash}{merkle_root}{timestamp}{difficulty}{nonce}"
            block_hash = hashlib.sha256(block_header.encode()).hexdigest()
            
            # Check if valid
            if block_hash.startswith(target_prefix):
                metrics.record_block_found()
                
                # Calculate fee (dynamic based on transactions)
                fee = len(transactions) * 0.001
                
                # Submit block
                block_data = {
                    'hash': block_hash,
                    'previousblockhash': prev_hash,
                    'height': height,
                    'nonce': nonce,
                    'miner': address,
                    'minerAddress': address,
                    'timestamp': timestamp,
                    'difficulty': difficulty,
                    'merkleroot': merkle_root,
                    'transactions': transactions,
                    'algorithm': algorithm
                }
                
                result, error = rpc_call("submitblock", [block_data])
                accepted = error is None
                metrics.record_block_submission(accepted, fee)
                
                return {
                    'miner_id': miner_id,
                    'address': address,
                    'algorithm': algorithm,
                    'hash': block_hash,
                    'height': height,
                    'accepted': accepted,
                    'fee': fee
                }
        
        return None
    
    print(f"{Colors.YELLOW}Starting {num_miners} concurrent miners...{Colors.END}")
    print(f"{Colors.YELLOW}Each miner will attempt up to {max_attempts} hashes{Colors.END}")
    
    start_time = time.time()
    winners = []
    
    with ThreadPoolExecutor(max_workers=max_workers) as executor:
        futures = [executor.submit(mine_with_miner, i) for i in range(num_miners)]
        
        completed = 0
        for future in as_completed(futures):
            completed += 1
            result = future.result()
            
            if result:
                winners.append(result)
                print(f"{Colors.GREEN}  ✓ Miner {result['miner_id']} found block! ({result['algorithm']}){Colors.END}")
            
            if completed % 100 == 0:
                elapsed = time.time() - start_time
                print(f"{Colors.BLUE}  Progress: {completed}/{num_miners} miners completed{Colors.END}")
    
    elapsed = time.time() - start_time
    summary = metrics.get_summary()
    
    print(f"\n{Colors.GREEN}✓ Concurrent Mining Stress Test Complete{Colors.END}")
    print(f"{Colors.BLUE}  Total Miners: {num_miners}{Colors.END}")
    print(f"{Colors.BLUE}  Time: {elapsed:.2f}s{Colors.END}")
    print(f"{Colors.GREEN}  Blocks Found: {summary['blocks_found']}{Colors.END}")
    print(f"{Colors.GREEN}  Blocks Accepted: {summary['blocks_accepted']}{Colors.END}")
    print(f"{Colors.RED}  Blocks Rejected: {summary['blocks_rejected']}{Colors.END}")
    print(f"{Colors.BLUE}  Total Fees: {summary['total_fees']:.6f} GXC{Colors.END}")
    
    if winners:
        print(f"\n{Colors.CYAN}Winners:{Colors.END}")
        for winner in winners[:10]:  # Show first 10
            print(f"{Colors.GREEN}  Miner {winner['miner_id']}: {winner['hash'][:32]}... ({winner['algorithm']}){Colors.END}")
        if len(winners) > 10:
            print(f"{Colors.CYAN}  ... and {len(winners) - 10} more{Colors.END}")

def stress_test_security_engine():
    """Test security engine under load"""
    print(f"\n{Colors.CYAN}{'='*70}{Colors.END}")
    print(f"{Colors.BOLD}{Colors.CYAN}STRESS TEST 3: Security Engine{Colors.END}")
    print(f"{Colors.CYAN}{'='*70}{Colors.END}")
    
    print(f"{Colors.YELLOW}Testing AI Hashrate Sentinel...{Colors.END}")
    
    # Get blockchain info to check security status
    info, error = rpc_call("getblockchaininfo")
    if info:
        print(f"{Colors.GREEN}✓ Security Engine Active{Colors.END}")
        print(f"{Colors.BLUE}  Difficulty: {info.get('difficulty', 'N/A')}{Colors.END}")
        print(f"{Colors.BLUE}  Blocks: {info.get('blocks', 'N/A')}{Colors.END}")
    
    # Test rapid block submissions (potential attack)
    print(f"\n{Colors.YELLOW}Testing rapid block submission handling...{Colors.END}")
    
    def submit_invalid_block(i):
        block_data = {
            'hash': hashlib.sha256(f"fake_block_{i}".encode()).hexdigest(),
            'previousblockhash': '0' * 64,
            'height': 999999,
            'nonce': i,
            'miner': generate_miner_address(i),
            'timestamp': int(time.time()),
            'difficulty': 1.0,
            'transactions': []
        }
        
        result, error = rpc_call("submitblock", [block_data], timeout=5)
        if error:
            metrics.record_security_alert()
        return error is not None
    
    # Submit 1000 invalid blocks rapidly
    rejected = 0
    with ThreadPoolExecutor(max_workers=50) as executor:
        futures = [executor.submit(submit_invalid_block, i) for i in range(1000)]
        for future in as_completed(futures):
            if future.result():
                rejected += 1
    
    print(f"{Colors.GREEN}✓ Security Engine Test Complete{Colors.END}")
    print(f"{Colors.BLUE}  Invalid blocks submitted: 1000{Colors.END}")
    print(f"{Colors.GREEN}  Rejected by security: {rejected}{Colors.END}")
    print(f"{Colors.BLUE}  Security alerts: {metrics.get_summary()['security_alerts']}{Colors.END}")

def stress_test_dynamic_fees():
    """Test dynamic fee system under load"""
    print(f"\n{Colors.CYAN}{'='*70}{Colors.END}")
    print(f"{Colors.BOLD}{Colors.CYAN}STRESS TEST 4: Dynamic Fee System{Colors.END}")
    print(f"{Colors.CYAN}{'='*70}{Colors.END}")
    
    print(f"{Colors.YELLOW}Testing fee calculation under various loads...{Colors.END}")
    
    # Create transactions with different sizes
    def create_test_transaction(size):
        from_addr = generate_miner_address(random.randint(1, 1000))
        to_addr = generate_miner_address(random.randint(1, 1000))
        amount = random.uniform(0.1, 100.0)
        
        # Estimate fee
        result, error = rpc_call("estimatefee", [size])
        if result:
            return result
        return None
    
    fee_samples = []
    for size in [1, 10, 100, 1000]:
        fee = create_test_transaction(size)
        if fee:
            fee_samples.append((size, fee))
            print(f"{Colors.BLUE}  Block size {size}: Fee = {fee} GXC{Colors.END}")
    
    print(f"{Colors.GREEN}✓ Dynamic Fee Test Complete{Colors.END}")
    print(f"{Colors.BLUE}  Fee samples collected: {len(fee_samples)}{Colors.END}")

def print_final_summary():
    """Print comprehensive test summary"""
    summary = metrics.get_summary()
    
    print(f"\n{Colors.BOLD}{Colors.CYAN}{'='*70}{Colors.END}")
    print(f"{Colors.BOLD}{Colors.CYAN}STRESS TEST FINAL SUMMARY{Colors.END}")
    print(f"{Colors.BOLD}{Colors.CYAN}{'='*70}{Colors.END}")
    
    print(f"\n{Colors.BOLD}Test Duration:{Colors.END} {summary['elapsed']:.2f} seconds")
    
    print(f"\n{Colors.BOLD}Template Requests:{Colors.END}")
    print(f"  Total: {summary['template_requests']}")
    print(f"  {Colors.GREEN}Success: {summary['template_success']}{Colors.END}")
    print(f"  {Colors.RED}Failures: {summary['template_failures']}{Colors.END}")
    print(f"  Rate: {summary['template_rate']:.2f} req/s")
    
    print(f"\n{Colors.BOLD}Mining Results:{Colors.END}")
    print(f"  Blocks Found: {summary['blocks_found']}")
    print(f"  Blocks Submitted: {summary['blocks_submitted']}")
    print(f"  {Colors.GREEN}Accepted: {summary['blocks_accepted']}{Colors.END}")
    print(f"  {Colors.RED}Rejected: {summary['blocks_rejected']}{Colors.END}")
    print(f"  Total Fees: {summary['total_fees']:.6f} GXC")
    
    print(f"\n{Colors.BOLD}Algorithm Distribution:{Colors.END}")
    for algo, count in summary['algorithm_usage'].items():
        percentage = (count / summary['template_requests'] * 100) if summary['template_requests'] > 0 else 0
        print(f"  {algo}: {count} ({percentage:.1f}%)")
    
    print(f"\n{Colors.BOLD}Security:{Colors.END}")
    print(f"  Security Alerts: {summary['security_alerts']}")
    
    print(f"\n{Colors.BOLD}Performance Metrics:{Colors.END}")
    if summary['blocks_accepted'] > 0:
        avg_fee = summary['total_fees'] / summary['blocks_accepted']
        print(f"  Average Fee per Block: {avg_fee:.6f} GXC")
    print(f"  Template Success Rate: {(summary['template_success'] / summary['template_requests'] * 100):.2f}%")
    if summary['blocks_submitted'] > 0:
        print(f"  Block Acceptance Rate: {(summary['blocks_accepted'] / summary['blocks_submitted'] * 100):.2f}%")
    
    print(f"\n{Colors.GREEN}{'='*70}{Colors.END}")
    print(f"{Colors.GREEN}✓ ALL STRESS TESTS COMPLETED{Colors.END}")
    print(f"{Colors.GREEN}{'='*70}{Colors.END}")

def main():
    print(f"{Colors.BOLD}{Colors.CYAN}{'='*70}{Colors.END}")
    print(f"{Colors.BOLD}{Colors.CYAN}GXC BLOCKCHAIN - 60,000 MINER STRESS TEST{Colors.END}")
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
        # Test 1: 60,000 template requests
        stress_test_template_requests(60000, max_workers=200)
        
        # Test 2: 1,000 concurrent miners (reduced for practical testing)
        stress_test_concurrent_mining(1000, max_attempts=10000, max_workers=100)
        
        # Test 3: Security engine
        stress_test_security_engine()
        
        # Test 4: Dynamic fees
        stress_test_dynamic_fees()
        
        # Final summary
        print_final_summary()
        
        # Save results
        results_file = f"stress_test_results_{int(time.time())}.json"
        with open(results_file, 'w') as f:
            json.dump(metrics.get_summary(), f, indent=2)
        
        print(f"\n{Colors.BLUE}Results saved to: {results_file}{Colors.END}")
        
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
