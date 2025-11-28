# 🏊 GXC Mining Pools - Setup Guide

## ✅ Created Mining Pool Infrastructure

### 1. **Base Pool Class** (`mining_pool/pool_base.py`)
- Full Stratum protocol implementation
- Blockchain RPC integration
- Database for miners, shares, blocks
- Real-time statistics
- Web dashboard

### 2. **ASIC Pool** (`mining_pool/asic_pool.py`)
- **URL**: https://asic-pool.gxc.network
- **Algorithm**: SHA-256
- **Stratum Port**: 3333
- **Dashboard Port**: 5001
- **Pool URL**: `stratum+tcp://asic-pool.gxc.network:3333`

### 3. **GPU Pool** (`mining_pool/gpu_pool.py`)
- **URL**: https://gpu-pool.gxc.network
- **Algorithm**: Ethash
- **Stratum Port**: 4444
- **Dashboard Port**: 5002
- **Pool URL**: `stratum+tcp://gpu-pool.gxc.network:4444`

### 4. **General Pool** (`mining_pool/general_pool.py`)
- **URL**: https://pool.gxc.network
- **Algorithms**: All (SHA-256, Ethash, GXHash)
- **Stratum Port**: 3333
- **Dashboard Port**: 5000
- **Pool URL**: `stratum+tcp://pool.gxc.network:3333`

### 5. **Documentation Site** (`docs_site/docs_app.py`)
- **URL**: https://docs.gxc.network
- **Port**: 5003
- Complete documentation for all GXC features

## 🔧 Features Implemented

### Pool Features:
- ✅ Stratum protocol server (TCP)
- ✅ Miner connection management
- ✅ Share validation
- ✅ Block submission to blockchain
- ✅ Real-time statistics
- ✅ Web dashboard
- ✅ Database tracking
- ✅ Payout system (database ready)

### Blockchain Integration:
- ✅ RPC connection to blockchain node
- ✅ `getblocktemplate` for work distribution
- ✅ `submitblock` for block submission
- ✅ Real-time difficulty adjustment
- ✅ Job generation and distribution

## 🚀 Running Locally

### Start ASIC Pool:
```bash
cd mining_pool
python asic_pool.py
```

### Start GPU Pool:
```bash
cd mining_pool
python gpu_pool.py
```

### Start General Pool:
```bash
cd mining_pool
python general_pool.py
```

### Start Documentation:
```bash
cd docs_site
python docs_app.py
```

## 📋 Configuration

Set environment variables:
```bash
export BLOCKCHAIN_NODE_URL=http://localhost:8545
export DATABASE_PATH=/tmp/pool.db  # For Vercel
```

## 🌐 Deployment to Vercel

Each pool needs separate Vercel deployment:

1. **ASIC Pool**: Deploy `mining_pool/` with subdomain `asic-pool.gxc.network`
2. **GPU Pool**: Deploy `mining_pool/` with subdomain `gpu-pool.gxc.network`
3. **General Pool**: Deploy `mining_pool/` with subdomain `pool.gxc.network`
4. **Documentation**: Deploy `docs_site/` with subdomain `docs.gxc.network`

## 📝 Next Steps

1. Deploy each pool to Vercel with appropriate subdomain
2. Configure DNS records
3. Set environment variables in Vercel dashboard
4. Test Stratum connections
5. Monitor pool performance

## 🔗 Integration

All pools connect to the blockchain RPC at:
- Default: `http://localhost:8545`
- Configurable via `BLOCKCHAIN_NODE_URL` environment variable

Pools automatically:
- Get block templates from blockchain
- Validate shares
- Submit blocks when found
- Track all statistics

