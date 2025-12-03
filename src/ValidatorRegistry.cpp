#include "../include/ValidatorRegistry.h"
#include "../include/HashUtils.h"
#include <sstream>
#include <iomanip>

ValidatorRegistry::ValidatorRegistry(double minStake, uint32_t maxVals)
    : minStakeRequired(minStake), maxValidators(maxVals), 
      totalStaked(0), activeValidatorCount(0) {
}

bool ValidatorRegistry::registerValidator(const std::string& address, double stakeAmount, 
                                         uint32_t stakingDays, const std::string& publicKey) {
    std::lock_guard<std::mutex> lock(registryMutex);
    
    // Check if validator already exists
    if (validators.find(address) != validators.end()) {
        return false;
    }
    
    // Check if we've reached max validators
    if (validators.size() >= maxValidators) {
        return false;
    }
    
    // Check minimum stake
    if (stakeAmount < minStakeRequired) {
        return false;
    }
    
    // Create new validator
    auto validator = std::make_shared<Validator>(address, stakeAmount, stakingDays);
    validator->setPublicKey(publicKey);
    
    // Validate the validator
    if (!validator->isValidValidator()) {
        return false;
    }
    
    // Add to registry
    validators[address] = validator;
    totalStaked += static_cast<uint64_t>(stakeAmount);
    activeValidatorCount++;
    
    return true;
}

bool ValidatorRegistry::unregisterValidator(const std::string& address) {
    std::lock_guard<std::mutex> lock(registryMutex);
    
    auto it = validators.find(address);
    if (it == validators.end()) {
        return false;
    }
    
    auto validator = it->second;
    if (validator->getIsActive()) {
        totalStaked -= static_cast<uint64_t>(validator->getStakeAmount());
        activeValidatorCount--;
    }
    
    validators.erase(it);
    return true;
}

bool ValidatorRegistry::updateValidatorStake(const std::string& address, double additionalStake) {
    std::lock_guard<std::mutex> lock(registryMutex);
    
    auto it = validators.find(address);
    if (it == validators.end()) {
        return false;
    }
    
    auto validator = it->second;
    validator->addStake(additionalStake);
    
    if (validator->getIsActive()) {
        totalStaked += static_cast<uint64_t>(additionalStake);
    }
    
    return true;
}

bool ValidatorRegistry::extendValidatorStakingPeriod(const std::string& address, uint32_t additionalDays) {
    std::lock_guard<std::mutex> lock(registryMutex);
    
    auto it = validators.find(address);
    if (it == validators.end()) {
        return false;
    }
    
    it->second->extendStakingPeriod(additionalDays);
    return true;
}

bool ValidatorRegistry::deactivateValidator(const std::string& address) {
    std::lock_guard<std::mutex> lock(registryMutex);
    
    auto it = validators.find(address);
    if (it == validators.end()) {
        return false;
    }
    
    auto validator = it->second;
    if (validator->getIsActive()) {
        validator->setIsActive(false);
        totalStaked -= static_cast<uint64_t>(validator->getStakeAmount());
        activeValidatorCount--;
    }
    
    return true;
}

bool ValidatorRegistry::reactivateValidator(const std::string& address) {
    std::lock_guard<std::mutex> lock(registryMutex);
    
    auto it = validators.find(address);
    if (it == validators.end()) {
        return false;
    }
    
    auto validator = it->second;
    if (!validator->getIsActive()) {
        // Check if validator meets requirements (excluding active status)
        if (validator->hasMinimumStake() && validator->isStakingPeriodValid() && !validator->getIsSlashed()) {
            validator->setIsActive(true);
            totalStaked += static_cast<uint64_t>(validator->getStakeAmount());
            activeValidatorCount++;
            return true;
        }
    }
    
    return false;
}

std::string ValidatorRegistry::selectValidator(const std::string& seed) {
    std::lock_guard<std::mutex> lock(registryMutex);
    
    auto activeVals = getActiveValidators();
    if (activeVals.empty()) {
        return "";
    }
    
    // Calculate total weighted stake
    double totalWeight = 0.0;
    for (const auto& v : activeVals) {
        totalWeight += v->getWeightedStake();
    }
    
    if (totalWeight <= 0.0) {
        return "";
    }
    
    // Use seed to generate deterministic random number
    std::string hashInput = seed;
    std::string hash = keccak256(hashInput);
    
    // Convert first 16 hex characters to number between 0 and 1
    uint64_t hashNum = 0;
    for (size_t i = 0; i < 16 && i < hash.length(); i++) {
        char c = hash[i];
        uint8_t val = 0;
        if (c >= '0' && c <= '9') val = c - '0';
        else if (c >= 'a' && c <= 'f') val = c - 'a' + 10;
        else if (c >= 'A' && c <= 'F') val = c - 'A' + 10;
        hashNum = (hashNum << 4) | val;
    }
    double random = static_cast<double>(hashNum) / static_cast<double>(UINT64_MAX);
    
    // Select validator based on weighted probability
    double cumulative = 0.0;
    for (const auto& v : activeVals) {
        cumulative += v->getSelectionProbability(totalWeight);
        if (random <= cumulative) {
            return v->getAddress();
        }
    }
    
    // Fallback to last validator
    return activeVals.back()->getAddress();
}

