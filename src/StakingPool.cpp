#include "../include/StakingPool.h"
#include "../include/Logger.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <sys/stat.h>
#include <sys/types.h>

StakingPool::StakingPool() : totalStakedCoins(0.0), dbPath("gxc_data/staking_pool_db") {
    // Initialize LevelDB for PRODUCTION
    dbOptions.create_if_missing = true;
    dbOptions.error_if_exists = false;
    
    // Create directory if it doesn't exist
    mkdir("gxc_data", 0755);
    
    // Open database
    leveldb::DB* dbPtr;
    leveldb::Status status = leveldb::DB::Open(dbOptions, dbPath, &dbPtr);
    
    if (status.ok()) {
        db.reset(dbPtr);
        LOG_CORE(LogLevel::INFO, "StakingPool LevelDB opened successfully");
        
        // Load existing data
        loadFromDatabase();
    } else {
        LOG_CORE(LogLevel::ERROR, "Failed to open StakingPool LevelDB: " + status.ToString());
    }
    
    LOG_CORE(LogLevel::INFO, "StakingPool initialized with " + 
             std::to_string(stakes.size()) + " stakes");
}

StakingPool::~StakingPool() {
    LOG_CORE(LogLevel::INFO, "StakingPool destroyed");
}

std::string StakingPool::generateStakeId(const std::string& owner, const std::string& validator) {
    std::stringstream ss;
    ss << "stake_" << owner.substr(0, 8) << "_" << validator.substr(0, 8) 
       << "_" << Utils::getCurrentTimestamp();
    return ss.str();
}

std::string StakingPool::addStake(const std::string& owner, const std::string& validator,
                                 double amount, uint32_t lockDays,
                                 const std::string& stakeTxHash,
                                 const std::string& previousTxHash,
                                 const std::vector<std::string>& inputTxHashes) {
    std::lock_guard<std::mutex> lock(poolMutex);
    
    // Validate parameters
    if (amount < MIN_STAKE_AMOUNT) {
        throw std::runtime_error("Stake amount below minimum: " + std::to_string(MIN_STAKE_AMOUNT));
    }
    
    if (lockDays < MIN_LOCK_PERIOD_DAYS || lockDays > MAX_LOCK_PERIOD_DAYS) {
        throw std::runtime_error("Lock period must be between " + 
                               std::to_string(MIN_LOCK_PERIOD_DAYS) + " and " +
                               std::to_string(MAX_LOCK_PERIOD_DAYS) + " days");
    }
    
    // Create stake entry with traceability
    StakeEntry entry;
    entry.stakeId = generateStakeId(owner, validator);
    entry.ownerAddress = owner;
    entry.validatorAddress = validator;
    entry.amount = amount;
    entry.lockStartTime = Utils::getCurrentTimestamp();
    entry.lockPeriodDays = lockDays;
    entry.isLocked = true;
    entry.accumulatedRewards = 0.0;
    
    // Traceability fields - CRITICAL for audit trail
    entry.stakeTxHash = stakeTxHash;
    entry.previousTxHash = previousTxHash;
    entry.referencedAmount = amount;
    entry.inputTxHashes = inputTxHashes;
    
    // Add to stakes map
    stakes[entry.stakeId] = entry;
    
    // Update totals
    totalStakedCoins += amount;
    
    // Update validator pool
    updateValidatorPool(validator, amount, true);
    
    LOG_CORE(LogLevel::INFO, "Stake added: " + entry.stakeId + 
             ", Amount: " + std::to_string(amount) + 
             ", Validator: " + validator +
             ", Lock: " + std::to_string(lockDays) + " days" +
             ", StakeTxHash: " + stakeTxHash.substr(0, 16) + "..." +
             ", PrevTxHash: " + previousTxHash.substr(0, 16) + "...");
    
    return entry.stakeId;
}

bool StakingPool::removeStake(const std::string& stakeId) {
    std::lock_guard<std::mutex> lock(poolMutex);
    
    auto it = stakes.find(stakeId);
    if (it == stakes.end()) {
        LOG_CORE(LogLevel::ERROR, "Stake not found: " + stakeId);
        return false;
    }
    
    const auto& entry = it->second;
    
    // Check if locked
    if (!entry.isLocked) {
        LOG_CORE(LogLevel::ERROR, "Stake already unstaked: " + stakeId);
        return false;
    }
    
    // Update totals
    totalStakedCoins -= entry.amount;
    
    // Update validator pool
    updateValidatorPool(entry.validatorAddress, entry.amount, false);
    
    LOG_CORE(LogLevel::INFO, "Stake removed: " + stakeId + 
             ", Amount: " + std::to_string(entry.amount));
    
    // Remove from map
    stakes.erase(it);
    
    return true;
}

