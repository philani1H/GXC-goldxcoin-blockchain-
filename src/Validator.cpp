#include "../include/Validator.h"
#include "../include/HashUtils.h"
#include <cmath>

// Static constants initialization
const double Validator::BETA = 0.5;
const double Validator::MIN_STAKE = 100.0;
const uint32_t Validator::MIN_STAKING_DAYS = 14;
const uint32_t Validator::MAX_STAKING_DAYS = 365;

Validator::Validator() 
    : address(""), stakeAmount(0.0), stakingDays(0), stakeStartTime(0), 
      isActive(false), publicKey(""), signature(""),
      blocksProduced(0), missedBlocks(0), uptime(0.0),
      totalRewards(0.0), pendingRewards(0.0),
      isSlashed(false), slashedAmount(0.0), isPending(false) {
}

Validator::Validator(const std::string& addressIn, double stakeAmountIn, uint32_t stakingDaysIn)
    : address(addressIn), stakeAmount(stakeAmountIn), stakingDays(stakingDaysIn),
      isActive(true), publicKey(""), signature(""),
      blocksProduced(0), missedBlocks(0), uptime(1.0),
      totalRewards(0.0), pendingRewards(0.0),
      isSlashed(false), slashedAmount(0.0), isPending(false) {
    stakeStartTime = std::time(nullptr);
}

void Validator::stake(double amount, uint32_t days) {
    if (amount < MIN_STAKE) {
        return; // Don't stake if below minimum
    }
    
    stakeAmount = amount;
    stakingDays = days;
    stakeStartTime = std::time(nullptr);
    isActive = true;
}

void Validator::unstake() {
    isActive = false;
    stakeAmount = 0.0;
    stakingDays = 0;
}

void Validator::addStake(double amount) {
    stakeAmount += amount;
}

void Validator::removeStake(double amount) {
    if (amount > stakeAmount) {
        amount = stakeAmount; // Can't remove more than we have
    }
    stakeAmount -= amount;
    
    // If stake drops below minimum, deactivate validator
    if (stakeAmount < MIN_STAKE) {
        isActive = false;
    }
}

void Validator::extendStakingPeriod(uint32_t additionalDays) {
    stakingDays += additionalDays;
}

std::string Validator::signBlock(const std::string& blockHash, const std::string& privateKey) {
    // In a real implementation, this would use proper cryptographic signing
    // For now, we'll just return a simple hash-based signature
    return keccak256(blockHash + privateKey + address);
}

bool Validator::verifySignature(const std::string& blockHash, const std::string& signature) const {
    // In a real implementation, this would verify the cryptographic signature
    // For now, we'll just check if the signature is not empty
    return !signature.empty() && signature.length() > 32;
}

double Validator::getWeightedStake() const {
    if (!isActive) return 0.0;
    return stakeAmount * getTimeWeight();
}

double Validator::getTimeWeight() const {
    return std::pow(static_cast<double>(stakingDays) / 365.0, BETA);
}

double Validator::getSelectionProbability(double totalWeightedStake) const {
    if (totalWeightedStake <= 0.0) return 0.0;
    return getWeightedStake() / totalWeightedStake;
}

void Validator::recordBlockProduced() {
    blocksProduced++;
}

void Validator::recordMissedBlock() {
    missedBlocks++;
}

void Validator::updateUptime(double newUptime) {
    uptime = newUptime;
}

void Validator::addReward(double amount) {
    pendingRewards += amount;
}

void Validator::distributePendingRewards() {
    totalRewards += pendingRewards;
    pendingRewards = 0.0;
}

double Validator::calculateAPY() const {
    // Simple APY calculation based on rewards and stake
    if (stakeAmount <= 0.0) return 0.0;
    return (totalRewards / stakeAmount) * 100.0;
}

void Validator::slash(double amount, const std::string& reason) {
    isSlashed = true;
    slashedAmount += amount;
    stakeAmount = std::max(0.0, stakeAmount - amount);
    // Log the slashing reason (in a real implementation)
}

void Validator::unslash() {
    isSlashed = false;
    slashedAmount = 0.0;
}

bool Validator::isValidValidator() const {
    return hasMinimumStake() && isStakingPeriodValid() && isActive && !isSlashed;
}

bool Validator::hasMinimumStake(double minimumRequired) const {
    return stakeAmount >= minimumRequired;
}

bool Validator::isStakingPeriodValid() const {
    return stakingDays >= MIN_STAKING_DAYS && stakingDays <= MAX_STAKING_DAYS;
}

std::string Validator::serialize() const {
    // Simple serialization (in a real implementation, use proper serialization)
    return address + ":" + std::to_string(stakeAmount) + ":" + std::to_string(stakingDays);
}

Validator Validator::deserialize(const std::string& data) {
    // Simple deserialization (in a real implementation, use proper deserialization)
    Validator v;
    // Parse the data string and set fields
    return v;
}
