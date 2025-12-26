# GXC Blockchain Explorer - Complete Documentation

## Table of Contents
1. [Overview](#overview)
2. [Features](#features)
3. [Installation](#installation)
4. [Configuration](#configuration)
5. [User Guide](#user-guide)
6. [API Reference](#api-reference)
7. [Architecture](#architecture)
8. [Troubleshooting](#troubleshooting)
9. [Development](#development)

---

## Overview

The GXC Blockchain Explorer is a comprehensive web application for visualizing and interacting with the GXC blockchain. It provides real-time data, transaction tracking, address monitoring, validator information, and much more.

### Key Features
- 🔍 **Real-time blockchain data**
- 📊 **Advanced analytics and charts**
- 💰 **Wallet integration**
- ⛏️ **Mining statistics**
- 🏛️ **Validator dashboard**
- 📈 **Stock token tracking**
- 🔗 **Cross-chain bridge**
- 🌐 **Multi-network support** (Testnet/Mainnet)
- 📱 **Responsive design**
- 🔌 **RESTful API**

### Technology Stack
- **Backend:** Python 3.8+ (Flask)
- **Frontend:** HTML5, CSS3, JavaScript
- **Real-time:** WebSocket (Socket.IO)
- **Database:** SQLite3
- **Charts:** Chart.js
- **Icons:** Font Awesome

---

## Features

### 1. Home Dashboard
**URL:** `/`

**Features:**
- Real-time blockchain statistics
- Latest blocks (live updates)
- Recent transactions
- Network status
- Hashrate and difficulty
- Active validators
- Market data
- Quick search

**Live Updates:**
- New blocks appear automatically
- Transaction count updates
- Network stats refresh every 10 seconds

### 2. Block Explorer
**URL:** `/blocks`

**Features:**
- Paginated block list
- Block details:
  - Block number/height
  - Block hash
  - Timestamp
  - Miner address
  - Number of transactions
  - Block reward
  - Difficulty
  - Nonce
  - Merkle root
- Filter by date range
- Sort by height/time
- Export to CSV/JSON

**Block Detail Page:** `/block/<block_number>`
- Complete block information
- All transactions in block
- Previous/Next block navigation
- Block validation status
- Mining algorithm used

### 3. Transaction Explorer
**URL:** `/transactions`

**Features:**
- Real-time transaction list
- Transaction details:
  - Transaction hash
  - From/To addresses
  - Amount
  - Fee
  - Timestamp
  - Confirmations
  - Block number
  - Status
- Filter by:
  - Type (normal, stake, unstake, coinbase)
  - Date range
  - Amount range
  - Address
- Search by TX hash
- Export transactions

**Transaction Detail Page:** `/tx/<tx_hash>`
- Complete transaction data
- Input/Output details
- Confirmation status
- Block inclusion
- Transaction graph
- Related transactions

### 4. Address Explorer
**URL:** `/address/<address>`

**Features:**
- Address balance
- Transaction history
- Sent/Received totals
- Token holdings
- Staking information
- QR code generation
- Address tags/labels
- Watch address (notifications)

**Address List:** `/addresses`
- Top addresses by balance
- Rich list
- Active addresses
- New addresses
- Filter and sort

### 5. Validator Dashboard
**URL:** `/validators`

**Features:**
- List all validators
- Validator details:
  - Address
  - Total stake
  - Staking days
  - Blocks mined
  - Total rewards
  - Uptime
  - Status (active/inactive)
- Stake to validator
- View validator performance
- Validator rankings
- Reward calculator

**Validator Actions:**
- Register as validator
- Stake coins
- Unstake coins
- View staking history
- Calculate expected rewards

### 6. Mining Dashboard
**URL:** `/mining`

**Features:**
- Network hashrate
- Mining difficulty
- Block time
- Mining algorithms:
  - SHA256
  - GXHASH (ASIC-resistant)
  - ETHASH (GPU-optimized)
- Mining pools
- Solo mining guide
- Mining calculator
- Mining software downloads

**Mining Guide:** `/mining/guide`
- Step-by-step setup
- Hardware requirements
- Software installation
- Configuration
- Troubleshooting

### 7. Stock Tokens
**URL:** `/stocks`

**Features:**
- Synthetic stock tokens
- Real-time prices
- Trading interface
- Stock details:
  - Ticker symbol
  - Current price
  - 24h change
  - Volume
  - Market cap
  - Chart
- Buy/Sell tokens
- Portfolio tracking

**Stock Detail:** `/stocks/<ticker>`
- Price history
- Trading volume
- Order book
- Recent trades
- Technical indicators

### 8. Wallet Interface
**URL:** `/wallet`

**Features:**
- Create new wallet
- Import wallet (private key/mnemonic)
- View balance
- Send transactions
- Receive (QR code)
- Transaction history
- Stake management
- Token management
- Export wallet

**Wallet Actions:**
- Send GXC
- Stake coins
- Unstake coins
- Buy stock tokens
- Sell stock tokens
- View pending transactions

### 9. Charts & Analytics
**URL:** `/charts`

**Features:**
- Price charts
- Transaction volume
- Active addresses
- Hashrate history
- Difficulty history
- Block time
- Fee trends
- Network growth
- Validator statistics

**Chart Types:**
- Line charts
- Bar charts
- Pie charts
- Area charts
- Candlestick (for stocks)

### 10. Cross-Chain Bridge
**URL:** `/bridge`

**Features:**
- Bridge GXC to other chains
- Supported chains:
  - Ethereum
  - Binance Smart Chain
  - Polygon
- Bridge transactions
- Bridge status
- Fee calculator

### 11. Search
**URL:** `/search?q=<query>`

**Search Types:**
- Block by number/hash
- Transaction by hash
- Address
- Validator
- Stock ticker

**Features:**
- Auto-complete
- Recent searches
- Search suggestions
- Advanced filters

### 12. API Documentation
**URL:** `/api`

**Features:**
- Complete API reference
- Interactive examples
- Authentication guide
- Rate limits
- Error codes
- Code samples (curl, Python, JavaScript)

---

## Installation

### Prerequisites
```bash
# Python 3.8 or higher
python3 --version

# pip package manager
pip3 --version

# Git
git --version
```

### Step 1: Clone Repository
```bash
git clone https://github.com/philani1H/GXC-goldxcoin-blockchain-.git
cd GXC-goldxcoin-blockchain-
```

### Step 2: Install Dependencies
```bash
# Install Python packages
pip3 install -r requirements.txt

# Or install manually
pip3 install flask flask-socketio requests sqlite3
```

### Step 3: Configure Environment
```bash
# Copy example config
cp .env.example .env

# Edit configuration
nano .env
```

### Step 4: Initialize Database
```bash
# Create database
python3 web/blockchain_explorer.py --init-db

# Or let it auto-create on first run
```

### Step 5: Start Explorer
```bash
# Development mode
python3 web/blockchain_explorer.py

# Production mode
gunicorn -w 4 -b 0.0.0.0:5000 web.blockchain_explorer:app

# With WebSocket support
gunicorn -k geventwebsocket.gunicorn.workers.GeventWebSocketWorker \
  -w 1 -b 0.0.0.0:5000 web.blockchain_explorer:app
```

### Step 6: Access Explorer
```
Open browser: http://localhost:5000
```

---

## Configuration

### Environment Variables

```bash
# Network Configuration
GXC_NETWORK=testnet              # testnet or mainnet
BLOCKCHAIN_NODE_URL=http://localhost:8332
RAILWAY_NODE_URL=https://...     # Railway deployment URL

# Database
DATABASE_PATH=/tmp/gxc_explorer_testnet.db

# Server
FLASK_ENV=development            # development or production
FLASK_DEBUG=1                    # 0 or 1
PORT=5000

# Forum Integration
FORUM_URL=http://localhost:5001

# API Keys (optional)
API_KEY_REQUIRED=false
API_RATE_LIMIT=100              # requests per minute

# WebSocket
SOCKETIO_ASYNC_MODE=threading   # threading or gevent
```

### Network Configuration

**Testnet:**
```python
NETWORK_CONFIG = {
    'testnet': {
        'address_prefix': 'tGXC',
        'block_time': 120,          # 2 minutes
        'difficulty': 0.1,
        'block_reward': 50.0,
        'rpc_port': 8332,
    }
}
```

**Mainnet:**
```python
NETWORK_CONFIG = {
    'mainnet': {
        'address_prefix': 'GXC',
        'block_time': 600,          # 10 minutes
        'difficulty': 1.0,
        'block_reward': 50.0,
        'rpc_port': 8332,
    }
}
```

### Database Schema

The explorer uses SQLite with the following tables:

**blocks:**
- block_number (PRIMARY KEY)
- block_hash
- timestamp
- miner_address
- difficulty
- nonce
- merkle_root
- transaction_count
- block_reward

**transactions:**
- tx_hash (PRIMARY KEY)
- block_number
- from_address
- to_address
- amount
- fee
- timestamp
- type
- status

**addresses:**
- address (PRIMARY KEY)
- balance
- total_received
- total_sent
- transaction_count
- first_seen
- last_seen

**validators:**
- address (PRIMARY KEY)
- stake_amount
- staking_days
- registration_time
- blocks_mined
- total_rewards
- is_active

---

## User Guide

### For General Users

#### Viewing Blockchain Data
1. **Home Page:** See latest blocks and transactions
2. **Search:** Use search bar to find blocks, transactions, or addresses
3. **Blocks:** Browse all blocks with pagination
4. **Transactions:** View all transactions with filters
5. **Addresses:** Check any address balance and history

#### Tracking Transactions
1. Get transaction hash from wallet
2. Enter hash in search bar
3. View transaction details
4. Check confirmation status
5. See block inclusion

#### Monitoring Addresses
1. Enter address in search
2. View balance and history
3. Click "Watch Address" for notifications
4. Export transaction history

### For Miners

#### Viewing Mining Stats
1. Go to `/mining`
2. See network hashrate
3. Check current difficulty
4. View block rewards
5. Calculate profitability

#### Choosing Mining Software
1. Go to `/mining/guide`
2. Select your hardware (CPU/GPU/ASIC)
3. Download appropriate miner
4. Follow setup instructions
5. Start mining

#### Tracking Mining Rewards
1. Enter your mining address
2. View mined blocks
3. Check total rewards
4. Monitor pending rewards

### For Validators

#### Becoming a Validator
1. Go to `/validators`
2. Click "Register as Validator"
3. Enter stake amount (minimum 1000 GXC)
4. Choose staking period (30-365 days)
5. Confirm transaction
6. Wait for confirmation

#### Staking to Validators
1. Go to `/validators`
2. Browse validator list
3. Click "Stake" on chosen validator
4. Enter stake amount
5. Choose staking period
6. Confirm transaction

#### Managing Stakes
1. Go to `/wallet`
2. View "My Stakes" section
3. See active stakes
4. Unstake when period ends
5. Claim rewards

### For Traders

#### Trading Stock Tokens
1. Go to `/stocks`
2. Browse available stocks
3. Click on stock ticker
4. View price and chart
5. Click "Buy" or "Sell"
6. Enter amount
7. Confirm transaction

#### Tracking Portfolio
1. Go to `/wallet`
2. View "Token Holdings"
3. See all stock tokens
4. Check profit/loss
5. View transaction history

### For Developers

#### Using the API
1. Go to `/api`
2. Read API documentation
3. Get API key (if required)
4. Make API calls
5. Handle responses

#### Integrating Explorer
1. Use REST API endpoints
2. Subscribe to WebSocket events
3. Parse JSON responses
4. Handle errors
5. Implement rate limiting

---

## API Reference

### Base URL
```
http://localhost:5000/api/v1
```

### Authentication
```bash
# If API keys are required
curl -H "X-API-Key: YOUR_API_KEY" http://localhost:5000/api/v1/stats
```

### Endpoints

#### 1. Get Blockchain Stats
```
GET /api/v1/stats
```

**Response:**
```json
{
  "height": 12345,
  "difficulty": 1.5,
  "hashrate": 1000000,
  "total_transactions": 50000,
  "total_addresses": 10000,
  "validators": 42,
  "network": "testnet"
}
```

#### 2. Get Latest Blocks
```
GET /api/v1/blocks?limit=10&offset=0
```

**Parameters:**
- `limit`: Number of blocks (default: 10, max: 100)
- `offset`: Pagination offset (default: 0)

**Response:**
```json
{
  "blocks": [
    {
      "number": 12345,
      "hash": "0x...",
      "timestamp": 1640995200,
      "miner": "tGXC...",
      "transactions": 5,
      "reward": 50.0
    }
  ],
  "total": 12345
}
```

#### 3. Get Block by Number
```
GET /api/v1/block/<number>
```

**Response:**
```json
{
  "number": 12345,
  "hash": "0x...",
  "previous_hash": "0x...",
  "timestamp": 1640995200,
  "miner": "tGXC...",
  "difficulty": 1.5,
  "nonce": 123456,
  "merkle_root": "0x...",
  "transactions": [...]
}
```

#### 4. Get Transaction
```
GET /api/v1/tx/<hash>
```

**Response:**
```json
{
  "hash": "0x...",
  "block_number": 12345,
  "from": "tGXC...",
  "to": "tGXC...",
  "amount": 10.5,
  "fee": 0.001,
  "timestamp": 1640995200,
  "confirmations": 6,
  "status": "confirmed"
}
```

#### 5. Get Address Info
```
GET /api/v1/address/<address>
```

**Response:**
```json
{
  "address": "tGXC...",
  "balance": 100.5,
  "total_received": 500.0,
  "total_sent": 399.5,
  "transaction_count": 42,
  "tokens": [...],
  "stakes": [...]
}
```

#### 6. Get Address Transactions
```
GET /api/v1/address/<address>/transactions?limit=10&offset=0
```

**Response:**
```json
{
  "transactions": [...],
  "total": 42
}
```

#### 7. Get Validators
```
GET /api/v1/validators
```

**Response:**
```json
{
  "validators": [
    {
      "address": "tGXC...",
      "stake": 10000.0,
      "staking_days": 365,
      "blocks_mined": 42,
      "total_rewards": 2100.0,
      "is_active": true
    }
  ]
}
```

#### 8. Search
```
GET /api/v1/search?q=<query>
```

**Response:**
```json
{
  "type": "transaction",
  "result": {...}
}
```

#### 9. Get Mining Stats
```
GET /api/v1/mining/stats
```

**Response:**
```json
{
  "hashrate": 1000000,
  "difficulty": 1.5,
  "block_time": 120,
  "algorithm": "GXHASH",
  "next_difficulty": 1.6
}
```

#### 10. Get Stock Tokens
```
GET /api/v1/stocks
```

**Response:**
```json
{
  "stocks": [
    {
      "ticker": "AAPL",
      "price": 150.25,
      "change_24h": 2.5,
      "volume": 1000000
    }
  ]
}
```

### WebSocket Events

#### Connect
```javascript
const socket = io('http://localhost:5000');
```

#### Subscribe to New Blocks
```javascript
socket.on('new_block', (data) => {
  console.log('New block:', data);
});
```

#### Subscribe to New Transactions
```javascript
socket.on('new_transaction', (data) => {
  console.log('New transaction:', data);
});
```

#### Subscribe to Address Updates
```javascript
socket.emit('watch_address', {address: 'tGXC...'});
socket.on('address_update', (data) => {
  console.log('Address updated:', data);
});
```

### Rate Limits
- **Default:** 100 requests per minute per IP
- **Authenticated:** 1000 requests per minute
- **WebSocket:** No limit

### Error Codes
- `200` - Success
- `400` - Bad Request
- `404` - Not Found
- `429` - Rate Limit Exceeded
- `500` - Internal Server Error

---

## Architecture

### System Overview
```
┌─────────────┐
│   Browser   │
└──────┬──────┘
       │ HTTP/WebSocket
       ▼
┌─────────────────────────────┐
│   Flask Web Server          │
│   - Routes                  │
│   - WebSocket Handler       │
│   - API Endpoints           │
└──────┬──────────────┬───────┘
       │              │
       ▼              ▼
┌─────────────┐  ┌──────────────┐
│   SQLite    │  │  Blockchain  │
│   Database  │  │     Node     │
│             │  │   (RPC API)  │
└─────────────┘  └──────────────┘
```

### Components

#### 1. Web Server (Flask)
- Handles HTTP requests
- Serves HTML templates
- Manages WebSocket connections
- Provides REST API

#### 2. Database (SQLite)
- Stores blockchain data
- Caches frequently accessed data
- Indexes for fast queries
- Periodic sync with node

#### 3. Blockchain Node
- Source of truth
- RPC API for data
- Real-time updates
- Transaction broadcasting

#### 4. Frontend
- Responsive HTML/CSS
- JavaScript for interactivity
- Chart.js for visualizations
- Socket.IO for real-time updates

### Data Flow

#### Block Discovery
```
1. Node mines new block
2. Explorer polls node every 10s
3. New block detected
4. Block data fetched via RPC
5. Block stored in database
6. WebSocket broadcast to clients
7. UI updates automatically
```

#### Transaction Tracking
```
1. User submits transaction
2. Transaction enters mempool
3. Explorer detects pending TX
4. TX displayed as "pending"
5. Block includes TX
6. TX status updated to "confirmed"
7. Confirmations increment
```

### Caching Strategy
- **Blocks:** Cached permanently
- **Transactions:** Cached permanently
- **Addresses:** Cached with 60s TTL
- **Stats:** Cached with 10s TTL
- **Validators:** Cached with 30s TTL

### Performance Optimization
- Database indexes on frequently queried fields
- Pagination for large datasets
- Lazy loading of images
- Minified CSS/JS
- Gzip compression
- CDN for static assets

---

## Troubleshooting

### Common Issues

#### 1. Explorer Won't Start
**Error:** `Address already in use`

**Solution:**
```bash
# Kill existing process
pkill -f blockchain_explorer

# Or use different port
PORT=5001 python3 web/blockchain_explorer.py
```

#### 2. Cannot Connect to Node
**Error:** `Connection refused`

**Solution:**
```bash
# Check node is running
curl http://localhost:8332/health

# Check node URL in config
echo $BLOCKCHAIN_NODE_URL

# Try Railway URL
export BLOCKCHAIN_NODE_URL=https://...
```

#### 3. Database Errors
**Error:** `database is locked`

**Solution:**
```bash
# Stop explorer
pkill -f blockchain_explorer

# Remove lock
rm /tmp/gxc_explorer_testnet.db-journal

# Restart explorer
python3 web/blockchain_explorer.py
```

#### 4. Missing Blocks
**Error:** Blocks not showing

**Solution:**
```bash
# Force resync
python3 web/blockchain_explorer.py --resync

# Or delete database and restart
rm /tmp/gxc_explorer_testnet.db
python3 web/blockchain_explorer.py
```

#### 5. WebSocket Not Working
**Error:** Real-time updates not appearing

**Solution:**
```bash
# Install gevent
pip3 install gevent gevent-websocket

# Use gevent worker
gunicorn -k geventwebsocket.gunicorn.workers.GeventWebSocketWorker \
  -w 1 -b 0.0.0.0:5000 web.blockchain_explorer:app
```

### Debug Mode

Enable debug logging:
```bash
export FLASK_DEBUG=1
export LOG_LEVEL=DEBUG
python3 web/blockchain_explorer.py
```

Check logs:
```bash
tail -f logs/explorer.log
```

### Performance Issues

#### Slow Page Load
1. Check database size
2. Add indexes
3. Enable caching
4. Use CDN for assets

#### High Memory Usage
1. Reduce cache size
2. Limit concurrent connections
3. Use pagination
4. Optimize queries

#### High CPU Usage
1. Reduce polling frequency
2. Optimize database queries
3. Use connection pooling
4. Scale horizontally

---

## Development

### Project Structure
```
web/
├── blockchain_explorer.py      # Main application
├── static/                     # Static assets
│   ├── css/
│   ├── js/
│   └── images/
├── templates/                  # HTML templates
│   ├── explorer_index.html
│   ├── block_detail.html
│   ├── transaction_detail.html
│   ├── address_detail.html
│   └── ...
└── requirements.txt           # Python dependencies
```

### Adding New Features

#### 1. Add Route
```python
@app.route('/my-feature')
def my_feature():
    # Your code here
    return render_template('my_feature.html')
```

#### 2. Add Template
```html
<!-- templates/my_feature.html -->
{% extends "base.html" %}
{% block content %}
  <h1>My Feature</h1>
{% endblock %}
```

#### 3. Add API Endpoint
```python
@app.route('/api/v1/my-endpoint')
def api_my_endpoint():
    data = {'result': 'success'}
    return jsonify(data)
```

#### 4. Add WebSocket Event
```python
@socketio.on('my_event')
def handle_my_event(data):
    emit('my_response', {'data': data})
```

### Testing

#### Unit Tests
```bash
python3 -m pytest tests/
```

#### Integration Tests
```bash
python3 tests/test_explorer.py
```

#### Load Testing
```bash
ab -n 1000 -c 10 http://localhost:5000/
```

### Deployment

#### Production Checklist
- [ ] Set `FLASK_ENV=production`
- [ ] Disable debug mode
- [ ] Use production database
- [ ] Enable HTTPS
- [ ] Set up monitoring
- [ ] Configure backups
- [ ] Set rate limits
- [ ] Use CDN
- [ ] Enable caching
- [ ] Set up logging

#### Docker Deployment
```dockerfile
FROM python:3.9
WORKDIR /app
COPY requirements.txt .
RUN pip install -r requirements.txt
COPY . .
CMD ["gunicorn", "-w", "4", "-b", "0.0.0.0:5000", "web.blockchain_explorer:app"]
```

#### Railway Deployment
```bash
# Install Railway CLI
npm install -g @railway/cli

# Login
railway login

# Deploy
railway up
```

---

## Summary

The GXC Blockchain Explorer is a feature-complete web application providing:

✅ **Real-time blockchain data**
✅ **Comprehensive transaction tracking**
✅ **Address monitoring**
✅ **Validator dashboard**
✅ **Mining statistics**
✅ **Stock token trading**
✅ **Wallet integration**
✅ **RESTful API**
✅ **WebSocket support**
✅ **Multi-network support**

**Everything you need to explore the GXC blockchain!** 🚀
