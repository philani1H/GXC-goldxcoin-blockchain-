#!/usr/bin/env python3
"""
Test sending using node's built-in wallet
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
    
    response = requests.post(NODE_URL, json=payload, timeout=10)
    return response.json()

def mine_block(miner_address):
    """Mine a single block"""
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
    print("WALLET SEND TEST - Using Node's Built-in Wallet")
    print("="*70)
    print(f"Node: {NODE_URL}")
    print()
    
    # Step 1: Get node's wallet address
    print("[1/7] Getting node's wallet address...")
    address_result = rpc_call("getnewaddress")
    if "result" in address_result:
        wallet_address = address_result["result"]
        print(f"  Wallet address: {wallet_address}")
    else:
        print(f"  ❌ Failed to get address")
        return
    
    # Step 2: Mine blocks to wallet (need 100+ for coinbase maturity)
    print(f"\n[2/7] Mining 105 blocks to wallet address (coinbase maturity = 100)...")
    for i in range(105):
        if mine_block(wallet_address):
            if (i+1) % 10 == 0 or i < 5:
                print(f"  Block {i+1}/105 mined")
        time.sleep(0.01)
    
    # Step 3: Check wallet balance
    print("\n[3/7] Checking wallet balance...")
    balance_result = rpc_call("getbalance", [wallet_address])
    if "result" in balance_result:
        balance = balance_result["result"]
        print(f"  Wallet balance: {balance} GXC")
    else:
        print(f"  ❌ Failed to get balance")
        return
    
    # Step 4: Use a different recipient address
    print("\n[4/7] Using recipient address...")
    recipient_address = "tGXC4VoV6DfSnM7YwTLCciBUzizARg2H64T7n"  # Different from wallet
    print(f"  Recipient address: {recipient_address}")
    
    # Step 5: Send transaction using wallet
    print("\n[5/7] Sending 30 GXC from wallet to recipient...")
    send_amount = 30.0
    
    # Use sendtoaddress with wallet's address as sender
    send_result = rpc_call("sendtoaddress", [recipient_address, send_amount, wallet_address])
    
    if "result" in send_result:
        tx_hash = send_result["result"]
        if isinstance(tx_hash, dict):
            tx_hash = tx_hash.get("txid", tx_hash.get("hash", "created"))
        print(f"  ✅ Transaction created: {tx_hash[:16] if isinstance(tx_hash, str) else 'success'}...")
    else:
        error = send_result.get("error", {})
        print(f"  ❌ Transaction failed: {error.get('message', error)}")
        return
    
    # Check mempool
    print("\n[6/7] Checking mempool...")
    mempool = rpc_call("getmempoolinfo")
    if "result" in mempool:
        size = mempool['result'].get('size', 0)
        print(f"  Mempool size: {size} transactions")
        
        if size > 0:
            print(f"  ✅ Transaction in mempool")
            
            # Mine block to confirm
            print(f"  Mining block to confirm...")
            if mine_block(wallet_address):
                print(f"  ✅ Confirmation block mined")
            time.sleep(0.5)
    
    # Step 7: Verify balances
    print("\n[7/7] Verifying final balances...")
    
    wallet_balance = rpc_call("getbalance", [wallet_address])
    if "result" in wallet_balance:
        print(f"  Wallet: {wallet_balance['result']} GXC")
    
    recipient_balance = rpc_call("getbalance", [recipient_address])
    if "result" in recipient_balance:
        print(f"  Recipient: {recipient_balance['result']} GXC")
    
    # Final summary
    print("\n" + "="*70)
    print("SUMMARY")
    print("="*70)
    
    if "result" in recipient_balance and recipient_balance['result'] >= send_amount:
        print(f"✅ SUCCESS: Transaction sent and confirmed!")
        print(f"   Sent: {send_amount} GXC")
        print(f"   Recipient received: {recipient_balance['result']} GXC")
    else:
        print(f"❌ FAILED: Transaction not confirmed")
    
    print("="*70)

if __name__ == "__main__":
    main()
