# 🌟 GXC Blockchain Ecosystem Complete Guide

## 🎯 Overview

The GXC blockchain now provides a complete real-world ecosystem with:

- **Wallet Creation APIs** - Allow users to create and manage wallets programmatically
- **Blockchain Explorer Website** - Beautiful interface to view transactions like real blockchains
- **User Authentication System** - Secure login and credential storage for network growth
- **Stock Market Maker Software** - Automated trading for tokenized stock contracts
- **Persistent Data Storage** - All user data stored securely for cross-app access

---

## 🚀 Quick Start

### 1. Start the Complete Ecosystem

```bash
# Start all services
chmod +x scripts/start_ecosystem.sh
./scripts/start_ecosystem.sh

# Services will be available at:
# - Blockchain Node: http://localhost:8545 (RPC)
# - Wallet API: http://localhost:5000
# - Blockchain Explorer: http://localhost:3000
# - Market Maker: http://localhost:4000
```

### 2. Stop the Ecosystem

```bash
./scripts/stop_ecosystem.sh
```

---

## 💼 Wallet Creation API

### Create User Account

```bash
curl -X POST http://localhost:5000/api/v1/register \
  -H "Content-Type: application/json" \
  -d '{
    "username": "john_doe",
    "email": "john@example.com",
    "password": "secure_password123",
    "backup_phrase": "optional recovery phrase"
  }'
```

**Response:**
```json
{
  "success": true,
  "user_id": "550e8400-e29b-41d4-a716-446655440000",
  "message": "User created successfully"
}
```

### Login and Get Token

```bash
curl -X POST http://localhost:5000/api/v1/login \
  -H "Content-Type: application/json" \
  -d '{
    "username": "john_doe",
    "password": "secure_password123"
  }'
```

**Response:**
```json
{
  "success": true,
  "token": "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9...",
  "user_id": "550e8400-e29b-41d4-a716-446655440000",
  "expires_in": 86400
}
```

### Create Wallet

```bash
curl -X POST http://localhost:5000/api/v1/wallets/create \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer YOUR_JWT_TOKEN" \
  -d '{
    "wallet_name": "My Main Wallet",
    "password": "wallet_password123",
    "wallet_type": "standard"
  }'
```

**Response:**
```json
{
  "success": true,
  "wallet_id": "wallet_550e8400-e29b-41d4-a716-446655440000",
  "address": "GXC1a2b3c4d5e6f7g8h9i0j1k2l3m4n5o6p7q8r9s0",
  "public_key": "-----BEGIN PUBLIC KEY-----\nMCowBQYDK2VwAyEA...",
  "qr_data": {
    "address": "GXC1a2b3c4d5e6f7g8h9i0j1k2l3m4n5o6p7q8r9s0",
    "network": "GXC",
    "type": "wallet"
  },
  "message": "Wallet created successfully"
}
```

### Store Wallet Backup (for Cross-App Access)

```bash
curl -X POST http://localhost:5000/api/v1/wallets/WALLET_ID/backup \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer YOUR_JWT_TOKEN" \
  -d '{
    "storage_type": "cloud",
    "storage_reference": "https://mycloud.com/backup/wallet_backup.enc",
    "encryption_hint": "Use your master password to decrypt"
  }'
```

### Get User Wallets

```bash
curl -X GET http://localhost:5000/api/v1/wallets \
  -H "Authorization: Bearer YOUR_JWT_TOKEN"
```

---

## 🔍 Blockchain Explorer

Visit **http://localhost:3000** to access the beautiful blockchain explorer:

### Features:
- **Real-time block and transaction display**
- **Search functionality** (addresses, transaction hashes, block numbers)
- **Live WebSocket updates** for new blocks
- **Network statistics dashboard**
- **Address transaction history**
- **Mobile-responsive design**

### API Endpoints:
```bash
# Get recent blocks
curl http://localhost:3000/api/blocks?limit=10

# Get recent transactions  
curl http://localhost:3000/api/transactions?limit=20

# Get network statistics
curl http://localhost:3000/api/stats
```

---

## 📈 Stock Market Maker

Access the market maker dashboard at **http://localhost:4000**

### Features:
- **Automated stock contract trading**
- **Real-time market data from Yahoo Finance**
- **Portfolio management and P&L tracking**
- **Multiple stock symbols support**
- **Risk management with position limits**

