#include "../include/FraudDetection.h"
#include "../include/blockchain.h"
#include "../include/transaction.h"
#include "../include/Logger.h"
#include "../include/ProofGenerator.h"
#include "../include/ReversalExecutor.h"
#include <algorithm>
#include <sstream>

FraudDetection::FraudDetection(Blockchain* bc) : blockchain(bc) {
    LOG_INFO("Fraud Detection System initialized");
}

FraudDetection::~FraudDetection() {
    LOG_INFO("Fraud Detection System shutdown");
}

// ============================================================================
// SEED MANAGEMENT
// ============================================================================

void FraudDetection::markAsStolen(const std::string& txHash) {
    stolenTransactions.insert(txHash);
    
    // Initialize taint: τ(T) = 1 for stolen transactions
    TaintInfo taint(1.0, txHash, std::time(nullptr));
    taint.ancestry.push_back(txHash);
    taintMap[txHash] = taint;
    
    LOG_SECURITY(LogLevel::CRITICAL, 
        "Transaction marked as stolen: " + txHash, "FraudDetection");
    
    // Automatically propagate taint
    propagateTaint(txHash);
}

void FraudDetection::removeFromStolen(const std::string& txHash) {
    stolenTransactions.erase(txHash);
    taintMap.erase(txHash);
    
    LOG_SECURITY(LogLevel::INFO, 
        "Transaction removed from stolen list: " + txHash, "FraudDetection");
}

bool FraudDetection::isStolen(const std::string& txHash) const {
    return stolenTransactions.find(txHash) != stolenTransactions.end();
}

// ============================================================================
// TAINT PROPAGATION (THE CORE ALGORITHM)
// ============================================================================

/**
 * Propagate taint through the transaction graph using BFS
 * 
 * Algorithm:
 * 1. Start from stolen transaction (τ = 1)
 * 2. For each descendant transaction T_j:
 *    τ(T_j) = Σ w_i · τ(T_i)
 *    where w_i = value_from_T_i / total_input_value
 * 3. Stop when τ < δ (TAINT_THRESHOLD)
 * 
 * This is O(edges touched), not O(chain size)
 */
void FraudDetection::propagateTaint(const std::string& startTxHash, uint32_t maxHops) {
    std::queue<std::pair<std::string, uint32_t>> queue; // (txHash, depth)
    std::unordered_set<std::string> visited;
    
    queue.push({startTxHash, 0});
    visited.insert(startTxHash);
    
    LOG_INFO("Starting taint propagation from: " + startTxHash);
    
    while (!queue.empty()) {
        auto [currentTxHash, depth] = queue.front();
        queue.pop();
        
        // Stop at max hops
        if (depth >= maxHops) {
            continue;
        }
        
        // Get current taint
        auto it = taintMap.find(currentTxHash);
        if (it == taintMap.end()) {
            continue;
        }
        
        double currentTaint = it->second.taintScore;
        
        // Stop if taint below threshold (δ)
        if (currentTaint < TAINT_THRESHOLD) {
            continue;
        }
        
        // Find all transactions that spend outputs from this transaction
        auto descendants = blockchain->getDescendantTransactions(currentTxHash);
        
        for (const auto& descendantTxHash : descendants) {
            if (visited.find(descendantTxHash) != visited.end()) {
                continue;
            }
            
            visited.insert(descendantTxHash);
            
            // Get the descendant transaction
            auto descendantTx = blockchain->getTransaction(descendantTxHash);
            if (!descendantTx) {
                continue;
            }
            
            // Calculate taint for descendant
            double descendantTaint = calculateTaint(*descendantTx);
            
            if (descendantTaint > 0) {
                // Store taint info
                TaintInfo taint(descendantTaint, it->second.sourceTransaction, std::time(nullptr));
                taint.ancestry = it->second.ancestry;
                taint.ancestry.push_back(descendantTxHash);
                taintMap[descendantTxHash] = taint;
                
                LOG_DEBUG("Taint propagated to " + descendantTxHash + 
                         " with score: " + std::to_string(descendantTaint));
                
                // Check for rule violations
                auto alerts = checkTransaction(*descendantTx);
                for (const auto& alert : alerts) {
                    addAlert(alert);
                }
                
                // Continue propagation
                queue.push({descendantTxHash, depth + 1});
            }
        }
    }
    
    LOG_INFO("Taint propagation complete. Visited " + std::to_string(visited.size()) + " transactions");
}

