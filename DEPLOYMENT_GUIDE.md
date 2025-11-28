# 🚀 GXC Blockchain Node Deployment Guide

## Where to Deploy the Node

You have several options for deploying your GXC blockchain node:

---

## 1. 🏠 Self-Hosted (Recommended for Full Control)

### Option A: VPS (Virtual Private Server)

**Best Providers:**

#### DigitalOcean
- **Cost:** $6-12/month
- **Specs:** 2GB RAM, 1 CPU, 50GB SSD
- **Setup:** Ubuntu 22.04 LTS
- **URL:** https://www.digitalocean.com

```bash
# After creating droplet
ssh root@your-server-ip

# Install dependencies
apt update && apt upgrade -y
apt install -y python3 python3-pip git cmake build-essential

# Clone repository
git clone https://github.com/YOUR_USERNAME/GXC-goldxcoin-blockchain-.git
cd GXC-goldxcoin-blockchain-

# Install Python dependencies
pip3 install -r requirements.txt

# Start node
./start_complete_ecosystem.sh
```

#### Linode (Akamai)
- **Cost:** $5-10/month
- **Specs:** 1GB RAM, 1 CPU, 25GB SSD
- **Setup:** Ubuntu 22.04 LTS
- **URL:** https://www.linode.com

#### Vultr
- **Cost:** $6-12/month
- **Specs:** 2GB RAM, 1 CPU, 55GB SSD
- **Setup:** Ubuntu 22.04 LTS
- **URL:** https://www.vultr.com

#### Hetzner (Best Value)
- **Cost:** €4-8/month (~$4-9)
- **Specs:** 4GB RAM, 2 CPU, 40GB SSD
- **Setup:** Ubuntu 22.04 LTS
- **URL:** https://www.hetzner.com

### Option B: Dedicated Server

**For Production/High Traffic:**

- **OVH:** https://www.ovh.com
- **Hetzner Dedicated:** https://www.hetzner.com/dedicated-rootserver
- **Cost:** $30-100/month
- **Specs:** 16GB+ RAM, 4+ CPU cores, 500GB+ SSD

### Option C: Home Server

**Requirements:**
- Stable internet connection (100+ Mbps)
- Static IP or Dynamic DNS
- Port forwarding (18332, 8080, 8081)
- 24/7 uptime
- UPS (Uninterruptible Power Supply)

**Hardware:**
- Raspberry Pi 4 (4GB+) - Budget option
- Intel NUC - Mid-range
- Custom PC - High performance

---

## 2. ☁️ Cloud Platforms

### AWS (Amazon Web Services)

**EC2 Instance:**
```
Instance Type: t3.medium
RAM: 4GB
CPU: 2 vCPUs
Storage: 50GB EBS
Cost: ~$30/month
Region: Choose closest to users
```

**Setup:**
```bash
# Launch EC2 instance with Ubuntu 22.04
# SSH into instance
ssh -i your-key.pem ubuntu@ec2-xx-xx-xx-xx.compute.amazonaws.com

# Install and run
git clone YOUR_REPO
cd GXC-goldxcoin-blockchain-
./start_complete_ecosystem.sh
```

**Configure Security Group:**
- Port 18332 (RPC) - Your IP only
- Port 8080 (REST API) - Public
- Port 8081 (WebSocket) - Public
- Port 3333-3335 (Mining pools) - Public
- Port 3000-3001 (Web services) - Public

### Google Cloud Platform (GCP)

**Compute Engine:**
```
Machine Type: e2-medium
RAM: 4GB
CPU: 2 vCPUs
Storage: 50GB SSD
Cost: ~$25/month
```

### Microsoft Azure

**Virtual Machine:**
```
Size: B2s
RAM: 4GB
CPU: 2 vCPUs
Storage: 50GB SSD
Cost: ~$30/month
```

---

## 3. 🐳 Docker Deployment

### Create Dockerfile