### Default Tracked Stocks:
- AAPL (Apple Inc.)
- MSFT (Microsoft Corporation)
- GOOGL (Alphabet Inc.)
- AMZN (Amazon.com Inc.)
- TSLA (Tesla Inc.)
- NVDA (NVIDIA Corporation)
- META (Meta Platforms Inc.)
- BRK-B (Berkshire Hathaway Inc.)
- JPM (JPMorgan Chase & Co.)
- JNJ (Johnson & Johnson)

### API Endpoints:
```bash
# Get portfolio summary
curl http://localhost:4000/api/portfolio

# Get all positions
curl http://localhost:4000/api/positions

# Get active orders
curl http://localhost:4000/api/orders

# Add new stock to track
curl -X POST http://localhost:4000/api/add_stock \
  -H "Content-Type: application/json" \
  -d '{
    "symbol": "NFLX",
    "company_name": "Netflix Inc.",
    "exchange": "NASDAQ",
    "sector": "Communication Services"
  }'
```

---

## 🛠️ Building Apps on GXC

### 1. Wallet Integration for Your App

```python
import requests
import json

class GXCWalletClient:
    def __init__(self, api_url="http://localhost:5000"):
        self.api_url = api_url
        self.token = None
    
    def register_user(self, username, email, password):
        """Register a new user"""
        response = requests.post(f"{self.api_url}/api/v1/register", json={
            "username": username,
            "email": email,
            "password": password
        })
        return response.json()
    
    def login(self, username, password):
        """Login and store token"""
        response = requests.post(f"{self.api_url}/api/v1/login", json={
            "username": username,
            "password": password
        })
        data = response.json()
        if data.get('success'):
            self.token = data['token']
        return data
    
    def create_wallet(self, wallet_name, password):
        """Create a new wallet"""
        headers = {"Authorization": f"Bearer {self.token}"}
        response = requests.post(f"{self.api_url}/api/v1/wallets/create", 
                               headers=headers, json={
            "wallet_name": wallet_name,
            "password": password
        })
        return response.json()
    
    def get_wallets(self):
        """Get user's wallets"""
        headers = {"Authorization": f"Bearer {self.token}"}
        response = requests.get(f"{self.api_url}/api/v1/wallets", headers=headers)
        return response.json()

# Usage example
client = GXCWalletClient()

# Register user
result = client.register_user("myapp_user", "user@myapp.com", "password123")
print(f"Registration: {result}")

# Login
login_result = client.login("myapp_user", "password123")
print(f"Login: {login_result}")

# Create wallet
wallet_result = client.create_wallet("MyApp Wallet", "wallet_pass")
print(f"Wallet: {wallet_result}")
```

### 2. Blockchain Data Integration

```javascript
// JavaScript client for blockchain explorer
class GXCExplorerClient {
    constructor(explorerUrl = 'http://localhost:3000') {
        this.explorerUrl = explorerUrl;
    }
    
    async getRecentBlocks(limit = 10) {
        const response = await fetch(`${this.explorerUrl}/api/blocks?limit=${limit}`);
        return await response.json();
    }
    
    async getRecentTransactions(limit = 20) {
        const response = await fetch(`${this.explorerUrl}/api/transactions?limit=${limit}`);
        return await response.json();
    }
    
    async getNetworkStats() {
        const response = await fetch(`${this.explorerUrl}/api/stats`);
        return await response.json();
    }
    
    // WebSocket for live updates
    connectLiveUpdates() {
        const socket = io(this.explorerUrl);
        
        socket.on('new_block', (block) => {
            console.log('New block received:', block);
            // Handle new block in your app
        });
        
        socket.on('new_transaction', (tx) => {
            console.log('New transaction received:', tx);
            // Handle new transaction in your app
        });
        
        return socket;
    }
}

// Usage
const explorer = new GXCExplorerClient();

// Get data
explorer.getNetworkStats().then(stats => {
    console.log('Network stats:', stats);
});

// Connect to live updates
const socket = explorer.connectLiveUpdates();
```

### 3. Stock Market Integration

