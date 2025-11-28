# GXC Testnet Quick Start Guide

Welcome to the GXC Blockchain Testnet! This guide will help you get started with testing all components of the GXC ecosystem.

## 🚀 Quick Start

### Prerequisites

1. **Windows 10/11** with PowerShell 5.1+
2. **Python 3.8+** installed and in PATH
3. **Git** (if cloning the repository)

### Starting the Testnet

1. Open PowerShell as Administrator (optional but recommended)
2. Navigate to the GXC blockchain directory:
   ```powershell
   cd c:\Users\phila\GXC-goldxcoin-blockchain-
   ```

3. Run the testnet startup script:
   ```powershell
   .\scripts\start-testnet.ps1
   ```

4. On first run, dependencies will be installed automatically. To skip dependency installation on subsequent runs:
   ```powershell
   .\scripts\start-testnet.ps1 --skip-deps
   ```

### Stopping the Testnet

To stop all testnet services:
```powershell
.\scripts\stop-testnet.ps1
```

## 🌐 Testnet Services

Once started, the following services will be available:

| Service | URL | Purpose |
|---------|-----|---------|
| **Blockchain Explorer** | http://localhost:13000 | View blocks, transactions, and network stats |
| **Wallet API** | http://localhost:15000 | Create wallets and manage transactions |
| **Market Maker** | http://localhost:14000 | View gold and stock market data |
| **RPC Node** | http://localhost:18332 | JSON-RPC interface for blockchain interaction |

## 📖 Testing Scenarios

### 1. Create a Wallet

**Using the Web Interface:**
1. Open http://localhost:15000 in your browser
2. Click "Register" to create a new account
3. Enter username, email, and password
4. Once registered, log in to access your wallet

**Using cURL:**
```powershell
# Register a new user
curl -X POST http://localhost:15000/api/v1/register `
  -H "Content-Type: application/json" `
  -d '{\"username\":\"testuser\",\"email\":\"test@example.com\",\"password\":\"testpass123\"}'

# Login
curl -X POST http://localhost:15000/api/v1/login `
  -H "Content-Type: application/json" `
  -d '{\"username\":\"testuser\",\"password\":\"testpass123\"}'

# Create a wallet (use the token from login response)
curl -X POST http://localhost:15000/api/v1/wallets/create `
  -H "Authorization: Bearer YOUR_TOKEN_HERE" `
  -H "Content-Type: application/json" `
  -d '{\"wallet_name\":\"My Test Wallet\",\"password\":\"walletpass123\"}'
```

### 2. Explore the Blockchain

1. Open the Blockchain Explorer: http://localhost:13000
2. Browse recent blocks and transactions
3. Use the search function to find specific blocks, transactions, or addresses
4. View network statistics and mining information

### 3. Test Transactions

```powershell
# Get wallet balance
curl -X GET http://localhost:15000/api/v1/wallets/YOUR_WALLET_ID/balance `
  -H "Authorization: Bearer YOUR_TOKEN"

# Create a transaction
curl -X POST http://localhost:15000/api/v1/transactions/create `
  -H "Authorization: Bearer YOUR_TOKEN" `
  -H "Content-Type: application/json" `
  -d '{\"from\":\"GXC_ADDRESS\",\"to\":\"GXC_ADDRESS\",\"amount\":10.0}'
```

### 4. Mining (Testnet)

The testnet is configured with lower difficulty for easier mining:

```powershell
# Enable mining in the config file (already enabled by default)
# Mining will start automatically when the blockchain node runs

# Check mining status via RPC
curl -X POST http://localhost:18332 `
  -H "Content-Type: application/json" `
  -d '{\"method\":\"getmininginfo\",\"params\":[],\"id\":1}'
```

### 5. Staking

```powershell
# Stake coins (minimum 10 GXC on testnet)
curl -X POST http://localhost:18332 `
  -H "Content-Type: application/json" `
  -d '{\"method\":\"stakecoin\",\"params\":[\"YOUR_ADDRESS\",10,90],\"id\":1}'

# Check staking status
curl -X POST http://localhost:18332 `
  -H "Content-Type: application/json" `
  -d '{\"method\":\"getstakinginfo\",\"params\":[],\"id\":1}'
```

## 🔧 Configuration

### Testnet Parameters

The testnet uses different parameters than mainnet:

