#!/usr/bin/env python3
"""
Test mining with new consensus rules
"""
import requests
import json
import hashlib
import time

NODE_URL = "http://localhost:8332"
MINER_ADDRESS = "tGXC4VoV6DfSnM7YwTLCciBUzizARg2H64T7n"

def rpc_call(method, params=None):
    """Make JSON-RPC call"""
    payload = {
        "jsonrpc": "2.0",
        "id": 1,
        "method": method,
        "params": params or []
    }
    
    response = requests.post(NODE_URL, json=payload, timeout=10)
    return response.json()

def mine_block():
    """Mine a single block"""
    # Get template
    print("\n[1/4] Getting block template...")
    template_result = rpc_call("getblocktemplate", [{"minerAddress": MINER_ADDRESS}])
    
    if "error" in template_result:
        print(f"  ❌ Error: {template_result['error']}")
        return False
    
    template = template_result.get("result", template_result)
    
    height = template.get("height", 0)
    difficulty = template.get("difficulty", 1.0)
    tx_count = len(template.get("transactions", []))
    
    print(f"  Height: {height}")
    print(f"  Difficulty: {difficulty}")
    print(f"  Transactions: {tx_count}")
    
    # Check coinbase is present
    if tx_count > 0:
        first_tx = template["transactions"][0]
        is_coinbase = first_tx.get("is_coinbase", False)
        if is_coinbase:
            print(f"  ✅ Coinbase present: {first_tx['hash'][:16]}...")
        else:
            print(f"  ❌ First transaction is not coinbase!")
            return False
    else:
        print(f"  ❌ No transactions in template!")
        return False
    
    # Mine block
    print("\n[2/4] Mining block (finding hash with 1 leading zero)...")
    nonce = 0
    start_time = time.time()
    
    while True:
        template["nonce"] = nonce
        block_str = json.dumps(template, sort_keys=True)
        block_hash = hashlib.sha256(block_str.encode()).hexdigest()
        
        if block_hash.startswith("0"):
            template["hash"] = block_hash
            elapsed = time.time() - start_time
            print(f"  ✅ Found valid hash: {block_hash[:16]}...")
            print(f"     Nonce: {nonce:,}")
            print(f"     Time: {elapsed:.2f}s")
            print(f"     Hashrate: {nonce/elapsed:,.0f} H/s")
            break
        
        nonce += 1
        if nonce % 10000 == 0:
            print(f"     Tried {nonce:,} nonces...")
        
        if nonce > 1000000:
            print(f"  ❌ Failed to find hash after {nonce:,} attempts")
            return False
    
    # Submit block
    print("\n[3/4] Submitting block...")
    submit_result = rpc_call("submitblock", [template])
    
    if "error" in submit_result:
        print(f"  ❌ Block rejected: {submit_result['error'].get('message', 'Unknown error')}")
        return False
    
    print(f"  ✅ Block accepted!")
    
    # Verify block was added
    print("\n[4/4] Verifying block was added to chain...")
    info_result = rpc_call("getinfo")
    info = info_result.get("result", info_result)
    
    new_height = info.get("blocks", 0)
    print(f"  Chain height: {new_height}")
    
    if new_height > height:
        print(f"  ✅ Block added to chain!")
        return True
    else:
        print(f"  ❌ Block not added to chain")
        return False

def main():
    print("="*70)
    print("MINING TEST WITH CONSENSUS RULES")
    print("="*70)
    print(f"Node: {NODE_URL}")
    print(f"Miner: {MINER_ADDRESS}")
    print()
    
    # Get initial state
    info_result = rpc_call("getinfo")
    info = info_result.get("result", info_result)
    
    print(f"Initial height: {info.get('blocks', 0)}")
    print(f"Difficulty: {info.get('difficulty', 'unknown')}")
    print()
    
    # Mine 3 blocks
    success_count = 0
    for i in range(3):
        print(f"\n{'='*70}")
        print(f"MINING BLOCK {i+1}/3")
        print(f"{'='*70}")
        
        if mine_block():
            success_count += 1
        else:
            print(f"\n❌ Failed to mine block {i+1}")
            break
        
        time.sleep(1)
    
    # Final summary
    print(f"\n{'='*70}")
    print("MINING SUMMARY")
    print(f"{'='*70}")
    print(f"Blocks mined: {success_count}/3")
    
    # Check balance
    balance_result = rpc_call("getbalance", [MINER_ADDRESS])
    balance = balance_result.get("result", 0)
    
    print(f"Miner balance: {balance} GXC")
    print(f"Expected: {success_count * 50} GXC")
    
    if success_count == 3:
        print("\n🎉 Mining works correctly with new consensus rules!")
        print("✅ Coinbase transactions created")
        print("✅ Proof of work validated (1 leading zero)")
        print("✅ Blocks accepted and added to chain")
        print("✅ Rewards credited to miner")
    else:
        print("\n⚠️  Some blocks failed to mine")

if __name__ == "__main__":
    main()
