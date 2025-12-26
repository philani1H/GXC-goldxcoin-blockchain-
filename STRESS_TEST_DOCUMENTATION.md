# GXC Blockchain Stress Test Documentation

## Overview

Comprehensive stress testing suite for the GXC blockchain, designed to test system limits and verify security, performance, and scalability under extreme load.

## Test Suite Components

### 1. 60,000 Miner Stress Test (`stress_test_60000_miners.py`)

Tests the blockchain's ability to handle massive concurrent mining operations.

#### Test 1: Template Requests (60,000 miners)
**Purpose:** Verify that 60,000 miners can request block templates simultaneously

**What it tests:**
- RPC API throughput
- Template generation performance
- Memory management under load
- Network connection handling
- Concurrent request processing

**Expected Results:**
- All 60,000 requests should succeed
- Rate: 100-500 requests/second
- No memory leaks
- No connection failures
- Each miner receives unique template

**Metrics Collected:**
- Total requests
- Success/failure rate
- Request rate (req/s)
- Response time
- Algorithm distribution (SHA256, GXHASH, ETHASH)

#### Test 2: Concurrent Mining (1,000 miners)
**Purpose:** Test actual mining competition with many miners

**What it tests:**
- Block submission handling
- Race condition management
- First-valid-block acceptance
- Duplicate block rejection
- Mining reward distribution

**Expected Results:**
- Multiple blocks found
- First submission accepted
- Subsequent submissions rejected
- Correct reward to winner
- Other miners move to next block

**Metrics Collected:**
- Blocks found
- Blocks accepted/rejected
- Mining time
- Hashrate
- Fee collection

#### Test 3: Security Engine
**Purpose:** Test AI Hashrate Sentinel under attack conditions

**What it tests:**
- Invalid block detection
- Rapid submission handling
- Attack pattern recognition
- Security alert generation
- System stability under attack

**Test Scenario:**
- Submit 1,000 invalid blocks rapidly
- Attempt to overwhelm validation
- Test duplicate height rejection
- Test invalid hash rejection

**Expected Results:**
- All invalid blocks rejected
- Security alerts generated
- System remains stable
- No successful attacks
- Performance not degraded

**Metrics Collected:**
- Invalid blocks submitted
- Blocks rejected
- Security alerts
- System response time

#### Test 4: Dynamic Fee System
**Purpose:** Test fee calculation under various loads

**What it tests:**
- Fee estimation accuracy
- Dynamic fee adjustment
- Transaction size impact
- Network congestion handling
- Fee fairness

**Test Scenarios:**
- Small transactions (1 tx)
- Medium transactions (10 tx)
- Large transactions (100 tx)
- Massive transactions (1000 tx)

**Expected Results:**
- Fees scale with transaction size
- Higher load = higher fees
- Fair fee distribution
- No fee manipulation
- Predictable fee calculation

**Metrics Collected:**
- Fee estimates
- Actual fees paid
- Fee variance
- Transaction sizes

---

### 2. Validator Stress Test (`stress_test_validators.py`)

Tests the Proof-of-Stake validator system under extreme load.

#### Test 1: Validator Registration (1,000-60,000 validators)
**Purpose:** Test mass validator registration

**What it tests:**
- Registration throughput
- Stake validation
- Validator storage
- Database performance
- Concurrent registration handling

**Test Parameters:**
- Stake amounts: 1,000-10,000 GXC (random)
- Staking periods: 30, 60, 90, 180, 365 days
- Concurrent registrations: 100 workers

**Expected Results:**
- High success rate (>95%)
- Fast registration (>10 reg/s)
- Correct stake recording
- No duplicate validators
- Proper database updates

**Metrics Collected:**
- Total registrations
- Success/failure rate
- Registration rate
- Total staked amount
- Average stake

#### Test 2: Concurrent Staking (5,000 stakes)
**Purpose:** Test concurrent staking operations

**What it tests:**
- Staking transaction handling
- Balance verification
- Lock period enforcement
- Concurrent stake processing
- Database consistency

**Test Parameters:**
- Stake amounts: 100-5,000 GXC (random)
- Staking periods: 30-365 days
- Concurrent operations: 100 workers

