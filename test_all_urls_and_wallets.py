#!/usr/bin/env python3
"""
Test all URLs and wallet endpoints to verify they return data
"""

import os
import sys
import json
import requests
from datetime import datetime

# Configuration
BASE_URL = os.environ.get('EXPLORER_URL', 'http://localhost:5000')
API_BASE = f"{BASE_URL}/api"
WALLET_API_BASE = os.environ.get('WALLET_API_URL', 'http://localhost:5001')

# Test results
results = {
    'passed': [],
    'failed': [],
    'warnings': []
}

def test_endpoint(name, url, method='GET', data=None, expected_keys=None, min_items=None, timeout=10):
    """Test an endpoint and verify response"""
    try:
        if method == 'GET':
            response = requests.get(url, timeout=timeout)
        elif method == 'POST':
            response = requests.post(url, json=data, timeout=timeout)
        else:
            results['failed'].append(f"{name}: Invalid method {method}")
            return False
        
        if response.status_code == 200:
            try:
                if response.headers.get('content-type', '').startswith('application/json'):
                    json_data = response.json()
                    
                    # Check for expected keys
                    if expected_keys:
                        missing_keys = [key for key in expected_keys if key not in json_data]
                        if missing_keys:
                            results['warnings'].append(f"{name}: Missing keys: {missing_keys}")
                    
                    # Check for minimum items in lists
                    if min_items and isinstance(json_data, dict):
                        for key, min_count in min_items.items():
                            if key in json_data and isinstance(json_data[key], list):
                                if len(json_data[key]) < min_count:
                                    results['warnings'].append(
                                        f"{name}: {key} has {len(json_data[key])} items, expected at least {min_count}"
                                    )
                    
                    results['passed'].append(f"{name}: ✅ Status 200, JSON response")
                    return True
                else:
                    # HTML response
                    if len(response.text) > 100:  # Has content
                        results['passed'].append(f"{name}: ✅ Status 200, HTML response ({len(response.text)} bytes)")
                        return True
                    else:
                        results['warnings'].append(f"{name}: ⚠️ Status 200 but small response ({len(response.text)} bytes)")
                        return True
            except json.JSONDecodeError:
                # Not JSON, check if it's HTML
                if len(response.text) > 100:
                    results['passed'].append(f"{name}: ✅ Status 200, HTML response ({len(response.text)} bytes)")
                    return True
                else:
                    results['warnings'].append(f"{name}: ⚠️ Status 200 but small response")
                    return True
        else:
            results['failed'].append(f"{name}: ❌ Status {response.status_code}")
            return False
    except requests.exceptions.ConnectionError:
        results['failed'].append(f"{name}: ❌ Connection refused (server not running?)")
        return False
    except requests.exceptions.Timeout:
        results['failed'].append(f"{name}: ❌ Timeout after {timeout}s")
        return False
    except Exception as e:
        results['failed'].append(f"{name}: ❌ Error: {str(e)}")
        return False

def test_explorer_endpoints():
    """Test all explorer endpoints"""
    print("\n" + "="*60)
    print("TESTING EXPLORER ENDPOINTS")
    print("="*60)
    
    # Main pages
    endpoints = [
        ("Home Page", f"{BASE_URL}/"),
        ("Blocks List", f"{BASE_URL}/blocks"),
        ("Transactions List", f"{BASE_URL}/transactions"),
        ("Validators", f"{BASE_URL}/validators"),
        ("Addresses", f"{BASE_URL}/addresses"),
        ("Mining Page", f"{BASE_URL}/mining"),
        ("Charts", f"{BASE_URL}/charts"),
        ("Analytics", f"{BASE_URL}/analytics"),
        ("Pending", f"{BASE_URL}/pending"),
        ("Gas Tracker", f"{BASE_URL}/gas"),
        ("Price", f"{BASE_URL}/price"),
        ("Portfolio", f"{BASE_URL}/portfolio"),
        ("Health Check", f"{BASE_URL}/health"),
        ("Healthz", f"{BASE_URL}/healthz"),
        ("Healthcheck", f"{BASE_URL}/healthcheck"),
    ]
    
    for name, url in endpoints:
        print(f"Testing {name}...")
        test_endpoint(name, url, timeout=15)
    
    # API endpoints
    print("\n" + "-"*60)
    print("TESTING API ENDPOINTS")
    print("-"*60)
    
    api_endpoints = [
        ("API Blocks", f"{API_BASE}/blocks", {'blocks': 1}),
        ("API Transactions", f"{API_BASE}/transactions", {'transactions': 1}),
        ("API Stats", f"{API_BASE}/stats", ['total_blocks', 'total_transactions']),
        ("API Charts", f"{API_BASE}/charts", None),
        ("API Hashrate", f"{API_BASE}/hashrate", None),
        ("API Gas", f"{API_BASE}/gas", None),
        ("API Price", f"{API_BASE}/price", None),
        ("API Portfolio", f"{API_BASE}/portfolio", None),
        ("API Health", f"{API_BASE}/health", None),
        ("API Connection Status", f"{API_BASE}/connection-status", None),
        ("API Network Info", f"{API_BASE}/network-info", ['network', 'address_prefix']),
        ("API Analytics", f"{API_BASE}/analytics", None),
        ("API Pending", f"{API_BASE}/pending", None),
    ]
    
    for name, url, expected in api_endpoints:
        print(f"Testing {name}...")
        if isinstance(expected, dict):
            test_endpoint(name, url, expected_keys=None, min_items=expected, timeout=15)
        else:
            test_endpoint(name, url, expected_keys=expected, timeout=15)

