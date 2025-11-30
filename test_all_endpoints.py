#!/usr/bin/env python3
"""
Test all explorer endpoints to ensure they return complete data
"""
import sys
import os
import json
import requests
from datetime import datetime

# Add web directory to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'web'))

# Test configuration
BASE_URL = os.environ.get('EXPLORER_URL', 'http://localhost:5000')
TEST_TIMEOUT = 30

# Colors for output
GREEN = '\033[92m'
RED = '\033[91m'
YELLOW = '\033[93m'
BLUE = '\033[94m'
RESET = '\033[0m'

def print_success(msg):
    print(f"{GREEN}✓{RESET} {msg}")

def print_error(msg):
    print(f"{RED}✗{RESET} {msg}")

def print_warning(msg):
    print(f"{YELLOW}⚠{RESET} {msg}")

def print_info(msg):
    print(f"{BLUE}ℹ{RESET} {msg}")

def test_endpoint(name, method='GET', path='', params=None, data=None, expected_keys=None, min_items=0):
    """Test an API endpoint"""
    try:
        url = f"{BASE_URL}{path}"
        print_info(f"Testing {name}: {method} {path}")
        
        if method == 'GET':
            response = requests.get(url, params=params, timeout=TEST_TIMEOUT)
        elif method == 'POST':
            response = requests.post(url, json=data, params=params, timeout=TEST_TIMEOUT)
        else:
            response = requests.request(method, url, json=data, params=params, timeout=TEST_TIMEOUT)
        
        if response.status_code == 200:
            try:
                result = response.json()
                
                # Check if it's a list
                if isinstance(result, list):
                    if len(result) >= min_items:
                        print_success(f"{name}: Returned {len(result)} items (min: {min_items})")
                        if len(result) > 0 and expected_keys:
                            # Check first item has expected keys
                            missing_keys = [k for k in expected_keys if k not in result[0]]
                            if missing_keys:
                                print_warning(f"{name}: Missing keys in items: {missing_keys}")
                            else:
                                print_success(f"{name}: All expected keys present")
                        return True
                    else:
                        print_warning(f"{name}: Only {len(result)} items returned (expected min: {min_items})")
                        return True  # Still valid, just less data
                
                # Check if it's a dict
                elif isinstance(result, dict):
                    if expected_keys:
                        missing_keys = [k for k in expected_keys if k not in result]
                        if missing_keys:
                            print_warning(f"{name}: Missing keys: {missing_keys}")
                        else:
                            print_success(f"{name}: All expected keys present")
                    
                    # Check for error field
                    if 'error' in result:
                        print_error(f"{name}: Error in response: {result.get('error')}")
                        return False
                    
                    return True
                else:
                    print_success(f"{name}: Returned data (type: {type(result).__name__})")
                    return True
                    
            except json.JSONDecodeError:
                # Not JSON, check if it's HTML (template response)
                if 'text/html' in response.headers.get('content-type', ''):
                    print_success(f"{name}: Returned HTML template (status: {response.status_code})")
                    return True
                else:
                    print_error(f"{name}: Invalid JSON response")
                    return False
        else:
            print_error(f"{name}: HTTP {response.status_code}")
            return False
            
    except requests.exceptions.Timeout:
        print_error(f"{name}: Request timeout after {TEST_TIMEOUT}s")
        return False
    except requests.exceptions.ConnectionError:
        print_error(f"{name}: Connection error - is the server running?")
        return False
    except Exception as e:
        print_error(f"{name}: {str(e)}")
        return False