/**
 * Calculate taint for a transaction based on its inputs
 * 
 * Formula: τ(T_j) = Σ w_i · τ(T_i)
 * where w_i = value_from_T_i / total_input_value
 */
double FraudDetection::calculateTaint(const Transaction& tx) {
    const auto& inputs = tx.getInputs();
    
    if (inputs.empty()) {
        return 0.0; // Coinbase transactions have no taint
    }
    
    double totalInputValue = 0.0;
    double weightedTaintSum = 0.0;
    
    for (const auto& input : inputs) {
        totalInputValue += input.amount;
        
        // Get taint of input transaction
        auto it = taintMap.find(input.txHash);
        if (it != taintMap.end()) {
            double inputTaint = it->second.taintScore;
            double weight = input.amount / totalInputValue;
            weightedTaintSum += weight * inputTaint;
        }
    }
    
    return weightedTaintSum;
}

double FraudDetection::getTaintScore(const std::string& txHash) const {
    auto it = taintMap.find(txHash);
    return (it != taintMap.end()) ? it->second.taintScore : 0.0;
}

TaintInfo FraudDetection::getTaintInfo(const std::string& txHash) const {
    auto it = taintMap.find(txHash);
    return (it != taintMap.end()) ? it->second : TaintInfo();
}

// ============================================================================
// RULE CHECKING (THE INTELLIGENCE)
// ============================================================================

std::vector<FraudAlert> FraudDetection::checkTransaction(const Transaction& tx) {
    std::vector<FraudAlert> alerts;
    
    auto taintIt = taintMap.find(tx.getHash());
    if (taintIt == taintMap.end() || taintIt->second.taintScore < TAINT_THRESHOLD) {
        return alerts; // No taint, no alerts
    }
    
    const TaintInfo& taint = taintIt->second;
    
    // Rule 1: Velocity Anomaly
    if (checkVelocityAnomaly(tx, taint)) {
        FraudAlert alert;
        alert.transactionHash = tx.getHash();
        alert.address = tx.getSenderAddress();
        alert.rule = FraudAlert::RuleViolation::VELOCITY_ANOMALY;
        alert.taintScore = taint.taintScore;
        alert.level = calculateAlertLevel(taint.taintScore, 1);
        alert.description = "Rapid fund movement detected (< 5 minutes between transactions)";
        alert.timestamp = std::time(nullptr);
        alert.evidence.push_back("Time delta: < " + std::to_string(VELOCITY_EPSILON) + " seconds");
        alert.evidence.push_back("Taint score: " + std::to_string(taint.taintScore));
        alerts.push_back(alert);
    }
    
    // Rule 2: Fan-Out Pattern (Smurfing)
    if (checkFanOutPattern(tx, taint)) {
        FraudAlert alert;
        alert.transactionHash = tx.getHash();
        alert.address = tx.getSenderAddress();
        alert.rule = FraudAlert::RuleViolation::FAN_OUT_PATTERN;
        alert.taintScore = taint.taintScore;
        alert.level = calculateAlertLevel(taint.taintScore, 1);
        alert.description = "Fund splitting pattern detected (smurfing)";
        alert.timestamp = std::time(nullptr);
        alert.evidence.push_back("Output count: > " + std::to_string(FAN_OUT_K));
        alert.evidence.push_back("Taint score: " + std::to_string(taint.taintScore));
        alerts.push_back(alert);
    }
    
    // Rule 3: Re-Aggregation (Layering)
    if (checkReAggregation(tx)) {
        FraudAlert alert;
        alert.transactionHash = tx.getHash();
        alert.address = tx.getReceiverAddress();
        alert.rule = FraudAlert::RuleViolation::RE_AGGREGATION;
        alert.taintScore = taint.taintScore;
        alert.level = calculateAlertLevel(taint.taintScore, 1);
        alert.description = "Fund re-aggregation detected (layering)";
        alert.timestamp = std::time(nullptr);
        alert.evidence.push_back("Multiple tainted inputs combined");
        alert.evidence.push_back("Combined taint: > " + std::to_string(RE_AGG_THETA));
        alerts.push_back(alert);
    }
    
    // Rule 4: Dormancy then Activation
    if (checkDormancyActivation(tx, taint)) {
        FraudAlert alert;
        alert.transactionHash = tx.getHash();
        alert.address = tx.getSenderAddress();
        alert.rule = FraudAlert::RuleViolation::DORMANCY_ACTIVATION;
        alert.taintScore = taint.taintScore;
        alert.level = calculateAlertLevel(taint.taintScore, 1);
        alert.description = "Dormant funds suddenly activated";
        alert.timestamp = std::time(nullptr);
        alert.evidence.push_back("Dormancy period: > " + std::to_string(DORMANCY_PERIOD) + " seconds");
        alert.evidence.push_back("Taint score: " + std::to_string(taint.taintScore));
        alerts.push_back(alert);
    }
    
    // Rule 5: Clean Zone Entry
    if (checkCleanZoneEntry(tx, taint)) {
        FraudAlert alert;
        alert.transactionHash = tx.getHash();
        alert.address = tx.getReceiverAddress();
        alert.rule = FraudAlert::RuleViolation::CLEAN_ZONE_ENTRY;
        alert.taintScore = taint.taintScore;
        alert.level = FraudAlert::AlertLevel::CRITICAL; // Always critical
        alert.description = "Tainted funds entering clean economic zone (exchange/staking)";
        alert.timestamp = std::time(nullptr);
        alert.evidence.push_back("Destination: Clean zone address");
        alert.evidence.push_back("Taint score: " + std::to_string(taint.taintScore));
        alerts.push_back(alert);
    }
    
    return alerts;
}