```python
import requests

class GXCMarketClient:
    def __init__(self, market_url="http://localhost:4000"):
        self.market_url = market_url
    
    def get_portfolio_summary(self):
        """Get portfolio summary"""
        response = requests.get(f"{self.market_url}/api/portfolio")
        return response.json()
    
    def get_positions(self):
        """Get all positions"""
        response = requests.get(f"{self.market_url}/api/positions")
        return response.json()
    
    def get_market_data(self, symbol):
        """Get market data for symbol"""
        response = requests.get(f"{self.market_url}/api/market_data/{symbol}")
        return response.json()
    
    def add_stock_to_track(self, symbol, company_name, exchange, sector=None):
        """Add new stock to tracking"""
        response = requests.post(f"{self.market_url}/api/add_stock", json={
            "symbol": symbol,
            "company_name": company_name,
            "exchange": exchange,
            "sector": sector
        })
        return response.json()

# Usage
market = GXCMarketClient()

# Get portfolio
portfolio = market.get_portfolio_summary()
print(f"Portfolio: {portfolio}")

# Get Apple stock data
aapl_data = market.get_market_data("AAPL")
print(f"AAPL Data: {aapl_data}")
```

---

## 🔐 User Authentication & Cross-App Access

### Session Management

The GXC ecosystem uses JWT tokens for authentication:

1. **Register once** - Users register with the wallet API
2. **Login anywhere** - Use credentials across all GXC apps
3. **Persistent storage** - All wallet data stored in SQLite database
4. **Backup system** - Users can store encrypted backups for recovery

### Database Schema

```sql
-- Users table
CREATE TABLE users (
    user_id TEXT PRIMARY KEY,
    username TEXT UNIQUE NOT NULL,
    email TEXT UNIQUE NOT NULL,
    password_hash TEXT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    backup_phrase_hash TEXT
);

-- Wallets table  
CREATE TABLE wallets (
    wallet_id TEXT PRIMARY KEY,
    user_id TEXT NOT NULL,
    wallet_name TEXT NOT NULL,
    address TEXT UNIQUE NOT NULL,
    encrypted_private_key TEXT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    backup_stored BOOLEAN DEFAULT FALSE,
    FOREIGN KEY (user_id) REFERENCES users (user_id)
);

-- Backup storage references
CREATE TABLE backup_storage (
    backup_id TEXT PRIMARY KEY,
    user_id TEXT NOT NULL,
    wallet_id TEXT NOT NULL,
    storage_type TEXT NOT NULL,
    storage_reference TEXT NOT NULL,
    encryption_key_hint TEXT
);
```

---

## 🏗️ App Development Examples

### 1. DeFi Trading App

```python
# Example DeFi app using GXC ecosystem
class GXCDeFiApp:
    def __init__(self):
        self.wallet_client = GXCWalletClient()
        self.market_client = GXCMarketClient()
        self.explorer_client = GXCExplorerClient()
    
    def create_trading_account(self, username, email, password):
        # Register user
        register_result = self.wallet_client.register_user(username, email, password)
        if not register_result.get('success'):
            return register_result
        
        # Login
        login_result = self.wallet_client.login(username, password)
        if not login_result.get('success'):
            return login_result
        
        # Create trading wallet
        wallet_result = self.wallet_client.create_wallet("Trading Wallet", password)
        
        return {
            'success': True,
            'user_id': register_result['user_id'],
            'wallet_address': wallet_result['address'],
            'message': 'Trading account created successfully'
        }
    
    def get_trading_dashboard_data(self):
        # Get portfolio from market maker
        portfolio = self.market_client.get_portfolio_summary()
        
        # Get recent transactions from explorer
        recent_txs = self.explorer_client.get_recent_transactions(10)
        
        # Get network stats
        network_stats = self.explorer_client.get_network_stats()
        
        return {
            'portfolio': portfolio,
            'recent_transactions': recent_txs,
            'network_stats': network_stats
        }
```

### 2. NFT Marketplace

```python
class GXCNFTMarketplace:
    def __init__(self):
        self.wallet_client = GXCWalletClient()
        self.explorer_client = GXCExplorerClient()
    
    def create_artist_account(self, username, email, password):
        # Use existing wallet system
        result = self.wallet_client.register_user(username, email, password)
        if result.get('success'):
            # Login and create NFT wallet
            self.wallet_client.login(username, password)
            wallet = self.wallet_client.create_wallet("NFT Collection Wallet", password)
            
            # Store backup for cross-device access
            backup_result = self.wallet_client.store_backup(
                wallet['wallet_id'],
                "ipfs",
                f"ipfs://QmHash{wallet['wallet_id']}",
                "Use your account password"
            )
            
            return {
                'success': True,
                'artist_wallet': wallet['address'],
                'backup_stored': backup_result.get('success', False)
            }
        return result
```

### 3. Gaming Platform

