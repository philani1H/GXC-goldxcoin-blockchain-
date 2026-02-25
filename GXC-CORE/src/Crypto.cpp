#include "../include/Crypto.h"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <cstring>

namespace Crypto {

// Utility: Convert hex string to bytes
std::vector<uint8_t> hexToBytes(const std::string& hex) {
    std::vector<uint8_t> bytes;
    for (size_t i = 0; i < hex.length(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        uint8_t byte = static_cast<uint8_t>(strtol(byteString.c_str(), nullptr, 16));
        bytes.push_back(byte);
    }
    return bytes;
}

// Utility: Convert bytes to hex string
std::string bytesToHex(const std::vector<uint8_t>& bytes) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (uint8_t byte : bytes) {
        ss << std::setw(2) << static_cast<int>(byte);
    }
    return ss.str();
}

std::string bytesToHex(const uint8_t* data, size_t len) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (size_t i = 0; i < len; i++) {
        ss << std::setw(2) << static_cast<int>(data[i]);
    }
    return ss.str();
}

// Generate a new secp256k1 key pair
KeyPair generateKeyPair() {
    KeyPair keyPair;
    
    // Create EC_KEY object for secp256k1
    EC_KEY* eckey = EC_KEY_new_by_curve_name(NID_secp256k1);
    if (!eckey) {
        throw std::runtime_error("Failed to create EC_KEY");
    }
    
    // Generate the key pair
    if (EC_KEY_generate_key(eckey) != 1) {
        EC_KEY_free(eckey);
        throw std::runtime_error("Failed to generate EC key pair");
    }
    
    // Get private key
    const BIGNUM* privKey = EC_KEY_get0_private_key(eckey);
    if (!privKey) {
        EC_KEY_free(eckey);
        throw std::runtime_error("Failed to get private key");
    }
    
    // Convert private key to hex (32 bytes)
    char* privKeyHex = BN_bn2hex(privKey);
    keyPair.privateKey = std::string(privKeyHex);
    OPENSSL_free(privKeyHex);
    
    // Pad to 64 characters (32 bytes)
    while (keyPair.privateKey.length() < 64) {
        keyPair.privateKey = "0" + keyPair.privateKey;
    }
    
    // Get public key (compressed format)
    const EC_GROUP* group = EC_KEY_get0_group(eckey);
    const EC_POINT* pubKey = EC_KEY_get0_public_key(eckey);
    
    // Convert to compressed format (33 bytes)
    size_t pubKeyLen = EC_POINT_point2oct(group, pubKey, POINT_CONVERSION_COMPRESSED, nullptr, 0, nullptr);
    std::vector<uint8_t> pubKeyBytes(pubKeyLen);
    EC_POINT_point2oct(group, pubKey, POINT_CONVERSION_COMPRESSED, pubKeyBytes.data(), pubKeyLen, nullptr);
    
    keyPair.publicKey = bytesToHex(pubKeyBytes);
    
    EC_KEY_free(eckey);
    return keyPair;
}

// Derive public key from private key
std::string derivePublicKey(const std::string& privateKeyHex) {
    // Create EC_KEY object
    EC_KEY* eckey = EC_KEY_new_by_curve_name(NID_secp256k1);
    if (!eckey) {
        throw std::runtime_error("Failed to create EC_KEY");
    }
    
    // Convert hex private key to BIGNUM
    BIGNUM* privKey = BN_new();
    if (BN_hex2bn(&privKey, privateKeyHex.c_str()) == 0) {
        EC_KEY_free(eckey);
        BN_free(privKey);
        throw std::runtime_error("Failed to convert private key");
    }
    
    // Set private key
    if (EC_KEY_set_private_key(eckey, privKey) != 1) {
        EC_KEY_free(eckey);
        BN_free(privKey);
        throw std::runtime_error("Failed to set private key");
    }
    
    // Derive public key from private key
    const EC_GROUP* group = EC_KEY_get0_group(eckey);
    EC_POINT* pubKey = EC_POINT_new(group);
    
    if (EC_POINT_mul(group, pubKey, privKey, nullptr, nullptr, nullptr) != 1) {
        EC_KEY_free(eckey);
        BN_free(privKey);
        EC_POINT_free(pubKey);
        throw std::runtime_error("Failed to derive public key");
    }
    
    // Set public key
    if (EC_KEY_set_public_key(eckey, pubKey) != 1) {
        EC_KEY_free(eckey);
        BN_free(privKey);
        EC_POINT_free(pubKey);
        throw std::runtime_error("Failed to set public key");
    }
    
    // Convert to compressed format
    size_t pubKeyLen = EC_POINT_point2oct(group, pubKey, POINT_CONVERSION_COMPRESSED, nullptr, 0, nullptr);
    std::vector<uint8_t> pubKeyBytes(pubKeyLen);
    EC_POINT_point2oct(group, pubKey, POINT_CONVERSION_COMPRESSED, pubKeyBytes.data(), pubKeyLen, nullptr);
    
    std::string result = bytesToHex(pubKeyBytes);
    
    EC_KEY_free(eckey);
    BN_free(privKey);
    EC_POINT_free(pubKey);
    
    return result;
}

