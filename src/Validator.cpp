#include "../include/Validator.h"
#include "../include/HashUtils.h"
#include <cmath>

const double BETA = 0.5; // Time weight factor

Validator::Validator(const std::string& addressIn, double stakeAmountIn, uint32_t stakingDaysIn)
    : address(addressIn), stakeAmount(stakeAmountIn), stakingDays(stakingDaysIn), isActive(true) {
    stakeStartTime = std::time(nullptr);
    stakeEndTime = stakeStartTime + (stakingDays * 24 * 60 * 60);
    calculateWeightedStake();
}

void Validator::calculateWeightedStake() {
    // Calculate weighted stake using the formula: stake * time^beta
    weightedStake = stakeAmount * std::pow(stakingDays, BETA);
}

std::string Validator::signBlock(const std::string& blockHash, const std::string& privateKey) const {
    // In a real implementation, we would sign the block hash with the private key
    // For simplicity, we'll just return a dummy signature
    return "SIGNED_" + blockHash + "_WITH_" + privateKey;
}

void Validator::setStakeAmount(double amount) {
    stakeAmount = amount;
    calculateWeightedStake();
}

void Validator::setStakingDays(uint32_t days) {
    stakingDays = days;
    stakeEndTime = stakeStartTime + (stakingDays * 24 * 60 * 60);
    calculateWeightedStake();
}
