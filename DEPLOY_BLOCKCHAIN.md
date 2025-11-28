# Deploy GXC Blockchain - Quick Guide

## Deployment Options

### Option 1: Local Development (Current Setup)

**For testing and development:**

```bash
# Build the blockchain
.\build-blockchain.ps1

# Run the node
.\build\Release\gxc-node.exe --rpc-port=8545 --rest-port=8080
```

**Access:**
- RPC: `http://localhost:8545`
- REST: `http://localhost:8080`
- Network: `localhost:9333`

---

### Option 2: VPS/Cloud Server Deployment

**Recommended for production:**

#### Step 1: Choose Provider
- **DigitalOcean**: $12/month (2GB RAM)
- **Linode**: $12/month (2GB RAM)
- **AWS EC2**: t3.small ($15/month)
- **Hetzner**: €4.15/month (4GB RAM)

#### Step 2: Setup Server

```bash
# SSH into server
ssh root@your-server-ip

# Update system
sudo apt-get update && sudo apt-get upgrade -y

# Install dependencies
sudo apt-get install -y \
    build-essential \
    cmake \
    git \
    libssl-dev \
    libboost-all-dev \
    libsqlite3-dev \
    pkg-config \
    curl

# Clone repository (or upload files)
git clone https://github.com/your-repo/gxc-blockchain.git
cd gxc-blockchain
```

#### Step 3: Build Blockchain

```bash
# Create build directory
mkdir build && cd build

# Configure
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
make -j$(nproc) gxc-node

# Verify
ls -lh gxc-node
```

#### Step 4: Configure

```bash
# Create config directory
mkdir -p ~/.gxc

# Create config file
cat > ~/.gxc/gxc.conf << EOF
network_port=9333
rpc_port=8545
rest_port=8080
data_dir=./gxc_data
testnet=false
log_level=INFO
log_file=gxc.log
EOF
```

#### Step 5: Setup as Service

```bash
# Create systemd service
sudo nano /etc/systemd/system/gxc-node.service
```

**Service file:**
```ini
[Unit]
Description=GXC Blockchain Node
After=network.target

[Service]
Type=simple
User=your-user
Group=your-user
WorkingDirectory=/home/your-user/gxc-blockchain
ExecStart=/home/your-user/gxc-blockchain/build/gxc-node --config=/home/your-user/.gxc/gxc.conf
Restart=always
RestartSec=10
StandardOutput=journal
StandardError=journal

[Install]
WantedBy=multi-user.target
```

```bash
# Enable and start
sudo systemctl daemon-reload
sudo systemctl enable gxc-node
sudo systemctl start gxc-node
sudo systemctl status gxc-node
```

#### Step 6: Configure Firewall

```bash
# Allow necessary ports
sudo ufw allow 9333/tcp  # P2P network
sudo ufw allow 8545/tcp  # RPC API
sudo ufw allow 8080/tcp  # REST API
sudo ufw enable
```

#### Step 7: Setup Reverse Proxy (Optional)

**Using Nginx:**

```bash
sudo apt-get install nginx

sudo nano /etc/nginx/sites-available/gxc-node
```

**Nginx config:**
```nginx
server {
    listen 80;
    server_name your-domain.com;

    location /rpc {
        proxy_pass http://localhost:8545;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
    }

    location /api {
        proxy_pass http://localhost:8080;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
    }
}
```

```bash
sudo ln -s /etc/nginx/sites-available/gxc-node /etc/nginx/sites-enabled/
sudo nginx -t
sudo systemctl restart nginx
```

---

### Option 3: Docker Deployment

**For containerized deployment:**

#### Dockerfile

```dockerfile
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libssl-dev \
    libboost-all-dev \
    libsqlite3-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    make -j$(nproc) gxc-node

EXPOSE 9333 8545 8080

CMD ["./build/gxc-node", "--rpc-port=8545", "--rest-port=8080"]
```

#### Build and Run

```bash
# Build image
docker build -t gxc-node .

# Run container
docker run -d \
  --name gxc-node \
  -p 9333:9333 \
  -p 8545:8545 \
  -p 8080:8080 \
  -v gxc-data:/app/gxc_data \
  gxc-node
```

#### Docker Compose

```yaml
version: '3.8'

services:
  gxc-node:
    build: .
    ports:
      - "9333:9333"
      - "8545:8545"
      - "8080:8080"
    volumes:
      - gxc-data:/app/gxc_data
    restart: unless-stopped
    environment:
      - RPC_PORT=8545
      - REST_PORT=8080
      - NETWORK_PORT=9333

volumes:
  gxc-data:
```

