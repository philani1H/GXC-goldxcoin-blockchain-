# Where to Deploy GXC Node for Mining

> **🆓 Looking for FREE options?** See **[FREE_DEPLOYMENT_GUIDE.md](FREE_DEPLOYMENT_GUIDE.md)** for completely free deployment options including Oracle Cloud Free Tier!

## 🎯 Quick Answer: Best Deployment Options

### **Option 1: VPS/Cloud Server (Recommended for Mining)**

**Best for:** Production mining, 24/7 operation, multiple miners

**Recommended Providers:**
- **Hetzner** - €4.15/month (4GB RAM) - Best value
- **DigitalOcean** - $12/month (2GB RAM)
- **Linode** - $12/month (2GB RAM)
- **AWS EC2** - t3.small ($15/month)
- **Vultr** - $6/month (1GB RAM) - Budget option

**Why VPS?**
- ✅ Always online (24/7 mining)
- ✅ Public IP address (miners can connect)
- ✅ Reliable uptime
- ✅ Can run multiple services (node + mining pool)
- ✅ Easy to scale

---

### **Option 2: Local Computer**

**Best for:** Testing, learning, small-scale mining

**Requirements:**
- Your computer running 24/7
- Port forwarding on router (ports 9333, 8545, 8080)
- Static IP or dynamic DNS (for external miners)

**Pros:**
- ✅ Free (no monthly cost)
- ✅ Full control
- ✅ Good for testing

**Cons:**
- ❌ Requires computer to be always on
- ❌ Home internet may be slower
- ❌ Power costs

---

### **Option 3: Docker (Any Platform)**

**Best for:** Easy deployment, consistent environment

Deploy on:
- Your own server
- VPS
- Cloud platforms
- Local machine

---

## 🚀 Quick Deployment Steps

### **VPS Deployment (Recommended)**

#### Step 1: Get a VPS
1. Sign up at Hetzner, DigitalOcean, or your preferred provider
2. Create Ubuntu 22.04 server (minimum 2GB RAM)
3. Note your server IP address

#### Step 2: Deploy Node
```bash
# SSH into your server
ssh root@your-server-ip

# Clone or upload your project
git clone https://github.com/your-repo/gxc-blockchain.git
cd gxc-blockchain

# Run automated deployment script
chmod +x scripts/deploy-blockchain.sh
./scripts/deploy-blockchain.sh
```

#### Step 3: Start Node
```bash
# If systemd service was created:
sudo systemctl start gxc-node
sudo systemctl status gxc-node

# Or manually:
./build/gxc-node --rpc-port=8545 --rest-port=8080 --network-port=9333
```

#### Step 4: Configure Firewall
```bash
# Allow necessary ports
sudo ufw allow 9333/tcp  # P2P network
sudo ufw allow 8545/tcp  # RPC API (for miners)
sudo ufw allow 8080/tcp  # REST API
sudo ufw enable
```

#### Step 5: Verify Node is Running
```bash
# Test RPC (should return blockchain info)
curl -X POST http://localhost:8545 \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getblockchaininfo","params":[],"id":1}'

# Test REST API
curl http://localhost:8080/api/v1/blockchain/info
```

---

## 🔧 Configuration for Mining

### Update Miners to Connect to Your Node

Once your node is deployed, update miners to connect:

**For Python Miner:**
```bash
python gxhash_miner.py --rpc-url http://YOUR-SERVER-IP:8545
```

**For C++ Miners:**
```bash
./gxc-gxhash-miner --rpc-url=http://YOUR-SERVER-IP:8545
./gxc-ethash-miner --rpc-url=http://YOUR-SERVER-IP:8545 --gpu
```

**Replace `YOUR-SERVER-IP` with:**
- Your VPS IP address (e.g., `123.45.67.89`)
- Or your domain name (e.g., `node.yourdomain.com`)

---

## 📍 Deployment Locations Summary

| Location | Cost | Best For | Difficulty |
|----------|------|----------|------------|
| **VPS (Hetzner/DigitalOcean)** | $4-15/month | Production mining | ⭐ Easy |
| **AWS EC2** | $15+/month | Enterprise | ⭐⭐ Medium |
| **Local Computer** | Free | Testing | ⭐⭐ Medium |
| **Docker** | Varies | Any platform | ⭐ Easy |

---

## 🔐 Security Checklist

After deployment:

- [ ] Firewall configured (ports 9333, 8545, 8080)
- [ ] Node running and accessible
- [ ] RPC API responding
- [ ] REST API responding
- [ ] Logs showing no errors
- [ ] Node syncing with network (if mainnet)

---

## 🎯 Next Steps After Deployment

1. **Start Mining:**
   ```bash
   # On your local machine or another server
   python mining/gxhash_miner.py --rpc-url http://YOUR-SERVER-IP:8545
   ```

2. **Monitor Node:**
   ```bash
   # On server
   sudo journalctl -u gxc-node -f
   ```

3. **Check Mining Status:**
   - Visit: `http://YOUR-SERVER-IP:8080` (REST API)
   - Or use RPC: `http://YOUR-SERVER-IP:8545`

---

## 💡 Pro Tips

1. **Use a Domain Name:**
   - Point domain to your VPS IP
   - Easier to remember than IP address
   - Can use SSL/HTTPS later

2. **Set Up Monitoring:**
   - Monitor node uptime
   - Alert if node goes down
   - Track mining statistics

3. **Backup Regularly:**
   - Backup `gxc_data/` directory
   - Keep wallet files safe
   - Store backups off-server

4. **Start with Testnet:**
   - Test deployment on testnet first
   - Use `--testnet` flag
   - Verify everything works before mainnet

---

## 🆘 Troubleshooting

### Can't Connect to Node
- Check firewall rules
- Verify node is running: `sudo systemctl status gxc-node`
- Check logs: `sudo journalctl -u gxc-node -f`
- Test locally first: `curl http://localhost:8545`

### Node Not Syncing
- Check network connectivity
- Verify ports are open
- Check for other nodes on network
- Review network logs

### Miners Can't Connect
- Verify RPC port (8545) is accessible
- Check firewall allows external connections
- Test with: `curl http://YOUR-IP:8545`
- Check node logs for connection attempts

---

## 📚 More Information

- **Full Deployment Guide:** See `DEPLOY_BLOCKCHAIN.md`
- **Build Instructions:** See `BUILD_INSTRUCTIONS.md`
- **Mining Guide:** See `MINERS_README.md`
- **Deployment Script:** `scripts/deploy-blockchain.sh`

---

## 🎉 Ready to Mine!

Once your node is deployed and running:

1. ✅ Node is accessible at `http://YOUR-SERVER-IP:8545`
2. ✅ Miners can connect and start mining
3. ✅ Blocks will be mined and rewards distributed
4. ✅ Monitor via REST API at `http://YOUR-SERVER-IP:8080`

**Happy Mining! ⛏️**