def test_wallet_endpoints():
    """Test wallet-related endpoints"""
    print("\n" + "="*60)
    print("TESTING WALLET ENDPOINTS")
    print("="*60)
    
    # Explorer wallet page
    print("Testing Explorer Wallet Page...")
    test_endpoint("Explorer Wallet Page", f"{BASE_URL}/wallet", timeout=15)
    
    # Wallet API endpoints (if wallet service is running)
    wallet_endpoints = [
        ("Wallet Service Health", f"{WALLET_API_BASE}/health"),
        ("Wallet Service Status", f"{WALLET_API_BASE}/status"),
        ("Create Wallet", f"{WALLET_API_BASE}/wallet/create", 'POST', {}),
        ("List Wallets", f"{WALLET_API_BASE}/wallet/list"),
        ("Wallet Balance", f"{WALLET_API_BASE}/wallet/balance"),
    ]
    
    for endpoint_info in wallet_endpoints:
        if len(endpoint_info) == 2:
            name, url = endpoint_info
            method = 'GET'
            data = None
        else:
            name, url, method, data = endpoint_info
        
        print(f"Testing {name}...")
        test_endpoint(name, url, method=method, data=data, timeout=15)

def test_dynamic_endpoints():
    """Test endpoints that require parameters"""
    print("\n" + "="*60)
    print("TESTING DYNAMIC ENDPOINTS")
    print("="*60)
    
    # Try to get a real block number from API
    try:
        response = requests.get(f"{API_BASE}/blocks", timeout=10)
        if response.status_code == 200:
            data = response.json()
            if 'blocks' in data and len(data['blocks']) > 0:
                block_num = data['blocks'][0].get('number') or data['blocks'][0].get('block_number', 1)
                
                print(f"Testing Block Detail (block #{block_num})...")
                test_endpoint(f"Block Detail #{block_num}", f"{BASE_URL}/block/{block_num}", timeout=15)
                
                # Try to get a transaction hash
                tx_response = requests.get(f"{API_BASE}/transactions", timeout=10)
                if tx_response.status_code == 200:
                    tx_data = tx_response.json()
                    if 'transactions' in tx_data and len(tx_data['transactions']) > 0:
                        tx_hash = tx_data['transactions'][0].get('hash') or tx_data['transactions'][0].get('tx_hash', '')
                        if tx_hash:
                            print(f"Testing Transaction Detail ({tx_hash[:16]}...)...")
                            test_endpoint(f"Transaction Detail", f"{BASE_URL}/tx/{tx_hash}", timeout=15)
                            
                            # Try address from transaction
                            from_addr = tx_data['transactions'][0].get('from') or tx_data['transactions'][0].get('from_address', '')
                            if from_addr:
                                print(f"Testing Address Detail ({from_addr[:16]}...)...")
                                test_endpoint(f"Address Detail", f"{BASE_URL}/address/{from_addr}", timeout=15)
    except Exception as e:
        print(f"⚠️ Could not test dynamic endpoints: {e}")

def print_results():
    """Print test results"""
    print("\n" + "="*60)
    print("TEST RESULTS SUMMARY")
    print("="*60)
    
    print(f"\n✅ PASSED: {len(results['passed'])}")
    for item in results['passed']:
        print(f"   {item}")
    
    print(f"\n⚠️  WARNINGS: {len(results['warnings'])}")
    for item in results['warnings']:
        print(f"   {item}")
    
    print(f"\n❌ FAILED: {len(results['failed'])}")
    for item in results['failed']:
        print(f"   {item}")
    
    total = len(results['passed']) + len(results['warnings']) + len(results['failed'])
    success_rate = (len(results['passed']) / total * 100) if total > 0 else 0
    
    print(f"\n{'='*60}")
    print(f"Success Rate: {success_rate:.1f}% ({len(results['passed'])}/{total})")
    print(f"{'='*60}\n")
    
    # Save results to file
    with open('URL_TEST_RESULTS.md', 'w') as f:
        f.write(f"# URL and Wallet Test Results\n\n")
        f.write(f"Generated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n\n")
        f.write(f"Base URL: {BASE_URL}\n")
        f.write(f"Wallet API URL: {WALLET_API_BASE}\n\n")
        f.write(f"## Summary\n\n")
        f.write(f"- ✅ Passed: {len(results['passed'])}\n")
        f.write(f"- ⚠️  Warnings: {len(results['warnings'])}\n")
        f.write(f"- ❌ Failed: {len(results['failed'])}\n")
        f.write(f"- Success Rate: {success_rate:.1f}%\n\n")
        f.write(f"## Passed Tests\n\n")
        for item in results['passed']:
            f.write(f"- {item}\n")
        f.write(f"\n## Warnings\n\n")
        for item in results['warnings']:
            f.write(f"- {item}\n")
        f.write(f"\n## Failed Tests\n\n")
        for item in results['failed']:
            f.write(f"- {item}\n")

if __name__ == '__main__':
    print("="*60)
    print("GXC EXPLORER URL & WALLET TEST SUITE")
    print("="*60)
    print(f"\nTesting against: {BASE_URL}")
    print(f"Wallet API: {WALLET_API_BASE}\n")
    
    try:
        test_explorer_endpoints()
        test_wallet_endpoints()
        test_dynamic_endpoints()
        print_results()
        
        # Exit code based on results
        if len(results['failed']) > 0:
            sys.exit(1)
        else:
            sys.exit(0)
    except KeyboardInterrupt:
        print("\n\nTest interrupted by user")
        print_results()
        sys.exit(1)
    except Exception as e:
        print(f"\n\nFatal error: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)
