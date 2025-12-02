#!/usr/bin/env python3
"""
GXC Blockchain Explorer Web Application
Real-time blockchain data visualization and transaction tracking
"""

import os
import json
import sqlite3
import requests
from datetime import datetime, timedelta
from flask import Flask, render_template, request, jsonify, redirect, url_for
from flask_socketio import SocketIO, emit
import threading
import time

app = Flask(__name__, static_folder='static', static_url_path='/static')
app.config['SECRET_KEY'] = 'gxc_explorer_secret_key'
socketio = SocketIO(app, cors_allowed_origins="*")

# Network Configuration
NETWORK = os.environ.get('GXC_NETWORK', 'testnet').lower()  # 'testnet' or 'mainnet'
NETWORK_CONFIG = {
    'testnet': {
        'address_prefix': 'tGXC',
        'block_time': 120,  # 2 minutes
        'difficulty': 0.1,
        'block_reward': 50.0,  # Initial reward (halves every 1,051,200 blocks)
        'rpc_port': 18332,
        'railway_url': 'https://gxc-chain112-blockchain-node-production.up.railway.app',
        'local_url': 'http://localhost:18332',
    },
    'mainnet': {
        'address_prefix': 'GXC',
        'block_time': 600,
        'difficulty': 1.0,
        'block_reward': 50.0,
        'rpc_port': 8332,
        'railway_url': None,  # Set when mainnet Railway node is deployed
        'local_url': 'http://localhost:8332',
    }
}

# Get current network config
CURRENT_NETWORK = NETWORK_CONFIG.get(NETWORK, NETWORK_CONFIG['testnet'])

# Railway Node URLs - Network specific
RAILWAY_NODE_URL = CURRENT_NETWORK.get('railway_url') or NETWORK_CONFIG['testnet']['railway_url']
LOCAL_NODE_URL = CURRENT_NETWORK['local_url']

# Configuration
# Use Railway URL from environment, fallback to network-specific Railway URL
BLOCKCHAIN_NODE_URL = os.environ.get('BLOCKCHAIN_NODE_URL', os.environ.get('RAILWAY_NODE_URL', RAILWAY_NODE_URL))
# Active node URL (will be set by connection test)
ACTIVE_NODE_URL = BLOCKCHAIN_NODE_URL
ACTIVE_NODE_TYPE = None  # 'railway' or 'local'
CONNECTION_STATUS = {
    'railway': {'connected': False, 'last_check': None, 'error': None},
    'local': {'connected': False, 'last_check': None, 'error': None}
}

# Network-aware database path - separate databases for testnet/mainnet
base_db_path = '/tmp' if os.path.exists('/tmp') else '.'
db_filename = f'gxc_explorer_{NETWORK}.db'
DATABASE_PATH = os.environ.get('DATABASE_PATH', os.path.join(base_db_path, db_filename))

# Forum URL - can be overridden via environment variable
FORUM_URL = os.environ.get('FORUM_URL', 'http://localhost:5000')

# Helper function for safe integer conversion from request parameters
def safe_int(value, default=0, min_val=None, max_val=None):
    """
    Safely convert a value to integer with bounds checking.
    
    Args:
        value: The value to convert
        default: Default value if conversion fails
        min_val: Minimum allowed value (optional)
        max_val: Maximum allowed value (optional)
    
    Returns:
        Integer value within specified bounds
    """
    try:
        result = int(value)
        if min_val is not None:
            result = max(result, min_val)
        if max_val is not None:
            result = min(result, max_val)
        return result
    except (ValueError, TypeError):
        return default

# Helper function for safe float conversion from request parameters
def safe_float(value, default=0.0, min_val=None, max_val=None):
    """
    Safely convert a value to float with bounds checking.
    
    Args:
        value: The value to convert
        default: Default value if conversion fails
        min_val: Minimum allowed value (optional)
        max_val: Maximum allowed value (optional)
    
    Returns:
        Float value within specified bounds
    """
    try:
        result = float(value)
        if min_val is not None:
            result = max(result, min_val)
        if max_val is not None:
            result = min(result, max_val)
        return result
    except (ValueError, TypeError):
        return default

# Global RPC call helper with better error handling
def rpc_call(method, params=None, timeout=10, retry=True, show_errors=True):
    """
    Make RPC call to blockchain node with robust error handling and fallback
    
    Args:
        method: RPC method name
        params: RPC parameters (list or dict)
        timeout: Request timeout in seconds
        retry: Whether to retry with fallback node
        show_errors: Whether to print error messages
    
    Returns:
        RPC result or None on failure
    """
    global ACTIVE_NODE_URL, ACTIVE_NODE_TYPE, CONNECTION_STATUS
    
    if params is None:
        params = []
    
    payload = {
        'jsonrpc': '2.0',
        'method': method,
        'params': params if isinstance(params, list) else [params] if params else [],
        'id': 1
    }
    
    # Try active node first
    nodes_to_try = []
    if ACTIVE_NODE_TYPE == 'railway' or ACTIVE_NODE_URL == RAILWAY_NODE_URL:
        nodes_to_try = [('railway', RAILWAY_NODE_URL), ('local', LOCAL_NODE_URL)]
    else:
        nodes_to_try = [('local', LOCAL_NODE_URL), ('railway', RAILWAY_NODE_URL)]
    
    for node_type, node_url in nodes_to_try:
        try:
            response = requests.post(node_url, json=payload, timeout=timeout)
            
            if response.status_code == 200:
                result = response.json()
                
                # Check for RPC error
                if 'error' in result:
                    error_msg = result['error']
                    CONNECTION_STATUS[node_type]['error'] = str(error_msg)
                    if show_errors:
                        print(f"[RPC] {node_type.upper()} node error: {error_msg}")
                    continue  # Try next node
                
                # Success!
                if 'result' in result and result['result'] is not None:
                    CONNECTION_STATUS[node_type]['connected'] = True
                    CONNECTION_STATUS[node_type]['last_check'] = time.time()
                    CONNECTION_STATUS[node_type]['error'] = None
                    ACTIVE_NODE_URL = node_url
                    ACTIVE_NODE_TYPE = node_type
                    return result['result']
                elif 'result' in result:
                    # Result is None or empty, which is valid for some methods
                    CONNECTION_STATUS[node_type]['connected'] = True
                    CONNECTION_STATUS[node_type]['last_check'] = time.time()
                    CONNECTION_STATUS[node_type]['error'] = None
                    ACTIVE_NODE_URL = node_url
                    ACTIVE_NODE_TYPE = node_type
                    return result.get('result')
            else:
                CONNECTION_STATUS[node_type]['error'] = f"HTTP {response.status_code}"
                if show_errors and retry:
                    print(f"[RPC] {node_type.upper()} node returned status {response.status_code}")
                
        except requests.exceptions.Timeout:
            CONNECTION_STATUS[node_type]['connected'] = False
            CONNECTION_STATUS[node_type]['error'] = "Timeout"
            if show_errors and retry:
                print(f"[RPC] {node_type.upper()} node timeout (>{timeout}s)")
        except requests.exceptions.ConnectionError as e:
            CONNECTION_STATUS[node_type]['connected'] = False
            error_msg = str(e)
            CONNECTION_STATUS[node_type]['error'] = error_msg
            if show_errors and retry:
                print(f"[RPC] Cannot connect to {node_type.upper()} node: {error_msg[:60]}")
        except Exception as e:
            CONNECTION_STATUS[node_type]['connected'] = False
            CONNECTION_STATUS[node_type]['error'] = str(e)[:100]
            if show_errors and retry:
                print(f"[RPC] {node_type.upper()} node error: {str(e)[:60]}")
        
        # If retry is False, don't try fallback
        if not retry:
            break
    
    # All nodes failed
    return None

def test_blockchain_connection():
    """Test connection to blockchain nodes and set active node with improved diagnostics"""
    global ACTIVE_NODE_URL, ACTIVE_NODE_TYPE
    
    print("[EXPLORER] Testing blockchain connection...")
    
    # List of RPC methods to try (in order)
    test_methods = [
        ('getblockchaininfo', []),
        ('getblockcount', []),
        ('getblocktemplate', [{'algorithm': 'sha256'}]),
    ]
    
    # Test Railway node
    print(f"  Testing Railway node: {RAILWAY_NODE_URL}")
    for method, params in test_methods:
        try:
            payload = {
                'jsonrpc': '2.0',
                'method': method,
                'params': params,
                'id': 1
            }
            response = requests.post(RAILWAY_NODE_URL, json=payload, timeout=10, 
                                   headers={'Content-Type': 'application/json'})
            if response.status_code == 200:
                result = response.json()
                # Check for valid response (either result or no error)
                if 'result' in result and result.get('result') is not None:
                    ACTIVE_NODE_URL = RAILWAY_NODE_URL
                    ACTIVE_NODE_TYPE = 'railway'
                    height = result['result'].get('blocks') or result['result'].get('height') or result['result'] or 0
                    print(f"  ✅ Railway node connected! Height: {height}")
                    CONNECTION_STATUS['railway']['connected'] = True
                    CONNECTION_STATUS['railway']['last_check'] = time.time()
                    CONNECTION_STATUS['railway']['error'] = None
                    return True, 'railway'
                elif 'error' in result:
                    # Method might not exist, try next method
                    continue
            elif response.status_code in [404, 405]:
                # Try root path
                try:
                    if not RAILWAY_NODE_URL.endswith('/'):
                        test_url = RAILWAY_NODE_URL + '/'
                    else:
                        test_url = RAILWAY_NODE_URL.rstrip('/')
                    response = requests.post(test_url, json=payload, timeout=10,
                                           headers={'Content-Type': 'application/json'})
                    if response.status_code == 200:
                        result = response.json()
                        if 'result' in result and result.get('result') is not None:
                            ACTIVE_NODE_URL = test_url if RAILWAY_NODE_URL.endswith('/') else RAILWAY_NODE_URL
                            ACTIVE_NODE_TYPE = 'railway'
                            print(f"  ✅ Railway node connected (via alternate path)!")
                            CONNECTION_STATUS['railway']['connected'] = True
                            CONNECTION_STATUS['railway']['last_check'] = time.time()
                            CONNECTION_STATUS['railway']['error'] = None
                            return True, 'railway'
                except:
                    pass
        except requests.exceptions.Timeout:
            CONNECTION_STATUS['railway']['error'] = "Timeout (>10s)"
            continue
        except requests.exceptions.ConnectionError as e:
            error_msg = str(e)
            CONNECTION_STATUS['railway']['error'] = error_msg[:100]
            # Only print error for first method attempt
            if method == test_methods[0][0]:
                print(f"  ⚠️  Railway node connection error: {error_msg[:60]}...")
            continue
        except Exception as e:
            error_msg = str(e)
            CONNECTION_STATUS['railway']['error'] = error_msg[:100]
            if method == test_methods[0][0]:
                print(f"  ⚠️  Railway node error: {error_msg[:60]}...")
            continue
    
    CONNECTION_STATUS['railway']['connected'] = False
    
    # Test local node
    print(f"  Testing local node: {LOCAL_NODE_URL}")
    for method, params in test_methods:
        try:
            payload = {
                'jsonrpc': '2.0',
                'method': method,
                'params': params,
                'id': 1
            }
            response = requests.post(LOCAL_NODE_URL, json=payload, timeout=5,
                                   headers={'Content-Type': 'application/json'})
            if response.status_code == 200:
                result = response.json()
                if 'result' in result and result.get('result') is not None:
                    ACTIVE_NODE_URL = LOCAL_NODE_URL
                    ACTIVE_NODE_TYPE = 'local'
                    height = result['result'].get('blocks') or result['result'].get('height') or result['result'] or 0
                    print(f"  ✅ Local node connected! Height: {height}")
                    CONNECTION_STATUS['local']['connected'] = True
                    CONNECTION_STATUS['local']['last_check'] = time.time()
                    CONNECTION_STATUS['local']['error'] = None
                    return True, 'local'
                elif 'error' in result:
                    continue
        except requests.exceptions.ConnectionError as e:
            error_msg = str(e)
            CONNECTION_STATUS['local']['error'] = error_msg[:100]
            if method == test_methods[0][0]:
                print(f"  ⚠️  Local node connection error: {error_msg[:60]}...")
            continue
        except Exception as e:
            error_msg = str(e)
            CONNECTION_STATUS['local']['error'] = error_msg[:100]
            if method == test_methods[0][0]:
                print(f"  ⚠️  Local node error: {error_msg[:60]}...")
            continue
    
    CONNECTION_STATUS['local']['connected'] = False
    print(f"  ❌ No blockchain nodes available")
    print(f"     Railway: {CONNECTION_STATUS['railway'].get('error', 'Unknown error')}")
    print(f"     Local: {CONNECTION_STATUS['local'].get('error', 'Unknown error')}")
    return False, None

# Detect environment (Vercel or local)
IS_VERCEL = os.environ.get('VERCEL', '0') == '1' or 'vercel' in os.environ.get('VERCEL_URL', '').lower()
EXPLORER_URL = os.environ.get('EXPLORER_URL', 'https://gxc-blockchain.vercel.app' if IS_VERCEL else 'http://localhost:3000')
MARKET_MAKER_URL = os.environ.get('MARKET_MAKER_URL', 'https://gxc-blockchain-market-maker.vercel.app' if IS_VERCEL else 'http://localhost:4000')
FORUM_URL = os.environ.get('FORUM_URL', 'https://gxc-blockchain-forum.vercel.app' if IS_VERCEL else 'http://localhost:3001')

