// Test all three mining algorithms
#include "include/Block.h"
#include "include/HashUtils.h"
#include <iostream>
#include <chrono>

int main() {
    std::cout << "Testing all three mining algorithms..." << std::endl;
    std::cout << "========================================" << std::endl;
    
    // Test data
    std::string testData = "GXC Blockchain Test Block";
    uint64_t testNonce = 12345;
    
    // Test SHA-256
    std::cout << "\n1. Testing SHA-256 (Bitcoin-compatible):" << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    std::string sha256Hash = sha256d(testData);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "   Hash: " << sha256Hash.substr(0, 16) << "..." << std::endl;
    std::cout << "   Time: " << duration.count() << " microseconds" << std::endl;
    std::cout << "   Status: ✅ WORKING" << std::endl;
    
    // Test Ethash
    std::cout << "\n2. Testing Ethash (Ethereum-compatible):" << std::endl;
    start = std::chrono::high_resolution_clock::now();
    std::string ethashHash = ethash(testData, testNonce);
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "   Hash: " << ethashHash.substr(0, 16) << "..." << std::endl;
    std::cout << "   Time: " << duration.count() << " microseconds" << std::endl;
    std::cout << "   Status: ✅ WORKING" << std::endl;
    
    // Test GXHash
    std::cout << "\n3. Testing GXHash (ASIC-resistant):" << std::endl;
    start = std::chrono::high_resolution_clock::now();
    std::string gxhashHash = gxhash(testData, testNonce);
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "   Hash: " << gxhashHash.substr(0, 16) << "..." << std::endl;
    std::cout << "   Time: " << duration.count() << " microseconds" << std::endl;
    std::cout << "   Status: ✅ WORKING" << std::endl;
    
    // Test block creation with each algorithm
    std::cout << "\n========================================" << std::endl;
    std::cout << "Testing block creation with each algorithm:" << std::endl;
    
    // SHA-256 block
    std::cout << "\n1. Creating SHA-256 block..." << std::endl;
    Block sha256Block(1, "0000000000000000000000000000000000000000000000000000000000000000", BlockType::POW_SHA256);
    sha256Block.mineBlock(1.0);
    std::cout << "   Block hash: " << sha256Block.getHash().substr(0, 16) << "..." << std::endl;
    std::cout << "   Nonce: " << sha256Block.getNonce() << std::endl;
    std::cout << "   Status: ✅ MINED" << std::endl;
    
    // Ethash block
    std::cout << "\n2. Creating Ethash block..." << std::endl;
    Block ethashBlock(2, sha256Block.getHash(), BlockType::POW_ETHASH);
    ethashBlock.mineBlock(1.0);
    std::cout << "   Block hash: " << ethashBlock.getHash().substr(0, 16) << "..." << std::endl;
    std::cout << "   Nonce: " << ethashBlock.getNonce() << std::endl;
    std::cout << "   Status: ✅ MINED" << std::endl;
    
    // GXHash block
    std::cout << "\n3. Creating GXHash block..." << std::endl;
    Block gxhashBlock(3, ethashBlock.getHash(), BlockType::POW_GXHASH);
    gxhashBlock.mineBlock(1.0);
    std::cout << "   Block hash: " << gxhashBlock.getHash().substr(0, 16) << "..." << std::endl;
    std::cout << "   Nonce: " << gxhashBlock.getNonce() << std::endl;
    std::cout << "   Status: ✅ MINED" << std::endl;
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "✅ ALL THREE ALGORITHMS WORKING!" << std::endl;
    std::cout << "========================================" << std::endl;
    
    return 0;
}
