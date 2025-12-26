#!/usr/bin/env python3
"""
Test sending transactions using sendrawtransaction
"""
import requests
import json
import hashlib
import time

NODE_URL = "http://localhost:8332"
MINER_ADDRESS = "tGXC4VoV6DfSnM7YwTLCciBUzizARg2H64T7n"
RECIPIENT_ADDRESS = "tGXC9fab7317231b966af85ac453e168c0932"

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
    print("SEND TRANSACTION TEST")
    print("="*70)
    print(f"Node: {NODE_URL}")
    print(f"From: {MINER_ADDRESS}")
    print(f"To: {RECIPIENT_ADDRESS}")
    print()
    
    # Step 1: Mine blocks to get coins
    print("[1/6] Mining 3 blocks to get coins...")
    for i in range(3):
        if mine_block(MINER_ADDRESS):
            print(f"  Block {i+1}/3 mined")
        time.sleep(0.1)
    
    # Step 2: Check balance
    print("\n[2/6] Checking miner balance...")
    balance_result = rpc_call("getbalance", [MINER_ADDRESS])
    if "result" in balance_result:
        balance = balance_result["result"]
        print(f"  Miner balance: {balance} GXC")
    else:
        print(f"  ❌ Failed to get balance")
        return
    
    # Step 3: Get UTXOs
    print("\n[3/6] Getting available UTXOs...")
    utxos_result = rpc_call("listunspent", [MINER_ADDRESS])
    if "result" not in utxos_result or len(utxos_result["result"]) == 0:
        print(f"  ❌ No UTXOs available")
        return
    
    utxos = utxos_result["result"]
    print(f"  Found {len(utxos)} UTXOs")
    utxo = utxos[0]
    print(f"  Using UTXO: {utxo['txHash'][:16]}..., Amount: {utxo['amount']} GXC")
    
    # Step 4: Create raw transaction
    print("\n[4/6] Creating raw transaction...")
    send_amount = 30.0
    fee = 0.001
    change = utxo["amount"] - send_amount - fee
    
    tx = {
        "type": "transfer",
        "inputs": [{
            "txHash": utxo["txHash"],
            "outputIndex": utxo["outputIndex"],
            "amount": utxo["amount"]
        }],
        "outputs": [
            {
                "address": RECIPIENT_ADDRESS,
                "amount": send_amount
            },
            {
                "address": MINER_ADDRESS,
                "amount": change
            }
        ],
        "timestamp": int(time.time())
    }
    
    print(f"  Sending: {send_amount} GXC to {RECIPIENT_ADDRESS[:20]}...")
    print(f"  Change: {change} GXC back to {MINER_ADDRESS[:20]}...")
    print(f"  Fee: {fee} GXC")
    
    # Step 5: Send raw transaction
    print("\n[5/6] Submitting transaction to mempool...")
    send_result = rpc_call("sendrawtransaction", [tx])
    
    if "result" in send_result:
        tx_hash = send_result["result"]
        if isinstance(tx_hash, dict):
            tx_hash = tx_hash.get("txid", tx_hash.get("hash", "unknown"))
        print(f"  ✅ Transaction submitted: {tx_hash[:16] if isinstance(tx_hash, str) else 'created'}...")
    else:
        error = send_result.get("error", {})
        print(f"  ❌ Transaction failed: {error.get('message', error)}")
        return
    
    # Check mempool
    mempool = rpc_call("getmempoolinfo")
    if "result" in mempool:
        print(f"  Mempool size: {mempool['result'].get('size', 0)} transactions")
    
    # Step 6: Mine block to confirm
    print("\n[6/6] Mining block to confirm transaction...")
    if mine_block(MINER_ADDRESS):
        print(f"  ✅ Confirmation block mined")
    
    time.sleep(0.5)
    
    # Verify balances
    print("\n" + "="*70)
    print("FINAL BALANCES")
    print("="*70)
    
    miner_balance = rpc_call("getbalance", [MINER_ADDRESS])
    if "result" in miner_balance:
        print(f"Miner: {miner_balance['result']} GXC")
    
    recipient_balance = rpc_call("getbalance", [RECIPIENT_ADDRESS])
    if "result" in recipient_balance:
        print(f"Recipient: {recipient_balance['result']} GXC")
        
        if recipient_balance['result'] >= send_amount:
            print(f"\n✅ SUCCESS: Recipient received {send_amount} GXC")
        else:
            print(f"\n❌ FAILED: Recipient has {recipient_balance['result']} GXC, expected {send_amount} GXC")
    
    print("="*70)

if __name__ == "__main__":
    main()