class BlockchainExplorer:
    def __init__(self):
        self.init_database()
        self.start_block_monitor()
    
    def init_database(self):
        """Initialize explorer database"""
        # Add timeout to prevent database locking issues
        conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
        cursor = conn.cursor()
        # Enable WAL mode for better concurrency
        cursor.execute('PRAGMA journal_mode=WAL')
        cursor.execute('PRAGMA synchronous=NORMAL')
        cursor.execute('PRAGMA busy_timeout=30000')
        
        # Blocks table (enhanced to match blockchain structure)
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS blocks (
                block_number INTEGER PRIMARY KEY,
                block_hash TEXT UNIQUE NOT NULL,
                parent_hash TEXT NOT NULL,
                merkle_root TEXT,
                timestamp TIMESTAMP NOT NULL,
                miner_address TEXT NOT NULL,
                difficulty REAL NOT NULL,
                total_difficulty REAL NOT NULL,
                nonce INTEGER DEFAULT 0,
                size INTEGER NOT NULL,
                gas_used INTEGER DEFAULT 0,
                gas_limit INTEGER DEFAULT 0,
                transaction_count INTEGER DEFAULT 0,
                reward REAL DEFAULT 0.0,
                consensus_type TEXT DEFAULT 'pow',
                block_type INTEGER DEFAULT 0,
                validator_signature TEXT,
                pow_hash TEXT,
                pos_hash TEXT,
                fee_burn_rate REAL DEFAULT 0.0,
                pop_reference TEXT
            )
        ''')
        
        # Transactions table (enhanced to match blockchain structure)
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS transactions (
                tx_hash TEXT PRIMARY KEY,
                block_number INTEGER NOT NULL,
                block_hash TEXT,
                tx_index INTEGER NOT NULL,
                from_address TEXT NOT NULL,
                to_address TEXT,
                value REAL NOT NULL,
                fee REAL NOT NULL,
                gas_price REAL DEFAULT 0.0,
                gas_used INTEGER DEFAULT 0,
                status TEXT DEFAULT 'success',
                timestamp TIMESTAMP NOT NULL,
                input_data TEXT,
                nonce INTEGER DEFAULT 0,
                tx_type TEXT DEFAULT 'transfer',
                contract_address TEXT,
                signature TEXT,
                is_coinbase BOOLEAN DEFAULT 0,
                prev_tx_hash TEXT,
                referenced_amount REAL,
                traceability_valid BOOLEAN DEFAULT 1,
                memo TEXT,
                lock_time INTEGER DEFAULT 0,
                is_gold_backed BOOLEAN DEFAULT 0,
                pop_reference TEXT,
                FOREIGN KEY (block_number) REFERENCES blocks (block_number)
            )
        ''')
        
        # Transaction inputs table
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS transaction_inputs (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                transaction_hash TEXT NOT NULL,
                input_index INTEGER NOT NULL,
                prev_tx_hash TEXT NOT NULL,
                prev_output_index INTEGER NOT NULL,
                amount REAL NOT NULL,
                signature TEXT,
                public_key TEXT,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (transaction_hash) REFERENCES transactions (tx_hash),
                UNIQUE(transaction_hash, input_index)
            )
        ''')
        
        # Transaction outputs table
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS transaction_outputs (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                transaction_hash TEXT NOT NULL,
                output_index INTEGER NOT NULL,
                address TEXT NOT NULL,
                amount REAL NOT NULL,
                script TEXT,
                spent BOOLEAN DEFAULT 0,
                spent_by_tx TEXT,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (transaction_hash) REFERENCES transactions (tx_hash),
                UNIQUE(transaction_hash, output_index)
            )
        ''')
        
        # UTXO table
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS utxo (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                tx_hash TEXT NOT NULL,
                output_index INTEGER NOT NULL,
                address TEXT NOT NULL,
                amount REAL NOT NULL,
                block_height INTEGER NOT NULL,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                UNIQUE(tx_hash, output_index)
            )
        ''')
        
        # Traceability index table
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS traceability_index (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                tx_hash TEXT UNIQUE NOT NULL,
                prev_tx_hash TEXT NOT NULL,
                referenced_amount REAL NOT NULL,
                timestamp TIMESTAMP NOT NULL,
                block_height INTEGER NOT NULL,
                validation_status BOOLEAN DEFAULT 1,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (tx_hash) REFERENCES transactions (tx_hash)
            )
        ''')
        
        # Addresses table (for balance tracking)
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS addresses (
                address TEXT PRIMARY KEY,
                balance REAL DEFAULT 0.0,
                transaction_count INTEGER DEFAULT 0,
                first_seen TIMESTAMP NOT NULL,
                last_activity TIMESTAMP NOT NULL,
                address_type TEXT DEFAULT 'user',
                contract_code TEXT
            )
        ''')
        
        # Network statistics
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS network_stats (
                stat_id INTEGER PRIMARY KEY AUTOINCREMENT,
                timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                total_blocks INTEGER NOT NULL,
                total_transactions INTEGER NOT NULL,
                total_addresses INTEGER NOT NULL,
                total_supply REAL NOT NULL,
                market_cap REAL DEFAULT 0.0,
                hash_rate REAL DEFAULT 0.0,
                difficulty REAL NOT NULL,
                avg_block_time REAL DEFAULT 0.0,
                network_health REAL DEFAULT 100.0
            )
        ''')
        
        # Price history
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS price_history (
                price_id INTEGER PRIMARY KEY AUTOINCREMENT,
                timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                price_usd REAL NOT NULL,
                price_btc REAL DEFAULT 0.0,
                volume_24h REAL DEFAULT 0.0,
                market_cap REAL DEFAULT 0.0,
                supply_circulating REAL NOT NULL
            )
        ''')
        
        # Pending transactions (mempool)
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS pending_transactions (
                tx_hash TEXT PRIMARY KEY,
                from_address TEXT NOT NULL,
                to_address TEXT,
                value REAL NOT NULL,
                fee REAL NOT NULL,
                gas_price REAL DEFAULT 0.0,
                timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                input_data TEXT,
                nonce INTEGER DEFAULT 0,
                tx_type TEXT DEFAULT 'transfer'
            )
        ''')
        
        # Gold Token (GXC-G) tables
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS gold_tokens (
                address TEXT PRIMARY KEY,
                balance REAL DEFAULT 0.0,
                total_received REAL DEFAULT 0.0,
                total_sent REAL DEFAULT 0.0,
                transaction_count INTEGER DEFAULT 0,
                first_seen TIMESTAMP,
                last_activity TIMESTAMP
            )
        ''')
        
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS gold_reserves (
                vault_id TEXT PRIMARY KEY,
                vault_address TEXT NOT NULL,
                gold_grams REAL NOT NULL,
                attestation_hash TEXT,
                last_audit_time TIMESTAMP,
                auditor_signature TEXT,
                is_active BOOLEAN DEFAULT 1
            )
        ''')
        
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS gold_token_transfers (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                tx_hash TEXT UNIQUE NOT NULL,
                from_address TEXT,
                to_address TEXT,
                amount REAL NOT NULL,
                transfer_type TEXT DEFAULT 'transfer',
                vault_id TEXT,
                redemption_id TEXT,
                pop_reference TEXT,
                timestamp TIMESTAMP NOT NULL,
                block_height INTEGER,
                FOREIGN KEY (tx_hash) REFERENCES transactions (tx_hash)
            )
        ''')
        
        # Governance tables
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS governance_proposals (
                proposal_id TEXT PRIMARY KEY,
                proposer_address TEXT NOT NULL,
                title TEXT NOT NULL,
                description TEXT,
                proposal_type TEXT NOT NULL,
                parameter_type TEXT,
                old_value REAL,
                new_value REAL,
                status TEXT DEFAULT 'active',
                start_block INTEGER NOT NULL,
                end_block INTEGER NOT NULL,
                votes_for REAL DEFAULT 0.0,
                votes_against REAL DEFAULT 0.0,
                votes_abstain REAL DEFAULT 0.0,
                total_votes REAL DEFAULT 0.0,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                executed_at TIMESTAMP
            )
        ''')
        
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS governance_votes (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                proposal_id TEXT NOT NULL,
                voter_address TEXT NOT NULL,
                vote_type TEXT NOT NULL,
                vote_weight REAL NOT NULL,
                timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                signature TEXT,
                FOREIGN KEY (proposal_id) REFERENCES governance_proposals (proposal_id),
                UNIQUE(proposal_id, voter_address)
            )
        ''')
        
        # Cross-chain bridge tables
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS bridge_transfers (
                transfer_id TEXT PRIMARY KEY,
                source_chain TEXT NOT NULL,
                source_asset TEXT NOT NULL,
                amount REAL NOT NULL,
                destination_address TEXT NOT NULL,
                destination_chain TEXT DEFAULT 'GXC',
                tx_hash TEXT,
                status TEXT DEFAULT 'pending',
                required_signatures INTEGER DEFAULT 0,
                collected_signatures INTEGER DEFAULT 0,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                confirmed_at TIMESTAMP
            )
        ''')
        
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS bridge_validators (
                validator_id TEXT PRIMARY KEY,
                public_key TEXT NOT NULL,
                collateral REAL DEFAULT 0.0,
                is_active BOOLEAN DEFAULT 1,
                transfers_validated INTEGER DEFAULT 0,
                last_seen TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            )
        ''')
        
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS bridge_signatures (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                transfer_id TEXT NOT NULL,
                validator_id TEXT NOT NULL,
                signature TEXT NOT NULL,
                timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (transfer_id) REFERENCES bridge_transfers (transfer_id),
                UNIQUE(transfer_id, validator_id)
            )
        ''')
        
        # Staking/Validator tables
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS staking_validators (
                validator_address TEXT PRIMARY KEY,
                stake_amount REAL NOT NULL,
                staking_days INTEGER NOT NULL,
                stake_start_time TIMESTAMP NOT NULL,
                is_active BOOLEAN DEFAULT 1,
                blocks_produced INTEGER DEFAULT 0,
                missed_blocks INTEGER DEFAULT 0,
                uptime REAL DEFAULT 1.0,
                total_rewards REAL DEFAULT 0.0,
                pending_rewards REAL DEFAULT 0.0,
                is_slashed BOOLEAN DEFAULT 0,
                slashed_amount REAL DEFAULT 0.0,
                public_key TEXT,
                last_seen TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            )
        ''')
        
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS staking_delegations (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                delegator_address TEXT NOT NULL,
                validator_address TEXT NOT NULL,
                amount REAL NOT NULL,
                start_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                end_time TIMESTAMP,
                is_active BOOLEAN DEFAULT 1,
                total_rewards REAL DEFAULT 0.0
            )
        ''')
        
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS staking_rewards (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                validator_address TEXT NOT NULL,
                delegator_address TEXT,
                amount REAL NOT NULL,
                block_number INTEGER NOT NULL,
                timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            )
        ''')
        
        # Price alerts (user preferences)
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS price_alerts (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                user_id TEXT,
                alert_type TEXT NOT NULL,
                threshold_price REAL NOT NULL,
                direction TEXT NOT NULL,
                is_active BOOLEAN DEFAULT 1,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                triggered_at TIMESTAMP
            )
        ''')
        
        # API keys for authentication
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS api_keys (
                api_key TEXT PRIMARY KEY,
                user_id TEXT,
                name TEXT,
                rate_limit INTEGER DEFAULT 100,
                requests_today INTEGER DEFAULT 0,
                last_reset TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                is_active BOOLEAN DEFAULT 1
            )
        ''')
        
        # Webhooks
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS webhooks (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                url TEXT NOT NULL,
                events TEXT NOT NULL,
                secret TEXT,
                is_active BOOLEAN DEFAULT 1,
                last_delivery TIMESTAMP,
                delivery_count INTEGER DEFAULT 0,
                failure_count INTEGER DEFAULT 0,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            )
        ''')
        
        # Address relationships (for graph visualization)
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS address_relationships (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                address1 TEXT NOT NULL,
                address2 TEXT NOT NULL,
                transaction_count INTEGER DEFAULT 1,
                total_value REAL DEFAULT 0.0,
                first_interaction TIMESTAMP,
                last_interaction TIMESTAMP,
                UNIQUE(address1, address2)
            )
        ''')
        
        # Stock contracts table
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS stock_contracts (
                ticker TEXT PRIMARY KEY,
                company_name TEXT NOT NULL,
                exchange TEXT NOT NULL,
                contract_address TEXT UNIQUE,
                price_oracle TEXT,
                total_shares INTEGER DEFAULT 0,
                current_price REAL DEFAULT 0.0,
                market_cap REAL DEFAULT 0.0,
                dividend_yield REAL DEFAULT 0.0,
                sector TEXT,
                industry TEXT,
                is_active BOOLEAN DEFAULT 1,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                last_price_update TIMESTAMP
            )
        ''')
        
        # Stock price history
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS stock_price_history (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                ticker TEXT NOT NULL,
                price REAL NOT NULL,
                volume INTEGER DEFAULT 0,
                pop_hash TEXT,
                source TEXT,
                timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (ticker) REFERENCES stock_contracts (ticker)
            )
        ''')
        
        # Stock share balances
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS stock_balances (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                ticker TEXT NOT NULL,
                address TEXT NOT NULL,
                balance INTEGER DEFAULT 0,
                last_updated TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (ticker) REFERENCES stock_contracts (ticker),
                UNIQUE(ticker, address)
            )
        ''')
        
        # Stock transfers
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS stock_transfers (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                ticker TEXT NOT NULL,
                from_address TEXT NOT NULL,
                to_address TEXT NOT NULL,
                shares INTEGER NOT NULL,
                price_at_transfer REAL,
                tx_hash TEXT UNIQUE,
                prev_tx_hash TEXT,
                timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (ticker) REFERENCES stock_contracts (ticker)
            )
        ''')
        
        # Corporate actions
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS corporate_actions (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                ticker TEXT NOT NULL,
                action_id INTEGER NOT NULL,
                action_type TEXT NOT NULL,
                value REAL,
                description TEXT,
                proof_hash TEXT,
                status TEXT DEFAULT 'pending',
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                executed_at TIMESTAMP,
                FOREIGN KEY (ticker) REFERENCES stock_contracts (ticker),
                UNIQUE(ticker, action_id)
            )
        ''')
        
        conn.commit()
        conn.close()
    
    def get_latest_block(self):
        """Get latest block from the node - ensures ALL data is fetched including transactions"""
        try:
            # First try to get blockchain info to get latest height
            # Use longer timeout for testnet nodes to ensure ALL data is returned
            info = rpc_call('getblockchaininfo', timeout=20, show_errors=False)
            if info:
                height = info.get('blocks') or info.get('height') or 0
                if height > 0:
                    # Get the latest block by number (which will fetch ALL transactions with full data)
                    block = self.get_block_by_number(height)
                    if block:
                        return block
            
            # Try multiple methods for compatibility - ALWAYS request verbose/full data
            methods = [
                ('gxc_getLatestBlock', [True]),  # verbose
                ('gxc_getLatestBlock', []),
                ('getlatestblock', [True]),  # verbose
                ('getlatestblock', []),
                ('getblock', ['latest', 2]),  # verbosity level 2 = full data
                ('getblock', ['latest', True]),  # verbose
                ('getblock', ['latest']),
                ('getblock', [-1, 2]),  # verbosity level 2
                ('getblock', [-1, True]),  # verbose
                ('getblock', [-1])
            ]
            
            for method, params in methods:
                # Use longer timeout (20s) to ensure ALL data is fetched
                result = rpc_call(method, params=params, timeout=20, show_errors=False)
                if result:
                    # Normalize the block data to ensure consistent field names
                    normalized = self._normalize_block_data(result)
                    # ALWAYS ensure transactions are included with FULL data
                    if not normalized.get('transactions') or len(normalized['transactions']) == 0:
                        # Try to fetch transactions separately with verbose flags
                        block_num = normalized.get('number') or normalized.get('height') or 0
                        if block_num > 0:
                            tx_methods = [
                                ('getblocktransactions', [block_num, True]),  # verbose
                                ('getblocktransactions', [block_num]),
                                ('getblocktxs', [block_num, True]),  # verbose
                                ('getblocktxs', [block_num]),
                                ('gxc_getBlockTransactions', [block_num, True]),  # verbose
                                ('gxc_getBlockTransactions', [block_num])
                            ]
                            for tx_method, tx_params in tx_methods:
                                tx_result = rpc_call(tx_method, params=tx_params, timeout=20, show_errors=False)
                                if tx_result:
                                    normalized['transactions'] = tx_result if isinstance(tx_result, list) else [tx_result]
                                    break
                    
                    # If transactions are just hashes, fetch full data
                    if normalized.get('transactions'):
                        full_transactions = []
                        for tx in normalized['transactions']:
                            if isinstance(tx, str):
                                tx_full = self._get_full_transaction_data(tx)
                                if tx_full:
                                    full_transactions.append(tx_full)
                                else:
                                    full_transactions.append({'hash': tx})
                            else:
                                full_transactions.append(tx)
                        normalized['transactions'] = full_transactions
                    
                    return normalized
            
            return None
        except Exception as e:
            print(f"[EXPLORER] Error getting latest block: {e}")
            import traceback
            traceback.print_exc()
            return None
    
    def _normalize_block_data(self, block_data):
        """Normalize block data from different node formats to consistent format"""
        if not block_data:
            return None
        
        # Create normalized block with all possible field mappings
        normalized = {}
        
        # Block number/height
        normalized['number'] = block_data.get('number') or block_data.get('block_number') or block_data.get('height') or 0
        normalized['block_number'] = normalized['number']
        normalized['height'] = normalized['number']
        
        # Block hash
        normalized['hash'] = block_data.get('hash') or block_data.get('block_hash') or ''
        normalized['block_hash'] = normalized['hash']
        
        # Parent hash
        normalized['parent_hash'] = block_data.get('parent_hash') or block_data.get('prev_hash') or block_data.get('previousblockhash') or ''
        normalized['prev_hash'] = normalized['parent_hash']
        
        # Timestamp
        normalized['timestamp'] = block_data.get('timestamp') or int(time.time())
        
        # Miner
        normalized['miner'] = block_data.get('miner') or block_data.get('miner_address') or ''
        normalized['miner_address'] = normalized['miner']
        
        # Transactions - handle both list and dict formats
        transactions = block_data.get('transactions') or []
        if isinstance(transactions, dict):
            transactions = [transactions]
        elif not isinstance(transactions, list):
            transactions = []
        
        normalized['transactions'] = transactions
        normalized['transaction_count'] = len(transactions) or block_data.get('transaction_count') or block_data.get('tx_count') or 0
        normalized['tx_count'] = normalized['transaction_count']
        
        # Difficulty
        normalized['difficulty'] = block_data.get('difficulty') or 0.1
        
        # Nonce
        normalized['nonce'] = block_data.get('nonce') or 0
        
        # Consensus type
        normalized['consensus_type'] = block_data.get('consensus_type') or block_data.get('consensusType') or 'pow'
        normalized['consensusType'] = normalized['consensus_type']
        
        # Copy other fields
        for key in ['reward', 'size', 'gas_used', 'gas_limit', 'merkle_root', 'merkleRoot', 'total_difficulty', 'totalDifficulty']:
            if key in block_data:
                normalized[key] = block_data[key]
        
        return normalized
    
    def get_block_by_number(self, block_number):
        """Get block by number from the node with full transaction data - ensures ALL data is fetched"""
        try:
            # Try multiple methods for compatibility, ALWAYS use verbose flag to get ALL transaction data
            # Use longer timeout for testnet nodes to ensure all data is fetched
            methods = [
                ('gxc_getBlockByNumber', [block_number, True, True]),  # block_number, verbose, include_tx_data
                ('gxc_getBlockByNumber', [block_number, True]),  # block_number, verbose
                ('getblockbynumber', [block_number, True, True]),  # verbose + include_tx_data
                ('getblockbynumber', [block_number, True]),  # verbose
                ('getblock', [int(block_number), 2]),  # verbosity level 2 = full data
                ('getblock', [int(block_number), True]),  # verbose
                ('getblock', [str(block_number), 2]),  # verbosity level 2
                ('getblock', [str(block_number), True]),  # verbose
                ('getblock', [int(block_number)]),  # Fallback without verbose
                ('getblock', [str(block_number)])
            ]
            
            for method, params in methods:
                # Use longer timeout (20s) to ensure testnet nodes can return ALL data
                result = rpc_call(method, params=params, timeout=20, show_errors=False)
                if result:
                    # Normalize the block data to ensure consistent field names
                    normalized = self._normalize_block_data(result)
                    
                    # ALWAYS try to fetch transactions separately if not included or incomplete
                    # This ensures we get ALL transaction data even if block response doesn't include it
                    if not normalized.get('transactions') or len(normalized['transactions']) == 0:
                        # Try multiple methods to get ALL transactions for this block
                        tx_methods = [
                            ('getblocktransactions', [block_number, True]),  # verbose
                            ('getblocktransactions', [block_number]),
                            ('getblocktxs', [block_number, True]),  # verbose
                            ('getblocktxs', [block_number]),
                            ('gettransactions', [block_number, True]),  # verbose
                            ('gettransactions', [block_number]),
                            ('gxc_getBlockTransactions', [block_number, True]),  # verbose
                            ('gxc_getBlockTransactions', [block_number])
                        ]
                        for tx_method, tx_params in tx_methods:
                            tx_result = rpc_call(tx_method, params=tx_params, timeout=20, show_errors=False)
                            if tx_result:
                                # Ensure we have a list of transactions
                                if isinstance(tx_result, list):
                                    normalized['transactions'] = tx_result
                                elif isinstance(tx_result, dict):
                                    normalized['transactions'] = [tx_result]
                                else:
                                    normalized['transactions'] = [tx_result] if tx_result else []
                                
                                # Update transaction count
                                normalized['transaction_count'] = len(normalized['transactions'])
                                normalized['tx_count'] = len(normalized['transactions'])
                                break
                    
                    # If we have transactions but they're incomplete (just hashes), fetch full data
                    if normalized.get('transactions'):
                        full_transactions = []
                        for tx in normalized['transactions']:
                            # If transaction is just a hash string, fetch full data
                            if isinstance(tx, str):
                                tx_full = self._get_full_transaction_data(tx)
                                if tx_full:
                                    full_transactions.append(tx_full)
                                else:
                                    full_transactions.append({'hash': tx})  # Keep hash if can't fetch
                            else:
                                full_transactions.append(tx)  # Already has data
                        normalized['transactions'] = full_transactions
                    
                    return normalized
            
            return None
        except Exception as e:
            print(f"[EXPLORER] Error getting block by number {block_number}: {e}")
            import traceback
            traceback.print_exc()
            return None
    
    def _get_full_transaction_data(self, tx_hash):
        """Get full transaction data by hash - ensures ALL fields are returned"""
        if not tx_hash:
            return None
        
        methods = [
            ('gettransaction', [tx_hash, True]),  # verbose
            ('gettransaction', [tx_hash]),
            ('getrawtransaction', [tx_hash, True]),  # verbose
            ('getrawtransaction', [tx_hash]),
            ('gxc_getTransaction', [tx_hash, True]),  # verbose
            ('gxc_getTransaction', [tx_hash]),
            ('gettx', [tx_hash, True]),  # verbose
            ('gettx', [tx_hash])
        ]
        
        for method, params in methods:
            result = rpc_call(method, params=params, timeout=15, show_errors=False)
            if result:
                return result
        
        return None
    
    def store_block(self, block_data):
        """Store block data in database"""
        if not block_data:
            return
        
        # Add timeout to prevent database locking issues
        conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
        cursor = conn.cursor()
        
        try:
            # Handle testnet node format (height, hash, prev_hash, timestamp, difficulty, nonce, miner)
            # Also handle explorer format with more fields
            block_number = block_data.get('height') or block_data.get('block_number') or block_data.get('number') or 0
            block_hash = block_data.get('hash') or block_data.get('block_hash') or ''
            parent_hash = block_data.get('prev_hash') or block_data.get('parent_hash') or block_data.get('previousblockhash') or ''
            timestamp = block_data.get('timestamp') or int(time.time())
            miner_address = block_data.get('miner') or block_data.get('miner_address') or ''
            difficulty = block_data.get('difficulty') or 0.1
            nonce = block_data.get('nonce') or 0
            transaction_count = block_data.get('transaction_count') or block_data.get('tx_count') or 0
            
            # Determine block type
            block_type_map = {'pow_sha256': 0, 'pow_ethash': 1, 'pos': 2}
            consensus = block_data.get('consensusType') or block_data.get('consensus_type') or 'pow'
            block_type = block_type_map.get(consensus.lower(), 0)
            
            # Check if block already exists
            cursor.execute('SELECT block_number FROM blocks WHERE block_number = ?', (block_number,))
            if cursor.fetchone():
                # Block exists, update it
                cursor.execute('''
                    UPDATE blocks SET
                        block_hash = ?, parent_hash = ?, timestamp = ?,
                        miner_address = ?, difficulty = ?, nonce = ?,
                        transaction_count = ?
                    WHERE block_number = ?
                ''', (block_hash, parent_hash, timestamp, miner_address, difficulty, nonce, transaction_count, block_number))
            else:
                # New block, insert it
                # Handle both testnet format and full format
                merkle_root = block_data.get('merkleRoot') or block_data.get('merkle_root') or ''
                total_difficulty = block_data.get('totalDifficulty') or block_data.get('total_difficulty') or difficulty
                block_size = block_data.get('size') or 0
                gas_used = block_data.get('gasUsed') or block_data.get('gas_used') or 0
                gas_limit = block_data.get('gasLimit') or block_data.get('gas_limit') or 0
                reward = block_data.get('reward') or 12.5  # Default testnet reward
                
                # Convert timestamp to datetime if needed
                if isinstance(timestamp, (int, float)):
                    from datetime import datetime
                    timestamp_dt = datetime.fromtimestamp(timestamp)
                elif isinstance(timestamp, str):
                    try:
                        timestamp_dt = datetime.fromisoformat(timestamp.replace('Z', '+00:00'))
                    except:
                        timestamp_dt = datetime.fromtimestamp(int(time.time()))
                else:
                    timestamp_dt = timestamp
                
                # Fix Python 3.12 datetime deprecation warning
                # Convert datetime to ISO format string for SQLite
                if isinstance(timestamp_dt, datetime):
                    timestamp_str = timestamp_dt.isoformat()
                else:
                    timestamp_str = str(timestamp_dt)
                
                cursor.execute('''
                    INSERT INTO blocks (
                        block_number, block_hash, parent_hash, merkle_root, timestamp,
                        miner_address, difficulty, total_difficulty, nonce, size,
                        gas_used, gas_limit, transaction_count, reward,
                        consensus_type, block_type, validator_signature,
                        pow_hash, pos_hash, fee_burn_rate, pop_reference
                    ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
            ''', (
                block_number,
                block_hash,
                parent_hash,
                merkle_root,
                timestamp_str,
                miner_address,
                float(difficulty),
                float(total_difficulty),
                int(nonce),
                int(block_size),
                int(gas_used),
                int(gas_limit),
                int(transaction_count),
                float(reward),
                consensus,
                block_type,
                block_data.get('validatorSignature') or block_data.get('validator_signature') or '',
                block_data.get('powHash') or block_data.get('pow_hash') or '',
                block_data.get('posHash') or block_data.get('pos_hash') or '',
                float(block_data.get('feeBurnRate') or block_data.get('fee_burn_rate') or 0.0),
                block_data.get('popReference') or block_data.get('pop_reference') or ''
            ))
            
            # Store transactions - handle both list and single transaction
            transactions = block_data.get('transactions') or []
            if isinstance(transactions, dict):
                transactions = [transactions]
            elif not isinstance(transactions, list):
                transactions = []
            
            for i, tx in enumerate(transactions):
                if tx:  # Only store if transaction data exists
                    try:
                        # Pass block_data context for coinbase transactions
                        self.store_transaction(tx, block_number, i, block_data)
                    except Exception as e:
                        print(f"Error storing transaction {i} in block {block_number}: {e}")
                        import traceback
                        traceback.print_exc()
                        continue  # Continue with next transaction
            
            conn.commit()
            
        except sqlite3.OperationalError as e:
            if "database is locked" in str(e).lower():
                # Retry once after a short delay
                import time
                time.sleep(0.1)
                try:
                    conn.rollback()
                    conn.close()
                    print(f"Error storing block (database locked): {e}")
                except:
                    print(f"Error storing block (database locked): {e}")
            else:
                conn.rollback()
                print(f"Error storing block: {e}")
        except Exception as e:
            conn.rollback()
            print(f"Error storing block: {e}")
        finally:
            try:
                conn.close()
            except:
                pass
    
    def store_transaction(self, tx_data, block_number, tx_index, block_data=None):
        """Store transaction data with robust field handling"""
        # Use timeout to handle database locking
        conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
        cursor = conn.cursor()
        # Enable WAL mode for better concurrency
        cursor.execute('PRAGMA journal_mode=WAL')
        cursor.execute('PRAGMA synchronous=NORMAL')
        cursor.execute('PRAGMA busy_timeout=30000')
        
        try:
            # Normalize transaction data - handle various field name formats
            tx_hash = tx_data.get('hash') or tx_data.get('tx_hash') or tx_data.get('transactionHash') or ''
            if not tx_hash:
                # Generate hash if missing (for coinbase transactions)
                import hashlib
                tx_hash = hashlib.sha256(str(tx_data).encode()).hexdigest()
            
            # Handle coinbase transactions (mining rewards) - they may not have 'from' address
            is_coinbase = tx_data.get('is_coinbase') or tx_data.get('isCoinbase') or tx_data.get('coinbase') or False
            
            # Auto-detect coinbase: first transaction (index 0) in block, or no 'from' address but has miner
            if not is_coinbase and tx_index == 0 and block_data:
                # First transaction in block is likely coinbase
                is_coinbase = True
            elif not is_coinbase:
                # Check if transaction has no 'from' but goes to miner
                from_addr_check = tx_data.get('from') or tx_data.get('from_address') or tx_data.get('fromAddress') or ''
                miner_addr_check = (block_data.get('miner_address') if block_data else '') or (block_data.get('miner') if block_data else '')
                to_addr_check = tx_data.get('to') or tx_data.get('to_address') or tx_data.get('toAddress') or None
                if not from_addr_check and to_addr_check == miner_addr_check and miner_addr_check:
                    is_coinbase = True
            
            from_addr = tx_data.get('from') or tx_data.get('from_address') or tx_data.get('fromAddress') or ''
            if is_coinbase and not from_addr:
                from_addr = 'coinbase'  # Use 'coinbase' as from address for mining rewards
            
            to_addr = tx_data.get('to') or tx_data.get('to_address') or tx_data.get('toAddress') or None
            # For coinbase, use miner address as recipient
            if is_coinbase and not to_addr:
                miner_addr = (tx_data.get('miner') or tx_data.get('miner_address') or 
                             (block_data.get('miner_address') if block_data else '') or
                             (block_data.get('miner') if block_data else ''))
                to_addr = miner_addr if miner_addr else ''
            
            value = tx_data.get('value') or tx_data.get('amount') or 0.0
            fee = tx_data.get('fee') or tx_data.get('gasPrice', 0.0) * tx_data.get('gasUsed', 0) or 0.0
            gas_price = tx_data.get('gasPrice') or tx_data.get('gas_price') or 0.0
            gas_used = tx_data.get('gasUsed') or tx_data.get('gas_used') or 0
            status = tx_data.get('status') or 'success'
            tx_type = 'coinbase' if is_coinbase else (tx_data.get('type') or tx_data.get('tx_type') or 'transfer')
            input_data = tx_data.get('input') or tx_data.get('input_data') or tx_data.get('data') or ''
            nonce = tx_data.get('nonce') or 0
            contract_addr = tx_data.get('contractAddress') or tx_data.get('contract_address') or None
            
            # Get timestamp - prefer from tx_data, fallback to current time
            if 'timestamp' in tx_data:
                if isinstance(tx_data['timestamp'], (int, float)):
                    tx_timestamp = datetime.fromtimestamp(tx_data['timestamp'])
                elif isinstance(tx_data['timestamp'], str):
                    try:
                        tx_timestamp = datetime.fromisoformat(tx_data['timestamp'].replace('Z', '+00:00'))
                    except:
                        tx_timestamp = datetime.utcnow()
                else:
                    tx_timestamp = tx_data['timestamp']
            else:
                tx_timestamp = datetime.utcnow()
            
            # Convert datetime to string for Python 3.12 compatibility (fixes deprecation warning)
            if isinstance(tx_timestamp, datetime):
                tx_timestamp_str = tx_timestamp.isoformat()
            elif isinstance(tx_timestamp, (int, float)):
                # Handle Unix timestamp
                tx_timestamp_str = datetime.fromtimestamp(tx_timestamp).isoformat()
            else:
                tx_timestamp_str = str(tx_timestamp) if tx_timestamp else datetime.utcnow().isoformat()
            
            # Use WAL mode for better concurrency and reduce locking
            cursor.execute('PRAGMA journal_mode=WAL')
            
            cursor.execute('''
                INSERT OR REPLACE INTO transactions (
                    tx_hash, block_number, tx_index, from_address,
                    to_address, value, fee, gas_price, gas_used,
                    status, timestamp, input_data, nonce, tx_type,
                    contract_address, is_coinbase
                ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
            ''', (
                tx_hash,
                block_number,
                tx_index,
                from_addr,
                to_addr,
                float(value),
                float(fee),
                float(gas_price),
                int(gas_used),
                status,
                tx_timestamp_str,
                input_data,
                int(nonce),
                tx_type,
                contract_addr,
                1 if is_coinbase else 0
            ))
            
            # Emit WebSocket event for new transaction
            try:
                if socketio:
                    emit_tx = {
                        'hash': tx_hash,
                        'tx_hash': tx_hash,
                        'block_number': block_number,
                        'from_address': from_addr,
                        'to_address': to_addr,
                        'value': float(value),
                        'fee': float(fee),
                        'status': status,
                        'timestamp': tx_timestamp.isoformat() if hasattr(tx_timestamp, 'isoformat') else str(tx_timestamp),
                        'is_coinbase': is_coinbase,
                        'tx_type': tx_type
                    }
                    socketio.emit('new_transaction', emit_tx)
            except Exception as e:
                print(f"[EXPLORER] Error emitting transaction: {e}")
                pass  # SocketIO might not be initialized
            
            # Update address balances and activity (handle coinbase transactions)
            from_addr_for_update = tx_data.get('from') or tx_data.get('from_address') or from_addr
            to_addr_for_update = tx_data.get('to') or tx_data.get('to_address') or to_addr
            if from_addr_for_update or to_addr_for_update:
                self.update_address_activity(from_addr_for_update, to_addr_for_update)
            
            # Update address relationships for graph (skip for coinbase)
            if to_addr_for_update and not is_coinbase:
                self.update_address_relationships(from_addr_for_update, to_addr_for_update, float(value))
            
            # Track gold token transfers
            if tx_data.get('isGoldBacked') or tx_data.get('is_gold_backed'):
                self.track_gold_token_transfer(tx_data, block_number)
            
            conn.commit()
            
        except sqlite3.OperationalError as e:
            if "database is locked" in str(e).lower():
                # Retry once after a short delay
                import time
                time.sleep(0.1)
                try:
                    conn.rollback()
                    conn.close()
                    conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
                    cursor = conn.cursor()
                    # Retry the insert (simplified - just log error for now)
                    print(f"Error storing transaction (database locked, retry failed): {e}")
                except:
                    print(f"Error storing transaction (database locked): {e}")
            else:
                conn.rollback()
                print(f"Error storing transaction: {e}")
        except Exception as e:
            conn.rollback()
            print(f"Error storing transaction: {e}")
        finally:
            try:
                conn.close()
            except:
                pass
    
    def update_address_activity(self, from_addr, to_addr):
        """Update address activity records"""
        # Add timeout to prevent database locking issues
        conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
        cursor = conn.cursor()
        
        try:
            for addr in [from_addr, to_addr]:
                if addr:
                    cursor.execute('''
                        INSERT OR IGNORE INTO addresses (
                            address, first_seen, last_activity
                        ) VALUES (?, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP)
                    ''', (addr,))
                    
                    # Get current balance for address update event
                    cursor.execute('SELECT balance FROM addresses WHERE address = ?', (addr,))
                    balance_row = cursor.fetchone()
                    current_balance = balance_row[0] if balance_row else 0.0
                    
                    cursor.execute('''
                        UPDATE addresses 
                        SET last_activity = CURRENT_TIMESTAMP,
                            transaction_count = transaction_count + 1
                        WHERE address = ?
                    ''', (addr,))
            
            conn.commit()
            
        except Exception as e:
            conn.rollback()
            print(f"Error updating address activity: {e}")
        finally:
            conn.close()
    
    def get_block_by_number_from_db(self, block_number):
        """Get a specific block from database by number with ALL fields"""
        # Add timeout to prevent database locking issues
        conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
        cursor = conn.cursor()
        
        try:
            # Get ALL available fields from blocks table
            cursor.execute('''
                SELECT block_number, block_hash, parent_hash, timestamp, miner_address,
                       transaction_count, reward, consensus_type, difficulty, total_difficulty,
                       size, gas_used, gas_limit, nonce, merkle_root
                FROM blocks
                WHERE block_number = ?
            ''', (block_number,))
            
            row = cursor.fetchone()
            if row:
                return {
                    'number': row[0] if row[0] is not None else 0,
                    'block_number': row[0] if row[0] is not None else 0,
                    'hash': row[1] if row[1] else '',
                    'block_hash': row[1] if row[1] else '',
                    'parent_hash': row[2] if len(row) > 2 and row[2] else '',
                    'timestamp': row[3] if row[3] else datetime.now(),
                    'miner': row[4] if len(row) > 4 and row[4] else '',
                    'miner_address': row[4] if len(row) > 4 and row[4] else '',
                    'tx_count': row[5] if len(row) > 5 and row[5] is not None else 0,
                    'transaction_count': row[5] if len(row) > 5 and row[5] is not None else 0,
                    'reward': row[6] if len(row) > 6 and row[6] is not None else 12.5,
                    'consensus_type': (row[7] if len(row) > 7 and row[7] else 'pow')[:3],  # Ensure max 3 chars (pow/pos)
                    'consensusType': (row[7] if len(row) > 7 and row[7] else 'pow')[:3],  # Ensure max 3 chars
                    'difficulty': row[8] if len(row) > 8 and row[8] is not None else 0.1,
                    'total_difficulty': row[9] if len(row) > 9 and row[9] is not None else (row[8] if len(row) > 8 and row[8] is not None else 0.1),
                    'size': row[10] if len(row) > 10 and row[10] is not None else 0,
                    'gas_used': row[11] if len(row) > 11 and row[11] is not None else 0,
                    'gas_limit': row[12] if len(row) > 12 and row[12] is not None else 0,
                    'nonce': row[13] if len(row) > 13 and row[13] is not None else 0,
                    'merkle_root': row[14] if len(row) > 14 and row[14] else '',
                    'transactions': []  # Transactions are stored separately
                }
            return None
            
        except Exception as e:
            print(f"Error getting block from database: {e}")
            import traceback
            traceback.print_exc()
            return None
        finally:
            conn.close()
    
    def get_recent_blocks(self, limit=10):
        """Get recent blocks from database with all fields"""
        # Add timeout to prevent database locking issues
        conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
        cursor = conn.cursor()
        # Enable WAL mode for better concurrency
        cursor.execute('PRAGMA journal_mode=WAL')
        cursor.execute('PRAGMA synchronous=NORMAL')
        cursor.execute('PRAGMA busy_timeout=30000')
        
        try:
            # Get all available fields from blocks table
            cursor.execute('''
                SELECT block_number, block_hash, parent_hash, timestamp, miner_address,
                       transaction_count, reward, consensus_type, difficulty, total_difficulty,
                       size, gas_used, gas_limit, nonce, merkle_root
                FROM blocks
                ORDER BY block_number DESC
                LIMIT ?
            ''', (limit,))
            
            blocks = []
            for row in cursor.fetchall():
                blocks.append({
                    'number': row[0] if row[0] is not None else 0,
                    'block_number': row[0] if row[0] is not None else 0,
                    'hash': row[1] if row[1] else '',
                    'block_hash': row[1] if row[1] else '',
                    'parent_hash': row[2] if len(row) > 2 and row[2] else '',
                    'timestamp': row[3] if row[3] else datetime.now(),
                    'miner': row[4] if len(row) > 4 and row[4] else '',
                    'miner_address': row[4] if len(row) > 4 and row[4] else '',
                    'tx_count': row[5] if len(row) > 5 and row[5] is not None else 0,
                    'transaction_count': row[5] if len(row) > 5 and row[5] is not None else 0,
                    'reward': row[6] if len(row) > 6 and row[6] is not None else 12.5,
                    'consensus_type': row[7] if len(row) > 7 and row[7] else 'pow',
                    'difficulty': row[8] if len(row) > 8 and row[8] is not None else 0.1,
                    'total_difficulty': row[9] if len(row) > 9 and row[9] is not None else (row[8] if len(row) > 8 and row[8] is not None else 0.1),
                    'size': row[10] if len(row) > 10 and row[10] is not None else 0,
                    'gas_used': row[11] if len(row) > 11 and row[11] is not None else 0,
                    'gas_limit': row[12] if len(row) > 12 and row[12] is not None else 0,
                    'nonce': row[13] if len(row) > 13 and row[13] is not None else 0,
                    'merkle_root': row[14] if len(row) > 14 and row[14] else ''
                })
            
            return blocks
            
        except Exception as e:
            print(f"Error getting recent blocks: {e}")
            import traceback
            traceback.print_exc()
            return []
        finally:
            conn.close()
    
    def get_recent_transactions(self, limit=20):
        """Get recent transactions from database with all fields including confirmations"""
        # Add timeout to prevent database locking issues
        conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
        cursor = conn.cursor()
        
        try:
            # Get current height for confirmations
            current_height = self.get_current_block_height()
            
            # Get all available fields from transactions table
            cursor.execute('''
                SELECT tx_hash, block_number, from_address, to_address,
                       value, fee, status, timestamp, tx_type, gas_price, gas_used,
                       nonce, input_data, contract_address, prev_tx_hash, referenced_amount,
                       traceability_valid, is_coinbase
                FROM transactions
                ORDER BY block_number DESC, COALESCE(tx_index, 0) DESC, timestamp DESC
                LIMIT ?
            ''', (limit,))
            
            transactions = []
            for row in cursor.fetchall():
                # Ensure all fields have defaults and return complete data
                transactions.append({
                    'hash': row[0] if row[0] else '',
                    'tx_hash': row[0] if row[0] else '',
                    'block': row[1] if row[1] is not None else 0,
                    'block_number': row[1] if row[1] is not None else 0,
                    'from': row[2] if row[2] else '',
                    'from_address': row[2] if row[2] else '',
                    'to': row[3] if row[3] else None,
                    'to_address': row[3] if row[3] else None,
                    'value': row[4] if row[4] is not None else 0.0,
                    'fee': row[5] if row[5] is not None else 0.0,
                    'status': row[6] if row[6] else 'success',
                    'timestamp': row[7] if row[7] else datetime.now(),
                    'type': row[8] if len(row) > 8 and row[8] else 'transfer',
                    'tx_type': row[8] if len(row) > 8 and row[8] else 'transfer',
                    'gas_price': row[9] if len(row) > 9 and row[9] is not None else 0.0,
                    'gas_used': row[10] if len(row) > 10 and row[10] is not None else 0,
                    'nonce': row[11] if len(row) > 11 and row[11] is not None else 0,
                    'input_data': row[12] if len(row) > 12 and row[12] else '',
                    'data': row[12] if len(row) > 12 and row[12] else '',
                    'contract_address': row[13] if len(row) > 13 and row[13] else None,
                    'prev_tx_hash': row[14] if len(row) > 14 and row[14] else None,
                    'referenced_amount': row[15] if len(row) > 15 and row[15] is not None else 0.0,
                    'traceability_valid': row[16] if len(row) > 16 and row[16] is not None else True,
                    'is_coinbase': row[17] if len(row) > 17 and row[17] is not None else False,
                    'confirmations': max(0, current_height - (row[1] if len(row) > 1 and row[1] else 0) + 1) if (row[1] if len(row) > 1 and row[1] else 0) > 0 else 0
                })
            
            return transactions
            
        except Exception as e:
            print(f"Error getting recent transactions: {e}")
            import traceback
            traceback.print_exc()
            return []
        finally:
            conn.close()
    
    def get_network_stats(self):
        """Get current network statistics with all fields"""
        # Add timeout to prevent database locking issues
        conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
        cursor = conn.cursor()
        
        try:
            # Get latest stats
            cursor.execute('''
                SELECT total_blocks, total_transactions, total_addresses,
                       total_supply, hash_rate, difficulty, avg_block_time, network_health
                FROM network_stats
                ORDER BY timestamp DESC
                LIMIT 1
            ''')
            
            stats = cursor.fetchone()
            if stats:
                return {
                    'total_blocks': stats[0] if stats[0] is not None else 0,
                    'total_transactions': stats[1] if stats[1] is not None else 0,
                    'total_addresses': stats[2] if stats[2] is not None else 0,
                    'total_supply': stats[3] if stats[3] is not None else 31000000,
                    'hash_rate': stats[4] if stats[4] is not None else 0.0,
                    'difficulty': stats[5] if stats[5] is not None else 0.0,
                    'avg_block_time': stats[6] if stats[6] is not None else 0.0,
                    'network_health': stats[7] if len(stats) > 7 and stats[7] is not None else 100.0
                }
            
            # Calculate stats if none exist
            cursor.execute('SELECT COUNT(*) FROM blocks')
            total_blocks = cursor.fetchone()[0] or 0
            
            cursor.execute('SELECT COUNT(*) FROM transactions')
            total_transactions = cursor.fetchone()[0] or 0
            
            cursor.execute('SELECT COUNT(*) FROM addresses')
            total_addresses = cursor.fetchone()[0] or 0
            
            return {
                'total_blocks': total_blocks,
                'total_transactions': total_transactions,
                'total_addresses': total_addresses,
                'total_supply': 31000000,  # Max supply
                'hash_rate': 0.0,
                'difficulty': 0.0,
                'avg_block_time': 0.0,
                'network_health': 100.0
            }
            
        except Exception as e:
            print(f"Error getting network stats: {e}")
            # Return default stats if error
            return {
                'total_blocks': 0,
                'total_transactions': 0,
                'total_addresses': 0,
                'total_supply': 31000000,
                'hash_rate': 0.0,
                'difficulty': 0.0,
                'avg_block_time': 0.0
            }
        finally:
            conn.close()
    
    def get_mining_stats(self):
        """Get mining statistics including top miners"""
        # Add timeout to prevent database locking issues
        conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
        cursor = conn.cursor()
        
        try:
            # Get top miners by block count
            cursor.execute('''
                SELECT miner_address, COUNT(*) as block_count,
                       SUM(reward) as total_reward
                FROM blocks
                GROUP BY miner_address
                ORDER BY block_count DESC
                LIMIT 10
            ''')
            
            top_miners = []
            for row in cursor.fetchall():
                top_miners.append({
                    'address': row[0],
                    'block_count': row[1],
                    'total_reward': row[2]
                })
            
            # Get latest difficulty and hash rate estimate
            cursor.execute('SELECT difficulty FROM blocks ORDER BY block_number DESC LIMIT 1')
            difficulty_row = cursor.fetchone()
            difficulty = difficulty_row[0] if difficulty_row else 0.0
            
            # Estimate hash rate (simplified calculation)
            cursor.execute('''
                SELECT AVG(
                    (julianday(b2.timestamp) - julianday(b1.timestamp)) * 86400
                ) as avg_time
                FROM blocks b1
                JOIN blocks b2 ON b2.block_number = b1.block_number + 1
                WHERE b1.block_number >= (SELECT MAX(block_number) - 100 FROM blocks)
            ''')
            avg_time_row = cursor.fetchone()
            avg_block_time = avg_time_row[0] if avg_time_row and avg_time_row[0] else 60.0
            
            # Simple hash rate estimate: difficulty / target_block_time
            hash_rate = difficulty / max(avg_block_time, 1.0) if difficulty > 0 else 0.0
            
            return {
                'top_miners': top_miners,
                'total_miners': len(top_miners),
                'current_difficulty': difficulty,
                'estimated_hash_rate': hash_rate,
                'avg_block_time': avg_block_time
            }
            
        except Exception as e:
            print(f"Error getting mining stats: {e}")
            return {
                'top_miners': [],
                'total_miners': 0,
                'current_difficulty': 0.0,
                'estimated_hash_rate': 0.0,
                'avg_block_time': 0.0
            }
        finally:
            conn.close()
    
    def get_validators(self):
        """Get validator information from blocks"""
        # Add timeout to prevent database locking issues
        conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
        cursor = conn.cursor()
        
        try:
            # Get validators from blocks (miners who have mined blocks)
            cursor.execute('''
                SELECT miner_address, COUNT(*) as blocks_mined,
                       SUM(reward) as total_reward,
                       MAX(timestamp) as last_seen
                FROM blocks
                WHERE consensus_type = 'pos' OR consensus_type = 'dpos'
                GROUP BY miner_address
                ORDER BY blocks_mined DESC
                LIMIT 50
            ''')
            
            validators = []
            for row in cursor.fetchall():
                validators.append({
                    'address': row[0],
                    'blocks_mined': row[1],
                    'total_reward': row[2] or 0.0,
                    'last_seen': row[3],
                    'stake': row[2] or 0.0  # Use reward as stake estimate
                })
            
            # Calculate total staked
            total_staked = sum(v['stake'] for v in validators)
            
            return {
                'validators': validators,
                'total_staked': total_staked,
                'total_validators': len(validators)
            }
            
        except Exception as e:
            print(f"Error getting validators: {e}")
            return {
                'validators': [],
                'total_staked': 0.0,
                'total_validators': 0
            }
        finally:
            conn.close()
    
    def get_top_addresses(self, limit=100):
        """Get top addresses by balance"""
        # Add timeout to prevent database locking issues
        conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
        cursor = conn.cursor()
        
        try:
            # Get addresses with their transaction counts
            cursor.execute('''
                SELECT address, balance, transaction_count, address_type
                FROM addresses
                WHERE balance > 0
                ORDER BY balance DESC
                LIMIT ?
            ''', (limit,))
            
            addresses = []
            for row in cursor.fetchall():
                addresses.append({
                    'address': row[0],
                    'balance': row[1] or 0.0,
                    'tx_count': row[2] or 0,
                    'type': row[3] or 'user'
                })
            
            # Get total addresses count
            cursor.execute('SELECT COUNT(*) FROM addresses')
            total_addresses = cursor.fetchone()[0]
            
            return {
                'addresses': addresses,
                'total_addresses': total_addresses,
                'total_supply': 31000000  # Max supply
            }
            
        except Exception as e:
            print(f"Error getting top addresses: {e}")
            return {
                'addresses': [],
                'total_addresses': 0,
                'total_supply': 31000000
            }
        finally:
            conn.close()
    
    def get_chart_data(self, days=14):
        """Get chart data for analytics"""
        # Add timeout to prevent database locking issues
        conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
        cursor = conn.cursor()
        
        try:
            # Get daily transaction counts
            cursor.execute('''
                SELECT date(timestamp) as date, COUNT(*) as tx_count
                FROM transactions
                WHERE timestamp >= datetime('now', '-' || ? || ' days')
                GROUP BY date(timestamp)
                ORDER BY date ASC
            ''', (days,))
            
            tx_data = cursor.fetchall()
            
            # Get daily address growth
            cursor.execute('''
                SELECT date(first_seen) as date, COUNT(*) as new_addresses
                FROM addresses
                WHERE first_seen >= datetime('now', '-' || ? || ' days')
                GROUP BY date(first_seen)
                ORDER BY date ASC
            ''', (days,))
            
            address_data = cursor.fetchall()
            
            # Get daily block time averages
            cursor.execute('''
                SELECT date(b1.timestamp) as date, AVG(
                    (julianday(b2.timestamp) - julianday(b1.timestamp)) * 86400
                ) as avg_time
                FROM blocks b1
                JOIN blocks b2 ON b2.block_number = b1.block_number + 1
                WHERE b1.timestamp >= datetime('now', '-' || ? || ' days')
                GROUP BY date(b1.timestamp)
                ORDER BY date ASC
            ''', (days,))
            
            block_time_data = cursor.fetchall()
            
            # Get daily difficulty
            cursor.execute('''
                SELECT date(timestamp) as date, AVG(difficulty) as avg_difficulty
                FROM blocks
                WHERE timestamp >= datetime('now', '-' || ? || ' days')
                GROUP BY date(timestamp)
                ORDER BY date ASC
            ''', (days,))
            
            difficulty_data = cursor.fetchall()
            
            return {
                'transactions': tx_data,
                'addresses': address_data,
                'block_times': block_time_data,
                'difficulty': difficulty_data
            }
            
        except Exception as e:
            print(f"Error getting chart data: {e}")
            return {
                'transactions': [],
                'addresses': [],
                'block_times': [],
                'difficulty': []
            }
        finally:
            conn.close()
    
    def get_hashrate_history(self, days=7):
        """Get hashrate history for mining chart"""
        # Add timeout to prevent database locking issues
        conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
        cursor = conn.cursor()
        
        try:
            # Get daily hashrate estimates based on difficulty and block times
            cursor.execute('''
                SELECT 
                    date(b1.timestamp) as date,
                    AVG(b1.difficulty) as avg_difficulty,
                    AVG((julianday(b2.timestamp) - julianday(b1.timestamp)) * 86400) as avg_block_time
                FROM blocks b1
                JOIN blocks b2 ON b2.block_number = b1.block_number + 1
                WHERE b1.timestamp >= datetime('now', '-' || ? || ' days')
                GROUP BY date(b1.timestamp)
                ORDER BY date ASC
            ''', (days,))
            
            hashrate_data = []
            for row in cursor.fetchall():
                date_str = row[0]
                difficulty = row[1] or 0.0
                avg_time = row[2] or 60.0
                # Calculate hashrate: difficulty / block_time
                hashrate = difficulty / max(avg_time, 1.0) if difficulty > 0 else 0.0
                hashrate_data.append([date_str, hashrate])
            
            return hashrate_data
            
        except Exception as e:
            print(f"Error getting hashrate history: {e}")
            return []
        finally:
            conn.close()
    
    def get_pending_transactions(self, limit=100):
        """Get pending transactions from mempool"""
        # Add timeout to prevent database locking issues
        conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
        cursor = conn.cursor()
        
        try:
            cursor.execute('''
                SELECT tx_hash, from_address, to_address, value, fee,
                       gas_price, timestamp, input_data, nonce, tx_type
                FROM pending_transactions
                ORDER BY timestamp DESC
                LIMIT ?
            ''', (limit,))
            
            transactions = []
            for row in cursor.fetchall():
                transactions.append({
                    'hash': row[0],
                    'from': row[1],
                    'to': row[2],
                    'value': row[3],
                    'fee': row[4],
                    'gas_price': row[5],
                    'timestamp': row[6],
                    'input': row[7],
                    'nonce': row[8],
                    'type': row[9]
                })
            
            return transactions
            
        except Exception as e:
            print(f"Error getting pending transactions: {e}")
            return []
        finally:
            conn.close()
    
    def add_pending_transaction(self, tx_data):
        """Add transaction to pending pool"""
        # Add timeout to prevent database locking issues
        conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
        cursor = conn.cursor()
        
        try:
            cursor.execute('''
                INSERT OR REPLACE INTO pending_transactions (
                    tx_hash, from_address, to_address, value, fee,
                    gas_price, timestamp, input_data, nonce, tx_type
                ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
            ''', (
                tx_data.get('hash'),
                tx_data.get('from'),
                tx_data.get('to'),
                float(tx_data.get('value', 0)),
                float(tx_data.get('fee', 0)),
                float(tx_data.get('gasPrice', 0)),
                datetime.utcnow(),
                tx_data.get('input'),
                int(tx_data.get('nonce', 0)),
                tx_data.get('type', 'transfer')
            ))
            
            conn.commit()
            return True
            
        except Exception as e:
            conn.rollback()
            print(f"Error adding pending transaction: {e}")
            return False
        finally:
            conn.close()
    
    def remove_pending_transaction(self, tx_hash):
        """Remove transaction from pending pool (when confirmed)"""
        # Add timeout to prevent database locking issues
        conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
        cursor = conn.cursor()
        
        try:
            cursor.execute('DELETE FROM pending_transactions WHERE tx_hash = ?', (tx_hash,))
            conn.commit()
            return True
            
        except Exception as e:
            conn.rollback()
            print(f"Error removing pending transaction: {e}")
            return False
        finally:
            conn.close()
    
    def get_gas_stats(self):
        """Get gas price statistics and recommendations"""
        # Add timeout to prevent database locking issues
        conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
        cursor = conn.cursor()
        
        try:
            # Get recent gas prices (last 100 transactions)
            cursor.execute('''
                SELECT gas_price, fee, gas_used
                FROM transactions
                WHERE gas_price > 0
                ORDER BY block_number DESC
                LIMIT 100
            ''')
            
            gas_data = cursor.fetchall()
            
            if not gas_data:
                return {
                    'slow': 0.0001,
                    'standard': 0.0002,
                    'fast': 0.0005,
                    'average': 0.0002,
                    'median': 0.0002,
                    'history': []
                }
            
            # Calculate statistics
            gas_prices = [row[0] for row in gas_data if row[0] > 0]
            fees = [row[1] for row in gas_data if row[1] > 0]
            
            if gas_prices:
                sorted_prices = sorted(gas_prices)
                median = sorted_prices[len(sorted_prices) // 2]
                average = sum(gas_prices) / len(gas_prices)
                
                # Percentiles for recommendations
                slow = sorted_prices[int(len(sorted_prices) * 0.1)] if len(sorted_prices) > 10 else median * 0.5
                standard = median
                fast = sorted_prices[int(len(sorted_prices) * 0.9)] if len(sorted_prices) > 10 else median * 2
                
                # Get hourly gas price history (last 24 hours)
                cursor.execute('''
                    SELECT 
                        strftime('%Y-%m-%d %H:00:00', timestamp) as hour,
                        AVG(gas_price) as avg_gas
                    FROM transactions
                    WHERE gas_price > 0 
                        AND timestamp >= datetime('now', '-24 hours')
                    GROUP BY hour
                    ORDER BY hour ASC
                ''')
                
                history = cursor.fetchall()
                
                return {
                    'slow': slow,
                    'standard': standard,
                    'fast': fast,
                    'average': average,
                    'median': median,
                    'history': history,
                    'total_samples': len(gas_prices)
                }
            else:
                return {
                    'slow': 0.0001,
                    'standard': 0.0002,
                    'fast': 0.0005,
                    'average': 0.0002,
                    'median': 0.0002,
                    'history': [],
                    'total_samples': 0
                }
            
        except Exception as e:
            print(f"Error getting gas stats: {e}")
            return {
                'slow': 0.0001,
                'standard': 0.0002,
                'fast': 0.0005,
                'average': 0.0002,
                'median': 0.0002,
                'history': [],
                'total_samples': 0
            }
        finally:
            conn.close()
    
    def get_price_data(self):
        """Get current price and market data"""
        # Add timeout to prevent database locking issues
        conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
        cursor = conn.cursor()
        
        try:
            # Get latest price
            cursor.execute('''
                SELECT price_usd, price_btc, volume_24h, market_cap, supply_circulating, timestamp
                FROM price_history
                ORDER BY timestamp DESC
                LIMIT 1
            ''')
            
            latest = cursor.fetchone()
            
            # Get 24h price history
            cursor.execute('''
                SELECT timestamp, price_usd, volume_24h
                FROM price_history
                WHERE timestamp >= datetime('now', '-24 hours')
                ORDER BY timestamp ASC
            ''')
            
            history_24h = cursor.fetchall()
            
            # Get 7-day history
            cursor.execute('''
                SELECT date(timestamp) as date, AVG(price_usd) as avg_price, SUM(volume_24h) as total_volume
                FROM price_history
                WHERE timestamp >= datetime('now', '-7 days')
                GROUP BY date
                ORDER BY date ASC
            ''')
            
            history_7d = cursor.fetchall()
            
            if latest:
                return {
                    'current_price': latest[0],
                    'price_btc': latest[1],
                    'volume_24h': latest[2],
                    'market_cap': latest[3],
                    'supply_circulating': latest[4],
                    'last_updated': latest[5],
                    'history_24h': history_24h,
                    'history_7d': history_7d
                }
            else:
                # Return default values if no price data
                stats = self.get_network_stats()
                total_supply = stats.get('total_supply', 31000000)
                
                return {
                    'current_price': 0.0,
                    'price_btc': 0.0,
                    'volume_24h': 0.0,
                    'market_cap': 0.0,
                    'supply_circulating': total_supply,
                    'last_updated': None,
                    'history_24h': [],
                    'history_7d': []
                }
            
        except Exception as e:
            print(f"Error getting price data: {e}")
            return {
                'current_price': 0.0,
                'price_btc': 0.0,
                'volume_24h': 0.0,
                'market_cap': 0.0,
                'supply_circulating': 31000000,
                'last_updated': None,
                'history_24h': [],
                'history_7d': []
            }
        finally:
            conn.close()
    
    def get_filtered_transactions(self, filters):
        """Get filtered transactions based on criteria"""
        # Add timeout to prevent database locking issues
        conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
        cursor = conn.cursor()
        
        try:
            query = 'SELECT tx_hash, block_number, from_address, to_address, value, fee, status, timestamp, tx_type FROM transactions WHERE 1=1'
            params = []
            
            # Date range filter
            if filters.get('date_from'):
                query += ' AND timestamp >= ?'
                params.append(filters['date_from'])
            
            if filters.get('date_to'):
                query += ' AND timestamp <= ?'
                params.append(filters['date_to'])
            
            # Amount range filter
            if filters.get('amount_min'):
                query += ' AND value >= ?'
                params.append(float(filters['amount_min']))
            
            if filters.get('amount_max'):
                query += ' AND value <= ?'
                params.append(float(filters['amount_max']))
            
            # Transaction type filter
            if filters.get('tx_type'):
                query += ' AND tx_type = ?'
                params.append(filters['tx_type'])
            
            # Status filter
            if filters.get('status'):
                query += ' AND status = ?'
                params.append(filters['status'])
            
            # Address filter
            if filters.get('address'):
                query += ' AND (from_address = ? OR to_address = ?)'
                params.extend([filters['address'], filters['address']])
            
            query += ' ORDER BY block_number DESC, tx_index DESC LIMIT ?'
            limit = min(int(filters.get('limit', 100)), 1000)
            params.append(limit)
            
            cursor.execute(query, params)
            
            transactions = []
            for row in cursor.fetchall():
                transactions.append({
                    'hash': row[0],
                    'block': row[1],
                    'from': row[2],
                    'to': row[3],
                    'value': row[4],
                    'fee': row[5],
                    'status': row[6],
                    'timestamp': row[7],
                    'type': row[8]
                })
            
            return transactions
            
        except Exception as e:
            print(f"Error getting filtered transactions: {e}")
            return []
        finally:
            conn.close()
    
    def get_portfolio_data(self, addresses):
        """Get portfolio data for multiple addresses"""
        # Add timeout to prevent database locking issues
        conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
        cursor = conn.cursor()
        
        try:
            portfolio = {
                'addresses': [],
                'total_balance': 0.0,
                'total_transactions': 0,
                'total_received': 0.0,
                'total_sent': 0.0
            }
            
            for address in addresses:
                # Get address balance
                cursor.execute('SELECT balance, transaction_count FROM addresses WHERE address = ?', (address,))
                addr_data = cursor.fetchone()
                
                balance = addr_data[0] if addr_data else 0.0
                tx_count = addr_data[1] if addr_data else 0
                
                # Get received amount
                cursor.execute('SELECT SUM(value) FROM transactions WHERE to_address = ?', (address,))
                received = cursor.fetchone()[0] or 0.0
                
                # Get sent amount
                cursor.execute('SELECT SUM(value) FROM transactions WHERE from_address = ?', (address,))
                sent = cursor.fetchone()[0] or 0.0
                
                portfolio['addresses'].append({
                    'address': address,
                    'balance': balance,
                    'transaction_count': tx_count,
                    'received': received,
                    'sent': sent
                })
                
                portfolio['total_balance'] += balance
                portfolio['total_transactions'] += tx_count
                portfolio['total_received'] += received
                portfolio['total_sent'] += sent
            
            return portfolio
            
        except Exception as e:
            print(f"Error getting portfolio data: {e}")
            return {
                'addresses': [],
                'total_balance': 0.0,
                'total_transactions': 0,
                'total_received': 0.0,
                'total_sent': 0.0
            }
        finally:
            conn.close()
    
    def get_current_block_height(self):
        """Get current blockchain height from database or node"""
        # Add timeout to prevent database locking issues
        conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
        cursor = conn.cursor()
        
        try:
            # Try database first
            cursor.execute('SELECT MAX(block_number) FROM blocks')
            result = cursor.fetchone()
            db_height = result[0] if result and result[0] is not None else 0
            
            # Also try to get from node for accuracy
            try:
                node_info = rpc_call('getblockchaininfo', timeout=5, show_errors=False)
                if node_info:
                    node_height = node_info.get('blocks') or node_info.get('height') or 0
                    # Return the higher of the two
                    return max(db_height, node_height)
            except:
                pass
            
            return db_height
        except Exception as e:
            print(f"Error getting current block height: {e}")
            return 0
        finally:
            conn.close()
    
    def get_network_health(self):
        """Get network health metrics"""
        # Add timeout to prevent database locking issues
        conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
        cursor = conn.cursor()
        
        try:
            # Get latest block
            cursor.execute('SELECT MAX(block_number), MAX(timestamp) FROM blocks')
            latest = cursor.fetchone()
            latest_block = latest[0] if latest[0] else 0
            latest_time = latest[1] if latest[1] else None
            
            # Calculate time since last block
            time_since_block = 0.0
            if latest_time:
                try:
                    last_time = datetime.fromisoformat(latest_time.replace('Z', '+00:00'))
                    time_since_block = (datetime.utcnow() - last_time.replace(tzinfo=None)).total_seconds()
                except:
                    pass
            
            # Get block production rate (last 24 hours)
            cursor.execute('''
                SELECT COUNT(*) 
                FROM blocks 
                WHERE timestamp >= datetime('now', '-24 hours')
            ''')
            blocks_24h = cursor.fetchone()[0]
            
            # Calculate expected vs actual blocks
            expected_blocks_24h = (24 * 60 * 60) / 150  # 150 seconds per block
            block_production_rate = (blocks_24h / expected_blocks_24h * 100) if expected_blocks_24h > 0 else 0
            
            # Get transaction rate
            cursor.execute('''
                SELECT COUNT(*) 
                FROM transactions 
                WHERE timestamp >= datetime('now', '-1 hour')
            ''')
            tx_per_hour = cursor.fetchone()[0]
            
            # Calculate network health score (0-100)
            health_score = 100.0
            if time_since_block > 300:  # More than 5 minutes since last block
                health_score -= 20
            if block_production_rate < 90:  # Less than 90% of expected blocks
                health_score -= 15
            if block_production_rate < 75:
                health_score -= 15
            
            health_score = max(0, min(100, health_score))
            
            return {
                'latest_block': latest_block,
                'time_since_block': time_since_block,
                'blocks_24h': blocks_24h,
                'expected_blocks_24h': expected_blocks_24h,
                'block_production_rate': block_production_rate,
                'tx_per_hour': tx_per_hour,
                'health_score': health_score,
                'status': 'healthy' if health_score >= 80 else 'degraded' if health_score >= 50 else 'unhealthy'
            }
            
        except Exception as e:
            print(f"Error getting network health: {e}")
            return {
                'latest_block': 0,
                'time_since_block': 0.0,
                'blocks_24h': 0,
                'expected_blocks_24h': 576,
                'block_production_rate': 0.0,
                'tx_per_hour': 0,
                'health_score': 0.0,
                'status': 'unknown'
            }
        finally:
            conn.close()
    
    def get_analytics(self):
        """Get advanced analytics data"""
        # Add timeout to prevent database locking issues
        conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
        cursor = conn.cursor()
        
        try:
            # Whale transactions (large value)
            cursor.execute('''
                SELECT tx_hash, from_address, to_address, value, timestamp
                FROM transactions
                WHERE value >= 1000
                ORDER BY value DESC
                LIMIT 20
            ''')
            
            whale_txs = []
            for row in cursor.fetchall():
                whale_txs.append({
                    'hash': row[0],
                    'from': row[1],
                    'to': row[2],
                    'value': row[3],
                    'timestamp': row[4]
                })
            
            # Top addresses by transaction count
            cursor.execute('''
                SELECT address, transaction_count, balance
                FROM addresses
                ORDER BY transaction_count DESC
                LIMIT 20
            ''')
            
            top_active = []
            for row in cursor.fetchall():
                top_active.append({
                    'address': row[0],
                    'tx_count': row[1],
                    'balance': row[2]
                })
            
            # Daily transaction volume
            cursor.execute('''
                SELECT date(timestamp) as date, COUNT(*) as tx_count, SUM(value) as total_value
                FROM transactions
                WHERE timestamp >= datetime('now', '-30 days')
                GROUP BY date
                ORDER BY date ASC
            ''')
            
            daily_volume = cursor.fetchall()
            
            # Address activity distribution
            cursor.execute('''
                SELECT 
                    CASE 
                        WHEN transaction_count = 0 THEN 'Inactive'
                        WHEN transaction_count < 10 THEN 'Low (1-9)'
                        WHEN transaction_count < 100 THEN 'Medium (10-99)'
                        WHEN transaction_count < 1000 THEN 'High (100-999)'
                        ELSE 'Very High (1000+)'
                    END as activity_level,
                    COUNT(*) as address_count
                FROM addresses
                GROUP BY activity_level
            ''')
            
            activity_dist = cursor.fetchall()
            
            return {
                'whale_transactions': whale_txs,
                'top_active_addresses': top_active,
                'daily_volume': daily_volume,
                'activity_distribution': activity_dist
            }
            
        except Exception as e:
            print(f"Error getting analytics: {e}")
            return {
                'whale_transactions': [],
                'top_active_addresses': [],
                'daily_volume': [],
                'activity_distribution': []
            }
        finally:
            conn.close()
    
    def decode_transaction_input(self, input_data):
        """Decode transaction input data"""
        try:
            if not input_data or input_data == '0x' or len(input_data) < 10:
                return {
                    'method': 'transfer',
                    'decoded': False,
                    'raw': input_data
                }
            
            # Extract function signature (first 4 bytes = 8 hex chars)
            if input_data.startswith('0x'):
                input_data = input_data[2:]
            
            if len(input_data) >= 8:
                function_sig = input_data[:8]
                
                # Common function signatures (simplified)
                known_signatures = {
                    'a9059cbb': 'transfer(address,uint256)',
                    '23b872dd': 'transferFrom(address,address,uint256)',
                    '095ea7b3': 'approve(address,uint256)',
                    '40c10f19': 'mint(address,uint256)',
                    '42966c68': 'burn(uint256)',
                    '70a08231': 'balanceOf(address)'
                }
                
                method = known_signatures.get(function_sig, f'Unknown (0x{function_sig})')
                
                return {
                    'method': method,
                    'signature': function_sig,
                    'decoded': method in known_signatures.values(),
                    'raw': input_data,
                    'parameters': input_data[8:] if len(input_data) > 8 else ''
                }
            else:
                return {
                    'method': 'data',
                    'decoded': False,
                    'raw': input_data
                }
                
        except Exception as e:
            print(f"Error decoding transaction input: {e}")
            return {
                'method': 'error',
                'decoded': False,
                'raw': input_data,
                'error': str(e)
            }
    
    def get_gold_token_stats(self):
        """Get GXC-G gold token statistics"""
        # Add timeout to prevent database locking issues
        conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
        cursor = conn.cursor()
        
        try:
            # Get total supply from gold-backed transactions
            cursor.execute('''
                SELECT 
                    COUNT(*) as total_transactions,
                    SUM(CASE WHEN from_address = '' OR from_address IS NULL THEN value ELSE 0 END) as total_minted,
                    SUM(CASE WHEN to_address = '' OR to_address IS NULL THEN value ELSE 0 END) as total_burned,
                    COUNT(DISTINCT from_address) + COUNT(DISTINCT to_address) as unique_addresses
                FROM transactions
                WHERE is_gold_backed = 1
            ''')
            
            stats = cursor.fetchone()
            
            # Get total reserves
            cursor.execute('SELECT SUM(gold_grams) FROM gold_reserves WHERE is_active = 1')
            total_reserves = cursor.fetchone()[0] or 0.0
            
            # Get top holders
            cursor.execute('''
                SELECT address, balance
                FROM gold_tokens
                ORDER BY balance DESC
                LIMIT 10
            ''')
            
            top_holders = []
            for row in cursor.fetchall():
                top_holders.append({
                    'address': row[0],
                    'balance': row[1]
                })
            
            # Get recent transfers
            cursor.execute('''
                SELECT tx_hash, from_address, to_address, amount, transfer_type, timestamp
                FROM gold_token_transfers
                ORDER BY timestamp DESC
                LIMIT 20
            ''')
            
            recent_transfers = []
            for row in cursor.fetchall():
                recent_transfers.append({
                    'tx_hash': row[0],
                    'from': row[1],
                    'to': row[2],
                    'amount': row[3],
                    'type': row[4],
                    'timestamp': row[5]
                })
            
            return {
                'total_transactions': stats[0] or 0,
                'total_minted': stats[1] or 0.0,
                'total_burned': stats[2] or 0.0,
                'total_supply': (stats[1] or 0.0) - (stats[2] or 0.0),
                'total_reserves': total_reserves,
                'reserve_ratio': (total_reserves / max((stats[1] or 0.0) - (stats[2] or 0.0), 1.0)) * 100 if (stats[1] or 0.0) - (stats[2] or 0.0) > 0 else 0.0,
                'unique_addresses': stats[3] or 0,
                'top_holders': top_holders,
                'recent_transfers': recent_transfers
            }
            
        except Exception as e:
            print(f"Error getting gold token stats: {e}")
            return {
                'total_transactions': 0,
                'total_minted': 0.0,
                'total_burned': 0.0,
                'total_supply': 0.0,
                'total_reserves': 0.0,
                'reserve_ratio': 0.0,
                'unique_addresses': 0,
                'top_holders': [],
                'recent_transfers': []
            }
        finally:
            conn.close()
    
    def get_gold_reserves(self):
        """Get gold reserves information"""
        # Add timeout to prevent database locking issues
        conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
        cursor = conn.cursor()
        
        try:
            cursor.execute('''
                SELECT vault_id, vault_address, gold_grams, attestation_hash,
                       last_audit_time, auditor_signature, is_active
                FROM gold_reserves
                ORDER BY gold_grams DESC
            ''')
            
            reserves = []
            for row in cursor.fetchall():
                reserves.append({
                    'vault_id': row[0],
                    'vault_address': row[1],
                    'gold_grams': row[2],
                    'attestation_hash': row[3],
                    'last_audit_time': row[4],
                    'auditor_signature': row[5],
                    'is_active': bool(row[6])
                })
            
            return reserves
            
        except Exception as e:
            print(f"Error getting gold reserves: {e}")
            return []
        finally:
            conn.close()
    
    def get_governance_proposals(self, status=None):
        """Get governance proposals"""
        # Add timeout to prevent database locking issues
        conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
        cursor = conn.cursor()
        
        try:
            if status:
                cursor.execute('''
                    SELECT proposal_id, proposer_address, title, description,
                           proposal_type, parameter_type, old_value, new_value,
                           status, start_block, end_block, votes_for, votes_against,
                           votes_abstain, total_votes, created_at, executed_at
                    FROM governance_proposals
                    WHERE status = ?
                    ORDER BY created_at DESC
                ''', (status,))
            else:
                cursor.execute('''
                    SELECT proposal_id, proposer_address, title, description,
                           proposal_type, parameter_type, old_value, new_value,
                           status, start_block, end_block, votes_for, votes_against,
                           votes_abstain, total_votes, created_at, executed_at
                    FROM governance_proposals
                    ORDER BY created_at DESC
                ''')
            
            proposals = []
            for row in cursor.fetchall():
                proposals.append({
                    'proposal_id': row[0],
                    'proposer': row[1],
                    'title': row[2],
                    'description': row[3],
                    'proposal_type': row[4],
                    'parameter_type': row[5],
                    'old_value': row[6],
                    'new_value': row[7],
                    'status': row[8],
                    'start_block': row[9],
                    'end_block': row[10],
                    'votes_for': row[11],
                    'votes_against': row[12],
                    'votes_abstain': row[13],
                    'total_votes': row[14],
                    'created_at': row[15],
                    'executed_at': row[16]
                })
            
            return proposals
            
        except Exception as e:
            print(f"Error getting governance proposals: {e}")
            return []
        finally:
            conn.close()
    
    def get_proposal_votes(self, proposal_id):
        """Get votes for a specific proposal"""
        # Add timeout to prevent database locking issues
        conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
        cursor = conn.cursor()
        
        try:
            cursor.execute('''
                SELECT voter_address, vote_type, vote_weight, timestamp, signature
                FROM governance_votes
                WHERE proposal_id = ?
                ORDER BY vote_weight DESC
            ''', (proposal_id,))
            
            votes = []
            for row in cursor.fetchall():
                votes.append({
                    'voter': row[0],
                    'vote_type': row[1],
                    'vote_weight': row[2],
                    'timestamp': row[3],
                    'signature': row[4]
                })
            
            return votes
            
        except Exception as e:
            print(f"Error getting proposal votes: {e}")
            return []
        finally:
            conn.close()
    
    def get_bridge_transfers(self, status=None, limit=100):
        """Get cross-chain bridge transfers"""
        # Add timeout to prevent database locking issues
        conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
        cursor = conn.cursor()
        
        try:
            query = '''
                SELECT transfer_id, source_chain, source_asset, amount,
                       destination_address, destination_chain, tx_hash, status,
                       required_signatures, collected_signatures, created_at, confirmed_at
                FROM bridge_transfers
            '''
            
            params = []
            if status:
                query += ' WHERE status = ?'
                params.append(status)
            
            query += ' ORDER BY created_at DESC LIMIT ?'
            params.append(limit)
            
            cursor.execute(query, params)
            
            transfers = []
            for row in cursor.fetchall():
                transfers.append({
                    'transfer_id': row[0],
                    'source_chain': row[1],
                    'source_asset': row[2],
                    'amount': row[3],
                    'destination_address': row[4],
                    'destination_chain': row[5],
                    'tx_hash': row[6],
                    'status': row[7],
                    'required_signatures': row[8],
                    'collected_signatures': row[9],
                    'created_at': row[10],
                    'confirmed_at': row[11]
                })
            
            return transfers
            
        except Exception as e:
            print(f"Error getting bridge transfers: {e}")
            return []
        finally:
            conn.close()
    
    def get_bridge_validators(self):
        """Get bridge validators"""
        # Add timeout to prevent database locking issues
        conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
        cursor = conn.cursor()
        
        try:
            cursor.execute('''
                SELECT validator_id, public_key, collateral, is_active,
                       transfers_validated, last_seen
                FROM bridge_validators
                ORDER BY transfers_validated DESC
            ''')
            
            validators = []
            for row in cursor.fetchall():
                validators.append({
                    'validator_id': row[0],
                    'public_key': row[1],
                    'collateral': row[2],
                    'is_active': bool(row[3]),
                    'transfers_validated': row[4],
                    'last_seen': row[5]
                })
            
            return validators
            
        except Exception as e:
            print(f"Error getting bridge validators: {e}")
            return []
        finally:
            conn.close()
    
    def get_staking_validators(self):
        """Get staking validators with performance metrics"""
        # Add timeout to prevent database locking issues
        conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
        cursor = conn.cursor()
        
        try:
            cursor.execute('''
                SELECT validator_address, stake_amount, staking_days, stake_start_time,
                       is_active, blocks_produced, missed_blocks, uptime,
                       total_rewards, pending_rewards, is_slashed, slashed_amount
                FROM staking_validators
                ORDER BY stake_amount DESC
            ''')
            
            validators = []
            for row in cursor.fetchall():
                # Calculate APY (simplified)
                days_staked = (datetime.utcnow() - datetime.fromisoformat(row[3].replace('Z', '+00:00').replace('+00:00', ''))).days if row[3] else 0
                apy = 0.0
                if days_staked > 0 and row[1] > 0:
                    annual_rewards = (row[8] / days_staked) * 365 if days_staked > 0 else 0
                    apy = (annual_rewards / row[1]) * 100 if row[1] > 0 else 0
                
                validators.append({
                    'address': row[0],
                    'stake_amount': row[1],
                    'staking_days': row[2],
                    'stake_start_time': row[3],
                    'is_active': bool(row[4]),
                    'blocks_produced': row[5],
                    'missed_blocks': row[6],
                    'uptime': row[7],
                    'total_rewards': row[8],
                    'pending_rewards': row[9],
                    'is_slashed': bool(row[10]),
                    'slashed_amount': row[11],
                    'apy': apy
                })
            
            return validators
            
        except Exception as e:
            print(f"Error getting staking validators: {e}")
            return []
        finally:
            conn.close()
    
    def get_address_relationships(self, address, limit=50):
        """Get address relationships for graph visualization"""
        # Add timeout to prevent database locking issues
        conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
        cursor = conn.cursor()
        
        try:
            cursor.execute('''
                SELECT 
                    CASE 
                        WHEN address1 = ? THEN address2 
                        ELSE address1 
                    END as related_address,
                    transaction_count,
                    total_value,
                    last_interaction
                FROM address_relationships
                WHERE address1 = ? OR address2 = ?
                ORDER BY transaction_count DESC, total_value DESC
                LIMIT ?
            ''', (address, address, address, limit))
            
            relationships = []
            for row in cursor.fetchall():
                relationships.append({
                    'address': row[0],
                    'transaction_count': row[1],
                    'total_value': row[2],
                    'last_interaction': row[3]
                })
            
            return relationships
            
        except Exception as e:
            print(f"Error getting address relationships: {e}")
            return []
        finally:
            conn.close()
    
    def update_address_relationships(self, from_addr, to_addr, value):
        """Update address relationship graph"""
        # Add timeout to prevent database locking issues
        conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
        cursor = conn.cursor()
        
        try:
            # Ensure addresses are in consistent order
            addr1, addr2 = sorted([from_addr, to_addr])
            
            cursor.execute('''
                INSERT INTO address_relationships (
                    address1, address2, transaction_count, total_value,
                    first_interaction, last_interaction
                ) VALUES (?, ?, 1, ?, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP)
                ON CONFLICT(address1, address2) DO UPDATE SET
                    transaction_count = transaction_count + 1,
                    total_value = total_value + ?,
                    last_interaction = CURRENT_TIMESTAMP
            ''', (addr1, addr2, value, value))
            
            conn.commit()
            
        except Exception as e:
            print(f"Error updating address relationships: {e}")
        finally:
            conn.close()
    
    def track_gold_token_transfer(self, tx_data, block_number):
        """Track gold token transfers"""
        # Add timeout to prevent database locking issues
        conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
        cursor = conn.cursor()
        
        try:
            # Determine transfer type
            transfer_type = 'transfer'
            if not tx_data.get('from') or tx_data.get('from') == '':
                transfer_type = 'mint'
            elif not tx_data.get('to') or tx_data.get('to') == '':
                transfer_type = 'burn'
            
            cursor.execute('''
                INSERT OR REPLACE INTO gold_token_transfers (
                    tx_hash, from_address, to_address, amount, transfer_type,
                    pop_reference, timestamp, block_height
                ) VALUES (?, ?, ?, ?, ?, ?, ?, ?)
            ''', (
                tx_data['hash'],
                tx_data.get('from'),
                tx_data.get('to'),
                float(tx_data['value']),
                transfer_type,
                tx_data.get('popReference'),
                datetime.utcnow(),
                block_number
            ))
            
            # Update gold token balances
            if tx_data.get('to'):
                cursor.execute('''
                    INSERT OR IGNORE INTO gold_tokens (
                        address, first_seen, last_activity
                    ) VALUES (?, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP)
                ''', (tx_data.get('to'),))
                
                cursor.execute('''
                    UPDATE gold_tokens
                    SET balance = balance + ?,
                        total_received = total_received + ?,
                        transaction_count = transaction_count + 1,
                        last_activity = CURRENT_TIMESTAMP
                    WHERE address = ?
                ''', (float(tx_data['value']), float(tx_data['value']), tx_data.get('to')))
            
            if tx_data.get('from'):
                cursor.execute('''
                    UPDATE gold_tokens
                    SET balance = balance - ?,
                        total_sent = total_sent + ?,
                        transaction_count = transaction_count + 1,
                        last_activity = CURRENT_TIMESTAMP
                    WHERE address = ?
                ''', (float(tx_data['value']), float(tx_data['value']), tx_data.get('from')))
            
            conn.commit()
            
        except Exception as e:
            conn.rollback()
            print(f"Error tracking gold token transfer: {e}")
        finally:
            conn.close()
    
    def start_block_monitor(self):
        """Start monitoring for new blocks"""
        def monitor():
            last_height = -1
            while True:
                try:
                    # Get latest block from node
                    latest_block = self.get_latest_block()
                    
                    if latest_block:
                        current_height = latest_block.get('height') or latest_block.get('block_number') or latest_block.get('number') or 0
                        
                        # Only store if it's a new block
                        if current_height > last_height:
                            print(f"[EXPLORER] New block detected: #{current_height}")
                            self.store_block(latest_block)
                            last_height = current_height
                            
                            # Emit to connected clients with normalized data
                            try:
                                # Get the stored block from database to ensure consistent format
                                stored_block = self.get_block_by_number_from_db(current_height)
                                if stored_block:
                                    # Convert to dict format for WebSocket - ensure ALL fields are present
                                    # stored_block is a dict from get_block_by_number_from_db
                                    emit_block = {
                                        'number': stored_block.get('number') or stored_block.get('block_number') or current_height or 0,
                                        'block_number': stored_block.get('number') or stored_block.get('block_number') or current_height or 0,
                                        'height': stored_block.get('number') or stored_block.get('block_number') or current_height or 0,
                                        'hash': stored_block.get('hash') or stored_block.get('block_hash') or '',
                                        'block_hash': stored_block.get('hash') or stored_block.get('block_hash') or '',
                                        'parent_hash': stored_block.get('parent_hash') or '',
                                        'miner': stored_block.get('miner') or stored_block.get('miner_address') or '',
                                        'miner_address': stored_block.get('miner') or stored_block.get('miner_address') or '',
                                        'tx_count': stored_block.get('tx_count') or stored_block.get('transaction_count') or 0,
                                        'transaction_count': stored_block.get('tx_count') or stored_block.get('transaction_count') or 0,
                                        'transactions': stored_block.get('transactions') or [],
                                        'consensus_type': stored_block.get('consensus_type') or stored_block.get('consensusType') or 'pow',
                                        'consensusType': stored_block.get('consensus_type') or stored_block.get('consensusType') or 'pow',
                                        'timestamp': str(stored_block.get('timestamp', '')) if stored_block.get('timestamp') else str(int(time.time())),
                                        'difficulty': stored_block.get('difficulty') or 0.1,
                                        'reward': stored_block.get('reward') or 12.5
                                    }
                                    socketio.emit('new_block', emit_block)
                                else:
                                    # Fallback to normalized latest_block - ensure ALL fields are present
                                    if latest_block:
                                        normalized_emit = {
                                            'number': latest_block.get('number') or latest_block.get('block_number') or latest_block.get('height') or current_height or 0,
                                            'block_number': latest_block.get('number') or latest_block.get('block_number') or latest_block.get('height') or current_height or 0,
                                            'height': latest_block.get('number') or latest_block.get('block_number') or latest_block.get('height') or current_height or 0,
                                            'hash': latest_block.get('hash') or latest_block.get('block_hash') or '',
                                            'block_hash': latest_block.get('hash') or latest_block.get('block_hash') or '',
                                            'parent_hash': latest_block.get('parent_hash') or latest_block.get('prev_hash') or '',
                                            'miner': latest_block.get('miner') or latest_block.get('miner_address') or '',
                                            'miner_address': latest_block.get('miner') or latest_block.get('miner_address') or '',
                                            'tx_count': latest_block.get('tx_count') or latest_block.get('transaction_count') or len(latest_block.get('transactions', [])) or 0,
                                            'transaction_count': latest_block.get('tx_count') or latest_block.get('transaction_count') or len(latest_block.get('transactions', [])) or 0,
                                            'transactions': latest_block.get('transactions') or [],
                                            'consensus_type': latest_block.get('consensus_type') or latest_block.get('consensusType') or 'pow',
                                            'consensusType': latest_block.get('consensus_type') or latest_block.get('consensusType') or 'pow',
                                            'timestamp': str(latest_block.get('timestamp') or int(time.time())),
                                            'difficulty': latest_block.get('difficulty') or 0.1,
                                            'reward': latest_block.get('reward') or 12.5
                                        }
                                        socketio.emit('new_block', normalized_emit)
                            except Exception as e:
                                print(f"[EXPLORER] Error emitting block: {e}")
                                import traceback
                                traceback.print_exc()
                                # Try to emit normalized block with ALL fields
                                try:
                                    if latest_block:
                                        safe_emit = {
                                            'number': latest_block.get('number') or latest_block.get('block_number') or latest_block.get('height') or current_height or 0,
                                            'block_number': latest_block.get('number') or latest_block.get('block_number') or latest_block.get('height') or current_height or 0,
                                            'height': latest_block.get('number') or latest_block.get('block_number') or latest_block.get('height') or current_height or 0,
                                            'hash': latest_block.get('hash') or latest_block.get('block_hash') or '',
                                            'miner': latest_block.get('miner') or latest_block.get('miner_address') or '',
                                            'miner_address': latest_block.get('miner') or latest_block.get('miner_address') or '',
                                            'tx_count': latest_block.get('tx_count') or latest_block.get('transaction_count') or len(latest_block.get('transactions', [])) or 0,
                                            'transaction_count': latest_block.get('tx_count') or latest_block.get('transaction_count') or len(latest_block.get('transactions', [])) or 0,
                                            'transactions': latest_block.get('transactions') or [],
                                            'consensus_type': latest_block.get('consensus_type') or latest_block.get('consensusType') or 'pow',
                                            'timestamp': str(latest_block.get('timestamp') or int(time.time())),
                                            'difficulty': latest_block.get('difficulty') or 0.1,
                                            'reward': latest_block.get('reward') or 12.5
                                        }
                                        socketio.emit('new_block', safe_emit)
                                except Exception as e2:
                                    print(f"[EXPLORER] Error in fallback block emission: {e2}")
                                    pass  # SocketIO might not be initialized
                            
                            # Update network stats periodically
                            if current_height % 10 == 0:
                                self.update_network_stats()
                        elif current_height == last_height:
                            # Same block, just update stats occasionally
                            pass
                    else:
                        # No block data, try to get blockchain info
                        info = rpc_call('getblockchaininfo', timeout=5, show_errors=False)
                        if info:
                            height = info.get('blocks', 0)
                            if height > last_height:
                                print(f"[EXPLORER] Blockchain height: {height}, fetching blocks...")
                                # Try to fetch missing blocks
                                for h in range(max(0, last_height + 1), height + 1):
                                    block = self.get_block_by_number(h)
                                    if block:
                                        self.store_block(block)
                                last_height = height
                    
                    time.sleep(5)  # Check every 5 seconds
                    
                except Exception as e:
                    print(f"[EXPLORER] Block monitor error: {e}")
                    import traceback
                    traceback.print_exc()
                    time.sleep(10)  # Wait longer on error
        
        thread = threading.Thread(target=monitor)
        thread.daemon = True
        thread.start()
    
    def update_network_stats(self):
        """Update network statistics"""
        # Add timeout to prevent database locking issues
        conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
        cursor = conn.cursor()
        
        try:
            # Calculate current stats
            cursor.execute('SELECT COUNT(*) FROM blocks')
            total_blocks = cursor.fetchone()[0]
            
            cursor.execute('SELECT COUNT(*) FROM transactions')
            total_transactions = cursor.fetchone()[0]
            
            cursor.execute('SELECT COUNT(*) FROM addresses')
            total_addresses = cursor.fetchone()[0]
            
            # Calculate average block time (last 100 blocks)
            cursor.execute('''
                SELECT AVG(
                    (julianday(b2.timestamp) - julianday(b1.timestamp)) * 86400
                ) as avg_time
                FROM blocks b1
                JOIN blocks b2 ON b2.block_number = b1.block_number + 1
                WHERE b1.block_number >= (SELECT MAX(block_number) - 100 FROM blocks)
            ''')
            avg_block_time = cursor.fetchone()[0] or 0.0
            
            # Get latest difficulty
            cursor.execute('SELECT difficulty FROM blocks ORDER BY block_number DESC LIMIT 1')
            difficulty = cursor.fetchone()
            difficulty = difficulty[0] if difficulty else 0.0
            
            # Insert stats
            cursor.execute('''
                INSERT INTO network_stats (
                    total_blocks, total_transactions, total_addresses,
                    total_supply, difficulty, avg_block_time
                ) VALUES (?, ?, ?, ?, ?, ?)
            ''', (total_blocks, total_transactions, total_addresses, 31000000, difficulty, avg_block_time))
            
            conn.commit()
            
        except Exception as e:
            conn.rollback()
            print(f"Error updating network stats: {e}")
        finally:
            conn.close()

# Initialize explorer
print("=" * 60)
print("[EXPLORER] Initializing blockchain explorer...")
print(f"[EXPLORER] Network: {NETWORK.upper()}")
print(f"[EXPLORER] Database: {DATABASE_PATH}")
print(f"[EXPLORER] Address prefix: {CURRENT_NETWORK['address_prefix']}")
print(f"[EXPLORER] Block reward: {CURRENT_NETWORK['block_reward']} GXC")
print(f"[EXPLORER] Node URL: {BLOCKCHAIN_NODE_URL}")
print("=" * 60)
explorer = BlockchainExplorer()

# Test blockchain connection on startup (non-blocking)
def test_connection_async():
    """Test connection in background thread to avoid blocking startup with retries"""
    # Wait longer for node to start up (especially if node was just started)
    time.sleep(5)  # Increased wait time for node initialization
    
    print("[EXPLORER] Testing blockchain connection on startup...")
    
    # Try multiple times with delays
    max_attempts = 3
    for attempt in range(max_attempts):
        try:
            connected, node_type = test_blockchain_connection()
            if connected:
                print(f"[EXPLORER] ✅ Connected to {node_type} node ({NETWORK.upper()})")
                return
            else:
                if attempt < max_attempts - 1:
                    wait_time = 5 * (attempt + 1)  # Progressive delay: 5s, 10s, 15s
                    print(f"[EXPLORER] ⚠️  Connection attempt {attempt + 1}/{max_attempts} failed, retrying in {wait_time}s...")
                    time.sleep(wait_time)
        except Exception as e:
            print(f"[EXPLORER] Error testing connection: {e}")
            if attempt < max_attempts - 1:
                time.sleep(5)
    
    print(f"[EXPLORER] ⚠️  No blockchain nodes available after {max_attempts} attempts - explorer will use cached data")
    print(f"[EXPLORER] 💡 Run 'python test_node_connection.py' to diagnose connection issues")

# Start connection test in background
connection_test_thread = threading.Thread(target=test_connection_async)
connection_test_thread.daemon = True
connection_test_thread.start()

# Web Routes
@app.route('/')
def index():
    """Main explorer page"""
    # Try to sync latest block and transactions first
    try:
        latest_block = explorer.get_latest_block()
        if latest_block:
            # Store block (which will also store transactions if included)
            explorer.store_block(latest_block)
            print(f"[EXPLORER] Synced latest block #{latest_block.get('number') or latest_block.get('height') or 0}")
    except Exception as e:
        print(f"Error syncing latest block: {e}")
        import traceback
        traceback.print_exc()
    
    # Get data from database
    recent_blocks = explorer.get_recent_blocks(10) or []
    recent_transactions = explorer.get_recent_transactions(15) or []
    
    # If no transactions but we have blocks, try to fetch transactions from recent blocks
    if not recent_transactions and recent_blocks:
        print("[EXPLORER] No transactions in database, trying to fetch from recent blocks...")
        for block in recent_blocks[:5]:  # Check last 5 blocks
            block_num = block.get('number', 0)
            if block_num > 0:
                try:
                    block_data = explorer.get_block_by_number(block_num)
                    if block_data and block_data.get('transactions'):
                        explorer.store_block(block_data)
                except Exception as e:
                    print(f"Error fetching transactions for block {block_num}: {e}")
        # Refresh transactions
        recent_transactions = explorer.get_recent_transactions(15) or []
    network_stats = explorer.get_network_stats() or {}
    
    # Ensure stats has all required keys with defaults
    if not network_stats or not isinstance(network_stats, dict):
        network_stats = {}
    network_stats.setdefault('total_blocks', 0)
    network_stats.setdefault('total_transactions', 0)
    network_stats.setdefault('total_addresses', 0)
    network_stats.setdefault('total_supply', 31000000)
    network_stats.setdefault('hash_rate', 0.0)
    network_stats.setdefault('difficulty', 0.0)
    network_stats.setdefault('avg_block_time', 0.0)
    
    # If no blocks, try to get blockchain info and show connection status
    if not recent_blocks:
        info = rpc_call('getblockchaininfo', timeout=5, show_errors=False)
        if info:
            height = info.get('blocks') or info.get('height') or 0
            if height > 0:
                print(f"[EXPLORER] No blocks in database, syncing from height {height}...")
                # Try to fetch and store recent blocks (last 20)
                synced_count = 0
                for h in range(max(0, height - 19), height + 1):
                    block = explorer.get_block_by_number(h)
                    if block:
                        explorer.store_block(block)
                        synced_count += 1
                print(f"[EXPLORER] Synced {synced_count} blocks from testnet node")
                # Refresh data
                recent_blocks = explorer.get_recent_blocks(10)
    
    # Get connection status for display
    connection_info = {
        'active_node': ACTIVE_NODE_TYPE or 'unknown',
        'active_url': ACTIVE_NODE_URL or 'none',
        'railway_status': CONNECTION_STATUS['railway'],
        'local_status': CONNECTION_STATUS['local'],
        'connected': ACTIVE_NODE_TYPE is not None
    }
    
    # Network information for display
    network_info = {
        'network': NETWORK.upper(),
        'address_prefix': CURRENT_NETWORK['address_prefix'],
        'block_reward': CURRENT_NETWORK['block_reward'],
        'block_time': CURRENT_NETWORK['block_time'],
        'is_testnet': NETWORK == 'testnet',
        'is_mainnet': NETWORK == 'mainnet'
    }
    
    return render_template('explorer_index.html', 
                         blocks=recent_blocks,
                         transactions=recent_transactions,
                         stats=network_stats,
                         blockchain_node_url=ACTIVE_NODE_URL or BLOCKCHAIN_NODE_URL,
                         connection_info=connection_info,
                         network_info=network_info,
                         forum_url=FORUM_URL)

@app.route('/block/<block_number>')
def block_detail(block_number):
    """Block detail page"""
    try:
        block_number = int(block_number)
    except:
        return "Invalid block number", 400
    
    conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
    cursor = conn.cursor()
    
    try:
        # Get block data from database - select ALL fields in correct order
        cursor.execute('''
            SELECT block_number, block_hash, parent_hash, timestamp, miner_address,
                   difficulty, total_difficulty, size, gas_used, gas_limit,
                   transaction_count, reward, consensus_type, nonce, validator_signature,
                   fee_burn_rate, pop_reference, merkle_root
            FROM blocks WHERE block_number = ?
        ''', (block_number,))
        
        block = cursor.fetchone()
        
        # If block not in database, try to fetch from node
        if not block:
            print(f"[EXPLORER] Block {block_number} not in database, fetching from node...")
            block_data = explorer.get_block_by_number(block_number)
            if block_data:
                explorer.store_block(block_data)
                # Re-fetch from database
                cursor.execute('''
                    SELECT block_number, block_hash, parent_hash, timestamp, miner_address,
                           difficulty, total_difficulty, size, gas_used, gas_limit,
                           transaction_count, reward, consensus_type, nonce, validator_signature,
                           fee_burn_rate, pop_reference, merkle_root
                    FROM blocks WHERE block_number = ?
                ''', (block_number,))
                block = cursor.fetchone()
        
        if not block:
            return "Block not found", 404
        
        # Ensure block tuple has all fields with defaults
        # Pad with None if tuple is shorter than expected
        block_list = list(block) if block else []
        while len(block_list) < 18:
            block_list.append(None)
        block = tuple(block_list)
        
        # Get block transactions
        cursor.execute('''
            SELECT * FROM transactions WHERE block_number = ?
            ORDER BY COALESCE(tx_index, 0)
        ''', (block_number,))
        
        transactions = cursor.fetchall()
        
        # If no transactions in DB but block has transaction_count > 0, try to fetch from node
        if not transactions and block[10] and block[10] > 0:  # transaction_count field
            print(f"[EXPLORER] Block {block_number} has {block[10]} transactions but none in DB, fetching from node...")
            block_data = explorer.get_block_by_number(block_number)
            if block_data and block_data.get('transactions'):
                # Store transactions
                for i, tx in enumerate(block_data['transactions']):
                    explorer.store_transaction(tx, block_number, i, block_data)
                # Re-fetch transactions
                cursor.execute('''
                    SELECT * FROM transactions WHERE block_number = ?
                    ORDER BY COALESCE(tx_index, 0)
                ''', (block_number,))
                transactions = cursor.fetchall()
        
        # Get connection info for template
        connection_info = {
            'active_node': ACTIVE_NODE_TYPE or 'unknown',
            'connected': ACTIVE_NODE_TYPE is not None
        }
        
        # Network information for display
        network_info = {
            'network': NETWORK.upper(),
            'address_prefix': CURRENT_NETWORK['address_prefix'],
            'block_reward': CURRENT_NETWORK['block_reward'],
            'block_time': CURRENT_NETWORK['block_time'],
            'is_testnet': NETWORK == 'testnet',
            'is_mainnet': NETWORK == 'mainnet'
        }
        
        return render_template('block_detail.html', 
                             block=block,
                             transactions=transactions,
                             connection_info=connection_info,
                             network_info=network_info,
                             forum_url=FORUM_URL)
        
    except Exception as e:
        print(f"Error in block_detail: {e}")
        import traceback
        traceback.print_exc()
        return f"Error loading block: {str(e)}", 500
    finally:
        conn.close()

@app.route('/tx/<tx_hash>')
def transaction_detail(tx_hash):
    """Transaction detail page with full blockchain data"""
    # Add timeout to prevent database locking issues
    conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
    cursor = conn.cursor()
    
    try:
        # Get transaction from database
        cursor.execute('''
            SELECT * FROM transactions WHERE tx_hash = ?
        ''', (tx_hash,))
        
        transaction = cursor.fetchone()
        
        # If transaction not in database, try to get it from node
        if not transaction:
            print(f"[EXPLORER] Transaction {tx_hash} not in database, trying to fetch from node...")
            # Try multiple RPC methods to get COMPLETE transaction data with ALL fields
            tx_data = None
            methods = [
                ('gettransaction', [tx_hash, True]),  # verbose - includes all fields
                ('gettransaction', [tx_hash]),
                ('getrawtransaction', [tx_hash, True]),  # verbose - decoded with all fields
                ('getrawtransaction', [tx_hash]),
                ('gxc_getTransaction', [tx_hash, True]),  # verbose
                ('gxc_getTransaction', [tx_hash]),
                ('gettx', [tx_hash, True]),  # verbose
                ('gettx', [tx_hash])
            ]
            
            for method, params in methods:
                # Use longer timeout to ensure all data is returned
                tx_data = rpc_call(method, params=params, timeout=20, show_errors=False)
                # Ensure tx_data is a dict, not a string
                if tx_data:
                    if isinstance(tx_data, str):
                        try:
                            import json
                            tx_data = json.loads(tx_data)
                        except:
                            tx_data = None
                    if isinstance(tx_data, dict):
                        break
                    else:
                        tx_data = None
            
            if tx_data and isinstance(tx_data, dict):
                # Get block number from transaction - try multiple field names
                block_number = (tx_data.get('blockNumber') or tx_data.get('block_number') or 
                              tx_data.get('blockheight') or tx_data.get('blockHeight') or 
                              tx_data.get('block') or 0)
                
                if block_number > 0:
                    # Store the block first to ensure it exists and get all its transactions
                    block_data = explorer.get_block_by_number(block_number)
                    if block_data:
                        explorer.store_block(block_data)
                        # If transaction wasn't in the block's transactions, store it separately
                        block_txs = block_data.get('transactions') or []
                        tx_found_in_block = any(
                            (tx.get('hash') or tx.get('tx_hash') or '') == tx_hash 
                            for tx in block_txs
                        )
                        if not tx_found_in_block:
                            explorer.store_transaction(tx_data, block_number, tx_data.get('index', 0), block_data)
                else:
                    # Transaction not in a block yet (pending), store with block 0
                    explorer.store_transaction(tx_data, 0, tx_data.get('index', 0), None)
                
                # Re-fetch from database
                cursor.execute('''
                    SELECT * FROM transactions WHERE tx_hash = ?
                ''', (tx_hash,))
                transaction = cursor.fetchone()
        
        if not transaction:
            # Try to fetch from node before giving up
            print(f"[TX] Transaction {tx_hash} not in DB, fetching from node...")
            tx_data = rpc_call('gettransaction', [tx_hash, True], timeout=15, show_errors=False)
            if not tx_data:
                tx_data = rpc_call('getrawtransaction', [tx_hash, True], timeout=15, show_errors=False)
            
            if tx_data:
                # Ensure tx_data is a dict, not a string
                if isinstance(tx_data, str):
                    try:
                        import json
                        tx_data = json.loads(tx_data)
                    except:
                        tx_data = None
                
                if tx_data and isinstance(tx_data, dict):
                    # Store the transaction
                    block_num = tx_data.get('blockNumber') or tx_data.get('block_number') or 0
                    if block_num > 0:
                        # Get block data for context
                        block_data = explorer.get_block_by_number(block_num)
                        if block_data:
                            explorer.store_block(block_data)
                        explorer.store_transaction(tx_data, block_num, tx_data.get('index', 0), block_data if block_num > 0 else None)
                    else:
                        explorer.store_transaction(tx_data, 0, tx_data.get('index', 0), None)
                
                # Re-fetch from database
                cursor.execute('SELECT * FROM transactions WHERE tx_hash = ?', (tx_hash,))
                transaction = cursor.fetchone()
            
            if not transaction:
                return render_template('error.html', 
                    error="Transaction not found",
                    message=f"The transaction {tx_hash} could not be found in the database or on the blockchain node.",
                    back_url="/transactions",
                    forum_url=FORUM_URL), 404
        
        # Get transaction inputs
        cursor.execute('''
            SELECT * FROM transaction_inputs 
            WHERE transaction_hash = ?
            ORDER BY input_index
        ''', (tx_hash,))
        inputs = cursor.fetchall()
        
        # Get transaction outputs
        cursor.execute('''
            SELECT * FROM transaction_outputs 
            WHERE transaction_hash = ?
            ORDER BY output_index
        ''', (tx_hash,))
        outputs = cursor.fetchall()
        
        # Get traceability chain - build from transaction data, fetch missing data from node
        traceability_chain = []
        current_tx_hash = tx_hash
        visited = set()  # Prevent infinite loops
        
        for level in range(20):  # Limit to 20 levels deep
            if current_tx_hash in visited:
                break
            visited.add(current_tx_hash)
            
            # Get current transaction's previous transaction info
            cursor.execute('''
                SELECT prev_tx_hash, referenced_amount, traceability_valid, block_number, value
                FROM transactions
                WHERE tx_hash = ?
            ''', (current_tx_hash,))
            
            tx_data = cursor.fetchone()
            
            # If transaction not in DB, try to fetch from node with ALL data
            if not tx_data:
                print(f"[EXPLORER] Transaction {current_tx_hash} not in DB for traceability, fetching from node...")
                # Use verbose flag to get ALL transaction fields
                tx_node_data = rpc_call('gettransaction', [current_tx_hash, True], timeout=15, show_errors=False)
                if not tx_node_data:
                    tx_node_data = rpc_call('getrawtransaction', [current_tx_hash, True], timeout=15, show_errors=False)
                
                if tx_node_data:
                    block_num = tx_node_data.get('blockNumber') or tx_node_data.get('block_number') or 0
                    explorer.store_transaction(tx_node_data, block_num, tx_node_data.get('index', 0), None)
                    # Re-fetch from DB
                    cursor.execute('''
                        SELECT prev_tx_hash, referenced_amount, traceability_valid, block_number, value
                        FROM transactions
                        WHERE tx_hash = ?
                    ''', (current_tx_hash,))
                    tx_data = cursor.fetchone()
            
            if not tx_data or not tx_data[0] or tx_data[0] == '0' or tx_data[0] is None:
                break
            
            prev_tx_hash, ref_amount, trace_valid, block_height, tx_value = tx_data
            
            # Get previous transaction details - fetch from node if not in DB
            cursor.execute('''
                SELECT value, block_number
                FROM transactions
                WHERE tx_hash = ?
            ''', (prev_tx_hash,))
            
            prev_tx_data = cursor.fetchone()
            
            # If previous transaction not in DB, try to fetch from node with ALL data
            if not prev_tx_data:
                print(f"[EXPLORER] Previous transaction {prev_tx_hash} not in DB for traceability, fetching from node...")
                # Use verbose flag to get ALL transaction fields
                prev_tx_node_data = rpc_call('gettransaction', [prev_tx_hash, True], timeout=15, show_errors=False)
                if not prev_tx_node_data:
                    prev_tx_node_data = rpc_call('getrawtransaction', [prev_tx_hash, True], timeout=15, show_errors=False)
                
                if prev_tx_node_data:
                    prev_block_num = prev_tx_node_data.get('blockNumber') or prev_tx_node_data.get('block_number') or 0
                    explorer.store_transaction(prev_tx_node_data, prev_block_num, prev_tx_node_data.get('index', 0), None)
                    # Re-fetch from DB
                    cursor.execute('''
                        SELECT value, block_number
                        FROM transactions
                        WHERE tx_hash = ?
                    ''', (prev_tx_hash,))
                    prev_tx_data = cursor.fetchone()
            
            if not prev_tx_data:
                break
            
            traceability_chain.append({
                'tx_hash': current_tx_hash,
                'prev_tx_hash': prev_tx_hash,
                'referenced_amount': ref_amount or 0.0,
                'validation_status': bool(trace_valid) if trace_valid is not None else True,
                'block_height': block_height or 0,
                'level': level + 1,
                'tx_value': tx_value or 0.0,
                'prev_tx_value': prev_tx_data[0] if prev_tx_data else 0.0
            })
            
            current_tx_hash = prev_tx_hash
        
        # Calculate confirmations (current_height - tx_block_height + 1)
        tx_block_number = transaction[1] if transaction and len(transaction) > 1 else 0
        current_height = explorer.get_current_block_height()
        confirmations = max(0, current_height - tx_block_number + 1) if tx_block_number > 0 else 0
        
        # Network information for display
        network_info = {
            'network': NETWORK.upper(),
            'address_prefix': CURRENT_NETWORK['address_prefix'],
            'block_reward': CURRENT_NETWORK['block_reward'],
            'block_time': CURRENT_NETWORK['block_time'],
            'is_testnet': NETWORK == 'testnet',
            'is_mainnet': NETWORK == 'mainnet'
        }
        
        return render_template('transaction_detail.html', 
                             transaction=transaction,
                             inputs=inputs,
                             outputs=outputs,
                             traceability_chain=traceability_chain,
                             network_info=network_info,
                             confirmations=confirmations,
                             current_height=current_height,
                             forum_url=FORUM_URL)
        
    finally:
        conn.close()

@app.route('/address/<address>')
def address_detail(address):
    """Address detail page"""
    conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
    cursor = conn.cursor()
    
    try:
        # Get address info
        cursor.execute('''
            SELECT * FROM addresses WHERE address = ?
        ''', (address,))
        
        addr_info = cursor.fetchone()
        
        # If address not in database, try to get balance and data from node
        if not addr_info:
            print(f"[EXPLORER] Address {address} not in database, fetching from node...")
            # Try multiple methods to get balance - ensure ALL address data is fetched
            balance = None
            methods = [
                ('getbalance', [address, True]),  # verbose - includes all balance info
                ('getbalance', [address]),
                ('gxc_getBalance', [address, True]),  # verbose
                ('gxc_getBalance', [address]),
                ('getaddressbalance', [address, True]),  # verbose
                ('getaddressbalance', [address])
            ]
            
            for method, params in methods:
                # Use longer timeout to ensure all data is returned
                balance = rpc_call(method, params=params, timeout=20, show_errors=False)
                if balance is not None:
                    break
            
            if balance is None:
                balance = 0.0
            
            # Try to get address transaction count and other info - fetch ALL transaction data
            tx_count = 0
            try:
                # Try to get transaction history - use verbose to get all transaction IDs
                tx_history = rpc_call('getaddresstxids', [address, True], timeout=20, show_errors=False)
                if not tx_history:
                    tx_history = rpc_call('getaddresstxids', [address], timeout=20, show_errors=False)
                
                if tx_history and isinstance(tx_history, list):
                    tx_count = len(tx_history)
                    # Store any missing transactions with FULL data
                    for tx_hash in tx_history[:100]:  # Limit to first 100
                        try:
                            # Use verbose flag to get ALL transaction fields
                            tx_data = rpc_call('gettransaction', [tx_hash, True], timeout=15, show_errors=False)
                            if not tx_data:
                                tx_data = rpc_call('getrawtransaction', [tx_hash, True], timeout=15, show_errors=False)
                            
                            if tx_data:
                                block_num = tx_data.get('blockNumber') or tx_data.get('block_number') or 0
                                explorer.store_transaction(tx_data, block_num, tx_data.get('index', 0), None)
                        except Exception as e:
                            print(f"Error fetching transaction {tx_hash}: {e}")
                            pass
            except Exception as e:
                print(f"Error getting address transaction history: {e}")
                pass
            
            # Create address entry
            cursor.execute('''
                INSERT OR IGNORE INTO addresses (address, balance, first_seen, last_seen, transaction_count)
                VALUES (?, ?, datetime('now'), datetime('now'), ?)
            ''', (address, balance, tx_count))
            conn.commit()
            
            # Re-fetch
            cursor.execute('''
                SELECT * FROM addresses WHERE address = ?
            ''', (address,))
            addr_info = cursor.fetchone()
        
        if not addr_info:
            return "Address not found", 404
        
        # Get address transactions
        cursor.execute('''
            SELECT * FROM transactions 
            WHERE from_address = ? OR to_address = ?
            ORDER BY block_number DESC, COALESCE(tx_index, 0) DESC
            LIMIT 50
        ''', (address, address))
        
        transactions = cursor.fetchall()
        
        # If no transactions in DB but address exists, try to fetch from node
        if not transactions and addr_info:
            print(f"[EXPLORER] Address {address} has no transactions in DB, trying to fetch from node...")
            try:
                # Try to get transaction history from node - use verbose to get ALL transaction IDs
                tx_history = rpc_call('getaddresstxids', [address, True], timeout=20, show_errors=False)
                if not tx_history:
                    tx_history = rpc_call('getaddresstxids', [address], timeout=20, show_errors=False)
                
                if tx_history and isinstance(tx_history, list):
                    # Fetch and store transactions with FULL data
                    for tx_hash in tx_history[:50]:  # Limit to 50
                        try:
                            # Use verbose flag to get ALL transaction fields
                            tx_data = rpc_call('gettransaction', [tx_hash, True], timeout=15, show_errors=False)
                            if not tx_data:
                                tx_data = rpc_call('getrawtransaction', [tx_hash, True], timeout=15, show_errors=False)
                            
                            if tx_data:
                                block_num = tx_data.get('blockNumber') or tx_data.get('block_number') or 0
                                if block_num > 0:
                                    # Ensure block exists with ALL data
                                    block_data = explorer.get_block_by_number(block_num)
                                    if block_data:
                                        explorer.store_block(block_data)
                                
                                # Store transaction with ALL fields
                                block_data_for_tx = explorer.get_block_by_number(block_num) if block_num > 0 else None
                                explorer.store_transaction(tx_data, block_num, tx_data.get('index', 0), block_data_for_tx)
                        except Exception as e:
                            print(f"Error fetching transaction {tx_hash}: {e}")
                            continue
                    
                    # Re-fetch transactions from database
                    cursor.execute('''
                        SELECT * FROM transactions 
                        WHERE from_address = ? OR to_address = ?
                        ORDER BY block_number DESC, COALESCE(tx_index, 0) DESC
                        LIMIT 50
                    ''', (address, address))
                    transactions = cursor.fetchall()
            except Exception as e:
                print(f"Error fetching address transactions: {e}")
        
        # Get connection info
        connection_info = {
            'active_node': ACTIVE_NODE_TYPE or 'unknown',
            'connected': ACTIVE_NODE_TYPE is not None
        }
        
        # Network information for display
        network_info = {
            'network': NETWORK.upper(),
            'address_prefix': CURRENT_NETWORK['address_prefix'],
            'block_reward': CURRENT_NETWORK['block_reward'],
            'block_time': CURRENT_NETWORK['block_time'],
            'is_testnet': NETWORK == 'testnet',
            'is_mainnet': NETWORK == 'mainnet'
        }
        
        return render_template('address_detail.html',
                             address_info=addr_info,
                             transactions=transactions,
                             connection_info=connection_info,
                             network_info=network_info,
                             forum_url=FORUM_URL)
        
    except Exception as e:
        print(f"Error in address_detail: {e}")
        import traceback
        traceback.print_exc()
        return f"Error loading address: {str(e)}", 500
    finally:
        conn.close()

@app.route('/search')
def search():
    """Search for blocks, transactions, or addresses"""
    query = request.args.get('q', '').strip()
    
    if not query:
        return redirect(url_for('index'))
    
    # Try to determine what we're searching for
    if query.startswith('GXC') and len(query) == 43:
        # Looks like an address
        return redirect(url_for('address_detail', address=query))
    elif query.startswith('0x') and len(query) == 66:
        # Looks like a transaction hash
        return redirect(url_for('transaction_detail', tx_hash=query))
    elif query.isdigit():
        # Looks like a block number
        return redirect(url_for('block_detail', block_number=int(query)))
    else:
        return "Search query not recognized", 400

@app.route('/blocks')
def blocks_list():
    """Blocks list page"""
    # Try to sync latest blocks first
    try:
        latest_block = explorer.get_latest_block()
        if latest_block:
            explorer.store_block(latest_block)
        
        # Also try to get blockchain info and sync missing blocks
        info = rpc_call('getblockchaininfo', timeout=5, show_errors=False)
        if info:
            height = info.get('blocks') or info.get('height') or 0
            if height > 0:
                # Get recent blocks from database to see what we have
                recent = explorer.get_recent_blocks(1)
                last_stored = recent[0]['number'] if recent and len(recent) > 0 else -1
                
                # Sync missing blocks (last 50 blocks or from last stored)
                sync_start = max(0, height - 49)
                if last_stored >= 0:
                    sync_start = max(sync_start, last_stored + 1)
                
                synced_count = 0
                for h in range(sync_start, height + 1):
                    block = explorer.get_block_by_number(h)
                    if block:
                        explorer.store_block(block)
                        synced_count += 1
                
                if synced_count > 0:
                    print(f"[EXPLORER] Synced {synced_count} new blocks on /blocks page")
    except Exception as e:
        print(f"Error syncing blocks: {e}")
        import traceback
        traceback.print_exc()
    
    blocks = explorer.get_recent_blocks(50)
    
    # Network information for display
    network_info = {
        'network': NETWORK.upper(),
        'address_prefix': CURRENT_NETWORK['address_prefix'],
        'block_reward': CURRENT_NETWORK['block_reward'],
        'block_time': CURRENT_NETWORK['block_time'],
        'is_testnet': NETWORK == 'testnet',
        'is_mainnet': NETWORK == 'mainnet'
    }
    
    return render_template('blocks.html', 
                         blocks=blocks, 
                         blockchain_node_url=BLOCKCHAIN_NODE_URL,
                         network_info=network_info,
                         forum_url=FORUM_URL)

@app.route('/transactions')
def transactions_list():
    """Transactions list page with traceability info - Permanent fix"""
    # Try to sync latest data first
    try:
        latest_block = explorer.get_latest_block()
        if latest_block:
            explorer.store_block(latest_block)
    except:
        pass
    
    conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
    cursor = conn.cursor()
    
    try:
        # Get recent transactions with traceability data
        # Use COALESCE for safe handling of NULL values
        # NOTE: Column is 'value' not 'amount' in database schema
        cursor.execute('''
            SELECT 
                COALESCE(tx.tx_hash, '') as tx_hash,
                COALESCE(tx.block_number, 0) as block_number,
                COALESCE(tx.from_address, '') as from_address,
                COALESCE(tx.to_address, '') as to_address,
                COALESCE(tx.value, 0.0) as value,
                COALESCE(tx.fee, 0.0) as fee,
                COALESCE(tx.status, 'unknown') as status,
                COALESCE(tx.timestamp, 0) as timestamp,
                COALESCE(tx.prev_tx_hash, '') as prev_tx_hash,
                COALESCE(tx.referenced_amount, 0.0) as referenced_amount,
                COALESCE(tx.traceability_valid, 1) as traceability_valid,
                COALESCE(tx.is_coinbase, 0) as is_coinbase
            FROM transactions tx
            ORDER BY tx.block_number DESC, COALESCE(tx.tx_index, tx.timestamp) DESC
            LIMIT 100
        ''')
        
        rows = cursor.fetchall()
        transactions = []
        
        # Get current block height for confirmations
        current_height = explorer.get_current_block_height()
        
        # Network information for display
        network_info = {
            'network': NETWORK.upper(),
            'address_prefix': CURRENT_NETWORK['address_prefix'],
            'block_reward': CURRENT_NETWORK['block_reward'],
            'block_time': CURRENT_NETWORK['block_time'],
            'is_testnet': NETWORK == 'testnet',
            'is_mainnet': NETWORK == 'mainnet'
        }
        
        # Handle empty result gracefully
        if not rows:
            return render_template('transactions.html', transactions=[], network_info=network_info, current_height=current_height, forum_url=FORUM_URL)
        
        for row in rows:
            # Safely unpack row - all values are guaranteed by COALESCE
            tx_hash, block, from_addr, to_addr, value, fee, status, timestamp, prev_tx, ref_amount, trace_valid, is_coinbase = row
            
            # Calculate age
            if timestamp:
                try:
                    from datetime import datetime
                    tx_time = datetime.fromtimestamp(timestamp)
                    now = datetime.now()
                    age_seconds = (now - tx_time).total_seconds()
                    
                    if age_seconds < 60:
                        age = f"{int(age_seconds)}s ago"
                    elif age_seconds < 3600:
                        age = f"{int(age_seconds/60)}m ago"
                    elif age_seconds < 86400:
                        age = f"{int(age_seconds/3600)}h ago"
                    else:
                        age = f"{int(age_seconds/86400)}d ago"
                except:
                    age = "Unknown"
            else:
                age = "Unknown"
            
            transactions.append({
                'hash': tx_hash,
                'block': block,
                'from': from_addr or 'Coinbase',
                'to': to_addr,
                'value': value or 0.0,
                'fee': fee or 0.0,
                'status': status or 'success',
                'age': age,
                'prev_tx_hash': prev_tx,
                'referenced_amount': ref_amount,
                'traceability_valid': bool(trace_valid) if trace_valid is not None else None,
                'is_coinbase': bool(is_coinbase) if is_coinbase is not None else False,
                'confirmations': max(0, current_height - block + 1) if block > 0 else 0
            })
        
        return render_template('transactions.html', transactions=transactions, network_info=network_info, current_height=current_height, forum_url=FORUM_URL)
        
    except Exception as e:
        # Log error for debugging
        import logging
        logging.error(f"Error in transactions_list: {e}")
        # Network information for display (even on error)
        network_info = {
            'network': NETWORK.upper(),
            'address_prefix': CURRENT_NETWORK['address_prefix'],
            'block_reward': CURRENT_NETWORK['block_reward'],
            'block_time': CURRENT_NETWORK['block_time'],
            'is_testnet': NETWORK == 'testnet',
            'is_mainnet': NETWORK == 'mainnet'
        }
        # Return empty list on error instead of crashing
        return render_template('transactions.html', transactions=[], network_info=network_info, forum_url=FORUM_URL)
    finally:
        conn.close()

@app.route('/validators')
def validators_list():
    """Validators list page"""
    validator_data = explorer.get_validators()
    network_info = {
        'network': NETWORK.upper(),
        'address_prefix': CURRENT_NETWORK['address_prefix'],
        'block_reward': CURRENT_NETWORK['block_reward'],
        'block_time': CURRENT_NETWORK['block_time'],
        'is_testnet': NETWORK == 'testnet',
        'is_mainnet': NETWORK == 'mainnet'
    }
    return render_template('validators.html', 
                         validators=validator_data['validators'],
                         total_staked=validator_data['total_staked'],
                         total_validators=validator_data['total_validators'],
                         network_info=network_info)

@app.route('/addresses')
def addresses_list():
    """Top addresses page"""
    address_data = explorer.get_top_addresses(100)
    network_info = {
        'network': NETWORK.upper(),
        'address_prefix': CURRENT_NETWORK['address_prefix'],
        'block_reward': CURRENT_NETWORK['block_reward'],
        'block_time': CURRENT_NETWORK['block_time'],
        'is_testnet': NETWORK == 'testnet',
        'is_mainnet': NETWORK == 'mainnet'
    }
    return render_template('addresses.html', 
                         addresses=address_data['addresses'],
                         total_addresses=address_data['total_addresses'],
                         total_supply=address_data['total_supply'],
                         network_info=network_info)

@app.route('/mining')
def mining_page():
    """Mining statistics page"""
    stats = explorer.get_network_stats()
    top_miners = explorer.get_mining_stats()
    network_info = {
        'network': NETWORK.upper(),
        'address_prefix': CURRENT_NETWORK['address_prefix'],
        'block_reward': CURRENT_NETWORK['block_reward'],
        'block_time': CURRENT_NETWORK['block_time'],
        'is_testnet': NETWORK == 'testnet',
        'is_mainnet': NETWORK == 'mainnet'
    }
    return render_template('mining.html', stats=stats, top_miners=top_miners, network_info=network_info)

@app.route('/mining/guide')
def mining_guide():
    """Mining guide page with instructions and downloads"""
    # Get Stratum URLs from environment or use defaults
    import sys
    import os
    pool_config_path = os.path.join(os.path.dirname(__file__), '..', 'mining_pool', 'pool_config.py')
    
    # Try to import pool config for Stratum URLs
    stratum_urls = {
        'general_pool': os.environ.get('GENERAL_POOL_STRATUM_URL', 'stratum+tcp://pool.gxc.network:3333'),
        'gpu_pool': os.environ.get('GPU_POOL_STRATUM_URL', 'stratum+tcp://gpu-pool.gxc.network:3334'),
        'asic_pool': os.environ.get('ASIC_POOL_STRATUM_URL', 'stratum+tcp://asic-pool.gxc.network:3335'),
        'general_dashboard': os.environ.get('GENERAL_POOL_DASHBOARD_URL', 'https://pool.gxc.network'),
        'gpu_dashboard': os.environ.get('GPU_POOL_DASHBOARD_URL', 'https://gpu-pool.gxc.network'),
        'asic_dashboard': os.environ.get('ASIC_POOL_DASHBOARD_URL', 'https://asic-pool.gxc.network')
    }
    
    # Try to get from pool_config if available
    try:
        sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'mining_pool'))
        from pool_config import (
            GENERAL_POOL_STRATUM_URL,
            GPU_POOL_STRATUM_URL,
            ASIC_POOL_STRATUM_URL,
            GENERAL_POOL_DASHBOARD_URL,
            GPU_POOL_DASHBOARD_URL,
            ASIC_POOL_DASHBOARD_URL
        )
        stratum_urls = {
            'general_pool': GENERAL_POOL_STRATUM_URL,
            'gpu_pool': GPU_POOL_STRATUM_URL,
            'asic_pool': ASIC_POOL_STRATUM_URL,
            'general_dashboard': GENERAL_POOL_DASHBOARD_URL,
            'gpu_dashboard': GPU_POOL_DASHBOARD_URL,
            'asic_dashboard': ASIC_POOL_DASHBOARD_URL
        }
    except:
        pass
    
    return render_template('mining_guide.html', stratum_urls=stratum_urls)

