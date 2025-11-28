#!/bin/bash
# GXC Blockchain Deployment Script
# Supports multiple deployment platforms

set -e  # Exit on error

echo "🚀 GXC Blockchain Deployment"
echo "======================================"
echo ""

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to display menu
show_menu() {
    echo -e "${BLUE}Choose deployment platform:${NC}"
    echo ""
    echo "1) Railway.app (Free, Recommended for testing)"
    echo "2) Render.com (Free tier available)"
    echo "3) Docker (Local or any Docker host)"
    echo "4) VPS/Server (Manual setup guide)"
    echo "5) Heroku (Deprecated but still works)"
    echo "6) Exit"
    echo ""
}

# Function to check prerequisites
check_prerequisites() {
    local tool=$1
    if ! command -v $tool &> /dev/null; then
        echo -e "${RED}❌ $tool not found${NC}"
        return 1
    else
        echo -e "${GREEN}✅ $tool found${NC}"
        return 0
    fi
}

# Railway.app deployment
deploy_railway() {
    echo ""
    echo -e "${YELLOW}Deploying to Railway.app...${NC}"
    echo ""
    
    # Check if Railway CLI is installed
    if ! check_prerequisites "railway"; then
        echo ""
        echo "Installing Railway CLI..."
        npm install -g @railway/cli
    fi
    
    echo ""
    echo "Logging in to Railway..."
    railway login
    
    echo ""
    echo "Initializing project..."
    railway init
    
    echo ""
    echo "Deploying..."
    railway up
    
    echo ""
    echo -e "${GREEN}✅ Deployment complete!${NC}"
    echo ""
    echo "Get your URL:"
    railway domain
    
    echo ""
    echo "View logs:"
    echo "  railway logs"
    echo ""
    echo "Open in browser:"
    echo "  railway open"
}

# Render.com deployment
deploy_render() {
    echo ""
    echo -e "${YELLOW}Deploying to Render.com...${NC}"
    echo ""
    
    echo "Creating render.yaml configuration..."
    
    cat > render.yaml << 'EOF'
services:
  - type: web
    name: gxc-blockchain
    env: python
    buildCommand: pip install -r requirements.txt
    startCommand: ./start_complete_ecosystem.sh
    envVars:
      - key: PYTHON_VERSION
        value: 3.11.0
    disk:
      name: gxc-data
      mountPath: /opt/render/project/src/gxc_data
      sizeGB: 10
EOF
    
    echo -e "${GREEN}✅ render.yaml created${NC}"
    echo ""
    echo "Next steps:"
    echo "1. Push code to GitHub"
    echo "2. Go to https://render.com"
    echo "3. Click 'New +' → 'Blueprint'"
    echo "4. Connect your GitHub repository"
    echo "5. Render will auto-deploy using render.yaml"
    echo ""
    echo "Or use Render CLI:"
    echo "  npm install -g render-cli"
    echo "  render login"
    echo "  render deploy"
}

# Docker deployment
deploy_docker() {
    echo ""
    echo -e "${YELLOW}Deploying with Docker...${NC}"
    echo ""
    
    # Check if Docker is installed
    if ! check_prerequisites "docker"; then
        echo ""
        echo "Please install Docker first:"
        echo "  https://docs.docker.com/get-docker/"
        return 1
    fi
    
    echo "Creating Dockerfile..."
    
    cat > Dockerfile << 'EOF'
FROM ubuntu:22.04

# Install dependencies
RUN apt-get update && apt-get install -y \
    python3 \
    python3-pip \
    git \
    cmake \
    build-essential \
    netcat \
    curl \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy application files
COPY . /app

# Install Python dependencies
RUN pip3 install -r requirements.txt

# Make scripts executable
RUN chmod +x start_complete_ecosystem.sh stop_complete_ecosystem.sh

# Expose ports
EXPOSE 18332 8080 8081 3000 3001 3333 3334 3335 5000 6000 6001 6002

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=60s --retries=3 \
    CMD curl -f http://localhost:3000 || exit 1

# Start services
CMD ["./start_complete_ecosystem.sh"]
EOF
    
    echo "Creating docker-compose.yml..."
    
    cat > docker-compose.yml << 'EOF'
version: '3.8'

services:
  gxc-blockchain:
    build: .
    container_name: gxc-blockchain
    ports:
      - "18332:18332"  # Blockchain RPC
      - "8080:8080"    # REST API
      - "8081:8081"    # WebSocket
      - "3000:3000"    # Explorer
      - "3001:3001"    # Forum
      - "3333:3333"    # General Pool Stratum
      - "3334:3334"    # GPU Pool Stratum
      - "3335:3335"    # ASIC Pool Stratum
      - "5000:5000"    # Wallet API
      - "6000:6000"    # General Pool Dashboard
      - "6001:6001"    # GPU Pool Dashboard
      - "6002:6002"    # ASIC Pool Dashboard
    volumes:
      - gxc-data:/app/gxc_data
      - gxc-logs:/app/logs
    restart: unless-stopped
    environment:
      - BLOCKCHAIN_NODE_URL=http://localhost:18332
      - DATABASE_PATH=/app/gxc_data/blockchain.db

volumes:
  gxc-data:
  gxc-logs:
EOF
    
    echo -e "${GREEN}✅ Docker files created${NC}"
    echo ""
    echo "Building Docker image..."
    docker build -t gxc-blockchain .
    
    echo ""
    echo "Starting containers..."
    docker-compose up -d
    
    echo ""
    echo -e "${GREEN}✅ Docker deployment complete!${NC}"
    echo ""
    echo "Access your blockchain:"
    echo "  Explorer:    http://localhost:3000"
    echo "  Wallet API:  http://localhost:5000"
    echo "  Pool:        http://localhost:6000"
    echo ""
    echo "View logs:"
    echo "  docker-compose logs -f"
    echo ""
    echo "Stop containers:"
    echo "  docker-compose down"
}

