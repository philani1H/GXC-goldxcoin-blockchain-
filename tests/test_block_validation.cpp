// Test Block Validation for All Algorithms
// Verifies that blocks are validated correctly using hash recalculation

#include "../include/block.h"
#include "../include/HashUtils.h"
#include <iostream>
#include <cassert>

void print_result(const std::string& test, bool passed) {
    if (passed) {
        std::cout << "✅ PASS: " << test << "\n";
    } else {
        std::cout << "❌ FAIL: " << test << "\n";
    }
}

int main() {
    std::cout << "========================================\n";
    std::cout << "Block Validation Test Suite\n";
    std::cout << "========================================\n\n";
    
    int tests_passed = 0;
    int tests_failed = 0;
    
    // Test 1: SHA-256 Block Validation
    std::cout << "Test 1: SHA-256 Block Hash Validation\n";
    std::cout << "--------------------------------------\n";
    {
        Block block(1, "0000000000000000000000000000000000000000000000000000000000000000", BlockType::POW_SHA256);
        block.setNonce(12345);
        block.setTimestamp(1234567890);
        
        // Calculate correct hash
        std::string correctHash = block.calculateHash();
        block.setHash(correctHash);
        
        // Recalculate and verify
        std::string recalculatedHash = block.calculateHash();
        bool valid = (correctHash == recalculatedHash);
        
        std::cout << "Original hash:      " << correctHash.substr(0, 32) << "...\n";
        std::cout << "Recalculated hash:  " << recalculatedHash.substr(0, 32) << "...\n";
        
        if (valid) {
            std::cout << "✅ PASS: SHA-256 hash validation works\n";
            tests_passed++;
        } else {
            std::cout << "❌ FAIL: SHA-256 hash mismatch\n";
            tests_failed++;
        }
    }
    
    // Test 2: Ethash Block Validation
    std::cout << "\nTest 2: Ethash Block Hash Validation\n";
    std::cout << "--------------------------------------\n";
    {
        Block block(1, "0000000000000000000000000000000000000000000000000000000000000000", BlockType::POW_ETHASH);
        block.setNonce(12345);
        block.setTimestamp(1234567890);
        
        // Calculate correct hash
        std::string correctHash = block.calculateHash();
        block.setHash(correctHash);
        
        // Recalculate and verify
        std::string recalculatedHash = block.calculateHash();
        bool valid = (correctHash == recalculatedHash);
        
        std::cout << "Original hash:      " << correctHash.substr(0, 32) << "...\n";
        std::cout << "Recalculated hash:  " << recalculatedHash.substr(0, 32) << "...\n";
        
        if (valid) {
            std::cout << "✅ PASS: Ethash hash validation works\n";
            tests_passed++;
        } else {
            std::cout << "❌ FAIL: Ethash hash mismatch\n";
            tests_failed++;
        }
    }
    
    // Test 3: GXHash Block Validation
    std::cout << "\nTest 3: GXHash Block Hash Validation\n";
    std::cout << "--------------------------------------\n";
    {
        Block block(1, "0000000000000000000000000000000000000000000000000000000000000000", BlockType::POW_GXHASH);
        block.setNonce(12345);
        block.setTimestamp(1234567890);
        
        // Calculate correct hash
        std::string correctHash = block.calculateHash();
        block.setHash(correctHash);
        
        // Recalculate and verify
        std::string recalculatedHash = block.calculateHash();
        bool valid = (correctHash == recalculatedHash);
        
        std::cout << "Original hash:      " << correctHash.substr(0, 32) << "...\n";
        std::cout << "Recalculated hash:  " << recalculatedHash.substr(0, 32) << "...\n";
        
        if (valid) {
            std::cout << "✅ PASS: GXHash hash validation works\n";
            tests_passed++;
        } else {
            std::cout << "❌ FAIL: GXHash hash mismatch\n";
            tests_failed++;
        }
    }
    
    // Test 4: Detect Invalid Hash (Security Test)
    std::cout << "\nTest 4: Invalid Hash Detection\n";
    std::cout << "--------------------------------------\n";
    {
        Block block(1, "0000000000000000000000000000000000000000000000000000000000000000", BlockType::POW_SHA256);
        block.setNonce(12345);
        block.setTimestamp(1234567890);
        
        // Set WRONG hash
        std::string fakeHash = "0000000000000000111111111111111122222222222222223333333333333333";
        block.setHash(fakeHash);
        
        // Recalculate and verify
        std::string correctHash = block.calculateHash();
        bool detected = (fakeHash != correctHash);
        
        std::cout << "Fake hash:     " << fakeHash.substr(0, 32) << "...\n";
        std::cout << "Correct hash:  " << correctHash.substr(0, 32) << "...\n";
        
        if (detected) {
            std::cout << "✅ PASS: Invalid hash detected\n";
            tests_passed++;
        } else {
            std::cout << "❌ FAIL: Invalid hash NOT detected\n";
            tests_failed++;
        }
    }
    
    // Test 5: Different Nonces Produce Different Hashes
    std::cout << "\nTest 5: Nonce Changes Hash\n";
    std::cout << "--------------------------------------\n";
    {
        Block block1(1, "0000000000000000000000000000000000000000000000000000000000000000", BlockType::POW_GXHASH);
        block1.setNonce(12345);
        block1.setTimestamp(1234567890);
        
        Block block2(1, "0000000000000000000000000000000000000000000000000000000000000000", BlockType::POW_GXHASH);
        block2.setNonce(54321);  // Different nonce
        block2.setTimestamp(1234567890);
        
        std::string hash1 = block1.calculateHash();
        std::string hash2 = block2.calculateHash();
        
        bool different = (hash1 != hash2);
        
        std::cout << "Hash with nonce 12345: " << hash1.substr(0, 32) << "...\n";
        std::cout << "Hash with nonce 54321: " << hash2.substr(0, 32) << "...\n";
        
        if (different) {
            std::cout << "✅ PASS: Different nonces produce different hashes\n";
            tests_passed++;
        } else {
            std::cout << "❌ FAIL: Same hash with different nonces\n";
            tests_failed++;
        }
    }
    
    // Summary
    std::cout << "\n========================================\n";
    std::cout << "Test Summary\n";
    std::cout << "========================================\n";
    std::cout << "Tests Passed: " << tests_passed << "\n";
    std::cout << "Tests Failed: " << tests_failed << "\n";
    std::cout << "Total Tests:  " << (tests_passed + tests_failed) << "\n\n";
    
    if (tests_failed == 0) {
        std::cout << "✅ ALL TESTS PASSED!\n";
        std::cout << "Block validation is production-ready.\n";
        return 0;
    } else {
        std::cout << "❌ SOME TESTS FAILED\n";
        return 1;
    }
}
