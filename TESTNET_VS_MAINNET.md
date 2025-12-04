# GXC Testnet vs Mainnet - Complete Comparison

## Quick Summary

| Feature | TESTNET | MAINNET |
|---------|---------|---------|
| **Purpose** | Testing & Development | Real Production |
| **Coins** | Worthless (test coins) | Real value |
| **Difficulty** | 0.1 (very easy) | 1000.0 (production) |
| **Address Prefix** | `tGXC...` | `GXC...` |
| **Database** | `blockchain_testnet.db` | `blockchain_mainnet.db` |
| **Network Port** | 18333 | 8333 |
| **RPC Port** | 18332 | 8332 |
| **Genesis Block** | Separate | Separate |
| **Security Level** | Relaxed | Full |

## Visual Comparison

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           TESTNET vs MAINNET                                 │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│   TESTNET (Development Network)           MAINNET (Production Network)      │
│   ═════════════════════════════           ═════════════════════════════     │
│                                                                              │
│   Purpose: Testing & Development          Purpose: Real Transactions        │
│   ┌─────────────────────────┐             ┌─────────────────────────┐       │
│   │  👨‍💻 Developers           │             │  💰 Real Users           │       │
│   │  🧪 Testing new features │             │  🏦 Real Value           │       │
│   │  🐛 Bug hunting          │             │  🔒 Maximum Security     │       │
│   │  📚 Learning             │             │  ⚡ Production Ready     │       │
│   └─────────────────────────┘             └─────────────────────────┘       │
│                                                                              │
│   Difficulty: 0.1                         Difficulty: 1000.0                │
│   ┌─────────────────────────┐             ┌─────────────────────────┐       │
│   │  ⚡ Very easy mining     │             │  🔨 Real mining effort  │       │
│   │  🚀 Fast block times    │             │  ⏱️ 10 second blocks    │       │
│   │  📦 Quick testing       │             │  🛡️ Attack resistant    │       │
│   └─────────────────────────┘             └─────────────────────────┘       │
│                                                                              │
│   Address: tGXC9fab7317...               Address: GXC9fab7317...           │
│   ┌─────────────────────────┐             ┌─────────────────────────┐       │
│   │  t = testnet prefix     │             │  No prefix = mainnet    │       │
│   │  Cannot be used on main │             │  Real GXC coins         │       │
│   │  Free to obtain         │             │  Must be mined/bought   │       │
│   └─────────────────────────┘             └─────────────────────────┘       │
│                                                                              │
│   Database: blockchain_testnet.db         Database: blockchain_mainnet.db   │
│   ┌─────────────────────────┐             ┌─────────────────────────┐       │
│   │  Separate chain         │             │  Separate chain         │       │
│   │  Can be reset           │             │  NEVER reset            │       │
│   │  Test data only         │             │  Real transactions      │       │
│   └─────────────────────────┘             └─────────────────────────┘       │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

## Detailed Differences

### 1. Difficulty Settings

```cpp
// From Blockchain.cpp constructor:

if (isTestnet) {
    difficulty = 0.1;   // TESTNET: Very easy - mine blocks in seconds
} else {
    difficulty = 1000.0; // MAINNET: Production difficulty
}
```

**Why Testnet is Easier:**
- Developers can mine blocks quickly for testing
- No need for specialized hardware
- Faster feedback during development
- Can test many scenarios rapidly

**Why Mainnet is Harder:**
- Prevents spam and attacks
- Ensures fair distribution
- Maintains 10-second block target
- Security Engine fully active

### 2. Address Formats

```
TESTNET ADDRESS:
┌──────────────────────────────────────────────────┐
│  tGXC9fab7317231b966af85ac453e168c0932           │
│  ↑                                               │
│  └─ "t" prefix = TESTNET                        │
│                                                  │
│  • Clearly identifies test coins                 │
│  • Cannot be sent to mainnet addresses           │
│  • Prevents accidental mixing of networks        │
└──────────────────────────────────────────────────┘

MAINNET ADDRESS:
┌──────────────────────────────────────────────────┐
│  GXC9fab7317231b966af85ac453e168c0932            │
│  ↑                                               │
│  └─ No "t" prefix = MAINNET                     │
│                                                  │
│  • Real GXC coins with value                    │
│  • Cannot be sent to testnet addresses           │
│  • Protected by full security                   │
└──────────────────────────────────────────────────┘
```

### 3. Network Ports

```
TESTNET:                          MAINNET:
═════════                         ═════════

P2P Port:  18333                  P2P Port:  8333
RPC Port:  18332                  RPC Port:  8332

┌─────────────────┐               ┌─────────────────┐
│   Testnet       │               │   Mainnet       │
│   Nodes         │               │   Nodes         │
│                 │               │                 │
│  :18333 ←──────→│               │  :8333 ←──────→ │
│  :18332 (RPC)   │               │  :8332 (RPC)    │
└─────────────────┘               └─────────────────┘

• Different ports prevent accidental connections
• Testnet nodes only connect to other testnet nodes
• Mainnet nodes only connect to other mainnet nodes
```

### 4. Database Separation

