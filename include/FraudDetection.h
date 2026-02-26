#pragma once

/**
 * ╔══════════════════════════════════════════════════════════════════════════════╗
 * ║            GXC FRAUD DETECTION - MATHEMATICAL FOUNDATION                     ║
 * ╠══════════════════════════════════════════════════════════════════════════════╣
 * ║                                                                              ║
 * ║  PROOF OF TRACEABILITY (POT) - TAINT PROPAGATION SYSTEM                      ║
 * ║                                                                              ║
 * ║  Based on graph theory and value conservation, this system treats the        ║
 * ║  blockchain as a directed acyclic graph (DAG) where stolen funds become      ║
 * ║  a "graph anomaly" that can be mathematically detected and traced.           ║
 * ║                                                                              ║
 * ║  Core Innovation: MATHEMATICAL TAINT PROPAGATION                             ║
 * ║                                                                              ║
 * ║    τ(T_j) = Σ_{i∈inputs(j)} w_i · τ(T_i)                                    ║
 * ║                                                                              ║
 * ║    where w_i = value_from_T_i / total_input_value                            ║
 * ║                                                                              ║
 * ║  This is CONSERVATION OF VALUE, not opinion or heuristics.                   ║
 * ║                                                                              ║
 * ╚══════════════════════════════════════════════════════════════════════════════╝
 *
 * ═══════════════════════════════════════════════════════════════════════════════
 *                        MATHEMATICAL FOUNDATIONS
 * ═══════════════════════════════════════════════════════════════════════════════
 *
 * GRAPH MODEL:
 * ────────────
 *   G = (V, E) where:
 *     V = set of all transactions
 *     E = {(T_i, T_j) | T_i is an input to T_j}
 *
 *   G is a Directed Acyclic Graph (DAG) because:
 *     • Time flows forward (no cycles)
 *     • Transactions can only reference past transactions
 *     • Genesis has no incoming edges
 *
 * TAINT FUNCTION:
 * ───────────────
 *   τ: V → [0, 1]
 *   τ(T) = "taint score" = probability that funds in T are stolen
 *
 *   Base case (stolen transaction):
 *     τ(T_stolen) = 1.0  ∀T_stolen ∈ S (seed set of stolen txs)
 *
 *   Base case (coinbase/clean transaction):
 *     τ(T_coinbase) = 0.0  ∀T_coinbase ∉ reachable(S)
 *
 *   Recursive case (propagation):
 *     τ(T_j) = Σ_{i∈inputs(j)} w_i · τ(T_i)
 *
 *     where w_i = value_i / Σ_{k∈inputs(j)} value_k  (weighted average)
 *
 * CONSERVATION PROPERTY:
 * ──────────────────────
 *   THEOREM: Σ w_i = 1  (weights sum to 1)
 *
 *   PROOF:
 *     Σ w_i = Σ (value_i / total_input_value)
 *           = (1 / total_input_value) · Σ value_i
 *           = (1 / total_input_value) · total_input_value
 *           = 1  ∎
 *
 *   COROLLARY: 0 ≤ τ(T_j) ≤ max{τ(T_i) | i∈inputs(j)}
 *              Taint never increases, only dilutes.
 *
 * COMPUTATIONAL PROPERTIES:
 * ─────────────────────────
 *   • Taint propagation is O(|E|) time (one pass through graph)
 *   • Taint storage is O(|V|) space
 *   • Graph traversal uses breadth-first search (BFS)
 *   • Convergence: Taint → 0 as distance from stolen tx → ∞
 *
 * NOTATION GUIDE:
 * ───────────────
 *   τ(T)      = Taint score of transaction T ∈ [0, 1]
 *   S         = Seed set of stolen transactions
 *   w_i       = Weight of input i (value fraction)
 *   δ         = Taint threshold (stop propagation if τ < δ)
 *   ε         = Time epsilon (velocity threshold)
 *   θ         = Re-aggregation threshold
 *   K         = Fan-out constant (smurfing detection)
 *   G         = Transaction graph (V, E)
 *   |V|       = Number of transactions
 *   |E|       = Number of transaction edges
 *   d(u,v)    = Graph distance from u to v
 *
 * REFERENCES:
 * ───────────
 *  [1] Meiklejohn, S. et al. "A Fistful of Bitcoins" (2013)
 *  [2] Ron, D. & Shamir, A. "Quantitative Analysis of Bitcoin..." (2013)
 *  [3] Möser, M. & Böhme, R. "Anonymous Alone?" (2017)
 *  [4] Fleder, M. et al. "Bitcoin Transaction Graph Analysis" (2015)
 *  [5] Weber, M. et al. "Anti-Money Laundering in Bitcoin" (2019)
 *  [6] Papadopoulos, P. et al. "MAD-HTLC" (2020)
 *  [7] Cormen, T. et al. "Introduction to Algorithms" 3rd Ed. (2009)
 *
 */

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <ctime>
#include <cmath>
#include <cstdint>
#include "AddressRegistry.h"

