# 🌐 Public Mining Pool Endpoints - Third-Party Miner Access

## ✅ All Pools Now Have Public Endpoints

All mining pools are configured with **public endpoints** for third-party miners to connect and test mine.

---

## 📋 Public Pool Endpoints

### 1. **General Pool** (All Algorithms)
```
Stratum Endpoint: stratum+tcp://pool.gxc.network:3333
Dashboard:        https://pool.gxc.network
Algorithms:       SHA-256, Ethash, GXHash
Port:             3333 (Stratum), 6000 (Dashboard)
Host:             0.0.0.0 (Public access)
```

### 2. **GPU Pool** (Ethash)
```
Stratum Endpoint: stratum+tcp://gpu-pool.gxc.network:3334
Dashboard:        https://gpu-pool.gxc.network
Algorithm:        Ethash (GPU optimized)
Port:             3334 (Stratum), 6001 (Dashboard)
Host:             0.0.0.0 (Public access)
```

### 3. **ASIC Pool** (SHA-256)
```
Stratum Endpoint: stratum+tcp://asic-pool.gxc.network:3335
Dashboard:        https://asic-pool.gxc.network
Algorithm:        SHA-256 (ASIC optimized)
Port:             3335 (Stratum), 6002 (Dashboard)
Host:             0.0.0.0 (Public access)
```

---

## 🔧 Configuration

### Environment Variables

You can override pool endpoints with environment variables:

```bash
# General Pool
export GENERAL_POOL_STRATUM_URL=stratum+tcp://your-domain.com:3333
export GENERAL_POOL_DASHBOARD_URL=https://your-domain.com
export GENERAL_POOL_STRATUM_PORT=3333
export GENERAL_POOL_DASHBOARD_PORT=6000

# GPU Pool
export GPU_POOL_STRATUM_URL=stratum+tcp://gpu.your-domain.com:3334
export GPU_POOL_DASHBOARD_URL=https://gpu.your-domain.com
export GPU_POOL_STRATUM_PORT=3334
export GPU_POOL_DASHBOARD_PORT=6001

# ASIC Pool
export ASIC_POOL_STRATUM_URL=stratum+tcp://asic.your-domain.com:3335
export ASIC_POOL_DASHBOARD_URL=https://asic.your-domain.com
export ASIC_POOL_STRATUM_PORT=3335
export ASIC_POOL_DASHBOARD_PORT=6002

# All pools listen on 0.0.0.0 (public) by default
export GENERAL_POOL_HOST=0.0.0.0
export GPU_POOL_HOST=0.0.0.0
export ASIC_POOL_HOST=0.0.0.0
```

---

## 🚀 How Third-Party Miners Connect

### Step 1: Get Pool Endpoint

Choose the pool based on your miner type:
- **CPU/General:** `stratum+tcp://pool.gxc.network:3333`
- **GPU:** `stratum+tcp://gpu-pool.gxc.network:3334`
- **ASIC:** `stratum+tcp://asic-pool.gxc.network:3335`

### Step 2: Configure Miner

**Example: CGMiner**
```bash
cgminer \
  --stratum-url stratum+tcp://pool.gxc.network:3333 \
  --user tGXC9fab7317231b966af85ac453e168c0932 \
  --pass x
```

**Example: BFGMiner**
```bash
bfgminer \
  --stratum-url stratum+tcp://pool.gxc.network:3333 \
  --user tGXC9fab7317231b966af85ac453e168c0932 \
  --pass x
```

**Example: NiceHash Miner**
```
1. Add pool
2. URL: stratum+tcp://pool.gxc.network:3333
3. Username: tGXC9fab7317231b966af85ac453e168c0932
4. Password: x
```

### Step 3: Start Mining

Your miner will:
1. ✅ Connect to public pool endpoint
2. ✅ Subscribe to mining jobs
3. ✅ Submit shares
4. ✅ Receive rewards

---

## 📊 Pool Features for Third Parties

### ✅ What Third-Party Miners Get

1. **Public Access** ✅
   - No localhost restrictions
   - Accessible from anywhere
   - Standard Stratum protocol

2. **Real-Time Stats** ✅
   - Dashboard access
   - Share tracking
   - Hash rate monitoring

3. **Rewards** ✅
   - Proportional payouts
   - Block reward sharing
   - Transparent statistics

4. **Support** ✅
   - Multiple algorithms
   - GPU/ASIC optimized pools
   - Standard Stratum protocol

---

## 🔒 Security & Access

### Public Access
- ✅ Pools listen on `0.0.0.0` (all interfaces)
- ✅ No IP restrictions
- ✅ Standard Stratum authentication

### Authentication
- **Username:** GXC wallet address (e.g., `tGXC9fab7317231b966af85ac453e168c0932`)
- **Password:** Can be anything (usually `x`)
- **Validation:** Address format checked

---

## 🌐 Deployment Options

### Option 1: Deploy to Railway

Create `railway-pool.toml`:
```toml
[build]
builder = "NIXPACKS"
buildCommand = "pip install -r requirements.txt"

[deploy]
startCommand = "python mining_pool/general_pool.py"
restartPolicyType = "ON_FAILURE"

[deploy.env]
GENERAL_POOL_STRATUM_URL = "stratum+tcp://your-pool.up.railway.app:3333"
GENERAL_POOL_DASHBOARD_URL = "https://your-pool.up.railway.app"
BLOCKCHAIN_NODE_URL = "https://gxc-chain112-blockchain-node-production.up.railway.app"
```

### Option 2: Deploy to VPS/Server

```bash
# Install dependencies
pip install -r requirements.txt

# Start pool (runs in background)
nohup python mining_pool/general_pool.py > pool.log 2>&1 &

# Or use systemd service
sudo systemctl start gxc-pool
```

### Option 3: Docker Deployment

```dockerfile
FROM python:3.12
WORKDIR /app
COPY mining_pool/ /app/
RUN pip install -r requirements.txt
EXPOSE 3333 6000
CMD ["python", "general_pool.py"]
```

---

## 📝 Public Endpoint URLs

### For Documentation/Sharing

**General Pool:**
```
Stratum: stratum+tcp://pool.gxc.network:3333
Web:     https://pool.gxc.network
```

**GPU Pool:**
```
Stratum: stratum+tcp://gpu-pool.gxc.network:3334
Web:     https://gpu-pool.gxc.network
```

**ASIC Pool:**
```
Stratum: stratum+tcp://asic-pool.gxc.network:3335
Web:     https://asic-pool.gxc.network
```

---

## ✅ Summary

**Status:** ✅ **All pools configured with public endpoints**

**Features:**
- ✅ Public Stratum endpoints (no localhost)
- ✅ Accessible from anywhere
- ✅ Standard Stratum protocol
- ✅ Third-party miner support
- ✅ Environment variable configuration
- ✅ Multiple pool types (General/GPU/ASIC)

**Third-party miners can now connect and test mine!** 🚀
