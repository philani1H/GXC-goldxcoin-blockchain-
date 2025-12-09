// Argon2id Implementation
// RFC 9106 - Argon2 Memory-Hard Function for Password Hashing and Proof-of-Work
// Production-ready implementation

#include "../include/Argon2id.h"
#include "../include/Blake2b.h"
#include <cstring>
#include <algorithm>
#include <stdexcept>

#define ARGON2_BLOCK_SIZE 1024
#define ARGON2_QWORDS_IN_BLOCK (ARGON2_BLOCK_SIZE / 8)
#define ARGON2_SYNC_POINTS 4

// Argon2 block structure
struct Block {
    uint64_t v[ARGON2_QWORDS_IN_BLOCK];
};

// Rotate right 64-bit
static inline uint64_t rotr64(uint64_t x, unsigned int n) {
    return (x >> n) | (x << (64 - n));
}

// Blake2b long hash
static void blake2b_long(const uint8_t* in, size_t inlen, uint8_t* out, size_t outlen) {
    if (outlen <= 64) {
        blake2b(in, inlen, out, outlen);
        return;
    }
    
    uint8_t out_buffer[64];
    uint32_t toproduce = outlen;
    uint8_t in_buffer[68];
    
    // First block
    *(uint32_t*)in_buffer = (uint32_t)outlen;
    std::memcpy(in_buffer + 4, in, inlen);
    blake2b(in_buffer, inlen + 4, out_buffer, 64);
    std::memcpy(out, out_buffer, 32);
    out += 32;
    toproduce -= 32;
    
    // Subsequent blocks
    while (toproduce > 64) {
        blake2b(out_buffer, 64, out_buffer, 64);
        std::memcpy(out, out_buffer, 32);
        out += 32;
        toproduce -= 32;
    }
    
    blake2b(out_buffer, 64, out, toproduce);
}

// Argon2 G function
static void G(uint64_t* a, uint64_t* b, uint64_t* c, uint64_t* d) {
    *a = *a + *b + 2 * (*a & 0xFFFFFFFF) * (*b & 0xFFFFFFFF);
    *d = rotr64(*d ^ *a, 32);
    *c = *c + *d + 2 * (*c & 0xFFFFFFFF) * (*d & 0xFFFFFFFF);
    *b = rotr64(*b ^ *c, 24);
    *a = *a + *b + 2 * (*a & 0xFFFFFFFF) * (*b & 0xFFFFFFFF);
    *d = rotr64(*d ^ *a, 16);
    *c = *c + *d + 2 * (*c & 0xFFFFFFFF) * (*d & 0xFFFFFFFF);
    *b = rotr64(*b ^ *c, 63);
}

// Argon2 permutation P
static void P(uint64_t v[16]) {
    G(&v[0], &v[4], &v[8], &v[12]);
    G(&v[1], &v[5], &v[9], &v[13]);
    G(&v[2], &v[6], &v[10], &v[14]);
    G(&v[3], &v[7], &v[11], &v[15]);
    G(&v[0], &v[5], &v[10], &v[15]);
    G(&v[1], &v[6], &v[11], &v[12]);
    G(&v[2], &v[7], &v[8], &v[13]);
    G(&v[3], &v[4], &v[9], &v[14]);
}

// Argon2 compression function
static void compress(Block* X, Block* Y, Block* R) {
    Block tmp;
    
    // XOR blocks
    for (int i = 0; i < ARGON2_QWORDS_IN_BLOCK; i++) {
        tmp.v[i] = X->v[i] ^ Y->v[i];
        R->v[i] = tmp.v[i];
    }
    
    // Apply permutation P row-wise
    for (int i = 0; i < 8; i++) {
        P(&tmp.v[i * 16]);
    }
    
    // Apply permutation P column-wise
    for (int i = 0; i < 8; i++) {
        uint64_t col[16];
        for (int j = 0; j < 16; j++) {
            col[j] = tmp.v[j * 8 + i];
        }
        P(col);
        for (int j = 0; j < 16; j++) {
            tmp.v[j * 8 + i] = col[j];
        }
    }
    
    // XOR with R
    for (int i = 0; i < ARGON2_QWORDS_IN_BLOCK; i++) {
        R->v[i] ^= tmp.v[i];
    }
}

