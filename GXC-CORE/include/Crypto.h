#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/obj_mac.h>
#include <openssl/bn.h>
#include <openssl/sha.h>
#include <openssl/ripemd.h>

namespace Crypto {

// Key pair structure
struct KeyPair {
    std::string privateKey;  // Hex-encoded private key (32 bytes)
    std::string publicKey;   // Hex-encoded compressed public key (33 bytes)
};

// Generate a new secp256k1 key pair
KeyPair generateKeyPair();

// Derive public key from private key using secp256k1
std::string derivePublicKey(const std::string& privateKeyHex);

// Sign data using ECDSA with secp256k1
std::string signData(const std::string& data, const std::string& privateKeyHex);

// Verify ECDSA signature
bool verifySignature(const std::string& data, const std::string& signature, const std::string& publicKeyHex);

// Hash functions
std::string sha256(const std::string& data);
std::string sha256d(const std::string& data); // Double SHA-256
std::string ripemd160(const std::string& data);
std::string keccak256(const std::string& data); // Proper Keccak-256

// Address generation
std::string generateAddress(const std::string& publicKeyHex, bool testnet = false);

// Utility functions
std::vector<uint8_t> hexToBytes(const std::string& hex);
std::string bytesToHex(const std::vector<uint8_t>& bytes);
std::string bytesToHex(const uint8_t* data, size_t len);

} // namespace Crypto
