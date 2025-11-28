# 🆓 Free Deployment Guide - Deploy GXC Node for Mining

## 🎯 Best FREE Options (Ranked)

### **Option 1: Oracle Cloud Free Tier ⭐ BEST FREE OPTION**

**100% FREE Forever** - No credit card required for basic tier

**What You Get:**
- ✅ 2 ARM-based VMs (Ampere A1) - 4 OCPUs total, 24GB RAM total
- ✅ 200GB storage
- ✅ 10TB outbound data transfer/month
- ✅ Always free (not a trial!)
- ✅ Public IP address included

**Perfect for:**
- Running blockchain node 24/7
- Mining operations
- Multiple services

**Sign Up:**
1. Go to: https://www.oracle.com/cloud/free/
2. Click "Start for Free"
3. Create account (no credit card needed for basic tier)
4. Verify email

**Deploy Steps:**

```bash
# Step 1: Create Instance
# - Go to Oracle Cloud Console
# - Compute > Instances > Create Instance
# - Select: "Always Free Eligible" shape
# - Choose: VM.Standard.A1.Flex (4 OCPUs, 24GB RAM)
# - OS: Ubuntu 22.04
# - Add SSH key or generate one
# - Create instance

# Step 2: SSH into instance
ssh ubuntu@YOUR-INSTANCE-IP

# Step 3: Install dependencies
sudo apt-get update
sudo apt-get install -y build-essential cmake git libssl-dev libsqlite3-dev

# Step 4: Clone/build your project
git clone YOUR-REPO-URL
cd gxc-blockchain

# Step 5: Build blockchain node
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc) gxc-node

# Step 6: Configure firewall
sudo ufw allow 9333/tcp
sudo ufw allow 8545/tcp
sudo ufw allow 8080/tcp
sudo ufw enable

# Step 7: Start node
cd ..
./build/gxc-node --rpc-port=8545 --rest-port=8080 --network-port=9333
```

**Important:** You need to configure Security List in Oracle Cloud:
- Go to: Networking > Virtual Cloud Networks > Your VCN > Security Lists
- Add Ingress Rules for ports: 9333, 8545, 8080

---

### **Option 2: Local Computer (Your PC) 🖥️**

**100% FREE** - Use your own computer

**Requirements:**
- Computer running 24/7 (or when mining)
- Internet connection
- Port forwarding on router

**Setup Steps:**

#### Step 1: Build Node Locally
```bash
# On Windows
.\build-blockchain.ps1

# On Linux/Mac
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc) gxc-node
```

#### Step 2: Start Node
```bash
# Windows
.\build\Release\gxc-node.exe --rpc-port=8545 --rest-port=8080

# Linux/Mac
./build/gxc-node --rpc-port=8545 --rest-port=8080
```

#### Step 3: Port Forwarding (For External Access)

**On Your Router:**
1. Log into router admin (usually 192.168.1.1)
2. Find "Port Forwarding" or "Virtual Server"
3. Forward these ports to your computer's local IP:
   - Port 9333 → Your PC IP (TCP)
   - Port 8545 → Your PC IP (TCP)
   - Port 8080 → Your PC IP (TCP)

**Find Your Local IP:**
```bash
# Windows
ipconfig
# Look for IPv4 Address (e.g., 192.168.1.100)

# Linux/Mac
ifconfig
# or
ip addr show
```

#### Step 4: Find Your Public IP
```bash
# Visit: https://whatismyipaddress.com
# Or run:
curl ifconfig.me
```

#### Step 5: Connect Miners
```bash
# Use your public IP
python mining/gxhash_miner.py --rpc-url http://YOUR-PUBLIC-IP:8545
```

**Note:** If your IP changes (dynamic IP), use a free Dynamic DNS service:
- No-IP (free): https://www.noip.com
- DuckDNS (free): https://www.duckdns.org

---

### **Option 3: AWS Free Tier (12 Months Free)**

**FREE for 12 months** (then paid)

**What You Get:**
- ✅ 750 hours/month of t2.micro or t3.micro
- ✅ 5GB storage
- ✅ 15GB data transfer