// Forward declarations
class Transaction;
class Blockchain;

// ═══════════════════════════════════════════════════════════════════════════
//                        TAINT DATA STRUCTURES
// ═══════════════════════════════════════════════════════════════════════════

/**
 * Taint Information
 * ─────────────────
 * Stores computed taint score and provenance path for a transaction.
 *
 * FIELDS:
 *   • taintScore: τ(T) ∈ [0, 1]
 *   • sourceTransaction: Original stolen tx from seed set S
 *   • timestamp: When taint was computed
 *   • ancestry: Path from source to this tx (for proof generation)
 */
struct TaintInfo {
    double taintScore;           // τ ∈ [0,1]
    std::string sourceTransaction; // Original stolen tx from seed set S
    uint64_t timestamp;
    std::vector<std::string> ancestry; // Path: [T_source, ..., T_current]

    TaintInfo() : taintScore(0.0), timestamp(0) {}
    TaintInfo(double score, const std::string& source, uint64_t ts)
        : taintScore(score), sourceTransaction(source), timestamp(ts) {}
};

/**
 * Fraud Alert
 * ───────────
 * Generated when suspicious patterns are detected [5].
 *
 * ALERT LEVELS:
 *   LOW      → τ ∈ [0.1, 0.3) or single rule violation
 *   MEDIUM   → τ ∈ [0.3, 0.5) or two rule violations
 *   HIGH     → τ ∈ [0.5, 0.8) or three rule violations
 *   CRITICAL → τ ≥ 0.8 or four+ rule violations
 *
 * RULE VIOLATIONS (Money Laundering Patterns):
 *   1. VELOCITY_ANOMALY    → Too-fast money movement (< 5 min)
 *   2. FAN_OUT_PATTERN     → Smurfing (split into >K outputs)
 *   3. RE_AGGREGATION      → Layering (fan-out then re-combine)
 *   4. DORMANCY_ACTIVATION → Wait-and-move (7+ day dormancy)
 *   5. CLEAN_ZONE_ENTRY    → Legitimization attempt (→ exchange/pool)
 */
struct FraudAlert {
    enum class AlertLevel {
        LOW,        // τ < 0.3 or 1 rule violation
        MEDIUM,     // τ < 0.5 or 2 rule violations
        HIGH,       // τ < 0.8 or 3 rule violations
        CRITICAL    // τ ≥ 0.8 or 4+ rule violations
    };

    enum class RuleViolation {
        VELOCITY_ANOMALY,      // Rule 1: ΔT < ε (too fast)
        FAN_OUT_PATTERN,       // Rule 2: |outputs| > K (smurfing)
        RE_AGGREGATION,        // Rule 3: fan-out → re-combine
        DORMANCY_ACTIVATION,   // Rule 4: dormant > 7 days then move
        CLEAN_ZONE_ENTRY       // Rule 5: → exchange/pool
    };

    std::string transactionHash;
    std::string address;
    AlertLevel level;
    RuleViolation rule;
    double taintScore;              // τ ∈ [0,1]
    std::string description;
    uint64_t timestamp;
    std::vector<std::string> evidence;

