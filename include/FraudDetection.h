#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <ctime>
#include <cmath>

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

class FraudDetection {
private:
    // Taint tracking
    std::unordered_map<std::string, TaintInfo> taintMap; // txHash -> taint
    std::unordered_set<std::string> stolenTransactions;  // Seed set S
    std::unordered_set<std::string> flaggedAddresses;    // Monitored addresses
    
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
    
public:
    FraudDetection(Blockchain* bc);
    ~FraudDetection();
    
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
    
    // Export/Import for persistence
    std::string exportState() const;
    void importState(const std::string& state);
};
