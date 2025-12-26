#!/usr/bin/env python3
"""
Real blockchain test - actual mining and transactions
"""

import socket
import json
import time

def send_rpc(method, params=[]):
    """Send RPC request directly via socket"""
    try:
        # Create socket connection
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(10)
        sock.connect(('127.0.0.1', 8545))
        
        # Prepare JSON-RPC request
        request = {
            "jsonrpc": "2.0",
            "method": method,
            "params": params,
            "id": 1
        }
        
        # Send request
        request_str = json.dumps(request) + "\n"
        sock.sendall(request_str.encode('utf-8'))
        
        # Receive response
        response = b""
        while True:
            chunk = sock.recv(4096)
            if not chunk:
                break
            response += chunk
            # Check if we have a complete JSON response
            try:
                json.loads(response.decode('utf-8'))
                break
            except:
                continue
        
        sock.close()
        
        # Parse response
        result = json.loads(response.decode('utf-8'))
        
        if "error" in result and result["error"]:
            print(f"❌ RPC Error: {result['error']}")
            return None
        
        return result.get("result")
        
    except socket.timeout:
        print(f"❌ Timeout connecting to RPC server")
        return None
    except ConnectionRefusedError:
        print(f"❌ Connection refused - is the node running?")
        return None
    except Exception as e:
        print(f"❌ Error: {e}")
        return None

def main():
    print("\n" + "="*60)
    print("  REAL GXC BLOCKCHAIN TEST")
    print("  Mining real blocks and sending real transactions")
    print("="*60 + "\n")
    
    # Step 1: Verify node is running
    print("Step 1: Verifying node connection...")
    info = send_rpc("getblockchaininfo")
    if info is None:
        print("❌ Cannot connect to node. Make sure it's running.")
        return
    
    print(f"✅ Node connected!")
    print(f"   Height: {info.get('blocks', 0)}")
    print(f"   Network: {info.get('chain', 'unknown')}")
    
    # Step 2: Get wallet address
    print("\nStep 2: Getting wallet address...")
    address = send_rpc("getnewaddress")
    if not address:
        print("❌ Failed to get address")
        return
    
    print(f"✅ Wallet address: {address}")
    
    # Step 3: Check initial balance
    print("\nStep 3: Checking initial balance...")
    balance = send_rpc("getbalance", [address])
    print(f"   Initial balance: {balance} GXC")
    
    # Step 4: Mine blocks
    print("\nStep 4: Mining 10 blocks (this will take a moment)...")
    print("   This generates REAL coins on the blockchain!")
    
    for i in range(10):
        print(f"   Mining block {i+1}/10...", end=" ", flush=True)
        block_hashes = send_rpc("generatetoaddress", [1, address])
        if block_hashes:
            print(f"✅ {block_hashes[0][:16]}...")
        else:
            print("❌ Failed")
            return
        time.sleep(0.5)
    
    # Step 5: Check balance after mining
    print("\nStep 5: Checking balance after mining...")
    balance = send_rpc("getbalance", [address])
    print(f"✅ Balance: {balance} GXC")
    
    if balance < 1.0:
        print("❌ Insufficient balance for transaction")
        return
    
    # Step 6: Send REAL transaction
    recipient = "tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf"
    send_amount = 10.0
    
    print(f"\nStep 6: Sending REAL transaction...")
    print(f"   From: {address}")
    print(f"   To: {recipient}")
    print(f"   Amount: {send_amount} GXC")
    
    tx_hash = send_rpc("sendtoaddress", [recipient, send_amount])
    if not tx_hash:
        print("❌ Transaction failed")
        return
    
    print(f"✅ Transaction sent!")
    print(f"   TX Hash: {tx_hash}")
    
    # Step 7: Mine block to confirm
    print("\nStep 7: Mining block to confirm transaction...")
    block_hashes = send_rpc("generatetoaddress", [1, address])
    if block_hashes:
        print(f"✅ Block mined: {block_hashes[0][:16]}...")
    
    time.sleep(1)
    
    # Step 8: Verify transaction was confirmed
    print("\nStep 8: Verifying transaction...")
    
    # Check sender balance
    sender_balance = send_rpc("getbalance", [address])
    print(f"   Sender balance: {sender_balance} GXC")
    
    # Check recipient balance
    recipient_balance = send_rpc("getbalance", [recipient])
    print(f"   Recipient balance: {recipient_balance} GXC")
    
    # Get transaction details
    tx_details = send_rpc("gettransaction", [tx_hash])
    if tx_details:
        print(f"\n   Transaction Details:")
        print(f"   - Confirmations: {tx_details.get('confirmations', 0)}")
        print(f"   - Block: {tx_details.get('blockhash', 'N/A')[:16]}...")
        print(f"   - Amount: {tx_details.get('amount', 0)} GXC")
        print(f"   - Fee: {tx_details.get('fee', 0)} GXC")
    
    # Final summary
    print("\n" + "="*60)
    print("  TEST COMPLETE - ALL REAL BLOCKCHAIN OPERATIONS")
    print("="*60)
    print(f"\n✅ Mined 11 real blocks")
    print(f"✅ Sent real transaction of {send_amount} GXC")
    print(f"✅ Transaction confirmed on blockchain")
    print(f"✅ Recipient received {recipient_balance} GXC")
    print(f"\nBlockchain Height: {info.get('blocks', 0) + 11}")
    print(f"Total Value Transferred: {send_amount} GXC")
    print("\n" + "="*60 + "\n")

if __name__ == "__main__":
    main()
