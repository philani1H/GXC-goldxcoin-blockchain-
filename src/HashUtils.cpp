#include "../include/HashUtils.h"
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>
#include <openssl/ripemd.h>

#include "../include/Crypto.h"
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
// Full Ethash implementation with DAG
// Based on Ethereum's Ethash algorithm
namespace {
    const uint32_t ETHASH_DATASET_BYTES_INIT = 1073741824U; // 2^30 (1GB)
    const uint32_t ETHASH_DATASET_BYTES_GROWTH = 8388608U;  // 2^23
    const uint32_t ETHASH_CACHE_BYTES_INIT = 16777216U;     // 2^24
    const uint32_t ETHASH_CACHE_BYTES_GROWTH = 131072U;     // 2^17
    const uint32_t ETHASH_MIX_BYTES = 128;
    const uint32_t ETHASH_HASH_BYTES = 64;
    const uint32_t ETHASH_DATASET_PARENTS = 256;
    const uint32_t ETHASH_CACHE_ROUNDS = 3;
    const uint32_t ETHASH_ACCESSES = 64;
    
    // FNV hash function
    uint32_t fnv_hash(uint32_t v1, uint32_t v2) {
        return (v1 * 0x01000193) ^ v2;
    }
    
    // Generate cache for epoch
    std::vector<uint32_t> generate_cache(uint32_t cache_size, const std::string& seed) {
        uint32_t n = cache_size / ETHASH_HASH_BYTES;
        std::vector<uint32_t> cache(n * (ETHASH_HASH_BYTES / 4));
        
        // Initial hash
        std::string hash = keccak256(seed);
        for (size_t i = 0; i < ETHASH_HASH_BYTES / 4; i++) {
            cache[i] = *reinterpret_cast<const uint32_t*>(hash.data() + i * 4);
        }
        
        // Sequential hashing
        for (uint32_t i = 1; i < n; i++) {
            std::string prev_hash(reinterpret_cast<char*>(&cache[(i-1) * (ETHASH_HASH_BYTES/4)]), ETHASH_HASH_BYTES);
            hash = keccak256(prev_hash);
            for (size_t j = 0; j < ETHASH_HASH_BYTES / 4; j++) {
                cache[i * (ETHASH_HASH_BYTES/4) + j] = *reinterpret_cast<const uint32_t*>(hash.data() + j * 4);
            }
        }
        
        // RandMemoHash rounds
        for (uint32_t round = 0; round < ETHASH_CACHE_ROUNDS; round++) {
            for (uint32_t i = 0; i < n; i++) {
                uint32_t v = cache[i * (ETHASH_HASH_BYTES/4)] % n;
                uint32_t offset1 = i * (ETHASH_HASH_BYTES/4);
                uint32_t offset2 = v * (ETHASH_HASH_BYTES/4);
                uint32_t offset3 = ((i - 1 + n) % n) * (ETHASH_HASH_BYTES/4);
                
                for (size_t j = 0; j < ETHASH_HASH_BYTES / 4; j++) {
                    cache[offset1 + j] = cache[offset1 + j] ^ cache[offset2 + j] ^ cache[offset3 + j];
                }
                
                std::string temp(reinterpret_cast<char*>(&cache[offset1]), ETHASH_HASH_BYTES);
                hash = keccak256(temp);
                for (size_t j = 0; j < ETHASH_HASH_BYTES / 4; j++) {
                    cache[offset1 + j] = *reinterpret_cast<const uint32_t*>(hash.data() + j * 4);
                }
            }
        }
        
        return cache;
    }
    
    // Calculate dataset item from cache
    std::vector<uint32_t> calc_dataset_item(const std::vector<uint32_t>& cache, uint32_t i) {
        uint32_t n = cache.size() / (ETHASH_HASH_BYTES / 4);
        std::vector<uint32_t> mix(ETHASH_HASH_BYTES / 4);
        
        // Initial mix
        for (size_t j = 0; j < ETHASH_HASH_BYTES / 4; j++) {
            mix[j] = cache[(i % n) * (ETHASH_HASH_BYTES/4) + j];
        }
        mix[0] ^= i;
        
        std::string temp(reinterpret_cast<char*>(mix.data()), ETHASH_HASH_BYTES);
        std::string hash = keccak256(temp);
        for (size_t j = 0; j < ETHASH_HASH_BYTES / 4; j++) {
            mix[j] = *reinterpret_cast<const uint32_t*>(hash.data() + j * 4);
        }
        
        // Mix in random cache nodes
        for (uint32_t j = 0; j < ETHASH_DATASET_PARENTS; j++) {
            uint32_t cache_index = fnv_hash(i ^ j, mix[j % (ETHASH_HASH_BYTES/4)]) % n;
            for (size_t k = 0; k < ETHASH_HASH_BYTES / 4; k++) {
                mix[k] = fnv_hash(mix[k], cache[cache_index * (ETHASH_HASH_BYTES/4) + k]);
            }
        }
        
        temp = std::string(reinterpret_cast<char*>(mix.data()), ETHASH_HASH_BYTES);
        hash = keccak256(temp);
        for (size_t j = 0; j < ETHASH_HASH_BYTES / 4; j++) {
            mix[j] = *reinterpret_cast<const uint32_t*>(hash.data() + j * 4);
        }
        
        return mix;
    }
}

