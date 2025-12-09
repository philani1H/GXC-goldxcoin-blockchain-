#pragma once

#include <cstdint>
#include <cstddef>

// Blake2b context
struct Blake2bContext {
    uint64_t h[8];      // Hash state
    uint64_t t[2];      // Byte counter
    uint64_t f[2];      // Finalization flags
    uint8_t buf[128];   // Input buffer
    size_t buflen;      // Buffer length
    size_t outlen;      // Output length
};

// Initialize Blake2b
void blake2b_init(Blake2bContext* ctx, size_t outlen, const uint8_t* key = nullptr, size_t keylen = 0);

// Update Blake2b with data
void blake2b_update(Blake2bContext* ctx, const uint8_t* data, size_t datalen);

// Finalize Blake2b
void blake2b_final(Blake2bContext* ctx, uint8_t* out);

// One-shot Blake2b hash
void blake2b(const uint8_t* data, size_t datalen, uint8_t* out, size_t outlen);

// Blake2b with key (for MAC)
void blake2b_keyed(const uint8_t* data, size_t datalen, const uint8_t* key, size_t keylen,
                   uint8_t* out, size_t outlen);
