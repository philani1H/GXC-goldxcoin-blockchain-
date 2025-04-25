# Contributing to GXC Blockchain

Thank you for your interest in contributing to the GXC project! We welcome developers, researchers, auditors, and enthusiasts to help us build a secure, transparent, and adaptive hybrid Proof-of-Work blockchain with gold-backed tokens and on-chain price oracles. This document outlines how you can get involved.

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
  - Implement dual-PoW consensus improvements (SHA-256 & Ethash)  
  - Enhance gold-token tracking, redemption, or proof-of-price oracle modules  
  - Tune adaptive monetary parameters & simulation tooling  
  - Write new smart contracts or audit existing ones  

- **Documentation & Examples**  
  - Improve or expand the white paper and technical specs  
  - Add tutorials: running nodes, staking oracles, issuing GXC-G  
  - Create diagrams for consensus, transaction flows, oracle architecture  

- **Testing & QA**  
  - Write unit, integration, and fuzz tests  
  - Set up continuous integration / continuous deployment (CI/CD)  
  - Perform security audits and performance benchmarks  

- **Community & Support**  
  - Answer questions on Discord, Reddit, and GitHub Discussions  
  - Review issues and help triage bug reports  
  - Translate docs into other languages  

---

## Getting Started

### Prerequisites

- **Development Environment**  
  - Linux or macOS (Ubuntu 20.04+ recommended)  
  - Git >= 2.25  
  - Node.js & npm (for any JavaScript tooling)  
  - Rust (if consensus code is in Rust) or Go/C++ toolchain as appropriate  
  - Docker & Docker Compose (optional, for quick testnet setup)

- **Blockchain Tools**  
  - PostgreSQL or other supported database (for indexer)  
  - Solidity compiler (for smart contracts)  
  - Truffle or Hardhat (for PoP and GXC-G contracts)  

### Clone & Build

```bash
# Fork the repo on GitHub: https://github.com/philchade/gxc
git clone https://github.com/<your-username>/gxc.git
cd gxc

# Install dependencies
npm install        # or `yarn install`
# Build node software
make                # or `cargo build --release`, or `go build ./cmd/gxc/...`
