#include "../include/ProofOfPrice.h"
#include "../include/HashUtils.h"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <sstream>

Oracle::Oracle(const std::string& idIn, const std::string& publicKeyIn)
    : id(idIn), publicKey(publicKeyIn), reputation(1.0) {
}

PriceSubmission Oracle::submitPrice(double price, const std::string& privateKey) {
    PriceSubmission submission;
    submission.oracleId = id;
    submission.price = price;
    submission.timestamp = std::time(nullptr);
    
    // In a real implementation, we would sign the price and timestamp with the private key
    std::stringstream ss;
    ss << price << submission.timestamp;
    submission.signature = "SIGNED_" + ss.str() + "_WITH_" + privateKey;
    
    return submission;
}

ProofOfPrice::ProofOfPrice(uint32_t requiredSubmissionsIn, double maxTimeDelayIn)
    : requiredSubmissions(requiredSubmissionsIn), maxTimeDelay(maxTimeDelayIn), currentRound(0) {
}

void ProofOfPrice::addOracle(const Oracle& oracle) {
    oracles.push_back(oracle);
}

bool ProofOfPrice::submitPrice(const PriceSubmission& submission) {
    // Verify the oracle is authorized
    bool isAuthorized = false;
    for (const auto& oracle : oracles) {
        if (oracle.getId() == submission.oracleId) {
            isAuthorized = true;
            break;
        }
    }
    
    if (!isAuthorized) {
        return false;
    }
    
    // Verify the signature
    // In a real implementation, we would verify the signature using the oracle's public key
    
    // Check if the timestamp is recent enough
    std::time_t now = std::time(nullptr);
    if (difftime(now, submission.timestamp) > maxTimeDelay) {
        return false;
    }
    
    // Store the submission
    submissions[currentRound].push_back(submission);
    
    // Check if we have enough submissions to aggregate
    if (submissions[currentRound].size() >= requiredSubmissions) {
        aggregatePoP();
    }
    
    return true;
}

PriceData ProofOfPrice::aggregatePoP() {
    // Get all submissions for the current round
    const auto& roundSubmissions = submissions[currentRound];
    
    // Extract prices
    std::vector<double> prices;
    for (const auto& submission : roundSubmissions) {
        prices.push_back(submission.price);
    }
    
    // Calculate median price
    double medianPrice = calculateMedian(prices);
    
    // Detect outliers
    std::vector<std::string> outliers = detectOutliers(prices, medianPrice);
    bool hasDispute = !outliers.empty();
    
    // Get oracle IDs
    std::vector<std::string> oracleIds;
    for (const auto& submission : roundSubmissions) {
        oracleIds.push_back(submission.oracleId);
    }
    
    // Create PoP hash
    std::stringstream ss;
    ss << medianPrice << std::time(nullptr);
    for (const auto& id : oracleIds) {
        ss << id;
    }
    std::string popHash = keccak256(ss.str());
    
    // Create price data
    PriceData data;
    data.price = medianPrice;
    data.timestamp = std::time(nullptr);
    data.popHash = popHash;
    data.hasDispute = hasDispute;
    data.oracleIds = oracleIds;
    
    // Store the price data
    priceData[currentRound] = data;
    
    // Move to next round
    currentRound++;
    
    return data;
}

std::vector<std::string> ProofOfPrice::detectOutliers(const std::vector<double>& prices, double median) {
    std::vector<std::string> outliers;
    
    // Calculate mean and standard deviation
    double sum = std::accumulate(prices.begin(), prices.end(), 0.0);
    double mean = sum / prices.size();
    
    double sq_sum = std::inner_product(prices.begin(), prices.end(), prices.begin(), 0.0);
    double stdev = std::sqrt(sq_sum / prices.size() - mean * mean);
    
    // Identify outliers (prices outside 2 standard deviations from the mean)
    for (size_t i = 0; i < prices.size(); i++) {
        if (std::abs(prices[i] - mean) > 2 * stdev) {
            outliers.push_back(submissions[currentRound][i].oracleId);
        }
    }
    
    return outliers;
}

double ProofOfPrice::calculateMedian(const std::vector<double>& prices) {
    if (prices.empty()) {
        return 0.0;
    }
    
    std::vector<double> sortedPrices = prices;
    std::sort(sortedPrices.begin(), sortedPrices.end());
    
    size_t size = sortedPrices.size();
    if (size % 2 == 0) {
        return (sortedPrices[size / 2 - 1] + sortedPrices[size / 2]) / 2.0;
    } else {
        return sortedPrices[size / 2];
    }
}

PriceData ProofOfPrice::getCurrentPrice() const {
    if (currentRound == 0) {
        // No price data yet
        PriceData emptyData;
        emptyData.price = 0.0;
        emptyData.timestamp = 0;
        emptyData.popHash = "";
        emptyData.hasDispute = false;
        return emptyData;
    }
    
    // Return the most recent price data
    auto it = priceData.find(currentRound - 1);
    if (it != priceData.end()) {
        return it->second;
    }
    
    // Fallback (should never reach here)
    PriceData emptyData;
    emptyData.price = 0.0;
    emptyData.timestamp = 0;
    emptyData.popHash = "";
    emptyData.hasDispute = false;
    return emptyData;
}
