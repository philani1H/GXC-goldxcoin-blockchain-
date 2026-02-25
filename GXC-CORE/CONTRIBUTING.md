# Contributing to GXC-CORE

Thank you for your interest in contributing to GoldXCoin! This document outlines
the process for contributing code, documentation, and bug reports.

---

## Table of Contents

1. [Code of Conduct](#code-of-conduct)
2. [Getting Started](#getting-started)
3. [How to Contribute](#how-to-contribute)
4. [Development Guidelines](#development-guidelines)
5. [Pull Request Process](#pull-request-process)
6. [Security Vulnerabilities](#security-vulnerabilities)

---

## Code of Conduct

Be respectful, constructive, and professional. All contributors are expected to
maintain a welcoming environment for everyone regardless of background or experience.

---

## Getting Started

### Prerequisites

```bash
# Ubuntu / Debian
sudo apt-get install -y build-essential cmake libssl-dev libleveldb-dev \
                        libsnappy-dev pkg-config git

# macOS
brew install cmake openssl leveldb snappy

# Windows (vcpkg)
vcpkg install openssl leveldb snappy
```

### Build

```bash
git clone https://github.com/GoldXCoin/GXC-CORE.git
cd GXC-CORE
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
```

### Run tests

```bash
cmake .. -DBUILD_TESTS=ON
cmake --build . -j$(nproc)
ctest --output-on-failure
```

---

## How to Contribute

### Reporting Bugs

1. Search [existing issues](https://github.com/GoldXCoin/GXC-CORE/issues) first.
2. Open a new issue using the **Bug Report** template.
3. Include: OS, compiler version, steps to reproduce, expected vs actual behaviour.

### Suggesting Features

1. Open an issue using the **Feature Request** template.
2. Describe the use case and why it belongs in the core node.

### Submitting Code

1. Fork the repository.
2. Create a feature branch: `git checkout -b feature/my-feature`
3. Commit with clear messages (see below).
4. Push: `git push origin feature/my-feature`
5. Open a Pull Request against `main`.

---

## Development Guidelines

### C++ Standard

- C++17 required.
- No compiler extensions (`-std=c++17`, not `gnu++17`).
- Compile cleanly with `-Wall -Wextra -Wpedantic`.

### Code Style

- 4-space indentation, no tabs.
- `camelCase` for variables and functions.
- `PascalCase` for classes and structs.
- `UPPER_SNAKE_CASE` for constants and macros.
- Keep functions under 80 lines where possible.
- Comment non-obvious algorithms with the mathematical formula they implement.

### Commit Messages

```
type(scope): short summary (≤72 chars)

Longer explanation if needed. Reference any issue numbers.

Closes #123
```

Types: `feat`, `fix`, `refactor`, `docs`, `test`, `build`, `chore`

### Testing

- Add a test in `tests/` for every new algorithm or validation rule.
- Tests must pass on Linux, macOS, and Windows.
- Cryptographic changes require a known-answer test (KAT) vector.

### Security-Sensitive Code

Changes to any of the following require an additional reviewer:

- `src/Crypto.cpp` — ECDSA, key generation
- `src/FraudDetection.cpp` — taint propagation
- `src/ReversalExecutor.cpp` — fund recovery
- `src/Blockchain.cpp` — block validation
- `src/Database.cpp` — persistence layer
- `src/security/SecurityEngine.cpp` — AI security

---

## Pull Request Process

1. Ensure the build passes on all platforms.
2. Ensure all existing tests pass.
3. Add new tests for new functionality.
4. Update `docs/WHITEPAPER.md` if a protocol-level change is made.
5. Request review from at least one core maintainer.
6. PRs are merged with **squash merge** to keep a clean history.

---

## Security Vulnerabilities

**Do not open a public issue for security vulnerabilities.**

Email: security@goldxcoin.network

Include:
- Description of the vulnerability
- Steps to reproduce
- Potential impact
- Suggested fix (if any)

We will respond within 48 hours and coordinate a responsible disclosure.
