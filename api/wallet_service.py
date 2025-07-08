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
from datetime import datetime, timedelta
from flask import Flask, request, jsonify, render_template
from flask_cors import CORS
from cryptography.hazmat.primitives.asymmetric import ed25519
from cryptography.hazmat.primitives import serialization
import base64
import uuid

app = Flask(__name__)
CORS(app)

# Configuration
JWT_SECRET = os.environ.get('JWT_SECRET', secrets.token_hex(32))
DATABASE_PATH = 'gxc_wallets.db'
BACKUP_STORAGE_PATH = 'wallet_backups/'

class WalletService:
    def __init__(self):
        self.init_database()
        
    def init_database(self):
        """Initialize the wallet database"""
        conn = sqlite3.connect(DATABASE_PATH)
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
                wallet_type TEXT DEFAULT 'standard',
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                last_used TIMESTAMP,
                balance REAL DEFAULT 0.0,
                is_default BOOLEAN DEFAULT FALSE,
                backup_stored BOOLEAN DEFAULT FALSE,
                FOREIGN KEY (user_id) REFERENCES users (user_id)
            )
        ''')
        
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
        """Generate GXC address from public key"""
        # Hash the public key
        public_key_bytes = public_key_pem.encode()
        hash1 = hashlib.sha256(public_key_bytes).digest()
        hash2 = hashlib.sha256(hash1).digest()
        
        # Take first 20 bytes and add GXC prefix
        address_bytes = hash2[:20]
        address = 'GXC' + address_bytes.hex()
        
        return address
    
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
    
    def authenticate_user(self, username, password):
        """Authenticate user and return JWT token"""
        conn = sqlite3.connect(DATABASE_PATH)
        cursor = conn.cursor()
        
        try:
            cursor.execute('''
                SELECT user_id, password_hash, is_active 
                FROM users 
                WHERE username = ? OR email = ?
            ''', (username, username))
            
            user = cursor.fetchone()
            if not user:
                return {'success': False, 'error': 'User not found'}
            
            user_id, password_hash, is_active = user
            
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
                'username': username,
                'exp': datetime.utcnow() + timedelta(hours=24)
            }
            token = jwt.encode(payload, JWT_SECRET, algorithm='HS256')
            
            return {
                'success': True,
                'token': token,
                'user_id': user_id,
                'expires_in': 86400  # 24 hours
            }
            
        except Exception as e:
            return {'success': False, 'error': str(e)}
        finally:
            conn.close()
    
    def create_wallet(self, user_id, wallet_name, password, wallet_type='standard'):
        """Create a new wallet for user"""
        conn = sqlite3.connect(DATABASE_PATH)
        cursor = conn.cursor()
        
        try:
            # Generate keypair
            private_key, public_key = self.generate_wallet_keypair()
            address = self.generate_address(public_key)
            
            # Encrypt private key
            encrypted_private_key = self.encrypt_private_key(private_key, password)
            
            # Generate wallet ID
            wallet_id = str(uuid.uuid4())
            
            # Check if this is the first wallet (make it default)
            cursor.execute('SELECT COUNT(*) FROM wallets WHERE user_id = ?', (user_id,))
            is_default = cursor.fetchone()[0] == 0
            
            # Insert wallet
            cursor.execute('''
                INSERT INTO wallets (
                    wallet_id, user_id, wallet_name, address, public_key,
                    encrypted_private_key, wallet_type, is_default
                ) VALUES (?, ?, ?, ?, ?, ?, ?, ?)
            ''', (wallet_id, user_id, wallet_name, address, public_key, 
                  encrypted_private_key, wallet_type, is_default))
            
            conn.commit()
            
            # Generate QR code for address
            qr_data = {
                'address': address,
                'network': 'GXC',
                'type': 'wallet'
            }
            
            return {
                'success': True,
                'wallet_id': wallet_id,
                'address': address,
                'public_key': public_key,
                'qr_data': qr_data,
                'message': 'Wallet created successfully'
            }
            
        except Exception as e:
            conn.rollback()
            return {'success': False, 'error': str(e)}
        finally:
            conn.close()
    
    def get_user_wallets(self, user_id):
        """Get all wallets for a user"""
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
                wallets.append({
                    'wallet_id': row[0],
                    'wallet_name': row[1],
                    'address': row[2],
                    'wallet_type': row[3],
                    'created_at': row[4],
                    'last_used': row[5],
                    'balance': row[6],
                    'is_default': bool(row[7])
                })
            
            return {'success': True, 'wallets': wallets}
            
        except Exception as e:
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
wallet_service = WalletService()

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
    
    result = wallet_service.create_wallet(
        user_id,
        data['wallet_name'],
        data['password'],
        data.get('wallet_type', 'standard')
    )
    
    return jsonify(result), 201 if result['success'] else 400

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
    
    return jsonify(result)

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
        'timestamp': datetime.utcnow().isoformat()
    })

if __name__ == '__main__':
    # Ensure backup directory exists
    os.makedirs(BACKUP_STORAGE_PATH, exist_ok=True)
    
    # Run the application
    app.run(host='0.0.0.0', port=5000, debug=False)