bool StakingPool::isStakeMature(const std::string& stakeId) const {
    std::lock_guard<std::mutex> lock(poolMutex);
    
    auto it = stakes.find(stakeId);
    if (it == stakes.end()) {
        return false;
    }
    
    const auto& entry = it->second;
    
    // Calculate maturity time
    uint64_t lockDurationSeconds = static_cast<uint64_t>(entry.lockPeriodDays) * 86400;
    uint64_t maturityTime = entry.lockStartTime + lockDurationSeconds;
    uint64_t currentTime = Utils::getCurrentTimestamp();
    
    return currentTime >= maturityTime;
}

bool StakingPool::canUnstake(const std::string& stakeId) const {
    std::lock_guard<std::mutex> lock(poolMutex);
    
    auto it = stakes.find(stakeId);
    if (it == stakes.end()) {
        return false;
    }
    
    const auto& entry = it->second;
    
    // Must be locked and mature
    return entry.isLocked && isStakeMature(stakeId);
}

StakingPool::StakeEntry StakingPool::getStake(const std::string& stakeId) const {
    std::lock_guard<std::mutex> lock(poolMutex);
    
    auto it = stakes.find(stakeId);
    if (it == stakes.end()) {
        throw std::runtime_error("Stake not found: " + stakeId);
    }
    
    return it->second;
}

std::vector<StakingPool::StakeEntry> StakingPool::getStakesForAddress(const std::string& address) const {
    std::lock_guard<std::mutex> lock(poolMutex);
    
    std::vector<StakeEntry> result;
    for (const auto& [stakeId, entry] : stakes) {
        if (entry.ownerAddress == address && entry.isLocked) {
            result.push_back(entry);
        }
    }
    
    return result;
}

std::vector<StakingPool::StakeEntry> StakingPool::getStakesForValidator(const std::string& validator) const {
    std::lock_guard<std::mutex> lock(poolMutex);
    
    std::vector<StakeEntry> result;
    for (const auto& [stakeId, entry] : stakes) {
        if (entry.validatorAddress == validator && entry.isLocked) {
            result.push_back(entry);
        }
    }
    
    return result;
}

bool StakingPool::stakeExists(const std::string& stakeId) const {
    std::lock_guard<std::mutex> lock(poolMutex);
    return stakes.find(stakeId) != stakes.end();
}

void StakingPool::initializeValidatorPool(const std::string& validatorAddress, double ownStake) {
    std::lock_guard<std::mutex> lock(poolMutex);
    
    ValidatorPoolStats stats;
    stats.validatorAddress = validatorAddress;
    stats.ownStake = ownStake;
    stats.totalDelegated = 0.0;
    stats.totalStake = ownStake;
    stats.delegatorCount = 0;
    stats.blocksProduced = 0;
    stats.missedBlocks = 0;
    stats.totalRewardsDistributed = 0.0;
    
    validatorPools[validatorAddress] = stats;
    
    LOG_CORE(LogLevel::INFO, "Validator pool initialized: " + validatorAddress);
}

void StakingPool::updateValidatorPool(const std::string& validatorAddress, double deltaStake, bool isAdd) {
    // Note: poolMutex already locked by caller
    
    auto it = validatorPools.find(validatorAddress);
    if (it == validatorPools.end()) {
        // Initialize if doesn't exist
        initializeValidatorPool(validatorAddress, 0.0);
        it = validatorPools.find(validatorAddress);
    }
    
    if (isAdd) {
        it->second.totalDelegated += deltaStake;
        it->second.totalStake += deltaStake;
        it->second.delegatorCount++;
    } else {
        it->second.totalDelegated -= deltaStake;
        it->second.totalStake -= deltaStake;
        it->second.delegatorCount--;
    }
}

void StakingPool::recordBlockProduced(const std::string& validatorAddress) {
    std::lock_guard<std::mutex> lock(poolMutex);
    
    auto it = validatorPools.find(validatorAddress);
    if (it != validatorPools.end()) {
        it->second.blocksProduced++;
    }
}

void StakingPool::recordMissedBlock(const std::string& validatorAddress) {
    std::lock_guard<std::mutex> lock(poolMutex);
    
    auto it = validatorPools.find(validatorAddress);
    if (it != validatorPools.end()) {
        it->second.missedBlocks++;
    }
}