@app.route('/stocks')
def stocks_page():
    """Stock contracts explorer page"""
    # Get stock contracts from database
    conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
    cursor = conn.cursor()
    
    try:
        cursor.execute('''
            SELECT ticker, company_name, exchange, current_price, market_cap, total_shares,
                   dividend_yield, sector, last_price_update
            FROM stock_contracts
            WHERE is_active = 1
            ORDER BY market_cap DESC
        ''')
        
        stocks = []
        for row in cursor.fetchall():
            stocks.append({
                'ticker': row[0],
                'company_name': row[1],
                'exchange': row[2],
                'current_price': row[3] or 0.0,
                'market_cap': row[4] or 0.0,
                'total_shares': row[5] or 0,
                'dividend_yield': row[6] or 0.0,
                'sector': row[7],
                'last_price_update': row[8]
            })
        
        # If no stocks in database, return default list
        if not stocks:
            stocks = [
                {'ticker': 'AAPL', 'company_name': 'Apple Inc.', 'exchange': 'NASDAQ', 'current_price': 175.50, 'market_cap': 2750000000000, 'total_shares': 15600000000, 'dividend_yield': 0.005, 'sector': 'Technology'},
                {'ticker': 'MSFT', 'company_name': 'Microsoft Corporation', 'exchange': 'NASDAQ', 'current_price': 378.85, 'market_cap': 2810000000000, 'total_shares': 7400000000, 'dividend_yield': 0.007, 'sector': 'Technology'},
                {'ticker': 'GOOGL', 'company_name': 'Alphabet Inc.', 'exchange': 'NASDAQ', 'current_price': 142.30, 'market_cap': 1780000000000, 'total_shares': 12500000000, 'dividend_yield': 0.0, 'sector': 'Technology'},
                {'ticker': 'AMZN', 'company_name': 'Amazon.com Inc.', 'exchange': 'NASDAQ', 'current_price': 151.20, 'market_cap': 1560000000000, 'total_shares': 10300000000, 'dividend_yield': 0.0, 'sector': 'Consumer Discretionary'},
                {'ticker': 'TSLA', 'company_name': 'Tesla Inc.', 'exchange': 'NASDAQ', 'current_price': 248.50, 'market_cap': 789000000000, 'total_shares': 3200000000, 'dividend_yield': 0.0, 'sector': 'Consumer Discretionary'},
                {'ticker': 'NVDA', 'company_name': 'NVIDIA Corporation', 'exchange': 'NASDAQ', 'current_price': 485.20, 'market_cap': 1190000000000, 'total_shares': 2500000000, 'dividend_yield': 0.001, 'sector': 'Technology'},
                {'ticker': 'META', 'company_name': 'Meta Platforms Inc.', 'exchange': 'NASDAQ', 'current_price': 325.40, 'market_cap': 832000000000, 'total_shares': 2600000000, 'dividend_yield': 0.0, 'sector': 'Technology'},
                {'ticker': 'BRK-B', 'company_name': 'Berkshire Hathaway Inc.', 'exchange': 'NYSE', 'current_price': 362.75, 'market_cap': 798000000000, 'total_shares': 2200000000, 'dividend_yield': 0.0, 'sector': 'Financial Services'},
                {'ticker': 'JPM', 'company_name': 'JPMorgan Chase & Co.', 'exchange': 'NYSE', 'current_price': 158.90, 'market_cap': 460000000000, 'total_shares': 2900000000, 'dividend_yield': 0.024, 'sector': 'Financial Services'},
                {'ticker': 'JNJ', 'company_name': 'Johnson & Johnson', 'exchange': 'NYSE', 'current_price': 162.45, 'market_cap': 430000000000, 'total_shares': 2600000000, 'dividend_yield': 0.031, 'sector': 'Healthcare'}
            ]
        
        return render_template('stocks.html', stocks=stocks, forum_url=FORUM_URL)
        
    finally:
        conn.close()