```
./gxc_data/
├── blockchain_testnet.db    ← TESTNET chain
│   ├── Test genesis block
│   ├── Test transactions
│   ├── Test validators
│   └── Can be deleted/reset
│
└── blockchain_mainnet.db    ← MAINNET chain
    ├── Real genesis block
    ├── Real transactions
    ├── Real validators
    └── NEVER delete!
```

### 5. Security Engine Behavior

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    SECURITY ENGINE BY NETWORK                                │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│   TESTNET Security:                       MAINNET Security:                  │
│   ═════════════════                       ═════════════════                  │
│                                                                              │
│   • Security Engine: ACTIVE               • Security Engine: ACTIVE         │
│   • Attack Detection: ON                  • Attack Detection: ON            │
│   • Difficulty Adjustment: RELAXED        • Difficulty Adjustment: STRICT   │
│   • Emission Guard: ACTIVE                • Emission Guard: ACTIVE          │
│   • Fee Surge Guard: ACTIVE               • Fee Surge Guard: ACTIVE         │
│                                                                              │
│   Key Difference:                                                           │
│   ┌─────────────────────────────────────────────────────────────────────┐   │
│   │  TESTNET: Base difficulty 0.1 allows quick mining                   │   │
│   │  MAINNET: Base difficulty 1000.0 requires real work                 │   │
│   │                                                                      │   │
│   │  Security Engine adjusts FROM these base values                     │   │
│   │  Both networks are protected - just with different baselines        │   │
│   └─────────────────────────────────────────────────────────────────────┘   │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 6. Genesis Block Differences

```cpp
// TESTNET Genesis:
{
    "height": 0,
    "hash": "0000000000000000000000000000000000000000000000000000000000000000",
    "miner": "tGXC9fab7317231b966af85ac453e168c0932",  // Testnet address
    "reward": 50.0,
    "network": "testnet"
}

// MAINNET Genesis:
{
    "height": 0,
    "hash": "0000000000000000000000000000000000000000000000000000000000000000",
    "miner": "GXC9fab7317231b966af85ac453e168c0932",   // Mainnet address
    "reward": 50.0,
    "network": "mainnet"
}
```

## How to Start Each Network

### Testnet (for Development)
```bash
# Start testnet node
./gxc-node --testnet

# Connect CLI to testnet
./gxc-cli --testnet getblockchaininfo

# Mine on testnet (easy difficulty)
./gxc-miner --testnet --address tGXCyouraddress...
```

### Mainnet (for Production)
```bash
# Start mainnet node (default)
./gxc-node

# Connect CLI to mainnet
./gxc-cli getblockchaininfo

# Mine on mainnet (full difficulty)
./gxc-miner --address GXCyouraddress...
```

## When to Use Each

### Use TESTNET When:
- 🧪 Developing new features
- 🐛 Testing bug fixes
- 📚 Learning how the blockchain works
- 🔄 Testing wallet integration
- ⚡ Need fast block times
- 💸 Need free test coins
- 🧹 Want to reset and start fresh

### Use MAINNET When:
- 💰 Handling real value
- 🏦 Running production services
- 👥 Serving real users
- 🔒 Maximum security needed
- 📊 Real network statistics
- 🌐 Connecting to real miners/stakers

## Coin Value Comparison

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           COIN VALUES                                        │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│   TESTNET COINS (tGXC):                   MAINNET COINS (GXC):              │
│                                                                              │
│   Value: $0.00 (worthless)               Value: Real market price           │
│   Purpose: Testing only                  Purpose: Real transactions         │
│   Obtained: Free faucets, easy mining    Obtained: Mining, staking, buying  │
│   Transferable: No                       Transferable: Yes, to exchanges    │
│                                                                              │
│   ┌───────────────────────┐              ┌───────────────────────┐          │
│   │  1000 tGXC = $0       │              │  1 GXC = Real Value   │          │
│   │  Cannot be exchanged  │              │  Trade on exchanges   │          │
│   │  Cannot be sold       │              │  Store of value       │          │
│   └───────────────────────┘              └───────────────────────┘          │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

## Summary Table

| Aspect | TESTNET | MAINNET |
|--------|---------|---------|
| **Start Command** | `./gxc-node --testnet` | `./gxc-node` |
| **Difficulty** | 0.1 | 1000.0 |
| **Block Time** | ~1 second (easy) | ~10 seconds |
| **Address Prefix** | `tGXC` | `GXC` |
| **P2P Port** | 18333 | 8333 |
| **RPC Port** | 18332 | 8332 |
| **Database** | `blockchain_testnet.db` | `blockchain_mainnet.db` |
| **Coin Value** | $0 (test coins) | Real value |
| **Can Reset** | Yes | **NEVER** |
| **Security Engine** | Active (relaxed base) | Active (strict base) |
| **Purpose** | Development/Testing | Production |

## Production Checklist

Before going to MAINNET, ensure:

- [ ] All tests pass (63/63 ✅)
- [ ] Security Engine integrated ✅
- [ ] Testnet testing complete
- [ ] No hardcoded testnet values
- [ ] Proper address validation
- [ ] Database backups configured
- [ ] Monitoring in place
- [ ] Security audit completed