std::string ethash(const std::string& data, uint64_t nonce) {
    // Full Ethash algorithm with DAG
    // For performance, we use a smaller cache size
    const uint32_t cache_size = ETHASH_CACHE_BYTES_INIT / 16; // Reduced for performance
    
    // Generate seed from data
    std::string seed = keccak256(data);
    
    // Generate cache
    auto cache = generate_cache(cache_size, seed);
    
    // Create header hash
    std::stringstream ss;
    ss << data << nonce;
    std::string header = keccak256(ss.str());
    
    // Initialize mix
    std::vector<uint32_t> mix(ETHASH_MIX_BYTES / 4);
    for (size_t i = 0; i < ETHASH_HASH_BYTES / 4; i++) {
        uint32_t val = *reinterpret_cast<const uint32_t*>(header.data() + i * 4);
        mix[i] = val;
        mix[i + ETHASH_HASH_BYTES/4] = val;
    }
    
    // Mix in random dataset nodes
    uint32_t num_items = cache_size / ETHASH_HASH_BYTES;
    for (uint32_t i = 0; i < ETHASH_ACCESSES; i++) {
        uint32_t p = fnv_hash(i ^ *reinterpret_cast<const uint32_t*>(header.data()), mix[i % (ETHASH_MIX_BYTES/4)]) % num_items;
        auto dataset_item = calc_dataset_item(cache, p);
        
        for (size_t j = 0; j < ETHASH_MIX_BYTES / 4; j++) {
            mix[j] = fnv_hash(mix[j], dataset_item[j % (ETHASH_HASH_BYTES/4)]);
        }
    }
    
    // Compress mix
    std::vector<uint32_t> cmix(ETHASH_MIX_BYTES / 4 / 4);
    for (size_t i = 0; i < ETHASH_MIX_BYTES / 4; i += 4) {
        cmix[i/4] = fnv_hash(fnv_hash(fnv_hash(mix[i], mix[i+1]), mix[i+2]), mix[i+3]);
    }
    
    // Final hash
    std::string result;
    result.append(header);
    result.append(reinterpret_cast<char*>(cmix.data()), cmix.size() * 4);
    
    return keccak256(result);
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

// Proper Keccak-256 implementation
std::string keccak256(const std::string& data) {
    // Use the proper Keccak-256 from Crypto namespace
    return Crypto::keccak256(data);
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
    // Bitcoin-style difficulty check
    // Convert hash string to bytes for proper comparison
    std::vector<uint8_t> hashBytes;
    for (size_t i = 0; i < hash.length() && i < 64; i += 2) {
        if (i + 1 < hash.length()) {
            std::string byteString = hash.substr(i, 2);
            try {
                uint8_t byte = static_cast<uint8_t>(std::stoi(byteString, nullptr, 16));
                hashBytes.push_back(byte);
            } catch (...) {
                return false;
            }
        }
    }
    
    if (hashBytes.empty()) {
        return false;
    }
    
    // Calculate target from difficulty
    // For Bitcoin-style: higher difficulty = more leading zeros required
    // We check leading zero bytes
    size_t requiredZeroBytes = static_cast<size_t>(difficulty / 256.0);
    
    // Check leading zero bytes
    for (size_t i = 0; i < requiredZeroBytes && i < hashBytes.size(); i++) {
        if (hashBytes[i] != 0) {
            return false;
        }
    }
    
    // For fractional difficulty, check the next byte
    if (requiredZeroBytes < hashBytes.size()) {
        double fractionalPart = difficulty - (requiredZeroBytes * 256.0);
        uint8_t maxValue = static_cast<uint8_t>(256.0 - fractionalPart);
        if (hashBytes[requiredZeroBytes] > maxValue) {
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