@app.route('/stocks/guide')
def stocks_guide():
    """Stock contracts guide page"""
    return render_template('stocks_guide.html')

@app.route('/forum')
def forum_redirect():
    """Redirect to forum"""
    return redirect('https://web-ixr2lyfaw-philanis-projects-ef60481c.vercel.app/forum')

@app.route('/stocks/<ticker>')
def stock_detail(ticker):
    """Stock contract detail page"""
    conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
    cursor = conn.cursor()
    
    try:
        # Get stock contract info
        cursor.execute('''
            SELECT ticker, company_name, exchange, contract_address, price_oracle,
                   total_shares, current_price, market_cap, dividend_yield, sector,
                   industry, last_price_update
            FROM stock_contracts
            WHERE ticker = ? AND is_active = 1
        ''', (ticker.upper(),))
        
        stock = cursor.fetchone()
        if not stock:
            return f"Stock contract {ticker} not found", 404
        
        # Get price history
        cursor.execute('''
            SELECT price, volume, timestamp, source
            FROM stock_price_history
            WHERE ticker = ?
            ORDER BY timestamp DESC
            LIMIT 100
        ''', (ticker.upper(),))
        
        price_history = cursor.fetchall()
        
        # Get top shareholders
        cursor.execute('''
            SELECT address, balance
            FROM stock_balances
            WHERE ticker = ? AND balance > 0
            ORDER BY balance DESC
            LIMIT 20
        ''', (ticker.upper(),))
        
        shareholders = cursor.fetchall()
        
        # Get recent transfers
        cursor.execute('''
            SELECT from_address, to_address, shares, price_at_transfer, timestamp, tx_hash
            FROM stock_transfers
            WHERE ticker = ?
            ORDER BY timestamp DESC
            LIMIT 50
        ''', (ticker.upper(),))
        
        transfers = cursor.fetchall()
        
        stock_data = {
            'ticker': stock[0],
            'company_name': stock[1],
            'exchange': stock[2],
            'contract_address': stock[3],
            'price_oracle': stock[4],
            'total_shares': stock[5],
            'current_price': stock[6],
            'market_cap': stock[7],
            'dividend_yield': stock[8],
            'sector': stock[9],
            'industry': stock[10],
            'last_price_update': stock[11],
            'price_history': price_history,
            'shareholders': shareholders,
            'transfers': transfers
        }
        
        return render_template('stock_detail.html', stock=stock_data)
        
    finally:
        conn.close()