// Rule 1: Velocity Anomaly
bool FraudDetection::checkVelocityAnomaly(const Transaction& tx, const TaintInfo& taint) {
    if (taint.ancestry.size() < 2) {
        return false;
    }
    
    // Get previous transaction in ancestry
    const std::string& prevTxHash = taint.ancestry[taint.ancestry.size() - 2];
    auto prevTx = blockchain->getTransaction(prevTxHash);
    
    if (!prevTx) {
        return false;
    }
    
    // Check time delta
    uint64_t timeDelta = tx.getTimestamp() - prevTx->getTimestamp();
    
    return (timeDelta < VELOCITY_EPSILON && taint.taintScore > HIGH_TAINT);
}

// Rule 2: Fan-Out Pattern
bool FraudDetection::checkFanOutPattern(const Transaction& tx, const TaintInfo& taint) {
    const auto& outputs = tx.getOutputs();
    
    // Count unique recipient addresses
    std::unordered_set<std::string> recipients;
    for (const auto& output : outputs) {
        recipients.insert(output.address);
    }
    
    return (recipients.size() > FAN_OUT_K && taint.taintScore > TAINT_THRESHOLD);
}

// Rule 3: Re-Aggregation
bool FraudDetection::checkReAggregation(const Transaction& tx) {
    const auto& inputs = tx.getInputs();
    
    if (inputs.size() < 2) {
        return false;
    }
    
    // Sum taint from all inputs
    double totalTaint = 0.0;
    for (const auto& input : inputs) {
        auto it = taintMap.find(input.txHash);
        if (it != taintMap.end()) {
            totalTaint += it->second.taintScore;
        }
    }
    
    return (totalTaint > RE_AGG_THETA);
}

// Rule 4: Dormancy then Activation
bool FraudDetection::checkDormancyActivation(const Transaction& tx, const TaintInfo& taint) {
    // Check if funds were dormant
    uint64_t currentTime = std::time(nullptr);
    uint64_t dormancyPeriod = currentTime - taint.timestamp;
    
    return (dormancyPeriod > DORMANCY_PERIOD && taint.taintScore > TAINT_THRESHOLD);
}

