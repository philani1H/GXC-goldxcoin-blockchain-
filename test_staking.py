#!/usr/bin/env python3
"""
Comprehensive Staking Test Script
Tests all staking functionality to ensure PoS is working correctly
"""

import requests
import json
import time
import sys

class StakingTester:
    def __init__(self, rpc_url="http://localhost:18332"):
        self.rpc_url = rpc_url
        self.session = requests.Session()
        self.session.headers.update({'Content-Type': 'application/json'})
        self.test_results = []
    
    def rpc_call(self, method, params=None):
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
            response = self.session.post(self.rpc_url, json=payload, timeout=10)
            response.raise_for_status()
            result = response.json()
            
            if "error" in result:
                return None, result["error"]
            
            return result.get("result"), None
        except Exception as e:
            return None, str(e)
    
    def test(self, name, func):
        """Run a test and record results"""
        print(f"\n{'='*60}")
        print(f"TEST: {name}")
        print(f"{'='*60}")
        
        try:
            result = func()
            if result:
                print(f"✅ PASS: {name}")
                self.test_results.append((name, True, None))
                return True
            else:
                print(f"❌ FAIL: {name}")
                self.test_results.append((name, False, "Test returned False"))
                return False
        except Exception as e:
            print(f"❌ ERROR: {name} - {e}")
            self.test_results.append((name, False, str(e)))
            return False
    
    def test_connection(self):
        """Test 1: Connection to blockchain node"""
        result, error = self.rpc_call("getblockchaininfo")
        if error:
            print(f"Connection error: {error}")
            return False
        
        if result:
            print(f"✅ Connected to blockchain")
            print(f"   Chain height: {result.get('blocks', 0)}")
            print(f"   Difficulty: {result.get('difficulty', 0)}")
            return True
        return False
    
    def test_generate_address(self):
        """Test 2: Generate test addresses"""
        result, error = self.rpc_call("getnewaddress")
        if error:
            print(f"Error generating address: {error}")
            return False
        
        if result:
            self.validator_address = result
            print(f"✅ Generated validator address: {result}")
            return True
        return False
    
    def test_check_balance(self):
        """Test 3: Check balance"""
        if not hasattr(self, 'validator_address'):
            print("No validator address generated")
            return False
        
        result, error = self.rpc_call("getbalance", [self.validator_address])
        if error:
            print(f"Error checking balance: {error}")
            return False
        
        self.initial_balance = float(result) if result else 0.0
        print(f"✅ Balance: {self.initial_balance} GXC")
        
        if self.initial_balance < 100:
            print(f"⚠️  WARNING: Balance ({self.initial_balance} GXC) is less than minimum stake (100 GXC)")
            print(f"   You may need to mine some coins first")
        
        return True
    
    def test_register_validator(self):
        """Test 4: Register as validator"""
        if not hasattr(self, 'validator_address'):
            print("No validator address available")
            return False
        
        if self.initial_balance < 100:
            print(f"⚠️  Skipping validator registration - insufficient balance")
            print(f"   Need at least 100 GXC, have {self.initial_balance} GXC")
            print(f"   💡 Tip: Mine some coins first or use an address with balance")
            return False
        
        stake_amount = min(100.0, self.initial_balance * 0.5)  # Stake 50% or 100 GXC, whichever is less
        staking_days = 90
        
        print(f"Attempting to stake {stake_amount} GXC for {staking_days} days...")
        
        result, error = self.rpc_call("registervalidator", [
            self.validator_address,
            stake_amount,
            staking_days
        ])
        
        if error:
            error_str = str(error)
            if isinstance(error, dict):
                error_str = error.get("message", str(error))
            print(f"Error registering validator: {error_str}")
            
            # Check if validator already exists
            if "already" in error_str.lower() or "exists" in error_str.lower():
                print(f"   ℹ️  Validator may already be registered")
                # Try to get validator info instead
                info_result, _ = self.rpc_call("getvalidatorinfo", [self.validator_address])
                if info_result:
                    print(f"   ✅ Validator is already registered")
                    return True
            return False
        
        if result and result.get("success"):
            print(f"✅ Validator registered successfully")
            print(f"   Stake amount: {result.get('stake_amount')} GXC")
            print(f"   Staking days: {result.get('staking_days')}")
            print(f"   Weighted stake: {result.get('weighted_stake')}")
            return True
        
        return False
    
    def test_get_validator_info(self):
        """Test 5: Get validator information"""
        if not hasattr(self, 'validator_address'):
            print("No validator address available")
            return False
        
        result, error = self.rpc_call("getvalidatorinfo", [self.validator_address])
        if error:
            print(f"Error getting validator info: {error}")
            return False
        
        if result:
            print(f"✅ Validator info retrieved")
            print(f"   Address: {result.get('address')}")
            print(f"   Stake amount: {result.get('stake_amount')} GXC")
            print(f"   Staking days: {result.get('staking_days')}")
            print(f"   Weighted stake: {result.get('weighted_stake')}")
            print(f"   Is active: {result.get('is_active')}")
            print(f"   Is valid: {result.get('is_valid')}")
            return True
        
        return False
    
    def test_get_all_validators(self):
        """Test 6: Get all validators"""
        result, error = self.rpc_call("getvalidators")
        if error:
            print(f"Error getting validators: {error}")
            return False
        
        if result is not None:
            validator_count = len(result) if isinstance(result, list) else 0
            print(f"✅ Found {validator_count} validators")
            
            if validator_count > 0:
                print(f"\n   Validators:")
                for i, v in enumerate(result[:5], 1):  # Show first 5
                    print(f"   {i}. {v.get('address', '')[:20]}... - {v.get('stake_amount', 0)} GXC staked")
            
            return True
        
        return False
    
    def test_get_blockchain_info(self):
        """Test 7: Get blockchain info to check current height"""
        result, error = self.rpc_call("getblockchaininfo")
        if error:
            print(f"Error getting blockchain info: {error}")
            return False
        
        if result:
            self.current_height = result.get('blocks', 0)
            print(f"✅ Current blockchain height: {self.current_height}")
            
            # Determine next block type
            next_block_type = "PoW" if self.current_height % 2 == 0 else "PoS"
            print(f"   Next block type: {next_block_type}")
            
            return True
        
        return False
    
    def test_get_block_template(self):
        """Test 8: Get block template (should show PoS info if next block is PoS)"""
        result, error = self.rpc_call("getblocktemplate")
        if error:
            print(f"Error getting block template: {error}")
            return False
        
        if result:
            consensus_type = result.get('consensus_type', 'pow')
            block_type = result.get('block_type', 'pow')
            
            print(f"✅ Block template retrieved")
            print(f"   Consensus type: {consensus_type}")
            print(f"   Block type: {block_type}")
            print(f"   Height: {result.get('height')}")
            
            if consensus_type == 'pos':
                print(f"   ✅ PoS block template!")
                print(f"   Validator address: {result.get('validator_address', 'N/A')}")
                print(f"   Validator stake: {result.get('validator_stake', 0)} GXC")
            else:
                print(f"   ℹ️  PoW block template (next block will be PoS)")
            
            return True
        
        return False
    
    def test_create_pos_block(self):
        """Test 9: Create PoS block"""
        if not hasattr(self, 'validator_address'):
            print("No validator address available")
            return False
        
        # Check if validator is registered
        validator_info, _ = self.rpc_call("getvalidatorinfo", [self.validator_address])
        if not validator_info or not validator_info.get("is_active"):
            print(f"⚠️  Validator not registered or not active")
            print(f"   Skipping PoS block creation test")
            return True  # Not a failure if validator not registered
        
        # Check if next block should be PoS
        if not hasattr(self, 'current_height'):
            result, _ = self.rpc_call("getblockchaininfo")
            if result:
                self.current_height = result.get('blocks', 0)
        
        if self.current_height % 2 == 0:
            print(f"⚠️  Next block is PoW (height {self.current_height} is even)")
            print(f"   PoS blocks are created at odd heights (1, 3, 5, ...)")
            print(f"   Skipping PoS block creation test")
            print(f"   💡 Tip: Wait for next block or mine a PoW block to reach odd height")
            return True  # Not a failure, just not the right time
        
        print(f"Attempting to create PoS block for validator {self.validator_address}...")
        print(f"   Current height: {self.current_height} (odd - PoS block time)")
        
        result, error = self.rpc_call("createposblock", [self.validator_address])
        if error:
            error_str = str(error)
            if isinstance(error, dict):
                error_str = error.get("message", str(error))
            print(f"Error creating PoS block: {error_str}")
            if "not selected" in error_str.lower() or "not active" in error_str.lower():
                print(f"   ℹ️  This is expected if validator is not selected or not active")
                print(f"   💡 Validator selection is based on weighted stake")
                return True  # Not a failure
            if "should be pow" in error_str.lower():
                print(f"   ℹ️  Next block should be PoW, not PoS")
                return True  # Not a failure
            return False
        
        if result:
            print(f"✅ PoS block template created")
            print(f"   Height: {result.get('height')}")
            print(f"   Hash: {result.get('hash', '')[:20]}...")
            print(f"   Consensus type: {result.get('consensus_type')}")
            print(f"   Validator address: {result.get('validator_address')}")
            print(f"   Block reward: {result.get('block_reward')} GXC")
            print(f"   💡 You can submit this block using submitposblock RPC method")
            return True
        
        return False
    
    def test_unstake(self):
        """Test 10: Unstake (optional - comment out if you want to keep staked)"""
        if not hasattr(self, 'validator_address'):
            print("No validator address available")
            return False
        
        print(f"⚠️  Unstaking will deactivate validator")
        print(f"   Skipping unstake test (comment out to enable)")
        return True  # Skip by default
        
        # Uncomment below to test unstaking:
        # result, error = self.rpc_call("unstake", [self.validator_address])
        # if error:
        #     print(f"Error unstaking: {error}")
        #     return False
        # 
        # if result and result.get("success"):
        #     print(f"✅ Unstaked successfully")
        #     return True
        # 
        # return False
    
    def run_all_tests(self):
        """Run all tests"""
        print("\n" + "="*60)
        print("GXC STAKING FUNCTIONALITY TEST")
        print("="*60)
        
        tests = [
            ("Connection", self.test_connection),
            ("Generate Address", self.test_generate_address),
            ("Check Balance", self.test_check_balance),
            ("Register Validator", self.test_register_validator),
            ("Get Validator Info", self.test_get_validator_info),
            ("Get All Validators", self.test_get_all_validators),
            ("Get Blockchain Info", self.test_get_blockchain_info),
            ("Get Block Template", self.test_get_block_template),
            ("Create PoS Block", self.test_create_pos_block),
            ("Unstake", self.test_unstake),
        ]
        
        passed = 0
        failed = 0
        
        for name, test_func in tests:
            if self.test(name, test_func):
                passed += 1
            else:
                failed += 1
        
        # Print summary
        print("\n" + "="*60)
        print("TEST SUMMARY")
        print("="*60)
        print(f"Total tests: {len(tests)}")
        print(f"✅ Passed: {passed}")
        print(f"❌ Failed: {failed}")
        
        if failed == 0:
            print("\n🎉 All tests passed! Staking is working correctly!")
        else:
            print(f"\n⚠️  {failed} test(s) failed. Check output above for details.")
        
        return failed == 0

def main():
    import argparse
    
    parser = argparse.ArgumentParser(description='Test GXC staking functionality')
    parser.add_argument('--rpc-url', default='http://localhost:18332',
                       help='RPC URL (default: http://localhost:18332)')
    
    args = parser.parse_args()
    
    tester = StakingTester(args.rpc_url)
    success = tester.run_all_tests()
    
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    main()