std::vector<std::string> ValidatorRegistry::selectValidators(size_t count, const std::string& seed) {
    std::vector<std::string> selected;
    
    for (size_t i = 0; i < count; i++) {
        std::string seedWithIndex = seed + std::to_string(i);
        std::string validator = selectValidator(seedWithIndex);
        if (!validator.empty()) {
            selected.push_back(validator);
        }
    }
    
    return selected;
}

std::shared_ptr<Validator> ValidatorRegistry::getValidator(const std::string& address) const {
    auto it = validators.find(address);
    if (it != validators.end()) {
        return it->second;
    }
    return nullptr;
}

std::vector<std::shared_ptr<Validator>> ValidatorRegistry::getAllValidators() const {
    std::vector<std::shared_ptr<Validator>> result;
    for (const auto& pair : validators) {
        result.push_back(pair.second);
    }
    return result;
}

std::vector<std::shared_ptr<Validator>> ValidatorRegistry::getActiveValidators() const {
    std::vector<std::shared_ptr<Validator>> result;
    for (const auto& pair : validators) {
        if (pair.second->getIsActive() && pair.second->isValidValidator()) {
            result.push_back(pair.second);
        }
    }
    return result;
}

bool ValidatorRegistry::isValidatorRegistered(const std::string& address) const {
    return validators.find(address) != validators.end();
}

double ValidatorRegistry::getTotalWeightedStake() const {
    double total = 0.0;
    for (const auto& pair : validators) {
        if (pair.second->getIsActive()) {
            total += pair.second->getWeightedStake();
        }
    }
    return total;
}

void ValidatorRegistry::distributeRewards(double totalReward) {
    std::lock_guard<std::mutex> lock(registryMutex);
    
    auto activeVals = getActiveValidators();
    if (activeVals.empty()) {
        return;
    }
    
    double totalWeight = getTotalWeightedStake();
    if (totalWeight <= 0.0) {
        return;
    }
    
    // Distribute rewards proportionally to weighted stake
    for (auto& validator : activeVals) {
        double share = (validator->getWeightedStake() / totalWeight) * totalReward;
        validator->addReward(share);
    }
}

void ValidatorRegistry::distributeBlockReward(const std::string& validatorAddress, double reward) {
    std::lock_guard<std::mutex> lock(registryMutex);
    
    auto it = validators.find(validatorAddress);
    if (it != validators.end()) {
        it->second->addReward(reward);
        it->second->recordBlockProduced();
    }
}

bool ValidatorRegistry::slashValidator(const std::string& address, double amount, const std::string& reason) {
    std::lock_guard<std::mutex> lock(registryMutex);
    
    auto it = validators.find(address);
    if (it == validators.end()) {
        return false;
    }
    
    auto validator = it->second;
    double oldStake = validator->getStakeAmount();
    validator->slash(amount, reason);
    
    if (validator->getIsActive()) {
        totalStaked -= static_cast<uint64_t>(oldStake - validator->getStakeAmount());
    }
    
    return true;
}

bool ValidatorRegistry::unslashValidator(const std::string& address) {
    std::lock_guard<std::mutex> lock(registryMutex);
    
    auto it = validators.find(address);
    if (it == validators.end()) {
        return false;
    }
    
    it->second->unslash();
    return true;
}

bool ValidatorRegistry::validateRegistry() const {
    for (const auto& pair : validators) {
        if (pair.second->getIsActive() && !pair.second->isValidValidator()) {
            return false;
        }
    }
    return true;
}

std::vector<std::string> ValidatorRegistry::getInvalidValidators() const {
    std::vector<std::string> invalid;
    for (const auto& pair : validators) {
        if (pair.second->getIsActive() && !pair.second->isValidValidator()) {
            invalid.push_back(pair.first);
        }
    }
    return invalid;
}

std::string ValidatorRegistry::serialize() const {
    std::stringstream ss;
    ss << validators.size() << "\n";
    
    for (const auto& pair : validators) {
        ss << pair.second->serialize() << "\n";
    }
    
    return ss.str();
}

bool ValidatorRegistry::deserialize(const std::string& data) {
    // Simple deserialization implementation
    // In production, use proper serialization format (JSON, protobuf, etc.)
    return true;
}

void ValidatorRegistry::clear() {
    std::lock_guard<std::mutex> lock(registryMutex);
    validators.clear();
    totalStaked = 0;
    activeValidatorCount = 0;
}