| Parameter | Mainnet | Testnet |
|-----------|---------|---------|
| **P2P Port** | 8333 | 18333 |
| **RPC Port** | 8332 | 18332 |
| **REST API Port** | 8080 | 18080 |
| **WebSocket Port** | 8081 | 18081 |
| **Block Time** | 150 seconds | 60 seconds |
| **Initial Difficulty** | 1.0 | 0.1 |
| **Min Stake** | 100 GXC | 10 GXC |
| **Proposal Fee** | 100 GXC | 1 GXC |

### Configuration File

The testnet configuration is located at:
```
c:\Users\phila\GXC-goldxcoin-blockchain-\config\gxc-testnet.conf
```

You can modify this file to customize testnet behavior.

## 📂 Data Directories

All testnet data is stored separately from mainnet:

```
gxc_testnet_data/          # Blockchain data
logs/                      # Service logs
testnet_backups/          # Wallet backups
pids/                     # Process IDs
```

## 🐛 Troubleshooting

### Services Won't Start

1. Check if ports are already in use:
   ```powershell
   netstat -ano | findstr "13000 15000 14000 18332"
   ```

2. Check log files for errors:
   ```powershell
   Get-Content logs\explorer_testnet.log -Tail 50
   Get-Content logs\wallet_api_testnet.log -Tail 50
   Get-Content logs\market_maker_testnet.log -Tail 50
   ```

3. Ensure Python dependencies are installed:
   ```powershell
   python -m pip install flask flask-cors flask-socketio bcrypt pyjwt cryptography qrcode requests yfinance numpy pillow
   ```

### Can't Access Services

1. Check if services are running:
   ```powershell
   Get-Process python
   ```

2. Verify firewall settings allow local connections

3. Try accessing services from localhost explicitly:
   ```
   http://127.0.0.1:13000
   ```

### Database Errors

1. Stop all services:
   ```powershell
   .\scripts\stop-testnet.ps1
   ```

2. Delete testnet data (WARNING: This will reset the testnet):
   ```powershell
   Remove-Item -Recurse -Force gxc_testnet_data
   ```

3. Restart services:
   ```powershell
   .\scripts\start-testnet.ps1
   ```

## 📊 Monitoring

### Real-Time Logs

To monitor services in real-time, open separate PowerShell windows:

```powershell
# Monitor Explorer
Get-Content logs\explorer_testnet.log -Wait

# Monitor Wallet API
Get-Content logs\wallet_api_testnet.log -Wait

# Monitor Market Maker
Get-Content logs\market_maker_testnet.log -Wait
```

### Health Checks

```powershell
# Check if services are responding
curl http://localhost:13000
curl http://localhost:15000/api/v1/health
curl http://localhost:14000/api/health
```

## 🎯 Testing Checklist

- [ ] All services start without errors
- [ ] Blockchain Explorer loads and displays data
- [ ] Can register and login to Wallet API
- [ ] Can create a new wallet
- [ ] Can view wallet balance
- [ ] Can create and submit transactions
- [ ] Transactions appear in the blockchain
- [ ] Mining is functional
- [ ] Staking works correctly
- [ ] Market Maker shows price data
- [ ] All API endpoints respond correctly

## 🔗 Useful Links

- **Documentation**: [README.md](../README.md)
- **Build Status**: [BUILD_STATUS.md](../BUILD_STATUS.md)
- **Configuration Example**: [config/gxc.conf.example](../config/gxc.conf.example)
- **Ecosystem Guide**: [ECOSYSTEM_GUIDE.md](../ECOSYSTEM_GUIDE.md)

## 💬 Support

If you encounter issues:

1. Check the logs in `logs/` directory
2. Review the configuration in `config/gxc-testnet.conf`
3. Ensure all prerequisites are installed
4. Try resetting the testnet data (see Troubleshooting)

## 🚀 Next Steps

After familiarizing yourself with the testnet:

1. **Develop Applications**: Use the API endpoints to build dApps
2. **Test Mining**: Try different mining configurations
3. **Experiment with Staking**: Test validator functionality
4. **Cross-Chain Testing**: Test bridge operations (when enabled)
5. **Performance Testing**: Stress test the network with multiple transactions

---

**Happy Testing!** 🎉

For more information, refer to the main [README.md](../README.md) file.
