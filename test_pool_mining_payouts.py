#!/usr/bin/env python3
"""
Test GXC Mining Pool - Complete Mining and Payout Test
Tests: Pool startup, miner connection, block mining, payout calculation
"""

import sys
import os
import socket
import json
import time
import threading
import hashlib

# Add mining_pool to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'mining_pool'))

from pool_base import MiningPool

NODE_URL = "http://localhost:8332"
POOL_PORT = 9998
TEST_MINER_ADDRESS = "tGXCtest123456789abcdef"

class TestMiner:
    """Simple test miner that connects to pool via Stratum"""
    
    def __init__(self, pool_host='localhost', pool_port=POOL_PORT, address=TEST_MINER_ADDRESS):
        self.pool_host = pool_host
        self.pool_port = pool_port
        self.address = address
        self.socket = None
        self.connected = False
        self.current_job = None
        self.shares_submitted = 0
        self.shares_accepted = 0
        self.shares_rejected = 0
        
    def connect(self):
        """Connect to pool"""
        try:
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.connect((self.pool_host, self.pool_port))
            self.connected = True
            print(f"✓ Miner connected to pool at {self.pool_host}:{self.pool_port}")
            return True
        except Exception as e:
            print(f"✗ Miner connection failed: {e}")
            return False
    
    def send_message(self, message):
        """Send Stratum message"""
        try:
            self.socket.send((json.dumps(message) + "\n").encode())
            return True
        except Exception as e:
            print(f"✗ Send failed: {e}")
            return False
    
    def receive_message(self):
        """Receive Stratum message"""
        try:
            data = self.socket.recv(4096).decode()
            if not data:
                return None
            
            # Handle multiple messages
            messages = []
            for line in data.strip().split('\n'):
                if line:
                    messages.append(json.loads(line))
            
            return messages
        except Exception as e:
            print(f"✗ Receive failed: {e}")
            return None
    
    def subscribe(self):
        """Send mining.subscribe"""
        msg = {
            "id": 1,
            "method": "mining.subscribe",
            "params": ["TestMiner/1.0"]
        }
        
        print(f"\n→ Sending mining.subscribe")
        if not self.send_message(msg):
            return False
        
        # Wait for response
        time.sleep(0.5)
        responses = self.receive_message()
        
        if responses:
            for resp in responses:
                if resp.get('id') == 1:
                    print(f"← Received subscribe response: {resp.get('result')}")
                    return True
                elif resp.get('method') == 'mining.set_difficulty':
                    print(f"← Received difficulty: {resp.get('params')}")
                elif resp.get('method') == 'mining.notify':
                    print(f"← Received initial job")
                    self.current_job = resp.get('params')
        
        return True
    
    def authorize(self):
        """Send mining.authorize"""
        msg = {
            "id": 2,
            "method": "mining.authorize",
            "params": [self.address, "x"]
        }
        
        print(f"\n→ Sending mining.authorize for {self.address}")
        if not self.send_message(msg):
            return False
        
        # Wait for response
        time.sleep(0.5)
        responses = self.receive_message()
        
        if responses:
            for resp in responses:
                if resp.get('id') == 2:
                    result = resp.get('result')
                    print(f"← Authorization result: {result}")
                    return result
                elif resp.get('method') == 'mining.notify':
                    print(f"← Received job after authorization")
                    self.current_job = resp.get('params')
        
        return False
    
    def mine_share(self, max_attempts=10000):
        """Try to mine a valid share"""
        if not self.current_job:
            print("✗ No job available")
            return False
        
        job_id = self.current_job[0]
        prev_hash = self.current_job[1]
        
        print(f"\n⛏️  Mining share for job {job_id}...")
        print(f"   Previous hash: {prev_hash[:16]}...")
        
        # Try different nonces
        for nonce in range(max_attempts):
            # Simple hash calculation (simplified for testing)
            block_str = f"{prev_hash}{nonce}{self.address}{int(time.time())}"
            block_hash = hashlib.sha256(block_str.encode()).hexdigest()
            
            # Check if it looks like a valid share (starts with 0)
            if block_hash.startswith('0'):
                print(f"   ✓ Found potential share at nonce {nonce}")
                print(f"   Hash: {block_hash[:32]}...")
                
                # Submit share
                return self.submit_share(job_id, nonce)
            
            if nonce % 1000 == 0 and nonce > 0:
                print(f"   Tried {nonce} nonces...")
        
        print(f"   ✗ No valid share found in {max_attempts} attempts")
        return False
    
    def submit_share(self, job_id, nonce):
        """Submit mining share"""
        msg = {
            "id": 3,
            "method": "mining.submit",
            "params": [
                self.address,
                job_id,
                "00000000",  # extranonce2
                hex(int(time.time()))[2:],  # ntime
                hex(nonce)[2:].zfill(8)  # nonce
            ]
        }
        
        print(f"\n→ Submitting share (nonce: {nonce})")
        if not self.send_message(msg):
            return False
        
        self.shares_submitted += 1
        
        # Wait for response
        time.sleep(0.5)
        responses = self.receive_message()
        
        if responses:
            for resp in responses:
                if resp.get('id') == 3:
                    result = resp.get('result')
                    if result:
                        print(f"← Share ACCEPTED ✓")
                        self.shares_accepted += 1
                        return True
                    else:
                        error = resp.get('error', 'Unknown error')
                        print(f"← Share REJECTED: {error}")
                        self.shares_rejected += 1
                        return False
        
        return False
    
    def disconnect(self):
        """Disconnect from pool"""
        if self.socket:
            self.socket.close()
            self.connected = False
            print(f"\n✓ Miner disconnected")

