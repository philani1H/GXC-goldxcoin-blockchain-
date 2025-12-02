#!/usr/bin/env python3
"""
Staking Implementation Verification Script
Verifies that all staking code is properly implemented without requiring a running node
"""

import os
import re
import sys

class StakingVerifier:
    def __init__(self):
        self.issues = []
        self.checks_passed = 0
        self.checks_failed = 0
    
    def check_file_exists(self, filepath, description):
        """Check if a file exists"""
        if os.path.exists(filepath):
            print(f"✅ {description}: {filepath}")
            self.checks_passed += 1
            return True
        else:
            print(f"❌ {description}: {filepath} - NOT FOUND")
            self.issues.append(f"Missing file: {filepath}")
            self.checks_failed += 1
            return False
    
    def check_code_pattern(self, filepath, pattern, description, required=True):
        """Check if code pattern exists in file"""
        if not os.path.exists(filepath):
            if required:
                print(f"❌ {description}: File not found - {filepath}")
                self.issues.append(f"Missing file: {filepath}")
                self.checks_failed += 1
            return False
        
        try:
            with open(filepath, 'r') as f:
                content = f.read()
            
            if re.search(pattern, content, re.IGNORECASE | re.MULTILINE):
                print(f"✅ {description}")
                self.checks_passed += 1
                return True
            else:
                if required:
                    print(f"❌ {description}: Pattern not found in {filepath}")
                    self.issues.append(f"Missing pattern in {filepath}: {description}")
                    self.checks_failed += 1
                else:
                    print(f"⚠️  {description}: Optional pattern not found")
                return False
        except Exception as e:
            print(f"❌ Error reading {filepath}: {e}")
            self.issues.append(f"Error reading {filepath}: {e}")
            self.checks_failed += 1
            return False
    
    def verify_blockchain_pos(self):
        """Verify PoS implementation in Blockchain.cpp"""
        print("\n" + "="*60)
        print("Verifying Blockchain PoS Implementation")
        print("="*60)
        
        blockchain_cpp = "src/Blockchain.cpp"
        
        checks = [
            ("validateProofOfStake function", r"bool\s+Blockchain::validateProofOfStake"),
            ("selectValidatorForBlock function", r"Validator\s+Blockchain::selectValidatorForBlock"),
            ("getNextBlockType function", r"BlockType\s+Blockchain::getNextBlockType"),
            ("PoS block type check", r"BlockType::POS"),
            ("Validator selection logic", r"selectValidatorForBlock"),
            ("Weighted stake calculation", r"getWeightedStake"),
            ("Hybrid consensus (alternating blocks)", r"currentHeight\s*%\s*2"),
            ("PoS difficulty", r"posDifficulty|0\.01"),
            ("Validator signature verification", r"verifySignature"),
            ("Random number generator for selection", r"std::mt19937|mt19937"),
        ]
        
        for desc, pattern in checks:
            self.check_code_pattern(blockchain_cpp, pattern, desc)
    
    def verify_rpc_staking(self):
        """Verify staking RPC methods in RPCAPI.cpp"""
        print("\n" + "="*60)
        print("Verifying Staking RPC Methods")
        print("="*60)
        
        rpcapi_cpp = "src/RPCAPI.cpp"
        
        checks = [
            ("registerValidator RPC method", r"JsonValue\s+RPCAPI::registerValidator"),
            ("stake RPC method (alias)", r"JsonValue\s+RPCAPI::stake"),
            ("unstake RPC method", r"JsonValue\s+RPCAPI::unstake"),
            ("addStake RPC method", r"JsonValue\s+RPCAPI::addStake"),
            ("getValidators RPC method", r"JsonValue\s+RPCAPI::getValidators"),
            ("getValidatorInfo RPC method", r"JsonValue\s+RPCAPI::getValidatorInfo"),
            ("createPoSBlock RPC method", r"JsonValue\s+RPCAPI::createPoSBlock"),
            ("submitPoSBlock RPC method", r"JsonValue\s+RPCAPI::submitPoSBlock"),
            ("RPC method registration", r"rpcMethods\[\"registervalidator\"\]"),
            ("RPC method registration (stake)", r"rpcMethods\[\"stake\"\]"),
            ("RPC method registration (getvalidators)", r"rpcMethods\[\"getvalidators\"\]"),
            ("RPC method registration (createposblock)", r"rpcMethods\[\"createposblock\"\]"),
            ("Minimum stake validation", r"MIN_STAKE|100\.0"),
            ("Balance check before staking", r"balance\s*<\s*stakeAmount|getBalance.*address"),
            ("Validator selection in createPoSBlock", r"selectValidatorForBlock"),
        ]
        
        for desc, pattern in checks:
            self.check_code_pattern(rpcapi_cpp, pattern, desc)
    
    def verify_validator_class(self):
        """Verify Validator class implementation"""
        print("\n" + "="*60)
        print("Verifying Validator Class")
        print("="*60)
        
        validator_h = "include/Validator.h"
        validator_cpp = "src/Validator.cpp"
        
        # Check header file
        checks_h = [
            ("Validator class definition", r"class\s+Validator"),
            ("getWeightedStake method", r"getWeightedStake"),
            ("isValidValidator method", r"isValidValidator"),
            ("signBlock method", r"signBlock"),
            ("verifySignature method", r"verifySignature"),
            ("MIN_STAKE constant", r"MIN_STAKE"),
            ("MIN_STAKING_DAYS constant", r"MIN_STAKING_DAYS"),
            ("MAX_STAKING_DAYS constant", r"MAX_STAKING_DAYS"),
        ]
        
        for desc, pattern in checks_h:
            self.check_code_pattern(validator_h, pattern, desc)
        
        # Check implementation file if it exists
        if os.path.exists(validator_cpp):
            checks_cpp = [
                ("Weighted stake calculation", r"getWeightedStake"),
                ("Validator validation logic", r"isValidValidator"),
            ]
            for desc, pattern in checks_cpp:
                self.check_code_pattern(validator_cpp, pattern, desc, required=False)
    
    def verify_block_pos_support(self):
        """Verify Block class supports PoS"""
        print("\n" + "="*60)
        print("Verifying Block PoS Support")
        print("="*60)
        
        block_h = "include/Block.h"
        block_cpp = "src/block.cpp"
        
        checks = [
            ("BlockType enum with POS", r"BlockType.*POS|POS.*BlockType"),
            ("getBlockType method", r"getBlockType"),
            ("setBlockType method", r"setBlockType"),
            ("getValidatorSignature method", r"getValidatorSignature"),
            ("setValidatorSignature method", r"setValidatorSignature"),
        ]
        
        for desc, pattern in checks:
            # Check both header and implementation
            found_h = self.check_code_pattern(block_h, pattern, f"{desc} (header)", required=False)
            found_cpp = self.check_code_pattern(block_cpp, pattern, f"{desc} (implementation)", required=False)
            if not found_h and not found_cpp:
                print(f"⚠️  {desc}: Not found in header or implementation")
    
    def verify_headers(self):
        """Verify header files include necessary declarations"""
        print("\n" + "="*60)
        print("Verifying Header Files")
        print("="*60)
        
        blockchain_h = "include/blockchain.h"
        
        checks = [
            ("validateProofOfStake declaration", r"validateProofOfStake"),
            ("selectValidatorForBlock declaration", r"selectValidatorForBlock"),
            ("getNextBlockType declaration", r"getNextBlockType"),
            ("Validator include", r"#include.*Validator\.h|Validator"),
        ]
        
        for desc, pattern in checks:
            self.check_code_pattern(blockchain_h, pattern, desc)
    
    def verify_test_files(self):
        """Verify test files exist"""
        print("\n" + "="*60)
        print("Verifying Test Files")
        print("="*60)
        
        test_files = [
            ("test_staking.py", "Python staking test script"),
            ("test_staking_quick.sh", "Quick bash staking test"),
            ("STAKING_TEST_GUIDE.md", "Staking test documentation"),
        ]
        
        for filepath, description in test_files:
            self.check_file_exists(filepath, description)
    
    def run_all_checks(self):
        """Run all verification checks"""
        print("\n" + "="*60)
        print("STAKING IMPLEMENTATION VERIFICATION")
        print("="*60)
        
        self.verify_blockchain_pos()
        self.verify_rpc_staking()
        self.verify_validator_class()
        self.verify_block_pos_support()
        self.verify_headers()
        self.verify_test_files()
        
        # Print summary
        print("\n" + "="*60)
        print("VERIFICATION SUMMARY")
        print("="*60)
        print(f"✅ Checks passed: {self.checks_passed}")
        print(f"❌ Checks failed: {self.checks_failed}")
        print(f"Total checks: {self.checks_passed + self.checks_failed}")
        
        if self.issues:
            print(f"\n⚠️  Issues found ({len(self.issues)}):")
            for issue in self.issues[:10]:  # Show first 10
                print(f"   - {issue}")
            if len(self.issues) > 10:
                print(f"   ... and {len(self.issues) - 10} more")
        else:
            print("\n🎉 All checks passed! Staking implementation looks good!")
        
        return len(self.issues) == 0

def main():
    verifier = StakingVerifier()
    success = verifier.run_all_checks()
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    main()
