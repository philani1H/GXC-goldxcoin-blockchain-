#!/usr/bin/env python3
"""
GXC Wallet Creation API Service
Provides wallet creation, authentication, and credential management
"""

import os
import json
import hashlib
import secrets
import sqlite3
import bcrypt
import jwt
import qrcode
import requests
from datetime import datetime, timedelta, timezone
from flask import Flask, request, jsonify, render_template, session, redirect, url_for, flash
from flask_cors import CORS
from cryptography.hazmat.primitives.asymmetric import ed25519
from cryptography.hazmat.primitives import serialization
import base64
import uuid
import logging
import hmac

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

app = Flask(__name__, template_folder='templates')
CORS(app)

# Import centralized network configuration
import sys
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))
from config.railway_config import (
    get_network_info, get_rpc_url, get_rest_url, get_wallet_database_path,
    get_network, get_network_config, CURRENT_NETWORK
)

# Configuration
app.secret_key = os.environ.get('SECRET_KEY', secrets.token_hex(32))
JWT_SECRET = os.environ.get('JWT_SECRET', secrets.token_hex(32))

# Network-aware database path
DATABASE_PATH = get_wallet_database_path()
BACKUP_STORAGE_PATH = 'wallet_backups/'

# Network/Chain Configuration - Uses centralized config
NETWORK_INFO = get_network_info()
# Add additional fields for wallet service
NETWORK_INFO.update({
    'explorer_url': os.environ.get('EXPLORER_URL', 'http://localhost:3000'),
    'api_url': os.environ.get('WALLET_API_URL', 'http://localhost:5000'),
    'consensus': 'Hybrid PoW/PoS',
    'block_time': f"{CURRENT_NETWORK['block_time']} seconds",
})