```python
class GXCGamingPlatform:
    def __init__(self):
        self.wallet_client = GXCWalletClient()
    
    def create_gamer_profile(self, username, email, password):
        # Register gamer
        result = self.wallet_client.register_user(username, email, password)
        if not result.get('success'):
            return result
        
        # Login
        self.wallet_client.login(username, password)
        
        # Create gaming wallet
        gaming_wallet = self.wallet_client.create_wallet("Gaming Wallet", password)
        
        # Create rewards wallet
        rewards_wallet = self.wallet_client.create_wallet("Rewards Wallet", password)
        
        return {
            'success': True,
            'gamer_id': result['user_id'],
            'gaming_address': gaming_wallet['address'],
            'rewards_address': rewards_wallet['address'],
            'message': 'Gaming profile created successfully'
        }
```

---

## 🌐 Network Growth & Scalability

### Cross-App Compatibility

Once users create accounts in the GXC ecosystem:

1. **Single Sign-On** - Login works across all GXC-based apps
2. **Wallet Portability** - Same wallets available in all apps
3. **Shared Identity** - User profiles persist across the network
4. **Backup Recovery** - Access wallets from any device/app

### Database Replication

For network growth, the system supports:

```bash
# Master-slave database replication
# Primary wallet service
python3 api/wallet_service.py --mode=primary --port=5000

# Replica wallet services  
python3 api/wallet_service.py --mode=replica --primary=http://primary:5000 --port=5001
python3 api/wallet_service.py --mode=replica --primary=http://primary:5000 --port=5002
```

### Load Balancing

```nginx
# Nginx configuration for load balancing
upstream gxc_wallet_api {
    server localhost:5000;
    server localhost:5001;
    server localhost:5002;
}

server {
    listen 80;
    server_name wallet-api.gxc.network;
    
    location / {
        proxy_pass http://gxc_wallet_api;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
    }
}
```

---

## 📊 Monitoring & Analytics

### Service Health Monitoring

```bash
# Check all services
curl http://localhost:5000/api/v1/health    # Wallet API
curl http://localhost:3000/api/stats        # Explorer
curl http://localhost:4000/api/portfolio    # Market Maker
```

### Log Analysis

```bash
# View logs
tail -f logs/wallet_api.log
tail -f logs/explorer.log  
tail -f logs/market_maker.log
tail -f logs/blockchain.log

# Search for errors
grep -i error logs/*.log

# Monitor user registrations
grep "User created successfully" logs/wallet_api.log | wc -l
```

---

## 🚀 Production Deployment

### Docker Deployment

```dockerfile
# Dockerfile.ecosystem
FROM ubuntu:22.04

# Install dependencies
RUN apt-get update && apt-get install -y \
    python3 python3-pip nodejs npm \
    build-essential cmake libssl-dev

# Copy GXC blockchain
COPY . /gxc
WORKDIR /gxc

# Build blockchain
RUN mkdir build && cd build && cmake .. && make

# Install Python dependencies
RUN pip3 install flask flask-cors flask-socketio sqlite3 bcrypt pyjwt cryptography qrcode requests yfinance numpy

# Expose ports
EXPOSE 8545 5000 3000 4000

# Start ecosystem
CMD ["./scripts/start_ecosystem.sh", "--monitor"]
```

```bash
# Build and run
docker build -f Dockerfile.ecosystem -t gxc-ecosystem .
docker run -p 8545:8545 -p 5000:5000 -p 3000:3000 -p 4000:4000 gxc-ecosystem
```

### Environment Variables

```bash
# Production configuration
export GXC_ENV=production
export JWT_SECRET=your-super-secret-jwt-key
export DATABASE_PATH=/data/gxc_wallets.db
export BLOCKCHAIN_NODE_URL=http://blockchain:8545
export BACKUP_STORAGE_PATH=/backups/
```

---

## 🎉 Success! 

You now have a complete blockchain ecosystem that provides:

✅ **Wallet Creation APIs** - Users can create wallets programmatically  
✅ **Beautiful Blockchain Explorer** - Real-time transaction viewing  
✅ **User Authentication System** - Secure login across apps  
✅ **Stock Market Maker** - Automated trading for tokenized assets  
✅ **Persistent Data Storage** - Cross-app wallet access  
✅ **Real-world Production Features** - Everything needed for a live blockchain

**The GXC blockchain is ready for real-world applications! 🚀**

---

## 📞 Support

For questions or issues:
- Check logs in the `logs/` directory
- Use the health check endpoints
- Review the API documentation in each service
- Examine the SQLite databases for user data

**Happy building on GXC! 🎯**