```dockerfile
FROM ubuntu:22.04

# Install dependencies
RUN apt-get update && apt-get install -y \
    python3 \
    python3-pip \
    git \
    cmake \
    build-essential \
    && rm -rf /var/lib/apt/lists/*

# Copy application
WORKDIR /app
COPY . /app

# Install Python dependencies
RUN pip3 install -r requirements.txt

# Expose ports
EXPOSE 18332 8080 8081 3000 3001 3333 3334 3335 5000 6000 6001 6002

# Start services
CMD ["./start_complete_ecosystem.sh"]
```

### Build and Run

```bash
# Build image
docker build -t gxc-blockchain .

# Run container
docker run -d \
  --name gxc-node \
  -p 18332:18332 \
  -p 8080:8080 \
  -p 8081:8081 \
  -p 3000:3000 \
  -p 3001:3001 \
  -p 3333:3333 \
  -p 3334:3334 \
  -p 3335:3335 \
  -p 5000:5000 \
  -p 6000:6000 \
  -p 6001:6001 \
  -p 6002:6002 \
  -v gxc-data:/app/gxc_data \
  gxc-blockchain
```

### Docker Compose

```yaml
version: '3.8'

services:
  gxc-node:
    build: .
    ports:
      - "18332:18332"  # Blockchain RPC
      - "8080:8080"    # REST API
      - "8081:8081"    # WebSocket
      - "3000:3000"    # Explorer
      - "3001:3001"    # Forum
      - "3333:3333"    # General Pool
      - "3334:3334"    # GPU Pool
      - "3335:3335"    # ASIC Pool
      - "5000:5000"    # Wallet API
      - "6000:6000"    # General Pool Dashboard
      - "6001:6001"    # GPU Pool Dashboard
      - "6002:6002"    # ASIC Pool Dashboard
    volumes:
      - gxc-data:/app/gxc_data
      - gxc-logs:/app/logs
    restart: unless-stopped

volumes:
  gxc-data:
  gxc-logs:
```

---

## 4. 🌐 Serverless Options (Limited)

### Vercel (Web Services Only)

**What Works:**
- ✅ Explorer (web/blockchain_explorer.py)
- ✅ Forum (web/forum.py)
- ✅ Wallet API (api/wallet_service.py)

**What Doesn't Work:**
- ❌ Blockchain node (needs persistent process)
- ❌ Mining pools (need socket connections)

**Deploy Web Services:**
```bash
# Install Vercel CLI
npm install -g vercel

# Deploy explorer
cd web
vercel --prod

# Deploy wallet API
cd api
vercel --prod
```

### Railway.app

**Better for Node:**
- ✅ Supports long-running processes
- ✅ Persistent storage
- ✅ Custom ports
- **Cost:** $5-20/month

```bash
# Install Railway CLI
npm install -g @railway/cli

# Login
railway login

# Deploy
railway up
```

### Render.com

**Good for Node:**
- ✅ Free tier available
- ✅ Persistent storage
- ✅ Custom ports
- **Cost:** Free - $25/month

---

## 5. 📦 Recommended Deployment Setup

### Production Architecture

```
┌─────────────────────────────────────────┐
│         Load Balancer (Cloudflare)      │
│              SSL/TLS Termination         │
└────────────────┬────────────────────────┘
                 │
    ┌────────────┴────────────┐
    │                         │
┌───▼────────┐        ┌──────▼──────┐
│  VPS #1    │        │   VPS #2    │
│  (Primary) │        │  (Backup)   │
│            │        │             │
│ • Node     │        │ • Node      │
│ • Pools    │        │ • Pools     │
│ • Web      │        │ • Web       │
└────────────┘        └─────────────┘
```

### Single Server Setup (Most Common)

**Server Specs:**
- **RAM:** 4GB minimum, 8GB recommended
- **CPU:** 2 cores minimum, 4 cores recommended
- **Storage:** 50GB minimum, 100GB+ recommended
- **Bandwidth:** 1TB/month minimum

**Installation:**

