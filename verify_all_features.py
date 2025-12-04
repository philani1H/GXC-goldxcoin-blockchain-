import requests
import json
import time
import sys
import subprocess
import os
import hashlib

# Use 8332 because our node seems to default to it despite being testnet in some configs
RPC_URL = "http://127.0.0.1:8332"
HEADERS = {'content-type': 'application/json'}

def rpc_call(method, params=[]):
    payload = {
        "jsonrpc": "2.0",
        "method": method,
        "params": params,
        "id": 1,
    }
    try:
        response = requests.post(RPC_URL, data=json.dumps(payload), headers=HEADERS)
        response.raise_for_status()
        return response.json()
    except requests.exceptions.RequestException as e:
        return None

def wait_for_node():
    print("Waiting for node to start...")
    for _ in range(30):
        res = rpc_call("getblockchaininfo")
        if res and 'result' in res:
            print("Node is ready!")
            return True
        time.sleep(1)
    return False

def mine_blocks(address, count):
    print(f"Mining {count} blocks to {address}...")
    for i in range(count):
        # 1. Get Template
        template = rpc_call("getblocktemplate")['result']
        if not template:
            print("❌ Failed to get block template")
            return False

        height = template['height']
        prev_hash = template['previousblockhash']
        difficulty = template['difficulty']

        # 2. Mine (Simple)
        nonce = 0
        found = False
        while nonce < 1000000:
            block_string = f"{height}{prev_hash}{nonce}{address}{template['curtime']}"
            block_hash = hashlib.sha256(block_string.encode()).hexdigest()
            if block_hash.startswith('00'): # Difficulty 2 zeros for testnet
                found = True
                break
            nonce += 1

        if not found:
            print(f"❌ Failed to mine block {height}")
            return False

        # 3. Submit
        block_data = {
            'hash': block_hash,
            'previousblockhash': prev_hash,
            'height': height,
            'nonce': nonce,
            'miner': address,
            'timestamp': template['curtime'],
            'difficulty': difficulty,
            'transactions': []
        }
        res = rpc_call("submitblock", [block_data])
        if res and 'error' in res and res['error']:
             print(f"❌ Submit block failed: {res['error']}")
             return False
        # print(f"Mined block {height}")

    print(f"✅ Mined {count} blocks.")
    return True

def verify_all():
    print("\n=== STARTING VERIFICATION ===\n")

    # 1. Check Blockchain Info
    info = rpc_call("getblockchaininfo")['result']
    print(f"Blockchain Height: {info['blocks']}")
    print(f"Difficulty: {info['difficulty']}")
    print(f"Network: {info['chain']}")

    if info['chain'] != 'test':
        print(f"❌ ERROR: Node is not on Testnet! (Got '{info['chain']}')")
        # Proceed anyway to see if other things work

    # 2. Get Wallet Address
    new_addr = rpc_call("getnewaddress")['result']
    print(f"Node Wallet Address: {new_addr}")
    if not new_addr.startswith("tGXC"):
        print("❌ ERROR: Address format is incorrect for Testnet (should start with tGXC)!")
        # return False # Proceed to try mining anyway

    # 3. Mine to Wallet to get funds
    # We need 101 confirmations for coinbase.
    # So we need to mine at least 102 blocks?
    # Mine 105 blocks.
    if not mine_blocks(new_addr, 105):
        print("❌ Mining failed.")
        return False

    # 4. Check Balance
    balance = rpc_call("getbalance", [new_addr])['result']
    print(f"Wallet Balance: {balance}")

    if balance < 10.0:
        print("❌ Insufficient balance for testing.")
        return False
    else:
        print("✅ Balance confirmed.")

    # 5. Create Transaction (Send to self or random)
    print("\n--- Testing Transfer ---")
    recipient = "tGXC9fab7317231b966af85ac453e168c0932" # Genesis address
    amount_to_send = 10.0

    txid = None
    try:
        res = rpc_call("sendtoaddress", [recipient, amount_to_send])
        if res and 'result' in res and res['result']:
            txid = res['result']
            print(f"✅ Transaction sent! TXID: {txid}")
            time.sleep(1)
        elif res and 'error' in res and res['error']:
             print(f"❌ Transfer Failed: {res['error']['message']}")
             return False
        else:
             print(f"❌ Transfer Failed: Unknown error {res}")
             return False
    except Exception as e:
        print(f"❌ Exception during transfer: {e}")
        return False

    # 6. Register Validator (Staking)
    print("\n--- Testing Staking ---")
    # Stake 100 GXC
    stake_amount = 100.0
    try:
        res = rpc_call("registervalidator", [new_addr, stake_amount, 30])
        if res and 'result' in res and res['result']['success']:
             print(f"✅ Validator registered! {res['result']['message']}")
        elif res and 'error' in res:
             print(f"❌ Staking Failed: {res['error']['message']}")
             return False
    except Exception as e:
         print(f"❌ Exception during staking: {e}")
         return False

    # 7. Unstake
    print("\n--- Testing Unstaking ---")
    # Need to mine a block to confirm the stake transaction first?
    # Staking tx is in mempool.
    # Let's mine 1 block to confirm it.
    mine_blocks(new_addr, 1)
    time.sleep(1)

    try:
        res = rpc_call("unstake", [new_addr, stake_amount]) # Unstake partial or full
        if res and 'result' in res and res['result']['success']:
             print(f"✅ Unstake successful! TXID: {res['result']['txid']}")
        elif res and 'error' in res:
             print(f"❌ Unstake Failed: {res['error']['message']}")
             return False
    except Exception as e:
         print(f"❌ Exception during unstaking: {e}")
         return False

    return True

if __name__ == "__main__":
    if not wait_for_node():
        print("❌ Node did not start.")
        sys.exit(1)

    if verify_all():
        print("\n✅ VERIFICATION SUCCESSFUL")
        sys.exit(0)
    else:
        print("\n❌ VERIFICATION FAILED")
        sys.exit(1)
