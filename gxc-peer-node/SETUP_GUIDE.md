# GXC Peer Node - Setup Guide

## Quick Setup for GitHub

### 1. Initialize Git Repository

```bash
cd gxc-peer-node
git init
git add .
git commit -m "Initial commit: GXC Peer Node in Rust"
```

### 2. Create GitHub Repository

Go to GitHub and create a new repository named `gxc-peer-node`.

### 3. Push to GitHub

```bash
git remote add origin https://github.com/YOUR_USERNAME/gxc-peer-node.git
git branch -M main
git push -u origin main
```

## Project Structure

```
gxc-peer-node/
├── src/
│   ├── main.rs              # CLI entry point
│   ├── p2p/
│   │   ├── mod.rs           # P2P module exports
│   │   ├── message.rs       # P2P message protocol
│   │   ├── peer.rs          # Peer connection handling
│   │   └── network.rs       # P2P network manager
│   ├── blockchain/
│   │   ├── mod.rs           # Blockchain module exports
│   │   ├── block.rs         # Block and transaction types
│   │   ├── validator.rs     # Block validation logic
│   │   └── sync.rs          # Blockchain synchronization
│   ├── rpc/
│   │   ├── mod.rs           # RPC module exports
│   │   └── client.rs        # RPC client for main node
│   └── config/
│       └── mod.rs           # Configuration management
├── .github/
│   └── workflows/
│       └── rust.yml         # CI/CD pipeline
├── Cargo.toml               # Rust dependencies
├── README.md                # Main documentation
├── LICENSE                  # MIT License
├── CONTRIBUTING.md          # Contribution guidelines
├── .gitignore               # Git ignore rules
└── SETUP_GUIDE.md           # This file
```

## Features Implemented

### ✅ P2P Networking
- Message protocol (JSON-based)
- Peer connection management
- Handshake and authentication
- Ping/pong keep-alive
- Block and transaction relay

### ✅ Blockchain Sync
- Sync from main node via RPC
- Block validation
- Work receipt verification
- Merkle root validation
- Proof-of-work verification

### ✅ RPC Client
- Connect to GXC main node
- Get blockchain info
- Fetch blocks
- Submit blocks
- Query balances

### ✅ CLI Interface
- Start node
- Sync blockchain
- Verify chain
- Show statistics
- Connect to peers
- Generate config

## Usage Examples

### Start Node

```bash
# With default config
cargo run -- start

# With custom config
cargo run -- --config config.toml start

# With CLI overrides
cargo run -- --network testnet --rpc-url http://localhost:18332 start
```

### Sync Blockchain

```bash
cargo run -- sync
```

### Verify Blockchain

```bash
cargo run -- verify
```

### Show Stats

```bash
cargo run -- stats
```

### Generate Config

```bash
cargo run -- init-config --output config.toml
```

## Configuration Example

```toml
[network]
name = "testnet"

[p2p]
listen_addr = "0.0.0.0"
listen_port = 18444
max_peers = 50
bootstrap_peers = [
    "node1.gxc.network:18333",
    "node2.gxc.network:18333"
]

[rpc]
node_url = "http://localhost:18332"

[node]
node_id = "gxc-peer-unique-id"
data_dir = "./data"
```

## Testing

### Run All Tests

```bash
cargo test
```

### Run Specific Test

```bash
cargo test test_work_receipt_computation
```

### Run with Logging

```bash
RUST_LOG=debug cargo test
```

## Building

### Debug Build

```bash
cargo build
```

Binary: `target/debug/gxc-peer-node`

### Release Build

```bash
cargo build --release
```

Binary: `target/release/gxc-peer-node`

### Cross-Compilation

```bash
# Linux
cargo build --release --target x86_64-unknown-linux-gnu

# Windows
cargo build --release --target x86_64-pc-windows-gnu

# macOS
cargo build --release --target x86_64-apple-darwin
```

## Deployment

### Systemd Service (Linux)

Create `/etc/systemd/system/gxc-peer-node.service`:

```ini
[Unit]
Description=GXC Peer Node
After=network.target

[Service]
Type=simple
User=gxc
WorkingDirectory=/opt/gxc-peer-node
ExecStart=/opt/gxc-peer-node/gxc-peer-node --config /etc/gxc/config.toml start
Restart=on-failure
RestartSec=10

[Install]
WantedBy=multi-user.target
```

Enable and start:

```bash
sudo systemctl enable gxc-peer-node
sudo systemctl start gxc-peer-node
sudo systemctl status gxc-peer-node
```

### Docker (Future)

```dockerfile
FROM rust:1.70 as builder
WORKDIR /app
COPY . .
RUN cargo build --release

FROM debian:bookworm-slim
COPY --from=builder /app/target/release/gxc-peer-node /usr/local/bin/
ENTRYPOINT ["gxc-peer-node"]
CMD ["start"]
```

## Monitoring

### Logs

```bash
# Follow logs
tail -f /var/log/gxc-peer-node.log

# With systemd
journalctl -u gxc-peer-node -f
```

### Metrics

Check peer count:
```bash
# TODO: Add metrics endpoint
```

## Troubleshooting

### Build Errors

```bash
# Update Rust
rustup update

# Clean build
cargo clean
cargo build
```

### Connection Issues

```bash
# Test RPC connection
curl -X POST http://localhost:18332/ \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getblockcount","params":[],"id":1}'

# Check port availability
netstat -tuln | grep 18444
```

### Sync Issues

```bash
# Check node is running
ps aux | grep gxc-node

# Verify RPC URL
cargo run -- --rpc-url http://localhost:18332 sync
```

## Next Steps

1. **Push to GitHub** - Share your peer node
2. **Run Tests** - Ensure everything works
3. **Deploy** - Set up on a server
4. **Monitor** - Watch logs and metrics
5. **Contribute** - Add features and improvements

## Resources

- [Rust Book](https://doc.rust-lang.org/book/)
- [Tokio Docs](https://tokio.rs/)
- [GXC Main Repo](https://github.com/philani1H/GXC-goldxcoin-blockchain-)

## Support

- **Issues**: GitHub Issues
- **Docs**: README.md
- **Community**: GXC Discord/Telegram

---

**Ready to push to GitHub!**

```bash
git init
git add .
git commit -m "Initial commit: GXC Peer Node"
git remote add origin https://github.com/YOUR_USERNAME/gxc-peer-node.git
git push -u origin main
```