@app.route('/charts')
def charts_page():
    """Charts and analytics page"""
    chart_data = explorer.get_chart_data(14)
    return render_template('charts.html', chart_data=chart_data)

@app.route('/wallet')
def wallet_page():
    """Wallet page with wallet service information"""
    network_info = {
        'network': NETWORK.upper(),
        'address_prefix': CURRENT_NETWORK['address_prefix'],
        'block_reward': CURRENT_NETWORK['block_reward'],
        'block_time': CURRENT_NETWORK['block_time'],
        'is_testnet': NETWORK == 'testnet',
        'is_mainnet': NETWORK == 'mainnet'
    }
    return render_template('wallet.html', network_info=network_info)

@app.route('/api')
def api_docs():
    """API documentation page"""
    return render_template('api.html')

# API Routes
@app.route('/api/blocks')
def api_blocks():
    """API endpoint for recent blocks"""
    limit = safe_int(request.args.get('limit', 10), default=10, min_val=1, max_val=100)
    blocks = explorer.get_recent_blocks(limit)
    return jsonify(blocks)

@app.route('/api/transactions')
def api_transactions():
    """API endpoint for recent transactions"""
    limit = safe_int(request.args.get('limit', 20), default=20, min_val=1, max_val=100)
    transactions = explorer.get_recent_transactions(limit)
    return jsonify(transactions)

