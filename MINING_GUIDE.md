# 🛠️ GXC Blockchain - Complete Mining Guide

## 📋 Table of Contents

1. [Quick Start Example](#quick-start-example)
2. [Download Links](#download-links)
3. [Overview](#overview)
4. [Mining Algorithms](#mining-algorithms)
5. [Software Requirements](#software-requirements)
6. [Installation](#installation)
5. [CPU Mining (GXHash)](#cpu-mining-gxhash)
6. [GPU Mining (Ethash)](#gpu-mining-ethash)
7. [ASIC Mining (SHA-256)](#asic-mining-sha-256)
8. [Pool Mining Setup](#pool-mining-setup)
9. [Solo Mining Setup](#solo-mining-setup)
10. [Performance Optimization](#performance-optimization)
11. [Hardware Requirements](#hardware-requirements)
12. [Troubleshooting](#troubleshooting)
13. [Mining Software Tools](#mining-software-tools)

---

## Quick Start Example

### 🚀 5-Minute Setup (CPU Mining)

#### Step 1: Download Mining Software
```bash
# Linux (64-bit)
wget https://github.com/gxc-blockchain/gxc-blockchain/releases/latest/download/gxc-gxhash-miner-linux-x64.tar.gz
tar -xzf gxc-gxhash-miner-linux-x64.tar.gz
cd gxc-gxhash-miner-linux-x64

# Windows (64-bit)
# Download: gxc-gxhash-miner-windows-x64.zip
# Extract and run: gxc-gxhash-miner.exe

# macOS (Intel)
# Download: gxc-gxhash-miner-macos-x64.tar.gz
# Extract and run: ./gxc-gxhash-miner
```

#### Step 2: Create Configuration File
```bash
# Create config directory
mkdir -p ~/.gxc

# Create mining config
cat > ~/.gxc/mining.conf << EOF
algorithm=gxhash
threads=0
address=GXC1YOUR_MINER_ADDRESS_HERE
pool=stratum+tcp://pool.gxc.network:3333
username=your_worker_name
password=x
EOF
```

#### Step 3: Start Mining
```bash
# Linux/macOS
./gxc-gxhash-miner -c ~/.gxc/mining.conf

# Windows
gxc-gxhash-miner.exe -c %APPDATA%\GXC\mining.conf
```

#### Step 4: Monitor Mining
```bash
# View stats (in another terminal)
gxc-gxhash-miner --stats

# Or check pool dashboard
# Visit: https://pool.gxc.network/dashboard
```

**Expected Output:**
```
GXC GXHash Miner v1.0.0
========================
Algorithm: GXHash
Threads: 8
Address: GXC1YOUR_ADDRESS
Pool: stratum+tcp://pool.gxc.network:3333
Status: Connected
Hash Rate: 850.5 H/s
Shares: 12 accepted, 0 rejected
Uptime: 00:05:23
```

---

## Download Links

### 📥 Official Downloads

#### Latest Releases
- **GitHub Releases**: https://github.com/gxc-blockchain/gxc-blockchain/releases/latest
- **Official Website**: https://gxc.network/downloads
- **Mirror (IPFS)**: https://ipfs.gxc.network/mining-software

#### Pre-built Binaries

##### Universal Miner (All Algorithms)
- **Linux x64**: [gxc-miner-linux-x64.tar.gz](https://github.com/gxc-blockchain/gxc-blockchain/releases/latest/download/gxc-miner-linux-x64.tar.gz)
- **Windows x64**: [gxc-miner-windows-x64.zip](https://github.com/gxc-blockchain/gxc-blockchain/releases/latest/download/gxc-miner-windows-x64.zip)
- **macOS x64**: [gxc-miner-macos-x64.tar.gz](https://github.com/gxc-blockchain/gxc-blockchain/releases/latest/download/gxc-miner-macos-x64.tar.gz)
- **macOS ARM64**: [gxc-miner-macos-arm64.tar.gz](https://github.com/gxc-blockchain/gxc-blockchain/releases/latest/download/gxc-miner-macos-arm64.tar.gz)

##### CPU Miner (GXHash)
- **Linux x64**: [gxc-gxhash-miner-linux-x64.tar.gz](https://github.com/gxc-blockchain/gxc-blockchain/releases/latest/download/gxc-gxhash-miner-linux-x64.tar.gz)
- **Windows x64**: [gxc-gxhash-miner-windows-x64.zip](https://github.com/gxc-blockchain/gxc-blockchain/releases/latest/download/gxc-gxhash-miner-windows-x64.zip)
- **macOS x64**: [gxc-gxhash-miner-macos-x64.tar.gz](https://github.com/gxc-blockchain/gxc-blockchain/releases/latest/download/gxc-gxhash-miner-macos-x64.tar.gz)

##### GPU Miner (Ethash)
- **Linux x64 (NVIDIA)**: [gxc-ethash-miner-linux-nvidia.tar.gz](https://github.com/gxc-blockchain/gxc-blockchain/releases/latest/download/gxc-ethash-miner-linux-nvidia.tar.gz)
- **Linux x64 (AMD)**: [gxc-ethash-miner-linux-amd.tar.gz](https://github.com/gxc-blockchain/gxc-blockchain/releases/latest/download/gxc-ethash-miner-linux-amd.tar.gz)
- **Windows x64 (NVIDIA)**: [gxc-ethash-miner-windows-nvidia.zip](https://github.com/gxc-blockchain/gxc-blockchain/releases/latest/download/gxc-ethash-miner-windows-nvidia.zip)
- **Windows x64 (AMD)**: [gxc-ethash-miner-windows-amd.zip](https://github.com/gxc-blockchain/gxc-blockchain/releases/latest/download/gxc-ethash-miner-windows-amd.zip)

##### ASIC Miner (SHA-256)
- **Linux x64**: [gxc-sha256-miner-linux-x64.tar.gz](https://github.com/gxc-blockchain/gxc-blockchain/releases/latest/download/gxc-sha256-miner-linux-x64.tar.gz)
- **Firmware Updates**: [ASIC Firmware Repository](https://github.com/gxc-blockchain/gxc-asic-firmware)

#### Source Code
- **Main Repository**: https://github.com/gxc-blockchain/gxc-blockchain
- **Mining Module**: https://github.com/gxc-blockchain/gxc-blockchain/tree/main/mining
- **Build Instructions**: See [Installation](#installation) section below

### 🔐 Verification

#### Verify Downloads (SHA256 Checksums)
```bash
# Download checksums file
wget https://github.com/gxc-blockchain/gxc-blockchain/releases/latest/download/checksums.txt

# Verify download
sha256sum -c checksums.txt
```

#### GPG Signature Verification
```bash
# Import GPG key
gpg --keyserver keyserver.ubuntu.com --recv-keys 0xGXC_PUBLIC_KEY

# Download signature
wget https://github.com/gxc-blockchain/gxc-blockchain/releases/latest/download/gxc-miner-linux-x64.tar.gz.asc

# Verify signature
gpg --verify gxc-miner-linux-x64.tar.gz.asc gxc-miner-linux-x64.tar.gz
```

### 📦 Package Managers

#### Linux Package Managers

**Debian/Ubuntu (APT)**
```bash
# Add repository
curl -fsSL https://apt.gxc.network/gxc-blockchain.gpg.key | sudo apt-key add -
echo "deb https://apt.gxc.network/ stable main" | sudo tee /etc/apt/sources.list.d/gxc-blockchain.list

# Install
sudo apt-get update
sudo apt-get install gxc-miner
```

**RedHat/CentOS (YUM/DNF)**
```bash
# Add repository
sudo yum install https://yum.gxc.network/gxc-blockchain-release.rpm

# Install
sudo yum install gxc-miner
```

**Arch Linux (AUR)**
```bash
yay -S gxc-miner
# OR
paru -S gxc-miner
```

#### Windows Package Managers

**Chocolatey**
```powershell
choco install gxc-miner
```

**Scoop**
```powershell
scoop bucket add gxc https://github.com/gxc-blockchain/scoop-bucket
scoop install gxc-miner
```

#### macOS Package Managers

**Homebrew**
```bash
brew tap gxc-blockchain/gxc
brew install gxc-miner
```

**MacPorts**
```bash
sudo port install gxc-miner
```

### 🐳 Docker Images

#### Official Docker Images
```bash
# Pull mining image
docker pull gxcblockchain/gxc-miner:latest

# Run CPU miner
docker run -d --name gxc-miner \
  -v ~/.gxc:/root/.gxc \
  gxcblockchain/gxc-miner:latest \
  --algorithm=gxhash --address=GXC1YOUR_ADDRESS

# Run GPU miner (requires nvidia-docker)
docker run -d --name gxc-gpu-miner \
  --gpus all \
  -v ~/.gxc:/root/.gxc \
  gxcblockchain/gxc-miner:latest \
  --algorithm=ethash --address=GXC1YOUR_ADDRESS
```

**Docker Hub**: https://hub.docker.com/r/gxcblockchain/gxc-miner

### 📱 Mobile Mining (Android)

**Note**: Mobile mining is experimental and not recommended for serious mining.

- **Android APK**: [gxc-miner-android.apk](https://github.com/gxc-blockchain/gxc-blockchain/releases/latest/download/gxc-miner-android.apk)
- **Google Play**: Coming Soon
- **F-Droid**: Coming Soon

### 🔧 Development Builds

#### Nightly Builds
- **Linux**: https://builds.gxc.network/nightly/linux/
- **Windows**: https://builds.gxc.network/nightly/windows/
- **macOS**: https://builds.gxc.network/nightly/macos/

#### CI/CD Artifacts
- **GitHub Actions**: https://github.com/gxc-blockchain/gxc-blockchain/actions
- **CircleCI**: https://circleci.com/gh/gxc-blockchain/gxc-blockchain

### 📚 Additional Resources

- **Documentation**: https://docs.gxc.network/mining
- **Video Tutorials**: https://www.youtube.com/c/GXCBlockchain
- **Community Forums**: https://forum.gxc.network
- **Discord Support**: https://discord.gg/gxc-blockchain

---

## Overview

GXC Blockchain supports **three mining algorithms** to ensure decentralization and accessibility:

- **SHA-256** (ASIC): Bitcoin-compatible, optimized for ASIC miners
- **Ethash** (GPU): Ethereum-compatible, optimized for GPU mining
- **GXHash** (CPU): Custom algorithm optimized for CPU mining with traceability features

The blockchain uses a **hybrid consensus** model with a **3:1 ratio** of PoW to PoS blocks.

---

## Mining Algorithms

### SHA-256 (ASIC Mining)
- **Target Hardware**: ASIC miners (Antminer, WhatsMiner, etc.)
- **Block Type**: PoW blocks (75% of blocks)
- **Difficulty**: Adjusts every 2016 blocks
- **Block Time**: ~2.5 minutes (150 seconds)
- **Reward**: 12.5 GXC per block (halving every 4 years)

### Ethash (GPU Mining)
- **Target Hardware**: NVIDIA/AMD GPUs
- **Block Type**: PoW blocks (75% of blocks)
- **Memory**: Requires DAG generation (~4GB+)
- **Block Time**: ~2.5 minutes (150 seconds)
- **Reward**: 12.5 GXC per block

### GXHash (CPU Mining)
- **Target Hardware**: CPU (multi-core recommended)
- **Block Type**: PoW blocks (75% of blocks)
- **Features**: Traceability-optimized, CPU-friendly
- **Block Time**: ~2.5 minutes (150 seconds)
- **Reward**: 12.5 GXC per block

---

## Software Requirements

### Operating Systems
- **Linux**: Ubuntu 20.04+, Debian 11+, CentOS 8+
- **Windows**: Windows 10/11 (64-bit)
- **macOS**: macOS 10.14+ (Intel/Apple Silicon)

### Build Dependencies

#### Linux
```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    git \
    libssl-dev \
    libboost-all-dev \
    libsqlite3-dev \
    pkg-config

# For GPU mining (NVIDIA)
sudo apt-get install -y nvidia-cuda-toolkit

# For GPU mining (AMD)
sudo apt-get install -y rocm-dev
```

#### Windows
- **Visual Studio 2019/2022** (Community Edition is free)
- **CMake** 3.16+
- **OpenSSL** (vcpkg or pre-built)
- **Boost** libraries
- **NVIDIA CUDA Toolkit** (for GPU mining)
- **AMD ROCm** (for AMD GPU mining)

#### macOS
```bash
# Install Homebrew if not already installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install cmake openssl boost sqlite3
```

---

## Installation

### 1. Clone the Repository
```bash
git clone https://github.com/gxc-blockchain/gxc-blockchain.git
cd gxc-blockchain
```

### 2. Build the Blockchain Node
```bash
# Create build directory
mkdir build && cd build

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build (use -j flag for parallel compilation)
make -j$(nproc)  # Linux
# OR
cmake --build . --config Release -j  # Windows/Cross-platform
```

### 3. Build Mining Software

#### Build All Miners
```bash
# From build directory
cmake .. -DBUILD_MINING_CLIENT=ON
make -j$(nproc)
```

#### Build Specific Miners
```bash
# CPU Miner (GXHash)
make gxc-gxhash-miner

# GPU Miner (Ethash)
make gxc-ethash-miner

# ASIC Miner (SHA-256)
make gxc-sha256-miner

# Universal Miner (All algorithms)
make gxc-miner
```

### 4. Install (Optional)
```bash
sudo make install
```

This installs:
- `gxc-node` - Blockchain node
- `gxc-miner` - Universal miner
- `gxc-gxhash-miner` - CPU miner
- `gxc-ethash-miner` - GPU miner
- `gxc-sha256-miner` - ASIC miner
- `gxc-cli` - Command-line interface

---

## CPU Mining (GXHash)

### Quick Start

#### 1. Create Mining Configuration
```bash
mkdir -p ~/.gxc
nano ~/.gxc/mining.conf
```

```ini
# GXHash CPU Mining Configuration
algorithm=gxhash
threads=0  # 0 = auto-detect optimal thread count
address=GXC1YOUR_MINER_ADDRESS_HERE

# Pool Mining (Recommended)
pool=stratum+tcp://pool.gxc.network:3333
username=your_worker_name
password=x

# OR Solo Mining (Large operations only)
# solo=true
# node=http://localhost:8332
# rpcuser=your_rpc_user
# rpcpassword=your_rpc_password
```

#### 2. Start Mining
```bash
# Using dedicated CPU miner
gxc-gxhash-miner -c ~/.gxc/mining.conf

# OR using universal miner
gxc-miner --algorithm=gxhash --address=GXC1YOUR_ADDRESS --pool=stratum+tcp://pool.gxc.network:3333 --username=worker1
```

### Advanced Configuration

```ini
# Advanced GXHash Configuration
algorithm=gxhash
threads=8  # Specific thread count
address=GXC1YOUR_ADDRESS

# Thread affinity (pin threads to specific CPU cores)
thread_affinity=0,1,2,3,4,5,6,7

# Enable huge pages for better performance
huge_pages=true

# Mining intensity (1-100)
intensity=75

# Pool configuration
pool=stratum+tcp://pool.gxc.network:3333
username=worker1
password=x

# Backup pool
backup_pool=stratum+tcp://backup-pool.gxc.network:3333
backup_username=worker1
backup_password=x

# Logging
log_level=INFO
log_file=~/.gxc/mining.log
```

### Performance Tips

1. **Use All CPU Cores**: Set `threads=0` for auto-detection
2. **Enable Huge Pages**: Improves performance by 10-15%
   ```bash
   # Linux
   sudo sysctl vm.nr_hugepages=128
   ```
3. **Set CPU Affinity**: Pin threads to specific cores
4. **Disable CPU Power Saving**: Set CPU governor to performance
   ```bash
   # Linux
   sudo cpupower frequency-set -g performance
   ```

### Expected Hash Rates

| CPU Type | Cores | Expected Hash Rate |
|----------|-------|-------------------|
| Intel i5-10400 | 6 cores | ~500-800 H/s |
| Intel i7-10700K | 8 cores | ~800-1200 H/s |
| AMD Ryzen 5 3600 | 6 cores | ~600-900 H/s |
| AMD Ryzen 7 3700X | 8 cores | ~1000-1500 H/s |
| AMD Ryzen 9 5900X | 12 cores | ~1500-2200 H/s |

---

## GPU Mining (Ethash)

### NVIDIA GPU Setup

#### 1. Install CUDA Toolkit
```bash
# Download CUDA Toolkit 12.0+
wget https://developer.download.nvidia.com/compute/cuda/12.0.0/local_installers/cuda_12.0.0_525.60.13_linux.run
sudo sh cuda_12.0.0_525.60.13_linux.run

# Verify installation
nvcc --version
nvidia-smi
```

#### 2. Create GPU Mining Configuration
```ini
# Ethash GPU Mining Configuration (NVIDIA)
algorithm=ethash
devices=0,1,2,3  # GPU device IDs (comma-separated)
intensity=85  # Mining intensity (1-100)
address=GXC1YOUR_ADDRESS

# Pool configuration
pool=stratum+tcp://gpu-pool.gxc.network:4444
username=your_worker_name.gpu
password=x

# Performance tuning
memory_tweak=1
core_clock=+100  # MHz offset
memory_clock=+500  # MHz offset
power_limit=250  # Watts

# Fan control
fan_speed=80  # Percentage (0-100)

# Temperature limits
temp_target=70  # Celsius
temp_limit=85  # Celsius
```

#### 3. Start GPU Mining
```bash
gxc-ethash-miner -c ~/.gxc/gpu-mining.conf
```

### AMD GPU Setup

#### 1. Install ROCm
```bash
# Ubuntu 22.04
wget https://repo.radeon.com/amdgpu-install/22.40/ubuntu/jammy/amdgpu-install_5.4.50400-1_all.deb
sudo dpkg -i amdgpu-install_5.4.50400-1_all.deb
sudo amdgpu-install --usecase=rocm

# Verify installation
rocminfo
```

#### 2. AMD GPU Configuration
```ini
# Ethash GPU Mining Configuration (AMD)
algorithm=ethash
devices=0,1  # AMD GPU device IDs
intensity=90
address=GXC1YOUR_ADDRESS

# Pool configuration
pool=stratum+tcp://gpu-pool.gxc.network:4444
username=your_worker_name.amd
password=x

# AMD-specific settings
compute_mode=1
memory_timing=1
```

#### 3. Start AMD GPU Mining
```bash
gxc-ethash-miner -c ~/.gxc/amd-mining.conf
```

### GPU Overclocking

#### NVIDIA (using nvidia-smi)
```bash
# Set power limit
sudo nvidia-smi -pl 250

# Set GPU clock offset
sudo nvidia-settings -a "[gpu:0]/GPUGraphicsClockOffset[3]=100"

# Set memory clock offset
sudo nvidia-settings -a "[gpu:0]/GPUMemoryTransferRateOffset[3]=500"

# Set fan speed
sudo nvidia-settings -a "[gpu:0]/GPUFanControlState=1" -a "[fan:0]/GPUTargetFanSpeed=80"
```

#### AMD (using rocm-smi)
```bash
# Set GPU clock
sudo rocm-smi --setclocks 1500

# Set memory clock
sudo rocm-smi --setmemoverclock 2000

# Set fan speed
sudo rocm-smi --setfan 80
```

### Expected Hash Rates

| GPU Model | Memory | Expected Hash Rate |
|-----------|--------|-------------------|
| NVIDIA RTX 3060 | 12GB | ~25-30 MH/s |
| NVIDIA RTX 3070 | 8GB | ~40-50 MH/s |
| NVIDIA RTX 3080 | 10GB | ~60-75 MH/s |
| NVIDIA RTX 3090 | 24GB | ~90-110 MH/s |
| AMD RX 5700 XT | 8GB | ~50-55 MH/s |
| AMD RX 6800 XT | 16GB | ~60-70 MH/s |
| AMD RX 6900 XT | 16GB | ~65-75 MH/s |

---

## ASIC Mining (SHA-256)

### Supported ASIC Miners

- **Antminer S19 Series** (S19, S19 Pro, S19j Pro)
- **Antminer S21 Series**
- **WhatsMiner M30 Series** (M30S, M30S+, M31S)
- **WhatsMiner M50 Series**
- **AvalonMiner 1246**
- **Other SHA-256 compatible ASICs**

### Configuration

#### 1. Direct Pool Connection
```ini
# ASIC Configuration for GXC SHA-256 mining
algorithm=sha256
pool=stratum+tcp://asic-pool.gxc.network:3333
username=your_worker_name.asic
password=x
address=GXC1YOUR_ADDRESS

# ASIC-specific settings
frequency=650  # MHz
voltage=0.9    # Volts
fan_speed=80   # Percentage
```

#### 2. Antminer Web Interface Configuration

1. Access miner web interface: `http://MINER_IP_ADDRESS`
2. Navigate to **Miner Configuration**
3. Configure pools:
   - **Pool 1 URL**: `stratum+tcp://asic-pool.gxc.network:3333`
   - **Worker**: `your_worker_name.asic`
   - **Password**: `x`
4. Save and restart miner

#### 3. WhatsMiner Configuration

1. Access miner: `http://MINER_IP_ADDRESS`
2. Go to **Miner Configuration** → **Pool Settings**
3. Add pool:
   - **URL**: `stratum+tcp://asic-pool.gxc.network:3333`
   - **Worker**: `your_worker_name.asic`
   - **Password**: `x`
4. Apply settings

### ASIC Optimization

#### Frequency Tuning
- **Conservative**: 600-650 MHz (stable, lower power)
- **Balanced**: 650-700 MHz (good balance)
- **Aggressive**: 700-750 MHz (higher hash rate, more heat)

#### Voltage Tuning
- Start with default voltage
- Reduce by 0.05V increments
- Test stability for 24 hours
- Optimal: Lowest stable voltage

#### Cooling
- Maintain temperature: 60-75°C
- Fan speed: 70-90% (auto-adjust if available)
- Ensure adequate ventilation

### Expected Hash Rates

| ASIC Model | Hash Rate | Power Consumption |
|------------|-----------|------------------|
| Antminer S19 | 95 TH/s | 3250W |
| Antminer S19 Pro | 110 TH/s | 3250W |
| Antminer S19j Pro | 104 TH/s | 3068W |
| Antminer S21 | 200 TH/s | 3550W |
| WhatsMiner M30S | 100 TH/s | 3400W |
| WhatsMiner M30S+ | 112 TH/s | 3472W |
| WhatsMiner M50 | 126 TH/s | 3420W |

---

## Pool Mining Setup

### Choosing a Mining Pool

**Recommended Pools:**
- `stratum+tcp://pool.gxc.network:3333` (Official GXC Pool)
- `stratum+tcp://gpu-pool.gxc.network:4444` (GPU Pool)
- `stratum+tcp://asic-pool.gxc.network:3333` (ASIC Pool)

### Pool Configuration

#### Basic Pool Setup
```ini
# Pool Mining Configuration
pool=stratum+tcp://pool.gxc.network:3333
username=your_worker_name
password=x
address=GXC1YOUR_ADDRESS
```

#### Multiple Pools (Failover)
```ini
# Primary Pool
pool=stratum+tcp://pool.gxc.network:3333
username=worker1
password=x

# Backup Pool 1
backup_pool1=stratum+tcp://backup-pool1.gxc.network:3333
backup_username1=worker1
backup_password1=x

# Backup Pool 2
backup_pool2=stratum+tcp://backup-pool2.gxc.network:3333
backup_username2=worker1
backup_password2=x
```

### Pool Worker Names

Use descriptive worker names to track individual miners:
- `main_rig.gpu1`
- `farm_rig.asic1`
- `home_miner.cpu`

### Pool Statistics

Monitor your mining performance:
- **Pool Dashboard**: `https://pool.gxc.network/dashboard`
- **Worker Stats**: View hash rate, shares, earnings
- **Payout History**: Track payments

---

## Solo Mining Setup

### Requirements

Solo mining requires:
- **High hash rate** (recommended: >1% of network hash rate)
- **Stable connection** to blockchain node
- **Sufficient storage** for blockchain data

### Configuration

#### 1. Run Full Node
```bash
# Start GXC node
gxc-node -daemon -rpcuser=your_user -rpcpassword=your_password
```

#### 2. Configure Solo Mining
```ini
# Solo Mining Configuration
solo=true
node=http://localhost:8332
rpcuser=your_rpc_user
rpcpassword=your_rpc_password
address=GXC1YOUR_ADDRESS
algorithm=gxhash  # or ethash, sha256
```

#### 3. Start Solo Mining
```bash
gxc-miner --solo --node=http://localhost:8332 --rpcuser=user --rpcpassword=pass --address=GXC1YOUR_ADDRESS
```

### Solo Mining Benefits

- **No pool fees** (typically 1-2%)
- **Full block rewards** (12.5 GXC per block)
- **Direct control** over mining

### Solo Mining Risks

- **Variable payouts** (may go days without finding a block)
- **Higher variance** in earnings
- **Requires significant hash rate** to be profitable

---

## Performance Optimization

### System Optimization

#### Linux
```bash
# Set CPU governor to performance
sudo cpupower frequency-set -g performance

# Enable huge pages
sudo sysctl vm.nr_hugepages=128

# Increase file descriptor limits
echo "* soft nofile 65536" | sudo tee -a /etc/security/limits.conf
echo "* hard nofile 65536" | sudo tee -a /etc/security/limits.conf

# Disable CPU power saving
echo "performance" | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor
```

#### Windows
```powershell
# Set power plan to High Performance
powercfg /setactive 8c5e7fda-e8bf-4a96-9a85-a6e23a8c635c

# Disable Windows Defender for mining directory
Add-MpPreference -ExclusionPath "C:\GXC\Mining"
```

### Mining Software Optimization

#### CPU Mining
- Use all available cores
- Enable huge pages
- Set thread affinity
- Disable CPU power saving

#### GPU Mining
- Overclock GPU (safely)
- Increase memory clock
- Optimize power limit
- Monitor temperatures

#### ASIC Mining
- Tune frequency/voltage
- Optimize cooling
- Use latest firmware
- Monitor chip health

### Monitoring Tools

#### Built-in Monitoring
```bash
# View mining stats
gxc-miner --stats

# Real-time monitoring
watch -n 1 'gxc-miner --stats'
```

#### External Monitoring
- **HiveOS**: Farm management platform
- **Minerstat**: Mining monitoring
- **Awesome Miner**: Multi-algorithm miner manager

---

## Hardware Requirements

### CPU Mining (GXHash)

**Minimum:**
- CPU: 4 cores, 2.0 GHz
- RAM: 4GB
- Storage: 10GB

**Recommended:**
- CPU: 8+ cores, 3.0+ GHz
- RAM: 8GB+
- Storage: 50GB+ SSD

**Optimal:**
- CPU: 12+ cores, 3.5+ GHz (AMD Ryzen 9, Intel i9)
- RAM: 16GB+
- Storage: 100GB+ NVMe SSD

### GPU Mining (Ethash)

**Minimum:**
- GPU: 4GB VRAM (NVIDIA GTX 1050 Ti, AMD RX 570)
- RAM: 8GB
- Storage: 50GB
- PSU: 500W

**Recommended:**
- GPU: 8GB+ VRAM (NVIDIA RTX 3060, AMD RX 6800)
- RAM: 16GB
- Storage: 100GB SSD
- PSU: 750W+

**Optimal:**
- GPU: 10GB+ VRAM (NVIDIA RTX 3080/3090, AMD RX 6900 XT)
- RAM: 32GB
- Storage: 500GB+ NVMe SSD
- PSU: 1000W+

### ASIC Mining (SHA-256)

**Requirements:**
- ASIC Miner (Antminer, WhatsMiner, etc.)
- PSU: Compatible with miner (typically 2000-4000W)
- Cooling: Adequate ventilation
- Network: Stable internet connection

---

## Troubleshooting

### Common Issues

#### 1. "Cannot connect to pool"
**Solutions:**
- Check internet connection
- Verify pool URL and port
- Check firewall settings
- Try backup pool

#### 2. "Low hash rate"
**Solutions:**
- Check CPU/GPU utilization
- Verify overclocking settings
- Check for thermal throttling
- Update drivers/firmware

#### 3. "High rejected shares"
**Solutions:**
- Check network latency
- Verify pool connection
- Reduce overclocking
- Update mining software

#### 4. "GPU not detected"
**Solutions:**
- Install GPU drivers
- Verify CUDA/ROCm installation
- Check GPU in device manager
- Restart system

#### 5. "Out of memory (DAG generation)"
**Solutions:**
- Close other applications
- Reduce GPU memory usage
- Use GPU with more VRAM
- Update to latest miner version

### Debug Mode

Enable verbose logging:
```bash
gxc-miner --verbose --log-level=DEBUG
```

### Getting Help

- **Documentation**: `docs/MINING.md`
- **GitHub Issues**: https://github.com/gxc-blockchain/gxc-blockchain/issues
- **Discord**: https://discord.gg/gxc-blockchain
- **Forum**: https://forum.gxc.network

---

## Mining Software Tools

### Available Miners

#### 1. `gxc-miner` (Universal Miner)
- Supports all three algorithms
- GUI and CLI modes
- Pool and solo mining
- Real-time monitoring

**Usage:**
```bash
gxc-miner --algorithm=gxhash --address=GXC1ADDRESS --pool=POOL_URL
```

#### 2. `gxc-gxhash-miner` (CPU Miner)
- Optimized for GXHash algorithm
- Multi-threaded
- Low resource usage

**Usage:**
```bash
gxc-gxhash-miner -c ~/.gxc/mining.conf
```

#### 3. `gxc-ethash-miner` (GPU Miner)
- NVIDIA and AMD support
- DAG management
- Overclocking controls

**Usage:**
```bash
gxc-ethash-miner -c ~/.gxc/gpu-mining.conf
```

#### 4. `gxc-sha256-miner` (ASIC Miner)
- ASIC device support
- Stratum protocol
- Device management

**Usage:**
```bash
gxc-sha256-miner -c ~/.gxc/asic-mining.conf
```

### Python API

Use the Python API for programmatic mining:

```python
from gxc_api import GXCMiner

# Initialize miner
miner = GXCMiner()

# Start mining
miner.start_mining(algorithm="gxhash", threads=8)

# Set pool
miner.set_pool_info(
    url="stratum+tcp://pool.gxc.network:3333",
    username="worker1",
    password="x"
)

# Get stats
stats = miner.get_stats()
print(f"Hash Rate: {stats.hash_rate} H/s")
print(f"Uptime: {stats.uptime} seconds")

# Stop mining
miner.stop_mining()
```

### Command-Line Interface

#### Start Mining
```bash
# CPU mining
gxc-miner --algorithm=gxhash --address=GXC1ADDRESS --threads=8

# GPU mining
gxc-miner --algorithm=ethash --address=GXC1ADDRESS --devices=0,1

# Pool mining
gxc-miner --algorithm=gxhash --address=GXC1ADDRESS \
    --pool=stratum+tcp://pool.gxc.network:3333 \
    --username=worker1

# Solo mining
gxc-miner --algorithm=gxhash --address=GXC1ADDRESS \
    --solo --node=http://localhost:8332 \
    --rpcuser=user --rpcpassword=pass
```

#### View Statistics
```bash
# Current stats
gxc-miner --stats

# Device info
gxc-miner --devices

# Pool status
gxc-miner --pool-status
```

#### Configuration
```bash
# Save configuration
gxc-miner --save-config ~/.gxc/mining.conf

# Load configuration
gxc-miner --config ~/.gxc/mining.conf
```

---

## Mining Rewards

### Block Rewards

- **Current Reward**: 12.5 GXC per block
- **Block Time**: ~2.5 minutes (150 seconds)
- **Halving**: Every 4 years
- **Total Supply**: 31M GXC (hard cap)

### Reward Schedule

| Years | Block Reward | Annual Supply |
|-------|-------------|---------------|
| 0-4   | 12.5 GXC    | ~3.29M GXC    |
| 4-8   | 6.25 GXC    | ~1.64M GXC    |
| 8-12  | 3.125 GXC   | ~0.82M GXC    |
| 12-16 | 1.56 GXC    | ~0.41M GXC    |
| 16+   | < 1 GXC     | < 0.21M GXC   |

### Pool Fees

Most pools charge **1-2%** fee on mining rewards. Solo mining has **0% fees** but requires significant hash rate.

### Payouts

- **Pool Mining**: Automatic payouts (typically daily or when threshold reached)
- **Solo Mining**: Full block reward when block is found
- **Minimum Payout**: Varies by pool (typically 0.1-1.0 GXC)

---

## Security Best Practices

1. **Secure Your Wallet**: Use strong passwords and enable encryption
2. **Backup Wallet**: Regularly backup wallet.dat and mnemonic phrases
3. **Secure RPC**: Use strong RPC passwords, restrict access
4. **Firewall**: Only allow necessary ports
5. **Updates**: Keep mining software updated
6. **Monitoring**: Monitor for unauthorized access

---

## Additional Resources

- **Official Website**: https://gxc.network
- **Blockchain Explorer**: https://explorer.gxc.network
- **Documentation**: https://docs.gxc.network
- **GitHub**: https://github.com/gxc-blockchain/gxc-blockchain
- **Discord**: https://discord.gg/gxc-blockchain
- **Telegram**: https://t.me/gxc_blockchain

---

## Quick Reference

### Configuration File Locations

- **Linux**: `~/.gxc/mining.conf`
- **Windows**: `%APPDATA%\GXC\mining.conf`
- **macOS**: `~/Library/Application Support/GXC/mining.conf`

### Default Ports

- **RPC**: 8332
- **P2P**: 9333
- **Stratum Pool**: 3333
- **GPU Pool**: 4444
- **REST API**: 8080
- **WebSocket**: 8081

### Useful Commands

```bash
# Check node status
gxc-cli getinfo

# Get mining info
gxc-cli getmininginfo

# Get network hash rate
gxc-cli getnetworkhashps

# Get current difficulty
gxc-cli getdifficulty

# Get block count
gxc-cli getblockcount
```

---

**Happy Mining! 🚀**

For support, visit: https://support.gxc.network

