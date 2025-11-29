#!/usr/bin/env python3
"""
GXC Testnet Blockchain Node
A Python implementation of the blockchain node RPC server for testnet
Provides getblocktemplate, submitblock, and other RPC methods for mining pools
"""

import json
import sqlite3
import hashlib
import time
import threading
from datetime import datetime
from http.server import HTTPServer, BaseHTTPRequestHandler
from urllib.parse import urlparse
import os

# Testnet configuration
# Railway provides PORT env var dynamically, default to 18332 for local
TESTNET_RPC_PORT = 18332  # Default port (Railway will override via PORT env var)
TESTNET_DATA_DIR = os.environ.get('GXC_DATA_DIR', "./gxc_testnet_data")
BLOCK_TIME = 60  # 60 seconds for testnet (vs 150 for mainnet)
INITIAL_DIFFICULTY = 0.1  # Lower difficulty for testnet
BLOCK_REWARD = 12.5

class Blockchain:
    def __init__(self, data_dir):
        self.data_dir = data_dir
        os.makedirs(data_dir, exist_ok=True)
        self.db_path = os.path.join(data_dir, "blockchain.db")
        self.init_database()
        self.current_height = self.get_height()
        self.current_difficulty = INITIAL_DIFFICULTY
        self.mining_thread = None
        self.running = True
        
    def init_database(self):
        """Initialize blockchain database"""
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        
        # Blocks table
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS blocks (
                height INTEGER PRIMARY KEY,
                hash TEXT UNIQUE NOT NULL,
                prev_hash TEXT NOT NULL,
                timestamp INTEGER NOT NULL,
                difficulty REAL NOT NULL,
                nonce INTEGER DEFAULT 0,
                miner TEXT,
                transaction_count INTEGER DEFAULT 0,
                merkle_root TEXT,
                size INTEGER DEFAULT 0
            )
        ''')
        
        # Transactions table
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS transactions (
                tx_hash TEXT PRIMARY KEY,
                block_height INTEGER,
                from_address TEXT,
                to_address TEXT,
                amount REAL NOT NULL,
                fee REAL NOT NULL,
                timestamp INTEGER NOT NULL,
                FOREIGN KEY (block_height) REFERENCES blocks (height)
            )
        ''')
        
        # Create genesis block if needed
        if cursor.execute('SELECT COUNT(*) FROM blocks').fetchone()[0] == 0:
            self.create_genesis_block(cursor)
        
        conn.commit()
        conn.close()
    
    def create_genesis_block(self, cursor):
        """Create genesis block"""
        genesis_hash = hashlib.sha256(b"GXC Testnet Genesis Block").hexdigest()
        cursor.execute('''
            INSERT INTO blocks (height, hash, prev_hash, timestamp, difficulty, miner, transaction_count)
            VALUES (0, ?, ?, ?, ?, ?, ?)
        ''', (genesis_hash, "0" * 64, int(time.time()), INITIAL_DIFFICULTY, "genesis", 0))
    
    def get_height(self):
        """Get current blockchain height"""
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        result = cursor.execute('SELECT MAX(height) FROM blocks').fetchone()[0] or 0
        conn.close()
        return result
    
    def get_latest_block(self):
        """Get latest block"""
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        block = cursor.execute('''
            SELECT height, hash, prev_hash, timestamp, difficulty, nonce, miner
            FROM blocks ORDER BY height DESC LIMIT 1
        ''').fetchone()
        conn.close()
        
        if block:
            return {
                'height': block[0],
                'hash': block[1],
                'prev_hash': block[2],
                'timestamp': block[3],
                'difficulty': block[4],
                'nonce': block[5],
                'miner': block[6]
            }
        return None
    
    def get_block_template(self, algorithm='sha256'):
        """Get block template for mining"""
        latest = self.get_latest_block()
        if not latest:
            return None
        
        # Calculate target difficulty
        target = int(2 ** (256 - self.current_difficulty))
        target_hex = hex(target)[2:].zfill(64)
        
        # Create block template
        template = {
            'version': 1,
            'previousblockhash': latest['hash'],
            'transactions': [],
            'coinbasevalue': int(BLOCK_REWARD * 100000000),  # In satoshis
            'target': target_hex,
            'mintime': int(time.time()),
            'maxtime': int(time.time()) + 3600,
            'curtime': int(time.time()),
            'bits': self.difficulty_to_bits(self.current_difficulty),
            'height': latest['height'] + 1,
            'algorithm': algorithm
        }
        
        return template
    
    def difficulty_to_bits(self, difficulty):
        """Convert difficulty to bits format"""
        # Simplified conversion
        target = int(2 ** (256 - difficulty))
        return hex(target)[2:10]
    
    def submit_block(self, block_data):
        """Submit a mined block"""
        try:
            # Validate block
            if not self.validate_block(block_data):
                return False
            
            # Add block to chain
            conn = sqlite3.connect(self.db_path)
            cursor = conn.cursor()
            
            block_hash = block_data.get('hash', '')
            prev_hash = block_data.get('previousblockhash', '')
            height = block_data.get('height', self.current_height + 1)
            nonce = block_data.get('nonce', 0)
            miner = block_data.get('miner', 'unknown')
            
            cursor.execute('''
                INSERT INTO blocks (height, hash, prev_hash, timestamp, difficulty, nonce, miner, transaction_count)
                VALUES (?, ?, ?, ?, ?, ?, ?, ?)
            ''', (height, block_hash, prev_hash, int(time.time()), self.current_difficulty, nonce, miner, 0))
            
            conn.commit()
            conn.close()
            
            self.current_height = height
            print(f"[BLOCKCHAIN] Block #{height} accepted! Hash: {block_hash[:16]}...")
            
            # Adjust difficulty (simplified)
            if height % 10 == 0:
                self.current_difficulty = min(self.current_difficulty * 1.1, 100.0)
            
            return True
        except Exception as e:
            print(f"[BLOCKCHAIN] Error submitting block: {e}")
            return False
    
    def validate_block(self, block_data):
        """Validate block structure"""
        required_fields = ['hash', 'previousblockhash', 'height', 'nonce']
        for field in required_fields:
            if field not in block_data:
                return False
        return True
    
    def get_blockchain_info(self):
        """Get blockchain information"""
        latest = self.get_latest_block()
        return {
            'chain': 'testnet',
            'blocks': self.current_height,
            'headers': self.current_height,
            'bestblockhash': latest['hash'] if latest else '0' * 64,
            'difficulty': self.current_difficulty,
            'mediantime': int(time.time()),
            'verificationprogress': 1.0,
            'chainwork': hex(self.current_height)[2:],
            'pruned': False
        }

