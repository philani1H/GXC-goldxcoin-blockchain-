#include "../include/SelectValidator.h"
#include <algorithm>
#include <cmath>

// Implementation of the validator selection logic based on the white paper
// This file corresponds to the attached select-validator-DbrdTiBTpiLLQJ160Mr8pAOeuox9MJ.tsx file

// The main logic is already implemented in the ValidatorSelector class in SelectValidator.h
// This file would contain any additional implementation details if needed

// For example, we could add more sophisticated selection algorithms:

// Select validators for a committee (for threshold signatures)
std::vector<Validator> selectValidatorCommittee(const ValidatorSelector& selector, size_t committeeSize) {
    const auto& allValidators = selector.getValidators();
    
    // Ensure we don't request more validators than available
    committeeSize = std::min(committeeSize, allValidators.size());
    
    // Create a copy of validators that we can modify
    std::vector<Validator> activeValidators;
    for (const auto& validator : allValidators) {
        if (validator.getIsActive()) {
            activeValidators.push_back(validator);
        }
    }
    
    // Ensure we have enough active validators
    if (activeValidators.size() < committeeSize) {
        return activeValidators; // Return all active validators if not enough
    }
    
    // Select committee members based on weighted stake
    std::vector<Validator> committee;
    
    // Calculate total weighted stake
    double totalWeightedStake = 0.0;
    for (const auto& validator : activeValidators) {
        totalWeightedStake += validator.getWeightedStake();
    }
    
    // Random selection without replacement
    std::random_device rd;
    std::mt19937 gen(rd());
    
    while (committee.size() < committeeSize && !activeValidators.empty()) {
        // Recalculate total weighted stake for remaining validators
        double remainingStake = 0.0;
        for (const auto& validator : activeValidators) {
            remainingStake += validator.getWeightedStake();
        }
        
        // Select a validator proportional to weighted stake
        std::uniform_real_distribution<> dis(0, remainingStake);
        double threshold = dis(gen);
        
        double cumulativeWeight = 0.0;
        size_t selectedIndex = 0;
        
        for (size_t i = 0; i < activeValidators.size(); i++) {
            cumulativeWeight += activeValidators[i].getWeightedStake();
            if (cumulativeWeight >= threshold) {
                selectedIndex = i;
                break;
            }
        }
        
        // Add selected validator to committee
        committee.push_back(activeValidators[selectedIndex]);
        
        // Remove selected validator from active validators
        activeValidators.erase(activeValidators.begin() + selectedIndex);
    }
    
    return committee;
}
