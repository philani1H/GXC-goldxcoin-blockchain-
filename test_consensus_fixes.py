#!/usr/bin/env python3
"""
Test consensus fixes - verify blocks are properly rejected
"""
import requests
import json
import hashlib
import time

NODE_URL = "http://localhost:8332"

def rpc_call(method, params=None):
    """Make JSON-RPC call"""
    payload = {
        "jsonrpc": "2.0",
        "id": 1,
        "method": method,
        "params": params or []
    }
    
    try:
        response = requests.post(NODE_URL, json=payload, timeout=10)
        result = response.json()
        return result
    except Exception as e:
        return {"error": str(e)}

def test_insufficient_pow():
    """Test 1: Block with insufficient PoW should be rejected"""
    print("\n[Test 1] Submitting block with insufficient PoW (0 leading zeros)...")
    
    # Get template
    template = rpc_call("getblocktemplate", [{"minerAddress": "tGXC_test_address"}])
    if "error" in template:
        print(f"  ❌ Failed to get template: {template['error']}")
        return False
    
    result = template.get("result", template)
    
    # Create block with bad hash (no leading zeros)
    bad_block = result.copy()
    bad_block["hash"] = "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"
    bad_block["nonce"] = 0
    
    # Try to submit
    submit_result = rpc_call("submitblock", [bad_block])
    
    if "error" in submit_result:
        print(f"  ✅ Block correctly rejected: {submit_result['error'].get('message', 'Unknown error')}")
        return True
    else:
        print(f"  ❌ Block was accepted (should have been rejected!)")
        return False

def test_missing_coinbase():
    """Test 2: Block without coinbase should be rejected"""
    print("\n[Test 2] Submitting block without coinbase transaction...")
    
    # Get template
    template = rpc_call("getblocktemplate", [{"minerAddress": "tGXC_test_address"}])
    if "error" in template:
        print(f"  ❌ Failed to get template: {template['error']}")
        return False
    
    result = template.get("result", template)
    
    # Remove all transactions (including coinbase)
    bad_block = result.copy()
    bad_block["transactions"] = []
    
    # Create valid hash with 1 leading zero
    block_str = json.dumps(bad_block, sort_keys=True)
    bad_block["hash"] = "0" + hashlib.sha256(block_str.encode()).hexdigest()[1:]
    bad_block["nonce"] = 1
    
    # Try to submit
    submit_result = rpc_call("submitblock", [bad_block])
    
    if "error" in submit_result:
        print(f"  ✅ Block correctly rejected: {submit_result['error'].get('message', 'Unknown error')}")
        return True
    else:
        print(f"  ❌ Block was accepted (should have been rejected!)")
        return False

def test_difficulty_mismatch():
    """Test 3: Block with wrong difficulty should be rejected"""
    print("\n[Test 3] Submitting block with wrong difficulty...")
    
    # Get template
    template = rpc_call("getblocktemplate", [{"minerAddress": "tGXC_test_address"}])
    if "error" in template:
        print(f"  ❌ Failed to get template: {template['error']}")
        return False
    
    result = template.get("result", template)
    
    # Change difficulty
    bad_block = result.copy()
    bad_block["difficulty"] = 0.1  # Wrong difficulty (network requires 1.0)
    
    # Create valid hash with 1 leading zero
    block_str = json.dumps(bad_block, sort_keys=True)
    bad_block["hash"] = "0" + hashlib.sha256(block_str.encode()).hexdigest()[1:]
    bad_block["nonce"] = 1
    
    # Try to submit
    submit_result = rpc_call("submitblock", [bad_block])
    
    if "error" in submit_result:
        print(f"  ✅ Block correctly rejected: {submit_result['error'].get('message', 'Unknown error')}")
        return True
    else:
        print(f"  ❌ Block was accepted (should have been rejected!)")
        return False

def test_valid_block():
    """Test 4: Valid block with proper PoW and coinbase should be accepted"""
    print("\n[Test 4] Submitting valid block with proper PoW and coinbase...")
    
    # Get template
    template = rpc_call("getblocktemplate", [{"minerAddress": "tGXC_test_address"}])
    if "error" in template:
        print(f"  ❌ Failed to get template: {template['error']}")
        return False
    
    result = template.get("result", template)
    
    # Mine block (find hash with 1 leading zero)
    nonce = 0
    while True:
        result["nonce"] = nonce
        block_str = json.dumps(result, sort_keys=True)
        block_hash = hashlib.sha256(block_str.encode()).hexdigest()
        
        if block_hash.startswith("0"):
            result["hash"] = block_hash
            break
        
        nonce += 1
        if nonce > 100000:
            print("  ❌ Failed to find valid hash")
            return False
    
    print(f"  Found valid hash: {block_hash[:16]}... (nonce: {nonce})")
    
    # Try to submit
    submit_result = rpc_call("submitblock", [result])
    
    if "error" in submit_result:
        print(f"  ❌ Block was rejected: {submit_result['error'].get('message', 'Unknown error')}")
        return False
    else:
        print(f"  ✅ Block correctly accepted")
        return True

def main():
    print("="*70)
    print("CONSENSUS FIXES VERIFICATION")
    print("="*70)
    print(f"Node: {NODE_URL}")
    print()
    
    # Check if node is running
    info = rpc_call("getinfo")
    if "error" in info and "result" not in info:
        print("❌ Node is not running. Start it with: ./gxc-node --testnet")
        return
    
    result = info.get("result", info)
    print(f"Node height: {result.get('height', 'unknown')}")
    print(f"Difficulty: {result.get('difficulty', 'unknown')}")
    print()
    
    # Run tests
    tests = [
        ("Insufficient PoW", test_insufficient_pow),
        ("Missing Coinbase", test_missing_coinbase),
        ("Difficulty Mismatch", test_difficulty_mismatch),
        ("Valid Block", test_valid_block),
    ]
    
    results = []
    for name, test_func in tests:
        try:
            passed = test_func()
            results.append((name, passed))
        except Exception as e:
            print(f"  ❌ Test error: {e}")
            results.append((name, False))
    
    # Summary
    print("\n" + "="*70)
    print("TEST SUMMARY")
    print("="*70)
    
    passed_count = sum(1 for _, passed in results if passed)
    total_count = len(results)
    
    for name, passed in results:
        status = "✅ PASS" if passed else "❌ FAIL"
        print(f"{status}: {name}")
    
    print()
    print(f"Results: {passed_count}/{total_count} tests passed")
    
    if passed_count == total_count:
        print("\n🎉 All consensus fixes working correctly!")
        print("Blocks are now properly validated and rejected when invalid.")
    else:
        print("\n⚠️  Some tests failed. Check the logs above.")

if __name__ == "__main__":
    main()
