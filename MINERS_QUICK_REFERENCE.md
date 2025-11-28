# GXC Miners - Quick Reference

Quick guide to choose the right miner for your hardware.

---

## Which Miner Do I Need?

### 🖥️ **I have a CPU only**
→ Use **gxc-gxhash-miner** or **gxhash_miner.py** (Python GUI)
- Works on any computer
- No special hardware needed
- GXC's native algorithm

### 🎮 **I have a GPU (NVIDIA/AMD)**
→ Use **gxc-ethash-miner**
- Best performance for GPUs
- Ethereum-compatible
- Supports CUDA (NVIDIA) and OpenCL (AMD)

### ⚡ **I have ASIC hardware**
→ Use **gxc-sha256-miner**
- Bitcoin-compatible
- Optimized for ASICs
- Highest hash rates

### 🔄 **I want one miner for everything**
→ Use **gxc-miner** (Universal)
- Supports all algorithms
- Auto-detects best algorithm
- Simplest option

---

## Algorithm Comparison

| Algorithm | Hardware | Performance | Compatibility |
|-----------|----------|-------------|---------------|
| **GXHash** | CPU | Good | GXC native |
| **Ethash** | GPU | Excellent | Ethereum |
| **SHA-256** | ASIC | Best | Bitcoin |

---

## Installation

All miners are included in one package:

**Download:** `gxc-miners-cli-*.zip` or `.tar.gz`

**Contains:**
- ✅ gxc-sha256-miner
- ✅ gxc-ethash-miner
- ✅ gxc-gxhash-miner
- ✅ gxc-miner (universal)
- ✅ gxc-node (blockchain node)

**Install:**
```bash
# Extract package
tar -xzf gxc-miners-cli-linux.tar.gz
cd gxc-miners-cli-linux

# Install
bash install.sh

# Done! All miners ready to use.
```

---

## Quick Start

### CPU Mining (GXHash)
```bash
./gxc-gxhash-miner --rpc-url=http://localhost:8545
```

### GPU Mining (Ethash)
```bash
./gxc-ethash-miner --gpu --rpc-url=http://localhost:8545
```

### ASIC Mining (SHA-256)
```bash
./gxc-sha256-miner --device=0
```

### Universal (All Algorithms)
```bash
./gxc-miner --address=YOUR_ADDRESS
```

---

## All Miners Are Installable! ✅

- ✅ SHA-256 miner (ASIC)
- ✅ Ethash miner (GPU)
- ✅ GXHash miner (CPU)
- ✅ Universal miner (All)
- ✅ Python GUI miner

**Download once, get all miners!**
