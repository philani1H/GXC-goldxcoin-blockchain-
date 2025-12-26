#!/usr/bin/env python3
"""
Complete staking test with correct parameters
"""
import requests
import json
import hashlib
import time

NODE_URL = "http://localhost:8332"

def rpc_call(method, params=None):
    payload = {"jsonrpc": "2.0", "id": 1, "method": method, "params": params or []}
    response = requests.post(NODE_URL, json=payload, timeout=10)
    return response.json()

def mine_block(miner_address):
    template_result = rpc_call("getblocktemplate", [{"minerAddress": miner_address}])
    if "error" in template_result:
        return False, None
    
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
    return "error" not in submit_result, template.get("height")

print("="*70)
print("STAKING TEST - With Correct Parameters")
print("="*70)

# Get address
address_result = rpc_call("getnewaddress")
wallet_address = address_result["result"]
print(f"Wallet: {wallet_address}")

# Mine 105 blocks
print("\nMining 105 blocks...")
for i in range(105):
    mine_block(wallet_address)
    if (i+1) % 20 == 0:
        print(f"  {i+1}/105 blocks mined")

# Check balance
balance = rpc_call("getbalance", [wallet_address])["result"]
print(f"\nBalance: {balance} GXC")

# Stake with correct parameters: address, amount, days
print("\n" + "="*70)
print("STAKING")
print("="*70)

stake_amount = 100.0
staking_days = 30
print(f"Staking {stake_amount} GXC for {staking_days} days...")

stake_result = rpc_call("registervalidator", [wallet_address, stake_amount, staking_days])

if "result" in stake_result:
    print(f"✅ Staked successfully!")
    print(f"   Result: {stake_result['result']}")
else:
    error = stake_result.get("error", {})
    print(f"❌ Staking failed: {error.get('message', error)}")

# Mine confirmation block
print("\nMining confirmation block...")
mine_block(wallet_address)
time.sleep(0.5)

# Check staking info
print("\n" + "="*70)
print("STAKING INFO")
print("="*70)

staking_info = rpc_call("getstakinginfo", [wallet_address])

if "result" in staking_info:
    info = staking_info["result"]
    print(f"Staked: {info.get('staked', 0)} GXC")
    print(f"Status: {info.get('status', 'unknown')}")
    print(f"Can stake: {info.get('canStake', False)}")
else:
    print(f"Info: {staking_info.get('error', {}).get('message', 'N/A')}")

# Check validators
print("\n" + "="*70)
print("VALIDATORS")
print("="*70)

validators = rpc_call("getvalidators")

if "result" in validators:
    val_list = validators["result"]
    print(f"Total validators: {len(val_list) if isinstance(val_list, list) else 'N/A'}")
    if isinstance(val_list, list) and len(val_list) > 0:
        for v in val_list[:3]:
            if isinstance(v, dict):
                print(f"  - {v.get('address', 'N/A')[:20]}... Stake: {v.get('stake', 0)} GXC")
else:
    print(f"Validators: {validators.get('error', {}).get('message', 'N/A')}")

# Unstake
print("\n" + "="*70)
print("UNSTAKING")
print("="*70)

unstake_amount = 50.0
print(f"Unstaking {unstake_amount} GXC...")

unstake_result = rpc_call("unstake", [wallet_address, unstake_amount])

if "result" in unstake_result:
    print(f"✅ Unstaked successfully!")
    print(f"   Result: {unstake_result['result']}")
else:
    error = unstake_result.get("error", {})
    print(f"❌ Unstaking failed: {error.get('message', error)}")

# Mine confirmation
print("\nMining confirmation block...")
mine_block(wallet_address)
time.sleep(0.5)

# Final balance
final_balance = rpc_call("getbalance", [wallet_address])["result"]
print(f"\nFinal balance: {final_balance} GXC")

print("\n" + "="*70)
print("TEST COMPLETE")
print("="*70)
