# GXC Blockchain Whitepaper (Technical Draft)

## Abstract
GXC is a hybrid Proof-of-Work (PoW) and Proof-of-Stake (PoS) blockchain designed to provide auditable transaction traceability, predictable monetary issuance, and protocol-level safeguards for network health. The system alternates PoW and PoS blocks, supports multiple PoW algorithms (SHA-256, Ethash, and GXHASH), and enforces a deterministic traceability formula for transaction chaining. This document summarizes the on-chain mechanics and math as implemented in the C++ codebase.

## 1. Design Goals
1. **Security and liveness** through alternating PoW/PoS consensus.
2. **Traceability** via explicit transaction chaining and work receipts.
3. **Economic predictability** with a capped supply and halving schedule.
4. **Practical throughput** with bounded block size and configurable target block time.

## 2. System Parameters (Defaults)
* **Target block time:** 600 seconds (10 minutes).【F:src/Config.cpp†L23-L70】
* **Difficulty adjustment interval:** 2016 blocks (~2 weeks at target time).【F:include/blockchain.h†L305-L312】
* **Max block size:** 1,048,576 bytes (1 MB).【F:src/Config.cpp†L23-L70】
* **Initial block reward:** 50 GXC.【F:src/Config.cpp†L23-L70】
* **Max supply:** 31,000,000 GXC.【F:include/blockchain.h†L305-L312】

## 3. Data Structures
### 3.1 Block Header and Hashing
A block hash is computed from core header fields (index, previous hash, timestamp, merkle root, nonce). The hashing algorithm depends on block type:

* PoW SHA-256: `sha256d(header)`
* PoW Ethash: `ethash(header, nonce)`
* PoW GXHASH: `gxhash(header, nonce)`
* PoS: `sha256(header)`

This is encoded in the block hash calculation logic.【F:src/block.cpp†L17-L55】

### 3.2 Merkle Root
The block merkle root is computed over all transaction hashes in the block. It is re-derived whenever transactions are added or modified.【F:src/block.cpp†L76-L111】

### 3.3 UTXO Model
GXC uses a UTXO set for tracking unspent transaction outputs. The blockchain rebuilds the UTXO set by iterating over all blocks on load and applying inputs/outputs in order.【F:src/Blockchain.cpp†L768-L834】

### 3.4 Transactions and Signatures
Transactions contain inputs and outputs and are hashed with Keccak-256 over all fields (inputs, outputs, metadata, and flags). Coinbase transactions have no inputs and a single output.【F:src/Transaction.cpp†L10-L70】

For non-coinbase transactions, each input must carry an ECDSA signature over `(txHash || outputIndex || amount)` and a public key, which are verified during transaction validation.【F:src/Transaction.cpp†L228-L277】

### 3.5 Wallets and Address Format
Wallets generate secp256k1 keypairs and derive addresses from the compressed public key. Address derivation uses `SHA-256` followed by `RIPEMD-160`, and prefixes with `GXC` (mainnet) or `tGXC` (testnet).【F:src/Wallet.cpp†L12-L39】【F:src/Crypto.cpp†L328-L349】

## 4. Consensus
GXC uses **hybrid consensus** that alternates PoW and PoS blocks. Even heights are PoW; odd heights are PoS when active validators exist, otherwise PoW is used as a fallback.【F:src/Blockchain.cpp†L1409-L1459】

### 4.1 Proof-of-Work (PoW)
A PoW block must satisfy a leading-zero target based on the network difficulty. For a hash `H` and difficulty `D`:

**PoW condition:**
```
leading_zeros(H) ≥ D
```

Difficulty is enforced with a minimum floor (testnet: 1.0, mainnet: 1000.0). The block’s declared difficulty must match the network’s current difficulty, otherwise the block is rejected.【F:src/Blockchain.cpp†L1206-L1257】

### 4.2 Proof-of-Stake (PoS)
PoS blocks are signed by an active validator and must pass a minimal hash target check. Validators must satisfy minimum stake and staking duration constraints, and the block must include a valid validator signature.【F:src/Blockchain.cpp†L1299-L1357】【F:src/Validator.cpp†L8-L74】

PoS target (simplified):
```
meetsTarget(hash, 0.01)
```
This makes PoS blocks much easier than PoW while still requiring a valid hash.【F:src/Blockchain.cpp†L1343-L1357】

### 4.3 Validator Selection
Validators are selected proportionally to weighted stake:

**Weighted stake:**
```
w_i = stake_i * (days_i / 365) ^ β
```
with β = 0.5, staking days in [14, 365].【F:src/Validator.cpp†L8-L63】

A deterministic RNG seeded by block height selects validators proportional to `w_i / Σ w`.【F:src/Blockchain.cpp†L1360-L1407】