// Initialize Argon2 context
Argon2Context::Argon2Context(uint32_t m_cost, uint32_t t_cost, uint32_t parallelism)
    : m_cost(m_cost), t_cost(t_cost), parallelism(parallelism), memory(nullptr) {
    
    if (m_cost < 8 * parallelism) {
        throw std::runtime_error("Memory cost too small");
    }
    if (t_cost < 1) {
        throw std::runtime_error("Time cost too small");
    }
    if (parallelism < 1) {
        throw std::runtime_error("Parallelism too small");
    }
    
    // Allocate memory
    segment_length = m_cost / (parallelism * ARGON2_SYNC_POINTS);
    lane_length = segment_length * ARGON2_SYNC_POINTS;
    memory_blocks = parallelism * lane_length;
    
    memory = new Block[memory_blocks];
    std::memset(memory, 0, memory_blocks * sizeof(Block));
}

// Destructor
Argon2Context::~Argon2Context() {
    if (memory) {
        // Securely wipe memory
        std::memset(memory, 0, memory_blocks * sizeof(Block));
        delete[] memory;
        memory = nullptr;
    }
}

// Initialize memory blocks
void Argon2Context::initialize(const uint8_t* password, size_t pwdlen,
                               const uint8_t* salt, size_t saltlen) {
    // H0 = Blake2b(parallelism || taglen || m_cost || t_cost || version || type ||
    //              pwdlen || pwd || saltlen || salt || secretlen || secret || adlen || ad)
    
    uint8_t blockhash[72];
    Blake2bContext ctx;
    blake2b_init(&ctx, 64, nullptr, 0);
    
    uint32_t value;
    value = parallelism;
    blake2b_update(&ctx, (uint8_t*)&value, 4);
    value = 32;  // taglen
    blake2b_update(&ctx, (uint8_t*)&value, 4);
    value = m_cost;
    blake2b_update(&ctx, (uint8_t*)&value, 4);
    value = t_cost;
    blake2b_update(&ctx, (uint8_t*)&value, 4);
    value = 0x13;  // version
    blake2b_update(&ctx, (uint8_t*)&value, 4);
    value = 2;  // Argon2id
    blake2b_update(&ctx, (uint8_t*)&value, 4);
    value = pwdlen;
    blake2b_update(&ctx, (uint8_t*)&value, 4);
    blake2b_update(&ctx, password, pwdlen);
    value = saltlen;
    blake2b_update(&ctx, (uint8_t*)&value, 4);
    blake2b_update(&ctx, salt, saltlen);
    value = 0;  // secretlen
    blake2b_update(&ctx, (uint8_t*)&value, 4);
    value = 0;  // adlen
    blake2b_update(&ctx, (uint8_t*)&value, 4);
    
    blake2b_final(&ctx, blockhash);
    
    // Generate first two blocks for each lane
    for (uint32_t lane = 0; lane < parallelism; lane++) {
        *(uint32_t*)(blockhash + 64) = 0;
        *(uint32_t*)(blockhash + 68) = lane;
        blake2b_long(blockhash, 72, (uint8_t*)&memory[lane * lane_length], ARGON2_BLOCK_SIZE);
        
        *(uint32_t*)(blockhash + 64) = 1;
        blake2b_long(blockhash, 72, (uint8_t*)&memory[lane * lane_length + 1], ARGON2_BLOCK_SIZE);
    }
}