// Sign data using ECDSA
std::string signData(const std::string& data, const std::string& privateKeyHex) {
    // Hash the data first
    std::string dataHash = sha256(data);
    std::vector<uint8_t> hashBytes = hexToBytes(dataHash);
    
    // Create EC_KEY object
    EC_KEY* eckey = EC_KEY_new_by_curve_name(NID_secp256k1);
    if (!eckey) {
        throw std::runtime_error("Failed to create EC_KEY for signing");
    }
    
    // Convert hex private key to BIGNUM
    BIGNUM* privKey = BN_new();
    if (BN_hex2bn(&privKey, privateKeyHex.c_str()) == 0) {
        EC_KEY_free(eckey);
        BN_free(privKey);
        throw std::runtime_error("Failed to convert private key for signing");
    }
    
    // Set private key
    if (EC_KEY_set_private_key(eckey, privKey) != 1) {
        EC_KEY_free(eckey);
        BN_free(privKey);
        throw std::runtime_error("Failed to set private key for signing");
    }
    
    // Derive and set public key (required for signing)
    const EC_GROUP* group = EC_KEY_get0_group(eckey);
    EC_POINT* pubKey = EC_POINT_new(group);
    EC_POINT_mul(group, pubKey, privKey, nullptr, nullptr, nullptr);
    EC_KEY_set_public_key(eckey, pubKey);
    
    // Sign the hash
    ECDSA_SIG* signature = ECDSA_do_sign(hashBytes.data(), hashBytes.size(), eckey);
    if (!signature) {
        EC_KEY_free(eckey);
        BN_free(privKey);
        EC_POINT_free(pubKey);
        throw std::runtime_error("Failed to create ECDSA signature");
    }
    
    // Convert signature to DER format
    int derLen = i2d_ECDSA_SIG(signature, nullptr);
    std::vector<uint8_t> derSig(derLen);
    uint8_t* derPtr = derSig.data();
    i2d_ECDSA_SIG(signature, &derPtr);
    
    std::string result = bytesToHex(derSig);
    
    ECDSA_SIG_free(signature);
    EC_KEY_free(eckey);
    BN_free(privKey);
    EC_POINT_free(pubKey);
    
    return result;
}

// Verify ECDSA signature
bool verifySignature(const std::string& data, const std::string& signatureHex, const std::string& publicKeyHex) {
    try {
        // Hash the data
        std::string dataHash = sha256(data);
        std::vector<uint8_t> hashBytes = hexToBytes(dataHash);
        
        // Convert signature from hex
        std::vector<uint8_t> sigBytes = hexToBytes(signatureHex);
        
        // Parse DER signature
        const uint8_t* sigPtr = sigBytes.data();
        ECDSA_SIG* signature = d2i_ECDSA_SIG(nullptr, &sigPtr, sigBytes.size());
        if (!signature) {
            return false;
        }
        
        // Create EC_KEY object
        EC_KEY* eckey = EC_KEY_new_by_curve_name(NID_secp256k1);
        if (!eckey) {
            ECDSA_SIG_free(signature);
            return false;
        }
        
        // Convert public key from hex
        std::vector<uint8_t> pubKeyBytes = hexToBytes(publicKeyHex);
        
        // Set public key
        const EC_GROUP* group = EC_KEY_get0_group(eckey);
        EC_POINT* pubKey = EC_POINT_new(group);
        
        if (EC_POINT_oct2point(group, pubKey, pubKeyBytes.data(), pubKeyBytes.size(), nullptr) != 1) {
            EC_KEY_free(eckey);
            EC_POINT_free(pubKey);
            ECDSA_SIG_free(signature);
            return false;
        }
        
        if (EC_KEY_set_public_key(eckey, pubKey) != 1) {
            EC_KEY_free(eckey);
            EC_POINT_free(pubKey);
            ECDSA_SIG_free(signature);
            return false;
        }
        
        // Verify signature
        int result = ECDSA_do_verify(hashBytes.data(), hashBytes.size(), signature, eckey);
        
        EC_KEY_free(eckey);
        EC_POINT_free(pubKey);
        ECDSA_SIG_free(signature);
        
        return result == 1;
        
    } catch (...) {
        return false;
    }
}

