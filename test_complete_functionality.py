#!/usr/bin/env python3
"""
Complete functionality test: Mining, Sending, Staking, and all RPC methods
"""
import requests
import json
import hashlib
import time

NODE_URL = "http://localhost:8332"

# Test addresses
MINER_ADDRESS = "tGXC4VoV6DfSnM7YwTLCciBUzizARg2H64T7n"
RECIPIENT_ADDRESS = "tGXC9fab7317231b966af85ac453e168c0932"
STAKER_ADDRESS = "tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9"

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

def mine_block(miner_address):
    """Mine a single block"""
    template_result = rpc_call("getblocktemplate", [{"minerAddress": miner_address}])
    if "error" in template_result:
        return False, template_result["error"]
    
    template = template_result.get("result", template_result)
    
    nonce = 0
    while nonce < 1000000:
        template["nonce"] = nonce
        block_str = json.dumps(template, sort_keys=True)
        block_hash = hashlib.sha256(block_str.encode()).hexdigest()
        
        if block_hash.startswith("0"):
            template["hash"] = block_hash
            break
        nonce += 1
    
    if nonce >= 1000000:
        return False, "Failed to find valid hash"
    
    submit_result = rpc_call("submitblock", [template])
    
    if "error" in submit_result:
        return False, submit_result["error"]
    
    return True, {"height": template["height"], "hash": block_hash[:16] + "...", "nonce": nonce}

def print_section(title):
    """Print section header"""
    print("\n" + "="*70)
    print(f"  {title}")
    print("="*70)

def print_test(name, passed, details=""):
    """Print test result"""
    status = "✅ PASS" if passed else "❌ FAIL"
    print(f"{status}: {name}")
    if details:
        print(f"       {details}")

