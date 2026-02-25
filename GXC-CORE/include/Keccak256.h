#pragma once

#include <string>
#include <cstdint>
#include <cstddef>

// Keccak-256 hash function (SHA-3)
// Returns hex string
std::string keccak256_hash(const uint8_t* data, size_t length);
std::string keccak256_hash(const std::string& data);

// Keccak-256 raw output (32 bytes)
void keccak256_raw(const uint8_t* data, size_t length, uint8_t output[32]);