@app.route('/api/v1/stats')
@app.route('/api/stats')
def api_stats():
    """API endpoint for network statistics"""
    stats = explorer.get_network_stats()
    return jsonify(stats)

@app.route('/api/charts')
def api_charts():
    """API endpoint for chart data"""
    days = safe_int(request.args.get('days', 14), default=14, min_val=1, max_val=365)
    chart_data = explorer.get_chart_data(days)
    return jsonify(chart_data)

@app.route('/api/hashrate')
def api_hashrate():
    """API endpoint for hashrate history"""
    days = safe_int(request.args.get('days', 7), default=7, min_val=1, max_val=365)
    hashrate_data = explorer.get_hashrate_history(days)
    return jsonify(hashrate_data)

@app.route('/gas')
def gas_tracker():
    """Gas & Fee Tracker page"""
    gas_stats = explorer.get_gas_stats() or {}
    # Ensure all required keys exist
    gas_stats.setdefault('avg_gas_price', 0.0)
    gas_stats.setdefault('avg_gas_used', 0)
    gas_stats.setdefault('total_gas_used', 0)
    gas_stats.setdefault('gas_limit', 0)
    return render_template('gas_tracker.html', gas_stats=gas_stats)

@app.route('/api/gas')
def api_gas():
    """API endpoint for gas statistics"""
    gas_stats = explorer.get_gas_stats()
    return jsonify(gas_stats)

