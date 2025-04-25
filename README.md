# GXC Blockchain White Paper

*Publisher / Owner:* Philchade  
*Contact:* [philchade@gmail.com](mailto:philchade@gmail.com)  
*Version:* 1.1  
*Date:* April 25, 2025  

---

## Table of Contents

1. [Abstract](#abstract)  
2. [Introduction](#1-introduction)  
3. [Problem Statement](#2-problem-statement)  
4. [Solution Overview](#3-solution-overview)  
5. [Technical Details](#4-technical-details)  
   - 4.1 [Consensus & Mining Algorithms](#41-consensus--mining-algorithms)  
   - 4.2 [Transaction Traceability](#42-transaction-traceability)  
   - 4.3 [Gold-Backed Token Tracking](#43-gold-backed-token-tracking)  
   - 4.4 [Proof of Price (PoP) Oracle](#44-proof-of-price-pop-oracle)  
6. [Mathematical Formulation](#5-mathematical-formulation)  
   - 5.1 [Mining Validity](#51-mining-validity)  
   - 5.2 [Adaptive Monetary Policy](#52-adaptive-monetary-policy)  
7. [Rewards & Incentives](#6-rewards--incentives)  
8. [Security & Transparency](#7-security--transparency)  
9. [Conclusion](#8-conclusion)  
10. [How to Contribute](#9-how-to-contribute)  
11. [Publication & Collaboration](#10-publication--collaboration)  
12. [License & Copyright](#11-license--copyright)  

---

## Abstract

GXC is a hybrid Proof-of-Work blockchain that unites Bitcoin's SHA-256 and Ethereum's Ethash miners, issues a capped supply of 31 million GXC coins, and introduces two novel on-chain mechanisms: gold-asset tracking and "Proof of Price" (PoP) oracles. PoP feeds real-world price data on-chain, enabling transparent gold-backed token issuance and adaptive monetary policy tied to external market conditions.

---

## 1. Introduction

With Ethereum's move to PoS, GPU miners lost their primary network, while ASIC mining remains closed to smaller participants. Simultaneously, gold-backed tokens lack transparent, tamper-proof pricing, and fixed monetary schedules can over- or under-supply coins as market demand shifts. GXC addresses these by:

- *Hybrid PoW* for both ASIC (SHA-256) and GPU (Ethash) miners  
- *Fixed 31 million supply* of GXC with Bitcoin-style halving every four years  
- *Traceable transactions* via chained TX hashes  
- *Gold Tracking*: mint and track gold-backed tokens on-chain  
- *Proof of Price (PoP)*: decentralized oracles delivering signed price data  
- *Adaptive Monetary Policy*: block rewards and fee burns react to on-chain activity and PoP feed

---

## 2. Problem Statement

1. *Mining Fragmentation*: GPU miners need a new home post-EthPoS; ASIC miners remain siloed.  
2. *Asset-backed Opacity*: Gold tokens rely on off-chain attestations, prone to fraud.  
3. *Price Volatility*: Fixed supply schedules can't adapt to real-world demand shocks.  
4. *Oracle Dependence*: On-chain price data is often siloed in centralized oracles.

---

## 3. Solution Overview

- *Dual-Algorithm PoW*: SHA-256 & Ethash in one network.  
- *Capped Supply*: 31 million GXC with halving.  
- *Chained TX Hashes*: Full provenance for every coin.  
- *Gold-backed Tokens*: 1 GXC-G = 1 gram gold, fully traceable.  
- *Proof of Price (PoP) Oracle*: Aggregates external price feeds into on-chain signed data.  
- *Adaptive Policy*: Rewards $R(n)$ and burn rate $\alpha$ auto-adjust via feedback from PoP and supply metrics.

---

## 4. Technical Details

### 4.1 Consensus & Mining Algorithms

- *PoW Puzzle*:  

$$H(\text{Header}_n) < \text{Target}_n$$

- SHA-256 for ASICs  
- Ethash for GPUs  
- *Difficulty Retarget* every $\Delta_n$ blocks.

### 4.2 Transaction Traceability

Every transaction hash includes its sender's previous TX hash:

$$T_{\text{new}} = H(T_{\text{sender}} \parallel T_{\text{receiver}} \parallel \text{amount} \parallel h_{\text{prev}})$$

Where:
- $T_{\text{new}}$ is the new transaction hash
- $T_{\text{sender}}$ is the sender's address
- $T_{\text{receiver}}$ is the receiver's address
- $\text{amount}$ is the transaction amount
- $h_{\text{prev}}$ is the hash of the sender's previous transaction

### 4.3 Gold-Backed Token Tracking

GXC implements a parallel token GXC-G representing gold grams with the following properties:

$$T_{\text{gold}} = H(T_{\text{sender}} \parallel T_{\text{receiver}} \parallel \text{amount}g \parallel g{\text{prev}} \parallel \text{PoP}_{\text{gold}})$$

Where:
- $T_{\text{gold}}$ is the gold token transaction hash
- $\text{amount}_g$ is the amount in gold grams
- $g_{\text{prev}}$ is the hash of the sender's previous gold transaction
- $\text{PoP}_{\text{gold}}$ is the latest Proof of Price for gold

*Issuance Process:*
1. Authorized issuers deposit physical gold in approved vaults
2. Vault conducts verification and provides attestation
3. Attestation is signed and published on-chain
4. Smart contract mints GXC-G tokens based on verified deposits
5. Each token transaction incorporates the latest gold price feed via PoP

*Redemption Process:*
1. Token holder initiates redemption request
2. Smart contract burns tokens and issues redemption certificate
3. Vault verifies certificate and prepares physical gold for delivery
4. Transaction complete after confirmation of delivery

Regular reserve audits link on-chain balances to off-chain vaults, with results published via PoP oracles.

### 4.4 Proof of Price (PoP) Oracle

#### Oracle Design

- *Oracles*: $k$ independent data providers each submit gold price $p_i$
- *Verification*: Each submission includes signature $\text{sig}i = \text{Sign}{\text{sk}_i}(p_i \parallel \text{timestamp})$
- *Aggregation*:

$$P_n = \text{median}(p_1, p_2, \ldots, p_k)$$
$$\text{PoP}_n = H(P_n \parallel \text{timestamp}_n \parallel \text{IDs})$$

- *Update Rule*: Oracles call submitPrice(p_i, sig_i); once $k$ valid signatures received, aggregatePoP() stores $(P_n, \text{PoP}_n)$ on-chain.
- *Dispute Resolution*: Outlier detection mechanism flags submissions outside $\mu \pm 2\sigma$ range

#### Gold Price Tracking Specifics

- *Price Sources*: Multiple gold price feeds (LBMA, COMEX, Shanghai Gold Exchange)
- *Update Frequency*: Prices updated every 6 hours or when market moves >1%
- *Reference Unit*: USD per troy ounce, converted to USD per gram on-chain
- *Volatility Index*: Calculated as 30-day rolling standard deviation
- *Weighting*: Price inputs weighted by trading volume and source reputation

#### Pseudocode

javascript
// Submit a new gold price point from an oracle
function submitPrice(oracle_id, price, timestamp, signature) {
    // Verify oracle is authorized
    require(isAuthorizedOracle(oracle_id));
    
    // Verify signature matches the data and oracle's public key
    require(verifySignature(oracle_id, price, timestamp, signature));
    
    // Check timestamp is recent enough
    require(block.timestamp - timestamp < MAX_TIME_DELAY);
    
    // Store the price submission
    priceSubmissions[currentRound][oracle_id] = PricePoint({
        price: price,
        timestamp: timestamp,
        signature: signature
    });
    
    // Check if we have enough submissions to aggregate
    if (countValidSubmissions(currentRound) >= REQUIRED_SUBMISSIONS) {
        aggregatePoP();
    }
}

// Aggregate price points into a single PoP
function aggregatePoP() {
    // Get all valid submissions for current round
    PricePoint[] memory validPoints = getValidSubmissions(currentRound);
    
    // Extract just the prices
    uint256[] memory prices = new uint256[](validPoints.length);
    for (uint i = 0; i < validPoints.length; i++) {
        prices[i] = validPoints[i].price;
    }
    
    // Calculate median price
    uint256 medianPrice = calculateMedian(prices);
    
    // Apply outlier detection
    bool hasOutliers = detectOutliers(prices, medianPrice);
    if (hasOutliers) {
        emitOutlierWarning(prices, medianPrice);
    }
    
    // Get timestamp and oracle IDs
    uint256 timestamp = block.timestamp;
    bytes32[] memory oracleIds = getSubmittingOracleIds(currentRound);
    
    // Calculate PoP hash
    bytes32 popHash = keccak256(abi.encodePacked(
        medianPrice, 
        timestamp, 
        oracleIds
    ));
    
    // Store on-chain
    goldPricePoPs[currentRound] = GoldPoP({
        price: medianPrice,
        timestamp: timestamp,
        popHash: popHash,
        hasDispute: hasOutliers
    });
    
    // Move to next round
    currentRound++;
    
    // Emit event
    emit NewGoldPoP(currentRound - 1, medianPrice, timestamp, popHash);
}

// Get the current gold price in USD per gram
function getCurrentGoldPrice() public view returns (uint256) {
    require(currentRound > 0, "No price data available");
    return goldPricePoPs[currentRound - 1].price;
}

// Convert troy ounces to grams (1 troy oz = 31.1034768 grams)
function ouncesToGrams(uint256 ounces) public pure returns (uint256) {
    return ounces * 311034768 / 10000000; // Scaled for precision
}


#### Gold PoP Integration with GXC-G Tokens

javascript
// Mint GXC-G tokens based on verified gold deposit
function mintGoldTokens(address to, uint256 goldGrams) external onlyAuthorizedIssuer {
    // Get current gold price from PoP
    uint256 currentGoldPrice = getCurrentGoldPrice();
    
    // Ensure price is fresh
    require(block.timestamp - goldPricePoPs[currentRound - 1].timestamp < MAX_PRICE_AGE, 
            "Gold price data too old");
    
    // Calculate token amount (1 GXC-G = 1 gram of gold)
    uint256 tokenAmount = goldGrams;
    
    // Record transaction with PoP reference
    bytes32 prevTxHash = userLastGoldTx[to];
    bytes32 popRef = goldPricePoPs[currentRound - 1].popHash;
    
    // Create new transaction hash including PoP reference
    bytes32 newTxHash = keccak256(abi.encodePacked(
        issuerAddress,
        to,
        tokenAmount,
        prevTxHash,
        popRef
    ));
    
    // Update last transaction hash
    userLastGoldTx[to] = newTxHash;
    
    // Mint tokens
    gxcGoldToken.mint(to, tokenAmount);
    
    // Record issuance with price reference
    emit GoldTokensIssued(to, tokenAmount, currentGoldPrice, newTxHash, popRef);
}


---

## 5. Mathematical Formulation

### 5.1 Mining Validity

- *ASIC (SHA-256)*:

$$\text{SHA256}^2(\text{Header}) < \text{Target}$$

- *GPU (Ethash)*:

$$\text{Ethash}(\text{Header}) < \text{Target}$$

- *Difficulty Adjustment*:

$$\text{Target}_{n+1} = \text{Target}_n \times \frac{\text{Expected Time}}{\text{Actual Time}} \times \text{Damping Factor}$$

Where:
- Expected Time = Desired average time between blocks
- Actual Time = Observed time for the last $n$ blocks
- Damping Factor = Parameter to prevent excessive swings

### 5.2 Adaptive Monetary Policy

- *Total Supply*:

$$S(n) = \sum_{i=1}^{n} R(i) - \sum_{i=1}^{n} B(i)$$

Where:
- $S(n)$ is the total supply after $n$ blocks
- $R(i)$ is the reward for block $i$
- $B(i)$ is the amount burned in block $i$

- *Observed Inflation*:

$$\pi_{\text{obs}}(n) = \frac{S(n) - S(n-\Delta)}{S(n-\Delta)}$$

Where:
- $\pi_{\text{obs}}(n)$ is the observed inflation rate
- $\Delta$ is the measurement window in blocks

- *Gold-GXC Price Ratio*:

$$\rho(n) = \frac{P_{\text{gold}}(n)}{P_{\text{GXC}}(n)}$$

Where:
- $P_{\text{gold}}(n)$ is the gold price from PoP
- $P_{\text{GXC}}(n)$ is the GXC market price

- *Reward Adjustment*:

$$R_{\text{new}} = R_{\text{old}} \times [1 + k_1(\pi^* - \pi_{\text{obs}}(n)) + k_2(\rho^* - \rho(n))]$$

Where:
- $\pi^*$ is the target inflation rate
- $\rho^*$ is the target gold-GXC price ratio
- $k_1$ and $k_2$ are adjustment parameters

- *Fee Burn Rate*:

$$\alpha(n) = \alpha_0 \times [1 + k_3(\pi_{\text{obs}}(n) - \pi^*)]$$

Where:
- $\alpha(n)$ is the adaptive burn rate
- $\alpha_0$ is the base burn rate
- $k_3$ is the adjustment parameter

- *Amount Burned in Block $n$*:

$$B(n) = \alpha(n) \sum_{t \in n} f_t$$

Where:
- $f_t$ is the fee for transaction $t$ in block $n$

- *Net Flow*:

$$\Delta S(n) = R(n) - B(n)$$

---

## 6. Rewards & Incentives

- *Initial Block Reward*: 12.5 GXC
- *Halving Schedule*: Reward halves every 4 years (approximately every 1,051,200 blocks)
- *Transaction Fees*: fraction $\alpha$ burned, remainder to miner
- *Oracle Rewards*: PoP providers earn GXC for accurate price submissions
- *Gold Token Issuance Fee*: Small fee (0.1%) on GXC-G issuance
- *PoP-Driven Adjustments*: Monetary parameters incorporate PoP metrics
- *Validator Bond*: Price oracles must stake GXC as security

### Reward Schedule

| Period (Years) | Blocks | Reward (GXC) | New Supply | Cumulative Supply |
|----------------|--------|--------------|------------|-------------------|
| 0-4            | 1,051,200 | 12.5      | 13,140,000 | 13,140,000        |
| 4-8            | 1,051,200 | 6.25      | 6,570,000  | 19,710,000        |
| 8-12           | 1,051,200 | 3.125     | 3,285,000  | 22,995,000        |
| 12-16          | 1,051,200 | 1.5625    | 1,642,500  | 24,637,500        |
| 16+            | ...      | ...         | < 6,362,500 | < 31,000,000      |

---

## 7. Security & Transparency

- *Dual-PoW Protection*: Heightens 51%-attack cost by requiring control of both SHA-256 and Ethash hashrate
- *Chained TX Hashes*: Guarantee full transaction provenance and prevent double-spending
- *Oracle Signatures*: Cryptographically verify all PoP feeds
- *Outlier Detection*: Identifies and flags suspicious price submissions
- *Fee Burn Mechanism*: Ties supply to real usage & market data
- *Gold Reserve Verification*: Regular third-party audits with on-chain attestations
- *Open-Source Verification*: All code publicly available for security analysis
- *Economic Security*: Price feeds impact adaptive monetary policy, creating incentives for accuracy

### Security Against Common Attacks

1. *51% Attack*: Dual PoW requires attacker to control majority of both SHA-256 and Ethash hashrate
2. *Sybil Attack*: Oracle identities must be staked and verified
3. *Price Manipulation*: Median calculation and outlier detection protect against price manipulation
4. *Double Spending*: Transaction provenance through chained hashes prevents double spending
5. *Smart Contract Vulnerabilities*: Formal verification of critical contracts

---

## 8. Conclusion

GXC merges ASIC and GPU mining communities, issues a capped 31 million coin supply, embeds gold-asset tracking, and on-chains real-world price feeds via Proof of Price. Its adaptive monetary policy ensures sustainable inflation while transparent oracles and transaction traceability guard against fraud. The gold-backed token system provides a stable store of value with full on-chain verification of both the gold price and transaction provenance.

By combining the security of dual PoW consensus with the stability of gold-price tracking and adaptive monetary policy, GXC is positioned to be a robust, inclusive, and economically stable blockchain suitable for both miners seeking a new home and users looking for transparent asset-backed tokens.

---

## 9. How to Contribute

- *Mining Clients*: Implement dual-PoW software compatible with both SHA-256 and Ethash
- *Oracle Nodes*: Run PoP data providers for gold price feeds
- *Smart Contracts*: Build gold-token issuance/redemption contracts and PoP aggregators
- *Economic Analysis: Tune parameters $\pi^$, $k_1$, $k_2$, $k_3$, $\Delta$, $\alpha_0$ via simulation
- *Audits & Research*: Review consensus, oracle, and monetary modules
- *Documentation*: Improve technical specs and user guides
- *Testing*: Conduct security audits and penetration testing

### Development Roadmap

1. *Q2 2025*: Testnet launch with basic PoW and transaction functionality
2. *Q3 2025*: PoP oracle integration and gold price tracking
3. *Q4 2025*: GXC-G token implementation with full audit trail
4. *Q1 2026*: Mainnet launch with adaptive monetary policy
5. *Q2 2026*: Exchange listings and gold redemption partnerships

---

## 10. Publication & Collaboration

1. *GitHub*: Host repository & white paper for PRs at [github.com/philchade/gxc](https://github.com/philchade/gxc)
2. *Reddit*: r/CryptoCurrency, r/Blockchain – solicit community feedback
3. *Medium/Dev.to*: Publish technical summaries with links to full specification
4. *Bitcointalk*: Announce in altcoin forums for miner outreach
5. *Social Media*: Tweet updates with #GXC #ProofOfPrice #HybridMining
6. *Developer Discord*: Join community discussions at [discord.gg/gxc-blockchain](https://discord.gg/gxc-blockchain)
7. *Academic Partners*: Collaborate with universities on economic modeling

---

## 11. License & Copyright

© 2025 Philchade  
All rights reserved.

This white paper describes an open protocol that may be implemented by anyone following the specifications herein. The GXC reference implementation will be released under the MIT License.

The mathematical formulations, Proof of Price mechanism, and gold-asset tracking system described in this document are offered for public implementation. Any patents that may result from this work will be licensed on FRAND (Fair, Reasonable, And Non-Discriminatory) terms to promote widespread adoption.