### 4.4 PoW Hashing Primitives
GXC implements SHA-256 (single and double), RIPEMD-160, Keccak-256, and an Ethash-style function for PoW hashing. The Ethash implementation includes cache generation, dataset mixing, and a Keccak-256 final hash over the mix, providing memory-hard work for mining configurations that select Ethash.【F:src/HashUtils.cpp†L1-L194】

## 5. Difficulty Adjustment
The implementation uses two mechanisms:

1. **Security-engine adjustment** on the configured interval (default from config). This uses an internal security engine to compute a new difficulty and clamps it to network-specific bounds.【F:src/Blockchain.cpp†L531-L570】
2. **Bitcoin-style interval formula** (for reference in code), which adjusts difficulty based on actual vs. expected time and clamps changes to 4× or 1/4× per interval.【F:src/Blockchain.cpp†L724-L756】

### 5.1 Bitcoin-style Formula (Reference)
Let `D_t` be current difficulty, `T_actual` the time for the last interval, and `T_expected = interval * target_block_time`:

```
D_{t+1} = D_t * (T_expected / T_actual)
```

Then clamp:
```
D_{t+1} ∈ [D_t / 4, 4 * D_t]
```
【F:src/Blockchain.cpp†L724-L756】

## 6. Monetary Policy
### 6.1 Halving Schedule
Block rewards follow a halving schedule every 1,051,200 blocks (~4 years at 10-minute blocks).【F:src/Blockchain.cpp†L640-L673】

### 6.2 Reward Formula
Let:
* `R0 = 50` GXC (initial reward)
* `h` = block height
* `H = floor(h / 1,051,200)` (halving count)
* `D` = network difficulty

**Base reward:**
```
R_base = R0 / 2^H
```

**Difficulty multiplier** (max +10%):
```
M_diff = 1 + min(0.1, (D - 1) / 100)
```

**Time bonus** (up to +5% if average block time is too slow):
```
M_time = 1 + min(0.05, (avg_time - target_time) / target_time)
```

**Final issuance (pre-fees):**
```
R = R_base * M_diff * M_time
```

Supply is capped to 31M GXC, and any remaining issuance is reduced accordingly. Transaction fees are added on top of issuance (fees are not capped).【F:src/Blockchain.cpp†L640-L713】【F:include/blockchain.h†L305-L312】

## 7. Transaction Fees and System Pool
Fees are split between miners and a system pool. Default fee pool split is 15%, where that portion becomes a real UTXO sent to a system pool address to fund reversal/fraud handling. The split is configurable via `fee_pool_split_percentage`.【F:src/Blockchain.cpp†L589-L650】

## 8. Traceability and Work Receipts
### 8.1 Traceability Formula
For non-coinbase, non-genesis transactions, GXC enforces:

```
Inputs[0].txHash == PrevTxHash
Inputs[0].amount == ReferencedAmount
```

Transactions that fail the traceability formula are rejected.【F:src/Transaction.cpp†L72-L140】

### 8.2 Work Receipt
Each PoW block includes a work receipt hash to bind PoW to the coinbase transaction:

```
WorkReceipt = H(prev_hash || merkle_root || nonce || miner_pubkey || difficulty || timestamp)
```

The work receipt must match both the block header and the coinbase transaction, and the coinbase block height must match the block index. This ensures mined rewards are cryptographically linked to the proof-of-work that produced the block.【F:src/block.cpp†L57-L75】【F:src/Blockchain.cpp†L1259-L1296】

## 9. Security and Validation Pipeline
Block validation includes:
1. Previous-hash linkage.
2. Consensus verification (PoW or PoS).
3. Work receipt validation for PoW blocks.
4. Per-transaction validation and traceability checks.

This pipeline ensures chain integrity, preventable double spends, and traceability of issuance and transfers.【F:src/Blockchain.cpp†L1120-L1296】

## 10. Staking Pool and Validator Requirements
Staking is tracked in a LevelDB-backed pool. Stakes must be within minimum and maximum lock periods and above minimum amounts. Staking entries record traceability inputs to preserve on-chain auditability of locked funds.【F:src/StakingPool.cpp†L1-L110】

## 11. Network Interfaces (Default Ports)
* P2P network: 9333
* RPC: 8332
* REST: 8080

These are configurable via `Config` defaults.【F:src/Config.cpp†L23-L70】

## 12. P2P Networking Overview
The node runs a TCP listener on the configured network port, accepts inbound peers, and runs separate threads for server accept, peer management, and message processing. Peer health checks occur periodically, and the network layer maintains the desired number of peers over time.【F:src/Network.cpp†L1-L120】【F:src/Network.cpp†L150-L191】

## 13. Summary
GXC combines hybrid consensus, deterministic traceability rules, and a capped issuance model with configurable governance and security components. The architecture emphasizes auditability and protocol-level integrity by linking block rewards to work receipts and enforcing transaction chaining constraints.

---

*This document reflects the current implementation in the C++ codebase and should be updated alongside future protocol changes.*