@app.route('/price')
def price_page():
    """Price & Market Data page"""
    price_data = explorer.get_price_data() or {}
    # Ensure all required keys exist
    price_data.setdefault('current_price', 0.0)
    price_data.setdefault('price_change_24h', 0.0)
    price_data.setdefault('volume_24h', 0.0)
    price_data.setdefault('market_cap', 0.0)
    return render_template('price.html', price_data=price_data)

@app.route('/api/price')
def api_price():
    """API endpoint for price data"""
    price_data = explorer.get_price_data()
    return jsonify(price_data)

@app.route('/portfolio')
def portfolio_page():
    """Portfolio Tracker page"""
    return render_template('portfolio.html')

@app.route('/api/portfolio')
def api_portfolio():
    """API endpoint for portfolio data"""
    addresses_json = request.args.get('addresses', '[]')
    try:
        addresses = json.loads(addresses_json)
        portfolio_data = explorer.get_portfolio_data(addresses)
        return jsonify(portfolio_data)
    except:
        return jsonify({'error': 'Invalid addresses'}), 400

@app.route('/health')
def network_health():
    """Network Health Dashboard"""
    health_data = explorer.get_network_health() or {}
    # Ensure all required keys exist
    health_data.setdefault('status', 'unknown')
    health_data.setdefault('uptime', 0.0)
    health_data.setdefault('peers', 0)
    health_data.setdefault('sync_status', 'unknown')
    return render_template('network_health.html', health=health_data)

@app.route('/api/health')
def api_health():
    """API endpoint for network health"""
    health_data = explorer.get_network_health()
    return jsonify(health_data)

@app.route('/api/connection-status')
def api_connection_status():
    """API endpoint for blockchain connection status"""
    return jsonify({
        'active_node': ACTIVE_NODE_TYPE or 'unknown',
        'active_url': ACTIVE_NODE_URL or 'none',
        'railway_status': CONNECTION_STATUS['railway'],
        'local_status': CONNECTION_STATUS['local'],
        'connected': ACTIVE_NODE_TYPE is not None,
        'railway_url': RAILWAY_NODE_URL,
        'local_url': LOCAL_NODE_URL
    })

@app.route('/api/network-info')
def api_network_info():
    """API endpoint for network information"""
    return jsonify({
        'network': NETWORK.upper(),
        'address_prefix': CURRENT_NETWORK['address_prefix'],
        'block_reward': CURRENT_NETWORK['block_reward'],
        'block_time': CURRENT_NETWORK['block_time'],
        'difficulty': CURRENT_NETWORK['difficulty'],
        'is_testnet': NETWORK == 'testnet',
        'is_mainnet': NETWORK == 'mainnet',
        'database_path': DATABASE_PATH
    })

@app.route('/healthz')
@app.route('/healthcheck')
def health_check():
    """Simple health check endpoint for deployment monitoring"""
    try:
        # Quick database check
        # Add timeout to prevent database locking issues
        conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
        cursor = conn.cursor()
        cursor.execute('SELECT 1')
        conn.close()
        
        # Quick RPC check (non-blocking)
        info = rpc_call('getblockchaininfo', timeout=3, retry=False, show_errors=False)
        
        return jsonify({
            'status': 'healthy',
            'database': 'ok',
            'blockchain_connected': info is not None,
            'active_node': ACTIVE_NODE_TYPE or 'unknown',
            'node_url': ACTIVE_NODE_URL if ACTIVE_NODE_URL else 'none',
            'timestamp': time.time()
        }), 200
    except Exception as e:
        return jsonify({
            'status': 'unhealthy',
            'error': str(e)[:100],
            'timestamp': time.time()
        }), 503
    health_data = explorer.get_network_health()
    return jsonify(health_data)

@app.route('/decoder')
def transaction_decoder():
    """Transaction Decoder page"""
    return render_template('decoder.html')

@app.route('/api/decode')
def api_decode():
    """API endpoint for transaction decoding"""
    input_data = request.args.get('data', '')
    decoded = explorer.decode_transaction_input(input_data)
    return jsonify(decoded)

@app.route('/analytics')
def analytics_page():
    """Analytics Dashboard"""
    analytics_data = explorer.get_analytics()
    return render_template('analytics.html', analytics=analytics_data)

@app.route('/api/analytics')
def api_analytics():
    """API endpoint for analytics"""
    analytics_data = explorer.get_analytics()
    return jsonify(analytics_data)

@app.route('/pending')
def pending_transactions():
    """Pending transactions (mempool) page"""
    pending_txs = explorer.get_pending_transactions(100)
    return render_template('pending.html', transactions=pending_txs)

@app.route('/api/pending')
def api_pending():
    """API endpoint for pending transactions"""
    limit = safe_int(request.args.get('limit', 50), default=50, min_val=1, max_val=200)
    pending_txs = explorer.get_pending_transactions(limit)
    return jsonify(pending_txs)

@app.route('/api/export/transactions')
def export_transactions():
    """Export transactions as CSV"""
    format_type = request.args.get('format', 'csv')
    limit = safe_int(request.args.get('limit', 1000), default=1000, min_val=1, max_val=10000)
    
    # Check for filters
    filters = {
        'date_from': request.args.get('date_from'),
        'date_to': request.args.get('date_to'),
        'amount_min': request.args.get('amount_min'),
        'amount_max': request.args.get('amount_max'),
        'tx_type': request.args.get('tx_type'),
        'status': request.args.get('status'),
        'address': request.args.get('address'),
        'limit': limit
    }
    
    # Remove None values
    filters = {k: v for k, v in filters.items() if v is not None}
    
    if any(filters.values()):
        transactions = explorer.get_filtered_transactions(filters)
    else:
        transactions = explorer.get_recent_transactions(limit)
    
    if format_type == 'csv':
        import csv
        from io import StringIO
        
        output = StringIO()
        writer = csv.writer(output)
        
        # Enhanced CSV with all blockchain fields
        headers = ['Hash', 'Block', 'From', 'To', 'Value', 'Fee', 'Status', 'Timestamp', 
                   'Type', 'Nonce', 'Is Coinbase', 'Prev Tx Hash', 'Referenced Amount',
                   'Traceability Valid', 'Memo', 'Lock Time', 'Is Gold Backed', 'PoP Reference']
        writer.writerow(headers)
        
        # Add timeout to prevent database locking issues
        conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
        cursor = conn.cursor()
        
        for tx in transactions:
            # Get full transaction data from database
            cursor.execute('''
                SELECT nonce, is_coinbase, prev_tx_hash, referenced_amount,
                       traceability_valid, memo, lock_time, is_gold_backed, pop_reference
                FROM transactions WHERE tx_hash = ?
            ''', (tx['hash'],))
            full_data = cursor.fetchone()
            
            writer.writerow([
                tx['hash'],
                tx['block'],
                tx['from'],
                tx.get('to', ''),
                tx['value'],
                tx['fee'],
                tx['status'],
                tx['timestamp'],
                tx['type'],
                full_data[0] if full_data else '',
                'Yes' if (full_data[1] if full_data else False) else 'No',
                full_data[2] if full_data else '',
                full_data[3] if full_data else '',
                'Yes' if (full_data[4] if full_data else True) else 'No',
                full_data[5] if full_data else '',
                full_data[6] if full_data else '',
                'Yes' if (full_data[7] if full_data else False) else 'No',
                full_data[8] if full_data else ''
            ])
        
        conn.close()
        
        from flask import Response
        return Response(
            output.getvalue(),
            mimetype='text/csv',
            headers={'Content-Disposition': 'attachment; filename=gxc_transactions_full.csv'}
        )
    else:
        return jsonify(transactions)

@app.route('/api/export/blocks')
def export_blocks():
    """Export blocks as CSV with all fields"""
    format_type = request.args.get('format', 'csv')
    limit = safe_int(request.args.get('limit', 1000), default=1000, min_val=1, max_val=10000)
    
    conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
    cursor = conn.cursor()
    
    cursor.execute('''
        SELECT block_number, block_hash, parent_hash, merkle_root, timestamp,
               miner_address, difficulty, nonce, size, transaction_count,
               reward, consensus_type, block_type, fee_burn_rate, pop_reference,
               validator_signature
        FROM blocks
        ORDER BY block_number DESC
        LIMIT ?
    ''', (limit,))
    
    blocks = cursor.fetchall()
    conn.close()
    
    if format_type == 'csv':
        import csv
        from io import StringIO
        
        output = StringIO()
        writer = csv.writer(output)
        writer.writerow(['Number', 'Hash', 'Parent Hash', 'Merkle Root', 'Timestamp',
                         'Miner', 'Difficulty', 'Nonce', 'Size', 'Tx Count',
                         'Reward', 'Consensus', 'Block Type', 'Fee Burn Rate',
                         'PoP Reference', 'Validator Signature'])
        
        for block in blocks:
            writer.writerow(block)
        
        from flask import Response
        return Response(
            output.getvalue(),
            mimetype='text/csv',
            headers={'Content-Disposition': 'attachment; filename=gxc_blocks_full.csv'}
        )
    else:
        return jsonify([{
            'number': b[0], 'hash': b[1], 'parent_hash': b[2],
            'merkle_root': b[3], 'timestamp': b[4], 'miner': b[5],
            'difficulty': b[6], 'nonce': b[7], 'size': b[8],
            'tx_count': b[9], 'reward': b[10], 'consensus': b[11],
            'block_type': b[12], 'fee_burn_rate': b[13],
            'pop_reference': b[14], 'validator_signature': b[15]
        } for b in blocks])

@app.route('/api/export/utxo')
def export_utxo():
    """Export UTXO set"""
    format_type = request.args.get('format', 'csv')
    address = request.args.get('address')
    
    conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
    cursor = conn.cursor()
    
    if address:
        cursor.execute('''
            SELECT tx_hash, output_index, address, amount, block_height
            FROM utxo WHERE address = ?
            ORDER BY block_height DESC
        ''', (address,))
    else:
        cursor.execute('''
            SELECT tx_hash, output_index, address, amount, block_height
            FROM utxo
            ORDER BY block_height DESC
            LIMIT 10000
        ''')
    
    utxos = cursor.fetchall()
    conn.close()
    
    if format_type == 'csv':
        import csv
        from io import StringIO
        
        output = StringIO()
        writer = csv.writer(output)
        writer.writerow(['Tx Hash', 'Output Index', 'Address', 'Amount', 'Block Height'])
        
        for utxo in utxos:
            writer.writerow(utxo)
        
        from flask import Response
        return Response(
            output.getvalue(),
            mimetype='text/csv',
            headers={'Content-Disposition': 'attachment; filename=gxc_utxo.csv'}
        )
    else:
        return jsonify([{
            'tx_hash': u[0], 'output_index': u[1], 'address': u[2],
            'amount': u[3], 'block_height': u[4]
        } for u in utxos])

@app.route('/api/export/all')
def export_all():
    """Export all blockchain data as ZIP"""
    from flask import Response
    import zipfile
    import io
    import csv
    
    zip_buffer = io.BytesIO()
    
    with zipfile.ZipFile(zip_buffer, 'w', zipfile.ZIP_DEFLATED) as zip_file:
        # Export transactions
        transactions = explorer.get_recent_transactions(10000)
        tx_csv = io.StringIO()
        writer = csv.writer(tx_csv)
        writer.writerow(['Hash', 'Block', 'From', 'To', 'Value', 'Fee', 'Status', 'Timestamp'])
        for tx in transactions:
            writer.writerow([tx['hash'], tx['block'], tx['from'], tx.get('to', ''), 
                            tx['value'], tx['fee'], tx['status'], tx['timestamp']])
        zip_file.writestr('transactions.csv', tx_csv.getvalue())
        
        # Export blocks
        # Add timeout to prevent database locking issues
        conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
        cursor = conn.cursor()
        cursor.execute('SELECT * FROM blocks ORDER BY block_number DESC LIMIT 10000')
        blocks = cursor.fetchall()
        block_csv = io.StringIO()
        writer = csv.writer(block_csv)
        writer.writerow(['Number', 'Hash', 'Parent Hash', 'Timestamp', 'Miner', 'Difficulty', 'Reward'])
        for block in blocks:
            writer.writerow([block[0], block[1], block[2], block[4], block[5], block[6], block[12]])
        zip_file.writestr('blocks.csv', block_csv.getvalue())
        conn.close()
    
    zip_buffer.seek(0)
    return Response(
        zip_buffer.read(),
        mimetype='application/zip',
        headers={'Content-Disposition': 'attachment; filename=gxc_blockchain_data.zip'}
    )