```bash
# 1. Update system
sudo apt update && sudo apt upgrade -y

# 2. Install dependencies
sudo apt install -y python3 python3-pip git cmake build-essential \
  nginx certbot python3-certbot-nginx

# 3. Clone repository
git clone https://github.com/YOUR_USERNAME/GXC-goldxcoin-blockchain-.git
cd GXC-goldxcoin-blockchain-

# 4. Install Python packages
pip3 install -r requirements.txt

# 5. Configure firewall
sudo ufw allow 22/tcp      # SSH
sudo ufw allow 80/tcp      # HTTP
sudo ufw allow 443/tcp     # HTTPS
sudo ufw allow 18332/tcp   # Blockchain RPC
sudo ufw allow 3333/tcp    # Mining pools
sudo ufw allow 3334/tcp
sudo ufw allow 3335/tcp
sudo ufw enable

# 6. Create systemd service
sudo nano /etc/systemd/system/gxc-blockchain.service
```

**Systemd Service File:**

```ini
[Unit]
Description=GXC Blockchain Node
After=network.target

[Service]
Type=forking
User=ubuntu
WorkingDirectory=/home/ubuntu/GXC-goldxcoin-blockchain-
ExecStart=/home/ubuntu/GXC-goldxcoin-blockchain-/start_complete_ecosystem.sh
ExecStop=/home/ubuntu/GXC-goldxcoin-blockchain-/stop_complete_ecosystem.sh
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target
```

**Enable and Start:**

```bash
sudo systemctl daemon-reload
sudo systemctl enable gxc-blockchain
sudo systemctl start gxc-blockchain
sudo systemctl status gxc-blockchain
```

---

## 6. 🔒 Security Configuration

### Nginx Reverse Proxy

```nginx
# /etc/nginx/sites-available/gxc-blockchain

# Explorer
server {
    listen 80;
    server_name explorer.yourdomain.com;

    location / {
        proxy_pass http://localhost:3000;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
    }
}

# Wallet API
server {
    listen 80;
    server_name api.yourdomain.com;

    location / {
        proxy_pass http://localhost:5000;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
    }
}

# Mining Pool
server {
    listen 80;
    server_name pool.yourdomain.com;

    location / {
        proxy_pass http://localhost:6000;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
    }
}
```

**Enable SSL:**

```bash
sudo certbot --nginx -d explorer.yourdomain.com
sudo certbot --nginx -d api.yourdomain.com
sudo certbot --nginx -d pool.yourdomain.com
```

### Environment Variables

```bash
# Create .env file
cat > .env << EOF
BLOCKCHAIN_NODE_URL=http://localhost:18332
DATABASE_PATH=/var/lib/gxc/blockchain.db
SECRET_KEY=$(openssl rand -hex 32)
JWT_SECRET=$(openssl rand -hex 32)
EXPLORER_URL=https://explorer.yourdomain.com
WALLET_API_URL=https://api.yourdomain.com
POOL_URL=https://pool.yourdomain.com
EOF

# Load environment
source .env
```

---

## 7. 📊 Monitoring

### Setup Monitoring

```bash
# Install monitoring tools
sudo apt install -y htop iotop nethogs

# Monitor processes
htop

# Monitor network
nethogs

# Monitor logs
tail -f logs/*.log
```

### Automated Monitoring Script

```bash
#!/bin/bash
# monitor.sh

while true; do
    # Check if node is running
    if ! pgrep -f "testnet_blockchain_node.py" > /dev/null; then
        echo "Node crashed! Restarting..."
        ./start_complete_ecosystem.sh
    fi
    
    # Check disk space
    DISK_USAGE=$(df -h / | awk 'NR==2 {print $5}' | sed 's/%//')
    if [ $DISK_USAGE -gt 90 ]; then
        echo "WARNING: Disk usage at ${DISK_USAGE}%"
    fi
    
    sleep 60
done
```

---

## 8. 💾 Backup Strategy

### Automated Backup Script