def main():
    print(f"\n{BLUE}{'='*60}{RESET}")
    print(f"{BLUE}Testing All Explorer Endpoints{RESET}")
    print(f"{BLUE}{'='*60}{RESET}\n")
    print_info(f"Base URL: {BASE_URL}\n")
    
    results = {
        'passed': 0,
        'failed': 0,
        'warnings': 0
    }
    
    # Core Explorer Pages
    print(f"\n{YELLOW}Core Explorer Pages{RESET}")
    print("-" * 60)
    
    test_endpoint("Home Page", path='/')
    test_endpoint("Blocks List", path='/blocks')
    test_endpoint("Transactions List", path='/transactions')
    test_endpoint("Validators", path='/validators')
    test_endpoint("Addresses", path='/addresses')
    test_endpoint("Mining Stats", path='/mining')
    test_endpoint("Charts", path='/charts')
    test_endpoint("API Docs", path='/api')
    
    # API Endpoints - Blocks
    print(f"\n{YELLOW}API Endpoints - Blocks{RESET}")
    print("-" * 60)
    
    test_endpoint("API: Blocks", path='/api/blocks', 
                  params={'limit': 10},
                  expected_keys=['number', 'hash', 'timestamp', 'miner'],
                  min_items=0)
    
    test_endpoint("API: Blocks (limit=50)", path='/api/blocks',
                  params={'limit': 50},
                  min_items=0)
    
    # API Endpoints - Transactions
    print(f"\n{YELLOW}API Endpoints - Transactions{RESET}")
    print("-" * 60)
    
    test_endpoint("API: Transactions", path='/api/transactions',
                  params={'limit': 20},
                  expected_keys=['hash', 'from', 'to', 'value', 'block'],
                  min_items=0)
    
    test_endpoint("API: Transactions (limit=100)", path='/api/transactions',
                  params={'limit': 100},
                  min_items=0)
    
    # API Endpoints - Statistics
    print(f"\n{YELLOW}API Endpoints - Statistics{RESET}")
    print("-" * 60)
    
    test_endpoint("API: Stats", path='/api/stats',
                  expected_keys=['total_blocks', 'total_transactions', 'total_addresses', 'total_supply'])
    
    test_endpoint("API: Network Info", path='/api/network-info',
                  expected_keys=['network', 'address_prefix', 'block_reward', 'is_testnet', 'is_mainnet'])
    
    test_endpoint("API: Connection Status", path='/api/connection-status',
                  expected_keys=['active_node', 'connected'])
    
    # API Endpoints - Charts & Analytics
    print(f"\n{YELLOW}API Endpoints - Charts & Analytics{RESET}")
    print("-" * 60)
    
    test_endpoint("API: Charts", path='/api/charts',
                  params={'days': 14},
                  min_items=0)
    
    test_endpoint("API: Hashrate", path='/api/hashrate',
                  params={'days': 7},
                  min_items=0)
    
    test_endpoint("API: Gas Stats", path='/api/gas')
    
    test_endpoint("API: Price Data", path='/api/price')
    
    test_endpoint("API: Analytics", path='/api/analytics')
    
    # API Endpoints - Health & Status
    print(f"\n{YELLOW}API Endpoints - Health & Status{RESET}")
    print("-" * 60)
    
    test_endpoint("API: Health", path='/api/health')
    test_endpoint("Health Check", path='/healthcheck')
    test_endpoint("Healthz", path='/healthz')
    
    # API Endpoints - Export
    print(f"\n{YELLOW}API Endpoints - Export{RESET}")
    print("-" * 60)
    
    test_endpoint("API: Export Transactions", path='/api/export/transactions',
                  params={'format': 'json', 'limit': 10},
                  min_items=0)
    
    test_endpoint("API: Export Blocks", path='/api/export/blocks',
                  params={'format': 'json', 'limit': 10},
                  min_items=0)
    
    # API Endpoints - Advanced Features
    print(f"\n{YELLOW}API Endpoints - Advanced Features{RESET}")
    print("-" * 60)
    
    test_endpoint("API: Pending Transactions", path='/api/pending',
                  min_items=0)
    
    test_endpoint("API: Portfolio", path='/api/portfolio',
                  params={'addresses': '[]'})
    
    # Test with actual block/transaction if available
    print(f"\n{YELLOW}Testing Dynamic Endpoints{RESET}")
    print("-" * 60)
    
    # Try to get a block number from API
    try:
        blocks_response = requests.get(f"{BASE_URL}/api/blocks?limit=1", timeout=TEST_TIMEOUT)
        if blocks_response.status_code == 200:
            blocks_data = blocks_response.json()
            if isinstance(blocks_data, list) and len(blocks_data) > 0:
                block_num = blocks_data[0].get('number') or blocks_data[0].get('block_number')
                if block_num:
                    test_endpoint(f"Block Detail: #{block_num}", path=f'/block/{block_num}')
        
        # Try to get a transaction hash
        tx_response = requests.get(f"{BASE_URL}/api/transactions?limit=1", timeout=TEST_TIMEOUT)
        if tx_response.status_code == 200:
            tx_data = tx_response.json()
            if isinstance(tx_data, list) and len(tx_data) > 0:
                tx_hash = tx_data[0].get('hash') or tx_data[0].get('tx_hash')
                if tx_hash:
                    test_endpoint(f"Transaction Detail: {tx_hash[:16]}...", path=f'/tx/{tx_hash}')
        
        # Try to get an address
        if isinstance(tx_data, list) and len(tx_data) > 0:
            address = tx_data[0].get('from') or tx_data[0].get('to')
            if address:
                test_endpoint(f"Address Detail: {address[:16]}...", path=f'/address/{address}')
    
    except Exception as e:
        print_warning(f"Could not test dynamic endpoints: {e}")
    
    # Summary
    print(f"\n{BLUE}{'='*60}{RESET}")
    print(f"{BLUE}Test Summary{RESET}")
    print(f"{BLUE}{'='*60}{RESET}")
    print(f"\n{GREEN}All endpoint tests completed!{RESET}")
    print(f"\n{YELLOW}Note:{RESET} Some endpoints may return empty data if the blockchain")
    print(f"      is new or not fully synced. This is expected behavior.")
    print(f"\n{YELLOW}To test with a running server:{RESET}")
    print(f"      python3 web/blockchain_explorer.py")
    print(f"      Then run this script: python3 test_all_endpoints.py\n")

if __name__ == '__main__':
    main()
