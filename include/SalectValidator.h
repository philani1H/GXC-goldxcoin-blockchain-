#pragma once

#include <vector>
#include <random>
#include "Validator.h"

class ValidatorSelector {
private:
    std::vector<Validator> validators;
    double beta; // Time weight factor

public:
    // Constructor
    ValidatorSelector(double betaIn = 0.5) : beta(betaIn) {}
    
    // Add a validator to the pool
    void addValidator(const Validator& validator) {
        validators.push_back(validator);
    }
    
    // Select a validator based on weighted stake
    Validator selectValidator() const {
        // Calculate total weighted stake
        double totalWeightedStake = 0.0;
        for (const auto& validator : validators) {
            if (validator.getIsActive()) {
                totalWeightedStake += validator.getWeightedStake();
            }
        }
        
        // Random selection proportional to weighted stake
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0, totalWeightedStake);
        double threshold = dis(gen);
        
        double cumulativeWeight = 0.0;
        for (const auto& validator : validators) {
            if (validator.getIsActive()) {
                cumulativeWeight += validator.getWeightedStake();
                if (cumulativeWeight >= threshold) {
                    return validator;
                }
            }
        }
        
        // Fallback (should never reach here)
        return validators.back();
    }
    
    // Get all validators
    const std::vector<Validator>& getValidators() const {
        return validators;
    }
};