class RPCHandler(BaseHTTPRequestHandler):
    blockchain = None
    
    def do_POST(self):
        """Handle RPC POST requests"""
        content_length = int(self.headers['Content-Length'])
        post_data = self.rfile.read(content_length)
        
        try:
            request = json.loads(post_data.decode('utf-8'))
            method = request.get('method', '')
            params = request.get('params', [])
            request_id = request.get('id', 1)
            
            # Handle RPC methods
            result = None
            error = None
            
            if method == 'getblockchaininfo':
                result = self.blockchain.get_blockchain_info()
            elif method == 'getblocktemplate' or method == 'gxc_getBlockTemplate':
                algorithm = params[0].get('algorithm', 'sha256') if params and isinstance(params[0], dict) else 'sha256'
                result = self.blockchain.get_block_template(algorithm)
            elif method == 'submitblock' or method == 'gxc_submitBlock':
                block_data = params[0] if params else {}
                success = self.blockchain.submit_block(block_data)
                if success:
                    result = {
                        'success': True,
                        'height': block_data.get('height'),
                        'hash': block_data.get('hash'),
                        'reward': BLOCK_REWARD
                    }
                else:
                    result = "Block rejected"
            elif method == 'getbestblockhash':
                latest = self.blockchain.get_latest_block()
                result = latest['hash'] if latest else '0' * 64
            elif method == 'getblockcount':
                result = self.blockchain.current_height
            elif method == 'getdifficulty':
                result = self.blockchain.current_difficulty
            elif method == 'getblock':
                # Get block by hash or height
                if params and len(params) > 0:
                    block_param = params[0]
                    if isinstance(block_param, int) or (isinstance(block_param, str) and block_param.isdigit()):
                        # Get block by height
                        height = int(block_param)
                        latest = self.blockchain.get_latest_block()
                        if latest and latest['height'] == height:
                            result = latest
                        else:
                            result = None
                    else:
                        # Get block by hash - simplified, return latest if hash matches
                        latest = self.blockchain.get_latest_block()
                        if latest and latest['hash'] == block_param:
                            result = latest
                        else:
                            result = None
                else:
                    # No params, return latest
                    latest = self.blockchain.get_latest_block()
                    result = latest if latest else None
            elif method == 'gxc_getLatestBlock' or method == 'getlatestblock':
                # Get latest block (for explorer)
                latest = self.blockchain.get_latest_block()
                if latest:
                    result = {
                        'height': latest['height'],
                        'hash': latest['hash'],
                        'prev_hash': latest['prev_hash'],
                        'timestamp': latest['timestamp'],
                        'difficulty': latest['difficulty'],
                        'nonce': latest['nonce'],
                        'miner': latest['miner'],
                        'transaction_count': 0
                    }
                else:
                    result = None
            elif method == 'gxc_getBlockByNumber' or method == 'getblockbynumber':
                # Get block by number (for explorer)
                block_number = params[0] if params and len(params) > 0 else None
                if block_number is not None:
                    latest = self.blockchain.get_latest_block()
                    if latest and latest['height'] == int(block_number):
                        result = {
                            'height': latest['height'],
                            'hash': latest['hash'],
                            'prev_hash': latest['prev_hash'],
                            'timestamp': latest['timestamp'],
                            'difficulty': latest['difficulty'],
                            'nonce': latest['nonce'],
                            'miner': latest['miner'],
                            'transaction_count': 0,
                            'transactions': []
                        }
                    else:
                        result = None
                else:
                    result = None
            else:
                error = {'code': -32601, 'message': f'Method not found: {method}'}
            
            # Send response
            response = {
                'jsonrpc': '2.0',
                'id': request_id
            }
            
            if error:
                response['error'] = error
            else:
                response['result'] = result
            
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            self.wfile.write(json.dumps(response).encode('utf-8'))
            
        except Exception as e:
            error_response = {
                'jsonrpc': '2.0',
                'id': request.get('id', 1) if 'request' in locals() else 1,
                'error': {'code': -32700, 'message': f'Parse error: {str(e)}'}
            }
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            self.wfile.write(json.dumps(error_response).encode('utf-8'))
    
    def log_message(self, format, *args):
        """Override to reduce logging noise"""
        pass