    FraudAlert() : level(AlertLevel::LOW), rule(RuleViolation::VELOCITY_ANOMALY),
                   taintScore(0.0), timestamp(0) {}
};

// ═══════════════════════════════════════════════════════════════════════════
//                  PROOF OF FEASIBILITY (POF) - REVERSAL PROOF
// ═══════════════════════════════════════════════════════════════════════════

/**
 * Proof of Feasibility (POF)
 * ──────────────────────────
 *
 * Cryptographic proof that a reversal is valid and feasible.
 * NOT a blockchain rollback - this is a forward state transition.
 *
 * WHAT POF PROVES:
 * ────────────────
 *   1. CAUSALITY: Funds in current_tx originated from stolen_tx
 *                 Proven via trace_path in transaction graph G
 *
 *   2. CONSERVATION: Recoverable amount ≤ current balance × taint score
 *                    R = τ(T_current) × balance(current_holder)
 *
 *   3. AUTHORIZATION: Admin approval with valid ECDSA signature
 *                     verify(admin_signature, proof_hash, admin_pubkey) = true
 *
 *   4. FINALITY SAFETY: Reversal occurs within time window
 *                       block_height < stolen_block_height + REVERSAL_WINDOW
 *
 * MATHEMATICAL FORMULATION:
 * ─────────────────────────
 *   Given:
 *     • T_s ∈ S (stolen transaction)
 *     • T_c (current transaction holding tainted funds)
 *     • Path P = [T_s, T_1, ..., T_k, T_c] in graph G
 *     • τ(T_c) computed via taint propagation
 *     • B_c = balance of current holder
 *
 *   Recoverable amount:
 *     R = ⌊τ(T_c) × B_c⌋
 *
 *   Reversal transaction:
 *     T_reversal: current_holder → victim, amount = R
 *     with attached POF as cryptographic justification
 *
 * SECURITY PROPERTIES:
 * ────────────────────
 *   • Proof is unforgeable (requires admin private key)
 *   • Proof is tamper-evident (cryptographic hash binding)
 *   • Proof is publicly verifiable (all data on-chain)
 *   • Proof preserves immutability (no rollback, just new tx)
 */
struct ProofOfFeasibility {
    std::string proof_type;              // "REVERSAL_PROOF"
    std::string stolen_tx;               // T_s: Original stolen transaction hash
    std::string current_tx;              // T_c: Current holding transaction hash
    std::vector<std::string> trace_path; // P: Full path [T_s, ..., T_c] in G
    double taint_score;                  // τ(T_c) ∈ [0,1]
    uint64_t recoverable_amount;         // R = ⌊τ(T_c) × B_c⌋
    std::string origin_owner;            // Victim address
    std::string current_holder;          // Current holder address
    std::string approved_by;             // Admin ID who approved
    std::string approval_signature;      // ECDSA(SHA256(proof_hash), admin_privkey)
    uint64_t timestamp;                  // Approval timestamp
    uint64_t block_height;               // Block height when approved
    std::string proof_hash;              // SHA256(all fields above) - tamper detection

    ProofOfFeasibility() : taint_score(0.0), recoverable_amount(0),
                          timestamp(0), block_height(0) {}
};

/**
 * Reversal Transaction
 * ────────────────────
 *
 * Special transaction type (REVERSAL) that reverses stolen funds.
 * NOT a rollback - a forward state transition with cryptographic justification.
 *
 * CONSENSUS RULES:
 * ────────────────
 *   1. Must have valid ProofOfFeasibility attached
 *   2. Amount must equal POF.recoverable_amount
 *   3. Must be signed by admin with governance authority
 *   4. Must occur within REVERSAL_WINDOW blocks of theft
 *   5. Fee paid from fraud detection pool (not from victim/holder)
 *
 * TRANSACTION FORMAT:
 * ───────────────────
 *   Type: REVERSAL (TransactionType::REVERSAL = 5)
 *   From: current_holder (involuntary debit)
 *   To: origin_owner (victim credit)
 *   Amount: POF.recoverable_amount
 *   Fee: paid by system fraud detection pool
 *   Proof: Hash(ProofOfFeasibility)
 *   Signature: Admin signature (not holder signature!)
 *
 * ECONOMIC MODEL:
 * ───────────────
 *   • Fee pool funded by % of all transaction fees
 *   • Self-sustaining fraud detection system
 *   • No cost to victims or honest holders
 *   • Incentivizes fraud reporting (future: bounties)
 */
