#pragma once

#include <cstdint>
#include <cstddef>

// Complete Ethash Algorithm Implementation
// Production-ready with full DAG generation
class EthashAlgorithm {
public:
    // Constructor - initializes for given block number
    explicit EthashAlgorithm(uint64_t block_number);
    
    // Destructor
    ~EthashAlgorithm();
    
    // Generate full dataset (for mining)
    // This is memory-intensive (1GB+) and takes time
    void generate_dataset();
    
    // Ethash hash - light client version (uses cache only)
    // Slower but uses less memory
    bool hash_light(const uint8_t header_hash[32], uint64_t nonce,
                   uint8_t result[32], uint8_t mix_hash[32]);
    
    // Ethash hash - full version (uses pre-generated dataset)
    // Faster but requires dataset generation first
    bool hash_full(const uint8_t header_hash[32], uint64_t nonce,
                  uint8_t result[32], uint8_t mix_hash[32]);
    
    // Verify Ethash solution
    bool verify(const uint8_t header_hash[32], uint64_t nonce,
               const uint8_t expected_result[32]);
    
    // Get epoch for block number
    static uint64_t get_epoch_for_block(uint64_t block_number);
    
    // Get cache size
    uint64_t get_cache_size() const { return cache_size; }
    
    // Get dataset size
    uint64_t get_dataset_size() const { return dataset_size; }
    
    // Check if dataset is generated
    bool has_dataset() const { return dataset != nullptr; }
    
private:
    uint64_t epoch;
    uint64_t cache_size;
    uint64_t dataset_size;
    uint8_t* cache;
    uint8_t* dataset;
    
    // Generate cache
    void generate_cache();
    
    // Calculate single dataset item
    void calc_dataset_item(uint32_t index, uint8_t* output);
    
    // Prevent copying
    EthashAlgorithm(const EthashAlgorithm&) = delete;
    EthashAlgorithm& operator=(const EthashAlgorithm&) = delete;
};
