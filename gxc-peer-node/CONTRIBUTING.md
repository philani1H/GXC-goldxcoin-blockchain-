# Contributing to GXC Peer Node

Thank you for your interest in contributing to the GXC Peer Node project!

## Getting Started

1. Fork the repository
2. Clone your fork: `git clone https://github.com/YOUR_USERNAME/gxc-peer-node.git`
3. Create a branch: `git checkout -b feature/your-feature-name`
4. Make your changes
5. Run tests: `cargo test`
6. Commit: `git commit -m "Add your feature"`
7. Push: `git push origin feature/your-feature-name`
8. Create a Pull Request

## Development Setup

### Prerequisites

- Rust 1.70+
- Git
- GXC main node (for testing)

### Build

```bash
cargo build
```

### Run Tests

```bash
cargo test
```

### Run with Logging

```bash
RUST_LOG=debug cargo run -- start
```

## Code Style

- Follow Rust standard formatting: `cargo fmt`
- Pass clippy lints: `cargo clippy`
- Write tests for new features
- Document public APIs

## Testing

- Write unit tests in the same file as the code
- Write integration tests in `tests/` directory
- Ensure all tests pass before submitting PR

## Pull Request Process

1. Update README.md with details of changes if needed
2. Update CHANGELOG.md with your changes
3. Ensure CI passes (formatting, clippy, tests)
4. Request review from maintainers
5. Address review feedback
6. Squash commits if requested

## Commit Messages

Follow conventional commits:

- `feat: Add new feature`
- `fix: Fix bug`
- `docs: Update documentation`
- `test: Add tests`
- `refactor: Refactor code`
- `chore: Update dependencies`

## Code of Conduct

- Be respectful and inclusive
- Focus on constructive feedback
- Help others learn and grow

## Questions?

Open an issue or ask in the GXC community channels.