**Expected Results:**
- High success rate
- Fast processing
- Correct balance deduction
- Proper lock enforcement
- No double-staking

**Metrics Collected:**
- Total stakes
- Success rate
- Total staked
- Processing time

#### Test 3: Validator Selection Algorithm
**Purpose:** Test validator selection under large validator set

**What it tests:**
- Selection algorithm performance
- Weighted random selection
- Stake-based probability
- Selection fairness
- Algorithm efficiency

**Test Scenario:**
- 100 selection attempts
- Large validator pool
- Various stake amounts
- Distribution analysis

**Expected Results:**
- Fast selection (<100ms)
- Fair distribution
- Stake-weighted probability
- No selection bias
- Efficient algorithm

**Metrics Collected:**
- Selection attempts
- Unique validators selected
- Distribution percentage
- Selection time

#### Test 4: PoS Block Generation
**Purpose:** Test Proof-of-Stake block creation

**What it tests:**
- PoS block creation
- Validator rotation
- Block validation
- Reward distribution
- PoS/PoW hybrid operation

**Expected Results:**
- PoS blocks generated
- Correct validator selection
- Valid block structure
- Proper rewards
- Smooth PoW/PoS transition

**Metrics Collected:**
- PoS blocks generated
- Validator rewards
- Block validation time
- PoS activation status

#### Test 5: Concurrent Unstaking (500 unstakes)
**Purpose:** Test unstaking under load

**What it tests:**
- Unstaking transaction handling
- Lock period verification
- Balance restoration
- Concurrent unstake processing
- Penalty calculation

**Expected Results:**
- Correct lock enforcement
- Proper balance restoration
- Penalty application (if early)
- Fast processing
- Database consistency

**Metrics Collected:**
- Unstake attempts
- Success rate
- Penalties applied
- Processing time

---

## System Components Tested

### 1. Security Engine (AI Hashrate Sentinel)

**Features Tested:**
- Invalid block detection
- Attack pattern recognition
- Rapid submission handling
- Security alert generation
- System stability

**Attack Scenarios:**
- Invalid hash attacks
- Duplicate height attacks
- Rapid submission attacks
- Malformed block attacks
- Timestamp manipulation

**Expected Behavior:**
- 100% invalid block rejection
- Security alerts generated
- System remains stable
- No performance degradation
- Attack patterns logged

### 2. Dynamic Fee System

**Features Tested:**
- Fee estimation
- Dynamic adjustment
- Load-based scaling
- Transaction size impact
- Fairness

**Test Conditions:**
- Low load (few transactions)
- Medium load (moderate transactions)
- High load (many transactions)
- Extreme load (massive transactions)

**Expected Behavior:**
- Fees scale with load
- Predictable calculation
- Fair distribution
- No manipulation
- Efficient processing

### 3. Mining Algorithms

**Algorithms Tested:**
- SHA256 (Bitcoin-compatible)
- GXHASH (ASIC-resistant)
- ETHASH (GPU-optimized)

**Test Scenarios:**
- Algorithm distribution
- Performance comparison
- Difficulty handling
- Block validation
- Reward calculation

**Expected Results:**
- All algorithms work
- Fair distribution
- Correct difficulty
- Valid blocks
- Proper rewards

### 4. Consensus Mechanism

**Features Tested:**
- PoW block acceptance
- PoS block generation
- Hybrid PoW/PoS operation
- Validator selection
- Block validation

**Test Scenarios:**
- Pure PoW mining
- PoS with validators
- Hybrid operation
- Validator rotation
- Consensus rules

**Expected Results:**
- Correct block type
- Valid consensus
- Smooth transitions
- Proper validation
- Rule enforcement

---

## Performance Benchmarks

### Template Request Performance

| Miners | Expected Time | Expected Rate |
|--------|---------------|---------------|
| 100 | ~1s | 100 req/s |
| 1,000 | ~10s | 100 req/s |
| 10,000 | ~100s | 100 req/s |
| 60,000 | ~600s | 100 req/s |

### Mining Performance

| Difficulty | Expected Time | Hashrate Required |
|------------|---------------|-------------------|
| 1.0 (2 zeros) | ~0.1s | 1,000 H/s |
| 4.0 (2 zeros) | ~0.1s | 1,000 H/s |
| 16.0 (3 zeros) | ~1s | 10,000 H/s |
| 1000.0 (5 zeros) | ~100s | 100,000 H/s |