struct ReversalTransaction {
    std::string tx_hash;                 // Transaction hash
    std::string from;                    // Current holder (involuntary debit)
    std::string to;                      // Original victim (credit)
    uint64_t amount;                     // POF.recoverable_amount
    std::string proof_hash;              // Hash(Proof of Feasibility)
    uint64_t fee;                        // Paid by fraud detection pool
    uint64_t timestamp;                  // Transaction timestamp
    uint64_t block_height;               // Block height when executed
    std::string admin_signature;         // Admin approval signature

    ReversalTransaction() : amount(0), fee(0), timestamp(0), block_height(0) {}
};

// ═══════════════════════════════════════════════════════════════════════════
//                        FRAUD DETECTION ENGINE
// ═══════════════════════════════════════════════════════════════════════════

class FraudDetection {
private:
    // ═════════════════════════════════════════════════════════════════════
    //                      INTERNAL STATE
    // ═════════════════════════════════════════════════════════════════════

    // Taint tracking
    std::unordered_map<std::string, TaintInfo> taintMap; // τ: V → [0,1]
    std::unordered_set<std::string> stolenTransactions;  // Seed set S ⊂ V
    std::unordered_set<std::string> flaggedAddresses;    // Monitored addresses

    // Clean zone registry (exchanges, pools, validators)
    enum class CleanZoneType {
        EXCHANGE,        // Centralized exchange
        STAKING_POOL,    // Staking pool
        MERCHANT,        // Merchant payment processor
        VALIDATOR        // PoS validator
    };

    struct CleanZoneInfo {
        CleanZoneType type;
        std::string name;
        uint64_t registeredAt;
    };

    std::unordered_map<std::string, CleanZoneInfo> cleanZoneRegistry;

    // Alert system
    std::vector<FraudAlert> alerts;
    std::unordered_map<std::string, std::vector<FraudAlert>> addressAlerts;

    // ═════════════════════════════════════════════════════════════════════
    //                  ALGORITHM PARAMETERS (Calibrated)
    // ═════════════════════════════════════════════════════════════════════

    static constexpr double TAINT_THRESHOLD = 0.1;      // δ: Stop propagation if τ < δ
    static constexpr double HIGH_TAINT = 0.5;           // τ_high: High taint threshold
    static constexpr double CRITICAL_TAINT = 0.8;       // τ_crit: Critical taint threshold

    static constexpr uint64_t VELOCITY_EPSILON = 300;   // ε: 5 minutes (velocity anomaly)
    static constexpr uint32_t FAN_OUT_K = 5;            // K: Max outputs before smurfing alert
    static constexpr double RE_AGG_THETA = 0.7;         // θ: Re-aggregation threshold
    static constexpr uint64_t DORMANCY_PERIOD = 604800; // 7 days (dormancy detection)

    // Reference to blockchain (for graph traversal)
    Blockchain* blockchain;

    // ═════════════════════════════════════════════════════════════════════
    //                      INTERNAL METHODS
    // ═════════════════════════════════════════════════════════════════════

