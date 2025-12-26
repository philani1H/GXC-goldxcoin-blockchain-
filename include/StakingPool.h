#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <mutex>
#include <memory>
#include <leveldb/db.h>
#include <leveldb/write_batch.h>
#include "Utils.h"

// Staking Pool - Tracks all staked coins without minting new ones
class StakingPool {
public:
    struct StakeEntry {
        std::string stakeId;
        std::string ownerAddress;
        std::string validatorAddress;
        double amount;
        uint64_t lockStartTime;
        uint32_t lockPeriodDays;
        bool isLocked;
        double accumulatedRewards;
        
        // Traceability fields
        std::string stakeTxHash;           // Hash of the staking transaction
        std::string previousTxHash;        // Previous transaction before staking
        double referencedAmount;           // Amount referenced from previous tx
        std::vector<std::string> inputTxHashes;  // Original UTXO transaction hashes
        
        StakeEntry() : amount(0), lockStartTime(0), lockPeriodDays(0), 
                      isLocked(true), accumulatedRewards(0), referencedAmount(0) {}
    };
    
    struct ValidatorPoolStats {
        std::string validatorAddress;
        double ownStake;
        double totalDelegated;
        double totalStake;
        uint32_t delegatorCount;
        uint32_t blocksProduced;
        uint32_t missedBlocks;
        double totalRewardsDistributed;
    };

private:
    std::unordered_map<std::string, StakeEntry> stakes;
    std::unordered_map<std::string, ValidatorPoolStats> validatorPools;
    double totalStakedCoins;
    mutable std::mutex poolMutex;
    
    // LevelDB for PRODUCTION persistence
    std::unique_ptr<leveldb::DB> db;
    leveldb::Options dbOptions;
    leveldb::ReadOptions readOptions;
    leveldb::WriteOptions writeOptions;
    std::string dbPath;
    
    std::string generateStakeId(const std::string& owner, const std::string& validator);
    
    // LevelDB helper methods
    std::string serializeStakeEntry(const StakeEntry& entry) const;
    StakeEntry deserializeStakeEntry(const std::string& data) const;
    std::string serializeValidatorStats(const ValidatorPoolStats& stats) const;
    ValidatorPoolStats deserializeValidatorStats(const std::string& data) const;

public:
    StakingPool();
    ~StakingPool();
    
    // Core staking operations
    std::string addStake(const std::string& owner, const std::string& validator,
                        double amount, uint32_t lockDays,
                        const std::string& stakeTxHash,
                        const std::string& previousTxHash,
                        const std::vector<std::string>& inputTxHashes);
    bool removeStake(const std::string& stakeId);
    bool isStakeMature(const std::string& stakeId) const;
    bool canUnstake(const std::string& stakeId) const;
    
    // Stake queries
    StakeEntry getStake(const std::string& stakeId) const;
    std::vector<StakeEntry> getStakesForAddress(const std::string& address) const;
    std::vector<StakeEntry> getStakesForValidator(const std::string& validator) const;
    bool stakeExists(const std::string& stakeId) const;
    
    // Validator pool management
    void initializeValidatorPool(const std::string& validatorAddress, double ownStake);
    void updateValidatorPool(const std::string& validatorAddress, double deltaStake, bool isAdd);
    void recordBlockProduced(const std::string& validatorAddress);
    void recordMissedBlock(const std::string& validatorAddress);
    ValidatorPoolStats getValidatorPoolStats(const std::string& validatorAddress) const;
    
    // Rewards
    void addReward(const std::string& stakeId, double rewardAmount);
    double getAccumulatedRewards(const std::string& stakeId) const;
    
    // Global statistics
    double getTotalStaked() const;
    double getStakedByAddress(const std::string& address) const;
    double getStakedWithValidator(const std::string& validator) const;
    size_t getTotalStakeCount() const;
    size_t getValidatorCount() const;
    
    // Persistence
    bool saveToDatabase();
    bool loadFromDatabase();
    
    // Constants
    static constexpr uint32_t MIN_LOCK_PERIOD_DAYS = 7;
    static constexpr uint32_t MAX_LOCK_PERIOD_DAYS = 365;
    static constexpr double MIN_STAKE_AMOUNT = 10.0;
};
