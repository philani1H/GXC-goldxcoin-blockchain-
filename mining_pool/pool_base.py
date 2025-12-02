#!/usr/bin/env python3
"""
GXC Mining Pool Base Class
Handles Stratum protocol, blockchain RPC connection, and pool management
"""

import os
import sys
import json
import sqlite3
import requests
import threading
import socket
import time
import secrets
from datetime import datetime
from typing import Dict, List, Optional, Tuple
from flask import Flask, render_template, jsonify, request, redirect, url_for
from flask_socketio import SocketIO, emit

# Import centralized network configuration
try:
    sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))
    from config.railway_config import get_rpc_url, get_network, get_network_config
    USE_CENTRAL_CONFIG = True
except ImportError:
    USE_CENTRAL_CONFIG = False

class MiningPool:
    """Base class for GXC mining pools"""
    
    def __init__(self, pool_name: str, algorithm: str, port: int, rpc_url: str = None):
        self.pool_name = pool_name
        self.algorithm = algorithm
        self.port = port
        
        # Use centralized network-aware configuration
        if USE_CENTRAL_CONFIG:
            self.rpc_url = rpc_url or get_rpc_url()
            self.network = get_network()
            self.network_config = get_network_config()
        else:
            # Fallback to environment variables
            railway_url = os.environ.get('RAILWAY_NODE_URL', 'https://gxc-chain112-blockchain-node-production.up.railway.app')
            self.rpc_url = rpc_url or os.environ.get('BLOCKCHAIN_NODE_URL', railway_url)
            self.network = os.environ.get('GXC_NETWORK', 'testnet')
        
        # Public endpoints for third-party miners
        self.stratum_url = os.environ.get(
            f'{pool_name.upper().replace("-", "_")}_STRATUM_URL',
            os.environ.get('POOL_STRATUM_URL', f'stratum+tcp://{pool_name}.gxc.network:{port}')
        )
        self.dashboard_url = os.environ.get(
            f'{pool_name.upper().replace("-", "_")}_DASHBOARD_URL',
            os.environ.get('POOL_DASHBOARD_URL', f'https://{pool_name}.gxc.network')
        )
        
        # Pool state
        self.miners = {}  # miner_id -> MinerInfo
        self.current_job = None
        self.job_id = 1
        self.difficulty = 1000.0
        self.blocks_found = 0
        self.total_shares = 0
        self.accepted_shares = 0
        self.rejected_shares = 0
        
        # Stratum server
        self.stratum_socket = None
        self.stratum_running = False
        self.stratum_thread = None
        
        # Database path
        db_path = os.environ.get('DATABASE_PATH', f'/tmp/{pool_name}.db' if os.path.exists('/tmp') else f'{pool_name}.db')
        self.db_path = db_path
        
        # Flask app - use relative paths (api.py changes directory)
        self.app = Flask(__name__, template_folder='templates', static_folder='static')
        self.app.config['SECRET_KEY'] = secrets.token_hex(32)
        
        # Detect if running on Vercel (serverless)
        vercel_env = os.environ.get('VERCEL', '0')
        vercel_url = os.environ.get('VERCEL_URL', '').lower()
        self.is_vercel = vercel_env == '1' or 'vercel' in vercel_url or 'VERCEL' in os.environ
        
        # SocketIO - only use if not on Vercel (serverless doesn't support WebSockets well)
        if not self.is_vercel:
            try:
                self.socketio = SocketIO(self.app, cors_allowed_origins="*", async_mode='threading')
            except:
                self.socketio = None
        else:
            # For Vercel, create a dummy socketio that won't be used
            self.socketio = None
        
        self.setup_routes()
        
        # Initialize database after Flask app is created
        try:
            self.init_database()
        except Exception as e:
            print(f"Warning: Database initialization failed: {e}")
        
        # Only start Stratum server if not on Vercel (serverless can't bind sockets)
        if not self.is_vercel:
            try:
                self.start_stratum_server()
            except Exception as e:
                print(f"Warning: Could not start Stratum server: {e}")
        
        # Start background tasks
        self.start_background_tasks()
    
    def init_database(self):
        """Initialize pool database"""
        try:
            conn = sqlite3.connect(self.db_path)
            cursor = conn.cursor()
        except Exception as e:
            print(f"Warning: Could not connect to database at {self.db_path}: {e}")
            # Try to use /tmp if available
            if not self.db_path.startswith('/tmp'):
                try:
                    self.db_path = f'/tmp/{self.pool_name}.db'
                    conn = sqlite3.connect(self.db_path)
                    cursor = conn.cursor()
                    print(f"Using fallback database path: {self.db_path}")
                except:
                    # If all else fails, use in-memory database
                    self.db_path = ':memory:'
                    conn = sqlite3.connect(self.db_path)
                    cursor = conn.cursor()
                    print(f"Using in-memory database")
        
        # Miners table
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS miners (
                miner_id TEXT PRIMARY KEY,
                username TEXT NOT NULL,
                address TEXT,
                algorithm TEXT NOT NULL,
                connected_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                last_seen TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                total_shares INTEGER DEFAULT 0,
                accepted_shares INTEGER DEFAULT 0,
                rejected_shares INTEGER DEFAULT 0,
                hash_rate REAL DEFAULT 0.0,
                difficulty REAL DEFAULT 0.0,
                pending_balance REAL DEFAULT 0.0,
                is_active BOOLEAN DEFAULT 1
            )
        ''')
        
        # Shares table
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS shares (
                share_id INTEGER PRIMARY KEY AUTOINCREMENT,
                miner_id TEXT NOT NULL,
                job_id TEXT NOT NULL,
                nonce TEXT NOT NULL,
                extra_nonce2 TEXT,
                difficulty REAL NOT NULL,
                is_valid BOOLEAN NOT NULL,
                is_block BOOLEAN DEFAULT 0,
                block_hash TEXT,
                submitted_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (miner_id) REFERENCES miners (miner_id)
            )
        ''')
        
        # Blocks table
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS blocks (
                block_id INTEGER PRIMARY KEY AUTOINCREMENT,
                block_hash TEXT UNIQUE NOT NULL,
                block_number INTEGER NOT NULL,
                miner_id TEXT,
                job_id TEXT,
                found_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                confirmed BOOLEAN DEFAULT 0,
                reward REAL DEFAULT 0.0,
                FOREIGN KEY (miner_id) REFERENCES miners (miner_id)
            )
        ''')
        
        # Payouts table
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS payouts (
                payout_id INTEGER PRIMARY KEY AUTOINCREMENT,
                miner_id TEXT NOT NULL,
                amount REAL NOT NULL,
                address TEXT NOT NULL,
                status TEXT DEFAULT 'pending',
                tx_hash TEXT,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                paid_at TIMESTAMP,
                FOREIGN KEY (miner_id) REFERENCES miners (miner_id)
            )
        ''')
        
        try:
            conn.commit()
        except Exception as e:
            print(f"Warning: Could not commit database changes: {e}")
        finally:
            try:
                conn.close()
            except:
                pass
    
    def get_blockchain_info(self) -> Optional[Dict]:
        """Get blockchain info from RPC"""
        try:
            # Try standard RPC method first, then GXC-specific
            for method in ['getblockchaininfo', 'gxc_getBlockchainInfo']:
                response = requests.post(self.rpc_url, json={
                    'jsonrpc': '2.0',
                    'method': method,
                    'params': [],
                    'id': 1
                }, timeout=10, headers={'Content-Type': 'application/json'})
                
                if response.status_code == 200:
                    data = response.json()
                    if 'error' in data:
                        continue  # Try next method
                    result = data.get('result')
                    if result:
                        return result
            return None
        except Exception as e:
            print(f"[{self.pool_name}] Error getting blockchain info: {e}")
            return None
    
    def get_block_template(self) -> Optional[Dict]:
        """Get block template from blockchain RPC"""
        try:
            # Try standard RPC method first, then GXC-specific
            for method in ['getblocktemplate', 'gxc_getBlockTemplate']:
                params = [{'algorithm': self.algorithm}] if method == 'gxc_getBlockTemplate' else []
                response = requests.post(self.rpc_url, json={
                    'jsonrpc': '2.0',
                    'method': method,
                    'params': params,
                    'id': 1
                }, timeout=10, headers={'Content-Type': 'application/json'})
                
                if response.status_code == 200:
                    data = response.json()
                    if 'error' in data:
                        continue  # Try next method
                    result = data.get('result')
                    if result:
                        return result
            return None
        except Exception as e:
            print(f"[{self.pool_name}] Error getting block template: {e}")
            return None
    
    def submit_block(self, block_data: Dict) -> bool:
        """Submit block to blockchain"""
        try:
            # Try standard RPC method first, then GXC-specific
            for method in ['submitblock', 'gxc_submitBlock']:
                response = requests.post(self.rpc_url, json={
                    'jsonrpc': '2.0',
                    'method': method,
                    'params': [block_data],
                    'id': 1
                }, timeout=10, headers={'Content-Type': 'application/json'})
                
                if response.status_code == 200:
                    data = response.json()
                    if 'error' in data:
                        error_msg = data['error']
                        if isinstance(error_msg, dict):
                            error_msg = error_msg.get('message', str(error_msg))
                        print(f"[{self.pool_name}] Block submission error: {error_msg}")
                        continue
                    result = data.get('result')
                    # submitblock returns null on success, or error string on failure
                    if result is None or result is True:
                        return True
                    elif isinstance(result, str) and result.lower() in ['null', '']:
                        return True
            return False
        except Exception as e:
            print(f"[{self.pool_name}] Error submitting block: {e}")
            return False
    
    def generate_mining_job(self) -> Dict:
        """Generate new mining job"""
        template = self.get_block_template()
        
        if not template:
            # Fallback job if RPC unavailable
            return {
                'job_id': str(self.job_id),
                'prev_block_hash': '0' * 64,
                'merkle_root': '0' * 64,
                'timestamp': int(time.time()),
                'difficulty': self.difficulty,
                'height': 0
            }
        
        self.job_id += 1
        job = {
            'job_id': str(self.job_id),
            'prev_block_hash': template.get('previousblockhash', '0' * 64),
            'merkle_root': template.get('merkleroot', '0' * 64),
            'timestamp': template.get('curtime', int(time.time())),
            'difficulty': float(template.get('difficulty', self.difficulty)),
            'height': int(template.get('height', 0)),
            'transactions': template.get('transactions', [])
        }
        
        self.current_job = job
        return job
    
    def validate_share(self, miner_id: str, job_id: str, nonce: str, extra_nonce2: str) -> Tuple[bool, bool]:
        """
        Validate share
        Returns: (is_valid_share, is_block_solution)
        """
        if not self.current_job or self.current_job['job_id'] != job_id:
            return False, False
        
        # Basic validation
        if not nonce or not extra_nonce2:
            return False, False
        
        # Construct block header
        block_header = (
            self.current_job['prev_block_hash'] +
            self.current_job['merkle_root'] +
            str(self.current_job['timestamp']) +
            str(self.current_job['difficulty']) +
            nonce +
            extra_nonce2
        )
        
        # Hash block header (simplified - would use actual algorithm)
        import hashlib
        block_hash = hashlib.sha256(block_header.encode()).hexdigest()
        
        # Check if meets pool difficulty
        is_valid = self.check_difficulty(block_hash, self.difficulty)
        
        # Check if meets blockchain difficulty
        blockchain_difficulty = self.current_job.get('difficulty', self.difficulty * 10)
        is_block = self.check_difficulty(block_hash, blockchain_difficulty)
        
        return is_valid, is_block
    
    def check_difficulty(self, hash_str: str, difficulty: float) -> bool:
        """Check if hash meets difficulty requirement"""
        # Simplified difficulty check - count leading zeros
        leading_zeros = 0
        for char in hash_str:
            if char == '0':
                leading_zeros += 1
            else:
                break
        
        required_zeros = int(difficulty / 1000.0) + 4
        return leading_zeros >= required_zeros
    
    def start_stratum_server(self):
        """Start Stratum TCP server"""
        def stratum_loop():
            self.stratum_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.stratum_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            
            try:
                self.stratum_socket.bind(('0.0.0.0', self.port))
                self.stratum_socket.listen(100)
                self.stratum_running = True
                
                print(f"[{self.pool_name}] Stratum server listening on port {self.port}")
                
                while self.stratum_running:
                    try:
                        client_socket, address = self.stratum_socket.accept()
                        client_thread = threading.Thread(
                            target=self.handle_stratum_client,
                            args=(client_socket, address)
                        )
                        client_thread.daemon = True
                        client_thread.start()
                    except Exception as e:
                        if self.stratum_running:
                            print(f"Error accepting connection: {e}")
            except Exception as e:
                print(f"Error starting Stratum server: {e}")
        
        self.stratum_thread = threading.Thread(target=stratum_loop)
        self.stratum_thread.daemon = True
        self.stratum_thread.start()
    
    def handle_stratum_client(self, client_socket: socket.socket, address: Tuple):
        """Handle Stratum client connection"""
        miner_id = f"{address[0]}:{address[1]}"
        buffer = ""
        
        try:
            # Send welcome message
            welcome = {
                "id": None,
                "method": "mining.set_difficulty",
                "params": [self.difficulty]
            }
            client_socket.send((json.dumps(welcome) + "\n").encode())
            
            # Send initial job
            job = self.generate_mining_job()
            job_msg = {
                "id": None,
                "method": "mining.notify",
                "params": [
                    job['job_id'],
                    job['prev_block_hash'],
                    job['merkle_root'],
                    job['timestamp'],
                    hex(int(job['difficulty']))[2:]
                ]
            }
            client_socket.send((json.dumps(job_msg) + "\n").encode())
            
            # Add miner
            self.add_miner(miner_id, address[0])
            
            # Handle messages
            while self.stratum_running:
                data = client_socket.recv(4096)
                if not data:
                    break
                
                buffer += data.decode('utf-8', errors='ignore')
                lines = buffer.split('\n')
                buffer = lines[-1]
                
                for line in lines[:-1]:
                    if line.strip():
                        self.process_stratum_message(miner_id, line.strip(), client_socket)
        
        except Exception as e:
            print(f"Error handling client {miner_id}: {e}")
        finally:
            self.remove_miner(miner_id)
            client_socket.close()
    
    def process_stratum_message(self, miner_id: str, message: str, client_socket: socket.socket):
        """Process Stratum protocol message"""
        try:
            msg = json.loads(message)
            method = msg.get('method')
            params = msg.get('params', [])
            msg_id = msg.get('id')
            
            if method == 'mining.subscribe':
                # Miner subscription
                response = {
                    "id": msg_id,
                    "result": [
                        ["mining.notify", "mining.set_difficulty"],
                        "08000000"  # Extra nonce 1
                    ],
                    "error": None
                }
                client_socket.send((json.dumps(response) + "\n").encode())
            
            elif method == 'mining.authorize':
                # Miner authorization
                username = params[0] if len(params) > 0 else ""
                password = params[1] if len(params) > 1 else ""
                
                self.authorize_miner(miner_id, username)
                
                response = {
                    "id": msg_id,
                    "result": True,
                    "error": None
                }
                client_socket.send((json.dumps(response) + "\n").encode())
                
                # Send job
                job = self.generate_mining_job()
                job_msg = {
                    "id": None,
                    "method": "mining.notify",
                    "params": [
                        job['job_id'],
                        job['prev_block_hash'],
                        job['merkle_root'],
                        job['timestamp'],
                        hex(int(job['difficulty']))[2:]
                    ]
                }
                client_socket.send((json.dumps(job_msg) + "\n").encode())
            
            elif method == 'mining.submit':
                # Share submission
                username = params[0] if len(params) > 0 else ""
                job_id = params[1] if len(params) > 1 else ""
                extra_nonce2 = params[2] if len(params) > 2 else ""
                nonce = params[3] if len(params) > 3 else ""
                
                is_valid, is_block = self.validate_share(miner_id, job_id, nonce, extra_nonce2)
                
                if is_valid:
                    self.accept_share(miner_id, job_id, nonce, extra_nonce2, is_block)
                    response = {
                        "id": msg_id,
                        "result": True,
                        "error": None
                    }
                else:
                    self.reject_share(miner_id, job_id, nonce, extra_nonce2)
                    response = {
                        "id": msg_id,
                        "result": False,
                        "error": "Invalid share"
                    }
                
                client_socket.send((json.dumps(response) + "\n").encode())
        
        except Exception as e:
            print(f"Error processing message: {e}")
    
    def add_miner(self, miner_id: str, address: str):
        """Add miner to pool"""
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        
        cursor.execute('''
            INSERT OR REPLACE INTO miners (miner_id, address, algorithm, connected_at, last_seen, is_active)
            VALUES (?, ?, ?, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP, 1)
        ''', (miner_id, address, self.algorithm))
        
        try:
            conn.commit()
        except Exception as e:
            print(f"Warning: Could not commit database changes: {e}")
        finally:
            try:
                conn.close()
            except:
                pass
        
        self.miners[miner_id] = {
            'id': miner_id,
            'address': address,
            'connected_at': datetime.now(),
            'last_seen': datetime.now(),
            'shares': 0,
            'accepted': 0,
            'rejected': 0
        }
    
    def remove_miner(self, miner_id: str):
        """Remove miner from pool"""
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        
        cursor.execute('UPDATE miners SET is_active = 0 WHERE miner_id = ?', (miner_id,))
        try:
            conn.commit()
        except Exception as e:
            print(f"Warning: Could not commit database changes: {e}")
        finally:
            try:
                conn.close()
            except:
                pass
        
        if miner_id in self.miners:
            del self.miners[miner_id]
    
    def authorize_miner(self, miner_id: str, username: str):
        """Authorize miner"""
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        
        cursor.execute('UPDATE miners SET username = ? WHERE miner_id = ?', (username, miner_id))
        try:
            conn.commit()
        except Exception as e:
            print(f"Warning: Could not commit database changes: {e}")
        finally:
            try:
                conn.close()
            except:
                pass
        
        if miner_id in self.miners:
            self.miners[miner_id]['username'] = username
    
    def accept_share(self, miner_id: str, job_id: str, nonce: str, extra_nonce2: str, is_block: bool):
        """Accept share from miner"""
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        
        cursor.execute('''
            INSERT INTO shares (miner_id, job_id, nonce, extra_nonce2, difficulty, is_valid, is_block)
            VALUES (?, ?, ?, ?, ?, 1, ?)
        ''', (miner_id, job_id, nonce, extra_nonce2, self.difficulty, 1 if is_block else 0))
        
        cursor.execute('''
            UPDATE miners 
            SET total_shares = total_shares + 1,
                accepted_shares = accepted_shares + 1,
                last_seen = CURRENT_TIMESTAMP
            WHERE miner_id = ?
        ''', (miner_id,))
        
        try:
            conn.commit()
        except Exception as e:
            print(f"Warning: Could not commit database changes: {e}")
        finally:
            try:
                conn.close()
            except:
                pass
        
        self.accepted_shares += 1
        self.total_shares += 1
        
        if miner_id in self.miners:
            self.miners[miner_id]['shares'] += 1
            self.miners[miner_id]['accepted'] += 1
            self.miners[miner_id]['last_seen'] = datetime.now()
        
        if is_block:
            self.handle_block_found(miner_id, job_id, nonce, extra_nonce2)
            # Distribute rewards fairly when block is found
            self.distribute_block_rewards(block_hash=None)
    
    def reject_share(self, miner_id: str, job_id: str, nonce: str, extra_nonce2: str):
        """Reject share from miner"""
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        
        cursor.execute('''
            INSERT INTO shares (miner_id, job_id, nonce, extra_nonce2, difficulty, is_valid)
            VALUES (?, ?, ?, ?, ?, 0)
        ''', (miner_id, job_id, nonce, extra_nonce2, self.difficulty))
        
        cursor.execute('''
            UPDATE miners 
            SET total_shares = total_shares + 1,
                rejected_shares = rejected_shares + 1,
                last_seen = CURRENT_TIMESTAMP
            WHERE miner_id = ?
        ''', (miner_id,))
        
        try:
            conn.commit()
        except Exception as e:
            print(f"Warning: Could not commit database changes: {e}")
        finally:
            try:
                conn.close()
            except:
                pass
        
        self.rejected_shares += 1
        self.total_shares += 1
        
        if miner_id in self.miners:
            self.miners[miner_id]['shares'] += 1
            self.miners[miner_id]['rejected'] += 1
            self.miners[miner_id]['last_seen'] = datetime.now()
    
    def handle_block_found(self, miner_id: str, job_id: str, nonce: str, extra_nonce2: str):
        """Handle block found"""
        if not self.current_job:
            return
        
        # Construct block
        block_data = {
            'height': self.current_job['height'],
            'prev_block_hash': self.current_job['prev_block_hash'],
            'merkle_root': self.current_job['merkle_root'],
            'timestamp': self.current_job['timestamp'],
            'nonce': nonce,
            'extra_nonce2': extra_nonce2,
            'algorithm': self.algorithm
        }
        
        # Submit to blockchain
        if self.submit_block(block_data):
            self.blocks_found += 1
            
            conn = sqlite3.connect(self.db_path)
            cursor = conn.cursor()
            
            block_hash = f"{self.current_job['prev_block_hash']}{nonce}{extra_nonce2}"
            cursor.execute('''
                INSERT INTO blocks (block_hash, block_number, miner_id, job_id, confirmed, reward)
                VALUES (?, ?, ?, ?, 1, 12.5)
            ''', (block_hash[:64], self.current_job['height'], miner_id, job_id))
            
            conn.commit()
            conn.close()
            
            print(f"[{self.pool_name}] Block found by {miner_id}!")
    
    def setup_routes(self):
        """Setup Flask routes"""
        @self.app.route('/')
        def dashboard():
            stats = self.get_pool_stats()
            miners = self.get_miners_list()
            # Detect environment for docs URL
            is_vercel = os.environ.get('VERCEL', '0') == '1' or 'vercel' in os.environ.get('VERCEL_URL', '').lower()
            # Use DOCS_URL env var if set, otherwise use Vercel URL (works immediately)
            if os.environ.get('DOCS_URL'):
                docs_url = os.environ.get('DOCS_URL')
            elif is_vercel:
                # Use actual Vercel URL that works now
                docs_url = 'https://gxc-docs.vercel.app'
            else:
                # Use environment variable or default public URL
                docs_url = os.environ.get('DOCS_URL', 'https://gxc-docs.vercel.app')
            return render_template('pool_dashboard.html',
                                 pool_name=self.pool_name,
                                 algorithm=self.algorithm,
                                 stats=stats,
                                 miners=miners,
                                 pool_port=self.port,
                                 stratum_url=self.stratum_url,
                                 dashboard_url=self.dashboard_url,
                                 docs_url=docs_url)
        
        @self.app.route('/api/stats')
        def api_stats():
            return jsonify(self.get_pool_stats())
        
        @self.app.route('/api/miners')
        def api_miners():
            return jsonify(self.get_miners_list())
        
        @self.app.route('/api/blocks')
        def api_blocks():
            return jsonify(self.get_blocks_list())
        
        @self.app.route('/register', methods=['GET', 'POST'])
        def register_miner():
            """Miner registration page"""
            if request.method == 'POST':
                data = request.form
                username = data.get('username')
                address = data.get('address', '').strip()
                email = data.get('email', '')
                
                # Validate inputs
                if not username:
                    return render_template('register_miner.html', 
                                         pool_name=self.pool_name, 
                                         algorithm=self.algorithm,
                                         error='Username is required')
                
                if not address or not address.startswith('GXC'):
                    return render_template('register_miner.html', 
                                         pool_name=self.pool_name, 
                                         algorithm=self.algorithm,
                                         error='Valid GXC address is required (must start with GXC)')
                
                # Register miner in database
                conn = sqlite3.connect(self.db_path)
                cursor = conn.cursor()
                try:
                    # Check if username already exists
                    cursor.execute('SELECT miner_id FROM miners WHERE miner_id = ? OR username = ?', (username, username))
                    if cursor.fetchone():
                        conn.close()
                        return render_template('register_miner.html', 
                                             pool_name=self.pool_name, 
                                             algorithm=self.algorithm,
                                             error='Username already exists. Please choose another.')
                    
                    # Check if address already registered
                    cursor.execute('SELECT username FROM miners WHERE address = ?', (address,))
                    existing = cursor.fetchone()
                    if existing:
                        # Address already registered, redirect to that miner's dashboard
                        conn.close()
                        return redirect(f'/miner/{existing[0]}')
                    
                    # Insert new miner
                    cursor.execute('''
                        INSERT INTO miners (miner_id, username, address, algorithm, is_active, connected_at, last_seen)
                        VALUES (?, ?, ?, ?, 1, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP)
                    ''', (username, username, address, self.algorithm))
                    conn.commit()
                    conn.close()
                    
                    # Redirect to success page with registration info
                    return redirect(f'/register-success?username={username}&address={address}')
                except Exception as e:
                    conn.close()
                    return render_template('register_miner.html', 
                                         pool_name=self.pool_name, 
                                         algorithm=self.algorithm,
                                         error=f'Registration error: {str(e)}')
            
            return render_template('register_miner.html', pool_name=self.pool_name, algorithm=self.algorithm)
        
        @self.app.route('/register-success')
        def register_success():
            """Registration success page"""
            username = request.args.get('username', '')
            address = request.args.get('address', '')
            
            return render_template('register_success.html',
                                 pool_name=self.pool_name,
                                 algorithm=self.algorithm,
                                 username=username,
                                 address=address,
                                 pool_port=self.port,
                                 stratum_url=self.stratum_url,
                                 dashboard_url=self.dashboard_url)
        
        @self.app.route('/search', methods=['GET', 'POST'])
        def search_miner():
            """Search miner by address or username"""
            query = ''
            results = []
            
            if request.method == 'POST':
                query = request.form.get('query', '').strip()
            else:
                query = request.args.get('q', '').strip()
            
            if query:
                conn = sqlite3.connect(self.db_path)
                cursor = conn.cursor()
                
                # Search by address or username
                cursor.execute('''
                    SELECT miner_id, username, address, accepted_shares, rejected_shares, 
                           hash_rate, last_seen, connected_at
                    FROM miners 
                    WHERE address LIKE ? OR username LIKE ? OR miner_id LIKE ?
                    ORDER BY accepted_shares DESC
                    LIMIT 20
                ''', (f'%{query}%', f'%{query}%', f'%{query}%'))
                
                miners = cursor.fetchall()
                
                # Get detailed stats for each miner
                for miner in miners:
                    miner_id = miner[0]
                    
                    # Get shares count
                    cursor.execute('''
                        SELECT COUNT(*) FROM shares WHERE miner_id = ? AND is_valid = 1
                    ''', (miner_id,))
                    shares_count = cursor.fetchone()[0] or 0
                    
                    # Get earnings using fair PPLNS calculation
                    earnings = self.calculate_miner_earnings(miner_id)
                    total_payouts = earnings['total_paid']
                    pending_balance = earnings['pending_balance']
                    
                    # Get blocks found
                    cursor.execute('''
                        SELECT COUNT(*) FROM blocks WHERE miner_id = ?
                    ''', (miner_id,))
                    blocks_found = cursor.fetchone()[0] or 0
                    
                    # Calculate estimated earnings
                    estimated_earnings = shares_count * 0.0001
                    
                    results.append({
                        'miner_id': miner[0],
                        'username': miner[1],
                        'address': miner[2],
                        'accepted_shares': miner[3] or 0,
                        'rejected_shares': miner[4] or 0,
                        'hash_rate': miner[5] or 0.0,
                        'last_seen': miner[6] or 'Never',
                        'connected_at': miner[7] or 'Unknown',
                        'total_shares': shares_count,
                        'total_payouts': total_payouts,
                        'pending_balance': pending_balance,
                        'total_earnings': earnings['total_earnings'],
                        'blocks_found': blocks_found,
                        'estimated_earnings': earnings['total_earnings']
                    })
                
                conn.close()
            
            return render_template('search.html',
                                 pool_name=self.pool_name,
                                 algorithm=self.algorithm,
                                 query=query,
                                 results=results)
        
        @self.app.route('/miner/<miner_id>')
        def miner_dashboard(miner_id):
            """Individual miner dashboard"""
            conn = sqlite3.connect(self.db_path)
            cursor = conn.cursor()
            
            cursor.execute('''
                SELECT miner_id, username, address, accepted_shares, rejected_shares, 
                       hash_rate, last_seen
                FROM miners WHERE miner_id = ? OR username = ?
            ''', (miner_id, miner_id))
            
            miner = cursor.fetchone()
            if not miner:
                return "Miner not found", 404
            
            # Get miner's shares count
            cursor.execute('''
                SELECT COUNT(*) FROM shares WHERE miner_id = ? AND is_valid = 1
            ''', (miner[0],))
            shares_count = cursor.fetchone()[0] or 0
            
            # Get total payouts
            cursor.execute('''
                SELECT SUM(amount) FROM payouts WHERE miner_id = ? AND (status = 'completed' OR status = 'paid')
            ''', (miner[0],))
            total_payouts = cursor.fetchone()[0] or 0.0
            
            # Calculate actual earnings using fair PPLNS system
            earnings = self.calculate_miner_earnings(miner[0])
            estimated_earnings = earnings['total_earnings']
            
            # Get miner's blocks
            cursor.execute('''
                SELECT block_hash, reward, found_at FROM blocks WHERE miner_id = ? ORDER BY found_at DESC LIMIT 10
            ''', (miner[0],))
            blocks = cursor.fetchall()
            
            conn.close()
            
            miner_stats = {
                'miner_id': miner[0],
                'username': miner[1],
                'address': miner[2],
                'accepted_shares': miner[3] or 0,
                'rejected_shares': miner[4] or 0,
                'hash_rate': miner[5] or 0.0,
                'last_seen': miner[6] or 'Never',
                'total_shares': shares_count,
                'total_payouts': earnings['total_paid'],
                'pending_balance': earnings['pending_balance'],
                'total_earnings': earnings['total_earnings'],
                'blocks': [{'hash': b[0], 'reward': b[1] or 0.0, 'timestamp': b[2]} for b in blocks]
            }
            
            return render_template('miner_dashboard.html', 
                                 pool_name=self.pool_name,
                                 algorithm=self.algorithm,
                                 miner=miner_stats,
                                 pool_port=self.port,
                                 stratum_url=self.stratum_url,
                                 dashboard_url=self.dashboard_url)
        
        @self.app.route('/payouts')
        def payouts():
            """Payouts page"""
            conn = sqlite3.connect(self.db_path)
            cursor = conn.cursor()
            
            cursor.execute('''
                SELECT payout_id, miner_id, amount, status, created_at, paid_at
                FROM payouts ORDER BY created_at DESC LIMIT 100
            ''')
            payouts_list = cursor.fetchall()
            
            cursor.execute('''
                SELECT SUM(amount) FROM payouts WHERE status = 'completed' OR status = 'paid'
            ''')
            total_paid = cursor.fetchone()[0] or 0.0
            
            conn.close()
            
            payouts_data = [{
                'payout_id': str(p[0]),
                'miner_id': p[1],
                'amount': p[2],
                'status': p[3],
                'timestamp': p[5] or p[4]  # Use paid_at if available, else created_at
            } for p in payouts_list]
            
            return render_template('payouts.html',
                                 pool_name=self.pool_name,
                                 algorithm=self.algorithm,
                                 payouts=payouts_data,
                                 total_paid=total_paid)
        
        @self.app.route('/settings')
        def settings():
            """Pool settings page"""
            stats = self.get_pool_stats()
            return render_template('pool_settings.html',
                                 pool_name=self.pool_name,
                                 algorithm=self.algorithm,
                                 stats=stats,
                                 pool_port=self.port,
                                 rpc_url=self.rpc_url,
                                 stratum_url=self.stratum_url,
                                 dashboard_url=self.dashboard_url)
        
        @self.app.route('/help')
        def help_page():
            """Help and documentation page"""
            return render_template('pool_help.html',
                                 pool_name=self.pool_name,
                                 algorithm=self.algorithm,
                                 pool_port=self.port,
                                 stratum_url=self.stratum_url,
                                 dashboard_url=self.dashboard_url)
        
        @self.app.route('/statistics')
        def statistics():
            """Detailed statistics page"""
            stats = self.get_pool_stats()
            conn = sqlite3.connect(self.db_path)
            cursor = conn.cursor()
            
            # Get 24h stats
            cursor.execute('''
                SELECT COUNT(*) FROM shares 
                WHERE submitted_at >= datetime('now', '-1 day')
            ''')
            shares_24h = cursor.fetchone()[0]
            
            cursor.execute('''
                SELECT COUNT(*) FROM blocks 
                WHERE found_at >= datetime('now', '-1 day')
            ''')
            blocks_24h = cursor.fetchone()[0]
            
            # Get hash rate distribution
            cursor.execute('''
                SELECT 
                    CASE 
                        WHEN hash_rate < 1000 THEN '< 1 KH/s'
                        WHEN hash_rate < 1000000 THEN '1-1000 KH/s'
                        WHEN hash_rate < 1000000000 THEN '1-1000 MH/s'
                        ELSE '> 1 GH/s'
                    END as range,
                    COUNT(*) as count
                FROM miners WHERE is_active = 1
                GROUP BY range
            ''')
            hash_rate_dist = cursor.fetchall()
            
            conn.close()
            
            return render_template('statistics.html',
                                 pool_name=self.pool_name,
                                 algorithm=self.algorithm,
                                 stats=stats,
                                 shares_24h=shares_24h,
                                 blocks_24h=blocks_24h,
                                 hash_rate_dist=hash_rate_dist)
        
        @self.app.route('/leaderboard')
        def leaderboard():
            """Top miners leaderboard"""
            conn = sqlite3.connect(self.db_path)
            cursor = conn.cursor()
            
            # Top miners by accepted shares
            cursor.execute('''
                SELECT username, address, accepted_shares, hash_rate, 
                       (SELECT COUNT(*) FROM blocks WHERE miner_id = miners.miner_id) as blocks_found
                FROM miners 
                WHERE is_active = 1
                ORDER BY accepted_shares DESC
                LIMIT 50
            ''')
            top_by_shares = cursor.fetchall()
            
            # Top miners by hash rate
            cursor.execute('''
                SELECT username, address, accepted_shares, hash_rate,
                       (SELECT COUNT(*) FROM blocks WHERE miner_id = miners.miner_id) as blocks_found
                FROM miners 
                WHERE is_active = 1
                ORDER BY hash_rate DESC
                LIMIT 50
            ''')
            top_by_hashrate = cursor.fetchall()
            
            # Top miners by blocks found
            cursor.execute('''
                SELECT m.username, m.address, m.accepted_shares, m.hash_rate,
                       COUNT(b.block_id) as blocks_found
                FROM miners m
                LEFT JOIN blocks b ON b.miner_id = m.miner_id
                WHERE m.is_active = 1
                GROUP BY m.miner_id
                ORDER BY blocks_found DESC
                LIMIT 50
            ''')
            top_by_blocks = cursor.fetchall()
            
            conn.close()
            
            return render_template('leaderboard.html',
                                 pool_name=self.pool_name,
                                 algorithm=self.algorithm,
                                 top_by_shares=top_by_shares,
                                 top_by_hashrate=top_by_hashrate,
                                 top_by_blocks=top_by_blocks)
        
        @self.app.route('/blocks')
        def blocks_page():
            """Recent blocks found"""
            conn = sqlite3.connect(self.db_path)
            cursor = conn.cursor()
            
            cursor.execute('''
                SELECT b.block_hash, b.block_number, m.username, b.reward, b.found_at, b.confirmed
                FROM blocks b
                LEFT JOIN miners m ON m.miner_id = b.miner_id
                ORDER BY b.found_at DESC
                LIMIT 100
            ''')
            blocks = cursor.fetchall()
            
            conn.close()
            
            blocks_data = [{
                'hash': b[0],
                'number': b[1],
                'miner': b[2] or 'Unknown',
                'reward': b[3] or 0.0,
                'timestamp': b[4],
                'confirmed': b[5]
            } for b in blocks]
            
            return render_template('blocks.html',
                                 pool_name=self.pool_name,
                                 algorithm=self.algorithm,
                                 blocks=blocks_data)
        
        @self.app.route('/calculator')
        def calculator():
            """Mining profitability calculator"""
            return render_template('calculator.html',
                                 pool_name=self.pool_name,
                                 algorithm=self.algorithm)
        
        @self.app.route('/api-docs')
        def api_docs():
            """API documentation"""
            base_url = request.host_url.rstrip('/')
            return render_template('api_docs.html',
                                 pool_name=self.pool_name,
                                 algorithm=self.algorithm,
                                 base_url=base_url)
        
        @self.app.route('/faq')
        def faq():
            """Frequently Asked Questions"""
            return render_template('faq.html',
                                 pool_name=self.pool_name,
                                 algorithm=self.algorithm)
        
        @self.app.route('/terms')
        def terms():
            """Terms of Service"""
            return render_template('terms.html',
                                 pool_name=self.pool_name)
        
        @self.app.route('/privacy')
        def privacy():
            """Privacy Policy"""
            return render_template('privacy.html',
                                 pool_name=self.pool_name)
        
        @self.app.route('/network-status')
        def network_status():
            """Network and pool status"""
            stats = self.get_pool_stats()
            blockchain_info = self.get_blockchain_info()
            pool_balance = self.get_pool_balance()
            
            return render_template('network_status.html',
                                 pool_name=self.pool_name,
                                 algorithm=self.algorithm,
                                 stats=stats,
                                 blockchain_info=blockchain_info,
                                 pool_balance=pool_balance,
                                 rpc_url=self.rpc_url)
        
        @self.app.route('/api/pool-balance')
        def api_pool_balance():
            """API endpoint for pool balance"""
            balance = self.get_pool_balance()
            return jsonify({'balance': balance, 'address': os.environ.get('POOL_ADDRESS', 'Not set')})
    
    def get_pool_stats(self) -> Dict:
        """Get pool statistics"""
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        
        cursor.execute('SELECT COUNT(*) FROM miners WHERE is_active = 1')
        active_miners = cursor.fetchone()[0]
        
        cursor.execute('SELECT SUM(accepted_shares) FROM miners')
        total_accepted = cursor.fetchone()[0] or 0
        
        cursor.execute('SELECT COUNT(*) FROM blocks WHERE confirmed = 1')
        confirmed_blocks = cursor.fetchone()[0]
        
        conn.close()
        
        return {
            'pool_name': self.pool_name,
            'algorithm': self.algorithm,
            'active_miners': active_miners,
            'total_shares': self.total_shares,
            'accepted_shares': self.accepted_shares,
            'rejected_shares': self.rejected_shares,
            'blocks_found': self.blocks_found,
            'confirmed_blocks': confirmed_blocks,
            'difficulty': self.difficulty,
            'current_job': self.current_job['job_id'] if self.current_job else None
        }
    
    def get_miners_list(self) -> List[Dict]:
        """Get list of miners"""
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        
        cursor.execute('''
            SELECT miner_id, username, address, total_shares, accepted_shares, 
                   rejected_shares, hash_rate, connected_at, last_seen
            FROM miners
            WHERE is_active = 1
            ORDER BY accepted_shares DESC
            LIMIT 100
        ''')
        
        miners = []
        for row in cursor.fetchall():
            miners.append({
                'id': row[0],
                'username': row[1],
                'address': row[2],
                'total_shares': row[3],
                'accepted_shares': row[4],
                'rejected_shares': row[5],
                'hash_rate': row[6],
                'connected_at': row[7],
                'last_seen': row[8]
            })
        
        conn.close()
        return miners
    
    def get_blocks_list(self) -> List[Dict]:
        """Get list of blocks found"""
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        
        cursor.execute('''
            SELECT block_hash, block_number, miner_id, found_at, confirmed, reward
            FROM blocks
            ORDER BY found_at DESC
            LIMIT 50
        ''')
        
        blocks = []
        for row in cursor.fetchall():
            blocks.append({
                'hash': row[0],
                'number': row[1],
                'miner': row[2],
                'found_at': row[3],
                'confirmed': bool(row[4]),
                'reward': row[5]
            })
        
        conn.close()
        return blocks
    
    def distribute_block_rewards(self, block_hash=None, block_reward=12.5, pool_fee=0.02):
        """
        Distribute block rewards fairly using PPLNS (Pay Per Last N Shares)
        PPLNS ensures fair distribution based on recent contribution
        """
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        
        try:
            # Calculate net reward after pool fee
            net_reward = block_reward * (1 - pool_fee)
            
            # Get last N shares (window for PPLNS - last 1000 shares for fairness)
            pplns_window = 1000
            cursor.execute('''
                SELECT miner_id, COUNT(*) as share_count
                FROM shares
                WHERE is_valid = 1
                AND share_id > (SELECT MAX(share_id) - ? FROM shares)
                GROUP BY miner_id
            ''', (pplns_window,))
            
            recent_shares = cursor.fetchall()
            
            if not recent_shares:
                print(f"[{self.pool_name}] No recent shares for reward distribution")
                return
            
            # Calculate total shares in window
            total_shares_in_window = sum(row[1] for row in recent_shares)
            
            if total_shares_in_window == 0:
                print(f"[{self.pool_name}] No valid shares for reward distribution")
                return
            
            # Distribute rewards proportionally
            for miner_id, share_count in recent_shares:
                # Calculate miner's share of reward
                miner_share = (share_count / total_shares_in_window) * net_reward
                
                # Get miner's address
                cursor.execute('SELECT address FROM miners WHERE miner_id = ?', (miner_id,))
                miner_data = cursor.fetchone()
                
                if miner_data:
                    address = miner_data[0]
                    
                    # Create pending payout
                    cursor.execute('''
                        INSERT INTO payouts (miner_id, amount, status, created_at)
                        VALUES (?, ?, 'pending', CURRENT_TIMESTAMP)
                    ''', (miner_id, miner_share))
                    
                    # Update miner's pending balance
                    cursor.execute('''
                        UPDATE miners
                        SET pending_balance = pending_balance + ?
                        WHERE miner_id = ?
                    ''', (miner_share, miner_id))
                    
                    print(f"[{self.pool_name}] Distributed {miner_share:.4f} GXC to {miner_id} ({share_count} shares)")
            
            conn.commit()
            print(f"[{self.pool_name}] Total reward distributed: {net_reward:.4f} GXC (fee: {block_reward * pool_fee:.4f} GXC)")
            
        except Exception as e:
            print(f"[{self.pool_name}] Error distributing rewards: {e}")
            conn.rollback()
        finally:
            conn.close()
    
    def get_pool_balance(self) -> float:
        """Get pool's balance from blockchain"""
        try:
            # Get pool address from environment or use a default
            pool_address = os.environ.get('POOL_ADDRESS', '')
            if not pool_address:
                print(f"[{self.pool_name}] Warning: POOL_ADDRESS not set, cannot check balance")
                return 0.0
            
            response = requests.post(self.rpc_url, json={
                'jsonrpc': '2.0',
                'method': 'getbalance',
                'params': [pool_address],
                'id': 1
            }, timeout=10, headers={'Content-Type': 'application/json'})
            
            if response.status_code == 200:
                data = response.json()
                if 'error' in data:
                    print(f"[{self.pool_name}] Error getting balance: {data['error']}")
                    return 0.0
                balance = float(data.get('result', 0))
                return balance
            return 0.0
        except Exception as e:
            print(f"[{self.pool_name}] Error checking pool balance: {e}")
            return 0.0
    
    def send_payout_transaction(self, from_address: str, to_address: str, amount: float) -> Optional[str]:
        """Send payout transaction to blockchain"""
        try:
            response = requests.post(self.rpc_url, json={
                'jsonrpc': '2.0',
                'method': 'sendtoaddress',
                'params': [from_address, to_address, amount],
                'id': 1
            }, timeout=30, headers={'Content-Type': 'application/json'})
            
            if response.status_code == 200:
                data = response.json()
                if 'error' in data:
                    error_msg = data['error']
                    if isinstance(error_msg, dict):
                        error_msg = error_msg.get('message', str(error_msg))
                    print(f"[{self.pool_name}] Payout transaction error: {error_msg}")
                    return None
                result = data.get('result')
                if isinstance(result, dict):
                    return result.get('txid')
                return str(result) if result else None
            return None
        except Exception as e:
            print(f"[{self.pool_name}] Error sending payout transaction: {e}")
            return None
    
    def process_payouts(self, min_payout=0.1):
        """
        Process pending payouts for miners who have reached minimum threshold
        This should be called periodically (e.g., daily)
        """
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        
        try:
            # Get pool address for sending payouts
            pool_address = os.environ.get('POOL_ADDRESS', '')
            if not pool_address:
                print(f"[{self.pool_name}] Error: POOL_ADDRESS not set, cannot process payouts")
                return
            
            # Check pool balance
            pool_balance = self.get_pool_balance()
            if pool_balance < min_payout:
                print(f"[{self.pool_name}] Warning: Pool balance ({pool_balance:.4f} GXC) is below minimum payout threshold")
            
            # Get miners with pending balance >= minimum payout
            cursor.execute('''
                SELECT miner_id, address, pending_balance
                FROM miners
                WHERE pending_balance >= ?
                AND address IS NOT NULL
                AND address != ''
            ''', (min_payout,))
            
            eligible_miners = cursor.fetchall()
            
            if not eligible_miners:
                print(f"[{self.pool_name}] No miners eligible for payout")
                return
            
            print(f"[{self.pool_name}] Processing {len(eligible_miners)} payouts...")
            
            successful_payouts = 0
            failed_payouts = 0
            
            for miner_id, address, pending_balance in eligible_miners:
                try:
                    # Check if we have enough balance
                    if pool_balance < pending_balance:
                        print(f"[{self.pool_name}] Insufficient balance for payout to {miner_id}: need {pending_balance:.4f}, have {pool_balance:.4f}")
                        continue
                    
                    # Send transaction to blockchain
                    txid = self.send_payout_transaction(pool_address, address, pending_balance)
                    
                    if txid:
                        # Update payout record with transaction hash
                        cursor.execute('''
                            UPDATE payouts
                            SET status = 'completed', tx_hash = ?, paid_at = CURRENT_TIMESTAMP
                            WHERE miner_id = ? AND status = 'pending'
                        ''', (txid, miner_id))
                        
                        # Reset pending balance
                        cursor.execute('''
                            UPDATE miners
                            SET pending_balance = 0.0
                            WHERE miner_id = ?
                        ''', (miner_id,))
                        
                        # Update pool balance estimate
                        pool_balance -= pending_balance
                        successful_payouts += 1
                        
                        print(f"[{self.pool_name}] ✅ Payout sent: {pending_balance:.4f} GXC to {address} (TX: {txid[:16]}...)")
                    else:
                        # Mark as failed
                        cursor.execute('''
                            UPDATE payouts
                            SET status = 'failed'
                            WHERE miner_id = ? AND status = 'pending'
                        ''', (miner_id,))
                        failed_payouts += 1
                        print(f"[{self.pool_name}] ❌ Failed to send payout to {miner_id}")
                    
                except Exception as e:
                    print(f"[{self.pool_name}] Error processing payout for {miner_id}: {e}")
                    # Mark as failed
                    cursor.execute('''
                        UPDATE payouts
                        SET status = 'failed'
                        WHERE miner_id = ? AND status = 'pending'
                    ''', (miner_id,))
                    failed_payouts += 1
            
            conn.commit()
            print(f"[{self.pool_name}] Payout processing complete: {successful_payouts} successful, {failed_payouts} failed")
            
        except Exception as e:
            print(f"[{self.pool_name}] Error processing payouts: {e}")
            conn.rollback()
        finally:
            conn.close()
    
    def calculate_miner_earnings(self, miner_id: str) -> Dict:
        """
        Calculate miner's total earnings and pending balance
        Uses PPLNS for fair calculation
        """
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        
        try:
            # Get miner's pending balance
            cursor.execute('SELECT pending_balance FROM miners WHERE miner_id = ?', (miner_id,))
            pending = cursor.fetchone()
            pending_balance = pending[0] if pending else 0.0
            
            # Get total payouts
            cursor.execute('''
                SELECT SUM(amount) FROM payouts 
                WHERE miner_id = ? AND (status = 'completed' OR status = 'paid')
            ''', (miner_id,))
            total_paid = cursor.fetchone()[0] or 0.0
            
            # Get shares in PPLNS window
            pplns_window = 1000
            cursor.execute('''
                SELECT COUNT(*) FROM shares
                WHERE miner_id = ? AND is_valid = 1
                AND share_id > (SELECT MAX(share_id) - ? FROM shares)
            ''', (miner_id, pplns_window))
            shares_in_window = cursor.fetchone()[0] or 0
            
            # Get total shares
            cursor.execute('''
                SELECT COUNT(*) FROM shares WHERE miner_id = ? AND is_valid = 1
            ''', (miner_id,))
            total_shares = cursor.fetchone()[0] or 0
            
            return {
                'pending_balance': pending_balance,
                'total_paid': total_paid,
                'total_earnings': pending_balance + total_paid,
                'shares_in_window': shares_in_window,
                'total_shares': total_shares
            }
        except Exception as e:
            print(f"Error calculating earnings: {e}")
            return {
                'pending_balance': 0.0,
                'total_paid': 0.0,
                'total_earnings': 0.0,
                'shares_in_window': 0,
                'total_shares': 0
            }
        finally:
            conn.close()
    
    def start_background_tasks(self):
        """Start background tasks for pool management"""
        def update_job_periodically():
            """Update mining job every 30 seconds"""
            while True:
                try:
                    time.sleep(30)
                    if self.stratum_running:
                        # Generate new job to keep miners updated
                        self.generate_mining_job()
                except Exception as e:
                    print(f"[{self.pool_name}] Error in job update task: {e}")
        
        def process_payouts_periodically():
            """Process payouts every hour"""
            while True:
                try:
                    time.sleep(3600)  # 1 hour
                    self.process_payouts()
                except Exception as e:
                    print(f"[{self.pool_name}] Error in payout processing task: {e}")
        
        # Start background threads
        job_thread = threading.Thread(target=update_job_periodically)
        job_thread.daemon = True
        job_thread.start()
        
        payout_thread = threading.Thread(target=process_payouts_periodically)
        payout_thread.daemon = True
        payout_thread.start()
        
        print(f"[{self.pool_name}] Background tasks started (job updates, payout processing)")
    
    def run(self, host='0.0.0.0', port=5000, debug=False, allow_unsafe_werkzeug=False):
        """Run the pool web server"""
        print(f"[{self.pool_name}] Starting web server on {host}:{port}")
        if self.socketio:
            self.socketio.run(self.app, host=host, port=port, debug=debug, allow_unsafe_werkzeug=allow_unsafe_werkzeug)
        else:
            # For Vercel/serverless, just run Flask directly
            self.app.run(host=host, port=port, debug=debug)