    /**
     * TAINT CALCULATION (Core Algorithm)
     * ───────────────────────────────────
     *
     * Computes taint score for a transaction using weighted averaging.
     *
     * ALGORITHM:
     *   Input: Transaction T_j with inputs {T_i1, T_i2, ..., T_in}
     *   Output: τ(T_j) ∈ [0, 1]
     *
     *   1. If T_j ∈ S (stolen set):
     *        return 1.0
     *
     *   2. If T_j is coinbase:
     *        return 0.0
     *
     *   3. Otherwise:
     *        total_input_value ← Σ value(T_ik)
     *        taint ← 0.0
     *        for each input T_i in inputs(T_j):
     *            w_i ← value(T_i) / total_input_value
     *            taint ← taint + w_i × τ(T_i)
     *        return taint
     *
     * COMPLEXITY: O(|inputs|) time, O(1) space
     *
     * CORRECTNESS:
     *   By conservation property, Σ w_i = 1
     *   Therefore: 0 ≤ τ(T_j) ≤ max{τ(T_i)}
     *   Taint dilutes over time (cannot increase)
     */
    double calculateTaint(const Transaction& tx);

    /**
     * RULE 1: VELOCITY ANOMALY DETECTION
     * ───────────────────────────────────
     *
     * Detects too-fast money movement (money laundering pattern) [5].
     *
     * FORMULA:
     *   ΔT = timestamp(T_current) - timestamp(T_source)
     *   anomaly = (ΔT < ε) AND (τ > δ)
     *
     *   where ε = 300s (5 minutes)
     *         δ = 0.1 (minimum taint to care)
     *
     * RATIONALE:
     *   Legitimate users don't move money every 30 seconds.
     *   Rapid movement suggests automated laundering.
     *
     * COMPLEXITY: O(1)
     */
    bool checkVelocityAnomaly(const Transaction& tx, const TaintInfo& taint);

    /**
     * RULE 2: FAN-OUT PATTERN (Smurfing Detection)
     * ─────────────────────────────────────────────
     *
     * Detects splitting tainted funds into many small outputs [1,2].
     *
     * FORMULA:
     *   smurfing = (|outputs(T)| > K) AND (τ(T) > δ)
     *
     *   where K = 5 (max outputs before alert)
     *
     * EXAMPLE:
     *   Stolen 1000 GXC → split into 100 outputs of 10 GXC
     *   Each output has τ = 1.0 (still fully tainted)
     *   Alert level: CRITICAL
     *
     * COMPLEXITY: O(1) (just count outputs)
     */
    bool checkFanOutPattern(const Transaction& tx, const TaintInfo& taint);

    /**
     * RULE 3: RE-AGGREGATION (Layering Detection)
     * ────────────────────────────────────────────
     *
     * Detects fan-out followed by re-combination (layering) [5].
     *
     * ALGORITHM:
     *   1. Check if inputs come from multiple fan-out transactions
     *   2. Compute aggregated taint: τ_agg = Σ w_i × τ(T_i)
     *   3. If τ_agg > θ, flag as re-aggregation
     *
     *   where θ = 0.7 (re-aggregation threshold)
     *
     * EXAMPLE:
     *   T_stolen → [T1, T2, T3, ..., T100] (fan-out)
     *   [T1, T2, T3] → T_combined (re-aggregation)
     *   τ(T_combined) = 1.0 (still fully tainted!)
     *
     * COMPLEXITY: O(|inputs|) time
     */
    bool checkReAggregation(const Transaction& tx);

    /**
     * RULE 4: DORMANCY ACTIVATION (Wait-and-Move)
     * ────────────────────────────────────────────
     *
     * Detects funds sitting dormant then suddenly moving [3].
     *
     * FORMULA:
     *   dormant_time = timestamp(T_current) - timestamp(T_last_move)
     *   anomaly = (dormant_time > DORMANCY_PERIOD) AND (τ > δ)
     *
     *   where DORMANCY_PERIOD = 7 days
     *
     * RATIONALE:
     *   Thieves often wait to "cool off" stolen funds.
     *   Sudden activation after dormancy is suspicious.
     *
     * COMPLEXITY: O(1)
     */
    bool checkDormancyActivation(const Transaction& tx, const TaintInfo& taint);

