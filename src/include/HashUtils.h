#pragma once

#include <string>
#include <vector>
#include <cstdint>

// SHA-256 hash function
std::string sha256(const std::string& data);

// Double SHA-256 hash function (used in Bitcoin)
std::string sha256d(const std::string& data);

// Ethash function (simplified for demonstration)
std::string ethash(const std::string& data, uint64_t nonce);

// RIPEMD-160 hash function
std::string ripemd160(const std::string& data);

// Keccak-256 hash function
std::string keccak256(const std::string& data);

// Calculate Merkle root from a list of transaction hashes
std::string calculateMerkleRoot(const std::vector<std::string>& txHashes);

// Verify if a hash meets the target difficulty
bool meetsTarget(const std::string& hash, double difficulty);

// Convert a hash string to a numeric value for comparison
double hashToValue(const std::string& hash);
