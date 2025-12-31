# GXC Peer Node

A lightweight peer node implementation for the GXC blockchain network, written in Rust.

## Features

- ✅ **P2P Networking** - Connect to other GXC nodes
- ✅ **Blockchain Sync** - Sync blockchain from main node
- ✅ **Block Validation** - Verify blocks and work receipts
- ✅ **RPC Client** - Communicate with GXC main node
- ✅ **CLI Interface** - Easy command-line management
- ✅ **Configurable** - TOML-based configuration

## Installation

### Prerequisites

- Rust 1.70+ (install from [rustup.rs](https://rustup.rs))
- GXC main node running (for blockchain sync)

### Build from Source

```bash
cd gxc-peer-node
cargo build --release
```

The binary will be at `target/release/gxc-peer-node`.

## Quick Start

### 1. Generate Config File

```bash
./gxc-peer-node init-config --output config.toml
```

This creates a default configuration file:

```toml
[network]
name = "testnet"

[p2p]
listen_addr = "0.0.0.0"
listen_port = 18444
max_peers = 50
bootstrap_peers = ["localhost:18333"]

[rpc]
node_url = "http://localhost:18332"

[node]
node_id = "gxc-peer-xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"
data_dir = "./data"
```

### 2. Edit Configuration

Edit `config.toml` to match your setup:

- **network.name**: `"mainnet"` or `"testnet"`
- **p2p.listen_port**: Port for P2P connections
- **p2p.bootstrap_peers**: List of peers to connect to
- **rpc.node_url**: URL of GXC main node

### 3. Start the Peer Node

```bash
./gxc-peer-node --config config.toml start
```

Or use CLI arguments:

```bash
./gxc-peer-node --network testnet --rpc-url http://localhost:18332 --port 18444 start
```

## Commands

### Start Node

Start the peer node and begin syncing:

```bash
gxc-peer-node start
```

### Sync Blockchain

Sync blockchain from main node without starting P2P:

```bash
gxc-peer-node sync
```

### Verify Blockchain

Verify blockchain integrity:

```bash
gxc-peer-node verify
```

### Show Statistics

Display blockchain statistics:

```bash
gxc-peer-node stats
```

Output:
```
╔════════════════════════════════════════╗
║       GXC Blockchain Statistics       ║
╠════════════════════════════════════════╣
║ Height:                          1234 ║
║ Total Transactions:              5678 ║
║ Avg Difficulty:                  1.50 ║
╚════════════════════════════════════════╝
```

### Connect to Peer

Connect to a specific peer:

```bash
gxc-peer-node connect 192.168.1.100:18333
```

## Configuration

### Network Configuration

```toml
[network]
name = "testnet"  # or "mainnet"
```

### P2P Configuration

```toml
[p2p]
listen_addr = "0.0.0.0"      # Listen on all interfaces
listen_port = 18444           # P2P port
max_peers = 50                # Maximum connected peers
bootstrap_peers = [           # Initial peers to connect to
    "node1.gxc.network:18333",
    "node2.gxc.network:18333"
]
```

### RPC Configuration

```toml
[rpc]
node_url = "http://localhost:18332"  # GXC main node RPC URL
```

### Node Configuration

```toml
[node]
node_id = "gxc-peer-unique-id"  # Unique node identifier
data_dir = "./data"              # Data directory
```

## Architecture

### Components

```
┌─────────────────────────────────────┐
│         CLI Interface               │
└──────────────┬──────────────────────┘
               │
    ┌──────────┴──────────┐
    │                     │
┌───▼────────┐    ┌──────▼──────┐
│ P2P Network│    │ Blockchain  │
│            │    │    Sync     │
│ - Messages │    │ - Validator │
│ - Peers    │    │ - Storage   │
│ - Network  │    │             │
└────────────┘    └──────┬──────┘
                         │
                  ┌──────▼──────┐
                  │ RPC Client  │
                  │             │
                  │ - getblock  │
                  │ - getinfo   │
                  │ - submit    │
                  └─────────────┘
```

### P2P Protocol

Messages are JSON-encoded and length-prefixed:

```
[4 bytes: length][N bytes: JSON message]
```

Message types:
- `Handshake` - Initial connection
- `GetBlocks` - Request blocks
- `Blocks` - Block data response
- `NewBlock` - Announce new block
- `GetPeers` - Request peer list
- `Ping/Pong` - Keep-alive

### Block Validation

Each block is validated for:

1. **Proof-of-Work** - Hash meets difficulty target
2. **Work Receipt** - Cryptographic proof of mining
3. **Merkle Root** - Transaction integrity
4. **Coinbase** - Reward amount and placement
5. **Chain Link** - Previous hash matches
6. **Timestamp** - Reasonable time range

## Development

### Run Tests

```bash
cargo test
```

### Run with Logging

```bash
RUST_LOG=debug cargo run -- start
```

Log levels: `error`, `warn`, `info`, `debug`, `trace`

### Build for Production

```bash
cargo build --release --target x86_64-unknown-linux-gnu
```

## API Reference

### RPC Client Methods

```rust
// Get blockchain info
let info = rpc_client.get_info().await?;

// Get block count
let height = rpc_client.get_block_count().await?;

// Get block by height
let block = rpc_client.get_block(100).await?;

// Get balance
let balance = rpc_client.get_balance("GXC...").await?;

// Submit block
rpc_client.submit_block(&block).await?;
```

### P2P Network Methods

```rust
// Connect to peer
p2p.connect_to_peer("192.168.1.100", 18333).await?;

// Broadcast message
p2p.broadcast(&message).await?;

// Get peer count
let count = p2p.peer_count().await;

// Request blocks
p2p.request_blocks(start_height, count).await?;
```

## Troubleshooting

### Connection Refused

```
Error: Failed to connect to peer: Connection refused
```

**Solution:** Ensure the main node is running and accessible.

### Sync Fails

```
Error: Failed to sync blockchain: RPC error
```

**Solution:** Check RPC URL in config and ensure node is responding:

```bash
curl -X POST http://localhost:18332/ \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getblockcount","params":[],"id":1}'
```

### Port Already in Use

```
Error: Failed to bind listener: Address already in use
```

**Solution:** Change `listen_port` in config or stop other process using the port.

## Security

### Network Security

- Validates all incoming blocks
- Verifies work receipts
- Checks proof-of-work
- Limits message sizes (10MB max)

### Best Practices

1. **Firewall**: Only expose P2P port, not RPC
2. **TLS**: Use TLS for RPC connections in production
3. **Monitoring**: Monitor peer connections and sync status
4. **Updates**: Keep node software updated

## Performance

### Resource Usage

- **Memory**: ~50-100MB base + blockchain data
- **CPU**: Low (validation only)
- **Network**: Depends on peer count and sync status
- **Disk**: Blockchain size (~1GB per year estimated)

### Optimization

- Use `--release` build for production
- Limit `max_peers` for lower bandwidth
- Use SSD for faster blockchain access

## Contributing

Contributions welcome! Please:

1. Fork the repository
2. Create a feature branch
3. Write tests for new features
4. Submit a pull request

## License

MIT License - see LICENSE file

## Support

- **Issues**: [GitHub Issues](https://github.com/philani1H/GXC-goldxcoin-blockchain-/issues)
- **Docs**: [GXC Documentation](../README.md)
- **Community**: GXC Discord/Telegram

## Roadmap

- [ ] Persistent blockchain storage (RocksDB)
- [ ] Mempool management
- [ ] Transaction relay
- [ ] Peer reputation system
- [ ] Metrics and monitoring
- [ ] WebSocket API
- [ ] Docker support

---

**Version:** 0.1.0  
**Last Updated:** December 31, 2025  
**Author:** GXC Development Team