### Validator Performance

| Operation | Expected Rate | Expected Time |
|-----------|---------------|---------------|
| Registration | 10-50 reg/s | <100ms each |
| Staking | 20-100 stake/s | <50ms each |
| Selection | N/A | <100ms |
| Unstaking | 10-50 unstake/s | <100ms each |

---

## Running the Tests

### Full 60,000 Miner Test
```bash
# Start blockchain node first
./gxc-node &

# Wait for node to be ready
sleep 10

# Run stress test
./stress_test_60000_miners.py
```

### Validator Stress Test
```bash
# Node must be running
./stress_test_validators.py
```

### Complete Test Suite
```bash
# Runs all tests automatically
./run_stress_tests.sh
```

### Quick Test (Smaller Scale)
```bash
# Quick validation test
./quick_stress_test.sh
```

---

## Test Results Interpretation

### Success Criteria

**Template Requests:**
- ✅ Success rate > 95%
- ✅ Rate > 50 req/s
- ✅ No memory leaks
- ✅ No connection failures

**Mining:**
- ✅ Blocks found and accepted
- ✅ First submission wins
- ✅ Duplicates rejected
- ✅ Correct rewards

**Security:**
- ✅ All invalid blocks rejected
- ✅ Security alerts generated
- ✅ System stable
- ✅ No successful attacks

**Validators:**
- ✅ Registration success > 90%
- ✅ Staking success > 90%
- ✅ Fair validator selection
- ✅ PoS blocks generated

**Fees:**
- ✅ Fees scale with load
- ✅ Predictable calculation
- ✅ Fair distribution

### Failure Indicators

**Red Flags:**
- ❌ Success rate < 80%
- ❌ System crashes
- ❌ Memory leaks
- ❌ Invalid blocks accepted
- ❌ Unfair validator selection
- ❌ Fee manipulation
- ❌ Database corruption

---

## Test Output Files

### Generated Files

1. **stress_test_results_[timestamp].json**
   - Miner stress test results
   - Template request metrics
   - Mining statistics
   - Security alerts
   - Fee data

2. **validator_stress_test_results_[timestamp].json**
   - Validator registration data
   - Staking statistics
   - Selection metrics
   - PoS block data
   - Unstaking results

3. **node_stress_test.log**
   - Node output during tests
   - Error messages
   - Debug information
   - Performance logs

### Result Format

```json
{
  "elapsed": 600.5,
  "template_requests": 60000,
  "template_success": 59850,
  "template_failures": 150,
  "template_rate": 99.75,
  "blocks_found": 45,
  "blocks_accepted": 42,
  "blocks_rejected": 3,
  "total_fees": 0.042,
  "security_alerts": 1000,
  "algorithm_usage": {
    "SHA256": 20000,
    "GXHASH": 20000,
    "ETHASH": 20000
  }
}
```

---

## Troubleshooting

### Node Won't Start
```bash
# Check dependencies
ldd ./gxc-node

# Install missing libraries
sudo apt-get install libleveldb-dev libsqlite3-dev

# Rebuild if needed
make clean && make
```

### Connection Refused
```bash
# Check if node is running
ps aux | grep gxc-node

# Check port
netstat -tlnp | grep 8332

# Check logs
tail -f node_stress_test.log
```

### Out of Memory
```bash
# Reduce concurrent workers
# Edit scripts: max_workers=50 (instead of 200)

# Reduce test size
# Edit scripts: num_miners=1000 (instead of 60000)
```

### Slow Performance
```bash
# Check system resources
top
free -h
df -h

# Reduce load
# Use smaller test parameters
```

---

## Conclusion

The GXC blockchain stress test suite provides comprehensive testing of:
- ✅ 60,000+ concurrent miners
- ✅ Security engine under attack
- ✅ Dynamic fee system
- ✅ All mining algorithms
- ✅ Validator system at scale
- ✅ PoS block generation
- ✅ Concurrent operations

These tests verify that the blockchain can handle extreme load while maintaining security, fairness, and performance.
