#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <ctime>

struct PriceSubmission {
    std::string oracleId;
    double price;
    std::time_t timestamp;
    std::string signature;
};

struct PriceData {
    double price;
    std::time_t timestamp;
    std::string popHash;
    bool hasDispute;
    std::vector<std::string> oracleIds;
};

class Oracle {
private:
    std::string id;
    std::string publicKey;
    double reputation;

public:
    Oracle(const std::string& idIn, const std::string& publicKeyIn);
    
    // Submit a price
    PriceSubmission submitPrice(double price, const std::string& privateKey);
    
    // Getters
    std::string getId() const { return id; }
    std::string getPublicKey() const { return publicKey; }
    double getReputation() const { return reputation; }
    
    // Setters
    void updateReputation(double newReputation) { reputation = newReputation; }
};

class ProofOfPrice {
private:
    std::vector<Oracle> oracles;
    std::unordered_map<uint32_t, std::vector<PriceSubmission>> submissions;
    std::unordered_map<uint32_t, PriceData> priceData;
    uint32_t currentRound;
    uint32_t requiredSubmissions;
    double maxTimeDelay;

public:
    // Constructor
    ProofOfPrice(uint32_t requiredSubmissionsIn = 5, double maxTimeDelayIn = 3600);
    
    // Add an oracle
    void addOracle(const Oracle& oracle);
    
    // Submit a price
    bool submitPrice(const PriceSubmission& submission);
    
    // Aggregate prices into a PoP
    PriceData aggregatePoP();
    
    // Detect outliers
    std::vector<std::string> detectOutliers(const std::vector<double>& prices, double median);
    
    // Calculate median price
    double calculateMedian(const std::vector<double>& prices);
    
    // Getters
    PriceData getCurrentPrice() const;
    uint32_t getCurrentRound() const { return currentRound; }
};
