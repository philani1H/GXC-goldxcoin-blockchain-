#pragma once

#include <string>
#include <ctime>

class Validator {
private:
    std::string address;
    double stakeAmount;
    uint32_t stakingDays;
    std::time_t stakeStartTime;
    std::time_t stakeEndTime;
    double weightedStake;
    bool isActive;

public:
    // Constructor
    Validator(const std::string& addressIn, double stakeAmountIn, uint32_t stakingDaysIn);
    
    // Calculate weighted stake
    void calculateWeightedStake();
    
    // Validate a block
    std::string signBlock(const std::string& blockHash, const std::string& privateKey) const;
    
    // Getters
    std::string getAddress() const { return address; }
    double getStakeAmount() const { return stakeAmount; }
    uint32_t getStakingDays() const { return stakingDays; }
    double getWeightedStake() const { return weightedStake; }
    bool getIsActive() const { return isActive; }
    
    // Setters
    void setStakeAmount(double amount);
    void setStakingDays(uint32_t days);
    void setIsActive(bool active) { isActive = active; }
};