class BlockchainClient:
    """Client to interact with GXC blockchain for real data - Auto-detects network from server"""
    
    def __init__(self, rest_url=None, rpc_url=None, expected_network=None):
        self.rest_url = rest_url or NETWORK_INFO['rest_url']
        self.rpc_url = rpc_url or get_rpc_url()
        self.timeout = 10
        self.expected_network = expected_network or get_network()
        self.detected_network = None
        self.network_detected = False
        
        # Detect network from blockchain server on initialization
        self.detect_server_network()
    
    def detect_server_network(self):
        """Detect the actual network of the blockchain server - improved detection"""
        try:
            # Try RPC getblockchaininfo first
            payload = {
                "jsonrpc": "2.0",
                "method": "getblockchaininfo",
                "params": [],
                "id": 1
            }
            
            response = requests.post(self.rpc_url, json=payload, timeout=self.timeout)
            if response.status_code == 200:
                result = response.json().get('result')
                if result:
                    # Check for network indicators
                    chain = result.get('chain', '').lower()
                    network_name = result.get('network', '').lower()
                    chain_id = result.get('chainid', '').lower()
                    
                    # Detect network from various fields
                    if 'testnet' in chain or 'testnet' in network_name or 'testnet' in chain_id:
                        self.detected_network = 'testnet'
                    elif 'mainnet' in chain or 'mainnet' in network_name or 'main' in chain:
                        self.detected_network = 'mainnet'
                    else:
                        # Check address prefix from latest block or any block
                        latest_block = result.get('blocks', 0) or result.get('height', 0)
                        # Try latest block, or block 0, or block 1
                        blocks_to_try = [latest_block, 0, 1] if latest_block > 0 else [0, 1]
                        
                        for block_num in blocks_to_try:
                            try:
                                block_payload = {
                                    "jsonrpc": "2.0",
                                    "method": "getblock",
                                    "params": [int(block_num), True],  # verbose=true
                                    "id": 1
                                }
                                block_response = requests.post(self.rpc_url, json=block_payload, timeout=self.timeout)
                                if block_response.status_code == 200:
                                    block_result = block_response.json().get('result')
                                    if block_result:
                                        # Check miner address
                                        miner = block_result.get('miner', '') or block_result.get('miner_address', '')
                                        if miner:
                                            if miner.startswith('tGXC'):
                                                self.detected_network = 'testnet'
                                                break
                                            elif miner.startswith('GXC') and not miner.startswith('tGXC'):
                                                self.detected_network = 'mainnet'
                                                break
                                        
                                        # Check transactions for address prefixes
                                        transactions = block_result.get('transactions', [])
                                        if transactions:
                                            for tx in transactions[:5]:  # Check first 5 transactions
                                                if isinstance(tx, dict):
                                                    from_addr = tx.get('from', '') or tx.get('from_address', '')
                                                    to_addr = tx.get('to', '') or tx.get('to_address', '')
                                                    if from_addr.startswith('tGXC') or to_addr.startswith('tGXC'):
                                                        self.detected_network = 'testnet'
                                                        break
                                                    elif (from_addr.startswith('GXC') and not from_addr.startswith('tGXC')) or \
                                                         (to_addr.startswith('GXC') and not to_addr.startswith('tGXC')):
                                                        self.detected_network = 'mainnet'
                                                        break
                                            if self.detected_network:
                                                break
                            except:
                                continue
                    
                    # If still not detected, default based on RPC port (testnet uses 18332, mainnet uses 8332)
                    if not self.detected_network:
                        if ':18332' in self.rpc_url or ':18080' in self.rest_url:
                            self.detected_network = 'testnet'
                        elif ':8332' in self.rpc_url or ':8080' in self.rest_url:
                            self.detected_network = 'mainnet'
                        else:
                            # Last resort: use expected network
                            self.detected_network = self.expected_network
                    
                    self.network_detected = True
                    logger.info(f"✅ Detected blockchain server network: {self.detected_network}")
                    
                    # Auto-update expected network to match detected network
                    if self.detected_network != self.expected_network:
                        logger.warning(f"⚠️ Network mismatch detected! Auto-adjusting wallet from {self.expected_network} to {self.detected_network}")
                        logger.warning(f"   Wallet was configured for: {self.expected_network}")
                        logger.warning(f"   Blockchain server is: {self.detected_network}")
                        logger.warning(f"   Auto-adjusting wallet to match server network...")
                        # Update expected network to match detected
                        self.expected_network = self.detected_network
                    
                    return self.detected_network
        except Exception as e:
            logger.warning(f"Could not detect server network: {e}")
            import traceback
            traceback.print_exc()
            # Fallback: assume network matches expected
            self.detected_network = self.expected_network
            self.network_detected = False
            return self.detected_network
        
        # Fallback: use expected network
        self.detected_network = self.expected_network
        self.network_detected = False
        return self.detected_network
    
    def get_address_balance(self, address):
        """Get real balance from blockchain - Verifies network match first"""
        # Verify network before getting balance
        if not self.network_detected:
            self.detect_server_network()
        
        # Check if address prefix matches detected network (but don't block the request)
        if self.detected_network == 'testnet' and not address.startswith('tGXC'):
            logger.debug(f"Note: Address {address[:16]}... has mainnet prefix but server is testnet - will still query")
        elif self.detected_network == 'mainnet' and not address.startswith('GXC'):
            logger.debug(f"Note: Address {address[:16]}... has testnet prefix but server is mainnet - will still query")
        
        try:
            # Try REST API first
            url = f"{self.rest_url}/api/v1/address/{address}/balance"
            response = requests.get(url, timeout=self.timeout)
            if response.status_code == 200:
                data = response.json()
                balance = float(data.get('balance', 0.0))
                logger.info(f"Balance fetched from REST API ({self.detected_network}): {address[:16]}... = {balance} GXC")
                return balance
            
            # Fallback to RPC if REST fails - use correct RPC endpoint
            logger.warning(f"REST API failed ({response.status_code}), trying RPC getbalance...")
            
            # Use correct RPC endpoint: getbalance takes only address parameter
            payload = {
                "jsonrpc": "2.0",
                "method": "getbalance",
                "params": [address],  # Only address parameter, no verbose flag
                "id": 1
            }
            rpc_response = requests.post(self.rpc_url, json=payload, timeout=self.timeout)
            if rpc_response.status_code == 200:
                result = rpc_response.json().get('result')
                if result is not None:
                    # RPC getbalance returns balance as float directly
                    if isinstance(result, dict):
                        balance = float(result.get('balance', 0.0))
                    else:
                        balance = float(result) if result else 0.0
                    logger.info(f"Balance fetched from RPC ({self.detected_network}): {address[:16]}... = {balance} GXC")
                    return balance
            
            return 0.0
        except requests.exceptions.Timeout:
            logger.error(f"Timeout fetching balance for {address}")
            return 0.0
        except Exception as e:
            logger.error(f"Error fetching balance for {address}: {e}")
            return 0.0
    
    def get_address_transactions(self, address, limit=50):
        """Get real transactions from blockchain - uses RPC to get ALL transaction data"""
        try:
            # Try REST API first
            url = f"{self.rest_url}/api/v1/address/{address}/transactions"
            response = requests.get(url, params={'limit': limit}, timeout=self.timeout)
            if response.status_code == 200:
                transactions = response.json()
                if transactions and isinstance(transactions, list) and len(transactions) > 0:
                    return transactions
            
            # Fallback to RPC: use listtransactions to get full transaction data
            logger.info(f"REST API failed, using RPC listtransactions for {address[:16]}...")
            payload = {
                "jsonrpc": "2.0",
                "method": "listtransactions",
                "params": [address, limit, 0],  # address, count, skip
                "id": 1
            }
            rpc_response = requests.post(self.rpc_url, json=payload, timeout=self.timeout)
            if rpc_response.status_code == 200:
                result = rpc_response.json().get('result')
                if result and isinstance(result, list):
                    logger.info(f"Got {len(result)} transactions from RPC for {address[:16]}...")
                    return result
            
            # Alternative: get transaction IDs and fetch full data
            logger.info(f"Trying getaddresstxids + gettransaction for {address[:16]}...")
            txids_payload = {
                "jsonrpc": "2.0",
                "method": "getaddresstxids",
                "params": [address],
                "id": 1
            }
            txids_response = requests.post(self.rpc_url, json=txids_payload, timeout=self.timeout)
            if txids_response.status_code == 200:
                txids_result = txids_response.json().get('result')
                if txids_result and isinstance(txids_result, list):
                    # Fetch full transaction data for each ID
                    transactions = []
                    for tx_hash in txids_result[:limit]:
                        tx_data = self.get_transaction(tx_hash)
                        if tx_data:
                            transactions.append(tx_data)
                    if transactions:
                        logger.info(f"Got {len(transactions)} transactions via getaddresstxids for {address[:16]}...")
                        return transactions
            
            logger.warning(f"Failed to get transactions for {address}: all methods failed")
            return []
        except Exception as e:
            logger.error(f"Error fetching transactions for {address}: {e}")
            return []
    
    def get_transaction(self, tx_hash):
        """Get transaction details from blockchain - uses RPC to get ALL transaction fields"""
        try:
            # Try REST API first
            url = f"{self.rest_url}/api/v1/transaction/{tx_hash}"
            response = requests.get(url, timeout=self.timeout)
            if response.status_code == 200:
                tx_data = response.json()
                if tx_data and isinstance(tx_data, dict):
                    return tx_data
            
            # Fallback to RPC: use gettransaction with verbose=true to get ALL fields
            logger.info(f"REST API failed, using RPC gettransaction for {tx_hash[:16]}...")
            payload = {
                "jsonrpc": "2.0",
                "method": "gettransaction",
                "params": [tx_hash, True],  # verbose=true returns full transaction data
                "id": 1
            }
            rpc_response = requests.post(self.rpc_url, json=payload, timeout=self.timeout)
            if rpc_response.status_code == 200:
                result = rpc_response.json().get('result')
                if result and isinstance(result, dict):
                    logger.info(f"Got transaction data from RPC for {tx_hash[:16]}...")
                    return result
            
            # Alternative: try getrawtransaction with verbose=true
            logger.info(f"Trying getrawtransaction for {tx_hash[:16]}...")
            raw_payload = {
                "jsonrpc": "2.0",
                "method": "getrawtransaction",
                "params": [tx_hash, True],  # verbose=true returns full JSON
                "id": 1
            }
            raw_response = requests.post(self.rpc_url, json=raw_payload, timeout=self.timeout)
            if raw_response.status_code == 200:
                raw_result = raw_response.json().get('result')
                if raw_result and isinstance(raw_result, dict):
                    logger.info(f"Got transaction data from getrawtransaction for {tx_hash[:16]}...")
                    return raw_result
            
            logger.warning(f"Failed to get transaction {tx_hash}: all methods failed")
            return None
        except Exception as e:
            logger.error(f"Error fetching transaction {tx_hash}: {e}")
            return None
    
    def submit_transaction(self, tx_data):
        """Submit transaction to blockchain"""
        try:
            url = f"{self.rest_url}/api/v1/transactions"
            response = requests.post(url, json=tx_data, timeout=self.timeout)
            if response.status_code == 200:
                return response.json()
            else:
                logger.error(f"Failed to submit transaction: {response.status_code}")
                return {'success': False, 'error': f'HTTP {response.status_code}'}
        except Exception as e:
            logger.error(f"Error submitting transaction: {e}")
            return {'success': False, 'error': str(e)}
    
    def get_blockchain_info(self):
        """Get blockchain information"""
        try:
            url = f"{self.rest_url}/api/v1/blockchain/info"
            response = requests.get(url, timeout=self.timeout)
            if response.status_code == 200:
                return response.json()
            else:
                return None
        except Exception as e:
            logger.error(f"Error fetching blockchain info: {e}")
            return None
    
    def get_utxos(self, address):
        """Get UTXOs for an address"""
        try:
            url = f"{self.rest_url}/api/v1/address/{address}/utxo"
            response = requests.get(url, timeout=self.timeout)
            if response.status_code == 200:
                return response.json()
            else:
                return []
        except Exception as e:
            logger.error(f"Error fetching UTXOs for {address}: {e}")
            return []
    
    def register_validator(self, address, stake_amount, staking_days, public_key):
        """Register as a validator via RPC"""
        try:
            # Use RPC API for validator registration
            rpc_url = self.rest_url.replace('/api/v1', '') if '/api/v1' in self.rest_url else self.rest_url
            
            payload = {
                "jsonrpc": "2.0",
                "method": "registervalidator",
                "params": [address, stake_amount, staking_days, public_key],
                "id": 1
            }
            
            response = requests.post(rpc_url, json=payload, timeout=self.timeout)
            if response.status_code == 200:
                result = response.json()
                if 'result' in result:
                    return result['result']
                elif 'error' in result:
                    error_msg = result['error'].get('message', 'Unknown error')
                    # Check if this is a "wallet does not control address" error
                    if 'does not control address' in error_msg.lower() or 'importprivkey' in error_msg.lower():
                        # Try external validator registration instead
                        logger.info(f"Standard registration failed, trying external validator registration for {address}")
                        return self.register_external_validator(address, stake_amount, staking_days, public_key)
                    return {'success': False, 'error': error_msg}
            return {'success': False, 'error': f'HTTP {response.status_code}'}
        except Exception as e:
            logger.error(f"Error registering validator: {e}")
            return {'success': False, 'error': str(e)}
    
    def register_external_validator(self, address, stake_amount, staking_days, public_key, signature=None, message=None):
        """Register as a validator for third-party/external wallets via RPC
        
        This method allows developers using third-party wallets to register as validators.
        If signature is provided, it proves ownership of the address.
        If not provided, the system will check if the address has sufficient balance.
        """
        try:
            rpc_url = self.rest_url.replace('/api/v1', '') if '/api/v1' in self.rest_url else self.rest_url
            
            # Create a message if not provided
            if not message:
                message = f"register_validator:{address}:{int(stake_amount)}:{staking_days}"
            
            # Use signature if provided, otherwise use empty string
            sig = signature if signature else ""
            
            payload = {
                "jsonrpc": "2.0",
                "method": "registerexternalvalidator",
                "params": [address, stake_amount, staking_days, sig, message, public_key],
                "id": 1
            }
            
            response = requests.post(rpc_url, json=payload, timeout=self.timeout)
            if response.status_code == 200:
                result = response.json()
                if 'result' in result:
                    return result['result']
                elif 'error' in result:
                    return {'success': False, 'error': result['error'].get('message', 'Unknown error')}
            return {'success': False, 'error': f'HTTP {response.status_code}'}
        except Exception as e:
            logger.error(f"Error registering external validator: {e}")
            return {'success': False, 'error': str(e)}
    
    def import_private_key(self, private_key_hex, label=""):
        """Import a private key to the node wallet for third-party wallet support"""
        try:
            rpc_url = self.rest_url.replace('/api/v1', '') if '/api/v1' in self.rest_url else self.rest_url
            
            payload = {
                "jsonrpc": "2.0",
                "method": "importprivkey",
                "params": [private_key_hex, label],
                "id": 1
            }
            
            response = requests.post(rpc_url, json=payload, timeout=self.timeout)
            if response.status_code == 200:
                result = response.json()
                if 'result' in result:
                    return result['result']
                elif 'error' in result:
                    return {'success': False, 'error': result['error'].get('message', 'Unknown error')}
            return {'success': False, 'error': f'HTTP {response.status_code}'}
        except Exception as e:
            logger.error(f"Error importing private key: {e}")
            return {'success': False, 'error': str(e)}
    
    def import_address(self, address, label=""):
        """Import an address as watch-only to the node wallet"""
        try:
            rpc_url = self.rest_url.replace('/api/v1', '') if '/api/v1' in self.rest_url else self.rest_url
            
            payload = {
                "jsonrpc": "2.0",
                "method": "importaddress",
                "params": [address, label],
                "id": 1
            }
            
            response = requests.post(rpc_url, json=payload, timeout=self.timeout)
            if response.status_code == 200:
                result = response.json()
                if 'result' in result:
                    return result['result']
                elif 'error' in result:
                    return {'success': False, 'error': result['error'].get('message', 'Unknown error')}
            return {'success': False, 'error': f'HTTP {response.status_code}'}
        except Exception as e:
            logger.error(f"Error importing address: {e}")
            return {'success': False, 'error': str(e)}
    
    def get_validators(self):
        """Get list of all validators"""
        try:
            rpc_url = self.rest_url.replace('/api/v1', '') if '/api/v1' in self.rest_url else self.rest_url
            
            payload = {
                "jsonrpc": "2.0",
                "method": "getvalidators",
                "params": [],
                "id": 1
            }
            
            response = requests.post(rpc_url, json=payload, timeout=self.timeout)
            if response.status_code == 200:
                result = response.json()
                if 'result' in result:
                    return result['result']
            return []
        except Exception as e:
            logger.error(f"Error fetching validators: {e}")
            return []
    
    def get_validator_info(self, address):
        """Get validator information for an address from blockchain"""
        try:
            rpc_url = self.rest_url.replace('/api/v1', '') if '/api/v1' in self.rest_url else self.rest_url
            
            payload = {
                "jsonrpc": "2.0",
                "method": "getvalidatorinfo",
                "params": [address],
                "id": 1
            }
            
            response = requests.post(rpc_url, json=payload, timeout=self.timeout)
            if response.status_code == 200:
                result = response.json()
                if 'result' in result:
                    return result['result']
            return None
        except Exception as e:
            logger.error(f"Error fetching validator info: {e}")
            return None
    
    def get_validator_rewards(self, address):
        """Get real validator rewards from blockchain"""
        try:
            validator_info = self.get_validator_info(address)
            if validator_info:
                return {
                    'total_rewards': validator_info.get('total_rewards', 0.0),
                    'pending_rewards': validator_info.get('pending_rewards', 0.0),
                    'apy': validator_info.get('apy', 0.0),
                    'blocks_produced': validator_info.get('blocks_produced', 0),
                    'uptime': validator_info.get('uptime', 0.0)
                }
            return None
        except Exception as e:
            logger.error(f"Error fetching validator rewards: {e}")
            return None
    
    def get_staking_stats(self):
        """Get real staking statistics from blockchain"""
        try:
            validators = self.get_validators()
            if not validators:
                return {
                    'total_validators': 0,
                    'total_staked': 0.0,
                    'average_apy': 0.0
                }
            
            total_staked = sum(v.get('stake_amount', 0.0) for v in validators if isinstance(v, dict))
            total_validators = len(validators)
            avg_apy = sum(v.get('apy', 0.0) for v in validators if isinstance(v, dict)) / total_validators if total_validators > 0 else 0.0
            
            return {
                'total_validators': total_validators,
                'total_staked': total_staked,
                'average_apy': avg_apy
            }
        except Exception as e:
            logger.error(f"Error fetching staking stats: {e}")
            return {
                'total_validators': 0,
                'total_staked': 0.0,
                'average_apy': 0.0
            }

