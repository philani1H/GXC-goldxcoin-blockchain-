# GXC Blockchain Explorer - Complete Documentation

## Overview

The GXC Blockchain Explorer is a comprehensive web application for visualizing and interacting with the GXC blockchain. It provides real-time data, transaction tracking, address monitoring, and advanced analytics.

**Live URL:** [https://gxc-explorer.vercel.app](https://gxc-explorer.vercel.app)

## Table of Contents

1. [Features](#features)
2. [All Pages & Routes](#all-pages--routes)
3. [Architecture](#architecture)
4. [Installation](#installation)
5. [Configuration](#configuration)
6. [API Reference](#api-reference)
7. [Developer Guide](#developer-guide)
8. [Deployment](#deployment)

---

## Features

### Core Features

#### 1. **Block Explorer**
- View all blocks with pagination
- Block details including transactions, miner, timestamp
- Real-time block updates via WebSocket
- Block search by height or hash

**Routes:**
- `GET /blocks` - List all blocks
- `GET /block/<block_number>` - View specific block details

#### 2. **Transaction Explorer**
- View all transactions with filtering
- Transaction details with inputs/outputs
- Transaction status and confirmations
- Transaction search by hash

**Routes:**
- `GET /transactions` - List all transactions
- `GET /tx/<tx_hash>` - View transaction details

#### 3. **Address Explorer**
- Address balance and transaction history
- Real-time balance updates from blockchain node
- Address activity tracking
- QR code generation for addresses

**Routes:**
- `GET /address/<address>` - View address details
- `GET /addresses` - List top addresses by balance

#### 4. **Search Functionality**
- Universal search for blocks, transactions, addresses
- Auto-detection of search type
- Instant results

**Route:**
- `GET /search?q=<query>` - Search blockchain

### Advanced Features

#### 5. **Mining Dashboard**
- Current network hashrate
- Mining difficulty
- Block rewards
- Mining pool statistics
- Mining guide for beginners

**Routes:**
- `GET /mining` - Mining dashboard
- `GET /mining/guide` - Mining guide

#### 6. **Staking Dashboard**
- Validator list and statistics
- Staking rewards calculator
- Delegation tracking
- Validator performance metrics

**Routes:**
- `GET /staking` - Staking dashboard
- `GET /api/staking/validators` - List validators
- `GET /api/staking/validator/<address>` - Validator details

#### 7. **Gold-Backed Tokens**
- Gold reserve tracking
- Gold-backed token transfers
- Reserve audit trail
- Real-time gold price integration

**Routes:**
- `GET /gold` - Gold token explorer
- `GET /api/gold/stats` - Gold statistics
- `GET /api/gold/reserves` - Reserve data
- `GET /api/gold/address/<address>` - Address gold holdings

#### 8. **Stock Contracts**
- Synthetic stock token tracking
- Stock price feeds
- Trading volume and market cap
- Stock contract details

**Routes:**
- `GET /stocks` - Stock contracts dashboard
- `GET /stocks/<ticker>` - Stock contract details
- `GET /stocks/guide` - Stock contracts guide

#### 9. **Governance**
- Proposal listing and voting
- Governance statistics
- Proposal details and vote tracking
- Community participation metrics

**Routes:**
- `GET /governance` - Governance dashboard
- `GET /api/governance/proposals` - List proposals
- `GET /api/governance/proposal/<id>` - Proposal details
- `GET /api/governance/proposal/<id>/votes` - Proposal votes

#### 10. **Bridge Explorer**
- Cross-chain transfer tracking
- Bridge validator monitoring
- Transfer status and confirmations
- Bridge statistics

**Routes:**
- `GET /bridge` - Bridge explorer
- `GET /api/bridge/transfers` - List transfers
- `GET /api/bridge/validators` - Bridge validators
- `GET /api/bridge/stats` - Bridge statistics

### Analytics & Tools

#### 11. **Network Analytics**
- Transaction volume charts
- Address growth metrics
- Network health monitoring
- Historical data analysis

**Routes:**
- `GET /analytics` - Analytics dashboard
- `GET /api/analytics` - Analytics data

#### 12. **Charts & Visualizations**
- Price charts
- Hashrate charts
- Transaction volume
- Network growth

**Route:**
- `GET /charts` - Charts page

#### 13. **Transaction Graph**
- Visual transaction flow
- Address relationship mapping
- Interactive graph visualization

**Routes:**
- `GET /graph` - Transaction graph viewer
- `GET /api/graph/address/<address>` - Address graph data
- `GET /api/graph/transaction/<tx_hash>` - Transaction graph

#### 14. **Gas Tracker**
- Current gas prices
- Gas usage statistics
- Fee estimation

**Routes:**
- `GET /gas` - Gas tracker
- `GET /api/gas` - Gas data

#### 15. **Transaction Decoder**
- Decode raw transactions
- Parse transaction data
- Verify signatures

**Routes:**
- `GET /decoder` - Transaction decoder
- `POST /api/decode` - Decode transaction

#### 16. **Transaction Simulator**
- Estimate transaction fees
- Simulate transaction execution
- Test transactions before sending

**Routes:**
- `GET /simulator` - Transaction simulator
- `POST /api/simulator/estimate` - Estimate transaction

### Developer Tools

#### 17. **REST API**
- Complete blockchain data access
- JSON responses
- Rate limiting
- API key management

**Route:**
- `GET /api` - API documentation

#### 18. **WebSocket API**
- Real-time block updates
- Real-time transaction notifications
- Live network statistics

**Events:**
- `connect` - Client connected
- `disconnect` - Client disconnected
- `new_block` - New block mined
- `new_transaction` - New transaction

#### 19. **Data Export**
- Export transactions (CSV, JSON)
- Export blocks
- Export UTXO set
- Bulk data export

**Routes:**
- `GET /api/export/transactions` - Export transactions
- `GET /api/export/blocks` - Export blocks
- `GET /api/export/utxo` - Export UTXO set
- `GET /api/export/all` - Export all data

#### 20. **Webhooks**
- Event notifications
- Custom webhook endpoints
- Webhook management

**Routes:**
- `GET /webhooks` - Webhooks page
- `GET /api/webhooks` - List webhooks
- `POST /api/webhooks` - Create webhook
- `DELETE /api/webhooks/<id>` - Delete webhook

#### 21. **API Key Management**
- Generate API keys
- Validate API keys
- Key usage tracking

**Routes:**
- `GET /api/keys` - API keys page
- `POST /api/keys/generate` - Generate key
- `POST /api/keys/validate` - Validate key

### Additional Features

#### 22. **Portfolio Tracker**
- Track multiple addresses
- Portfolio value calculation
- Transaction history

**Routes:**
- `GET /portfolio` - Portfolio page
- `GET /api/portfolio` - Portfolio data

#### 23. **Price Alerts**
- Set price alerts
- Email/webhook notifications
- Alert management

**Routes:**
- `GET /alerts` - Price alerts page
- `GET /api/alerts` - Alerts data

#### 24. **Pending Transactions**
- View mempool transactions
- Transaction priority
- Fee estimation

**Routes:**
- `GET /pending` - Pending transactions
- `GET /api/pending` - Pending data

#### 25. **Network Health**
- Node status
- Connection monitoring
- Network diagnostics

**Routes:**
- `GET /health` - Network health page
- `GET /api/health` - Health data
- `GET /api/connection-status` - Connection status
- `GET /healthz` - Health check endpoint

#### 26. **Miner Downloads**
- Download mining software
- Platform-specific builds
- Algorithm-specific miners

**Routes:**
- `GET /downloads` - Downloads page
- `GET /api/downloads/<platform>/<algorithm>` - Download miner

---

## All Pages & Routes

### Complete Route List

#### Main Pages (26 pages)

1. **`/`** - Homepage with stats and recent blocks/transactions
2. **`/blocks`** - Block list with pagination
3. **`/block/<block_number>`** - Individual block details
4. **`/transactions`** - Transaction list with filtering
5. **`/tx/<tx_hash>`** - Individual transaction details with traceability
6. **`/address/<address>`** - Address details, balance, and transaction history
7. **`/addresses`** - Top addresses by balance
8. **`/search`** - Universal search (blocks, transactions, addresses)
9. **`/mining`** - Mining dashboard with hashrate and difficulty
10. **`/mining/guide`** - Mining guide for beginners
11. **`/staking`** - Staking dashboard with validators
12. **`/validators`** - Validator list
13. **`/gold`** - Gold-backed token explorer
14. **`/stocks`** - Stock contracts dashboard
15. **`/stocks/<ticker>`** - Individual stock contract details
16. **`/stocks/guide`** - Stock contracts guide
17. **`/governance`** - Governance proposals and voting
18. **`/bridge`** - Cross-chain bridge explorer
19. **`/charts`** - Network charts and visualizations
20. **`/analytics`** - Network analytics dashboard
21. **`/graph`** - Transaction graph visualization
22. **`/gas`** - Gas tracker
23. **`/price`** - Price tracker
24. **`/portfolio`** - Portfolio tracker
25. **`/pending`** - Pending transactions (mempool)
26. **`/decoder`** - Transaction decoder
27. **`/simulator`** - Transaction simulator
28. **`/alerts`** - Price alerts
29. **`/wallet`** - Web wallet interface
30. **`/webhooks`** - Webhook management
31. **`/downloads`** - Miner downloads
32. **`/forum`** - Forum redirect
33. **`/health`** - Network health dashboard
34. **`/api`** - API documentation

#### API Endpoints (47 endpoints)

**Statistics & Info:**
1. **`GET /api/stats`** - Network statistics
2. **`GET /api/v1/stats`** - Network statistics (v1)
3. **`GET /api/network-info`** - Network information
4. **`GET /api/connection-status`** - Node connection status
5. **`GET /api/health`** - Health check with details
6. **`GET /healthz`** - Simple health check
7. **`GET /healthcheck`** - Health check alias

**Blocks & Transactions:**
8. **`GET /api/blocks`** - Block list API
9. **`GET /api/transactions`** - Transaction list API
10. **`GET /api/pending`** - Pending transactions API
11. **`GET /api/filter/transactions`** - Filter transactions

**Charts & Analytics:**
12. **`GET /api/charts`** - Chart data
13. **`GET /api/analytics`** - Analytics data
14. **`GET /api/hashrate`** - Hashrate data

**Mining & Staking:**
15. **`GET /api/staking/validators`** - Validator list
16. **`GET /api/staking/validator/<address>`** - Validator details
17. **`GET /api/staking/stats`** - Staking statistics

**Gold-Backed Tokens:**
18. **`GET /api/gold/stats`** - Gold token statistics
19. **`GET /api/gold/reserves`** - Gold reserves
20. **`GET /api/gold/transfers`** - Gold token transfers
21. **`GET /api/gold/address/<address>`** - Address gold holdings

**Governance:**
22. **`GET /api/governance/proposals`** - Proposal list
23. **`GET /api/governance/proposal/<id>`** - Proposal details
24. **`GET /api/governance/proposal/<id>/votes`** - Proposal votes

**Bridge:**
25. **`GET /api/bridge/transfers`** - Bridge transfers
26. **`GET /api/bridge/validators`** - Bridge validators
27. **`GET /api/bridge/stats`** - Bridge statistics

**Transaction Graph:**
28. **`GET /api/graph/address/<address>`** - Address transaction graph
29. **`GET /api/graph/transaction/<tx_hash>`** - Transaction graph

**Tools:**
30. **`GET /api/gas`** - Gas prices
31. **`GET /api/price`** - Price data
32. **`GET /api/portfolio`** - Portfolio data
33. **`GET /api/alerts`** - Price alerts
34. **`GET /api/wallet`** - Wallet API
35. **`POST /api/decode`** - Decode transaction
36. **`POST /api/simulator/estimate`** - Estimate transaction

**Data Export:**
37. **`GET /api/export/transactions`** - Export transactions (CSV/JSON)
38. **`GET /api/export/blocks`** - Export blocks
39. **`GET /api/export/utxo`** - Export UTXO set
40. **`GET /api/export/all`** - Export all data

**API Keys:**
41. **`GET /api/keys`** - API keys page
42. **`POST /api/keys/generate`** - Generate API key
43. **`POST /api/keys/validate`** - Validate API key

**Webhooks:**
44. **`GET /api/webhooks`** - List webhooks
45. **`POST /api/webhooks`** - Create webhook
46. **`DELETE /api/webhooks/<id>`** - Delete webhook

**Downloads:**
47. **`GET /api/downloads/<platform>/<algorithm>`** - Download miner

### Transaction Traceability

**Feature:** Every transaction includes traceability validation

**How it works:**
1. Each transaction is validated for traceability when stored
2. Transaction details page shows traceability status
3. Database stores `traceability_valid` boolean flag
4. API returns traceability status in transaction objects

**Transaction Object Fields:**
```json
{
  "hash": "tx_hash",
  "from": "sender_address",
  "to": "recipient_address",
  "amount": 50.0,
  "fee": 0.001,
  "timestamp": 1234567890,
  "block_number": 100,
  "confirmations": 6,
  "traceability_valid": true,
  "is_coinbase": false,
  "memo": "optional memo",
  "inputs": [...],
  "outputs": [...]
}
```

**Traceability Validation:**
- ✅ Valid: Transaction inputs can be traced to previous outputs
- ❌ Invalid: Transaction has invalid or missing input references
- Coinbase transactions are always valid (no inputs required)

**Where to see it:**
- Transaction details page: `/tx/<tx_hash>`
- API response: `GET /api/transactions`
- Database: `transactions.traceability_valid` column

---

## Architecture

### Technology Stack

- **Backend:** Python 3.12, Flask
- **Database:** SQLite (with PostgreSQL support)
- **Real-time:** Flask-SocketIO (WebSocket)
- **Frontend:** HTML5, CSS3, JavaScript, Bootstrap 5
- **Charts:** Chart.js
- **Icons:** Font Awesome 6
- **Deployment:** Vercel (serverless), Railway

### Key Features

1. **Real-time Updates** - WebSocket for live block and transaction updates
2. **Transaction Traceability** - Full UTXO tracking and validation
3. **Multi-Network Support** - Testnet and Mainnet with separate databases
4. **Advanced Search** - Search blocks, transactions, and addresses
5. **Data Export** - Export data in CSV and JSON formats
6. **API Access** - Complete REST API with rate limiting
7. **Webhook Support** - Event notifications for integrations
8. **Mobile Responsive** - Works on all devices

### Database Schema

The explorer uses a comprehensive database schema with the following tables:

1. **blocks** - Block data
2. **transactions** - Transaction data
3. **addresses** - Address balances and activity
4. **validators** - Staking validators
5. **delegations** - Staking delegations
6. **gold_reserves** - Gold-backed token reserves
7. **gold_transfers** - Gold token transfers
8. **stock_contracts** - Stock contract data
9. **governance_proposals** - Governance proposals
10. **governance_votes** - Proposal votes
11. **bridge_transfers** - Cross-chain transfers
12. **bridge_validators** - Bridge validators
13. **webhooks** - Webhook configurations
14. **api_keys** - API key management

### Network Configuration

The explorer supports both testnet and mainnet:

```python
NETWORK_CONFIG = {
    'testnet': {
        'address_prefix': 'tGXC',
        'block_time': 120,  # 2 minutes
        'block_reward': 50.0,
        'rpc_port': 8332,
        'railway_url': 'https://gxc-chain112-blockchain-node-production.up.railway.app',
        'local_url': 'http://localhost:8332',
    },
    'mainnet': {
        'address_prefix': 'GXC',
        'block_time': 600,  # 10 minutes
        'block_reward': 50.0,
        'rpc_port': 8332,
        'local_url': 'http://localhost:8332',
    }
}
```

---

## Installation

### Prerequisites

- Python 3.12+
- pip
- SQLite3 (or PostgreSQL)

### Local Setup

```bash
# Clone repository
git clone https://github.com/philani1H/GXC-goldxcoin-blockchain-.git
cd GXC-goldxcoin-blockchain-

# Install dependencies
pip install -r requirements.txt

# Set environment variables
export GXC_NETWORK=testnet
export BLOCKCHAIN_NODE_URL=http://localhost:8332

# Run explorer
cd web
python3 blockchain_explorer.py
```

The explorer will be available at `http://localhost:5001`

### Docker Setup

```bash
# Build image
docker build -t gxc-explorer -f Dockerfile .

# Run container
docker run -p 5001:5001 \
  -e GXC_NETWORK=testnet \
  -e BLOCKCHAIN_NODE_URL=http://localhost:8332 \
  gxc-explorer
```

---

## Configuration

### Environment Variables

| Variable | Description | Default |
|----------|-------------|---------|
| `GXC_NETWORK` | Network type (testnet/mainnet) | `testnet` |
| `BLOCKCHAIN_NODE_URL` | Blockchain node RPC URL | Railway URL |
| `DATABASE_PATH` | Database file path | `/tmp/gxc_explorer_{network}.db` |
| `FORUM_URL` | Forum URL | `http://localhost:5000` |
| `PORT` | Server port | `5001` |

### Network Selection

Set the network via environment variable:

```bash
# Testnet
export GXC_NETWORK=testnet

# Mainnet
export GXC_NETWORK=mainnet
```

### Node Connection

The explorer automatically tries to connect to:
1. Railway node (if available)
2. Local node (fallback)

Connection status is displayed in the UI.

---

## API Reference

### Base URL

- **Production:** `https://gxc-explorer.vercel.app/api`
- **Local:** `http://localhost:5001/api`

### Authentication

Most endpoints are public. API keys are required for:
- Webhook management
- High-frequency requests
- Data export

### Endpoints

#### Network Statistics

```http
GET /api/stats
```

Returns network statistics including block count, transaction count, total supply, etc.

**Response:**
```json
{
  "blocks": 1000,
  "transactions": 5000,
  "addresses": 250,
  "hashrate": 1000000,
  "difficulty": 0.1,
  "total_supply": 50000.0
}
```

#### Block Data

```http
GET /api/blocks?page=1&limit=20
```

Returns paginated list of blocks.

**Parameters:**
- `page` - Page number (default: 1)
- `limit` - Results per page (default: 20, max: 100)

#### Transaction Data

```http
GET /api/transactions?page=1&limit=20
```

Returns paginated list of transactions.

#### Address Balance

```http
GET /api/address/<address>/balance
```

Returns address balance from blockchain node.

#### Staking Validators

```http
GET /api/staking/validators
```

Returns list of active validators.

#### Gold Statistics

```http
GET /api/gold/stats
```

Returns gold-backed token statistics.

#### Governance Proposals

```http
GET /api/governance/proposals
```

Returns list of governance proposals.

#### Bridge Transfers

```http
GET /api/bridge/transfers
```

Returns cross-chain bridge transfers.

### WebSocket Events

Connect to WebSocket for real-time updates:

```javascript
const socket = io('https://gxc-explorer.vercel.app');

socket.on('new_block', (data) => {
  console.log('New block:', data);
});

socket.on('new_transaction', (data) => {
  console.log('New transaction:', data);
});
```

---

## Developer Guide

### Adding New Features

1. **Add Route Handler**

```python
@app.route('/my-feature')
def my_feature():
    return render_template('my_feature.html')
```

2. **Add API Endpoint**

```python
@app.route('/api/my-feature')
def api_my_feature():
    data = get_my_data()
    return jsonify(data)
```

3. **Add Database Table**

```python
cursor.execute('''
    CREATE TABLE IF NOT EXISTS my_table (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        data TEXT NOT NULL,
        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
    )
''')
```

### RPC Integration

Use the `rpc_call` function to interact with the blockchain node:

```python
# Get block by number
block = rpc_call('getblock', [block_number])

# Get transaction
tx = rpc_call('gettransaction', [tx_hash])

# Get address balance
balance = rpc_call('getbalance', [address])
```

### Database Queries

```python
import sqlite3

conn = sqlite3.connect(DATABASE_PATH)
cursor = conn.cursor()

# Query data
cursor.execute('SELECT * FROM blocks ORDER BY height DESC LIMIT 10')
blocks = cursor.fetchall()

# Insert data
cursor.execute('INSERT INTO addresses (address, balance) VALUES (?, ?)', 
               (address, balance))
conn.commit()

conn.close()
```

### WebSocket Events

Emit events to connected clients:

```python
from flask_socketio import emit

# Emit to all clients
socketio.emit('new_block', {'height': 100, 'hash': '0x...'})

# Emit to specific room
socketio.emit('address_update', {'address': 'GXC...', 'balance': 100.0}, 
              room=address)
```

---

## Deployment

### Vercel Deployment

The explorer is configured for Vercel serverless deployment.

**Configuration:** `vercel.json`

```json
{
  "version": 2,
  "builds": [
    {
      "src": "api/explorer.py",
      "use": "@vercel/python"
    }
  ],
  "routes": [
    {
      "src": "/(.*)",
      "dest": "api/explorer.py"
    }
  ]
}
```

**Deploy:**

```bash
# Install Vercel CLI
npm install -g vercel

# Deploy
vercel --prod
```

### Railway Deployment

```bash
# Install Railway CLI
npm install -g @railway/cli

# Login
railway login

# Deploy
railway up
```

### Environment Variables (Production)

Set these in your deployment platform:

```bash
GXC_NETWORK=testnet
BLOCKCHAIN_NODE_URL=https://gxc-chain112-blockchain-node-production.up.railway.app
DATABASE_PATH=/tmp/gxc_explorer_testnet.db
```

---

## Troubleshooting

### Address Shows 0 Balance

**Issue:** Explorer shows 0 balance for addresses with funds.

**Solution:** Ensure the explorer is connecting to the correct node port (8332).

**Fix Applied:** Updated `NETWORK_CONFIG` to use port 8332 for both testnet and mainnet.

### Node Connection Failed

**Issue:** Cannot connect to blockchain node.

**Solution:** 
1. Check node is running: `curl http://localhost:8332`
2. Verify `BLOCKCHAIN_NODE_URL` environment variable
3. Check firewall settings

### Database Locked

**Issue:** SQLite database locked error.

**Solution:** Increase timeout in connection:

```python
conn = sqlite3.connect(DATABASE_PATH, timeout=30.0)
```

---

## Support

- **GitHub:** [https://github.com/philani1H/GXC-goldxcoin-blockchain-](https://github.com/philani1H/GXC-goldxcoin-blockchain-)
- **Documentation:** See repository docs
- **Issues:** Report on GitHub Issues

---

## License

MIT License - See LICENSE file for details

---

## Changelog

### v1.0.0 (2024-12-24)

**Initial Release:**
- Complete blockchain explorer with 34 pages
- 47 API endpoints
- Real-time updates via WebSocket
- Transaction traceability validation
- Advanced analytics and charts
- Gold-backed tokens support
- Stock contracts integration
- Governance dashboard
- Bridge explorer
- Staking dashboard
- Developer API with webhooks
- Data export (CSV/JSON)
- Mobile responsive design

**Bug Fixes:**
- Fixed address balance display (port 8332 configuration)
- Fixed explorer stats to show real blockchain data
- Fixed total supply display (31M GXC max cap)
- Added circulating supply (2,500 GXC current)
- Fixed GUI miner transaction display
- Fixed node priority (local first)

**Features:**
- Transaction traceability on all transactions
- UTXO tracking and validation
- Multi-network support (testnet/mainnet)
- Real-time circulating supply from blockchain
- Percentage of max supply display
- Complete API documentation
- Webhook event notifications
- API key management
