#!/usr/bin/env python3
"""
Test pending balance vs confirmed balance
"""
import requests
import json

NODE_URL = "http://localhost:8332"

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

def mine_block(miner_address):
    """Mine a single block"""
    import hashlib
    import time
    
    template_result = rpc_call("getblocktemplate", [{"minerAddress": miner_address}])
    if "error" in template_result:
        return False
    
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
    
    submit_result = rpc_call("submitblock", [template])
    return "error" not in submit_result

def main():
    print("="*70)
    print("PENDING vs CONFIRMED BALANCE TEST")
    print("="*70)
    
    wallet_address = "tGXCf7100e1540f3785bb5d03aa93c1ee0c8f9"
    recipient_address = "tGXC4VoV6DfSnM7YwTLCciBUzizARg2H64T7n"
    
    # Mine 105 blocks first
    print("\n[0] Mining 105 blocks (for mature coinbase)...")
    print("-" * 70)
    for i in range(105):
        if mine_block(wallet_address):
            if (i+1) % 20 == 0:
                print(f"  Mined {i+1}/105 blocks")
    print("  ✅ Mining complete")
    
    # Check current balances
    print("\n[1] Current Balances (Before Transaction)")
    print("-" * 70)
    
    wallet_balance = rpc_call("getbalance", [wallet_address])
    if "result" in wallet_balance:
        print(f"Wallet: {wallet_balance['result']} GXC")
    
    recipient_balance = rpc_call("getbalance", [recipient_address])
    if "result" in recipient_balance:
        print(f"Recipient: {recipient_balance['result']} GXC")
    
    # Check mempool
    print("\n[2] Mempool Status")
    print("-" * 70)
    
    mempool = rpc_call("getmempoolinfo")
    if "result" in mempool:
        print(f"Pending transactions: {mempool['result']['size']}")
        print(f"Mempool size: {mempool['result']['bytes']} bytes")
    
    # Send a transaction (without mining confirmation block)
    print("\n[3] Sending 20 GXC (without confirmation)")
    print("-" * 70)
    
    send_result = rpc_call("sendtoaddress", [recipient_address, 20.0, wallet_address])
    
    if "result" in send_result:
        tx_hash = send_result["result"]
        print(f"✅ Transaction created: {tx_hash[:16] if isinstance(tx_hash, str) else 'success'}...")
    else:
        print(f"❌ Transaction failed: {send_result.get('error', {}).get('message', 'Unknown')}")
        return
    
    # Check mempool again
    print("\n[4] Mempool After Transaction")
    print("-" * 70)
    
    mempool = rpc_call("getmempoolinfo")
    if "result" in mempool:
        print(f"Pending transactions: {mempool['result']['size']}")
        print(f"Mempool size: {mempool['result']['bytes']} bytes")
    
    # Check balances again
    print("\n[5] Balances After Transaction (Unconfirmed)")
    print("-" * 70)
    
    wallet_balance = rpc_call("getbalance", [wallet_address])
    if "result" in wallet_balance:
        print(f"Wallet: {wallet_balance['result']} GXC")
    
    recipient_balance = rpc_call("getbalance", [recipient_address])
    if "result" in recipient_balance:
        print(f"Recipient: {recipient_balance['result']} GXC")
    
    # Check pending balance
    print("\n[6] Pending Balance (New Method)")
    print("-" * 70)
    
    pending_balance = rpc_call("getpendingbalance", [recipient_address])
    if "result" in pending_balance:
        print(f"Recipient pending balance: {pending_balance['result']} GXC")
    
    # Check if balance includes pending
    print("\n[7] Analysis")
    print("-" * 70)
    
    if "result" in recipient_balance:
        if recipient_balance['result'] > 30.0:
            print("❌ Recipient confirmed balance INCLUDES pending transaction")
            print("   This is WRONG - should only show confirmed")
        else:
            print("✅ Recipient confirmed balance DOES NOT include pending")
            print("   Balance only shows confirmed transactions (CORRECT!)")
    
    if "result" in pending_balance:
        if pending_balance['result'] >= 20.0:
            print("✅ Pending balance shows unconfirmed transaction (CORRECT!)")
        else:
            print("❌ Pending balance does not show unconfirmed transaction")
    
    print("\n" + "="*70)
    print("RECOMMENDATION")
    print("="*70)
    print("Best Practice (Bitcoin/Ethereum):")
    print("  - Confirmed balance: Only confirmed transactions")
    print("  - Pending balance: Separate field for unconfirmed")
    print("  - User sees both, but can only spend confirmed")
    print("="*70)

if __name__ == "__main__":
    main()
