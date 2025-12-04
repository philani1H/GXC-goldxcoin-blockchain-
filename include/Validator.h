#pragma once

#include <string>
#include <ctime>
#include <cmath>
#include <cstdint>

class Validator {
private:
    std::string address;
    double stakeAmount;
    uint32_t stakingDays;
    std::time_t stakeStartTime;
    bool isActive;
    std::string publicKey;
    std::string signature;
    
    // Performance metrics
    uint32_t blocksProduced;
    uint32_t missedBlocks;
    double uptime;
    
    // Rewards
    double totalRewards;
    double pendingRewards;
    
    // Slashing
    bool isSlashed;
    double slashedAmount;
    
public:
    // Constructor
    Validator();
    Validator(const std::string& addressIn, double stakeAmountIn, uint32_t stakingDaysIn);
    
    // Stake management
    void stake(double amount, uint32_t days);
    void unstake();
    void addStake(double amount);
    void removeStake(double amount);
    void extendStakingPeriod(uint32_t additionalDays);
    
    // Validator operations
    std::string signBlock(const std::string& blockHash, const std::string& privateKey);
    bool verifySignature(const std::string& blockHash, const std::string& signature) const;
    
    // Weight calculation (for validator selection)
    double getWeightedStake() const;
    double getTimeWeight() const;
    double getSelectionProbability(double totalWeightedStake) const;
    
    // Performance tracking
    void recordBlockProduced();
    void recordMissedBlock();
    void updateUptime(double newUptime);
    
    // Rewards
    void addReward(double amount);
    void distributePendingRewards();
    double calculateAPY() const;
    
    // Slashing
    void slash(double amount, const std::string& reason);
    void unslash();
    
    // Getters
    std::string getAddress() const { return address; }
    double getStakeAmount() const { return stakeAmount; }
    uint32_t getStakingDays() const { return stakingDays; }
    std::time_t getStakeStartTime() const { return stakeStartTime; }
    bool getIsActive() const { return isActive; }
    std::string getPublicKey() const { return publicKey; }
    uint32_t getBlocksProduced() const { return blocksProduced; }
    uint32_t getMissedBlocks() const { return missedBlocks; }
    double getUptime() const { return uptime; }
    double getTotalRewards() const { return totalRewards; }
    double getPendingRewards() const { return pendingRewards; }
    bool getIsSlashed() const { return isSlashed; }
    double getSlashedAmount() const { return slashedAmount; }
    
    // Setters
    void setAddress(const std::string& addr) { address = addr; }
    void setPublicKey(const std::string& pubKey) { publicKey = pubKey; }
    void setIsActive(bool active) { isActive = active; }
    
    // Validation
    bool isValidValidator() const;
    bool hasMinimumStake(double minimumRequired = 100.0) const;
    bool isStakingPeriodValid() const;
    
    // Serialization
    std::string serialize() const;
    static Validator deserialize(const std::string& data);
    
    // Constants
    static const double BETA; // Time weight factor (0.5)
    static const double MIN_STAKE; // Minimum stake amount (100 GXC)
    static const uint32_t MIN_STAKING_DAYS; // Minimum staking period (14 days)
    static const uint32_t MAX_STAKING_DAYS; // Maximum staking period (365 days)
};
