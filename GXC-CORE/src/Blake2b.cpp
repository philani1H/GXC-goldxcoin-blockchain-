// Blake2b Implementation
// RFC 7693 - The BLAKE2 Cryptographic Hash and Message Authentication Code (MAC)
// Production-ready implementation for Argon2id

#include "../include/Blake2b.h"
#include <cstring>
#include <algorithm>
#include <stdexcept>

// Blake2b initialization vectors
static const uint64_t blake2b_IV[8] = {
    0x6a09e667f3bcc908ULL, 0xbb67ae8584caa73bULL,
    0x3c6ef372fe94f82bULL, 0xa54ff53a5f1d36f1ULL,
    0x510e527fade682d1ULL, 0x9b05688c2b3e6c1fULL,
    0x1f83d9abfb41bd6bULL, 0x5be0cd19137e2179ULL
};

// Blake2b sigma permutations
static const uint8_t blake2b_sigma[12][16] = {
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
    {14, 10, 4, 8, 9, 15, 13, 6, 1, 12, 0, 2, 11, 7, 5, 3},
    {11, 8, 12, 0, 5, 2, 15, 13, 10, 14, 3, 6, 7, 1, 9, 4},
    {7, 9, 3, 1, 13, 12, 11, 14, 2, 6, 5, 10, 4, 0, 15, 8},
    {9, 0, 5, 7, 2, 4, 10, 15, 14, 1, 11, 12, 6, 8, 3, 13},
    {2, 12, 6, 10, 0, 11, 8, 3, 4, 13, 7, 5, 15, 14, 1, 9},
    {12, 5, 1, 15, 14, 13, 4, 10, 0, 7, 6, 3, 9, 2, 8, 11},
    {13, 11, 7, 14, 12, 1, 3, 9, 5, 0, 15, 4, 8, 6, 2, 10},
    {6, 15, 14, 9, 11, 3, 0, 8, 12, 2, 13, 7, 1, 4, 10, 5},
    {10, 2, 8, 4, 7, 6, 1, 5, 15, 11, 9, 14, 3, 12, 13, 0},
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
    {14, 10, 4, 8, 9, 15, 13, 6, 1, 12, 0, 2, 11, 7, 5, 3}
};

// Rotate right
static inline uint64_t rotr64(uint64_t x, int n) {
    return (x >> n) | (x << (64 - n));
}

// Blake2b mixing function G
static void blake2b_G(uint64_t* v, int a, int b, int c, int d, uint64_t x, uint64_t y) {
    v[a] = v[a] + v[b] + x;
    v[d] = rotr64(v[d] ^ v[a], 32);
    v[c] = v[c] + v[d];
    v[b] = rotr64(v[b] ^ v[c], 24);
    v[a] = v[a] + v[b] + y;
    v[d] = rotr64(v[d] ^ v[a], 16);
    v[c] = v[c] + v[d];
    v[b] = rotr64(v[b] ^ v[c], 63);
}

// Blake2b compression function
static void blake2b_compress(Blake2bContext* ctx, const uint8_t block[128]) {
    uint64_t m[16];
    uint64_t v[16];
    
    // Load message block
    for (int i = 0; i < 16; i++) {
        m[i] = ((uint64_t*)block)[i];
    }
    
    // Initialize work vector
    for (int i = 0; i < 8; i++) {
        v[i] = ctx->h[i];
        v[i + 8] = blake2b_IV[i];
    }
    
    v[12] ^= ctx->t[0];
    v[13] ^= ctx->t[1];
    
    if (ctx->f[0]) {
        v[14] = ~v[14];
    }
    
    // 12 rounds
    for (int round = 0; round < 12; round++) {
        blake2b_G(v, 0, 4, 8, 12, m[blake2b_sigma[round][0]], m[blake2b_sigma[round][1]]);
        blake2b_G(v, 1, 5, 9, 13, m[blake2b_sigma[round][2]], m[blake2b_sigma[round][3]]);
        blake2b_G(v, 2, 6, 10, 14, m[blake2b_sigma[round][4]], m[blake2b_sigma[round][5]]);
        blake2b_G(v, 3, 7, 11, 15, m[blake2b_sigma[round][6]], m[blake2b_sigma[round][7]]);
        blake2b_G(v, 0, 5, 10, 15, m[blake2b_sigma[round][8]], m[blake2b_sigma[round][9]]);
        blake2b_G(v, 1, 6, 11, 12, m[blake2b_sigma[round][10]], m[blake2b_sigma[round][11]]);
        blake2b_G(v, 2, 7, 8, 13, m[blake2b_sigma[round][12]], m[blake2b_sigma[round][13]]);
        blake2b_G(v, 3, 4, 9, 14, m[blake2b_sigma[round][14]], m[blake2b_sigma[round][15]]);
    }
    
    // Update hash state
    for (int i = 0; i < 8; i++) {
        ctx->h[i] ^= v[i] ^ v[i + 8];
    }
}