    /**
     * RULE 5: CLEAN ZONE ENTRY (Legitimization Attempt)
     * ──────────────────────────────────────────────────
     *
     * Detects tainted funds entering exchanges/pools (cashing out) [4].
     *
     * FORMULA:
     *   legitimization = (recipient ∈ cleanZoneRegistry) AND (τ > δ)
     *
     * RATIONALE:
     *   Thieves must eventually cash out via exchange.
     *   This is the "choke point" for fund recovery.
     *
     * COMPLEXITY: O(1) (hash table lookup)
     */
    bool checkCleanZoneEntry(const Transaction& tx, const TaintInfo& taint);

    /**
     * ALERT LEVEL CALCULATION
     * ───────────────────────
     *
     * Maps taint score + rule violations to alert severity.
     *
     * FORMULA:
     *   Let τ = taint score, n = number of rule violations
     *
     *   level = CRITICAL if τ ≥ 0.8 OR n ≥ 4
     *   level = HIGH     if τ ≥ 0.5 OR n ≥ 3
     *   level = MEDIUM   if τ ≥ 0.3 OR n ≥ 2
     *   level = LOW      otherwise
     *
     * COMPLEXITY: O(1)
     */
    FraudAlert::AlertLevel calculateAlertLevel(double taintScore, uint32_t ruleViolations);

    /**
     * Adds an alert to the alert system.
     */
    void addAlert(const FraudAlert& alert);

    // Reversal system components (dependency injection)
    class ProofGenerator* proofGenerator;
    class ReversalExecutor* reversalExecutor;

public:
    // ═════════════════════════════════════════════════════════════════════
    //                      PUBLIC API
    // ═════════════════════════════════════════════════════════════════════

    FraudDetection(Blockchain* bc);
    ~FraudDetection();

    /**
     * Sets reversal system components (dependency injection).
     */
    void setReversalSystem(class ProofGenerator* pg, class ReversalExecutor* re) {
        proofGenerator = pg;
        reversalExecutor = re;
    }

    // ═════════════════════════════════════════════════════════════════════
    //                  SEED MANAGEMENT (Stolen Tx Set S)
    // ═════════════════════════════════════════════════════════════════════

    /**
     * Marks a transaction as stolen (adds to seed set S).
     * Triggers taint propagation from this source.
     *
     * COMPLEXITY: O(1) for insertion, O(|E|) for propagation
     */
    void markAsStolen(const std::string& txHash);

    /**
     * Removes a transaction from stolen set (false positive correction).
     * Requires full taint recalculation.
     *
     * COMPLEXITY: O(|E|) (must recalculate all downstream taint)
     */
    void removeFromStolen(const std::string& txHash);

    /**
     * Checks if a transaction is in the stolen seed set S.
     *
     * COMPLEXITY: O(1)
     */
    bool isStolen(const std::string& txHash) const;

    // ═════════════════════════════════════════════════════════════════════
    //              TAINT PROPAGATION (Core Algorithm)
    // ═════════════════════════════════════════════════════════════════════

    /**
     * BREADTH-FIRST TAINT PROPAGATION
     * ────────────────────────────────
     *
     * Propagates taint from a seed transaction through the graph G.
     *
     * ALGORITHM (BFS with Taint Calculation):
     *   Input: startTxHash (stolen transaction), maxHops
     *   Output: Updated taintMap with τ(T) for all reachable T
     *
     *   1. Initialize: queue ← [startTxHash], visited ← {}, taintMap[startTxHash] ← 1.0
     *   2. While queue not empty AND hops < maxHops:
     *        T_current ← queue.pop()
     *        for each T_child that spends outputs of T_current:
     *            if T_child not in visited:
     *                τ(T_child) ← calculateTaint(T_child)
     *                if τ(T_child) > δ:  // Only propagate if above threshold
     *                    taintMap[T_child] ← τ(T_child)
     *                    queue.push(T_child)
     *                visited.add(T_child)
     *   3. Return taintMap
     *
     * COMPLEXITY:
     *   Time: O(min(|E|, maxHops × branching_factor))
     *   Space: O(min(|V|, maxHops × branching_factor))
     *
     * CONVERGENCE:
     *   Taint diminishes by dilution as funds mix with clean funds.
     *   Eventually τ < δ and propagation stops.
     *   Worst case: Pure stolen funds never mix → τ = 1.0 forever (correct!)
     *
     * @param startTxHash: Stolen transaction to start from
     * @param maxHops: Maximum graph distance to propagate (default 10)
     */
    void propagateTaint(const std::string& startTxHash, uint32_t maxHops = 10);

