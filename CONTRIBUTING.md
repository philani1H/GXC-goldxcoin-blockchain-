# Contributing to GXC Blockchain

Thank you for your interest in contributing to the GXC project! We welcome developers, researchers, auditors, and enthusiasts to help us build a secure, transparent, and adaptive hybrid Proof-of-Work blockchain with gold-backed coins and on-chain price oracles. This document outlines how you can get involved.

---

## Table of Contents

1. [Code of Conduct](#code-of-conduct)  
2. [How Can I Contribute?](#how-can-i-contribute)  
3. [Getting Started](#getting-started)  
   - [Prerequisites](#prerequisites)  
   - [Clone & Build](#clone--build)  
4. [Development Workflow](#development-workflow)  
   - [Branching Model](#branching-model)  
   - [Commit Messages](#commit-messages)  
   - [Pull Requests](#pull-requests)  
5. [Running a GXC Node & Testnet](#running-a-gxc-node--testnet)  
6. [Oracle Node Setup (PoP Providers)](#oracle-node-setup-pop-providers)  
7. [Smart Contract Development (GXC-G & PoP)](#smart-contract-development-gxc-g--pop)  
8. [Testing & Simulation](#testing--simulation)  
9. [Style & Quality Guidelines](#style--quality-guidelines)  
10. [Reporting Bugs & Requesting Features](#reporting-bugs--requesting-features)  
11. [Community & Support](#community--support)  
12. [License](#license)  

---

## Code of Conduct

We follow the [Contributor Covenant Code of Conduct](https://www.contributor-covenant.org/). Please read it and be respectful to all community members.  
— Be courteous, inclusive, and supportive.  
— No harassment or abusive language.  
— Report unacceptable behavior to the maintainers.

---

## How Can I Contribute?

You can contribute in many ways:

- **Code & Features**  
  - Improve dual-PoW consensus (SHA-256 & Ethash)  
  - Enhance gold-coin tracking, redemption, or PoP oracle modules  
  - Tune adaptive monetary parameters & simulation tooling  
  - Write or audit smart contracts (GXC-G, PoP aggregation)  

- **Documentation & Examples**  
  - Expand the white paper and technical specs  
  - Add tutorials: running nodes, staking oracles, issuing gold-backed coins  
  - Create architecture diagrams and sample code snippets  

- **Testing & QA**  
  - Develop unit, integration, and fuzz tests  
  - Configure continuous integration (CI) pipelines  
  - Perform security audits and performance benchmarks  

- **Community & Support**  
  - Answer questions on Discord, Reddit, and GitHub Discussions  
  - Review issues and help triage bug reports  
  - Translate docs into other languages  

---

## Getting Started

### Prerequisites

- **Operating System:** Linux or macOS (Ubuntu 20.04+ recommended)  
- **Tools:**  
  - Git >= 2.25  
  - Node.js & npm (for JS tooling)  
  - **C++17 toolchain** (GCC 9+/Clang 10+) with CMake >= 3.10  
  - Docker & Docker Compose (optional, for local testnet)  
- **Blockchain Dependencies:**  
  - PostgreSQL or another supported database (indexer)  
  - Solidity compiler (0.8.x)  
  - Hardhat or Truffle (for smart contracts)

### Clone & Build

```bash
# Fork the repository on GitHub:
# https://github.com/philchade/gxc

git clone https://github.com/<your-username>/gxc.git
cd gxc

# Install JS tooling (if applicable)
npm install

# Build the node software (C++17)
cd core
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make

# Return to project root\ ncd ../..
```

```bash
# Fork the repository on GitHub:
# https://github.com/philchade/gxc

git clone https://github.com/<your-username>/gxc.git
cd gxc

# Install JS tooling (if applicable)
npm install

# Build the node software (Rust example)
cd core
cargo build --release

# Or, for Go/C++:
# cd core && make build

# Return to project root for smart contracts and tools
cd ../
```

---

## Development Workflow

### Branching Model

We use the **GitFlow** model:

- `master` branch: production-ready code  
- `develop` branch: latest integrated changes  
- Feature branches: `feature/<short-description>` from `develop`  
- Release branches: `release/vX.Y.Z` from `develop`  
- Hotfix branches: `hotfix/<short-description>` from `master`

### Commit Messages

Follow **Conventional Commits**:
```
<type>(<scope>): <short summary>

[optional body]

[optional footer]
```
- **type**: `feat`, `fix`, `docs`, `style`, `refactor`, `perf`, `test`, `chore`  
- **scope**: package or module name  
- Keep `<short summary>` under 72 characters  

### Pull Requests

1. Fork `philchade/gxc` and push your branch.  
2. Open a PR against `develop`.  
3. Provide a clear description and reference related issues.  
4. Ensure all tests pass and CI checks are green.  
5. Address maintainers’ feedback promptly.

---

## Running a GXC Node & Testnet

```bash
# From project root:
cd core
cargo run --release -- --chain testnet --port 30333
```

- Data directory: `~/.gxc-testnet`  
- RPC endpoint: `http://127.0.0.1:8545`  
- Monitor logs for block production and peer connections.

To simulate difficulty and PoP updates:
```bash
# In separate terminal:
cd tools/test-faucet
docker-compose up
```

---

## Oracle Node Setup (PoP Providers)

1. Clone the PoP oracle module:
   ```bash
   git clone https://github.com/philchade/gxc-oracles.git
   cd gxc-oracles
   ```
2. Install dependencies:
   ```bash
   npm install
   ```
3. Configure `.env` with your API keys and node endpoint:
   ```ini
   NODE_URL=http://localhost:8545
   ORACLE_ID=<your-node-id>
   PRIVATE_KEY=<your-wallet-key>
   DATA_SOURCES=LBMA,COMEX,SGE
   ```
4. Run the provider:
   ```bash
   npm start
   ```
5. Your node will submit signed price points every 6 hours or upon significant moves.

---

## Smart Contract Development (GXC-G & PoP)

1. Navigate to contracts:
   ```bash
   cd contracts
   ```
2. Install Hardhat and dependencies:
   ```bash
   npm install
   ```
3. Compile and test:
   ```bash
   npx hardhat compile
   npx hardhat test
   ```
4. Deploy to local testnet or public testnet:
   ```bash
   npx hardhat run scripts/deploy.js --network testnet
   ```
5. Verify on Etherscan (if applicable) or your explorer.

---

## Testing & Simulation

- **Unit Tests**:  
  ```bash
  # Core node (Rust)
  cd core
  cargo test
  
  # Contracts & JS modules
  cd ../contracts
  npm test
  ```
- **Integration Tests**: Use our `integration-tests` suite:
  ```bash
  cd integration-tests
  ./run_integration.sh
  ```
- **Fuzz Testing**: Employ `cargo-fuzz` or `foundry` for smart contracts.

---

## Style & Quality Guidelines

- **Linting**:  
  - Rust: `cargo fmt && cargo clippy`  
  - JavaScript: `npm run lint` (ESLint & Prettier)  
- **Documentation Comments**: Use Rustdoc or JSDoc for public APIs.  
- **Coverage**: Aim for ≥90% code coverage.  
- **Security**: Run `cargo audit` for Rust and `npm audit` for JS.

---

## Reporting Bugs & Requesting Features

Please use GitHub Issues:

- **Bug Report**: Choose the **bug** template, include steps to reproduce, logs, environment.  
- **Feature Request**: Use the **enhancement** template, describe motivation and use cases.  

---

## Community & Support

- **Discord**: https://discord.gg/gxc-blockchain  
- **GitHub Discussions**: https://github.com/philchade/gxc/discussions  
- **Reddit**: r/CryptoCurrency, r/altcoin  
- **Mailing List**: Subscribe at [gxc@lists.philchade.com](mailto:gxc@lists.philchade.com)

---

## License

This project is licensed under the **MIT License**. See [LICENSE](LICENSE.md) for details.