class WalletService:
    def __init__(self, network=None, testnet=None):
        # Use current network or specified network - network-aware
        if testnet is not None:
            # Explicit testnet flag takes precedence
            self.testnet = testnet
            self.network = 'testnet' if testnet else 'mainnet'
        elif network:
            # Network string provided
            self.network = network
            self.testnet = (self.network == 'testnet')
        else:
            # Use current network from config
            self.network = get_network()
            self.testnet = (self.network == 'testnet')
        
        self.db_path = get_wallet_database_path()
        self.init_database()
        # Use current network configuration - blockchain client will detect server network
        self.blockchain = BlockchainClient(
            rest_url=NETWORK_INFO['rest_url'],
            rpc_url=get_rpc_url(),
            expected_network=self.network
        )
        
        # Log network info for debugging
        logger.info(f"WalletService initialized: network={self.network}, testnet={self.testnet}, address_prefix={CURRENT_NETWORK.get('address_prefix', 'GXC')}")
        
        # Auto-detect and adjust network from server
        if self.blockchain.network_detected:
            logger.info(f"Blockchain server network detected: {self.blockchain.detected_network}")
            if self.blockchain.detected_network != self.network:
                logger.warning(f"⚠️ NETWORK MISMATCH DETECTED: Wallet configured for {self.network}, but server is {self.blockchain.detected_network}")
                logger.warning(f"   Auto-adjusting wallet network to match server...")
                # Auto-update wallet network to match detected server network
                self.network = self.blockchain.detected_network
                self.testnet = (self.network == 'testnet')
                logger.info(f"✅ Wallet network auto-adjusted to: {self.network} (testnet={self.testnet})")
        else:
            # Force network detection on initialization
            detected = self.blockchain.detect_server_network()
            if detected and detected != self.network:
                logger.warning(f"⚠️ Auto-adjusting wallet from {self.network} to {detected}")
                self.network = detected
                self.testnet = (self.network == 'testnet')
                logger.info(f"✅ Wallet network auto-adjusted to: {self.network} (testnet={self.testnet})")
        
    def get_db_connection(self):
        """Get database connection (testnet or mainnet)"""
        return sqlite3.connect(self.db_path)
    
    def init_database(self):
        """Initialize the wallet database"""
        conn = self.get_db_connection()
        cursor = conn.cursor()
        
        # Users table
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS users (
                user_id TEXT PRIMARY KEY,
                username TEXT UNIQUE NOT NULL,
                email TEXT UNIQUE NOT NULL,
                password_hash TEXT NOT NULL,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                last_login TIMESTAMP,
                is_active BOOLEAN DEFAULT TRUE,
                two_factor_secret TEXT,
                backup_phrase_hash TEXT,
                recovery_questions TEXT
            )
        ''')
        
        # Wallets table
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS wallets (
                wallet_id TEXT PRIMARY KEY,
                user_id TEXT NOT NULL,
                wallet_name TEXT NOT NULL,
                address TEXT UNIQUE NOT NULL,
                public_key TEXT NOT NULL,
                encrypted_private_key TEXT NOT NULL,
                encrypted_mnemonic TEXT,
                wallet_type TEXT DEFAULT 'standard',
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                last_used TIMESTAMP,
                balance REAL DEFAULT 0.0,
                is_default BOOLEAN DEFAULT FALSE,
                backup_stored BOOLEAN DEFAULT FALSE,
                FOREIGN KEY (user_id) REFERENCES users (user_id)
            )
        ''')
        
        # Add mnemonic column if it doesn't exist (for existing databases)
        try:
            cursor.execute('ALTER TABLE wallets ADD COLUMN encrypted_mnemonic TEXT')
        except sqlite3.OperationalError:
            pass  # Column already exists
        
        # Wallet transactions
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS wallet_transactions (
                tx_id TEXT PRIMARY KEY,
                wallet_id TEXT NOT NULL,
                transaction_hash TEXT NOT NULL,
                transaction_type TEXT NOT NULL,
                amount REAL NOT NULL,
                fee REAL DEFAULT 0.0,
                to_address TEXT,
                from_address TEXT,
                status TEXT DEFAULT 'pending',
                timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                block_number INTEGER,
                FOREIGN KEY (wallet_id) REFERENCES wallets (wallet_id)
            )
        ''')
        
        # Validators table
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS validators (
                validator_id TEXT PRIMARY KEY,
                wallet_id TEXT NOT NULL,
                address TEXT NOT NULL UNIQUE,
                stake_amount REAL NOT NULL,
                staking_days INTEGER NOT NULL,
                registered_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                is_active BOOLEAN DEFAULT TRUE,
                blocks_produced INTEGER DEFAULT 0,
                missed_blocks INTEGER DEFAULT 0,
                total_rewards REAL DEFAULT 0.0,
                FOREIGN KEY (wallet_id) REFERENCES wallets (wallet_id)
            )
        ''')
        
        # Authentication tokens
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS auth_tokens (
                token_id TEXT PRIMARY KEY,
                user_id TEXT NOT NULL,
                token_hash TEXT NOT NULL,
                device_info TEXT,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                expires_at TIMESTAMP NOT NULL,
                is_active BOOLEAN DEFAULT TRUE,
                FOREIGN KEY (user_id) REFERENCES users (user_id)
            )
        ''')
        
        # Backup storage references
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS backup_storage (
                backup_id TEXT PRIMARY KEY,
                user_id TEXT NOT NULL,
                wallet_id TEXT NOT NULL,
                storage_type TEXT NOT NULL,
                storage_reference TEXT NOT NULL,
                encryption_key_hint TEXT,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (user_id) REFERENCES users (user_id),
                FOREIGN KEY (wallet_id) REFERENCES wallets (wallet_id)
            )
        ''')
        
        conn.commit()
        conn.close()
    
    def generate_wallet_keypair(self):
        """Generate a new ED25519 keypair for wallet"""
        private_key = ed25519.Ed25519PrivateKey.generate()
        public_key = private_key.public_key()
        
        # Serialize keys
        private_pem = private_key.private_bytes(
            encoding=serialization.Encoding.PEM,
            format=serialization.PrivateFormat.PKCS8,
            encryption_algorithm=serialization.NoEncryption()
        )
        
        public_pem = public_key.public_bytes(
            encoding=serialization.Encoding.PEM,
            format=serialization.PublicFormat.SubjectPublicKeyInfo
        )
        
        return private_pem.decode(), public_pem.decode()
    
    def generate_address(self, public_key_pem):
        """Generate GXC address from public key (matches C++ implementation)"""
        # Hash the public key with SHA-256
        public_key_bytes = public_key_pem.encode()
        sha256_hash = hashlib.sha256(public_key_bytes).digest()
        
        # Apply RIPEMD-160 (or use SHA-256 again for simplicity)
        try:
            ripemd160 = hashlib.new('ripemd160')
            ripemd160.update(sha256_hash)
            hash_result = ripemd160.hexdigest()
        except:
            # Fallback if RIPEMD-160 not available
            hash_result = hashlib.sha256(sha256_hash).hexdigest()
        
        # Take first 34 hex characters and add prefix
        # TESTNET: tGXC prefix (different from mainnet)
        # MAINNET: GXC prefix
        if self.testnet:
            address = 'tGXC' + hash_result[:33]  # tGXC + 33 chars = 37 total
        else:
            address = 'GXC' + hash_result[:34]   # GXC + 34 chars = 37 total
        
        return address
    
    def generate_mnemonic(self, strength=128):
        """
        Generate BIP39 mnemonic seed phrase (12 words for 128-bit, 24 words for 256-bit)
        strength: entropy in bits (128 for 12 words, 256 for 24 words)
        """
        # Import full BIP39 wordlist
        from bip39_wordlist import BIP39_WORDLIST
        
        # Generate entropy (16 bytes for 12 words, 32 bytes for 24 words)
        import hashlib
        entropy = secrets.token_bytes(strength // 8)
        
        # Calculate checksum
        entropy_hash = hashlib.sha256(entropy).digest()
        checksum_bits = strength // 32
        
        # Convert entropy to binary string
        entropy_bits = ''.join(format(b, '08b') for b in entropy)
        checksum = format(entropy_hash[0], '08b')[:checksum_bits]
        entropy_bits += checksum
        
        # Split into 11-bit chunks and convert to words
        mnemonic_words = []
        for i in range(0, len(entropy_bits), 11):
            index = int(entropy_bits[i:i+11], 2)
            mnemonic_words.append(BIP39_WORDLIST[index])
        
        return ' '.join(mnemonic_words)
    
    def encrypt_private_key(self, private_key, password):
        """Encrypt private key with user password"""
        # Simple encryption using bcrypt-derived key
        salt = bcrypt.gensalt()
        key = bcrypt.kdf(
            password.encode(),
            salt,
            desired_key_bytes=32,
            rounds=100
        )
        
        # XOR encryption (in production, use proper AES)
        encrypted = bytes(a ^ b for a, b in zip(private_key.encode(), key * (len(private_key) // 32 + 1)))
        
        return base64.b64encode(salt + encrypted).decode()
    
    def decrypt_private_key(self, encrypted_key, password):
        """Decrypt private key with user password"""
        try:
            data = base64.b64decode(encrypted_key.encode())
            salt = data[:29]  # bcrypt salt length
            encrypted = data[29:]
            
            key = bcrypt.kdf(
                password.encode(),
                salt,
                desired_key_bytes=32,
                rounds=100
            )
            
            decrypted = bytes(a ^ b for a, b in zip(encrypted, key * (len(encrypted) // 32 + 1)))
            return decrypted.decode()
        except:
            return None
    
    def create_user(self, username, email, password, backup_phrase=None):
        """Create a new user account"""
        conn = sqlite3.connect(DATABASE_PATH)
        cursor = conn.cursor()
        
        try:
            # Check if user exists
            cursor.execute('SELECT user_id FROM users WHERE username = ? OR email = ?', (username, email))
            if cursor.fetchone():
                return {'success': False, 'error': 'User already exists'}
            
            # Generate user ID
            user_id = str(uuid.uuid4())
            
            # Hash password
            password_hash = bcrypt.hashpw(password.encode(), bcrypt.gensalt()).decode()
            
            # Hash backup phrase if provided
            backup_phrase_hash = None
            if backup_phrase:
                backup_phrase_hash = hashlib.sha256(backup_phrase.encode()).hexdigest()
            
            # Insert user
            cursor.execute('''
                INSERT INTO users (user_id, username, email, password_hash, backup_phrase_hash)
                VALUES (?, ?, ?, ?, ?)
            ''', (user_id, username, email, password_hash, backup_phrase_hash))
            
            conn.commit()
            
            return {
                'success': True,
                'user_id': user_id,
                'message': 'User created successfully'
            }
            
        except Exception as e:
            conn.rollback()
            return {'success': False, 'error': str(e)}
        finally:
            conn.close()
    
    def get_user_info(self, user_id):
        """Get user information by user_id"""
        conn = sqlite3.connect(DATABASE_PATH)
        cursor = conn.cursor()
        
        try:
            cursor.execute('''
                SELECT user_id, username, email, created_at, last_login
                FROM users 
                WHERE user_id = ?
            ''', (user_id,))
            
            user = cursor.fetchone()
            if not user:
                return None
            
            return {
                'user_id': user[0],
                'username': user[1],
                'email': user[2],
                'created_at': user[3],
                'last_login': user[4]
            }
        except Exception as e:
            return None
        finally:
            conn.close()
    
    def authenticate_user(self, username, password):
        """Authenticate user and return JWT token"""
        conn = sqlite3.connect(DATABASE_PATH)
        cursor = conn.cursor()
        
        try:
            cursor.execute('''
                SELECT user_id, password_hash, is_active, username, email
                FROM users 
                WHERE username = ? OR email = ?
            ''', (username, username))
            
            user = cursor.fetchone()
            if not user:
                return {'success': False, 'error': 'User not found'}
            
            user_id, password_hash, is_active, db_username, email = user
            
            if not is_active:
                return {'success': False, 'error': 'Account is disabled'}
            
            # Verify password
            if not bcrypt.checkpw(password.encode(), password_hash.encode()):
                return {'success': False, 'error': 'Invalid credentials'}
            
            # Update last login
            cursor.execute('UPDATE users SET last_login = CURRENT_TIMESTAMP WHERE user_id = ?', (user_id,))
            conn.commit()
            
            # Generate JWT token
            payload = {
                'user_id': user_id,
                'username': db_username,
                'exp': datetime.now(timezone.utc) + timedelta(hours=24)
            }
            token = jwt.encode(payload, JWT_SECRET, algorithm='HS256')
            
            return {
                'success': True,
                'token': token,
                'user_id': user_id,
                'username': db_username,
                'email': email,
                'expires_in': 86400  # 24 hours
            }
            
        except Exception as e:
            return {'success': False, 'error': str(e)}
        finally:
            conn.close()
    
    def create_wallet(self, user_id, wallet_name, password, wallet_type='standard'):
        """Create a new wallet for user - returns all data needed for wallet access"""
        conn = sqlite3.connect(DATABASE_PATH)
        cursor = conn.cursor()
        
        try:
            # Get user info
            user_info = self.get_user_info(user_id)
            if not user_info:
                return {'success': False, 'error': 'User not found'}
            
            # Generate keypair
            private_key, public_key = self.generate_wallet_keypair()
            address = self.generate_address(public_key)
            
            # Generate mnemonic seed phrase (12 words)
            seed_phrase = self.generate_mnemonic(strength=128)
            
            # Encrypt private key and mnemonic
            encrypted_private_key = self.encrypt_private_key(private_key, password)
            encrypted_mnemonic = self.encrypt_private_key(seed_phrase, password)
            
            # Generate wallet ID
            wallet_id = str(uuid.uuid4())
            
            # Check if this is the first wallet (make it default)
            cursor.execute('SELECT COUNT(*) FROM wallets WHERE user_id = ?', (user_id,))
            is_default = cursor.fetchone()[0] == 0
            
            # Insert wallet
            cursor.execute('''
                INSERT INTO wallets (
                    wallet_id, user_id, wallet_name, address, public_key,
                    encrypted_private_key, encrypted_mnemonic, wallet_type, is_default
                ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
            ''', (wallet_id, user_id, wallet_name, address, public_key, 
                  encrypted_private_key, encrypted_mnemonic, wallet_type, is_default))
            
            conn.commit()
            
            # Generate JWT token for this wallet session
            payload = {
                'user_id': user_id,
                'username': user_info['username'],
                'wallet_id': wallet_id,
                'exp': datetime.now(timezone.utc) + timedelta(hours=24)
            }
            access_token = jwt.encode(payload, JWT_SECRET, algorithm='HS256')
            
            # Generate QR code for address
            qr_data = {
                'address': address,
                'network': 'GXC',
                'type': 'wallet'
            }
            
            # Return all data needed to access wallet and chains
            # IMPORTANT: Return private_key and seed_phrase in plain text ONCE for user to save
            return {
                'success': True,
                'wallet_id': wallet_id,
                'wallet_name': wallet_name,
                'address': address,
                'public_key': public_key,
                'private_key': private_key,  # Plain text - shown once only
                'seed_phrase': seed_phrase,  # Plain text - shown once only
                'qr_data': qr_data,
                'access_token': access_token,
                'user_info': {
                    'user_id': user_info['user_id'],
                    'username': user_info['username'],
                    'email': user_info['email']
                },
                'network_info': NETWORK_INFO,
                'recovery_info': {
                    'wallet_id': wallet_id,
                    'address': address,
                    'username': user_info['username'],
                    'email': user_info['email'],
                    'wallet_name': wallet_name,
                    'created_at': datetime.now(timezone.utc).isoformat(),
                    'private_key': private_key,  # Also in recovery info
                    'seed_phrase': seed_phrase  # Also in recovery info
                },
                'access_instructions': {
                    'login_url': f'{NETWORK_INFO["api_url"]}/login',
                    'api_endpoint': f'{NETWORK_INFO["api_url"]}/api/v1',
                    'explorer_url': f'{NETWORK_INFO["explorer_url"]}/address/{address}',
                    'rpc_url': NETWORK_INFO['rpc_url']
                },
                'message': 'Wallet created successfully - Save this information to access your wallet later!'
            }
            
        except Exception as e:
            conn.rollback()
            return {'success': False, 'error': str(e)}
        finally:
            conn.close()
    
    def get_wallet(self, wallet_id):
        """Get a single wallet by ID"""
        conn = sqlite3.connect(DATABASE_PATH)
        cursor = conn.cursor()
        
        try:
            cursor.execute('''
                SELECT wallet_id, user_id, wallet_name, address, public_key,
                       wallet_type, created_at, last_used, balance, is_default
                FROM wallets 
                WHERE wallet_id = ?
            ''', (wallet_id,))
            
            row = cursor.fetchone()
            if not row:
                return None
            
            wallet_id, user_id, wallet_name, address, public_key, wallet_type, created_at, last_used, balance, is_default = row
            
            return {
                'wallet_id': wallet_id,
                'user_id': user_id,
                'wallet_name': wallet_name,
                'address': address,
                'public_key': public_key,
                'wallet_type': wallet_type,
                'created_at': created_at,
                'last_used': last_used,
                'balance': balance,
                'is_default': bool(is_default)
            }
            
        except Exception as e:
            logger.error(f"Error getting wallet: {e}")
            return None
        finally:
            conn.close()
    
    def get_user_wallets(self, user_id):
        """Get all wallets for a user with real blockchain balances"""
        conn = sqlite3.connect(DATABASE_PATH)
        cursor = conn.cursor()
        
        try:
            cursor.execute('''
                SELECT wallet_id, wallet_name, address, wallet_type, 
                       created_at, last_used, balance, is_default
                FROM wallets 
                WHERE user_id = ?
                ORDER BY is_default DESC, created_at DESC
            ''', (user_id,))
            
            wallets = []
            for row in cursor.fetchall():
                wallet_id, wallet_name, address, wallet_type, created_at, last_used, cached_balance, is_default = row
                
                # Get real balance from blockchain (network verified)
                # Verify network detection first
                if not self.blockchain.network_detected:
                    self.blockchain.detect_server_network()
                real_balance = self.blockchain.get_address_balance(address)
                
                # Update cached balance in database
                if real_balance != cached_balance:
                    cursor.execute('UPDATE wallets SET balance = ? WHERE wallet_id = ?', (real_balance, wallet_id))
                    conn.commit()
                
                wallets.append({
                    'wallet_id': wallet_id,
                    'wallet_name': wallet_name,
                    'address': address,
                    'wallet_type': wallet_type,
                    'created_at': created_at,
                    'last_used': last_used,
                    'balance': real_balance,
                    'is_default': bool(is_default)
                })
            
            return {'success': True, 'wallets': wallets}
            
        except Exception as e:
            logger.error(f"Error getting user wallets: {e}")
            return {'success': False, 'error': str(e)}
        finally:
            conn.close()
    
    def get_wallet_balance(self, wallet_id, user_id):
        """Get real balance for a specific wallet"""
        conn = sqlite3.connect(DATABASE_PATH)
        cursor = conn.cursor()
        
        try:
            cursor.execute('SELECT address FROM wallets WHERE wallet_id = ? AND user_id = ?', (wallet_id, user_id))
            row = cursor.fetchone()
            if not row:
                return {'success': False, 'error': 'Wallet not found'}
            
            address = row[0]
            
            # Verify network detection before getting balance
            if not self.blockchain.network_detected:
                self.blockchain.detect_server_network()
            
            # Check network mismatch
            network_warning = None
            if self.blockchain.detected_network != self.network:
                network_warning = f"Network mismatch: Wallet configured for {self.network}, but server is {self.blockchain.detected_network}"
                logger.warning(f"⚠️ {network_warning} for address {address[:16]}...")
            
            balance = self.blockchain.get_address_balance(address)
            
            # Update cached balance
            cursor.execute('UPDATE wallets SET balance = ?, last_used = CURRENT_TIMESTAMP WHERE wallet_id = ?', 
                         (balance, wallet_id))
            conn.commit()
            
            result_data = {
                'success': True,
                'balance': balance,
                'address': address,
                'network': self.blockchain.detected_network or self.network,
                'expected_network': self.network
            }
            
            if network_warning:
                result_data['warning'] = network_warning
            
            return result_data
        except Exception as e:
            logger.error(f"Error getting wallet balance: {e}")
            return {'success': False, 'error': str(e)}
        finally:
            conn.close()
    
    def get_wallet_transactions(self, wallet_id, user_id, limit=50):
        """Get real transactions for a wallet from blockchain"""
        conn = sqlite3.connect(DATABASE_PATH)
        cursor = conn.cursor()
        
        try:
            cursor.execute('SELECT address FROM wallets WHERE wallet_id = ? AND user_id = ?', (wallet_id, user_id))
            row = cursor.fetchone()
            if not row:
                return {'success': False, 'error': 'Wallet not found'}
            
            address = row[0]
            transactions = self.blockchain.get_address_transactions(address, limit)
            
            # Update last_used
            cursor.execute('UPDATE wallets SET last_used = CURRENT_TIMESTAMP WHERE wallet_id = ?', (wallet_id,))
            conn.commit()
            
            return {'success': True, 'transactions': transactions, 'address': address}
        except Exception as e:
            logger.error(f"Error getting wallet transactions: {e}")
            return {'success': False, 'error': str(e)}
        finally:
            conn.close()
    
    def send_transaction(self, wallet_id, user_id, to_address, amount, password, fee=None):
        """Send transaction using real blockchain"""
        conn = sqlite3.connect(DATABASE_PATH)
        cursor = conn.cursor()
        
        try:
            # Get wallet info
            cursor.execute('''
                SELECT address, encrypted_private_key, balance 
                FROM wallets 
                WHERE wallet_id = ? AND user_id = ?
            ''', (wallet_id, user_id))
            
            row = cursor.fetchone()
            if not row:
                return {'success': False, 'error': 'Wallet not found'}
            
            from_address, encrypted_private_key, cached_balance = row
            
            # Get real balance (network verified)
            # Verify network detection first
            if not self.blockchain.network_detected:
                self.blockchain.detect_server_network()
            real_balance = self.blockchain.get_address_balance(from_address)
            if real_balance < amount + (fee or 0.001):
                return {'success': False, 'error': 'Insufficient balance'}
            
            # Decrypt private key
            private_key = self.decrypt_private_key(encrypted_private_key, password)
            if not private_key:
                return {'success': False, 'error': 'Invalid password'}
            
            # Get UTXOs for transaction
            utxos = self.blockchain.get_utxos(from_address)
            if not utxos:
                return {'success': False, 'error': 'No UTXOs available'}
            
            # Create transaction (simplified - in production, use proper transaction builder)
            tx_data = {
                'from_address': from_address,
                'to_address': to_address,
                'amount': amount,
                'fee': fee or 0.001,
                'utxos': utxos[:10]  # Use first 10 UTXOs
            }
            
            # Submit to blockchain
            result = self.blockchain.submit_transaction(tx_data)
            
            if result.get('success') or 'txid' in result:
                tx_hash = result.get('txid') or result.get('transaction_hash', 'pending')
                
                # Store transaction in database
                cursor.execute('''
                    INSERT INTO wallet_transactions 
                    (tx_id, wallet_id, transaction_hash, transaction_type, amount, fee, to_address, from_address, status)
                    VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
                ''', (str(uuid.uuid4()), wallet_id, tx_hash, 'send', amount, fee or 0.001, to_address, from_address, 'pending'))
                
                # Update balance
                new_balance = real_balance - amount - (fee or 0.001)
                cursor.execute('UPDATE wallets SET balance = ? WHERE wallet_id = ?', (new_balance, wallet_id))
                
                conn.commit()
                
                return {
                    'success': True,
                    'transaction_hash': tx_hash,
                    'message': 'Transaction submitted successfully'
                }
            else:
                return {'success': False, 'error': result.get('error', 'Transaction submission failed')}
                
        except Exception as e:
            logger.error(f"Error sending transaction: {e}")
            conn.rollback()
            return {'success': False, 'error': str(e)}
        finally:
            conn.close()
    
    def register_validator(self, wallet_id, user_id, stake_amount, staking_days, password):
        """Register wallet as a validator"""
        conn = sqlite3.connect(DATABASE_PATH)
        cursor = conn.cursor()
        
        try:
            # Get wallet info
            cursor.execute('''
                SELECT address, encrypted_private_key, public_key
                FROM wallets 
                WHERE wallet_id = ? AND user_id = ?
            ''', (wallet_id, user_id))
            
            row = cursor.fetchone()
            if not row:
                return {'success': False, 'error': 'Wallet not found'}
            
            address, encrypted_private_key, public_key = row
            
            # Validate minimum stake
            MIN_STAKE = 100.0
            MIN_STAKING_DAYS = 14
            MAX_STAKING_DAYS = 365
            
            if stake_amount < MIN_STAKE:
                return {'success': False, 'error': f'Minimum stake is {MIN_STAKE} GXC'}
            
            if staking_days < MIN_STAKING_DAYS or staking_days > MAX_STAKING_DAYS:
                return {'success': False, 'error': f'Staking period must be between {MIN_STAKING_DAYS} and {MAX_STAKING_DAYS} days'}
            
            # Check balance (network verified)
            # Verify network detection first
            if not self.blockchain.network_detected:
                self.blockchain.detect_server_network()
            balance = self.blockchain.get_address_balance(address)
            if balance < stake_amount:
                return {'success': False, 'error': 'Insufficient balance'}
            
            # Register validator via blockchain
            result = self.blockchain.register_validator(address, stake_amount, staking_days, public_key)
            
            if result.get('success') or 'validator_id' in result or result is True:
                # Store validator info in database
                validator_id = str(uuid.uuid4())
                cursor.execute('''
                    INSERT INTO validators (validator_id, wallet_id, address, stake_amount, staking_days, 
                                          registered_at, is_active)
                    VALUES (?, ?, ?, ?, ?, CURRENT_TIMESTAMP, TRUE)
                ''', (validator_id, wallet_id, address, stake_amount, staking_days))
                
                # Update wallet balance (stake is locked)
                new_balance = balance - stake_amount
                cursor.execute('UPDATE wallets SET balance = ? WHERE wallet_id = ?', (new_balance, wallet_id))
                
                conn.commit()
                
                return {
                    'success': True,
                    'validator_id': validator_id,
                    'address': address,
                    'stake_amount': stake_amount,
                    'staking_days': staking_days,
                    'message': 'Validator registered successfully'
                }
            else:
                return {'success': False, 'error': result.get('error', 'Validator registration failed')}
                
        except Exception as e:
            logger.error(f"Error registering validator: {e}")
            conn.rollback()
            return {'success': False, 'error': str(e)}
        finally:
            conn.close()
    
    def get_validator_status(self, wallet_id, user_id):
        """Get validator status for a wallet"""
        conn = sqlite3.connect(DATABASE_PATH)
        cursor = conn.cursor()
        
        try:
            cursor.execute('''
                SELECT v.validator_id, v.address, v.stake_amount, v.staking_days, 
                       v.registered_at, v.is_active, w.address
                FROM validators v
                JOIN wallets w ON v.wallet_id = w.wallet_id
                WHERE v.wallet_id = ? AND w.user_id = ?
            ''', (wallet_id, user_id))
            
            row = cursor.fetchone()
            if not row:
                return {'success': True, 'is_validator': False}
            
            validator_id, address, stake_amount, staking_days, registered_at, is_active, wallet_address = row
            
            # Get real validator info from blockchain
            validator_info = self.blockchain.get_validator_info(address)
            
            return {
                'success': True,
                'is_validator': True,
                'validator_id': validator_id,
                'address': address,
                'stake_amount': stake_amount,
                'staking_days': staking_days,
                'registered_at': registered_at,
                'is_active': bool(is_active),
                'blockchain_info': validator_info
            }
        except Exception as e:
            logger.error(f"Error getting validator status: {e}")
            return {'success': False, 'error': str(e)}
        finally:
            conn.close()
    
    def store_backup(self, user_id, wallet_id, storage_type, storage_reference, encryption_hint=None):
        """Store wallet backup reference"""
        conn = sqlite3.connect(DATABASE_PATH)
        cursor = conn.cursor()
        
        try:
            backup_id = str(uuid.uuid4())
            
            cursor.execute('''
                INSERT INTO backup_storage (
                    backup_id, user_id, wallet_id, storage_type,
                    storage_reference, encryption_key_hint
                ) VALUES (?, ?, ?, ?, ?, ?)
            ''', (backup_id, user_id, wallet_id, storage_type, storage_reference, encryption_hint))
            
            # Mark wallet as backed up
            cursor.execute('UPDATE wallets SET backup_stored = TRUE WHERE wallet_id = ?', (wallet_id,))
            
            conn.commit()
            
            return {
                'success': True,
                'backup_id': backup_id,
                'message': 'Backup stored successfully'
            }
            
        except Exception as e:
            conn.rollback()
            return {'success': False, 'error': str(e)}
        finally:
            conn.close()

# Initialize service
# Global wallet service instance - uses current network from config (testnet/mainnet)
# Network is determined by get_network() which reads GXC_NETWORK environment variable
wallet_service = WalletService()  # Will automatically use testnet or mainnet based on config

# Helper functions
def get_user_from_token():
    """Extract user from JWT token in request"""
    auth_header = request.headers.get('Authorization')
    if not auth_header or not auth_header.startswith('Bearer '):
        return None
    
    try:
        token = auth_header.split(' ')[1]
        payload = jwt.decode(token, JWT_SECRET, algorithms=['HS256'])
        return payload
    except:
        return None

def get_user_from_session():
    """Get user from Flask session"""
    if 'user_id' in session and 'token' in session:
        try:
            payload = jwt.decode(session['token'], JWT_SECRET, algorithms=['HS256'])
            return payload
        except:
            return None
    return None

# Web Page Routes
@app.route('/', methods=['GET'])
def index():
    """Home page"""
    return render_template('index.html', network_info=NETWORK_INFO)

@app.route('/register', methods=['GET', 'POST'])
def register_page():
    """Registration page"""
    if request.method == 'POST':
        data = request.form
        result = wallet_service.create_user(
            data.get('username'),
            data.get('email'),
            data.get('password'),
            data.get('backup_phrase')
        )
        
        if result['success']:
            flash('Account created successfully! Please login.', 'success')
            return redirect(url_for('login_page'))
        else:
            flash(result.get('error', 'Registration failed'), 'error')
    
    return render_template('register.html', network_info=NETWORK_INFO)

@app.route('/login', methods=['GET', 'POST'])
def login_page():
    """Login page"""
    if request.method == 'POST':
        data = request.form
        result = wallet_service.authenticate_user(
            data.get('username'),
            data.get('password')
        )
        
        if result['success']:
            session['user_id'] = result['user_id']
            session['username'] = result['username']
            session['token'] = result['token']
            session['email'] = result.get('email', '')
            flash('Login successful!', 'success')
            return redirect(url_for('dashboard'))
        else:
            flash(result.get('error', 'Login failed'), 'error')
    
    return render_template('login.html', network_info=NETWORK_INFO)

@app.route('/logout', methods=['GET'])
def logout():
    """Logout"""
    session.clear()
    flash('Logged out successfully', 'success')
    return redirect(url_for('index'))

@app.route('/create-wallet', methods=['GET', 'POST'])
def create_wallet_page():
    """Create wallet page"""
    user = get_user_from_session()
    if not user:
        flash('Please login first', 'error')
        return redirect(url_for('login_page'))
    
    if request.method == 'POST':
        data = request.form
        
        # Get network selection (mainnet or testnet)
        network = data.get('network', 'mainnet')
        testnet = (network == 'testnet')
        
        # Create appropriate wallet service for selected network
        service = WalletService(testnet=testnet)
        
        result = service.create_wallet(
            user['user_id'],
            data.get('wallet_name'),
            data.get('password'),
            data.get('wallet_type', 'standard')
        )
        
        if result['success']:
            # Add network info to result
            result['network'] = network
            result['network_name'] = 'Testnet' if testnet else 'Mainnet'
            
            # Store wallet data in session temporarily for success page
            session['new_wallet_data'] = result
            return redirect(url_for('wallet_success'))
        else:
            flash(result.get('error', 'Wallet creation failed'), 'error')
    
    return render_template('create_wallet.html', network_info=NETWORK_INFO, user=user)

@app.route('/wallet-success', methods=['GET'])
def wallet_success():
    """Wallet creation success page - shows all recovery data"""
    user = get_user_from_session()
    if not user:
        flash('Please login first', 'error')
        return redirect(url_for('login_page'))
    
    wallet_data = session.pop('new_wallet_data', None)
    if not wallet_data:
        flash('No wallet data found', 'error')
        return redirect(url_for('create_wallet_page'))
    
    return render_template('wallet_success.html', 
                         wallet_data=wallet_data, 
                         network_info=NETWORK_INFO,
                         user=user)

@app.route('/dashboard', methods=['GET'])
def dashboard():
    """Wallet dashboard"""
    user = get_user_from_session()
    if not user:
        flash('Please login first', 'error')
        return redirect(url_for('login_page'))
    
    wallets_result = wallet_service.get_user_wallets(user['user_id'])
    wallets = wallets_result.get('wallets', []) if wallets_result['success'] else []
    
    # Get blockchain info for dashboard
    blockchain_info = wallet_service.blockchain.get_blockchain_info() or {}
    
    return render_template('wallet_dashboard.html', 
                         wallets=wallets,
                         network_info=NETWORK_INFO,
                         blockchain_info=blockchain_info,
                         user=user)

@app.route('/wallet/<wallet_id>/send', methods=['GET', 'POST'])
def send_page(wallet_id):
    """Send transaction page"""
    user = get_user_from_session()
    if not user:
        flash('Please login first', 'error')
        return redirect(url_for('login_page'))
    
    if request.method == 'POST':
        data = request.form
        result = wallet_service.send_transaction(
            wallet_id,
            user['user_id'],
            data.get('to_address'),
            float(data.get('amount', 0)),
            data.get('password'),
            float(data.get('fee', 0.001)) if data.get('fee') else None
        )
        
        if result['success']:
            flash(f"Transaction submitted! Hash: {result['transaction_hash']}", 'success')
            return redirect(url_for('wallet_history', wallet_id=wallet_id))
        else:
            flash(result.get('error', 'Transaction failed'), 'error')
    
    # Get wallet info
    conn = sqlite3.connect(DATABASE_PATH)
    cursor = conn.cursor()
    cursor.execute('SELECT wallet_id, wallet_name, address FROM wallets WHERE wallet_id = ? AND user_id = ?', 
                  (wallet_id, user['user_id']))
    wallet = cursor.fetchone()
    conn.close()
    
    if not wallet:
        flash('Wallet not found', 'error')
        return redirect(url_for('dashboard'))
    
    # Get real balance
    balance_result = wallet_service.get_wallet_balance(wallet_id, user['user_id'])
    balance = balance_result.get('balance', 0.0) if balance_result['success'] else 0.0
    
    return render_template('send.html',
                         wallet={'wallet_id': wallet[0], 'wallet_name': wallet[1], 'address': wallet[2]},
                         balance=balance,
                         network_info=NETWORK_INFO,
                         user=user)

@app.route('/wallet/<wallet_id>/receive', methods=['GET'])
def receive_page(wallet_id):
    """Receive page - show QR code and address"""
    user = get_user_from_session()
    if not user:
        flash('Please login first', 'error')
        return redirect(url_for('login_page'))
    
    # Get wallet info
    conn = sqlite3.connect(DATABASE_PATH)
    cursor = conn.cursor()
    cursor.execute('SELECT wallet_id, wallet_name, address FROM wallets WHERE wallet_id = ? AND user_id = ?', 
                  (wallet_id, user['user_id']))
    wallet = cursor.fetchone()
    conn.close()
    
    if not wallet:
        flash('Wallet not found', 'error')
        return redirect(url_for('dashboard'))
    
    return render_template('receive.html',
                         wallet={'wallet_id': wallet[0], 'wallet_name': wallet[1], 'address': wallet[2]},
                         network_info=NETWORK_INFO,
                         user=user)

@app.route('/wallet/<wallet_id>/history', methods=['GET'])
def wallet_history(wallet_id):
    """Transaction history page"""
    user = get_user_from_session()
    if not user:
        flash('Please login first', 'error')
        return redirect(url_for('login_page'))
    
    limit = int(request.args.get('limit', 50))
    
    # Get real transactions from blockchain
    tx_result = wallet_service.get_wallet_transactions(wallet_id, user['user_id'], limit)
    transactions = tx_result.get('transactions', []) if tx_result['success'] else []
    
    # Get wallet info
    conn = sqlite3.connect(DATABASE_PATH)
    cursor = conn.cursor()
    cursor.execute('SELECT wallet_id, wallet_name, address FROM wallets WHERE wallet_id = ? AND user_id = ?', 
                  (wallet_id, user['user_id']))
    wallet = cursor.fetchone()
    conn.close()
    
    if not wallet:
        flash('Wallet not found', 'error')
        return redirect(url_for('dashboard'))
    
    return render_template('history.html',
                         wallet={'wallet_id': wallet[0], 'wallet_name': wallet[1], 'address': wallet[2]},
                         transactions=transactions,
                         network_info=NETWORK_INFO,
                         user=user)

@app.route('/wallet/<wallet_id>/settings', methods=['GET', 'POST'])
def wallet_settings(wallet_id):
    """Wallet settings page"""
    user = get_user_from_session()
    if not user:
        flash('Please login first', 'error')
        return redirect(url_for('login_page'))
    
    if request.method == 'POST':
        data = request.form
        conn = sqlite3.connect(DATABASE_PATH)
        cursor = conn.cursor()
        
        if 'wallet_name' in data:
            cursor.execute('UPDATE wallets SET wallet_name = ? WHERE wallet_id = ? AND user_id = ?',
                         (data['wallet_name'], wallet_id, user['user_id']))
            conn.commit()
            flash('Wallet name updated', 'success')
        
        conn.close()
        return redirect(url_for('wallet_settings', wallet_id=wallet_id))
    
    # Get wallet info
    conn = sqlite3.connect(DATABASE_PATH)
    cursor = conn.cursor()
    cursor.execute('SELECT wallet_id, wallet_name, address, wallet_type, created_at FROM wallets WHERE wallet_id = ? AND user_id = ?', 
                  (wallet_id, user['user_id']))
    wallet = cursor.fetchone()
    conn.close()
    
    if not wallet:
        flash('Wallet not found', 'error')
        return redirect(url_for('dashboard'))
    
    return render_template('settings.html',
                         wallet={'wallet_id': wallet[0], 'wallet_name': wallet[1], 'address': wallet[2], 
                                'wallet_type': wallet[3], 'created_at': wallet[4]},
                         network_info=NETWORK_INFO,
                         user=user)

@app.route('/wallet/<wallet_id>/staking', methods=['GET', 'POST'])
def staking_page(wallet_id):
    """Staking page"""
    user = get_user_from_session()
    if not user:
        flash('Please login first', 'error')
        return redirect(url_for('login_page'))
    
    # Get wallet info
    conn = sqlite3.connect(DATABASE_PATH)
    cursor = conn.cursor()
    cursor.execute('SELECT wallet_id, wallet_name, address FROM wallets WHERE wallet_id = ? AND user_id = ?', 
                  (wallet_id, user['user_id']))
    wallet = cursor.fetchone()
    conn.close()
    
    if not wallet:
        flash('Wallet not found', 'error')
        return redirect(url_for('dashboard'))
    
    # Get real balance
    balance_result = wallet_service.get_wallet_balance(wallet_id, user['user_id'])
    balance = balance_result.get('balance', 0.0) if balance_result['success'] else 0.0
    
    # Get validator status
    validator_status = wallet_service.get_validator_status(wallet_id, user['user_id'])
    is_validator = validator_status.get('is_validator', False) if validator_status['success'] else False
    
    return render_template('staking.html',
                         wallet={'wallet_id': wallet[0], 'wallet_name': wallet[1], 'address': wallet[2]},
                         balance=balance,
                         is_validator=is_validator,
                         validator_info=validator_status if is_validator else None,
                         network_info=NETWORK_INFO,
                         user=user)

@app.route('/wallet/<wallet_id>/become-validator', methods=['GET', 'POST'])
def become_validator_page(wallet_id):
    """Become a validator page"""
    user = get_user_from_session()
    if not user:
        flash('Please login first', 'error')
        return redirect(url_for('login_page'))
    
    if request.method == 'POST':
        data = request.form
        result = wallet_service.register_validator(
            wallet_id,
            user['user_id'],
            float(data.get('stake_amount', 0)),
            int(data.get('staking_days', 0)),
            data.get('password')
        )
        
        if result['success']:
            flash(f"Successfully registered as validator! Stake: {result['stake_amount']} GXC", 'success')
            return redirect(url_for('staking_page', wallet_id=wallet_id))
        else:
            flash(result.get('error', 'Validator registration failed'), 'error')
    
    # Get wallet info
    conn = sqlite3.connect(DATABASE_PATH)
    cursor = conn.cursor()
    cursor.execute('SELECT wallet_id, wallet_name, address FROM wallets WHERE wallet_id = ? AND user_id = ?', 
                  (wallet_id, user['user_id']))
    wallet = cursor.fetchone()
    conn.close()
    
    if not wallet:
        flash('Wallet not found', 'error')
        return redirect(url_for('dashboard'))
    
    # Get real balance
    balance_result = wallet_service.get_wallet_balance(wallet_id, user['user_id'])
    balance = balance_result.get('balance', 0.0) if balance_result['success'] else 0.0
    
    # Check if already a validator
    validator_status = wallet_service.get_validator_status(wallet_id, user['user_id'])
    if validator_status.get('is_validator', False):
        flash('This wallet is already registered as a validator', 'info')
        return redirect(url_for('staking_page', wallet_id=wallet_id))
    
    # Validator requirements
    MIN_STAKE = 100.0
    MIN_STAKING_DAYS = 14
    MAX_STAKING_DAYS = 365
    
    return render_template('become_validator.html',
                         wallet={'wallet_id': wallet[0], 'wallet_name': wallet[1], 'address': wallet[2]},
                         balance=balance,
                         min_stake=MIN_STAKE,
                         min_staking_days=MIN_STAKING_DAYS,
                         max_staking_days=MAX_STAKING_DAYS,
                         network_info=NETWORK_INFO,
                         user=user)

@app.route('/recovery', methods=['GET', 'POST'])
def recovery_page():
    """Wallet recovery page"""
    if request.method == 'POST':
        data = request.form
        username = data.get('username')
        password = data.get('password')
        
        result = wallet_service.authenticate_user(username, password)
        if result['success']:
            session['user_id'] = result['user_id']
            session['username'] = result['username']
            session['token'] = result['token']
            session['email'] = result.get('email', '')
            return redirect(url_for('dashboard'))
        else:
            flash(result.get('error', 'Recovery failed'), 'error')
    
    return render_template('recovery.html', network_info=NETWORK_INFO)

# API Routes
@app.route('/api/v1/register', methods=['POST'])
def register():
    """Register a new user"""
    data = request.get_json()
    
    required_fields = ['username', 'email', 'password']
    if not all(field in data for field in required_fields):
        return jsonify({'success': False, 'error': 'Missing required fields'}), 400
    
    result = wallet_service.create_user(
        data['username'],
        data['email'],
        data['password'],
        data.get('backup_phrase')
    )
    
    return jsonify(result), 201 if result['success'] else 400

@app.route('/api/v1/login', methods=['POST'])
def login():
    """Authenticate user"""
    data = request.get_json()
    
    if 'username' not in data or 'password' not in data:
        return jsonify({'success': False, 'error': 'Username and password required'}), 400
    
    result = wallet_service.authenticate_user(data['username'], data['password'])
    
    return jsonify(result), 200 if result['success'] else 401

@app.route('/api/v1/wallets/create', methods=['POST'])
def create_wallet():
    """Create a new wallet"""
    # Verify JWT token
    auth_header = request.headers.get('Authorization')
    if not auth_header or not auth_header.startswith('Bearer '):
        return jsonify({'success': False, 'error': 'Missing or invalid token'}), 401
    
    try:
        token = auth_header.split(' ')[1]
        payload = jwt.decode(token, JWT_SECRET, algorithms=['HS256'])
        user_id = payload['user_id']
    except jwt.ExpiredSignatureError:
        return jsonify({'success': False, 'error': 'Token expired'}), 401
    except jwt.InvalidTokenError:
        return jsonify({'success': False, 'error': 'Invalid token'}), 401
    
    data = request.get_json()
    
    if 'wallet_name' not in data or 'password' not in data:
        return jsonify({'success': False, 'error': 'Wallet name and password required'}), 400
    
    # Get network selection (mainnet or testnet)
    network = data.get('network', 'mainnet')
    testnet = (network == 'testnet')
    
    # Create appropriate wallet service for selected network
    service = WalletService(testnet=testnet)
    
    result = service.create_wallet(
        user_id,
        data['wallet_name'],
        data['password'],
        data.get('wallet_type', 'standard')
    )
    
    if result['success']:
        result['network'] = network
        result['network_name'] = 'Testnet' if testnet else 'Mainnet'
        result['address_prefix'] = NETWORK_INFO.get('address_prefix', 'GXC')
        result['is_testnet'] = testnet
        logger.info(f"Wallet created via API: network={network}, testnet={testnet}, address_prefix={result['address_prefix']}")
        return jsonify(result), 201
    else:
        return jsonify(result), 400

@app.route('/api/v1/wallets', methods=['GET'])
def get_wallets():
    """Get user's wallets"""
    # Verify JWT token
    auth_header = request.headers.get('Authorization')
    if not auth_header or not auth_header.startswith('Bearer '):
        return jsonify({'success': False, 'error': 'Missing or invalid token'}), 401
    
    try:
        token = auth_header.split(' ')[1]
        payload = jwt.decode(token, JWT_SECRET, algorithms=['HS256'])
        user_id = payload['user_id']
    except jwt.ExpiredSignatureError:
        return jsonify({'success': False, 'error': 'Token expired'}), 401
    except jwt.InvalidTokenError:
        return jsonify({'success': False, 'error': 'Invalid token'}), 401
    
    result = wallet_service.get_user_wallets(user_id)
    
    if result['success']:
        return jsonify({
            'success': True,
            'wallets': result['wallets'],
            'network': NETWORK_INFO.get('network', 'unknown'),
            'network_name': NETWORK_INFO.get('network_name', 'Unknown'),
            'address_prefix': NETWORK_INFO.get('address_prefix', 'GXC'),
            'is_testnet': NETWORK_INFO.get('network', '').lower() == 'testnet',
            'detected_network': wallet_service.blockchain.detected_network if wallet_service.blockchain.network_detected else None,
            'network_match': wallet_service.blockchain.detected_network == wallet_service.network if wallet_service.blockchain.network_detected else None,
            'network_warning': f"Network mismatch: Wallet configured for {wallet_service.network}, but server is {wallet_service.blockchain.detected_network}" if (wallet_service.blockchain.network_detected and wallet_service.blockchain.detected_network != wallet_service.network) else None
        })
    else:
        return jsonify(result), 400

@app.route('/api/v1/wallets/<wallet_id>/balance', methods=['GET'])
def get_wallet_balance_api(wallet_id):
    """Get wallet balance (API)"""
    auth_header = request.headers.get('Authorization')
    if not auth_header or not auth_header.startswith('Bearer '):
        return jsonify({'success': False, 'error': 'Missing or invalid token'}), 401
    
    try:
        token = auth_header.split(' ')[1]
        payload = jwt.decode(token, JWT_SECRET, algorithms=['HS256'])
        user_id = payload['user_id']
    except:
        return jsonify({'success': False, 'error': 'Invalid token'}), 401
    
    result = wallet_service.get_wallet_balance(wallet_id, user_id)
    
    if result['success']:
        response_data = {
            'success': True,
            'balance': result['balance'],
            'address': result['address'],
            'network': NETWORK_INFO.get('network', 'unknown'),
            'network_name': NETWORK_INFO.get('network_name', 'Unknown'),
            'address_prefix': NETWORK_INFO.get('address_prefix', 'GXC'),
            'is_testnet': NETWORK_INFO.get('network', '').lower() == 'testnet',
            'detected_network': wallet_service.blockchain.detected_network if wallet_service.blockchain.network_detected else None,
            'network_match': wallet_service.blockchain.detected_network == wallet_service.network if wallet_service.blockchain.network_detected else None
        }
        
        # Add network warning if mismatch
        if 'warning' in result:
            response_data['network_warning'] = result['warning']
        elif wallet_service.blockchain.network_detected and wallet_service.blockchain.detected_network != wallet_service.network:
            response_data['network_warning'] = f"Network mismatch: Wallet configured for {wallet_service.network}, but server is {wallet_service.blockchain.detected_network}"
        
        return jsonify(response_data), 200
    else:
        return jsonify(result), 400

@app.route('/api/v1/wallets/<wallet_id>/transactions', methods=['GET'])
def get_wallet_transactions_api(wallet_id):
    """Get wallet transactions (API)"""
    auth_header = request.headers.get('Authorization')
    if not auth_header or not auth_header.startswith('Bearer '):
        return jsonify({'success': False, 'error': 'Missing or invalid token'}), 401
    
    try:
        token = auth_header.split(' ')[1]
        payload = jwt.decode(token, JWT_SECRET, algorithms=['HS256'])
        user_id = payload['user_id']
    except:
        return jsonify({'success': False, 'error': 'Invalid token'}), 401
    
    limit = int(request.args.get('limit', 50))
    result = wallet_service.get_wallet_transactions(wallet_id, user_id, limit)
    return jsonify(result), 200 if result['success'] else 400

@app.route('/api/v1/wallets/<wallet_id>/become-validator', methods=['POST'])
def become_validator_api(wallet_id):
    """Become a validator (API)"""
    auth_header = request.headers.get('Authorization')
    if not auth_header or not auth_header.startswith('Bearer '):
        return jsonify({'success': False, 'error': 'Missing or invalid token'}), 401
    
    try:
        token = auth_header.split(' ')[1]
        payload = jwt.decode(token, JWT_SECRET, algorithms=['HS256'])
        user_id = payload['user_id']
    except:
        return jsonify({'success': False, 'error': 'Invalid token'}), 401
    
    data = request.get_json()
    required_fields = ['stake_amount', 'staking_days', 'password']
    if not all(field in data for field in required_fields):
        return jsonify({'success': False, 'error': 'Missing required fields'}), 400
    
    result = wallet_service.register_validator(
        wallet_id,
        user_id,
        float(data['stake_amount']),
        int(data['staking_days']),
        data['password']
    )
    
    return jsonify(result), 200 if result['success'] else 400

@app.route('/api/v1/wallets/<wallet_id>/validator-status', methods=['GET'])
def get_validator_status_api(wallet_id):
    """Get validator status (API)"""
    auth_header = request.headers.get('Authorization')
    if not auth_header or not auth_header.startswith('Bearer '):
        return jsonify({'success': False, 'error': 'Missing or invalid token'}), 401
    
    try:
        token = auth_header.split(' ')[1]
        payload = jwt.decode(token, JWT_SECRET, algorithms=['HS256'])
        user_id = payload['user_id']
    except:
        return jsonify({'success': False, 'error': 'Invalid token'}), 401
    
    result = wallet_service.get_validator_status(wallet_id, user_id)
    return jsonify(result), 200 if result['success'] else 400

@app.route('/api/v1/wallets/<wallet_id>/send', methods=['POST'])
def send_transaction_api(wallet_id):
    """Send transaction (API)"""
    auth_header = request.headers.get('Authorization')
    if not auth_header or not auth_header.startswith('Bearer '):
        return jsonify({'success': False, 'error': 'Missing or invalid token'}), 401
    
    try:
        token = auth_header.split(' ')[1]
        payload = jwt.decode(token, JWT_SECRET, algorithms=['HS256'])
        user_id = payload['user_id']
    except:
        return jsonify({'success': False, 'error': 'Invalid token'}), 401
    
    data = request.get_json()
    required_fields = ['to_address', 'amount', 'password']
    if not all(field in data for field in required_fields):
        return jsonify({'success': False, 'error': 'Missing required fields'}), 400
    
    result = wallet_service.send_transaction(
        wallet_id,
        user_id,
        data['to_address'],
        float(data['amount']),
        data['password'],
        float(data['fee']) if data.get('fee') else None
    )
    
    return jsonify(result), 200 if result['success'] else 400

@app.route('/api/v1/wallets/<wallet_id>/backup', methods=['POST'])
def store_wallet_backup(wallet_id):
    """Store wallet backup"""
    # Verify JWT token
    auth_header = request.headers.get('Authorization')
    if not auth_header or not auth_header.startswith('Bearer '):
        return jsonify({'success': False, 'error': 'Missing or invalid token'}), 401
    
    try:
        token = auth_header.split(' ')[1]
        payload = jwt.decode(token, JWT_SECRET, algorithms=['HS256'])
        user_id = payload['user_id']
    except jwt.ExpiredSignatureError:
        return jsonify({'success': False, 'error': 'Token expired'}), 401
    except jwt.InvalidTokenError:
        return jsonify({'success': False, 'error': 'Invalid token'}), 401
    
    data = request.get_json()
    
    required_fields = ['storage_type', 'storage_reference']
    if not all(field in data for field in required_fields):
        return jsonify({'success': False, 'error': 'Missing required fields'}), 400
    
    result = wallet_service.store_backup(
        user_id,
        wallet_id,
        data['storage_type'],
        data['storage_reference'],
        data.get('encryption_hint')
    )
    
    return jsonify(result), 201 if result['success'] else 400

@app.route('/api/v1/health', methods=['GET'])
def health_check():
    """Health check endpoint"""
    return jsonify({
        'status': 'healthy',
        'service': 'GXC Wallet API',
        'version': '1.0.0',
        'timestamp': datetime.now(timezone.utc).isoformat()
    })

# ============= THIRD-PARTY WALLET SUPPORT API ENDPOINTS =============

@app.route('/api/v1/external-validator/register', methods=['POST'])
def register_external_validator_api():
    """
    Register as a validator using a third-party wallet address.
    
    This endpoint is designed for developers using external wallets (not the built-in wallet)
    to register as validators on the GXC blockchain.
    
    Required fields:
    - address: The wallet address to register (GXC or tGXC prefix)
    - stake_amount: Amount of GXC to stake (minimum 100 GXC)
    - staking_days: Number of days to stake (14-365)
    
    Optional fields:
    - signature: Cryptographic signature proving ownership
    - message: The message that was signed
    - public_key: The public key corresponding to the address
    
    If signature is not provided, the system will verify the address has sufficient balance.
    """
    data = request.get_json()
    
    required_fields = ['address', 'stake_amount', 'staking_days']
    if not all(field in data for field in required_fields):
        return jsonify({
            'success': False, 
            'error': 'Missing required fields. Required: address, stake_amount, staking_days',
            'required': required_fields,
            'optional': ['signature', 'message', 'public_key']
        }), 400
    
    address = data['address']
    stake_amount = float(data['stake_amount'])
    staking_days = int(data['staking_days'])
    signature = data.get('signature', '')
    message = data.get('message', '')
    public_key = data.get('public_key', '')
    
    # Validate address format
    if not (address.startswith('GXC') or address.startswith('tGXC')):
        return jsonify({
            'success': False,
            'error': f'Invalid address format: {address}. Must start with GXC (mainnet) or tGXC (testnet)'
        }), 400
    
    # Validate stake amount
    MIN_STAKE = 100.0
    if stake_amount < MIN_STAKE:
        return jsonify({
            'success': False,
            'error': f'Stake amount must be at least {MIN_STAKE} GXC'
        }), 400
    
    # Validate staking days
    MIN_DAYS = 14
    MAX_DAYS = 365
    if staking_days < MIN_DAYS or staking_days > MAX_DAYS:
        return jsonify({
            'success': False,
            'error': f'Staking days must be between {MIN_DAYS} and {MAX_DAYS}'
        }), 400
    
    result = wallet_service.blockchain.register_external_validator(
        address, stake_amount, staking_days, public_key, signature, message
    )
    
    if result.get('success'):
        return jsonify(result), 200
    else:
        return jsonify(result), 400

@app.route('/api/v1/wallet/import-key', methods=['POST'])
def import_private_key_api():
    """
    Import a private key to enable full wallet control.
    
    This allows developers to import their third-party wallet's private key
    so the node can sign transactions on their behalf.
    
    Required fields:
    - private_key: 64-character hex string (32 bytes)
    
    Optional fields:
    - label: A label to identify this imported key
    
    Security Warning: Only use this in secure environments. 
    The private key will be stored by the node.
    """
    data = request.get_json()
    
    if 'private_key' not in data:
        return jsonify({
            'success': False,
            'error': 'Missing required field: private_key (64-character hex string)'
        }), 400
    
    private_key = data['private_key']
    label = data.get('label', 'imported')
    
    # Basic validation
    if len(private_key) != 64:
        return jsonify({
            'success': False,
            'error': 'Private key must be 64 characters (32 bytes in hex)'
        }), 400
    
    result = wallet_service.blockchain.import_private_key(private_key, label)
    
    if result.get('success'):
        return jsonify(result), 200
    else:
        return jsonify(result), 400

@app.route('/api/v1/wallet/import-address', methods=['POST'])
def import_address_api():
    """
    Import an address for watch-only tracking.
    
    This allows developers to track balances and transactions for an address
    without importing the private key. Cannot sign transactions.
    
    Required fields:
    - address: The wallet address to import
    
    Optional fields:
    - label: A label to identify this address
    """
    data = request.get_json()
    
    if 'address' not in data:
        return jsonify({
            'success': False,
            'error': 'Missing required field: address'
        }), 400
    
    address = data['address']
    label = data.get('label', 'watch-only')
    
    # Validate address format
    if not (address.startswith('GXC') or address.startswith('tGXC')):
        return jsonify({
            'success': False,
            'error': f'Invalid address format: {address}. Must start with GXC (mainnet) or tGXC (testnet)'
        }), 400
    
    result = wallet_service.blockchain.import_address(address, label)
    
    if result.get('success'):
        return jsonify(result), 200
    else:
        return jsonify(result), 400

@app.route('/api/v1/address/<address>/fund', methods=['POST'])
def fund_address_api(address):
    """
    Request testnet funds for development/testing.
    
    This is a faucet endpoint for testnet only.
    Provides a small amount of tGXC for testing purposes.
    """
    # Only allow on testnet
    if wallet_service.network != 'testnet':
        return jsonify({
            'success': False,
            'error': 'Faucet only available on testnet'
        }), 400
    
    # Validate address format
    if not address.startswith('tGXC'):
        return jsonify({
            'success': False,
            'error': f'Invalid testnet address format: {address}. Must start with tGXC'
        }), 400
    
    data = request.get_json() or {}
    amount = min(float(data.get('amount', 100.0)), 1000.0)  # Max 1000 tGXC per request
    
    try:
        # Try to fund from the node's wallet
        rpc_url = wallet_service.blockchain.rpc_url
        
        payload = {
            "jsonrpc": "2.0",
            "method": "sendtoaddress",
            "params": [address, amount],
            "id": 1
        }
        
        response = requests.post(rpc_url, json=payload, timeout=30)
        if response.status_code == 200:
            result = response.json()
            if 'result' in result:
                return jsonify({
                    'success': True,
                    'address': address,
                    'amount': amount,
                    'txid': result['result'],
                    'message': f'Successfully sent {amount} tGXC to {address}'
                }), 200
            elif 'error' in result:
                return jsonify({
                    'success': False,
                    'error': result['error'].get('message', 'Unknown error'),
                    'hint': 'The faucet may have insufficient funds. Contact the node operator.'
                }), 400
        
        return jsonify({
            'success': False,
            'error': f'HTTP {response.status_code}',
            'hint': 'Failed to communicate with blockchain node'
        }), 500
        
    except Exception as e:
        logger.error(f"Error funding address: {e}")
        return jsonify({
            'success': False,
            'error': str(e)
        }), 500

# ============= END THIRD-PARTY WALLET SUPPORT =============

@app.route('/api/v1/docs', methods=['GET'])
@app.route('/docs', methods=['GET'])
def api_docs():
    """API Documentation page - returns JSON"""
    docs = {
        'title': 'GXC Wallet API Documentation',
        'version': '1.0.0',
        'base_url': 'http://localhost:5000/api/v1',
        'endpoints': [
            {
                'method': 'POST',
                'path': '/register',
                'description': 'Register a new user account',
                'body': {
                    'username': 'string (required)',
                    'email': 'string (required)',
                    'password': 'string (required)',
                    'backup_phrase': 'string (optional)'
                },
                'response': {
                    'success': 'boolean',
                    'user_id': 'string',
                    'message': 'string'
                }
            },
            {
                'method': 'POST',
                'path': '/login',
                'description': 'Authenticate user and get JWT token',
                'body': {
                    'username': 'string (required)',
                    'password': 'string (required)'
                },
                'response': {
                    'success': 'boolean',
                    'token': 'string (JWT)',
                    'user_id': 'string'
                }
            },
            {
                'method': 'POST',
                'path': '/wallets/create',
                'description': 'Create a new wallet (requires authentication)',
                'headers': {
                    'Authorization': 'Bearer <JWT_TOKEN>'
                },
                'body': {
                    'wallet_name': 'string (required)',
                    'password': 'string (required)',
                    'wallet_type': 'string (optional, default: standard)'
                },
                'response': {
                    'success': 'boolean',
                    'wallet_id': 'string',
                    'address': 'string',
                    'public_key': 'string',
                    'qr_code': 'string (base64)'
                }
            },
            {
                'method': 'GET',
                'path': '/wallets',
                'description': 'Get all wallets for authenticated user',
                'headers': {
                    'Authorization': 'Bearer <JWT_TOKEN>'
                },
                'response': {
                    'success': 'boolean',
                    'wallets': 'array of wallet objects'
                }
            },
            {
                'method': 'POST',
                'path': '/wallets/<wallet_id>/backup',
                'description': 'Store wallet backup reference',
                'headers': {
                    'Authorization': 'Bearer <JWT_TOKEN>'
                },
                'body': {
                    'storage_type': 'string (required)',
                    'storage_reference': 'string (required)',
                    'encryption_hint': 'string (optional)'
                },
                'response': {
                    'success': 'boolean',
                    'backup_id': 'string'
                }
            },
            {
                'method': 'GET',
                'path': '/health',
                'description': 'Health check endpoint',
                'response': {
                    'status': 'healthy',
                    'service': 'GXC Wallet API',
                    'version': 'string'
                }
            }
        ]
    }
    
    return jsonify(docs)

@app.route('/api/v1/network/status', methods=['GET'])
def network_status():
    """Get network status and detection info"""
    # Ensure network is detected
    if not wallet_service.blockchain.network_detected:
        wallet_service.blockchain.detect_server_network()
    
    return jsonify({
        'success': True,
        'configured_network': wallet_service.network,
        'detected_network': wallet_service.blockchain.detected_network,
        'network_detected': wallet_service.blockchain.network_detected,
        'network_match': wallet_service.blockchain.detected_network == wallet_service.network if wallet_service.blockchain.network_detected else None,
        'network_info': NETWORK_INFO,
        'warning': f"Network mismatch: Wallet configured for {wallet_service.network}, but server is {wallet_service.blockchain.detected_network}" if (wallet_service.blockchain.network_detected and wallet_service.blockchain.detected_network != wallet_service.network) else None
    }), 200

@app.route('/api', methods=['GET'])
def api_index():
    """API homepage - returns JSON"""
    return jsonify({
        'service': 'GXC Wallet API',
        'version': '1.0.0',
        'status': 'running',
        'endpoints': {
            'register': '/api/v1/register',
            'network_status': '/api/v1/network/status',
            'login': '/api/v1/login',
            'create_wallet': '/api/v1/wallets/create',
            'get_wallets': '/api/v1/wallets',
            'docs': '/api/v1/docs',
            'health': '/api/v1/health'
        }
    })

if __name__ == '__main__':
    # Ensure backup directory exists
    os.makedirs(BACKUP_STORAGE_PATH, exist_ok=True)
    
    # Run the application
    app.run(host='0.0.0.0', port=5000, debug=False)