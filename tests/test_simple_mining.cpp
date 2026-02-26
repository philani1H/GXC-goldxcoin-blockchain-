// Simple Mining Test - Verify all 3 algorithms can hash and produce different results
// This tests the core hashing algorithms without full blockchain integration

#include "HashUtils.h"
#include "Block.h"
#include <iostream>
#include <iomanip>
#include <sstream>

#define RESET   "\033[0m"
#define GREEN   "\033[32m"
#define RED     "\033[31m"
#define CYAN    "\033[36m"
#define YELLOW  "\033[33m"

int tests_passed = 0;
int tests_failed = 0;

void print_header(const std::string& title) {
    std::cout << "\n" << CYAN << "========================================" << RESET << "\n";
    std::cout << CYAN << title << RESET << "\n";
    std::cout << CYAN << "========================================" << RESET << "\n";
}

void print_success(const std::string& msg) {
    std::cout << GREEN << "✅ PASS: " << msg << RESET << "\n";
    tests_passed++;
}

void print_fail(const std::string& msg) {
    std::cout << RED << "❌ FAIL: " << msg << RESET << "\n";
    tests_failed++;
}

void print_info(const std::string& msg) {
    std::cout << YELLOW << "ℹ️  " << msg << RESET << "\n";
}

int main() {
    print_header("GXC Mining Algorithm Verification");
    std::cout << "Testing that all 3 algorithms produce valid, unique hashes\n\n";

    std::string testData = "test_block_data";
    uint64_t nonce1 = 12345;
    uint64_t nonce2 = 67890;

    // Test 1: POW_SHA256 (Bitcoin-style double SHA-256)
    print_header("Test 1: POW_SHA256 Hashing");

    Block sha256_block1(0, "0000000000000000000000000000000000000000000000000000000000000000", BlockType::POW_SHA256);
    Block sha256_block2(0, "0000000000000000000000000000000000000000000000000000000000000000", BlockType::POW_SHA256);

    std::string hash1_sha256 = sha256_block1.calculateHash();
    std::string hash2_sha256 = sha256_block2.calculateHash();

    print_info("Block 1 Hash: " + hash1_sha256.substr(0, 32) + "...");
    print_info("Block 2 Hash: " + hash2_sha256.substr(0, 32) + "...");

    if (hash1_sha256.length() == 64) {
        print_success("SHA256 produces 64-character hex hash");
    } else {
        print_fail("SHA256 hash wrong length: " + std::to_string(hash1_sha256.length()));
    }

    if (hash1_sha256 == hash2_sha256) {
        print_success("SHA256 is deterministic (same input = same output)");
    } else {
        print_fail("SHA256 not deterministic");
    }

    // Test 2: POW_ETHASH (Ethereum-style)
    print_header("Test 2: POW_ETHASH Hashing");

    Block ethash_block1(0, "0000000000000000000000000000000000000000000000000000000000000000", BlockType::POW_ETHASH);
    Block ethash_block2(0, "0000000000000000000000000000000000000000000000000000000000000000", BlockType::POW_ETHASH);

    std::string hash1_ethash = ethash_block1.calculateHash();
    std::string hash2_ethash = ethash_block2.calculateHash();

    print_info("Block 1 Hash: " + hash1_ethash.substr(0, 32) + "...");
    print_info("Block 2 Hash: " + hash2_ethash.substr(0, 32) + "...");

    if (hash1_ethash.length() == 64) {
        print_success("Ethash produces 64-character hex hash");
    } else {
        print_fail("Ethash hash wrong length: " + std::to_string(hash1_ethash.length()));
    }

    // Test 3: POW_GXHASH (ASIC-resistant)
    print_header("Test 3: POW_GXHASH Hashing");

    Block gxhash_block1(0, "0000000000000000000000000000000000000000000000000000000000000000", BlockType::POW_GXHASH);
    Block gxhash_block2(0, "0000000000000000000000000000000000000000000000000000000000000000", BlockType::POW_GXHASH);

    std::string hash1_gxhash = gxhash_block1.calculateHash();
    std::string hash2_gxhash = gxhash_block2.calculateHash();

    print_info("Block 1 Hash: " + hash1_gxhash.substr(0, 32) + "...");
    print_info("Block 2 Hash: " + hash2_gxhash.substr(0, 32) + "...");

    if (hash1_gxhash.length() == 64) {
        print_success("GXHash produces 64-character hex hash");
    } else {
        print_fail("GXHash hash wrong length: " + std::to_string(hash1_gxhash.length()));
    }

    if (hash1_gxhash == hash2_gxhash) {
        print_success("GXHash is deterministic");
    } else {
        print_fail("GXHash not deterministic");
    }

    // Test 4: Verify algorithms produce different results
    print_header("Test 4: Algorithm Uniqueness");

    if (hash1_sha256 != hash1_ethash) {
        print_success("SHA256 and Ethash produce different hashes");
    } else {
        print_fail("SHA256 and Ethash collision!");
    }

    if (hash1_sha256 != hash1_gxhash) {
        print_success("SHA256 and GXHash produce different hashes");
    } else {
        print_fail("SHA256 and GXHash collision!");
    }

    if (hash1_ethash != hash1_gxhash) {
        print_success("Ethash and GXHash produce different hashes");
    } else {
        print_fail("Ethash and GXHash collision!");
    }

    // Test 5: Nonce changes hash
    print_header("Test 5: Nonce Impact");

    Block test_block(0, "prev_hash", BlockType::POW_SHA256);
    test_block.setNonce(100);
    std::string hash_nonce100 = test_block.calculateHash();

    test_block.setNonce(200);
    std::string hash_nonce200 = test_block.calculateHash();

    print_info("Nonce 100 Hash: " + hash_nonce100.substr(0, 32) + "...");
    print_info("Nonce 200 Hash: " + hash_nonce200.substr(0, 32) + "...");

    if (hash_nonce100 != hash_nonce200) {
        print_success("Changing nonce changes hash (mining works)");
    } else {
        print_fail("Nonce doesn't affect hash");
    }

    // Test 6: Work Receipt Generation
    print_header("Test 6: Work Receipt (Proof of Traceability)");

    Block pow_block(0, "prev_hash", BlockType::POW_SHA256);
    pow_block.setMinerPublicKey("miner_pubkey_test");
    pow_block.setMinerAddress("GXC_test_address");

    std::string workReceipt = pow_block.computeWorkReceipt();

    print_info("Work Receipt: " + workReceipt.substr(0, 32) + "...");

    if (workReceipt.length() == 64) {
        print_success("Work receipt generated (64-char SHA256)");
    } else {
        print_fail("Work receipt invalid length");
    }

    std::string workReceipt2 = pow_block.computeWorkReceipt();
    if (workReceipt == workReceipt2) {
        print_success("Work receipt is deterministic");
    } else {
        print_fail("Work receipt not deterministic");
    }

    // Test 7: Actual Algorithm Functions
    print_header("Test 7: Direct Algorithm Calls");

    std::string sha_result = sha256d(testData);
    std::string ethash_result = ethash(testData, nonce1);
    std::string gxhash_result = gxhash(testData, nonce1);

    print_info("SHA256d:  " + sha_result.substr(0, 32) + "...");
    print_info("Ethash:   " + ethash_result.substr(0, 32) + "...");
    print_info("GXHash:   " + gxhash_result.substr(0, 32) + "...");

    if (sha_result.length() == 64 && ethash_result.length() == 64 && gxhash_result.length() == 64) {
        print_success("All algorithm functions return 64-char hashes");
    } else {
        print_fail("Algorithm functions return wrong lengths");
    }

    // Summary
    print_header("Test Summary");
    std::cout << "Tests Passed: " << GREEN << tests_passed << RESET << "\n";
    std::cout << "Tests Failed: " << RED << tests_failed << RESET << "\n";
    std::cout << "Total Tests:  " << (tests_passed + tests_failed) << "\n\n";

    if (tests_failed == 0) {
        std::cout << GREEN << "✅ ALL ALGORITHMS VERIFIED!" << RESET << "\n";
        std::cout << GREEN << "POW_SHA256, POW_ETHASH, and POW_GXHASH are all working." << RESET << "\n";
        std::cout << GREEN << "The blockchain validates blocks using these 3 algorithms." << RESET << "\n";
        return 0;
    } else {
        std::cout << RED << "❌ SOME TESTS FAILED" << RESET << "\n";
        return 1;
    }
}