```bash
#!/bin/bash
# backup.sh

BACKUP_DIR="/backup/gxc"
DATE=$(date +%Y%m%d_%H%M%S)

# Create backup directory
mkdir -p $BACKUP_DIR

# Backup blockchain data
tar -czf $BACKUP_DIR/blockchain_$DATE.tar.gz gxc_data/

# Backup databases
cp *.db $BACKUP_DIR/

# Backup configuration
tar -czf $BACKUP_DIR/config_$DATE.tar.gz *.json *.conf

# Keep only last 7 days
find $BACKUP_DIR -name "*.tar.gz" -mtime +7 -delete

echo "Backup completed: $DATE"
```

**Setup Cron Job:**

```bash
# Edit crontab
crontab -e

# Add daily backup at 2 AM
0 2 * * * /home/ubuntu/GXC-goldxcoin-blockchain-/backup.sh
```

---

## 9. 🌍 Domain Configuration

### DNS Records

```
Type    Name        Value                   TTL
A       @           YOUR_SERVER_IP          3600
A       explorer    YOUR_SERVER_IP          3600
A       api         YOUR_SERVER_IP          3600
A       pool        YOUR_SERVER_IP          3600
A       forum       YOUR_SERVER_IP          3600
CNAME   www         yourdomain.com          3600
```

### Update Miner Download Links

After deployment, update miner configurations to point to your domain:

```json
{
    "pool_url": "pool.yourdomain.com:3333",
    "api_url": "https://api.yourdomain.com",
    "explorer_url": "https://explorer.yourdomain.com"
}
```

---

## 10. 📈 Scaling

### Horizontal Scaling

**Multiple Nodes:**
```
Node 1: Primary (US East)
Node 2: Backup (US West)
Node 3: Backup (Europe)
```

**Load Balancer:**
- Use Cloudflare for DDoS protection
- Use HAProxy for load balancing
- Use GeoDNS for regional routing

### Vertical Scaling

**Upgrade Path:**
```
Starter:  2GB RAM, 1 CPU  → $6/month
Growth:   4GB RAM, 2 CPU  → $12/month
Business: 8GB RAM, 4 CPU  → $24/month
Enterprise: 16GB RAM, 8 CPU → $48/month
```

---

## 🎯 Quick Deployment Checklist

- [ ] Choose hosting provider
- [ ] Create server/instance
- [ ] Install dependencies
- [ ] Clone repository
- [ ] Configure firewall
- [ ] Start services
- [ ] Setup domain/DNS
- [ ] Configure SSL
- [ ] Setup monitoring
- [ ] Configure backups
- [ ] Test all services
- [ ] Update miner links

---

## 📞 Recommended Setup for Different Scales

### Hobby/Testing ($0-10/month)
- **Platform:** Railway.app or Render.com (Free tier)
- **Specs:** 512MB RAM, shared CPU
- **Users:** 1-10

### Small Project ($10-25/month)
- **Platform:** DigitalOcean or Hetzner VPS
- **Specs:** 2GB RAM, 1 CPU
- **Users:** 10-100

### Medium Project ($25-50/month)
- **Platform:** DigitalOcean or AWS
- **Specs:** 4GB RAM, 2 CPU
- **Users:** 100-1000

### Large Project ($50-200/month)
- **Platform:** AWS or GCP
- **Specs:** 8GB+ RAM, 4+ CPU, Load Balancer
- **Users:** 1000-10000+

---

## 🚀 Fastest Deployment (5 Minutes)

### Railway.app (Recommended for Quick Start)

```bash
# 1. Install Railway CLI
npm install -g @railway/cli

# 2. Login
railway login

# 3. Initialize project
railway init

# 4. Deploy
railway up

# 5. Get URL
railway domain
```

**Your node will be live at:** `https://your-project.up.railway.app`

---

**Choose the deployment option that fits your needs and budget!**

For most users, we recommend:
- **Testing:** Railway.app (Free)
- **Production:** Hetzner VPS ($5-10/month)
- **Enterprise:** AWS with load balancing ($50+/month)