def main():
    print("="*70)
    print("COMPLETE FUNCTIONALITY TEST")
    print("="*70)
    print(f"Node: {NODE_URL}")
    print(f"Miner: {MINER_ADDRESS}")
    print(f"Recipient: {RECIPIENT_ADDRESS}")
    print(f"Staker: {STAKER_ADDRESS}")
    
    # Test 1: Node Info
    print_section("TEST 1: Node Information")
    
    info = rpc_call("getinfo")
    if "result" in info:
        result = info["result"]
        print_test("getinfo", True, f"Height: {result.get('blocks', 0)}, Difficulty: {result.get('difficulty', 0)}")
    else:
        print_test("getinfo", False, str(info.get("error", "Unknown error")))
        return
    
    # Test 2: Mine blocks to miner
    print_section("TEST 2: Mining Blocks")
    
    print("Mining 5 blocks to miner address...")
    mined_count = 0
    for i in range(5):
        success, result = mine_block(MINER_ADDRESS)
        if success:
            mined_count += 1
            print(f"  Block {i+1}/5: Height {result['height']}, Nonce {result['nonce']}, Hash {result['hash']}")
        else:
            print(f"  Block {i+1}/5: FAILED - {result}")
        time.sleep(0.1)
    
    print_test("Mining", mined_count == 5, f"{mined_count}/5 blocks mined")
    
    # Test 3: Check miner balance
    print_section("TEST 3: Balance Check")
    
    balance_result = rpc_call("getbalance", [MINER_ADDRESS])
    if "result" in balance_result:
        balance = balance_result["result"]
        expected = mined_count * 50
        print_test("Miner balance", balance == expected, f"{balance} GXC (expected {expected} GXC)")
    else:
        print_test("Miner balance", False, str(balance_result.get("error", "Unknown error")))
    
    # Test 4: List UTXOs
    print_section("TEST 4: List UTXOs")
    
    utxos_result = rpc_call("listunspent", [MINER_ADDRESS])
    if "result" in utxos_result:
        utxos = utxos_result["result"]
        print_test("listunspent", len(utxos) > 0, f"Found {len(utxos)} UTXOs")
        if len(utxos) > 0:
            tx_hash = utxos[0].get('txHash', utxos[0].get('txid', 'N/A'))
            print(f"       First UTXO: {tx_hash[:16] if tx_hash != 'N/A' else 'N/A'}..., Amount: {utxos[0].get('amount', 0)} GXC")
    else:
        print_test("listunspent", False, str(utxos_result.get("error", "Unknown error")))
        utxos = []
    
    # Test 5: Send transaction
    print_section("TEST 5: Send Transaction")
    
    if len(utxos) > 0:
        send_amount = 30.0
        print(f"Sending {send_amount} GXC from miner to recipient...")
        
        # Get UTXO details
        utxo = utxos[0]
        tx_hash = utxo.get("txHash", utxo.get("txid"))
        output_index = utxo.get("outputIndex", utxo.get("vout", 0))
        utxo_amount = utxo.get("amount")
        
        send_result = rpc_call("sendtoaddress", [RECIPIENT_ADDRESS, send_amount, MINER_ADDRESS])
        
        if "result" in send_result:
            tx_hash = send_result["result"]
            print_test("sendtoaddress", True, f"TX: {tx_hash[:16] if isinstance(tx_hash, str) else 'created'}...")
            
            # Mine a block to confirm
            print("  Mining block to confirm transaction...")
            success, result = mine_block(MINER_ADDRESS)
            if success:
                print(f"  Confirmation block mined at height {result['height']}")
            
            time.sleep(0.5)
            
            # Check recipient balance
            recipient_balance = rpc_call("getbalance", [RECIPIENT_ADDRESS])
            if "result" in recipient_balance:
                balance = recipient_balance["result"]
                print_test("Recipient received", balance >= send_amount, f"{balance} GXC")
            
        else:
            print_test("sendtoaddress", False, str(send_result.get("error", "Unknown error")))
    else:
        print_test("Send transaction", False, "No UTXOs available")
    
    # Test 6: Staking
    print_section("TEST 6: Staking")
    
    # Mine some blocks to staker address first
    print("Mining 3 blocks to staker address...")
    for i in range(3):
        success, result = mine_block(STAKER_ADDRESS)
        if success:
            print(f"  Block {i+1}/3 mined")
        time.sleep(0.1)
    
    # Check staker balance
    staker_balance_result = rpc_call("getbalance", [STAKER_ADDRESS])
    if "result" in staker_balance_result:
        staker_balance = staker_balance_result["result"]
        print(f"  Staker balance: {staker_balance} GXC")
        
        if staker_balance >= 100:
            # Try to stake
            stake_amount = 100.0
            print(f"  Attempting to stake {stake_amount} GXC...")
            
            stake_result = rpc_call("stake", [STAKER_ADDRESS, stake_amount])
            
            if "result" in stake_result:
                print_test("Staking", True, f"Staked {stake_amount} GXC")
            else:
                error_msg = str(stake_result.get("error", {}).get("message", "Unknown error"))
                print_test("Staking", False, error_msg)
        else:
            print_test("Staking", False, f"Insufficient balance: {staker_balance} GXC (need 100 GXC)")
    
    # Test 7: Get blockchain info
    print_section("TEST 7: Blockchain Information")
    
    blockcount = rpc_call("getblockcount")
    if "result" in blockcount:
        print_test("getblockcount", True, f"{blockcount['result']} blocks")
    else:
        print_test("getblockcount", False)
    
    difficulty = rpc_call("getdifficulty")
    if "result" in difficulty:
        print_test("getdifficulty", True, f"{difficulty['result']}")
    else:
        print_test("getdifficulty", False)
    
    # Test 8: Get block by height
    print_section("TEST 8: Get Block Information")
    
    if "result" in blockcount and blockcount["result"] > 0:
        block_height = 0
        block_result = rpc_call("getblock", [block_height])
        
        if "result" in block_result:
            block = block_result["result"]
            print_test("getblock", True, f"Height {block.get('height', 'N/A')}, Hash: {block.get('hash', 'N/A')[:16]}...")
        else:
            print_test("getblock", False)
    
    # Test 9: Get transaction
    print_section("TEST 9: Get Transaction")
    
    if len(utxos) > 0:
        tx_hash = utxos[0].get("txHash", utxos[0].get("txid"))
        tx_result = rpc_call("gettransaction", [tx_hash])
        
        if "result" in tx_result:
            tx = tx_result["result"]
            print_test("gettransaction", True, f"TX: {tx.get('txid', 'N/A')[:16]}...")
        else:
            print_test("gettransaction", False)
    
    # Test 10: Mempool info
    print_section("TEST 10: Mempool Information")
    
    mempool_result = rpc_call("getmempoolinfo")
    if "result" in mempool_result:
        mempool = mempool_result["result"]
        print_test("getmempoolinfo", True, f"{mempool.get('size', 0)} transactions in mempool")
    else:
        print_test("getmempoolinfo", False)
    
    # Final Summary
    print_section("FINAL SUMMARY")
    
    final_info = rpc_call("getinfo")
    if "result" in final_info:
        result = final_info["result"]
        print(f"Final blockchain height: {result.get('blocks', 0)}")
        print(f"Final difficulty: {result.get('difficulty', 0)}")
    
    miner_final = rpc_call("getbalance", [MINER_ADDRESS])
    if "result" in miner_final:
        print(f"Miner balance: {miner_final['result']} GXC")
    
    recipient_final = rpc_call("getbalance", [RECIPIENT_ADDRESS])
    if "result" in recipient_final:
        print(f"Recipient balance: {recipient_final['result']} GXC")
    
    staker_final = rpc_call("getbalance", [STAKER_ADDRESS])
    if "result" in staker_final:
        print(f"Staker balance: {staker_final['result']} GXC")
    
    print("\n" + "="*70)
    print("TEST COMPLETE")
    print("="*70)

if __name__ == "__main__":
    main()
