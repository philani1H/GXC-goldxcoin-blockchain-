# GXC System Pool - Self-Sustaining Fraud Detection

## What is the System Pool?

The System Pool is a **blockchain-level wallet** that automatically accumulates funds to support the fraud detection and reversal system. It's **not a mining pool** - it's part of the blockchain's economic design.

## How It Works

### Block Rewards

Miners receive **100% of the block reward** (e.g., 50 GXC per block).

The system pool does **NOT** take a cut of block rewards.

### Transaction Fee Split

**Only transaction fees are split:**
- **85% to Miner**: Included in block reward
- **15% to System Pool**: Accumulated for fraud operations

**Example:**
- Block reward: 50 GXC → 100% to miner
- Transaction fees: 10 GXC total
  - Miner receives: 8.5 GXC (85%)
  - System Pool receives: 1.5 GXC (15%)
- **Total miner payment: 58.5 GXC**

## System Pool Addresses

The pool uses deterministic addresses generated from a known seed:

**Testnet:**
```
tGXC2a9d9ddb2e9ee658bca1c2ff41ffed99
```

**Mainnet:**
```
GXC2a9d9ddb2e9ee658bca1c2ff41ffed99
```

These addresses are hardcoded in the blockchain and cannot be changed without a protocol upgrade.

## What Are The Funds Used For?

1. **Fraud Reversals**: When fraud is detected and verified, the pool pays for:
   - Transaction reversals
   - Victim compensation
   - Network security operations

2. **Self-Sustaining Model**: The blockchain doesn't need external funding for fraud protection

3. **Transparent Operations**: All pool transactions are on-chain and auditable

## Configuration

The split percentage can be configured:

```cpp
// In gxc_config.json
{
  "fee_pool_split_percentage": 0.15  // 15% default
}
```

## Checking Pool Balance

**Via RPC:**
```bash
curl -X POST https://your-node.com/api/getbalance \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","id":1,"method":"getbalance","params":["tGXC2a9d9ddb2e9ee658bca1c2ff41ffed99"]}'
```

**Via Python:**
```python
import requests

response = requests.post(
    "https://your-node.com/api/getbalance",
    json={
        "jsonrpc": "2.0",
        "id": 1,
        "method": "getbalance",
        "params": ["tGXC2a9d9ddb2e9ee658bca1c2ff41ffed99"]
    }
)
print(f"System Pool Balance: {response.json()['result']} GXC")
```

## Why This Design?

### Traditional Approach (Problematic)
- External funding required for fraud protection
- Centralized control of funds
- Sustainability issues

### GXC Approach (Better)
- ✅ Self-sustaining from block rewards
- ✅ Decentralized and transparent
- ✅ Predictable funding model
- ✅ No external dependencies

## Economic Impact

**For Miners:**
- **Block rewards**: 100% to miner (no reduction)
- **Transaction fees**: 85% to miner, 15% to pool
- Network is more secure and valuable
- Long-term: Higher coin value from fraud protection

**For Network:**
- Sustainable fraud protection funded by transaction fees
- No need for external funding
- Transparent operations
- Predictable economics
- Block rewards remain fully incentivized

## Governance

The system pool is controlled by:
1. **Smart Contracts**: Automated fraud detection
2. **Admin API**: Manual intervention for complex cases
3. **On-Chain Voting**: Future governance mechanism

## Comparison to Other Blockchains

| Feature | GXC | Bitcoin | Ethereum |
|---------|-----|---------|----------|
| Fraud Protection | Built-in, funded | None | Limited |
| Reversal Capability | Yes | No | No |
| Self-Sustaining | Yes | N/A | N/A |
| Pool Transparency | On-chain | N/A | N/A |

## FAQ

**Q: Is this a mining pool taking a cut?**
A: No. This is a blockchain protocol feature, not a third-party service. Block rewards are 100% to miners.

**Q: Does this reduce my mining rewards?**
A: No. Block rewards (50 GXC) go 100% to miners. Only transaction fees are split 85/15.

**Q: Can I opt out?**
A: No. The fee split is part of the consensus rules, like transaction fees themselves.

**Q: Who controls the pool funds?**
A: The blockchain protocol and admin system, with full transparency.

**Q: What if the pool runs out of funds?**
A: It continuously accumulates from every block, making depletion unlikely.

**Q: Can the split percentage change?**
A: Only through a protocol upgrade requiring network consensus.

## Technical Details

### Coinbase Transaction Structure

Block rewards go 100% to miner:

```json
{
  "outputs": [
    {
      "address": "tGXC9fab7317231b966af85ac453e168c0932",
      "amount": 50.0,
      "script": "OP_DUP OP_HASH160 ... OP_CHECKSIG"
    }
  ]
}
```

Transaction fee splits are handled separately during block processing.

### Code Location

- **Pool Initialization**: `src/Blockchain.cpp:24-27`
- **Reward Split**: `src/RPCAPI.cpp:2617-2638`
- **Fee Split**: `src/Blockchain.cpp:645-656`

## Monitoring

Track pool accumulation:
```bash
# Check current balance
./gxc-cli getbalance tGXC2a9d9ddb2e9ee658bca1c2ff41ffed99

# View pool transactions
./gxc-cli listtransactions tGXC2a9d9ddb2e9ee658bca1c2ff41ffed99
```

## Conclusion

The System Pool is a key innovation that makes GXC's fraud detection system financially sustainable without requiring external funding or centralized control. It's a small cost (15%) for a major benefit (fraud protection).