StakingPool::ValidatorPoolStats StakingPool::getValidatorPoolStats(const std::string& validatorAddress) const {
    std::lock_guard<std::mutex> lock(poolMutex);
    
    auto it = validatorPools.find(validatorAddress);
    if (it == validatorPools.end()) {
        throw std::runtime_error("Validator pool not found: " + validatorAddress);
    }
    
    return it->second;
}

void StakingPool::addReward(const std::string& stakeId, double rewardAmount) {
    std::lock_guard<std::mutex> lock(poolMutex);
    
    auto it = stakes.find(stakeId);
    if (it != stakes.end()) {
        it->second.accumulatedRewards += rewardAmount;
    }
}

double StakingPool::getAccumulatedRewards(const std::string& stakeId) const {
    std::lock_guard<std::mutex> lock(poolMutex);
    
    auto it = stakes.find(stakeId);
    if (it == stakes.end()) {
        return 0.0;
    }
    
    return it->second.accumulatedRewards;
}

double StakingPool::getTotalStaked() const {
    std::lock_guard<std::mutex> lock(poolMutex);
    return totalStakedCoins;
}

double StakingPool::getStakedByAddress(const std::string& address) const {
    std::lock_guard<std::mutex> lock(poolMutex);
    
    double total = 0.0;
    for (const auto& [stakeId, entry] : stakes) {
        if (entry.ownerAddress == address && entry.isLocked) {
            total += entry.amount;
        }
    }
    
    return total;
}

double StakingPool::getStakedWithValidator(const std::string& validator) const {
    std::lock_guard<std::mutex> lock(poolMutex);
    
    auto it = validatorPools.find(validator);
    if (it == validatorPools.end()) {
        return 0.0;
    }
    
    return it->second.totalStake;
}

size_t StakingPool::getTotalStakeCount() const {
    std::lock_guard<std::mutex> lock(poolMutex);
    return stakes.size();
}

size_t StakingPool::getValidatorCount() const {
    std::lock_guard<std::mutex> lock(poolMutex);
    return validatorPools.size();
}

// Serialization helpers for LevelDB
std::string StakingPool::serializeStakeEntry(const StakeEntry& entry) const {
    std::stringstream ss;
    ss << entry.stakeId << "|"
       << entry.ownerAddress << "|"
       << entry.validatorAddress << "|"
       << entry.amount << "|"
       << entry.lockStartTime << "|"
       << entry.lockPeriodDays << "|"
       << (entry.isLocked ? "1" : "0") << "|"
       << entry.accumulatedRewards << "|"
       << entry.stakeTxHash << "|"
       << entry.previousTxHash << "|"
       << entry.referencedAmount << "|";
    
    // Serialize input tx hashes
    for (size_t i = 0; i < entry.inputTxHashes.size(); i++) {
        if (i > 0) ss << ",";
        ss << entry.inputTxHashes[i];
    }
    
    return ss.str();
}

StakingPool::StakeEntry StakingPool::deserializeStakeEntry(const std::string& data) const {
    StakeEntry entry;
    std::stringstream ss(data);
    std::string token;
    
    std::getline(ss, entry.stakeId, '|');
    std::getline(ss, entry.ownerAddress, '|');
    std::getline(ss, entry.validatorAddress, '|');
    
    std::getline(ss, token, '|');
    entry.amount = std::stod(token);
    
    std::getline(ss, token, '|');
    entry.lockStartTime = std::stoull(token);
    
    std::getline(ss, token, '|');
    entry.lockPeriodDays = std::stoul(token);
    
    std::getline(ss, token, '|');
    entry.isLocked = (token == "1");
    
    std::getline(ss, token, '|');
    entry.accumulatedRewards = std::stod(token);
    
    std::getline(ss, entry.stakeTxHash, '|');
    std::getline(ss, entry.previousTxHash, '|');
    
    std::getline(ss, token, '|');
    entry.referencedAmount = std::stod(token);
    
    // Deserialize input tx hashes
    std::string inputHashes;
    std::getline(ss, inputHashes);
    if (!inputHashes.empty()) {
        std::stringstream hashStream(inputHashes);
        std::string hash;
        while (std::getline(hashStream, hash, ',')) {
            entry.inputTxHashes.push_back(hash);
        }
    }
    
    return entry;
}

