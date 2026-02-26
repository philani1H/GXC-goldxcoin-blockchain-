# GoldXCoin Railway Deployment Guide

## ⚠️ CRITICAL: Database Persistence Required

**Without persistent storage, ALL blockchain data (blocks, transactions, balances) will be LOST on every deployment/restart.**

## Problem: Ephemeral Filesystems

Railway (and most PaaS platforms) use **ephemeral filesystems** by default:
- All data stored in `/app`, `./gxc_data`, or `/tmp` is **wiped on restart**
- Your mined blocks, transaction history, and wallet balances will **disappear**
- The node will create a fresh genesis block on each restart

## Solution: Mount a Persistent Volume

### Option 1: Railway Volumes (Recommended)

1. **Create a Persistent Volume** in Railway dashboard:
   ```
   Service Settings → Volumes → Add Volume
   ```

2. **Configure mount point:**
   ```
   Mount Path: /data
   Size: 10GB (minimum)
   ```

3. **Set environment variable:**
   ```bash
   GXC_DATA_DIR=/data/blockchain
   ```

4. **Update your start command:**
   ```bash
   mkdir -p /data/blockchain && ./gxc-node --data-dir=/data/blockchain
   ```

### Option 2: External Database (PostgreSQL/MySQL)

Railway provides managed databases that persist across deployments:

1. Add PostgreSQL service to your Railway project
2. Set connection environment variables
3. Modify `Database.cpp` to use PostgreSQL instead of LevelDB (requires code changes)

## Configuration

### Environment Variables

Set these in Railway dashboard:

```bash
# REQUIRED: Persistent storage path
GXC_DATA_DIR=/data/blockchain

# Optional: Wallet persistence
GXC_WALLET_PRIVATE_KEY=your_private_key_here

# Network
GXC_NETWORK=mainnet  # or testnet
GXC_PORT=8333
GXC_RPC_PORT=8332
```

### Config File

Create `config.json` in your persistent volume:

```json
{
  "data_dir": "/data/blockchain",
  "network": "mainnet",
  "rpc_port": 8332,
  "p2p_port": 8333,
  "min_peers": 3,
  "max_peers": 20
}
```

## Verification

After deployment, check the logs for:

✅ **Good Signs:**
```
✅ Successfully loaded 1234 blocks from persistent database
   Database path: /data/blockchain
```

❌ **Bad Signs (Data Loss!):**
```
⚠️  WARNING: Database path appears to be on EPHEMERAL storage!
   Path: ./gxc_data
   ALL BLOCKCHAIN DATA WILL BE LOST ON RESTART!
```

## Data Backup

### Manual Backup

```bash
# SSH into Railway container
railway run bash

# Backup blockchain data
tar -czf blockchain-backup-$(date +%Y%m%d).tar.gz /data/blockchain

# Download from Railway
railway files download blockchain-backup-*.tar.gz
```

### Automated Backup

Add to your deployment script:

```bash
#!/bin/bash
# Pre-deployment backup
if [ -d "/data/blockchain" ]; then
    tar -czf /data/backup-$(date +%Y%m%d-%H%M%S).tar.gz /data/blockchain
    # Keep only last 7 backups
    ls -t /data/backup-*.tar.gz | tail -n +8 | xargs rm -f
fi

# Start node
./gxc-node --data-dir=/data/blockchain
```

## Migration from Ephemeral to Persistent Storage

If you've been running without persistent storage and have lost data:

1. **Stop the service**
2. **Add persistent volume** (see Option 1 above)
3. **Set `GXC_DATA_DIR=/data/blockchain`**
4. **Restart** - a fresh genesis block will be created on persistent storage
5. **Re-mine blocks** or sync from network peers

## Troubleshooting

### "Database contains X blocks but loading failed"

This error means the database file exists but is corrupted:

```bash
# Option 1: Try to repair (if implemented)
./gxc-node --repair-db --data-dir=/data/blockchain

# Option 2: Restore from backup
cd /data
tar -xzf blockchain-backup-20260226.tar.gz

# Option 3: Start fresh (LOSES ALL DATA)
rm -rf /data/blockchain/*
./gxc-node --data-dir=/data/blockchain
```

### "ALL BLOCKCHAIN DATA WILL BE LOST ON RESTART"

You're running on ephemeral storage. Follow Option 1 above to add a persistent volume.

### Wallet Balance Shows Zero After Restart

This means the database was reset. Check:

1. Is `GXC_DATA_DIR` pointing to a persistent volume?
2. Are blocks being loaded from database? (check logs)
3. Is the volume properly mounted? (`df -h | grep /data`)

## Cost Considerations

**Railway Pricing (as of 2024):**
- Persistent volumes: ~$0.25/GB/month
- 10GB volume: ~$2.50/month
- **Worth it to avoid losing your blockchain data!**

## Production Checklist

- [ ] Persistent volume mounted at `/data` or `/mnt/blockchain`
- [ ] `GXC_DATA_DIR` environment variable set
- [ ] Backup script configured
- [ ] Logs show "Successfully loaded X blocks from persistent database"
- [ ] Test restart: verify blocks persist across restarts
- [ ] Monitor volume usage (blockchain grows over time)

## Support

- GitHub Issues: https://github.com/your-repo/GXC-goldxcoin-blockchain/issues
- Documentation: `/docs/WHITEPAPER.md`
- Database schema: See `COMPREHENSIVE_WHITEPAPER.md` Section 26
