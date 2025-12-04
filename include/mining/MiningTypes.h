#pragma once

#include "../Logger.h"
#include "../Utils.h"
#include "../HashUtils.h"
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <cstdint>
#include <functional>
#include <sstream>
#include <unordered_map>

// Common mining types - shared across all miners

enum class MiningAlgorithm {
    SHA256,
    ETHASH,
    GXHASH
};

struct MiningJob {
    std::string jobId;
    std::string previousHash;
    std::string merkleRoot;
    std::time_t timestamp;
    std::string bits;
    double difficulty;
    uint32_t blockNumber;
    std::vector<std::string> transactions;
};

struct MiningStats {
    MiningAlgorithm algorithm;
    double hashRate;
    uint64_t totalHashes;
    uint32_t threadsActive;
    std::time_t uptime;
    uint64_t dagSize = 0;
    uint64_t epoch = 0;
    uint64_t traceabilityValidations = 0;
};

struct MiningSolution {
    std::string jobId;
    uint64_t nonce;
    uint64_t extraNonce;
    std::time_t timestamp;
    MiningAlgorithm algorithm;
    // GXHash specific
    std::string traceabilityHash;
    std::string crossValidationHash;
    bool traceabilityValid = false;
};

using SolutionCallback = std::function<void(const MiningSolution&)>;
