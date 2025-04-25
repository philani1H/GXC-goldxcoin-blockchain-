#include "../include/HashUtils.h"
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>
#include <openssl/ripemd.h>

// Simple SHA-256 implementation using OpenSSL
std::string sha256(const std::string& data) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, data.c_str(), data.size());
    SHA256_Final(hash, &sha256);
    
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    
    return ss.str();
}

// Double SHA-256 (used in Bitcoin)
std::string sha256d(const std::string& data) {
    return sha256(sha256(data));
}

// Simplified Ethash function (for demonstration)
std::string ethash(const std::string& data, uint64_t nonce) {
    std::stringstream ss;
    ss << data << nonce;
    return sha256(ss.str());
}

// RIPEMD-160 implementation using OpenSSL
std::string ripemd160(const std::string& data) {
    unsigned char hash[RIPEMD160_DIGEST_LENGTH];
    RIPEMD160_CTX ripemd160;
    RIPEMD160_Init(&ripemd160);
    RIPEMD160_Update(&ripemd160, data.c_str(), data.size());
    RIPEMD160_Final(hash, &ripemd160);
    
    std::stringstream ss;
    for (int i = 0; i < RIPEMD160_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    
    return ss.str();
}

// Simplified Keccak-256 (for demonstration)
std::string keccak256(const std::string& data) {
    // In a real implementation, we would use a proper Keccak-256 library
    // For simplicity, we'll just use SHA-256 as a placeholder
    return sha256(data);
}

// Calculate Merkle root from a list of transaction hashes
std::string calculateMerkleRoot(const std::vector<std::string>& txHashes) {
    if (txHashes.empty()) {
        return "";
    }
    
    if (txHashes.size() == 1) {
        return txHashes[0];
    }
    
    std::vector<std::string> newHashes;
    
    // Pair up hashes and hash them together
    for (size_t i = 0; i < txHashes.size(); i += 2) {
        if (i + 1 < txHashes.size()) {
            newHashes.push_back(sha256d(txHashes[i] + txHashes[i + 1]));
        } else {
            // If there's an odd number of hashes, duplicate the last one
            newHashes.push_back(sha256d(txHashes[i] + txHashes[i]));
        }
    }
    
    // Recursively calculate the Merkle root
    return calculateMerkleRoot(newHashes);
}

// Verify if a hash meets the target difficulty
bool meetsTarget(const std::string& hash, double difficulty) {
    // Convert difficulty to target number of leading zeros
    size_t targetZeros = static_cast<size_t>(difficulty);
    
    // Check if the hash has at least targetZeros leading zeros
    for (size_t i = 0; i < targetZeros; i++) {
        if (hash[i] != '0') {
            return false;
        }
    }
    
    return true;
}

// Convert a hash string to a numeric value for comparison
double hashToValue(const std::string& hash) {
    // Take the first 16 characters (64 bits) of the hash
    std::string prefix = hash.substr(0, 16);
    
    // Convert to a numeric value
    std::stringstream ss;
    ss << std::hex << prefix;
    uint64_t value;
    ss >> value;
    
    return static_cast<double>(value);
}