@app.route('/api/filter/transactions')
def api_filter_transactions():
    """API endpoint for filtered transactions"""
    filters = {
        'date_from': request.args.get('date_from'),
        'date_to': request.args.get('date_to'),
        'amount_min': request.args.get('amount_min'),
        'amount_max': request.args.get('amount_max'),
        'tx_type': request.args.get('tx_type'),
        'status': request.args.get('status'),
        'address': request.args.get('address'),
        'limit': request.args.get('limit', 100)
    }
    
    # Remove None values
    filters = {k: v for k, v in filters.items() if v is not None}
    
    transactions = explorer.get_filtered_transactions(filters)
    return jsonify(transactions)

# ==================== GOLD TOKEN (GXC-G) ROUTES ====================

@app.route('/gold')
def gold_token_explorer():
    """Gold Token (GXC-G) Explorer page"""
    stats = explorer.get_gold_token_stats() or {}
    reserves = explorer.get_gold_reserves() or []
    # Ensure all required keys exist
    stats.setdefault('total_supply', 0.0)
    stats.setdefault('total_reserves', 0.0)
    stats.setdefault('reserve_ratio', 0.0)
    stats.setdefault('unique_addresses', 0)
    stats.setdefault('top_holders', [])
    return render_template('gold_token.html', stats=stats, reserves=reserves)

@app.route('/api/gold/stats')
def api_gold_stats():
    """API endpoint for gold token statistics"""
    stats = explorer.get_gold_token_stats()
    return jsonify(stats)

@app.route('/api/gold/reserves')
def api_gold_reserves():
    """API endpoint for gold reserves"""
    reserves = explorer.get_gold_reserves()
    return jsonify(reserves)

@app.route('/api/gold/transfers')
def api_gold_transfers():
    """API endpoint for gold token transfers"""
    limit = safe_int(request.args.get('limit', 50), default=50, min_val=1, max_val=200)
    conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
    cursor = conn.cursor()
    
    cursor.execute('''
        SELECT tx_hash, from_address, to_address, amount, transfer_type,
               pop_reference, timestamp, block_height
        FROM gold_token_transfers
        ORDER BY timestamp DESC
        LIMIT ?
    ''', (limit,))
    
    transfers = cursor.fetchall()
    conn.close()
    
    return jsonify([{
        'tx_hash': t[0],
        'from': t[1],
        'to': t[2],
        'amount': t[3],
        'type': t[4],
        'pop_reference': t[5],
        'timestamp': t[6],
        'block_height': t[7]
    } for t in transfers])

@app.route('/api/gold/address/<address>')
def api_gold_address(address):
    """Get gold token balance for address"""
    conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
    cursor = conn.cursor()
    
    cursor.execute('''
        SELECT balance, total_received, total_sent, transaction_count,
               first_seen, last_activity
        FROM gold_tokens
        WHERE address = ?
    ''', (address,))
    
    data = cursor.fetchone()
    conn.close()
    
    if data:
        return jsonify({
            'address': address,
            'balance': data[0],
            'total_received': data[1],
            'total_sent': data[2],
            'transaction_count': data[3],
            'first_seen': data[4],
            'last_activity': data[5]
        })
    else:
        return jsonify({
            'address': address,
            'balance': 0.0,
            'total_received': 0.0,
            'total_sent': 0.0,
            'transaction_count': 0
        })

# ==================== GOVERNANCE ROUTES ====================

@app.route('/governance')
def governance_dashboard():
    """Governance Dashboard page"""
    proposals = explorer.get_governance_proposals()
    active_proposals = [p for p in proposals if p['status'] == 'active']
    return render_template('governance.html', 
                         proposals=proposals,
                         active_proposals=active_proposals)

@app.route('/api/governance/proposals')
def api_governance_proposals():
    """API endpoint for governance proposals"""
    status = request.args.get('status')
    proposals = explorer.get_governance_proposals(status)
    return jsonify(proposals)

@app.route('/api/governance/proposal/<proposal_id>')
def api_governance_proposal(proposal_id):
    """Get specific proposal details"""
    conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
    cursor = conn.cursor()
    
    cursor.execute('''
        SELECT proposal_id, proposer_address, title, description,
               proposal_type, parameter_type, old_value, new_value,
               status, start_block, end_block, votes_for, votes_against,
               votes_abstain, total_votes, created_at, executed_at
        FROM governance_proposals
        WHERE proposal_id = ?
    ''', (proposal_id,))
    
    proposal = cursor.fetchone()
    if not proposal:
        return jsonify({'error': 'Proposal not found'}), 404
    
    votes = explorer.get_proposal_votes(proposal_id)
    
    conn.close()
    
    return jsonify({
        'proposal_id': proposal[0],
        'proposer': proposal[1],
        'title': proposal[2],
        'description': proposal[3],
        'proposal_type': proposal[4],
        'parameter_type': proposal[5],
        'old_value': proposal[6],
        'new_value': proposal[7],
        'status': proposal[8],
        'start_block': proposal[9],
        'end_block': proposal[10],
        'votes_for': proposal[11],
        'votes_against': proposal[12],
        'votes_abstain': proposal[13],
        'total_votes': proposal[14],
        'created_at': proposal[15],
        'executed_at': proposal[16],
        'votes': votes
    })

@app.route('/api/governance/proposal/<proposal_id>/votes')
def api_proposal_votes(proposal_id):
    """Get votes for a proposal"""
    votes = explorer.get_proposal_votes(proposal_id)
    return jsonify(votes)

# ==================== CROSS-CHAIN BRIDGE ROUTES ====================

@app.route('/bridge')
def bridge_explorer():
    """Cross-Chain Bridge Explorer page"""
    transfers = explorer.get_bridge_transfers(limit=50)
    validators = explorer.get_bridge_validators()
    
    # Calculate bridge stats
    conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
    cursor = conn.cursor()
    
    cursor.execute('''
        SELECT 
            COUNT(*) as total_transfers,
            SUM(amount) as total_volume,
            COUNT(DISTINCT source_chain) as chains_supported
        FROM bridge_transfers
        WHERE status = 'confirmed'
    ''')
    
    stats = cursor.fetchone()
    conn.close()
    
    bridge_stats = {
        'total_transfers': stats[0] or 0,
        'total_volume': stats[1] or 0.0,
        'chains_supported': stats[2] or 0
    }
    
    return render_template('bridge.html',
                         transfers=transfers,
                         validators=validators,
                         stats=bridge_stats)

@app.route('/api/bridge/transfers')
def api_bridge_transfers():
    """API endpoint for bridge transfers"""
    status = request.args.get('status')
    limit = safe_int(request.args.get('limit', 50), default=50, min_val=1, max_val=200)
    transfers = explorer.get_bridge_transfers(status, limit)
    return jsonify(transfers)

@app.route('/api/bridge/validators')
def api_bridge_validators():
    """API endpoint for bridge validators"""
    validators = explorer.get_bridge_validators()
    return jsonify(validators)

@app.route('/api/bridge/stats')
def api_bridge_stats():
    """API endpoint for bridge statistics"""
    conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
    cursor = conn.cursor()
    
    cursor.execute('''
        SELECT 
            source_chain,
            COUNT(*) as transfer_count,
            SUM(amount) as total_volume
        FROM bridge_transfers
        WHERE status = 'confirmed'
        GROUP BY source_chain
    ''')
    
    chain_stats = []
    for row in cursor.fetchall():
        chain_stats.append({
            'chain': row[0],
            'transfer_count': row[1],
            'total_volume': row[2]
        })
    
    conn.close()
    
    return jsonify({
        'chain_stats': chain_stats
    })

# ==================== STAKING ROUTES ====================

@app.route('/staking')
def staking_dashboard():
    """Staking Dashboard page"""
    validators = explorer.get_staking_validators()
    
    # Calculate total staked
    total_staked = sum(v['stake_amount'] for v in validators)
    total_rewards = sum(v['total_rewards'] for v in validators)
    active_validators = len([v for v in validators if v['is_active']])
    
    return render_template('staking.html',
                         validators=validators,
                         total_staked=total_staked,
                         total_rewards=total_rewards,
                         active_validators=active_validators)

@app.route('/api/staking/validators')
def api_staking_validators():
    """API endpoint for staking validators"""
    validators = explorer.get_staking_validators()
    return jsonify(validators)

@app.route('/api/staking/validator/<address>')
def api_staking_validator(address):
    """Get specific validator details"""
    conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
    cursor = conn.cursor()
    
    cursor.execute('''
        SELECT validator_address, stake_amount, staking_days, stake_start_time,
               is_active, blocks_produced, missed_blocks, uptime,
               total_rewards, pending_rewards, is_slashed, slashed_amount
        FROM staking_validators
        WHERE validator_address = ?
    ''', (address,))
    
    validator = cursor.fetchone()
    if not validator:
        return jsonify({'error': 'Validator not found'}), 404
    
    # Get delegations
    cursor.execute('''
        SELECT delegator_address, amount, start_time, total_rewards, is_active
        FROM staking_delegations
        WHERE validator_address = ?
        ORDER BY amount DESC
    ''', (address,))
    
    delegations = []
    for row in cursor.fetchall():
        delegations.append({
            'delegator': row[0],
            'amount': row[1],
            'start_time': row[2],
            'total_rewards': row[3],
            'is_active': bool(row[4])
        })
    
    # Get recent rewards
    cursor.execute('''
        SELECT amount, block_number, timestamp
        FROM staking_rewards
        WHERE validator_address = ?
        ORDER BY timestamp DESC
        LIMIT 20
    ''', (address,))
    
    rewards = []
    for row in cursor.fetchall():
        rewards.append({
            'amount': row[0],
            'block_number': row[1],
            'timestamp': row[2]
        })
    
    conn.close()
    
    return jsonify({
        'address': validator[0],
        'stake_amount': validator[1],
        'staking_days': validator[2],
        'stake_start_time': validator[3],
        'is_active': bool(validator[4]),
        'blocks_produced': validator[5],
        'missed_blocks': validator[6],
        'uptime': validator[7],
        'total_rewards': validator[8],
        'pending_rewards': validator[9],
        'is_slashed': bool(validator[10]),
        'slashed_amount': validator[11],
        'delegations': delegations,
        'recent_rewards': rewards
    })

@app.route('/api/staking/stats')
def api_staking_stats():
    """API endpoint for staking statistics"""
    validators = explorer.get_staking_validators()
    
    total_staked = sum(v['stake_amount'] for v in validators)
    total_rewards = sum(v['total_rewards'] for v in validators)
    active_validators = len([v for v in validators if v['is_active']])
    
    return jsonify({
        'total_staked': total_staked,
        'total_rewards': total_rewards,
        'active_validators': active_validators,
        'total_validators': len(validators),
        'average_apy': sum(v.get('apy', 0) for v in validators) / len(validators) if validators else 0.0
    })

# ==================== TRANSACTION GRAPH ROUTES ====================

@app.route('/graph')
def transaction_graph():
    """Transaction Graph Visualization page"""
    return render_template('transaction_graph.html')

@app.route('/api/graph/address/<address>')
def api_address_graph(address):
    """Get address relationship graph data"""
    relationships = explorer.get_address_relationships(address, 100)
    
    # Build graph structure
    nodes = [{'id': address, 'label': address[:16] + '...', 'type': 'center'}]
    edges = []
    
    for rel in relationships:
        nodes.append({
            'id': rel['address'],
            'label': rel['address'][:16] + '...',
            'type': 'related',
            'transaction_count': rel['transaction_count'],
            'total_value': rel['total_value']
        })
        edges.append({
            'from': address,
            'to': rel['address'],
            'value': rel['transaction_count'],
            'label': f"{rel['transaction_count']} txs"
        })
    
    return jsonify({
        'nodes': nodes,
        'edges': edges
    })

@app.route('/api/graph/transaction/<tx_hash>')
def api_transaction_graph(tx_hash):
    """Get transaction flow graph"""
    conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
    cursor = conn.cursor()
    
    # Get transaction
    cursor.execute('SELECT from_address, to_address, value FROM transactions WHERE tx_hash = ?', (tx_hash,))
    tx = cursor.fetchone()
    
    if not tx:
        # Try to fetch from node before giving up
        tx_data = rpc_call('gettransaction', [tx_hash, True], timeout=15, show_errors=False)
        if not tx_data:
            tx_data = rpc_call('getrawtransaction', [tx_hash, True], timeout=15, show_errors=False)
        
        if tx_data:
            # Store the transaction
            explorer = BlockchainExplorer()
            block_num = tx_data.get('blockNumber') or tx_data.get('block_number') or 0
            if block_num > 0:
                block_data = explorer.get_block_by_number(block_num)
                if block_data:
                    explorer.store_block(block_data)
                explorer.store_transaction(tx_data, block_num, tx_data.get('index', 0), block_data if block_num > 0 else None)
            else:
                explorer.store_transaction(tx_data, 0, tx_data.get('index', 0), None)
            
            # Re-fetch from database
            cursor.execute('SELECT from_address, to_address, value FROM transactions WHERE tx_hash = ?', (tx_hash,))
            tx = cursor.fetchone()
    
    if not tx:
        return jsonify({'error': 'Transaction not found'}), 404
    
    # Get inputs and outputs
    cursor.execute('SELECT prev_tx_hash, amount FROM transaction_inputs WHERE transaction_hash = ?', (tx_hash,))
    inputs = cursor.fetchall()
    
    cursor.execute('SELECT address, amount FROM transaction_outputs WHERE transaction_hash = ?', (tx_hash,))
    outputs = cursor.fetchall()
    
    conn.close()
    
    nodes = []
    edges = []
    
    # Add transaction node
    nodes.append({'id': tx_hash, 'label': tx_hash[:16] + '...', 'type': 'transaction'})
    
    # Add input nodes
    for inp in inputs:
        nodes.append({'id': inp[0], 'label': inp[0][:16] + '...', 'type': 'input'})
        edges.append({'from': inp[0], 'to': tx_hash, 'value': inp[1], 'label': f"{inp[1]} GXC"})
    
    # Add output nodes
    for out in outputs:
        nodes.append({'id': out[0], 'label': out[0][:16] + '...', 'type': 'output'})
        edges.append({'from': tx_hash, 'to': out[0], 'value': out[1], 'label': f"{out[1]} GXC"})
    
    return jsonify({
        'nodes': nodes,
        'edges': edges
    })

# ==================== PRICE ALERTS ROUTES ====================

@app.route('/alerts')
def price_alerts():
    """Price Alerts page"""
    return render_template('price_alerts.html')

@app.route('/api/alerts')
def api_alerts():
    """Get user's price alerts (from localStorage on frontend)"""
    # Alerts are stored in localStorage, but we can provide API for server-side alerts
    return jsonify({'message': 'Use localStorage for client-side alerts'})

# ==================== TRANSACTION SIMULATOR ROUTES ====================

@app.route('/simulator')
def transaction_simulator():
    """Transaction Simulator page"""
    gas_stats = explorer.get_gas_stats() or {}
    gas_stats.setdefault('slow', 0.0001)
    gas_stats.setdefault('standard', 0.0002)
    gas_stats.setdefault('fast', 0.0005)
    gas_stats.setdefault('average', 0.0002)
    gas_stats.setdefault('median', 0.0002)
    return render_template('transaction_simulator.html', gas_stats=gas_stats)

@app.route('/api/simulator/estimate')
def api_simulator_estimate():
    """Estimate transaction fee and confirmation time"""
    amount = safe_float(request.args.get('amount', 0), default=0.0, min_val=0.0)
    gas_price = safe_float(request.args.get('gas_price', 0), default=0.0, min_val=0.0)
    gas_limit = safe_int(request.args.get('gas_limit', 21000), default=21000, min_val=21000, max_val=10000000)
    
    # Calculate fee
    fee = gas_price * gas_limit
    
    # Estimate confirmation time (based on current block time)
    conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
    cursor = conn.cursor()
    
    cursor.execute('''
        SELECT AVG(
            (julianday(b2.timestamp) - julianday(b1.timestamp)) * 86400
        ) as avg_time
        FROM blocks b1
        JOIN blocks b2 ON b2.block_number = b1.block_number + 1
        WHERE b1.block_number >= (SELECT MAX(block_number) - 100 FROM blocks)
    ''')
    
    avg_block_time = cursor.fetchone()[0] or 150.0  # Default 150 seconds
    conn.close()
    
    # Estimate blocks to confirmation (based on gas price tier)
    gas_stats = explorer.get_gas_stats()
    
    if gas_price >= gas_stats.get('fast', 0.0005):
        blocks_to_confirm = 1
    elif gas_price >= gas_stats.get('standard', 0.0002):
        blocks_to_confirm = 2
    else:
        blocks_to_confirm = 5
    
    estimated_time = blocks_to_confirm * avg_block_time
    
    return jsonify({
        'fee': fee,
        'gas_price': gas_price,
        'gas_limit': gas_limit,
        'estimated_blocks': blocks_to_confirm,
        'estimated_time_seconds': estimated_time,
        'estimated_time_minutes': estimated_time / 60
    })

# ==================== API AUTHENTICATION ROUTES ====================

@app.route('/api/keys')
def api_keys_page():
    """API Keys management page"""
    return render_template('api_keys.html')

@app.route('/api/keys/generate', methods=['POST'])
def api_generate_key():
    """Generate new API key"""
    import secrets
    import hashlib
    
    user_id = request.json.get('user_id', 'anonymous')
    name = request.json.get('name', 'API Key')
    rate_limit = int(request.json.get('rate_limit', 100))
    
    # Generate API key
    api_key = 'gxc_' + secrets.token_urlsafe(32)
    key_hash = hashlib.sha256(api_key.encode()).hexdigest()
    
    conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
    cursor = conn.cursor()
    
    cursor.execute('''
        INSERT INTO api_keys (api_key, user_id, name, rate_limit)
        VALUES (?, ?, ?, ?)
    ''', (key_hash, user_id, name, rate_limit))
    
    conn.commit()
    conn.close()
    
    return jsonify({
        'api_key': api_key,
        'name': name,
        'rate_limit': rate_limit,
        'message': 'Store this key securely - it will not be shown again'
    })

@app.route('/api/keys/validate', methods=['POST'])
def api_validate_key():
    """Validate API key"""
    import hashlib
    
    api_key = request.json.get('api_key', '')
    key_hash = hashlib.sha256(api_key.encode()).hexdigest()
    
    conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
    cursor = conn.cursor()
    
    cursor.execute('''
        SELECT api_key, rate_limit, requests_today, is_active
        FROM api_keys
        WHERE api_key = ? AND is_active = 1
    ''', (key_hash,))
    
    key_data = cursor.fetchone()
    conn.close()
    
    if key_data:
        return jsonify({
            'valid': True,
            'rate_limit': key_data[1],
            'requests_today': key_data[2]
        })
    else:
        return jsonify({'valid': False}), 401

# ==================== WEBHOOK ROUTES ====================

@app.route('/webhooks')
def webhooks_page():
    """Webhooks management page"""
    return render_template('webhooks.html')

@app.route('/api/webhooks', methods=['GET'])
def api_list_webhooks():
    """List webhooks"""
    conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
    cursor = conn.cursor()
    
    cursor.execute('''
        SELECT id, url, events, is_active, last_delivery,
               delivery_count, failure_count, created_at
        FROM webhooks
        ORDER BY created_at DESC
    ''')
    
    webhooks = []
    for row in cursor.fetchall():
        webhooks.append({
            'id': row[0],
            'url': row[1],
            'events': row[2].split(',') if row[2] else [],
            'is_active': bool(row[3]),
            'last_delivery': row[4],
            'delivery_count': row[5],
            'failure_count': row[6],
            'created_at': row[7]
        })
    
    conn.close()
    return jsonify(webhooks)

@app.route('/api/webhooks', methods=['POST'])
def api_create_webhook():
    """Create new webhook"""
    url = request.json.get('url')
    events = ','.join(request.json.get('events', []))
    secret = request.json.get('secret', '')
    
    if not url:
        return jsonify({'error': 'URL is required'}), 400
    
    conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
    cursor = conn.cursor()
    
    cursor.execute('''
        INSERT INTO webhooks (url, events, secret)
        VALUES (?, ?, ?)
    ''', (url, events, secret))
    
    webhook_id = cursor.lastrowid
    conn.commit()
    conn.close()
    
    return jsonify({
        'id': webhook_id,
        'url': url,
        'events': events.split(',') if events else [],
        'message': 'Webhook created successfully'
    })

@app.route('/api/webhooks/<int:webhook_id>', methods=['DELETE'])
def api_delete_webhook(webhook_id):
    """Delete webhook"""
    conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
    cursor = conn.cursor()
    
    cursor.execute('DELETE FROM webhooks WHERE id = ?', (webhook_id,))
    conn.commit()
    conn.close()
    
    return jsonify({'message': 'Webhook deleted'})

# WebSocket Events
@socketio.on('connect')
def handle_connect():
    """Handle client connection"""
    emit('connected', {'message': 'Connected to GXC Explorer'})

@socketio.on('disconnect')
def handle_disconnect():
    """Handle client disconnection"""
    print('Client disconnected')

# ==================== MINER DOWNLOADS ====================

@app.route('/downloads')
def downloads_page():
    # Get Stratum URLs from environment or use defaults
    import sys
    import os
    
    stratum_urls = {
        'general_pool': os.environ.get('GENERAL_POOL_STRATUM_URL', 'stratum+tcp://pool.gxc.network:3333'),
        'gpu_pool': os.environ.get('GPU_POOL_STRATUM_URL', 'stratum+tcp://gpu-pool.gxc.network:3334'),
        'asic_pool': os.environ.get('ASIC_POOL_STRATUM_URL', 'stratum+tcp://asic-pool.gxc.network:3335'),
    }
    
    # Try to get from pool_config if available
    try:
        sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'mining_pool'))
        from pool_config import (
            GENERAL_POOL_STRATUM_URL,
            GPU_POOL_STRATUM_URL,
            ASIC_POOL_STRATUM_URL
        )
        stratum_urls = {
            'general_pool': GENERAL_POOL_STRATUM_URL,
            'gpu_pool': GPU_POOL_STRATUM_URL,
            'asic_pool': ASIC_POOL_STRATUM_URL
        }
    except:
        pass
    
    """Miner downloads page"""
    return render_template('downloads.html', stratum_urls=stratum_urls)

@app.route('/api/downloads/<platform>/<algorithm>')
def download_miner(platform, algorithm):
    """Download miner binary"""
    from flask import send_file, abort
    
    # Map algorithm names
    algo_map = {
        'sha256': 'sha256_miner_gui',
        'ethash': 'ethash_miner_gui',
        'gxhash': 'gxhash_miner_gui'
    }
    
    if algorithm not in algo_map:
        abort(404)
    
    miner_name = algo_map[algorithm]
    
    # Determine file path
    if platform == 'windows':
        file_path = f'miners/installers/windows/{miner_name}-windows-x64.zip'
        mimetype = 'application/zip'
    elif platform == 'linux':
        file_path = f'miners/installers/linux/{miner_name}-linux-x64.tar.gz'
        mimetype = 'application/gzip'
    else:
        abort(404)
    
    # Check if file exists
    if not os.path.exists(file_path):
        # Return a helpful message if miners haven't been built yet
        return jsonify({
            'error': 'Miner not built yet',
            'message': 'Please run ./build_miners_installers.sh to build the miners',
            'platform': platform,
            'algorithm': algorithm
        }), 404
    
    return send_file(file_path, as_attachment=True, mimetype=mimetype)

if __name__ == '__main__':
    # Ensure database is initialized
    explorer.init_database()
    
    # Run the application
    socketio.run(app, host='0.0.0.0', port=3000, debug=False)