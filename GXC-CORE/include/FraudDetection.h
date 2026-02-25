#pragma once

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

/**
 * Taint Propagation & Fraud Detection System
 * 
 * Based on Proof of Traceability (POT), this system treats the blockchain
 * as a causal graph where stolen funds become a "graph anomaly" that can
 * be detected through mathematical taint propagation.
 * 
 * Core Principle:
 * τ(T_j) = Σ w_i · τ(T_i)
 * where w_i = value_from_T_i / total_input_value
 * 
 * This is conservation of value, not opinion.
 */

struct TaintInfo {
    double taintScore;           // τ ∈ [0,1]
    std::string sourceTransaction; // Original stolen tx
    uint64_t timestamp;
    std::vector<std::string> ancestry; // Path from source
    
    TaintInfo() : taintScore(0.0), timestamp(0) {}
    TaintInfo(double score, const std::string& source, uint64_t ts)
        : taintScore(score), sourceTransaction(source), timestamp(ts) {}
};

struct FraudAlert {
    enum class AlertLevel {
        LOW,
        MEDIUM,
        HIGH,
        CRITICAL
    };
    
    enum class RuleViolation {
        VELOCITY_ANOMALY,      // Rule 1: Too fast movement
        FAN_OUT_PATTERN,       // Rule 2: Smurfing
        RE_AGGREGATION,        // Rule 3: Layering
        DORMANCY_ACTIVATION,   // Rule 4: Wait then move
        CLEAN_ZONE_ENTRY       // Rule 5: Legitimization attempt
    };
    
    std::string transactionHash;
    std::string address;
    AlertLevel level;
    RuleViolation rule;
    double taintScore;
    std::string description;
    uint64_t timestamp;
    std::vector<std::string> evidence;
    
    FraudAlert() : level(AlertLevel::LOW), rule(RuleViolation::VELOCITY_ANOMALY), 
                   taintScore(0.0), timestamp(0) {}
};

/**
 * Proof of Feasibility (POF)
 * 
 * Cryptographic proof that a reversal is valid and feasible.
 * This proof demonstrates:
 * 1. Causality: Funds originated from a stolen transaction
 * 2. Conservation: Reversal amount ≤ recoverable amount
 * 3. Authorization: Admin approval with signature
 * 4. Finality Safety: Within reversal window
 */
struct ProofOfFeasibility {
    std::string proof_type;              // "REVERSAL_PROOF"
    std::string stolen_tx;               // Original stolen transaction hash
    std::string current_tx;              // Current holding transaction hash
    std::vector<std::string> trace_path; // Full path from stolen to current
    double taint_score;                  // Taint score of current tx (τ ∈ [0,1])
    uint64_t recoverable_amount;         // Amount that can be recovered (R = τ × balance)
    std::string origin_owner;            // Original victim address
    std::string current_holder;          // Current holder address
    std::string approved_by;             // Admin ID who approved
    std::string approval_signature;      // Admin signature
    uint64_t timestamp;                  // Approval timestamp
    uint64_t block_height;               // Block height when approved
    std::string proof_hash;              // Hash of entire proof (for verification)
    
    ProofOfFeasibility() : taint_score(0.0), recoverable_amount(0), 
                          timestamp(0), block_height(0) {}
};

/**
 * Reversal Transaction
 * 
 * Special transaction type that reverses stolen funds.
 * Not a rollback - a forward state transition with cryptographic justification.
 */
struct ReversalTransaction {
    std::string tx_hash;                 // Transaction hash
    std::string from;                    // Current holder (debited)
    std::string to;                      // Original victim (credited)
    uint64_t amount;                     // Recoverable amount
    std::string proof_hash;              // Hash(Proof of Feasibility)
    uint64_t fee;                        // Transaction fee (paid by system pool)
    uint64_t timestamp;                  // Transaction timestamp
    uint64_t block_height;               // Block height when executed
    std::string admin_signature;         // Admin approval signature
    
    ReversalTransaction() : amount(0), fee(0), timestamp(0), block_height(0) {}
};

class FraudDetection {
private:
    // Taint tracking
    std::unordered_map<std::string, TaintInfo> taintMap; // txHash -> taint
    std::unordered_set<std::string> stolenTransactions;  // Seed set S
    std::unordered_set<std::string> flaggedAddresses;    // Monitored addresses
    
    // Clean zone registry
    enum class CleanZoneType {
        EXCHANGE,
        STAKING_POOL,
        MERCHANT,
        VALIDATOR
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
    
    // Configuration parameters
    static constexpr double TAINT_THRESHOLD = 0.1;      // δ: Stop propagation below this
    static constexpr double HIGH_TAINT = 0.5;           // High taint threshold
    static constexpr double CRITICAL_TAINT = 0.8;       // Critical taint threshold
    