    /**
     * Gets taint score for a transaction.
     *
     * @return τ(T) ∈ [0, 1], or 0.0 if not tainted
     * COMPLEXITY: O(1)
     */
    double getTaintScore(const std::string& txHash) const;

    /**
     * Gets full taint information (score + provenance).
     *
     * COMPLEXITY: O(1)
     */
    TaintInfo getTaintInfo(const std::string& txHash) const;

    // ═════════════════════════════════════════════════════════════════════
    //                  FRAUD DETECTION RULES
    // ═════════════════════════════════════════════════════════════════════

    /**
     * Checks a transaction against all 5 fraud detection rules.
     * Returns vector of alerts (empty if clean).
     *
     * COMPLEXITY: O(|inputs(T)| + |outputs(T)|)
     */
    std::vector<FraudAlert> checkTransaction(const Transaction& tx);

    /**
     * Checks all transactions involving an address.
     *
     * COMPLEXITY: O(|transactions_for_address| × |inputs| × |outputs|)
     */
    std::vector<FraudAlert> checkAddress(const std::string& address);

    // ═════════════════════════════════════════════════════════════════════
    //                  ALERT MANAGEMENT
    // ═════════════════════════════════════════════════════════════════════

    std::vector<FraudAlert> getAlerts(FraudAlert::AlertLevel minLevel = FraudAlert::AlertLevel::LOW) const;
    std::vector<FraudAlert> getAddressAlerts(const std::string& address) const;
    void clearAlerts();

    // ═════════════════════════════════════════════════════════════════════
    //                  ADDRESS FLAGGING
    // ═════════════════════════════════════════════════════════════════════

    void flagAddress(const std::string& address, const std::string& reason);
    void unflagAddress(const std::string& address);
    bool isAddressFlagged(const std::string& address) const;
    std::vector<std::string> getFlaggedAddresses() const;

    // ═════════════════════════════════════════════════════════════════════
    //                  CLEAN ZONE REGISTRY
    // ═════════════════════════════════════════════════════════════════════

    void registerExchange(const std::string& address, const std::string& name);
    void registerStakingPool(const std::string& address, const std::string& name);
    void registerMerchant(const std::string& address, const std::string& name);
    void registerValidator(const std::string& address, const std::string& name);
    void unregisterCleanZone(const std::string& address);
    bool isCleanZone(const std::string& address) const;
    std::string getCleanZoneType(const std::string& address) const;
    std::vector<std::string> getAllCleanZones() const;

    // ═════════════════════════════════════════════════════════════════════
    //                  GRAPH ANALYSIS
    // ═════════════════════════════════════════════════════════════════════

    /**
     * Flow path in transaction graph.
     * Used for visualizing taint propagation and generating proofs.
     */
    struct FlowPath {
        std::vector<std::string> transactions; // [T_s, T_1, ..., T_c]
        std::vector<std::string> addresses;    // [addr_s, addr_1, ..., addr_c]
        std::vector<double> taintScores;       // [τ(T_s), τ(T_1), ..., τ(T_c)]
        double finalTaint;                     // τ(T_c)
        uint32_t hops;                         // d(T_s, T_c) in graph G
    };

    /**
     * Traces all paths from a stolen transaction to current holders.
     * Returns up to maxHops paths sorted by taint score (highest first).
     *
     * COMPLEXITY: O(|E| × maxHops) worst case
     */
    std::vector<FlowPath> traceTaintedFlow(const std::string& startTxHash, uint32_t maxHops = 10);

    // ═════════════════════════════════════════════════════════════════════
    //                  STATISTICS
    // ═════════════════════════════════════════════════════════════════════

