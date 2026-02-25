# Security Policy

## Supported Versions

| Version | Supported          |
|---------|--------------------|
| 2.x     | ✅ Active support  |
| 1.x     | ❌ End of life     |

## Reporting a Vulnerability

**Please do NOT open a public GitHub issue for security vulnerabilities.**

Send a detailed report to: **security@goldxcoin.network**

Include:
- A clear description of the vulnerability
- The affected component and file(s)
- Steps to reproduce
- Potential impact (funds at risk, network disruption, etc.)
- A suggested fix if you have one

We will acknowledge your report within **48 hours** and work with you on a
responsible disclosure timeline. Credit will be given in the release notes
unless you prefer to remain anonymous.

## Security Architecture

GXC-CORE implements multiple layers of security:

1. **Cryptographic Layer** — secp256k1 ECDSA, SHA-256, Keccak-256, Blake2b,
   Argon2id, RIPEMD-160
2. **Consensus Layer** — Hybrid PoW/PoS with multi-algorithm support
3. **Fraud Detection** — Mathematical taint propagation with five detection rules
4. **Reversal System** — Proof of Feasibility with admin/protocol separation
5. **AI Security Engine** — Predictive hashrate sentinel and attack detection
6. **Database Isolation** — Mainnet/testnet network mode guard
7. **Traceability** — Cryptographic transaction chaining (POT)