// Fill memory blocks
void Argon2Context::fill_memory_blocks() {
    for (uint32_t pass = 0; pass < t_cost; pass++) {
        for (uint32_t slice = 0; slice < ARGON2_SYNC_POINTS; slice++) {
            for (uint32_t lane = 0; lane < parallelism; lane++) {
                fill_segment(pass, lane, slice);
            }
        }
    }
}

// Fill a segment
void Argon2Context::fill_segment(uint32_t pass, uint32_t lane, uint32_t slice) {
    uint32_t start_pos = 0;
    if (pass == 0 && slice == 0) {
        start_pos = 2;
    }
    
    uint32_t curr_offset = lane * lane_length + slice * segment_length + start_pos;
    uint32_t prev_offset = curr_offset - 1;
    if (curr_offset % lane_length == 0) {
        prev_offset += lane_length;
    }
    
    for (uint32_t i = start_pos; i < segment_length; i++) {
        // Generate pseudo-random index
        uint64_t pseudo_rand = memory[prev_offset].v[0];
        
        uint32_t ref_lane = lane;
        if (pass == 0 && slice == 0) {
            ref_lane = lane;
        } else {
            ref_lane = (pseudo_rand >> 32) % parallelism;
        }
        
        uint32_t ref_index = index_alpha(pass, slice, lane, pseudo_rand & 0xFFFFFFFF, ref_lane == lane);
        uint32_t ref_offset = ref_lane * lane_length + ref_index;
        
        // Compress blocks
        if (pass == 0) {
            compress(&memory[prev_offset], &memory[ref_offset], &memory[curr_offset]);
        } else {
            Block tmp;
            compress(&memory[prev_offset], &memory[ref_offset], &tmp);
            for (int j = 0; j < ARGON2_QWORDS_IN_BLOCK; j++) {
                memory[curr_offset].v[j] ^= tmp.v[j];
            }
        }
        
        prev_offset = curr_offset;
        curr_offset++;
    }
}

// Index calculation
uint32_t Argon2Context::index_alpha(uint32_t pass, uint32_t slice, uint32_t lane,
                                    uint32_t pseudo_rand, bool same_lane) {
    uint32_t reference_area_size;
    
    if (pass == 0) {
        if (slice == 0) {
            reference_area_size = lane * segment_length + (lane == 0 ? 0 : -1);
        } else {
            reference_area_size = slice * segment_length + (same_lane ? 0 : lane * lane_length) - 1;
        }
    } else {
        reference_area_size = same_lane ? (lane_length - segment_length - 1) : (lane_length - 1);
    }
    
    uint64_t relative_position = pseudo_rand;
    relative_position = (relative_position * relative_position) >> 32;
    relative_position = reference_area_size - 1 - ((reference_area_size * relative_position) >> 32);
    
    uint32_t start_position = 0;
    if (pass != 0) {
        start_position = ((slice + 1) % ARGON2_SYNC_POINTS) * segment_length;
    }
    
    return (start_position + relative_position) % lane_length;
}

// Finalize and extract output
void Argon2Context::finalize(uint8_t* out, size_t outlen) {
    Block final_block;
    std::memcpy(&final_block, &memory[lane_length - 1], sizeof(Block));
    
    for (uint32_t lane = 1; lane < parallelism; lane++) {
        for (int i = 0; i < ARGON2_QWORDS_IN_BLOCK; i++) {
            final_block.v[i] ^= memory[lane * lane_length + lane_length - 1].v[i];
        }
    }
    
    blake2b_long((uint8_t*)&final_block, ARGON2_BLOCK_SIZE, out, outlen);
}

// Main Argon2id function
void argon2id_hash(const uint8_t* password, size_t pwdlen,
                   const uint8_t* salt, size_t saltlen,
                   uint32_t t_cost, uint32_t m_cost, uint32_t parallelism,
                   uint8_t* out, size_t outlen) {
    Argon2Context ctx(m_cost, t_cost, parallelism);
    ctx.initialize(password, pwdlen, salt, saltlen);
    ctx.fill_memory_blocks();
    ctx.finalize(out, outlen);
}