**Sign Up:**
1. Go to: https://aws.amazon.com/free/
2. Create account (credit card required, but won't charge during free tier)
3. Launch EC2 instance (t2.micro)

**Deploy:**
```bash
# Same as Oracle Cloud steps above
# Use Ubuntu 22.04 AMI
```

---

### **Option 4: Google Cloud Free Tier**

**Always Free Tier** (limited resources)

**What You Get:**
- ✅ e2-micro instance (1 vCPU, 1GB RAM) - limited hours
- ✅ 30GB storage
- ✅ 1GB egress/month

**Sign Up:**
1. Go to: https://cloud.google.com/free
2. Create account ($300 free credit for 90 days)
3. Create VM instance

**Note:** e2-micro has limited always-free hours per month

---

### **Option 5: Railway.app Free Tier**

**FREE with limits**

**What You Get:**
- ✅ $5 free credit/month
- ✅ Easy deployment
- ✅ Automatic HTTPS

**Deploy:**
1. Sign up: https://railway.app
2. Connect GitHub repo
3. Deploy (may need Docker setup)

---

## 🚀 Quick Start: Oracle Cloud (Recommended Free Option)

### Complete Setup Script

```bash
#!/bin/bash
# Oracle Cloud Free Tier - Complete Setup

echo "Setting up GXC Node on Oracle Cloud Free Tier..."

# Update system
sudo apt-get update && sudo apt-get upgrade -y

# Install dependencies
sudo apt-get install -y \
    build-essential \
    cmake \
    git \
    libssl-dev \
    libsqlite3-dev \
    curl \
    ufw

# Clone your repository (replace with your repo)
# git clone https://github.com/your-repo/gxc-blockchain.git
# cd gxc-blockchain

# Or if files are already uploaded:
# cd ~/gxc-blockchain

# Build
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc) gxc-node

cd ..

# Create config directory
mkdir -p ~/.gxc
mkdir -p gxc_data

# Create config
cat > ~/.gxc/gxc.conf << EOF
network_port=9333
rpc_port=8545
rest_port=8080
data_dir=./gxc_data
testnet=false
log_level=INFO
EOF

# Setup firewall
sudo ufw allow 22/tcp    # SSH
sudo ufw allow 9333/tcp  # P2P
sudo ufw allow 8545/tcp  # RPC
sudo ufw allow 8080/tcp  # REST
sudo ufw --force enable

# Create systemd service
sudo tee /etc/systemd/system/gxc-node.service > /dev/null << EOF
[Unit]
Description=GXC Blockchain Node
After=network.target

[Service]
Type=simple
User=$USER
WorkingDirectory=$(pwd)
ExecStart=$(pwd)/build/gxc-node --rpc-port=8545 --rest-port=8080 --network-port=9333
Restart=always
RestartSec=10
StandardOutput=journal
StandardError=journal

[Install]
WantedBy=multi-user.target
EOF

# Enable and start
sudo systemctl daemon-reload
sudo systemctl enable gxc-node
sudo systemctl start gxc-node

echo "✅ Setup complete!"
echo "Check status: sudo systemctl status gxc-node"
echo "View logs: sudo journalctl -u gxc-node -f"
echo ""
echo "Your node IP: $(curl -s ifconfig.me)"
echo "RPC URL: http://$(curl -s ifconfig.me):8545"
```

---

## 🔧 Oracle Cloud Security List Configuration

**CRITICAL:** You must configure Security Lists for external access!

1. **Go to Oracle Cloud Console**
2. **Networking > Virtual Cloud Networks**
3. **Click your VCN**
4. **Click "Security Lists"**
5. **Click "Default Security List"**
6. **Click "Add Ingress Rules"**

Add these rules:

| Source Type | Source CIDR | IP Protocol | Destination Port Range | Description |
|------------|-------------|-------------|----------------------|-------------|
| CIDR | 0.0.0.0/0 | TCP | 9333 | GXC P2P Network |
| CIDR | 0.0.0.0/0 | TCP | 8545 | GXC RPC API |
| CIDR | 0.0.0.0/0 | TCP | 8080 | GXC REST API |

7. **Click "Add Ingress Rules"** for each
8. **Save**

---

## 📊 Free Options Comparison

| Option | Cost | RAM | CPU | Storage | Best For |
|--------|------|-----|-----|---------|----------|
| **Oracle Cloud** | FREE Forever | 24GB | 4 ARM cores | 200GB | ⭐ Best overall |
| **Local PC** | FREE | Your RAM | Your CPU | Your disk | Testing/learning |
| **AWS Free Tier** | FREE 12mo | 1GB | 1 vCPU | 5GB | Short-term |
| **Google Cloud** | FREE (limited) | 1GB | 1 vCPU | 30GB | Light usage |
| **Railway** | $5 credit/mo | Varies | Varies | Varies | Easy deploy |

---

## 🎯 Recommended: Oracle Cloud Setup

### Why Oracle Cloud?
- ✅ **Permanent free tier** (not a trial)
- ✅ **Generous resources** (24GB RAM, 4 CPUs)
- ✅ **No credit card required** for basic tier
- ✅ **Perfect for blockchain nodes**
- ✅ **24/7 uptime**

### Step-by-Step Oracle Cloud Setup

1. **Sign Up** (5 minutes)
   - Visit: https://www.oracle.com/cloud/free/
   - Create account
   - Verify email

2. **Create Instance** (10 minutes)
   - Compute > Instances > Create Instance
   - Name: `gxc-node`
   - Image: Ubuntu 22.04
   - Shape: Always Free Eligible (VM.Standard.A1.Flex)
   - OCPUs: 4, Memory: 24GB
   - Add SSH key
   - Create

3. **Configure Security** (5 minutes)
   - Networking > VCN > Security Lists
   - Add rules for ports 9333, 8545, 8080

4. **Deploy Node** (15 minutes)
   - SSH into instance
   - Run setup script above
   - Start node

5. **Start Mining** (2 minutes)
   ```bash
   python mining/gxhash_miner.py --rpc-url http://YOUR-ORACLE-IP:8545
   ```

**Total Time: ~35 minutes**
**Cost: $0.00**

---

## 🔍 Verify Your Free Deployment

### Test Node is Running
```bash
# On server
curl -X POST http://localhost:8545 \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getblockchaininfo","params":[],"id":1}'

# From your computer
curl -X POST http://YOUR-SERVER-IP:8545 \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getblockchaininfo","params":[],"id":1}'
```

### Test REST API
```bash
curl http://YOUR-SERVER-IP:8080/api/v1/blockchain/info
```

### Check Node Status
```bash
# On server
sudo systemctl status gxc-node
sudo journalctl -u gxc-node -f
```

---

## 💡 Tips for Free Deployment

1. **Oracle Cloud:**
   - Use ARM-based instances (always free)
   - Monitor usage in dashboard
   - Set up billing alerts (just in case)

2. **Local Computer:**
   - Use Dynamic DNS for changing IPs
   - Consider running only when mining
   - Use power-saving mode when idle

3. **General:**
   - Start with testnet first
   - Monitor resource usage
   - Keep backups locally

---

## 🆘 Troubleshooting Free Deployments

### Oracle Cloud: Can't Connect
- ✅ Check Security List rules
- ✅ Verify firewall (ufw) is configured
- ✅ Check instance is running
- ✅ Verify public IP is assigned

### Local PC: Port Forwarding Not Working
- ✅ Check router port forwarding rules
- ✅ Verify computer's local IP hasn't changed
- ✅ Check Windows Firewall allows connections
- ✅ Test with localhost first

### Node Not Starting
- ✅ Check logs: `sudo journalctl -u gxc-node -f`
- ✅ Verify build succeeded
- ✅ Check disk space: `df -h`
- ✅ Check memory: `free -h`

---

## 🎉 You're Ready to Mine for FREE!

**Recommended Path:**
1. ✅ Sign up for Oracle Cloud Free Tier
2. ✅ Deploy node (35 minutes)
3. ✅ Configure security lists
4. ✅ Start mining!

**Your free node will be at:**
- RPC: `http://YOUR-ORACLE-IP:8545`
- REST: `http://YOUR-ORACLE-IP:8080`

**Connect miners:**
```bash
python mining/gxhash_miner.py --rpc-url http://YOUR-ORACLE-IP:8545
```

**Happy FREE Mining! ⛏️💰**
