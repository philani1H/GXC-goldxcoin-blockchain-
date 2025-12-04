#pragma once

#include "Validator.h"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <algorithm>
#include <random>

class ValidatorRegistry {
private:
    std::map<std::string, std::shared_ptr<Validator>> validators;
    std::mutex registryMutex;

    // Configuration
    double minStakeRequired;
    uint32_t maxValidators;

    // Statistics
    uint64_t totalStaked;
    uint32_t activeValidatorCount;

public:
    ValidatorRegistry(double minStake = 100.0, uint32_t maxVals = 100);

    // Registration
    bool registerValidator(const std::string& address, double stakeAmount, uint32_t stakingDays, const std::string& publicKey);
    bool unregisterValidator(const std::string& address);

    // Validator management
    bool updateValidatorStake(const std::string& address, double additionalStake);
    bool extendValidatorStakingPeriod(const std::string& address, uint32_t additionalDays);
    bool deactivateValidator(const std::string& address);
    bool reactivateValidator(const std::string& address);

    // Validator selection
    std::string selectValidator(const std::string& seed);
    std::vector<std::string> selectValidators(size_t count, const std::string& seed);

    // Queries
    std::shared_ptr<Validator> getValidator(const std::string& address) const;
    std::vector<std::shared_ptr<Validator>> getAllValidators() const;
    std::vector<std::shared_ptr<Validator>> getActiveValidators() const;
    bool isValidatorRegistered(const std::string& address) const;

    // Statistics
    double getTotalWeightedStake() const;
    uint64_t getTotalStaked() const { return totalStaked; }
    uint32_t getActiveValidatorCount() const { return activeValidatorCount; }
    uint32_t getTotalValidatorCount() const { return validators.size(); }

    // Rewards distribution
    void distributeRewards(double totalReward);
    void distributeBlockReward(const std::string& validatorAddress, double reward);

    // Slashing
    bool slashValidator(const std::string& address, double amount, const std::string& reason);
    bool unslashValidator(const std::string& address);

    // Validation
    bool validateRegistry() const;
    std::vector<std::string> getInvalidValidators() const;

    // Serialization
    std::string serialize() const;
    bool deserialize(const std::string& data);

    // Persistence
    bool loadFromDatabase();
    bool saveToDatabase();

    // Clear all validators (for testing)
    void clear();
};