// Rule 5: Clean Zone Entry
bool FraudDetection::checkCleanZoneEntry(const Transaction& tx, const TaintInfo& taint) {
    const std::string& destination = tx.getReceiverAddress();
    
    // Check if it's a staking transaction
    if (tx.getType() == TransactionType::STAKE) {
        return taint.taintScore > TAINT_THRESHOLD;
    }
    
    // Check if destination is a registered clean zone
    if (isCleanZone(destination)) {
        return taint.taintScore > TAINT_THRESHOLD;
    }
    
    return false;
}

// ============================================================================
// ALERT MANAGEMENT
// ============================================================================

FraudAlert::AlertLevel FraudDetection::calculateAlertLevel(double taintScore, uint32_t ruleViolations) {
    if (taintScore >= CRITICAL_TAINT || ruleViolations >= 3) {
        return FraudAlert::AlertLevel::CRITICAL;
    } else if (taintScore >= HIGH_TAINT || ruleViolations >= 2) {
        return FraudAlert::AlertLevel::HIGH;
    } else if (taintScore >= TAINT_THRESHOLD) {
        return FraudAlert::AlertLevel::MEDIUM;
    }
    return FraudAlert::AlertLevel::LOW;
}

void FraudDetection::addAlert(const FraudAlert& alert) {
    alerts.push_back(alert);
    addressAlerts[alert.address].push_back(alert);
    
    // Log based on severity
    std::string levelStr;
    switch (alert.level) {
        case FraudAlert::AlertLevel::CRITICAL:
            levelStr = "CRITICAL";
            LOG_SECURITY(LogLevel::CRITICAL, "FRAUD ALERT: " + alert.description, "FraudDetection");
            break;
        case FraudAlert::AlertLevel::HIGH:
            levelStr = "HIGH";
            LOG_SECURITY(LogLevel::ERROR, "FRAUD ALERT: " + alert.description, "FraudDetection");
            break;
        case FraudAlert::AlertLevel::MEDIUM:
            levelStr = "MEDIUM";
            LOG_SECURITY(LogLevel::WARNING, "FRAUD ALERT: " + alert.description, "FraudDetection");
            break;
        case FraudAlert::AlertLevel::LOW:
            levelStr = "LOW";
            LOG_SECURITY(LogLevel::INFO, "FRAUD ALERT: " + alert.description, "FraudDetection");
            break;
    }
    
    LOG_SECURITY(LogLevel::INFO, 
        "Alert Level: " + levelStr + 
        " | TX: " + alert.transactionHash + 
        " | Taint: " + std::to_string(alert.taintScore), "FraudDetection");
}

std::vector<FraudAlert> FraudDetection::getAlerts(FraudAlert::AlertLevel minLevel) const {
    std::vector<FraudAlert> filtered;
    
    for (const auto& alert : alerts) {
        if (alert.level >= minLevel) {
            filtered.push_back(alert);
        }
    }
    
    return filtered;
}

std::vector<FraudAlert> FraudDetection::getAddressAlerts(const std::string& address) const {
    auto it = addressAlerts.find(address);
    return (it != addressAlerts.end()) ? it->second : std::vector<FraudAlert>();
}

void FraudDetection::clearAlerts() {
    alerts.clear();
    addressAlerts.clear();
    LOG_INFO("All fraud alerts cleared");
}

// ============================================================================
// ADDRESS FLAGGING
// ============================================================================

void FraudDetection::flagAddress(const std::string& address, const std::string& reason) {
    flaggedAddresses.insert(address);
    LOG_SECURITY(LogLevel::WARNING, 
        "Address flagged: " + address + " | Reason: " + reason, "FraudDetection");
}

void FraudDetection::unflagAddress(const std::string& address) {
    flaggedAddresses.erase(address);
    LOG_INFO("Address unflagged: " + address);
}

