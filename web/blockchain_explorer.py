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

app = Flask(__name__)
app.config['SECRET_KEY'] = 'gxc_explorer_secret_key'
socketio = SocketIO(app, cors_allowed_origins="*")

# Configuration
BLOCKCHAIN_NODE_URL = 'http://localhost:8545'  # GXC node RPC endpoint
DATABASE_PATH = 'gxc_explorer.db'

class BlockchainExplorer:
    def __init__(self):
        self.init_database()
        self.start_block_monitor()
    
    def init_database(self):
        """Initialize explorer database"""
        conn = sqlite3.connect(DATABASE_PATH)
        cursor = conn.cursor()
        
        # Blocks table
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS blocks (
                block_number INTEGER PRIMARY KEY,
                block_hash TEXT UNIQUE NOT NULL,
                parent_hash TEXT NOT NULL,
                timestamp TIMESTAMP NOT NULL,
                miner_address TEXT NOT NULL,
                difficulty REAL NOT NULL,
                total_difficulty REAL NOT NULL,
                size INTEGER NOT NULL,
                gas_used INTEGER DEFAULT 0,
                gas_limit INTEGER DEFAULT 0,
                transaction_count INTEGER DEFAULT 0,
                reward REAL DEFAULT 0.0,
                consensus_type TEXT DEFAULT 'pow',
                validator_signature TEXT
            )
        ''')
        
        # Transactions table
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS transactions (
                tx_hash TEXT PRIMARY KEY,
                block_number INTEGER NOT NULL,
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
                FOREIGN KEY (block_number) REFERENCES blocks (block_number)
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
        
        conn.commit()
        conn.close()
    
    def get_latest_block(self):
        """Get latest block from the node"""
        try:
            response = requests.post(BLOCKCHAIN_NODE_URL, json={
                'method': 'gxc_getLatestBlock',
                'params': [],
                'id': 1
            }, timeout=10)
            
            if response.status_code == 200:
                return response.json().get('result')
            return None
        except:
            return None
    
    def get_block_by_number(self, block_number):
        """Get block by number from the node"""
        try:
            response = requests.post(BLOCKCHAIN_NODE_URL, json={
                'method': 'gxc_getBlockByNumber',
                'params': [block_number, True],  # True to include transactions
                'id': 1
            }, timeout=10)
            
            if response.status_code == 200:
                return response.json().get('result')
            return None
        except:
            return None
    
    def store_block(self, block_data):
        """Store block data in database"""
        conn = sqlite3.connect(DATABASE_PATH)
        cursor = conn.cursor()
        
        try:
            cursor.execute('''
                INSERT OR REPLACE INTO blocks (
                    block_number, block_hash, parent_hash, timestamp,
                    miner_address, difficulty, total_difficulty, size,
                    gas_used, gas_limit, transaction_count, reward,
                    consensus_type, validator_signature
                ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
            ''', (
                block_data['number'],
                block_data['hash'],
                block_data['parentHash'],
                datetime.fromisoformat(block_data['timestamp']),
                block_data['miner'],
                float(block_data['difficulty']),
                float(block_data['totalDifficulty']),
                int(block_data['size']),
                int(block_data.get('gasUsed', 0)),
                int(block_data.get('gasLimit', 0)),
                len(block_data.get('transactions', [])),
                float(block_data.get('reward', 0.0)),
                block_data.get('consensusType', 'pow'),
                block_data.get('validatorSignature')
            ))
            
            # Store transactions
            if 'transactions' in block_data:
                for i, tx in enumerate(block_data['transactions']):
                    self.store_transaction(tx, block_data['number'], i)
            
            conn.commit()
            
        except Exception as e:
            conn.rollback()
            print(f"Error storing block: {e}")
        finally:
            conn.close()
    
    def store_transaction(self, tx_data, block_number, tx_index):
        """Store transaction data"""
        conn = sqlite3.connect(DATABASE_PATH)
        cursor = conn.cursor()
        
        try:
            cursor.execute('''
                INSERT OR REPLACE INTO transactions (
                    tx_hash, block_number, tx_index, from_address,
                    to_address, value, fee, gas_price, gas_used,
                    status, timestamp, input_data, nonce, tx_type,
                    contract_address
                ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
            ''', (
                tx_data['hash'],
                block_number,
                tx_index,
                tx_data['from'],
                tx_data.get('to'),
                float(tx_data['value']),
                float(tx_data.get('fee', 0.0)),
                float(tx_data.get('gasPrice', 0.0)),
                int(tx_data.get('gasUsed', 0)),
                tx_data.get('status', 'success'),
                datetime.utcnow(),
                tx_data.get('input'),
                int(tx_data.get('nonce', 0)),
                tx_data.get('type', 'transfer'),
                tx_data.get('contractAddress')
            ))
            
            # Update address balances and activity
            self.update_address_activity(tx_data['from'], tx_data.get('to'))
            
            conn.commit()
            
        except Exception as e:
            conn.rollback()
            print(f"Error storing transaction: {e}")
        finally:
            conn.close()
    
    def update_address_activity(self, from_addr, to_addr):
        """Update address activity records"""
        conn = sqlite3.connect(DATABASE_PATH)
        cursor = conn.cursor()
        
        try:
            for addr in [from_addr, to_addr]:
                if addr:
                    cursor.execute('''
                        INSERT OR IGNORE INTO addresses (
                            address, first_seen, last_activity
                        ) VALUES (?, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP)
                    ''', (addr,))
                    
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
    
    def get_recent_blocks(self, limit=10):
        """Get recent blocks from database"""
        conn = sqlite3.connect(DATABASE_PATH)
        cursor = conn.cursor()
        
        try:
            cursor.execute('''
                SELECT block_number, block_hash, timestamp, miner_address,
                       transaction_count, reward, consensus_type
                FROM blocks
                ORDER BY block_number DESC
                LIMIT ?
            ''', (limit,))
            
            blocks = []
            for row in cursor.fetchall():
                blocks.append({
                    'number': row[0],
                    'hash': row[1],
                    'timestamp': row[2],
                    'miner': row[3],
                    'tx_count': row[4],
                    'reward': row[5],
                    'consensus_type': row[6]
                })
            
            return blocks
            
        except Exception as e:
            print(f"Error getting recent blocks: {e}")
            return []
        finally:
            conn.close()
    
    def get_recent_transactions(self, limit=20):
        """Get recent transactions from database"""
        conn = sqlite3.connect(DATABASE_PATH)
        cursor = conn.cursor()
        
        try:
            cursor.execute('''
                SELECT tx_hash, block_number, from_address, to_address,
                       value, fee, status, timestamp, tx_type
                FROM transactions
                ORDER BY block_number DESC, tx_index DESC
                LIMIT ?
            ''', (limit,))
            
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
            print(f"Error getting recent transactions: {e}")
            return []
        finally:
            conn.close()
    
    def get_network_stats(self):
        """Get current network statistics"""
        conn = sqlite3.connect(DATABASE_PATH)
        cursor = conn.cursor()
        
        try:
            # Get latest stats
            cursor.execute('''
                SELECT total_blocks, total_transactions, total_addresses,
                       total_supply, hash_rate, difficulty, avg_block_time
                FROM network_stats
                ORDER BY timestamp DESC
                LIMIT 1
            ''')
            
            stats = cursor.fetchone()
            if stats:
                return {
                    'total_blocks': stats[0],
                    'total_transactions': stats[1],
                    'total_addresses': stats[2],
                    'total_supply': stats[3],
                    'hash_rate': stats[4],
                    'difficulty': stats[5],
                    'avg_block_time': stats[6]
                }
            
            # Calculate stats if none exist
            cursor.execute('SELECT COUNT(*) FROM blocks')
            total_blocks = cursor.fetchone()[0]
            
            cursor.execute('SELECT COUNT(*) FROM transactions')
            total_transactions = cursor.fetchone()[0]
            
            cursor.execute('SELECT COUNT(*) FROM addresses')
            total_addresses = cursor.fetchone()[0]
            
            return {
                'total_blocks': total_blocks,
                'total_transactions': total_transactions,
                'total_addresses': total_addresses,
                'total_supply': 31000000,  # Max supply
                'hash_rate': 0.0,
                'difficulty': 0.0,
                'avg_block_time': 0.0
            }
            
        except Exception as e:
            print(f"Error getting network stats: {e}")
            return {}
        finally:
            conn.close()
    
    def start_block_monitor(self):
        """Start monitoring for new blocks"""
        def monitor():
            while True:
                try:
                    latest_block = self.get_latest_block()
                    if latest_block:
                        self.store_block(latest_block)
                        
                        # Emit to connected clients
                        socketio.emit('new_block', latest_block)
                        
                        # Update network stats
                        self.update_network_stats()
                    
                    time.sleep(5)  # Check every 5 seconds
                    
                except Exception as e:
                    print(f"Block monitor error: {e}")
                    time.sleep(10)  # Wait longer on error
        
        thread = threading.Thread(target=monitor)
        thread.daemon = True
        thread.start()
    
    def update_network_stats(self):
        """Update network statistics"""
        conn = sqlite3.connect(DATABASE_PATH)
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
explorer = BlockchainExplorer()

# Web Routes
@app.route('/')
def index():
    """Main explorer page"""
    recent_blocks = explorer.get_recent_blocks(10)
    recent_transactions = explorer.get_recent_transactions(15)
    network_stats = explorer.get_network_stats()
    
    return render_template('explorer_index.html', 
                         blocks=recent_blocks,
                         transactions=recent_transactions,
                         stats=network_stats)

@app.route('/block/<block_number>')
def block_detail(block_number):
    """Block detail page"""
    conn = sqlite3.connect(DATABASE_PATH)
    cursor = conn.cursor()
    
    try:
        # Get block data
        cursor.execute('''
            SELECT * FROM blocks WHERE block_number = ?
        ''', (block_number,))
        
        block = cursor.fetchone()
        if not block:
            return "Block not found", 404
        
        # Get block transactions
        cursor.execute('''
            SELECT * FROM transactions WHERE block_number = ?
            ORDER BY tx_index
        ''', (block_number,))
        
        transactions = cursor.fetchall()
        
        return render_template('block_detail.html', 
                             block=block,
                             transactions=transactions)
        
    finally:
        conn.close()

@app.route('/tx/<tx_hash>')
def transaction_detail(tx_hash):
    """Transaction detail page"""
    conn = sqlite3.connect(DATABASE_PATH)
    cursor = conn.cursor()
    
    try:
        cursor.execute('''
            SELECT * FROM transactions WHERE tx_hash = ?
        ''', (tx_hash,))
        
        transaction = cursor.fetchone()
        if not transaction:
            return "Transaction not found", 404
        
        return render_template('transaction_detail.html', 
                             transaction=transaction)
        
    finally:
        conn.close()

@app.route('/address/<address>')
def address_detail(address):
    """Address detail page"""
    conn = sqlite3.connect(DATABASE_PATH)
    cursor = conn.cursor()
    
    try:
        # Get address info
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
            ORDER BY block_number DESC, tx_index DESC
            LIMIT 50
        ''', (address, address))
        
        transactions = cursor.fetchall()
        
        return render_template('address_detail.html',
                             address_info=addr_info,
                             transactions=transactions)
        
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

# API Routes
@app.route('/api/blocks')
def api_blocks():
    """API endpoint for recent blocks"""
    limit = min(int(request.args.get('limit', 10)), 100)
    blocks = explorer.get_recent_blocks(limit)
    return jsonify(blocks)

@app.route('/api/transactions')
def api_transactions():
    """API endpoint for recent transactions"""
    limit = min(int(request.args.get('limit', 20)), 100)
    transactions = explorer.get_recent_transactions(limit)
    return jsonify(transactions)

@app.route('/api/stats')
def api_stats():
    """API endpoint for network statistics"""
    stats = explorer.get_network_stats()
    return jsonify(stats)

# WebSocket Events
@socketio.on('connect')
def handle_connect():
    """Handle client connection"""
    emit('connected', {'message': 'Connected to GXC Explorer'})

@socketio.on('disconnect')
def handle_disconnect():
    """Handle client disconnection"""
    print('Client disconnected')

if __name__ == '__main__':
    # Ensure database is initialized
    explorer.init_database()
    
    # Run the application
    socketio.run(app, host='0.0.0.0', port=3000, debug=False)