// Keccak-256 Implementation
// Based on FIPS 202 (SHA-3 Standard)
// Production-ready implementation

#include "../include/Keccak256.h"
#include <cstring>
#include <stdexcept>

// Keccak round constants
static const uint64_t KECCAK_ROUND_CONSTANTS[24] = {
    0x0000000000000001ULL, 0x0000000000008082ULL, 0x800000000000808aULL,
    0x8000000080008000ULL, 0x000000000000808bULL, 0x0000000080000001ULL,
    0x8000000080008081ULL, 0x8000000000008009ULL, 0x000000000000008aULL,
    0x0000000000000088ULL, 0x0000000080008009ULL, 0x000000008000000aULL,
    0x000000008000808bULL, 0x800000000000008bULL, 0x8000000000008089ULL,
    0x8000000000008003ULL, 0x8000000000008002ULL, 0x8000000000000080ULL,
    0x000000000000800aULL, 0x800000008000000aULL, 0x8000000080008081ULL,
    0x8000000000008080ULL, 0x0000000080000001ULL, 0x8000000080008008ULL
};

// Rotation offsets
static const int KECCAK_ROTATION_OFFSETS[24] = {
    1,  3,  6,  10, 15, 21, 28, 36, 45, 55, 2,  14,
    27, 41, 56, 8,  25, 43, 62, 18, 39, 61, 20, 44
};

// Pi lane indices
static const int KECCAK_PI_LANE[24] = {
    10, 7,  11, 17, 18, 3, 5,  16, 8,  21, 24, 4,
    15, 23, 19, 13, 12, 2, 20, 14, 22, 9,  6,  1
};

// Rotate left
static inline uint64_t rotl64(uint64_t x, int n) {
    return (x << n) | (x >> (64 - n));
}

// Keccak-f[1600] permutation
static void keccak_f1600(uint64_t state[25]) {
    uint64_t C[5], D[5], B[25];
    
    for (int round = 0; round < 24; round++) {
        // Theta step
        for (int x = 0; x < 5; x++) {
            C[x] = state[x] ^ state[x + 5] ^ state[x + 10] ^ state[x + 15] ^ state[x + 20];
        }
        
        for (int x = 0; x < 5; x++) {
            D[x] = C[(x + 4) % 5] ^ rotl64(C[(x + 1) % 5], 1);
        }
        
        for (int x = 0; x < 5; x++) {
            for (int y = 0; y < 5; y++) {
                state[x + 5 * y] ^= D[x];
            }
        }
        
        // Rho and Pi steps
        B[0] = state[0];
        for (int x = 0; x < 24; x++) {
            B[KECCAK_PI_LANE[x]] = rotl64(state[(x + 1) % 25], KECCAK_ROTATION_OFFSETS[x]);
        }
        
        // Chi step
        for (int y = 0; y < 5; y++) {
            for (int x = 0; x < 5; x++) {
                state[x + 5 * y] = B[x + 5 * y] ^ ((~B[(x + 1) % 5 + 5 * y]) & B[(x + 2) % 5 + 5 * y]);
            }
        }
        
        // Iota step
        state[0] ^= KECCAK_ROUND_CONSTANTS[round];
    }
}

// Keccak-256 hash function
std::string keccak256_hash(const uint8_t* data, size_t length) {
    // State: 1600 bits = 25 x 64-bit words
    uint64_t state[25] = {0};
    
    // Rate for Keccak-256: 1088 bits = 136 bytes
    const size_t rate = 136;
    const size_t rate_words = rate / 8;
    
    // Absorb phase
    size_t offset = 0;
    while (offset < length) {
        size_t block_size = (length - offset < rate) ? (length - offset) : rate;
        
        // XOR input into state
        for (size_t i = 0; i < block_size; i++) {
            size_t word_index = i / 8;
            size_t byte_index = i % 8;
            state[word_index] ^= ((uint64_t)data[offset + i]) << (8 * byte_index);
        }
        
        offset += block_size;
        
        if (block_size == rate) {
            keccak_f1600(state);
        }
    }
    
    // Padding: 10*1 pattern
    size_t last_byte = length % rate;
    size_t word_index = last_byte / 8;
    size_t byte_index = last_byte % 8;
    
    state[word_index] ^= ((uint64_t)0x01) << (8 * byte_index);
    state[rate_words - 1] ^= ((uint64_t)0x80) << 56;
    
    keccak_f1600(state);
    
    // Squeeze phase - extract 256 bits (32 bytes)
    uint8_t hash[32];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            hash[i * 8 + j] = (state[i] >> (8 * j)) & 0xFF;
        }
    }
    
    // Convert to hex string
    std::string result;
    result.reserve(64);
    const char hex_chars[] = "0123456789abcdef";
    
    for (int i = 0; i < 32; i++) {
        result += hex_chars[(hash[i] >> 4) & 0x0F];
        result += hex_chars[hash[i] & 0x0F];
    }
    
    return result;
}

// Convenience wrapper for std::string
std::string keccak256_hash(const std::string& data) {
    return keccak256_hash(reinterpret_cast<const uint8_t*>(data.data()), data.length());
}

// Keccak-256 for raw bytes (returns raw bytes)
void keccak256_raw(const uint8_t* data, size_t length, uint8_t output[32]) {
    uint64_t state[25] = {0};
    const size_t rate = 136;
    const size_t rate_words = rate / 8;
    
    // Absorb
    size_t offset = 0;
    while (offset < length) {
        size_t block_size = (length - offset < rate) ? (length - offset) : rate;
        
        for (size_t i = 0; i < block_size; i++) {
            size_t word_index = i / 8;
            size_t byte_index = i % 8;
            state[word_index] ^= ((uint64_t)data[offset + i]) << (8 * byte_index);
        }
        
        offset += block_size;
        
        if (block_size == rate) {
            keccak_f1600(state);
        }
    }
    
    // Padding
    size_t last_byte = length % rate;
    size_t word_index = last_byte / 8;
    size_t byte_index = last_byte % 8;
    
    state[word_index] ^= ((uint64_t)0x01) << (8 * byte_index);
    state[rate_words - 1] ^= ((uint64_t)0x80) << 56;
    
    keccak_f1600(state);
    
    // Extract
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            output[i * 8 + j] = (state[i] >> (8 * j)) & 0xFF;
        }
    }
}
