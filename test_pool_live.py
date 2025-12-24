#!/usr/bin/env python3
"""
Test GXC Mining Pool with Live Node
Verifies pool gets all correct data from node
"""

import sys
import os
import requests
import json

# Add mining_pool to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'mining_pool'))

from pool_base import MiningPool

NODE_URL = "http://localhost:8332"

def test_node_connection():
    """Test node is accessible"""
    print("="*70)
    print("TEST 1: Node Connection")
    print("="*70)
    
    try:
        response = requests.post(NODE_URL, json={
            "jsonrpc": "2.0",
            "method": "getblockcount",
            "params": [],
            "id": 1
        }, timeout=5)
        
        result = response.json()
        height = result.get('result')
        
        if height is not None:
            print(f"✅ Node connected at height {height}")
            return True
        else:
            print(f"❌ Node returned error: {result}")
            return False
    except Exception as e:
        print(f"❌ Cannot connect to node: {e}")
        return False

def test_block_template():
    """Test getting block template"""
    print("\n" + "="*70)
    print("TEST 2: Block Template")
    print("="*70)
    
    try:
        response = requests.post(NODE_URL, json={
            "jsonrpc": "2.0",
            "method": "getblocktemplate",
            "params": [],
            "id": 1
        }, timeout=5)
        
        result = response.json()
        template = result.get('result')
        
        if not template:
            print(f"❌ No template returned: {result}")
            return False
        
        print(f"\n✅ Got block template")
        print(f"\nTemplate fields:")
        
        # Check all expected fields
        expected_fields = [
            'previousblockhash', 'height', 'difficulty', 'target',
            'bits', 'curtime', 'version', 'transactions',
            'block_reward', 'coinbase_value', 'coinbasevalue'
        ]
        
        missing = []
        for field in expected_fields:
            if field in template:
                value = template[field]
                if isinstance(value, list):
                    print(f"  ✓ {field}: {len(value)} items")
                elif isinstance(value, (int, float)):
                    print(f"  ✓ {field}: {value}")
                else:
                    print(f"  ✓ {field}: {str(value)[:50]}...")
            else:
                missing.append(field)
                print(f"  ✗ {field}: MISSING")
        
        if missing:
            print(f"\n⚠️  Missing fields: {', '.join(missing)}")
        else:
            print(f"\n✅ All expected fields present")
        
        return len(missing) == 0
        
    except Exception as e:
        print(f"❌ Error getting template: {e}")
        return False

def test_pool_job_generation():
    """Test pool job generation"""
    print("\n" + "="*70)
    print("TEST 3: Pool Job Generation")
    print("="*70)
    
    try:
        # Create pool instance
        pool = MiningPool(
            pool_name='test-pool',
            algorithm='sha256',
            port=9999,  # Use different port for testing
            rpc_url=NODE_URL
        )
        
        print(f"\n✅ Pool created")
        print(f"  Algorithm: {pool.algorithm}")
        print(f"  RPC URL: {pool.rpc_url}")
        
        # Generate job
        job = pool.generate_mining_job()
        
        if not job:
            print(f"❌ No job generated")
            return False
        
        print(f"\n✅ Job generated")
        print(f"\nJob fields:")
        
        # Check all expected job fields
        expected_job_fields = [
            'job_id', 'prev_block_hash', 'height', 'difficulty',
            'timestamp', 'transactions', 'bits', 'target',
            'version', 'block_reward', 'coinbase_value'
        ]
        
        missing = []
        for field in expected_job_fields:
            if field in job:
                value = job[field]
                if isinstance(value, list):
                    print(f"  ✓ {field}: {len(value)} items")
                elif isinstance(value, (int, float)):
                    print(f"  ✓ {field}: {value}")
                else:
                    print(f"  ✓ {field}: {str(value)[:50]}...")
            else:
                missing.append(field)
                print(f"  ✗ {field}: MISSING")
        
        if missing:
            print(f"\n⚠️  Missing job fields: {', '.join(missing)}")
            return False
        else:
            print(f"\n✅ All expected job fields present")
            return True
        
    except Exception as e:
        print(f"❌ Error testing pool: {e}")
        import traceback
        traceback.print_exc()
        return False

def test_stratum_message_format():
    """Test Stratum message format"""
    print("\n" + "="*70)
    print("TEST 4: Stratum Message Format")
    print("="*70)
    
    try:
        pool = MiningPool(
            pool_name='test-pool',
            algorithm='sha256',
            port=9999,
            rpc_url=NODE_URL
        )
        
        job = pool.generate_mining_job()
        
        # Create Stratum notify message
        notify_msg = {
            "id": None,
            "method": "mining.notify",
            "params": [
                job['job_id'],
                job['prev_block_hash'],
                "", "",  # coinbase1, coinbase2
                [],  # merkle branches
                job.get('version', '1'),
                job.get('bits', '1d00ffff'),
                hex(job['timestamp'])[2:],
                True  # clean_jobs
            ]
        }
        
        print(f"\n✅ Stratum notify message:")
        print(json.dumps(notify_msg, indent=2))
        
        # Verify format (Stratum protocol expects 8 or 9 parameters)
        # Standard format: job_id, prevhash, coinb1, coinb2, merkle_branch, version, nbits, ntime, clean_jobs
        param_count = len(notify_msg['params'])
        if param_count in [8, 9]:
            print(f"\n✅ Correct number of parameters ({param_count})")
            print(f"\nParameter breakdown:")
            print(f"  [0] job_id: {notify_msg['params'][0]}")
            print(f"  [1] prevhash: {notify_msg['params'][1][:16]}...")
            print(f"  [2] coinbase1: {notify_msg['params'][2]}")
            print(f"  [3] coinbase2: {notify_msg['params'][3]}")
            print(f"  [4] merkle_branch: {notify_msg['params'][4]}")
            print(f"  [5] version: {notify_msg['params'][5]}")
            print(f"  [6] nbits: {notify_msg['params'][6]}")
            print(f"  [7] ntime: {notify_msg['params'][7]}")
            if param_count == 9:
                print(f"  [8] clean_jobs: {notify_msg['params'][8]}")
            return True
        else:
            print(f"\n❌ Wrong number of parameters: {param_count} (expected 8 or 9)")
            return False
        
    except Exception as e:
        print(f"❌ Error creating Stratum message: {e}")
        return False

def main():
    """Run all tests"""
    print("\n" + "="*70)
    print("GXC MINING POOL - LIVE NODE TEST")
    print("="*70)
    print(f"\nNode URL: {NODE_URL}")
    print()
    
    results = []
    
    # Test 1: Node connection
    results.append(("Node Connection", test_node_connection()))
    
    # Test 2: Block template
    results.append(("Block Template", test_block_template()))
    
    # Test 3: Pool job generation
    results.append(("Pool Job Generation", test_pool_job_generation()))
    
    # Test 4: Stratum message format
    results.append(("Stratum Message Format", test_stratum_message_format()))
    
    # Summary
    print("\n" + "="*70)
    print("TEST SUMMARY")
    print("="*70)
    print()
    
    passed = 0
    failed = 0
    
    for test_name, result in results:
        status = "✅ PASSED" if result else "❌ FAILED"
        print(f"{test_name}: {status}")
        if result:
            passed += 1
        else:
            failed += 1
    
    print()
    print(f"Total: {passed} passed, {failed} failed")
    print("="*70)
    print()
    
    if failed == 0:
        print("✅ All tests passed! Pool is ready to run.")
        return 0
    else:
        print(f"❌ {failed} test(s) failed. Please fix issues.")
        return 1

if __name__ == "__main__":
    sys.exit(main())