```bash
docker-compose up -d
```

---

### Option 4: Cloud Platform Deployment

#### AWS EC2

1. **Launch Instance:**
   - AMI: Ubuntu 22.04 LTS
   - Instance Type: t3.small or larger
   - Security Group: Open ports 9333, 8545, 8080

2. **Connect and Setup:**
   ```bash
   ssh -i your-key.pem ubuntu@ec2-instance-ip
   # Follow VPS setup steps above
   ```

3. **Elastic IP (Optional):**
   - Allocate Elastic IP
   - Associate with instance
   - Use for static endpoint

#### Google Cloud Platform

1. **Create VM:**
   ```bash
   gcloud compute instances create gxc-node \
     --image-family=ubuntu-2204-lts \
     --image-project=ubuntu-os-cloud \
     --machine-type=e2-small \
     --tags=gxc-node
   ```

2. **Configure Firewall:**
   ```bash
   gcloud compute firewall-rules create allow-gxc-node \
     --allow tcp:9333,tcp:8545,tcp:8080 \
     --target-tags=gxc-node
   ```

---

## Quick Deployment Script

### Automated Deployment Script

```bash
#!/bin/bash
# deploy-blockchain.sh

echo "GXC Blockchain Deployment Script"
echo "================================="

# Install dependencies
echo "Installing dependencies..."
sudo apt-get update
sudo apt-get install -y build-essential cmake git libssl-dev libboost-all-dev libsqlite3-dev

# Build
echo "Building blockchain..."
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc) gxc-node

# Create config
echo "Creating configuration..."
mkdir -p ~/.gxc
cat > ~/.gxc/gxc.conf << EOF
network_port=9333
rpc_port=8545
rest_port=8080
data_dir=./gxc_data
testnet=false
EOF

# Create systemd service
echo "Setting up service..."
sudo tee /etc/systemd/system/gxc-node.service > /dev/null << EOF
[Unit]
Description=GXC Blockchain Node
After=network.target

[Service]
Type=simple
User=$USER
WorkingDirectory=$(pwd)
ExecStart=$(pwd)/build/gxc-node --config=$HOME/.gxc/gxc.conf
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target
EOF

# Enable and start
sudo systemctl daemon-reload
sudo systemctl enable gxc-node
sudo systemctl start gxc-node

echo "✅ Blockchain node deployed!"
echo "Check status: sudo systemctl status gxc-node"
echo "View logs: sudo journalctl -u gxc-node -f"
```

---

## Post-Deployment

### Verify Deployment

```bash
# Check if node is running
sudo systemctl status gxc-node

# Check logs
sudo journalctl -u gxc-node -f

# Test RPC
curl -X POST http://localhost:8545 \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getblockchaininfo","params":[],"id":1}'

# Test REST
curl http://localhost:8080/api/v1/blockchain/info
```

### Monitor Node

```bash
# View logs
sudo journalctl -u gxc-node -f

# Check resource usage
htop

# Check disk usage
df -h

# Check network connections
netstat -tulpn | grep -E '9333|8545|8080'
```

### Backup

```bash
# Backup script
#!/bin/bash
BACKUP_DIR="/backups/gxc"
DATA_DIR="./gxc_data"
DATE=$(date +%Y%m%d_%H%M%S)

mkdir -p $BACKUP_DIR
tar -czf $BACKUP_DIR/gxc_backup_$DATE.tar.gz $DATA_DIR

# Keep only last 7 days
find $BACKUP_DIR -name "gxc_backup_*.tar.gz" -mtime +7 -delete
```

---

## Production Checklist

- [ ] Blockchain node built successfully
- [ ] Configuration file created
- [ ] Systemd service configured
- [ ] Firewall rules set
- [ ] Node starts automatically on boot
- [ ] Logs configured and rotating
- [ ] Backup strategy in place
- [ ] Monitoring set up
- [ ] RPC/REST APIs accessible
- [ ] Network connectivity verified

---

## Next Steps

After deployment:

1. **Connect Miners:**
   - Update miner RPC URL to your server
   - Miners will auto-connect

2. **Connect Wallet Service:**
   - Update `NETWORK_INFO` in `wallet_service.py`
   - Point to your deployed node

3. **Connect Explorer:**
   - Update explorer API endpoint
   - Point to your REST API

4. **Monitor:**
   - Set up monitoring/alerts
   - Regular backups
   - Performance monitoring

---

## Support

For issues:
- Check logs: `sudo journalctl -u gxc-node -f`
- Verify ports: `netstat -tulpn`
- Test connectivity: `curl http://localhost:8080/api/v1/blockchain/info`