def main():
    print("=" * 60)
    print("GXC Testnet Blockchain Node")
    print("=" * 60)
    
    # Railway provides PORT env var, use it if available
    port = int(os.environ.get('PORT', TESTNET_RPC_PORT))
    print(f"RPC Port: {port}")
    print(f"Data Directory: {TESTNET_DATA_DIR}")
    print(f"Block Time: {BLOCK_TIME} seconds")
    print(f"Initial Difficulty: {INITIAL_DIFFICULTY}")
    print(f"Environment: {'Railway' if os.environ.get('RAILWAY_ENVIRONMENT') else 'Local'}")
    print("=" * 60)
    print()
    
    try:
        # Initialize blockchain
        blockchain = Blockchain(TESTNET_DATA_DIR)
        RPCHandler.blockchain = blockchain
        
        # Start RPC server
        # Always bind to 0.0.0.0 for better compatibility (accessible via localhost and 127.0.0.1)
        host = '0.0.0.0'
        server_address = (host, port)
        httpd = HTTPServer(server_address, RPCHandler)
        
        if os.environ.get('RAILWAY_ENVIRONMENT') or os.environ.get('PORT'):
            print(f"[BLOCKCHAIN] RPC server started on http://0.0.0.0:{port} (Railway)")
        else:
            print(f"[BLOCKCHAIN] RPC server started on http://localhost:{port}")
            print(f"[BLOCKCHAIN] Accessible at: http://localhost:{port} or http://127.0.0.1:{port}")
        print(f"[BLOCKCHAIN] Current height: {blockchain.current_height}")
        print(f"[BLOCKCHAIN] Current difficulty: {blockchain.current_difficulty}")
        print()
        print("Ready to accept RPC requests from mining pools!")
        print("Press Ctrl+C to stop")
        print()
        
        httpd.serve_forever()
    except OSError as e:
        if "Address already in use" in str(e):
            print(f"[ERROR] Port {port} is already in use. Please use a different port.")
        else:
            print(f"[ERROR] Failed to start server: {e}")
        raise
    except KeyboardInterrupt:
        print("\n[BLOCKCHAIN] Shutting down...")
        if 'httpd' in locals():
            httpd.shutdown()
        print("[BLOCKCHAIN] Node stopped")
    except Exception as e:
        print(f"[ERROR] Fatal error: {e}")
        import traceback
        traceback.print_exc()
        raise

if __name__ == '__main__':
    main()