bool FraudDetection::isAddressFlagged(const std::string& address) const {
    return flaggedAddresses.find(address) != flaggedAddresses.end();
}

std::vector<std::string> FraudDetection::getFlaggedAddresses() const {
    return std::vector<std::string>(flaggedAddresses.begin(), flaggedAddresses.end());
}

// ============================================================================
// STATISTICS
// ============================================================================

FraudDetection::FraudStats FraudDetection::getStatistics() const {
    FraudStats stats;
    stats.totalStolenTx = stolenTransactions.size();
    stats.totalTaintedTx = taintMap.size();
    stats.totalAlerts = alerts.size();
    stats.criticalAlerts = 0;
    stats.highAlerts = 0;
    stats.mediumAlerts = 0;
    stats.lowAlerts = 0;
    stats.flaggedAddresses = flaggedAddresses.size();
    stats.totalTaintedValue = 0.0;
    
    for (const auto& alert : alerts) {
        switch (alert.level) {
            case FraudAlert::AlertLevel::CRITICAL:
                stats.criticalAlerts++;
                break;
            case FraudAlert::AlertLevel::HIGH:
                stats.highAlerts++;
                break;
            case FraudAlert::AlertLevel::MEDIUM:
                stats.mediumAlerts++;
                break;
            case FraudAlert::AlertLevel::LOW:
                stats.lowAlerts++;
                break;
        }
    }
    
    return stats;
}

// ============================================================================
// CONSENSUS INTEGRATION
// ============================================================================

bool FraudDetection::shouldBlockTransaction(const Transaction& tx) const {
    // Block if sender or receiver is flagged
    if (isAddressFlagged(tx.getSenderAddress()) || isAddressFlagged(tx.getReceiverAddress())) {
        return true;
    }
    
    // Block if taint is critical
    double taint = getTaintScore(tx.getHash());
    if (taint >= CRITICAL_TAINT) {
        return true;
    }
    
    return false;
}

bool FraudDetection::shouldFreezeAddress(const std::string& address) const {
    // Freeze if flagged
    if (isAddressFlagged(address)) {
        return true;
    }
    
    // Freeze if multiple critical alerts
    auto alerts = getAddressAlerts(address);
    uint32_t criticalCount = 0;
    
    for (const auto& alert : alerts) {
        if (alert.level == FraudAlert::AlertLevel::CRITICAL) {
            criticalCount++;
        }
    }
    
    return (criticalCount >= 2);
}

// ============================================================================
// CLEAN ZONE REGISTRY MANAGEMENT
// ============================================================================

void FraudDetection::registerExchange(const std::string& address, const std::string& name) {
    CleanZoneInfo info;
    info.type = CleanZoneType::EXCHANGE;
    info.name = name;
    info.registeredAt = std::time(nullptr);
    
    cleanZoneRegistry[address] = info;
    
    LOG_SECURITY(LogLevel::INFO, 
        "Exchange registered: " + name + " (" + address + ")", "FraudDetection");
}

void FraudDetection::registerStakingPool(const std::string& address, const std::string& name) {
    CleanZoneInfo info;
    info.type = CleanZoneType::STAKING_POOL;
    info.name = name;
    info.registeredAt = std::time(nullptr);
    
    cleanZoneRegistry[address] = info;
    
    LOG_SECURITY(LogLevel::INFO, 
        "Staking pool registered: " + name + " (" + address + ")", "FraudDetection");
}

void FraudDetection::registerMerchant(const std::string& address, const std::string& name) {
    CleanZoneInfo info;
    info.type = CleanZoneType::MERCHANT;
    info.name = name;
    info.registeredAt = std::time(nullptr);
    
    cleanZoneRegistry[address] = info;
    
    LOG_SECURITY(LogLevel::INFO, 
        "Merchant registered: " + name + " (" + address + ")", "FraudDetection");
}

void FraudDetection::registerValidator(const std::string& address, const std::string& name) {
    CleanZoneInfo info;
    info.type = CleanZoneType::VALIDATOR;
    info.name = name;
    info.registeredAt = std::time(nullptr);
    
    cleanZoneRegistry[address] = info;
    
    LOG_SECURITY(LogLevel::INFO, 
        "Validator registered: " + name + " (" + address + ")", "FraudDetection");
}