    struct FraudStats {
        uint32_t totalStolenTx;        // |S|: Size of seed set
        uint32_t totalTaintedTx;       // |{T | τ(T) > 0}|
        uint32_t totalAlerts;          // Total alerts generated
        uint32_t criticalAlerts;       // Count by severity
        uint32_t highAlerts;
        uint32_t mediumAlerts;
        uint32_t lowAlerts;
        uint32_t flaggedAddresses;     // Number of flagged addresses
        double totalTaintedValue;      // Σ (balance × τ) for all tainted txs
    };

    FraudStats getStatistics() const;

    // ═════════════════════════════════════════════════════════════════════
    //                  CONSENSUS INTEGRATION
    // ═════════════════════════════════════════════════════════════════════

    /**
     * Determines if a transaction should be blocked by consensus.
     * Returns true if τ > 0.8 (critical taint).
     */
    bool shouldBlockTransaction(const Transaction& tx) const;

    /**
     * Determines if an address should be frozen.
     * Returns true if flagged with CRITICAL alerts.
     */
    bool shouldFreezeAddress(const std::string& address) const;

    // ═════════════════════════════════════════════════════════════════════
    //                  REVERSAL SYSTEM (POF Generation & Execution)
    // ═════════════════════════════════════════════════════════════════════

    /**
     * PROOF OF FEASIBILITY GENERATION
     * ────────────────────────────────
     *
     * Generates a cryptographic proof that reversal is valid.
     * Called by admin system when fraud report is approved.
     *
     * ALGORITHM:
     *   1. Verify stolen_tx ∈ S (seed set)
     *   2. Find path P from stolen_tx to current_holder in graph G (BFS)
     *   3. Compute τ(current_tx) via taint propagation
     *   4. Calculate recoverable amount: R = ⌊τ × balance⌋
     *   5. Generate proof structure with all fields
     *   6. Hash proof: proof_hash = SHA256(all fields)
     *   7. Sign proof: approval_signature = ECDSA_sign(proof_hash, admin_privkey)
     *   8. Return complete POF
     *
     * COMPLEXITY: O(|E|) for graph search + O(1) for crypto
     */
    ProofOfFeasibility generateReversalProof(const std::string& stolen_tx,
                                            const std::string& current_holder,
                                            const std::string& admin_id,
                                            const std::string& admin_signature);

    /**
     * PROOF VALIDATION
     * ────────────────
     *
     * Validates a Proof of Feasibility.
     *
     * CHECKS:
     *   1. Verify path exists in graph G
     *   2. Verify taint calculation is correct
     *   3. Verify recoverable amount: R = ⌊τ × balance⌋
     *   4. Verify proof_hash = SHA256(all fields)
     *   5. Verify signature: ECDSA_verify(proof_hash, approval_signature, admin_pubkey)
     *   6. Verify time constraints (reversal window)
     *
     * COMPLEXITY: O(|trace_path|) for verification
     */
    bool validateReversalProof(const ProofOfFeasibility& proof);

    /**
     * REVERSAL EXECUTION
     * ──────────────────
     *
     * Executes a validated reversal transaction.
     *
     * STEPS:
     *   1. Validate POF
     *   2. Create REVERSAL transaction
     *   3. Debit current_holder (amount = POF.recoverable_amount)
     *   4. Credit origin_owner (victim)
     *   5. Pay fee from fraud detection pool
     *   6. Add transaction to blockchain
     *   7. Update taint map (recovered funds now clean)
     *
     * ATOMICITY: Either fully succeeds or fully fails (no partial reversals)
     *
     * COMPLEXITY: O(|trace_path|) validation + O(1) execution
     */
    bool executeReversal(const ProofOfFeasibility& proof);

    // ═════════════════════════════════════════════════════════════════════
    //                  PERSISTENCE (Export/Import State)
    // ═════════════════════════════════════════════════════════════════════

    std::string exportState() const;
    void importState(const std::string& state);
};