// SHA-256 hash
std::string sha256(const std::string& data) {
    uint8_t hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const uint8_t*>(data.c_str()), data.length(), hash);
    return bytesToHex(hash, SHA256_DIGEST_LENGTH);
}

// Double SHA-256
std::string sha256d(const std::string& data) {
    std::string firstHash = sha256(data);
    std::vector<uint8_t> firstHashBytes = hexToBytes(firstHash);
    
    uint8_t hash[SHA256_DIGEST_LENGTH];
    SHA256(firstHashBytes.data(), firstHashBytes.size(), hash);
    return bytesToHex(hash, SHA256_DIGEST_LENGTH);
}

// RIPEMD-160 hash
std::string ripemd160(const std::string& data) {
    uint8_t hash[RIPEMD160_DIGEST_LENGTH];
    RIPEMD160(reinterpret_cast<const uint8_t*>(data.c_str()), data.length(), hash);
    return bytesToHex(hash, RIPEMD160_DIGEST_LENGTH);
}

// Keccak-256 (using OpenSSL EVP interface)
std::string keccak256(const std::string& data) {
    // OpenSSL 3.0+ supports Keccak via EVP
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) {
        throw std::runtime_error("Failed to create EVP context");
    }
    
    const EVP_MD* md = EVP_MD_fetch(nullptr, "KECCAK-256", nullptr);
    if (!md) {
        // Fallback to SHA3-256 if Keccak not available
        md = EVP_sha3_256();
        if (!md) {
            EVP_MD_CTX_free(ctx);
            // Ultimate fallback: use SHA-256
            return sha256(data);
        }
    }
    
    if (EVP_DigestInit_ex(ctx, md, nullptr) != 1) {
        EVP_MD_CTX_free(ctx);
        throw std::runtime_error("Failed to initialize digest");
    }
    
    if (EVP_DigestUpdate(ctx, data.c_str(), data.length()) != 1) {
        EVP_MD_CTX_free(ctx);
        throw std::runtime_error("Failed to update digest");
    }
    
    uint8_t hash[32];
    unsigned int hashLen;
    if (EVP_DigestFinal_ex(ctx, hash, &hashLen) != 1) {
        EVP_MD_CTX_free(ctx);
        throw std::runtime_error("Failed to finalize digest");
    }
    
    EVP_MD_CTX_free(ctx);
    return bytesToHex(hash, hashLen);
}

// Generate address from public key
std::string generateAddress(const std::string& publicKeyHex, bool testnet) {
    // Convert public key to bytes
    std::vector<uint8_t> pubKeyBytes = hexToBytes(publicKeyHex);
    
    // SHA-256 hash of public key
    uint8_t sha256Hash[SHA256_DIGEST_LENGTH];
    SHA256(pubKeyBytes.data(), pubKeyBytes.size(), sha256Hash);
    
    // RIPEMD-160 hash of SHA-256 hash
    uint8_t ripemd160Hash[RIPEMD160_DIGEST_LENGTH];
    RIPEMD160(sha256Hash, SHA256_DIGEST_LENGTH, ripemd160Hash);
    
    // Convert to hex
    std::string hash160 = bytesToHex(ripemd160Hash, RIPEMD160_DIGEST_LENGTH);
    
    // Add prefix
    std::string prefix = testnet ? "tGXC" : "GXC";
    return prefix + hash160.substr(0, 34);
}

} // namespace Crypto