    static constexpr uint64_t VELOCITY_EPSILON = 300;   // 5 minutes
    static constexpr uint32_t FAN_OUT_K = 5;            // Max outputs before alert
    static constexpr double RE_AGG_THETA = 0.7;         // Re-aggregation threshold
    static constexpr uint64_t DORMANCY_PERIOD = 604800; // 7 days
    
    // Reference to blockchain
    Blockchain* blockchain;
    
    // Internal methods
    double calculateTaint(const Transaction& tx);
    bool checkVelocityAnomaly(const Transaction& tx, const TaintInfo& taint);
    bool checkFanOutPattern(const Transaction& tx, const TaintInfo& taint);
    bool checkReAggregation(const Transaction& tx);
    bool checkDormancyActivation(const Transaction& tx, const TaintInfo& taint);
    bool checkCleanZoneEntry(const Transaction& tx, const TaintInfo& taint);
    
    FraudAlert::AlertLevel calculateAlertLevel(double taintScore, uint32_t ruleViolations);
    void addAlert(const FraudAlert& alert);
    
    // Reversal system (forward declarations)
    class ProofGenerator* proofGenerator;
    class ReversalExecutor* reversalExecutor;
    
public:
    FraudDetection(Blockchain* bc);
    ~FraudDetection();
    
    // Set reversal system components (called after initialization)
    void setReversalSystem(class ProofGenerator* pg, class ReversalExecutor* re) {
        proofGenerator = pg;
        reversalExecutor = re;
    }
    
    // Seed management
    void markAsStolen(const std::string& txHash);
    void removeFromStolen(const std::string& txHash);
    bool isStolen(const std::string& txHash) const;
    
    // Taint propagation (the core algorithm)
    void propagateTaint(const std::string& startTxHash, uint32_t maxHops = 10);
    double getTaintScore(const std::string& txHash) const;
    TaintInfo getTaintInfo(const std::string& txHash) const;
    
    // Rule checking
    std::vector<FraudAlert> checkTransaction(const Transaction& tx);
    std::vector<FraudAlert> checkAddress(const std::string& address);
    
    // Alert management
    std::vector<FraudAlert> getAlerts(FraudAlert::AlertLevel minLevel = FraudAlert::AlertLevel::LOW) const;
    std::vector<FraudAlert> getAddressAlerts(const std::string& address) const;
    void clearAlerts();
    
    // Address flagging
    void flagAddress(const std::string& address, const std::string& reason);
    void unflagAddress(const std::string& address);
    bool isAddressFlagged(const std::string& address) const;
    std::vector<std::string> getFlaggedAddresses() const;
    
    // Clean zone registry management
    void registerExchange(const std::string& address, const std::string& name);
    void registerStakingPool(const std::string& address, const std::string& name);
    void registerMerchant(const std::string& address, const std::string& name);
    void registerValidator(const std::string& address, const std::string& name);
    void unregisterCleanZone(const std::string& address);
    bool isCleanZone(const std::string& address) const;
    std::string getCleanZoneType(const std::string& address) const;
    std::vector<std::string> getAllCleanZones() const;
    
    // Graph analysis
    struct FlowPath {
        std::vector<std::string> transactions;
        std::vector<std::string> addresses;
        std::vector<double> taintScores;
        double finalTaint;
        uint32_t hops;
    };
    
    std::vector<FlowPath> traceTaintedFlow(const std::string& startTxHash, uint32_t maxHops = 10);
    
    // Statistics
    struct FraudStats {
        uint32_t totalStolenTx;
        uint32_t totalTaintedTx;
        uint32_t totalAlerts;
        uint32_t criticalAlerts;
        uint32_t highAlerts;
        uint32_t mediumAlerts;
        uint32_t lowAlerts;
        uint32_t flaggedAddresses;
        double totalTaintedValue;
    };
    
    FraudStats getStatistics() const;
    
    // Consensus integration
    bool shouldBlockTransaction(const Transaction& tx) const;
    bool shouldFreezeAddress(const std::string& address) const;
    
    // Reversal system integration
    /**
     * Generate Proof of Feasibility for reversal
     * Called by admin system when fraud report is approved
     */
    ProofOfFeasibility generateReversalProof(const std::string& stolen_tx,
                                            const std::string& current_holder,
                                            const std::string& admin_id,
                                            const std::string& admin_signature);
    
    /**
     * Validate reversal proof
     */
    bool validateReversalProof(const ProofOfFeasibility& proof);
    
    /**
     * Execute reversal
     * Returns true if successful
     */
    bool executeReversal(const ProofOfFeasibility& proof);
    
    // Export/Import for persistence
    std::string exportState() const;
    void importState(const std::string& state);
};
