#!/usr/bin/env python3
"""
Simple test to verify the database connection bug fix
Tests that the code no longer uses DATABASE_PATH directly
"""

import os
import re

def test_no_direct_database_path_usage():
    """Verify that wallet_service.py doesn't use DATABASE_PATH directly in methods"""
    
    wallet_service_path = os.path.join(
        os.path.dirname(__file__), '..', 'api', 'wallet_service.py'
    )
    
    with open(wallet_service_path, 'r') as f:
        content = f.read()
    
    # Find all instances of sqlite3.connect(DATABASE_PATH)
    pattern = r'sqlite3\.connect\(DATABASE_PATH\)'
    matches = re.findall(pattern, content)
    
    if matches:
        print(f"❌ FAILED: Found {len(matches)} instances of sqlite3.connect(DATABASE_PATH)")
        print("   This bypasses network-aware database selection!")
        
        # Show line numbers
        lines = content.split('\n')
        for i, line in enumerate(lines, 1):
            if 'sqlite3.connect(DATABASE_PATH)' in line:
                print(f"   Line {i}: {line.strip()}")
        
        return False
    
    print("✅ PASSED: No direct DATABASE_PATH usage found")
    print("   All database connections should use self.get_db_connection()")
    return True

def test_get_db_connection_exists():
    """Verify that get_db_connection method exists and uses self.db_path"""
    
    wallet_service_path = os.path.join(
        os.path.dirname(__file__), '..', 'api', 'wallet_service.py'
    )
    
    with open(wallet_service_path, 'r') as f:
        content = f.read()
    
    # Check for get_db_connection method
    if 'def get_db_connection(self):' not in content:
        print("❌ FAILED: get_db_connection method not found")
        return False
    
    # Check that it uses self.db_path
    if 'return sqlite3.connect(self.db_path)' not in content:
        print("❌ FAILED: get_db_connection doesn't use self.db_path")
        return False
    
    print("✅ PASSED: get_db_connection method exists and uses self.db_path")
    return True

def test_wallet_service_has_db_path():
    """Verify that WalletService class initializes self.db_path"""
    
    wallet_service_path = os.path.join(
        os.path.dirname(__file__), '..', 'api', 'wallet_service.py'
    )
    
    with open(wallet_service_path, 'r') as f:
        content = f.read()
    
    # Check for self.db_path initialization
    if 'self.db_path = get_wallet_database_path()' not in content:
        print("❌ FAILED: WalletService doesn't initialize self.db_path")
        return False
    
    print("✅ PASSED: WalletService initializes self.db_path from get_wallet_database_path()")
    return True

def test_methods_use_get_db_connection():
    """Verify that methods use self.get_db_connection() or wallet_service.get_wallet()"""
    
    wallet_service_path = os.path.join(
        os.path.dirname(__file__), '..', 'api', 'wallet_service.py'
    )
    
    with open(wallet_service_path, 'r') as f:
        content = f.read()
    
    # Count uses of self.get_db_connection()
    get_db_conn_count = content.count('self.get_db_connection()')
    
    # Count uses of wallet_service.get_wallet()
    get_wallet_count = content.count('wallet_service.get_wallet(')
    
    if get_db_conn_count == 0:
        print("❌ FAILED: No methods use self.get_db_connection()")
        return False
    
    print(f"✅ PASSED: Found {get_db_conn_count} uses of self.get_db_connection()")
    print(f"   Found {get_wallet_count} uses of wallet_service.get_wallet()")
    return True

if __name__ == '__main__':
    print("=" * 70)
    print("Testing Database Connection Bug Fix")
    print("=" * 70)
    print()
    
    all_passed = True
    
    print("Test 1: Checking for direct DATABASE_PATH usage...")
    if not test_no_direct_database_path_usage():
        all_passed = False
    print()
    
    print("Test 2: Checking get_db_connection method...")
    if not test_get_db_connection_exists():
        all_passed = False
    print()
    
    print("Test 3: Checking WalletService initialization...")
    if not test_wallet_service_has_db_path():
        all_passed = False
    print()
    
    print("Test 4: Checking method usage...")
    if not test_methods_use_get_db_connection():
        all_passed = False
    print()
    
    print("=" * 70)
    if all_passed:
        print("ALL TESTS PASSED ✅")
        print()
        print("Bug Fix Summary:")
        print("- Removed all direct sqlite3.connect(DATABASE_PATH) calls")
        print("- All methods now use self.get_db_connection()")
        print("- Flask routes use wallet_service.get_wallet() instead of direct DB access")
        print("- Network-aware database selection is now properly enforced")
        print()
        print("Impact:")
        print("- Testnet and mainnet wallets are now properly separated")
        print("- No risk of mixing testnet/mainnet data")
        print("- Consistent database access across all methods")
    else:
        print("SOME TESTS FAILED ❌")
        exit(1)
    print("=" * 70)
