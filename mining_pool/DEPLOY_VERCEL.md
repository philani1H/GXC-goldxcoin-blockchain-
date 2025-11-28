# Deploy Mining Pool to Vercel

## Quick Deployment Guide

### Prerequisites

1. **Vercel Account**: Sign up at [vercel.com](https://vercel.com)
2. **Vercel CLI**: Install globally
   ```bash
   npm install -g vercel
   ```
3. **Git Repository**: Your code should be in a Git repository

---

## Deployment Steps

### Option 1: Deploy via Vercel CLI (Recommended)

1. **Navigate to mining pool directory**:
   ```bash
   cd mining_pool
   ```

2. **Login to Vercel**:
   ```bash
   vercel login
   ```

3. **Deploy**:
   ```bash
   vercel
   ```
   
   Follow the prompts:
   - Set up and deploy? **Yes**
   - Which scope? **Your account**
   - Link to existing project? **No**
   - Project name? **gxc-mining-pool** (or your choice)
   - Directory? **./** (current directory)
   - Override settings? **No**

4. **Set Environment Variables**:
   ```bash
   vercel env add BLOCKCHAIN_NODE_URL
   # Enter your blockchain RPC URL when prompted
   # Example: http://your-blockchain-node:8545
   ```

5. **Redeploy with environment variables**:
   ```bash
   vercel --prod
   ```

### Option 2: Deploy via Vercel Dashboard

1. **Go to [vercel.com/dashboard](https://vercel.com/dashboard)**

2. **Click "Add New Project"**

3. **Import your Git repository** (GitHub, GitLab, or Bitbucket)

4. **Configure Project**:
   - **Framework Preset**: Other
   - **Root Directory**: `mining_pool`
   - **Build Command**: (leave empty)
   - **Output Directory**: (leave empty)

5. **Add Environment Variables**:
   - Go to **Settings** → **Environment Variables**
   - Add the following:
     ```
     BLOCKCHAIN_NODE_URL = http://your-blockchain-node:8545
     DATABASE_PATH = /tmp/pool.db
     VERCEL = 1
     ```

6. **Deploy**: Click "Deploy"

---

## Environment Variables

### Required Variables

| Variable | Description | Example |
|----------|-------------|---------|
| `BLOCKCHAIN_NODE_URL` | Your blockchain RPC endpoint | `http://localhost:8545` |
| `DATABASE_PATH` | Database file path (Vercel uses `/tmp`) | `/tmp/pool.db` |
| `VERCEL` | Set to `1` to enable Vercel mode | `1` |

### Optional Variables

| Variable | Description | Default |
|----------|-------------|---------|
| `POOL_TYPE` | Pool type identifier | `general` |
| `POOL_NAME` | Pool name | `pool` |
| `ALGORITHM` | Mining algorithm | `all` |

---

## Post-Deployment

### 1. Verify Deployment

Visit your Vercel URL (e.g., `https://gxc-mining-pool.vercel.app`) and check:
- ✅ Dashboard loads
- ✅ API endpoints work (`/api/stats`)
- ✅ All pages are accessible

### 2. Test API Endpoints

```bash
# Get pool statistics
curl https://your-pool.vercel.app/api/stats

# Get miners list
curl https://your-pool.vercel.app/api/miners

# Get blocks
curl https://your-pool.vercel.app/api/blocks
```

### 3. Custom Domain (Optional)

1. Go to **Settings** → **Domains**
2. Add your custom domain (e.g., `pool.gxc.network`)
3. Follow DNS configuration instructions

---

## Deploying Multiple Pools

To deploy ASIC, GPU, and General pools separately:

### ASIC Pool

1. Create new Vercel project: `gxc-asic-pool`
2. Update `api.py` to import from `asic_pool`:
   ```python
   from asic_pool import app
   ```
3. Set environment variable: `POOL_TYPE=asic`
4. Deploy

### GPU Pool

1. Create new Vercel project: `gxc-gpu-pool`
2. Update `api.py` to import from `gpu_pool`:
   ```python
   from gpu_pool import app
   ```
3. Set environment variable: `POOL_TYPE=gpu`
4. Deploy

### General Pool

1. Create new Vercel project: `gxc-general-pool`
2. Use existing `api.py` (imports from `general_pool`)
3. Set environment variable: `POOL_TYPE=general`
4. Deploy

---

## Important Notes

### Database Persistence

⚠️ **Vercel's `/tmp` directory is ephemeral** - data will be lost on serverless function restarts.

**Solutions:**
1. **Use External Database**: 
   - PostgreSQL (Vercel Postgres, Supabase)
   - MySQL (PlanetScale)
   - MongoDB (MongoDB Atlas)

2. **Use Vercel KV** (Redis) for caching

3. **Use External Storage** for persistent data

### Stratum Server Limitation

⚠️ **Vercel serverless functions cannot run persistent TCP servers** (Stratum protocol).

**Solutions:**
1. **Run Stratum server separately** on a VPS or dedicated server
2. **Use Vercel only for web dashboard** and API
3. **Connect Stratum server to same database** as web dashboard

### Recommended Architecture

```
┌─────────────────┐
│  Vercel (Web)   │  ← Dashboard, API, Pages
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  External DB    │  ← PostgreSQL, MySQL, etc.
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  VPS (Stratum)  │  ← TCP Server for Miners
└─────────────────┘
```

---

## Troubleshooting

### Issue: "Module not found"

**Solution**: Ensure all dependencies are in `requirements.txt`

### Issue: "Database locked"

**Solution**: Vercel's `/tmp` has limitations. Use external database.

### Issue: "Function timeout"

**Solution**: Increase timeout in `vercel.json`:
```json
{
  "functions": {
    "api.py": {
      "maxDuration": 30
    }
  }
}
```

### Issue: "Templates not found"

**Solution**: Ensure `templates/` directory is included in deployment (check `.vercelignore`)

---

## Production Checklist

- [ ] Environment variables configured
- [ ] External database set up (if needed)
- [ ] Custom domain configured
- [ ] SSL certificate active
- [ ] Monitoring set up
- [ ] Error logging configured
- [ ] Backup strategy in place

---

## Support

For issues or questions:
- Check [Vercel Documentation](https://vercel.com/docs)
- Review [MINING_POOL_DEVELOPMENT_GUIDE.md](../MINING_POOL_DEVELOPMENT_GUIDE.md)
- Check pool logs in Vercel dashboard

---

**Last Updated**: 2024