void FraudDetection::unregisterCleanZone(const std::string& address) {
    auto it = cleanZoneRegistry.find(address);
    if (it != cleanZoneRegistry.end()) {
        std::string name = it->second.name;
        cleanZoneRegistry.erase(it);
        
        LOG_SECURITY(LogLevel::INFO, 
            "Clean zone unregistered: " + name + " (" + address + ")", "FraudDetection");
    }
}

bool FraudDetection::isCleanZone(const std::string& address) const {
    return cleanZoneRegistry.find(address) != cleanZoneRegistry.end();
}

std::string FraudDetection::getCleanZoneType(const std::string& address) const {
    auto it = cleanZoneRegistry.find(address);
    if (it == cleanZoneRegistry.end()) {
        return "NOT_REGISTERED";
    }
    
    switch (it->second.type) {
        case CleanZoneType::EXCHANGE:
            return "EXCHANGE";
        case CleanZoneType::STAKING_POOL:
            return "STAKING_POOL";
        case CleanZoneType::MERCHANT:
            return "MERCHANT";
        case CleanZoneType::VALIDATOR:
            return "VALIDATOR";
        default:
            return "UNKNOWN";
    }
}

std::vector<std::string> FraudDetection::getAllCleanZones() const {
    std::vector<std::string> zones;
    zones.reserve(cleanZoneRegistry.size());
    
    for (const auto& pair : cleanZoneRegistry) {
        zones.push_back(pair.first);
    }
    
    return zones;
}

// ============================================================================
// ADDRESS CHECKING
// ============================================================================

std::vector<FraudAlert> FraudDetection::checkAddress(const std::string& address) {
    std::vector<FraudAlert> addressSpecificAlerts;
    
    // Get all transactions involving this address
    auto transactions = blockchain->getTransactionsByAddress(address);
    
    for (const auto& tx : transactions) {
        auto txAlerts = checkTransaction(*tx);
        for (const auto& alert : txAlerts) {
            if (alert.address == address) {
                addressSpecificAlerts.push_back(alert);
            }
        }
    }
    
    return addressSpecificAlerts;
}

// ============================================================================
// GRAPH ANALYSIS
// ============================================================================

std::vector<FraudDetection::FlowPath> FraudDetection::traceTaintedFlow(
    const std::string& startTxHash, uint32_t maxHops) {
    
    std::vector<FlowPath> paths;
    std::queue<FlowPath> queue;
    
    // Initialize with start transaction
    FlowPath initialPath;
    initialPath.transactions.push_back(startTxHash);
    initialPath.taintScores.push_back(getTaintScore(startTxHash));
    initialPath.finalTaint = getTaintScore(startTxHash);
    initialPath.hops = 0;
    
    queue.push(initialPath);
    
    while (!queue.empty()) {
        FlowPath currentPath = queue.front();
        queue.pop();
        
        if (currentPath.hops >= maxHops) {
            paths.push_back(currentPath);
            continue;
        }
        
        const std::string& lastTx = currentPath.transactions.back();
        auto descendants = blockchain->getDescendantTransactions(lastTx);
        
        if (descendants.empty()) {
            paths.push_back(currentPath);
            continue;
        }
        
        for (const auto& descTxHash : descendants) {
            double descTaint = getTaintScore(descTxHash);
            
            if (descTaint < TAINT_THRESHOLD) {
                continue;
            }
            
            FlowPath newPath = currentPath;
            newPath.transactions.push_back(descTxHash);
            newPath.taintScores.push_back(descTaint);
            newPath.finalTaint = descTaint;
            newPath.hops++;
            
            auto tx = blockchain->getTransaction(descTxHash);
            if (tx) {
                newPath.addresses.push_back(tx->getReceiverAddress());
            }
            
            queue.push(newPath);
        }
    }
    
    return paths;
}

