# 🚀 Explorer Deployment Guide

## ✅ Railway URL Configuration

The explorer **uses the Railway URL** for all blockchain connections:

- **Railway Node URL**: `https://gxc-chain112-blockchain-node-production.up.railway.app`
- **Environment Variable**: `BLOCKCHAIN_NODE_URL` or `RAILWAY_NODE_URL`

## 📋 How It Works

### 1. **Code Configuration** (`web/blockchain_explorer.py`)

```python
# Railway Node URL (Testnet)
RAILWAY_NODE_URL = "https://gxc-chain112-blockchain-node-production.up.railway.app"

# Use Railway URL from environment, fallback to Railway URL for production
BLOCKCHAIN_NODE_URL = os.environ.get('BLOCKCHAIN_NODE_URL', 
                                     os.environ.get('RAILWAY_NODE_URL', 
                                                   RAILWAY_NODE_URL))
```

### 2. **Vercel Deployment** (`web/api.py`)

The Vercel entry point automatically sets the Railway URL:

```python
# Set Railway URL environment variable if not set (for Vercel deployment)
if not os.environ.get('BLOCKCHAIN_NODE_URL') and not os.environ.get('RAILWAY_NODE_URL'):
    os.environ['RAILWAY_NODE_URL'] = 'https://gxc-chain112-blockchain-node-production.up.railway.app'
    os.environ['BLOCKCHAIN_NODE_URL'] = 'https://gxc-chain112-blockchain-node-production.up.railway.app'
```

### 3. **Vercel Configuration** (`web/vercel.json`)

The Railway URL is set as an environment variable:

```json
{
  "env": {
    "VERCEL": "1",
    "RAILWAY_NODE_URL": "https://gxc-chain112-blockchain-node-production.up.railway.app",
    "BLOCKCHAIN_NODE_URL": "https://gxc-chain112-blockchain-node-production.up.railway.app"
  }
}
```

## 🚀 Deploy to Vercel

### Option 1: Using Vercel CLI

```bash
cd web
vercel --prod
```

### Option 2: Using Vercel Dashboard

1. Go to [Vercel Dashboard](https://vercel.com)
2. Import your repository
3. Set root directory to `web`
4. Environment variables are already in `vercel.json`, but you can also set them manually:
   - `RAILWAY_NODE_URL` = `https://gxc-chain112-blockchain-node-production.up.railway.app`
   - `BLOCKCHAIN_NODE_URL` = `https://gxc-chain112-blockchain-node-production.up.railway.app`
   - `VERCEL` = `1`
5. Deploy!

## ✅ Verification

After deployment, the explorer will:

1. ✅ Connect to Railway node at `https://gxc-chain112-blockchain-node-production.up.railway.app`
2. ✅ Fetch blocks, transactions, and addresses from the testnet
3. ✅ Display real-time blockchain data
4. ✅ Store data in SQLite database (or PostgreSQL if configured)

## 🔍 Test the Deployment

1. Visit your deployed explorer URL
2. Check the browser console for any errors
3. Verify blocks are loading from the Railway node
4. Test transaction lookups

## 📝 Notes

- **SocketIO**: Real-time features may have limitations on Vercel (serverless). HTTP routes work perfectly.
- **Database**: Uses SQLite by default (`/tmp/gxc_explorer.db` on Vercel). For persistence, configure PostgreSQL.
- **Railway URL**: All RPC calls go to the Railway testnet node automatically.

## 🎯 Summary

**Yes, the explorer uses the Railway URL!** ✅

- Configured in code with fallback
- Set in Vercel environment variables
- Automatically connects to Railway testnet node
- Ready for production deployment