// Initialize Blake2b context
void blake2b_init(Blake2bContext* ctx, size_t outlen, const uint8_t* key, size_t keylen) {
    if (outlen == 0 || outlen > 64) {
        throw std::runtime_error("Invalid Blake2b output length");
    }
    if (keylen > 64) {
        throw std::runtime_error("Invalid Blake2b key length");
    }
    
    // Initialize state
    for (int i = 0; i < 8; i++) {
        ctx->h[i] = blake2b_IV[i];
    }
    
    // Mix in parameters
    ctx->h[0] ^= 0x01010000 ^ (keylen << 8) ^ outlen;
    
    ctx->t[0] = 0;
    ctx->t[1] = 0;
    ctx->f[0] = 0;
    ctx->f[1] = 0;
    ctx->buflen = 0;
    ctx->outlen = outlen;
    
    // Process key if present
    if (keylen > 0) {
        uint8_t block[128] = {0};
        std::memcpy(block, key, keylen);
        blake2b_update(ctx, block, 128);
        std::memset(block, 0, 128);  // Clear sensitive data
    }
}

// Update Blake2b with data
void blake2b_update(Blake2bContext* ctx, const uint8_t* data, size_t datalen) {
    for (size_t i = 0; i < datalen; i++) {
        if (ctx->buflen == 128) {
            ctx->t[0] += 128;
            if (ctx->t[0] < 128) {
                ctx->t[1]++;
            }
            blake2b_compress(ctx, ctx->buf);
            ctx->buflen = 0;
        }
        ctx->buf[ctx->buflen++] = data[i];
    }
}

// Finalize Blake2b and output hash
void blake2b_final(Blake2bContext* ctx, uint8_t* out) {
    // Pad last block
    if (ctx->buflen < 128) {
        std::memset(ctx->buf + ctx->buflen, 0, 128 - ctx->buflen);
    }
    
    ctx->t[0] += ctx->buflen;
    if (ctx->t[0] < ctx->buflen) {
        ctx->t[1]++;
    }
    ctx->f[0] = (uint64_t)-1;
    
    blake2b_compress(ctx, ctx->buf);
    
    // Output hash
    std::memcpy(out, ctx->h, ctx->outlen);
    
    // Clear context
    std::memset(ctx, 0, sizeof(Blake2bContext));
}

// One-shot Blake2b hash
void blake2b(const uint8_t* data, size_t datalen, uint8_t* out, size_t outlen) {
    Blake2bContext ctx;
    blake2b_init(&ctx, outlen, nullptr, 0);
    blake2b_update(&ctx, data, datalen);
    blake2b_final(&ctx, out);
}

// Blake2b with key
void blake2b_keyed(const uint8_t* data, size_t datalen, const uint8_t* key, size_t keylen,
                   uint8_t* out, size_t outlen) {
    Blake2bContext ctx;
    blake2b_init(&ctx, outlen, key, keylen);
    blake2b_update(&ctx, data, datalen);
    blake2b_final(&ctx, out);
}