// ============================================================================
// PERSISTENCE
// ============================================================================

std::string FraudDetection::exportState() const {
    std::ostringstream oss;
    
    // Export stolen transactions
    oss << "STOLEN:" << stolenTransactions.size() << "\n";
    for (const auto& txHash : stolenTransactions) {
        oss << txHash << "\n";
    }
    
    // Export taint map
    oss << "TAINT:" << taintMap.size() << "\n";
    for (const auto& pair : taintMap) {
        oss << pair.first << "|"
            << pair.second.taintScore << "|"
            << pair.second.sourceTransaction << "|"
            << pair.second.timestamp << "\n";
    }
    
    // Export flagged addresses
    oss << "FLAGGED:" << flaggedAddresses.size() << "\n";
    for (const auto& address : flaggedAddresses) {
        oss << address << "\n";
    }
    
    // Export clean zones
    oss << "CLEANZONES:" << cleanZoneRegistry.size() << "\n";
    for (const auto& pair : cleanZoneRegistry) {
        oss << pair.first << "|"
            << static_cast<int>(pair.second.type) << "|"
            << pair.second.name << "|"
            << pair.second.registeredAt << "\n";
    }
    
    return oss.str();
}

void FraudDetection::importState(const std::string& state) {
    std::istringstream iss(state);
    std::string line;
    std::string section;
    
    while (std::getline(iss, line)) {
        if (line.find("STOLEN:") == 0) {
            section = "STOLEN";
            continue;
        } else if (line.find("TAINT:") == 0) {
            section = "TAINT";
            continue;
        } else if (line.find("FLAGGED:") == 0) {
            section = "FLAGGED";
            continue;
        } else if (line.find("CLEANZONES:") == 0) {
            section = "CLEANZONES";
            continue;
        }
        
        if (section == "STOLEN") {
            stolenTransactions.insert(line);
        } else if (section == "TAINT") {
            std::istringstream lineStream(line);
            std::string txHash, sourceHash;
            double score;
            uint64_t timestamp;
            
            std::getline(lineStream, txHash, '|');
            lineStream >> score;
            lineStream.ignore(1);
            std::getline(lineStream, sourceHash, '|');
            lineStream >> timestamp;
            
            TaintInfo taint(score, sourceHash, timestamp);
            taintMap[txHash] = taint;
        } else if (section == "FLAGGED") {
            flaggedAddresses.insert(line);
        } else if (section == "CLEANZONES") {
            std::istringstream lineStream(line);
            std::string address, name;
            int typeInt;
            uint64_t registeredAt;
            
            std::getline(lineStream, address, '|');
            lineStream >> typeInt;
            lineStream.ignore(1);
            std::getline(lineStream, name, '|');
            lineStream >> registeredAt;
            
            CleanZoneInfo info;
            info.type = static_cast<CleanZoneType>(typeInt);
            info.name = name;
            info.registeredAt = registeredAt;
            
            cleanZoneRegistry[address] = info;
        }
    }
    
    LOG_INFO("Fraud detection state imported successfully");
}

// ===== REVERSAL SYSTEM INTEGRATION =====

ProofOfFeasibility FraudDetection::generateReversalProof(const std::string& stolen_tx,
                                                        const std::string& current_holder,
                                                        const std::string& admin_id,
                                                        const std::string& admin_signature) {
    if (!proofGenerator) {
        LOG_ERROR("Proof generator not initialized");
        return ProofOfFeasibility();
    }
    
    return proofGenerator->generateProof(stolen_tx, current_holder, admin_id, admin_signature);
}

bool FraudDetection::validateReversalProof(const ProofOfFeasibility& proof) {
    if (!proofGenerator) {
        LOG_ERROR("Proof generator not initialized");
        return false;
    }
    
    return proofGenerator->validateProof(proof);
}

bool FraudDetection::executeReversal(const ProofOfFeasibility& proof) {
    if (!reversalExecutor) {
        LOG_ERROR("Reversal executor not initialized");
        return false;
    }
    
    return reversalExecutor->executeReversal(proof);
}