std::string StakingPool::serializeValidatorStats(const ValidatorPoolStats& stats) const {
    std::stringstream ss;
    ss << stats.validatorAddress << "|"
       << stats.ownStake << "|"
       << stats.totalDelegated << "|"
       << stats.totalStake << "|"
       << stats.delegatorCount << "|"
       << stats.blocksProduced << "|"
       << stats.missedBlocks << "|"
       << stats.totalRewardsDistributed;
    return ss.str();
}

StakingPool::ValidatorPoolStats StakingPool::deserializeValidatorStats(const std::string& data) const {
    ValidatorPoolStats stats;
    std::stringstream ss(data);
    std::string token;
    
    std::getline(ss, stats.validatorAddress, '|');
    
    std::getline(ss, token, '|');
    stats.ownStake = std::stod(token);
    
    std::getline(ss, token, '|');
    stats.totalDelegated = std::stod(token);
    
    std::getline(ss, token, '|');
    stats.totalStake = std::stod(token);
    
    std::getline(ss, token, '|');
    stats.delegatorCount = std::stoul(token);
    
    std::getline(ss, token, '|');
    stats.blocksProduced = std::stoul(token);
    
    std::getline(ss, token, '|');
    stats.missedBlocks = std::stoul(token);
    
    std::getline(ss, token);
    stats.totalRewardsDistributed = std::stod(token);
    
    return stats;
}

bool StakingPool::saveToDatabase() {
    std::lock_guard<std::mutex> lock(poolMutex);
    
    if (!db) {
        LOG_CORE(LogLevel::ERROR, "StakingPool database not initialized");
        return false;
    }
    
    try {
        leveldb::WriteBatch batch;
        
        // Save total staked coins
        batch.Put("meta:totalStaked", std::to_string(totalStakedCoins));
        
        // Save all stakes
        for (const auto& [stakeId, entry] : stakes) {
            std::string key = "stake:" + stakeId;
            std::string value = serializeStakeEntry(entry);
            batch.Put(key, value);
        }
        
        // Save all validator pools
        for (const auto& [validatorAddr, stats] : validatorPools) {
            std::string key = "validator:" + validatorAddr;
            std::string value = serializeValidatorStats(stats);
            batch.Put(key, value);
        }
        
        // Write batch to database
        leveldb::Status status = db->Write(writeOptions, &batch);
        
        if (!status.ok()) {
            LOG_CORE(LogLevel::ERROR, "Failed to write to StakingPool database: " + status.ToString());
            return false;
        }
        
        LOG_CORE(LogLevel::INFO, "StakingPool saved to LevelDB: " + 
                 std::to_string(stakes.size()) + " stakes, " +
                 std::to_string(validatorPools.size()) + " validators");
        
        return true;
    } catch (const std::exception& e) {
        LOG_CORE(LogLevel::ERROR, "Failed to save staking pool: " + std::string(e.what()));
        return false;
    }
}

bool StakingPool::loadFromDatabase() {
    // Note: poolMutex already locked by constructor
    
    if (!db) {
        LOG_CORE(LogLevel::INFO, "StakingPool database not initialized, starting fresh");
        return true;
    }
    
    try {
        // Load total staked coins
        std::string totalStakedStr;
        leveldb::Status status = db->Get(readOptions, "meta:totalStaked", &totalStakedStr);
        if (status.ok()) {
            totalStakedCoins = std::stod(totalStakedStr);
        }
        
        // Load all stakes
        leveldb::Iterator* it = db->NewIterator(readOptions);
        for (it->Seek("stake:"); it->Valid() && it->key().ToString().substr(0, 6) == "stake:"; it->Next()) {
            std::string stakeId = it->key().ToString().substr(6);
            StakeEntry entry = deserializeStakeEntry(it->value().ToString());
            stakes[stakeId] = entry;
        }
        delete it;
        
        // Load all validator pools
        it = db->NewIterator(readOptions);
        for (it->Seek("validator:"); it->Valid() && it->key().ToString().substr(0, 10) == "validator:"; it->Next()) {
            std::string validatorAddr = it->key().ToString().substr(10);
            ValidatorPoolStats stats = deserializeValidatorStats(it->value().ToString());
            validatorPools[validatorAddr] = stats;
        }
        delete it;
        
        LOG_CORE(LogLevel::INFO, "StakingPool loaded from LevelDB: " + 
                 std::to_string(stakes.size()) + " stakes, " +
                 std::to_string(validatorPools.size()) + " validators, " +
                 std::to_string(totalStakedCoins) + " GXC total staked");
        
        return true;
    } catch (const std::exception& e) {
        LOG_CORE(LogLevel::ERROR, "Failed to load staking pool: " + std::string(e.what()));
        return false;
    }
}
