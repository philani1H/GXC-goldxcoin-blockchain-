#pragma once

#include <cstdint>
#include <cstddef>

// Forward declaration
struct Block;

// Argon2id context
class Argon2Context {
public:
    Argon2Context(uint32_t m_cost, uint32_t t_cost, uint32_t parallelism);
    ~Argon2Context();
    
    void initialize(const uint8_t* password, size_t pwdlen,
                   const uint8_t* salt, size_t saltlen);
    void fill_memory_blocks();
    void finalize(uint8_t* out, size_t outlen);
    
private:
    uint32_t m_cost;
    uint32_t t_cost;
    uint32_t parallelism;
    uint32_t segment_length;
    uint32_t lane_length;
    uint32_t memory_blocks;
    Block* memory;
    
    void fill_segment(uint32_t pass, uint32_t lane, uint32_t slice);
    uint32_t index_alpha(uint32_t pass, uint32_t slice, uint32_t lane,
                        uint32_t pseudo_rand, bool same_lane);
    
    // Prevent copying
    Argon2Context(const Argon2Context&) = delete;
    Argon2Context& operator=(const Argon2Context&) = delete;
};

// Main Argon2id hash function
void argon2id_hash(const uint8_t* password, size_t pwdlen,
                   const uint8_t* salt, size_t saltlen,
                   uint32_t t_cost, uint32_t m_cost, uint32_t parallelism,
                   uint8_t* out, size_t outlen);
