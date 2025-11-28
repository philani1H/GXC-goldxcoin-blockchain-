# GXC GXHash Miner - Standalone Package

## Installation

### Windows
1. Double-click `install.bat`
2. Or run manually:
   ```
   pip install -r requirements.txt --user
   python gxhash_miner.py
   ```

### Linux/macOS
1. Run: `bash install.sh`
2. Or run manually:
   ```
   pip3 install -r requirements.txt --user
   python3 gxhash_miner.py
   ```

## Usage

```bash
python gxhash_miner.py --rpc-url http://localhost:8545
```

## Options

- `--rpc-url URL` - Blockchain RPC URL (default: http://localhost:8545)
- `--rest-url URL` - Blockchain REST URL (default: http://localhost:8080)
- `--threads N` - Number of mining threads (0 = auto-detect)
- `--address ADDRESS` - Miner address for rewards
- `--daemon` - Run in background mode

## Requirements

- Python 3.7 or higher
- GXC blockchain node running
- RPC enabled on node (port 8545)