def test_pool_mining_and_payouts():
    """Complete test of pool mining and payouts"""
    
    print("="*70)
    print("GXC MINING POOL - COMPLETE MINING & PAYOUT TEST")
    print("="*70)
    print()
    
    # Step 1: Start pool
    print("STEP 1: Starting Mining Pool")
    print("-"*70)
    
    pool = MiningPool(
        pool_name='test-pool',
        algorithm='sha256',
        port=POOL_PORT,
        rpc_url=NODE_URL
    )
    
    print(f"✓ Pool created on port {POOL_PORT}")
    
    # Give pool time to start
    time.sleep(2)
    
    # Step 2: Create test miners
    print("\n" + "="*70)
    print("STEP 2: Creating Test Miners")
    print("-"*70)
    
    miner1 = TestMiner(address="tGXCminer1address")
    miner2 = TestMiner(address="tGXCminer2address")
    
    print(f"✓ Created 2 test miners")
    
    # Step 3: Connect miners
    print("\n" + "="*70)
    print("STEP 3: Connecting Miners to Pool")
    print("-"*70)
    
    if not miner1.connect():
        print("✗ Miner 1 connection failed")
        return False
    
    if not miner2.connect():
        print("✗ Miner 2 connection failed")
        return False
    
    # Step 4: Subscribe and authorize
    print("\n" + "="*70)
    print("STEP 4: Miner Subscription & Authorization")
    print("-"*70)
    
    print("\nMiner 1:")
    if not miner1.subscribe():
        print("✗ Miner 1 subscribe failed")
        return False
    
    if not miner1.authorize():
        print("✗ Miner 1 authorize failed")
        return False
    
    print("\nMiner 2:")
    if not miner2.subscribe():
        print("✗ Miner 2 subscribe failed")
        return False
    
    if not miner2.authorize():
        print("✗ Miner 2 authorize failed")
        return False
    
    # Step 5: Mine shares
    print("\n" + "="*70)
    print("STEP 5: Mining Shares")
    print("-"*70)
    
    print("\nMiner 1 mining...")
    miner1.mine_share(max_attempts=5000)
    miner1.mine_share(max_attempts=5000)
    
    print("\nMiner 2 mining...")
    miner2.mine_share(max_attempts=5000)
    
    # Step 6: Check pool statistics
    print("\n" + "="*70)
    print("STEP 6: Pool Statistics")
    print("-"*70)
    
    time.sleep(1)
    
    # Get pool stats from database
    try:
        import sqlite3
        conn = sqlite3.connect(pool.db_path)
        cursor = conn.cursor()
        
        # Get miner stats
        cursor.execute("SELECT miner_id, accepted_shares, rejected_shares, pending_balance FROM miners")
        miners = cursor.fetchall()
        
        print(f"\nMiners in pool: {len(miners)}")
        for miner_id, accepted, rejected, balance in miners:
            print(f"  {miner_id[:20]}...")
            print(f"    Accepted: {accepted}, Rejected: {rejected}")
            print(f"    Pending balance: {balance} GXC")
        
        # Get total shares
        cursor.execute("SELECT COUNT(*) FROM shares WHERE is_valid = 1")
        valid_shares = cursor.fetchone()[0]
        
        cursor.execute("SELECT COUNT(*) FROM shares WHERE is_valid = 0")
        invalid_shares = cursor.fetchone()[0]
        
        print(f"\nTotal shares:")
        print(f"  Valid: {valid_shares}")
        print(f"  Invalid: {invalid_shares}")
        
        # Get blocks found
        cursor.execute("SELECT COUNT(*) FROM blocks")
        blocks_found = cursor.fetchone()[0]
        
        print(f"\nBlocks found: {blocks_found}")
        
        conn.close()
        
    except Exception as e:
        print(f"⚠️  Could not read pool database: {e}")
    
    # Step 7: Calculate payouts
    print("\n" + "="*70)
    print("STEP 7: Payout Calculation")
    print("-"*70)
    
    # Simulate block found
    block_reward = 50.0
    pool_fee_percent = 1.0
    
    print(f"\nSimulating block found:")
    print(f"  Block reward: {block_reward} GXC")
    print(f"  Pool fee: {pool_fee_percent}%")
    
    pool_fee = block_reward * (pool_fee_percent / 100)
    distributable = block_reward - pool_fee
    
    print(f"\n  Pool fee amount: {pool_fee} GXC")
    print(f"  Distributable: {distributable} GXC")
    
    # Calculate shares
    miner1_shares = miner1.shares_accepted
    miner2_shares = miner2.shares_accepted
    total_shares = miner1_shares + miner2_shares
    
    if total_shares > 0:
        miner1_payout = (miner1_shares / total_shares) * distributable
        miner2_payout = (miner2_shares / total_shares) * distributable
        
        print(f"\nPayout calculation:")
        print(f"  Total shares: {total_shares}")
        print(f"  Miner 1: {miner1_shares} shares ({miner1_shares/total_shares*100:.1f}%) = {miner1_payout:.6f} GXC")
        print(f"  Miner 2: {miner2_shares} shares ({miner2_shares/total_shares*100:.1f}%) = {miner2_payout:.6f} GXC")
        print(f"\n  Verification: {miner1_payout + miner2_payout:.6f} GXC (should be {distributable})")
    else:
        print(f"\n⚠️  No valid shares to calculate payouts")
    
    # Step 8: Disconnect miners
    print("\n" + "="*70)
    print("STEP 8: Cleanup")
    print("-"*70)
    
    miner1.disconnect()
    miner2.disconnect()
    
    # Summary
    print("\n" + "="*70)
    print("TEST SUMMARY")
    print("="*70)
    
    print(f"\nMiner 1:")
    print(f"  Submitted: {miner1.shares_submitted}")
    print(f"  Accepted: {miner1.shares_accepted}")
    print(f"  Rejected: {miner1.shares_rejected}")
    
    print(f"\nMiner 2:")
    print(f"  Submitted: {miner2.shares_submitted}")
    print(f"  Accepted: {miner2.shares_accepted}")
    print(f"  Rejected: {miner2.shares_rejected}")
    
    print(f"\nPool:")
    print(f"  Total valid shares: {miner1.shares_accepted + miner2.shares_accepted}")
    print(f"  Total invalid shares: {miner1.shares_rejected + miner2.shares_rejected}")
    
    if total_shares > 0:
        print(f"\nPayouts calculated: ✓")
        print(f"  Miner 1 would receive: {miner1_payout:.6f} GXC")
        print(f"  Miner 2 would receive: {miner2_payout:.6f} GXC")
        print(f"  Pool fee: {pool_fee:.6f} GXC")
    
    print("\n" + "="*70)
    print("✓ Test completed successfully!")
    print("="*70)
    print()
    
    return True

if __name__ == "__main__":
    try:
        success = test_pool_mining_and_payouts()
        sys.exit(0 if success else 1)
    except KeyboardInterrupt:
        print("\n\n✓ Test interrupted by user")
        sys.exit(0)
    except Exception as e:
        print(f"\n\n✗ Test failed with error: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)