# VPS/Server deployment guide
deploy_vps() {
    echo ""
    echo -e "${YELLOW}VPS/Server Deployment Guide${NC}"
    echo ""
    
    cat << 'EOF'
====================================
VPS/Server Deployment Steps
====================================

1. CHOOSE A PROVIDER:
   - Hetzner: €4/month (Best value)
   - DigitalOcean: $6/month
   - Linode: $5/month
   - Vultr: $6/month

2. CREATE SERVER:
   - OS: Ubuntu 22.04 LTS
   - RAM: 4GB minimum
   - CPU: 2 cores minimum
   - Storage: 50GB minimum

3. SSH INTO SERVER:
   ssh root@YOUR_SERVER_IP

4. UPDATE SYSTEM:
   apt update && apt upgrade -y

5. INSTALL DEPENDENCIES:
   apt install -y python3 python3-pip git cmake build-essential \
     nginx certbot python3-certbot-nginx

6. CLONE REPOSITORY:
   git clone https://github.com/YOUR_USERNAME/GXC-goldxcoin-blockchain-.git
   cd GXC-goldxcoin-blockchain-

7. INSTALL PYTHON PACKAGES:
   pip3 install -r requirements.txt

8. CONFIGURE FIREWALL:
   ufw allow 22/tcp      # SSH
   ufw allow 80/tcp      # HTTP
   ufw allow 443/tcp     # HTTPS
   ufw allow 18332/tcp   # Blockchain RPC
   ufw allow 3333/tcp    # Mining pools
   ufw allow 3334/tcp
   ufw allow 3335/tcp
   ufw enable

9. CREATE SYSTEMD SERVICE:
   nano /etc/systemd/system/gxc-blockchain.service

   Paste:
   [Unit]
   Description=GXC Blockchain Node
   After=network.target

   [Service]
   Type=forking
   User=root
   WorkingDirectory=/root/GXC-goldxcoin-blockchain-
   ExecStart=/root/GXC-goldxcoin-blockchain-/start_complete_ecosystem.sh
   ExecStop=/root/GXC-goldxcoin-blockchain-/stop_complete_ecosystem.sh
   Restart=always
   RestartSec=10

   [Install]
   WantedBy=multi-user.target

10. START SERVICE:
    systemctl daemon-reload
    systemctl enable gxc-blockchain
    systemctl start gxc-blockchain
    systemctl status gxc-blockchain

11. CONFIGURE NGINX (Optional):
    nano /etc/nginx/sites-available/gxc-blockchain

    Paste:
    server {
        listen 80;
        server_name yourdomain.com;

        location / {
            proxy_pass http://localhost:3000;
            proxy_set_header Host $host;
            proxy_set_header X-Real-IP $remote_addr;
        }
    }

    ln -s /etc/nginx/sites-available/gxc-blockchain /etc/nginx/sites-enabled/
    nginx -t
    systemctl restart nginx

12. SETUP SSL (Optional):
    certbot --nginx -d yourdomain.com

13. VERIFY:
    curl http://localhost:3000
    curl http://yourdomain.com

====================================
Your blockchain is now deployed!
====================================

Access:
  Explorer:    http://yourdomain.com
  Wallet API:  http://yourdomain.com:5000
  Pool:        http://yourdomain.com:6000

Logs:
  journalctl -u gxc-blockchain -f

Restart:
  systemctl restart gxc-blockchain

Stop:
  systemctl stop gxc-blockchain

EOF
}

# Heroku deployment
deploy_heroku() {
    echo ""
    echo -e "${YELLOW}Deploying to Heroku...${NC}"
    echo ""
    
    # Check if Heroku CLI is installed
    if ! check_prerequisites "heroku"; then
        echo ""
        echo "Installing Heroku CLI..."
        curl https://cli-assets.heroku.com/install.sh | sh
    fi
    
    echo "Creating Procfile..."
    cat > Procfile << 'EOF'
web: ./start_complete_ecosystem.sh
EOF
    
    echo "Creating runtime.txt..."
    echo "python-3.11.0" > runtime.txt
    
    echo ""
    echo "Logging in to Heroku..."
    heroku login
    
    echo ""
    echo "Creating Heroku app..."
    heroku create gxc-blockchain-$(date +%s)
    
    echo ""
    echo "Deploying..."
    git push heroku main
    
    echo ""
    echo -e "${GREEN}✅ Deployment complete!${NC}"
    echo ""
    echo "Open your app:"
    heroku open
    
    echo ""
    echo "View logs:"
    heroku logs --tail
}

# Main menu loop
while true; do
    show_menu
    read -p "Enter choice [1-6]: " choice
    
    case $choice in
        1)
            deploy_railway
            break
            ;;
        2)
            deploy_render
            break
            ;;
        3)
            deploy_docker
            break
            ;;
        4)
            deploy_vps
            break
            ;;
        5)
            deploy_heroku
            break
            ;;
        6)
            echo "Exiting..."
            exit 0
            ;;
        *)
            echo -e "${RED}Invalid option. Please try again.${NC}"
            echo ""
            ;;
    esac
done

echo ""
echo "======================================"
echo -e "${GREEN}✅ Deployment process complete!${NC}"
echo "======================================"
echo ""
echo "📚 For more information, see:"
echo "  - DEPLOYMENT_GUIDE.md"
echo "  - COMPLETE_SETUP_GUIDE.md"
echo ""
