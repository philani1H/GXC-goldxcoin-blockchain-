# GXC Blockchain Explorer - Complete Documentation
## Based on Actual Blockchain Implementation

---

## Table of Contents
1. [What the Explorer Does](#what-the-explorer-does)
2. [Blockchain Data Available](#blockchain-data-available)
3. [RPC Methods (116 Total)](#rpc-methods)
4. [Explorer Features](#explorer-features)
5. [Installation & Setup](#installation--setup)
6. [User Guide](#user-guide)
7. [API Integration](#api-integration)
8. [Architecture](#architecture)

---

## What the Explorer Does

The GXC Blockchain Explorer is a web interface that:
1. **Connects** to GXC blockchain node via RPC (port 8332/8545)
2. **Fetches** blockchain data using 116 RPC methods
3. **Displays** data in user-friendly web pages
4. **Updates** in real-time via WebSocket
5. **Provides** search, filtering, and analytics

**Explorer does NOT:**
- Store blockchain data (reads from node)
- Mine blocks
- Create transactions
- Hold private keys

---

## Blockchain Data Available

### From Blockchain Node (src/Blockchain.cpp)

**Core Data Structures:**
```cpp
class Blockchain {
    std::vector<Block> chain;                    // All blocks
    std::vector<Transaction> pendingTransactions; // Mempool
    std::unordered_map<string, TransactionOutput> utxoSet; // UTXO
    std::vector<Validator> validators;            // Validators
    std::unordered_map<string, StockContract> stockContracts; // Stocks
    std::unordered_map<string, GoldToken> goldTokens; // Gold tokens
    std::unique_ptr<SecurityEngine> securityEngine; // AI security
    std::unique_ptr<StakingPool> stakingPool;     // Staking data
}
```

### Block Data
```cpp
class Block {
    uint32_t index;           // Block height
    string previousHash;      // Previous block hash
    uint64_t timestamp;       // Unix timestamp
    string hash;              // Block hash
    uint32_t nonce;           // Mining nonce
    double difficulty;        // Mining difficulty
    string minerAddress;      // Miner who found block
    BlockType type;           // POW or POS
    vector<Transaction> transactions; // All transactions
    string merkleRoot;        // Merkle tree root
}
```

### Transaction Data
```cpp
class Transaction {
    string hash;              // Transaction hash
    vector<TransactionInput> inputs;   // Inputs (UTXOs spent)
    vector<TransactionOutput> outputs; // Outputs (new UTXOs)
    uint64_t timestamp;       // Unix timestamp
    TransactionType type;     // NORMAL, STAKE, UNSTAKE, COINBASE, REWARD
    double fee;               // Transaction fee
    string signature;         // ECDSA signature
}
```

### Validator Data
```cpp
class Validator {
    string address;           // Validator address
    double stakeAmount;       // Amount staked
    uint32_t stakingDays;     // Lock period
    uint64_t registrationTime; // When registered
    bool isActive;            // Currently active
    uint32_t blocksProduced;  // Blocks mined
    uint32_t missedBlocks;    // Blocks missed
    double totalRewards;      // Total earned
    string name;              // Display name (optional)
    string logo;              // Logo URL (optional)
    string bio;               // Description (optional)
    string website;           // Website URL (optional)
    string contact;           // Contact info (optional)
    double commissionRate;    // Commission 0-20%
}
```

---

## RPC Methods

### Blockchain Queries (15 methods)
```
getblockchaininfo    - Get blockchain status
getblockcount        - Get current height
getdifficulty        - Get mining difficulty
getbestblockhash     - Get latest block hash
getinfo              - Get node info
getchaintips         - Get chain tips
getmempoolinfo       - Get mempool status
getrawmempool        - Get mempool transactions
gettxoutsetinfo      - Get UTXO set info
verifychain          - Verify blockchain
getblockstats        - Get block statistics
getchaintxstats      - Get transaction statistics
getnetworkinfo       - Get network info
getpeerinfo          - Get peer info
getconnectioncount   - Get peer count
```

### Block Queries (10 methods)
```
getblock             - Get block by hash
getblockbynumber     - Get block by height
getblockhash         - Get hash by height
getblockheader       - Get block header
getblocktemplate     - Get mining template
getlatestblock       - Get latest block
getblocktransactions - Get block TXs
getblocktxs          - Get block TXs (alias)
gxc_getBlockByNumber - Get block (GXC format)
gxc_getLatestBlock   - Get latest (GXC format)
```

### Transaction Queries (20 methods)
```
gettransaction       - Get TX by hash
gettx                - Get TX (alias)
getrawtransaction    - Get raw TX
gxc_getTransaction   - Get TX (GXC format)
listtransactions     - List address TXs
getaddresstransactions - Get address TXs
getaddresshistory    - Get address history
getaddresstxids      - Get address TX IDs
searchrawtransactions - Search TXs
gettransactions      - Get multiple TXs
getblocktransactions - Get block TXs
gxc_getBlockTransactions - Get block TXs (GXC)
sendrawtransaction   - Broadcast TX
sendtoaddress        - Send coins
createtransaction    - Create TX
signtransaction      - Sign TX
decoderawtransaction - Decode TX
decodescript         - Decode script
gettxout             - Get TX output
gettxoutproof        - Get TX proof
```

### Address/Balance (15 methods)
```
getbalance           - Get address balance
getpendingbalance    - Get pending balance
gxc_getBalance       - Get balance (GXC format)
getaddressbalance    - Get address balance
getaccountbalance    - Get account balance
getnewaddress        - Generate new address
validateaddress      - Validate address format
getaddressinfo       - Get address info
listaddresses        - List all addresses
getaddressutxos      - Get address UTXOs
listunspent          - List unspent outputs
listaccounts         - List accounts
getaccount           - Get account
setaccount           - Set account
getreceivedbyaddress - Get received amount
```

### Validator/Staking (12 methods)
```
registervalidator    - Register as validator
getvalidators        - List all validators
listvalidators       - List validators (alias)
getvalidatorinfo     - Get validator details
stake                - Stake coins
unstake              - Unstake coins
getstakinginfo       - Get staking info
liststakes           - List all stakes
getvalidatorstats    - Get validator stats
selectvalidator      - Select next validator
getvalidatorperformance - Get performance
calculatestakereward - Calculate rewards
```

### Mining (10 methods)
```
getmininginfo        - Get mining stats
getnetworkhashps     - Get network hashrate
submitblock          - Submit mined block
getblocktemplate     - Get mining template
prioritisetransaction - Prioritize TX
getwork              - Get work (deprecated)
getminingalgorithm   - Get current algorithm
setminingalgorithm   - Set mining algorithm
estimatesmartfee     - Estimate fee
estimatefee          - Estimate fee (simple)
```

### Stock Contracts (8 methods)
```
liststockcontracts   - List all stocks
getstockcontract     - Get stock details
createstockcontract  - Create stock
updatestockprice     - Update price
buystocktoken        - Buy stock token
sellstocktoken       - Sell stock token
getstockbalance      - Get stock balance
liststocktransactions - List stock TXs
```

### Gold Tokens (6 methods)
```
listgoldtokens       - List gold tokens
getgoldtoken         - Get gold token
creategoldtoken      - Create gold token
redeemgoldtoken      - Redeem gold token
getgoldreserves      - Get gold reserves
getgoldprice         - Get gold price
```

### Cross-Chain Bridge (5 methods)
```
bridgetransfer       - Transfer cross-chain
getbridgestatus      - Get bridge status
listbridgetransactions - List bridge TXs
getbridgebalance     - Get bridge balance
estimatebridgefee    - Estimate bridge fee
```

### Governance (5 methods)
```
createproposal       - Create proposal
voteproposal         - Vote on proposal
listproposals        - List proposals
getproposal          - Get proposal details
getproposalvotes     - Get proposal votes
```

### Utility (10 methods)
```
help                 - List all methods
stop                 - Stop node
ping                 - Ping node
uptime               - Get uptime
getmemoryinfo        - Get memory usage
logging              - Control logging
echo                 - Echo test
validateaddress      - Validate address
verifymessage        - Verify signature
signmessage          - Sign message
```

---

## Explorer Features

### 1. Home Dashboard
**URL:** `/`

**Displays:**
- Current block height
- Latest blocks (10 most recent)
- Recent transactions (20 most recent)
- Network hashrate
- Mining difficulty
- Active validators count
- Total supply
- Mempool size
- Network status

**Data Sources:**
```javascript
// Fetches from node every 10 seconds
getblockchaininfo()  // Height, difficulty, supply
getlatestblock()     // Latest blocks
listtransactions()   // Recent TXs
getmininginfo()      // Hashrate
getmempoolinfo()     // Mempool
listvalidators()     // Validator count
```

### 2. Block Explorer
**URL:** `/blocks` or `/block/<number>`

**Block List Page:**
- Paginated list of all blocks
- Columns: Height, Hash, Time, Miner, TXs, Reward
- Filter by date range
- Sort by height/time
- Search by height/hash

**Block Detail Page:**
- Complete block information
- All transactions in block
- Block type (PoW/PoS)
- Mining algorithm used
- Previous/Next navigation
- Raw block data (JSON)

**Data Sources:**
```javascript
getblock(height)           // Block details
getblocktransactions(height) // All TXs
getblockstats(height)      // Statistics
```

### 3. Transaction Explorer
**URL:** `/transactions` or `/tx/<hash>`

**Transaction List:**
- Real-time transaction feed
- Filter by type (NORMAL, STAKE, UNSTAKE, COINBASE, REWARD)
- Filter by date range
- Filter by amount
- Search by hash

**Transaction Detail:**
- Complete TX information
- Inputs and outputs
- Confirmations
- Block inclusion
- Fee paid
- Transaction type
- Traceability chain
- Raw TX data (JSON)

**Data Sources:**
```javascript
gettransaction(hash)   // TX details
getrawtransaction(hash) // Raw TX
getblock(height)       // Confirmation count
```

### 4. Address Explorer
**URL:** `/address/<address>`

**Displays:**
- Current balance
- Pending balance
- Total received
- Total sent
- Transaction count
- Transaction history
- UTXO list
- Staking info (if validator)
- Stock token holdings
- Gold token holdings

**Data Sources:**
```javascript
getbalance(address)        // Current balance
getpendingbalance(address) // Pending
listtransactions(address)  // TX history
getaddressutxos(address)   // UTXOs
getstakinginfo(address)    // Staking
getstockbalance(address)   // Stocks
```

### 5. Validator Dashboard
**URL:** `/validators` or `/validator/<address>`

**Validator List:**
- All active validators
- Columns: Name, Address, Stake, Days, Blocks, Rewards, APY, Uptime
- Sort by stake/rewards/uptime
- Filter by active/inactive
- Search by name/address

**Validator Detail:**
- Complete validator profile
- Name, logo, bio, website, contact
- Stake amount and period
- Blocks produced/missed
- Total rewards earned
- Current APY
- Uptime percentage
- Commission rate
- Performance chart
- Delegators list

**Data Sources:**
```javascript
listvalidators()           // All validators
getvalidatorinfo(address)  // Validator details
getvalidatorstats(address) // Statistics
getvalidatorperformance(address) // Performance
```

### 6. Mining Dashboard
**URL:** `/mining`

**Displays:**
- Network hashrate
- Current difficulty
- Next difficulty estimate
- Block time (average)
- Mining algorithms:
  - SHA256 stats
  - GXHASH stats
  - ETHASH stats
- Mining pools list
- Mining calculator
- Mining guide

**Data Sources:**
```javascript
getmininginfo()        // Mining stats
getnetworkhashps()     // Hashrate
getdifficulty()        // Difficulty
getblockstats()        // Block time
```

### 7. Staking Dashboard
**URL:** `/staking`

**Displays:**
- Total staked amount
- Active validators
- Staking APY calculator
- Staking guide
- My stakes (if logged in)
- Staking rewards history
- Lock period countdown

**Data Sources:**
```javascript
listvalidators()       // All validators
getstakinginfo()       // Staking info
calculatestakereward() // Reward calculator
```

### 8. Stock Tokens
**URL:** `/stocks` or `/stock/<ticker>`

**Stock List:**
- All available stock tokens
- Columns: Ticker, Price, 24h Change, Volume, Market Cap
- Real-time price updates
- Trading interface

**Stock Detail:**
- Price chart
- Trading volume
- Order book
- Recent trades
- Buy/Sell interface
- Stock contract details

**Data Sources:**
```javascript
liststockcontracts()   // All stocks
getstockcontract(ticker) // Stock details
liststocktransactions(ticker) // Trades
```

### 9. Gold Tokens
**URL:** `/gold`

**Displays:**
- Gold-backed tokens (GXC-G)
- Current gold price
- Gold reserves
- Redemption interface
- Gold token history

**Data Sources:**
```javascript
listgoldtokens()       // All gold tokens
getgoldprice()         // Current price
getgoldreserves()      // Reserves
```

### 10. Cross-Chain Bridge
**URL:** `/bridge`

**Displays:**
- Supported chains (Ethereum, BSC, Polygon)
- Bridge interface
- Bridge transactions
- Bridge status
- Fee calculator

**Data Sources:**
```javascript
getbridgestatus()      // Bridge status
listbridgetransactions() // Bridge TXs
estimatebridgefee()    // Fee estimate
```

### 11. Governance
**URL:** `/governance`

**Displays:**
- Active proposals
- Voting interface
- Proposal history
- Voting results
- Create proposal form

**Data Sources:**
```javascript
listproposals()        // All proposals
getproposal(id)        // Proposal details
getproposalvotes(id)   // Votes
```

### 12. Search
**URL:** `/search?q=<query>`

**Searches:**
- Blocks (by height or hash)
- Transactions (by hash)
- Addresses
- Validators (by name or address)
- Stock tickers

**Auto-detects:**
- Block number (numeric)
- Block hash (64 hex chars)
- TX hash (64 hex chars)
- Address (starts with GXC/tGXC)
- Stock ticker (uppercase letters)

---

## Installation & Setup

### Prerequisites
```bash
# Python 3.8+
python3 --version

# GXC Node running
./gxc-node --testnet --rpc-port=8545
```

### Install Explorer
```bash
# Clone repository
git clone https://github.com/philani1H/GXC-goldxcoin-blockchain-.git
cd GXC-goldxcoin-blockchain-/web

# Install dependencies
pip3 install flask flask-socketio requests

# Configure
export BLOCKCHAIN_NODE_URL=http://localhost:8545
export GXC_NETWORK=testnet

# Run explorer
python3 blockchain_explorer.py
```

### Access
```
Open browser: http://localhost:5000
```

---

## User Guide

### For General Users

**View Blockchain Data:**
1. Go to homepage
2. See latest blocks and transactions
3. Click on any block/transaction for details
4. Use search bar to find specific data

**Track Address:**
1. Enter address in search
2. View balance and history
3. Bookmark page for monitoring

### For Miners

**Check Mining Stats:**
1. Go to `/mining`
2. See network hashrate and difficulty
3. Use mining calculator
4. Download mining software

**Track Mining Rewards:**
1. Enter your mining address
2. View mined blocks
3. Check total rewards

### For Validators

**View Validators:**
1. Go to `/validators`
2. Browse all validators
3. Compare APY and performance

**Become Validator:**
1. Get minimum 100 GXC
2. Use RPC to register
3. Monitor on validator page

### For Traders

**Trade Stock Tokens:**
1. Go to `/stocks`
2. Select stock ticker
3. View price and chart
4. Use buy/sell interface

---

## API Integration

### Connect to Explorer API
```javascript
const EXPLORER_API = 'http://localhost:5000/api/v1';

// Get blockchain stats
fetch(`${EXPLORER_API}/stats`)
  .then(r => r.json())
  .then(data => console.log(data));
```

### WebSocket Real-time Updates
```javascript
const socket = io('http://localhost:5000');

// Subscribe to new blocks
socket.on('new_block', (block) => {
  console.log('New block:', block);
});

// Subscribe to new transactions
socket.on('new_transaction', (tx) => {
  console.log('New TX:', tx);
});
```

---

## Architecture

### Data Flow
```
Blockchain Node (C++)
       ↓ RPC (116 methods)
Explorer Backend (Python/Flask)
       ↓ HTTP/WebSocket
Explorer Frontend (HTML/JS)
       ↓
User Browser
```

### Components

**1. Blockchain Node**
- Source of truth
- Stores all blockchain data
- Exposes RPC API (port 8332/8545)
- Processes blocks and transactions

**2. Explorer Backend**
- Connects to node via RPC
- Fetches blockchain data
- Caches frequently accessed data
- Provides REST API
- Manages WebSocket connections

**3. Explorer Frontend**
- Displays data in web pages
- Real-time updates via WebSocket
- Search and filtering
- Charts and analytics

### Caching Strategy
- Blocks: Permanent (immutable)
- Transactions: Permanent (immutable)
- Addresses: 60 second TTL
- Stats: 10 second TTL
- Validators: 30 second TTL

---

## Summary

The GXC Blockchain Explorer provides complete visibility into:

✅ **All blockchain data** (blocks, transactions, addresses)
✅ **116 RPC methods** from blockchain node
✅ **Real-time updates** via WebSocket
✅ **Validator system** with profiles and performance
✅ **Mining statistics** and calculator
✅ **Staking dashboard** with APY calculator
✅ **Stock tokens** trading interface
✅ **Gold tokens** management
✅ **Cross-chain bridge** interface
✅ **Governance** proposals and voting
✅ **Complete traceability** of all transactions

**Everything the blockchain provides, the explorer displays!** 🚀
