// Comprehensive Mining Test - All 3 Algorithms with Reward Validation
// Tests POW_SHA256, POW_ETHASH, POW_GXHASH mining and verifies coinbase rewards

#include "blockchain.h"
#include "Block.h"
#include "transaction.h"
#include "Wallet.h"
#include "Config.h"
#include <iostream>
#include <iomanip>
#include <chrono>

// ANSI color codes for output
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"

// Test result tracking
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
    std::cout << BLUE << "ℹ️  INFO: " << msg << RESET << "\n";
}

void print_mining(const std::string& msg) {
    std::cout << YELLOW << "⛏️  MINING: " << msg << RESET << "\n";
}

// Helper: Get block type name
std::string getBlockTypeName(BlockType type) {
    switch (type) {
        case BlockType::POW_SHA256: return "POW_SHA256";
        case BlockType::POW_ETHASH: return "POW_ETHASH";
        case BlockType::POW_GXHASH: return "POW_GXHASH";
        case BlockType::POS: return "POS";
        default: return "UNKNOWN";
    }
}

// Helper: Print block info
void print_block_info(const Block& block) {
    std::cout << "  Index: " << block.getIndex() << "\n";
    std::cout << "  Type: " << getBlockTypeName(block.getBlockType()) << "\n";
    std::cout << "  Hash: " << block.getHash().substr(0, 16) << "..." << "\n";
    std::cout << "  Prev: " << block.getPreviousHash().substr(0, 16) << "..." << "\n";
    std::cout << "  Difficulty: " << block.getDifficulty() << "\n";
    std::cout << "  Nonce: " << block.getNonce() << "\n";
    std::cout << "  Transactions: " << block.getTransactions().size() << "\n";
    std::cout << "  Work Receipt: " << block.getWorkReceiptHash().substr(0, 16) << "..." << "\n";
}

// Helper: Get coinbase reward from block
double get_coinbase_reward(const Block& block) {
    for (const auto& tx : block.getTransactions()) {
        if (tx.isCoinbaseTransaction()) {
            double total_output = 0.0;
            for (const auto& output : tx.getOutputs()) {
                total_output += output.amount;
            }
            return total_output;
        }
    }
    return 0.0;
}

// Helper: Verify block is in blockchain
bool verify_block_in_chain(Blockchain& blockchain, uint32_t index) {
    try {
        Block block = blockchain.getBlock(index);
        return block.getIndex() == index;
    } catch (...) {
        return false;
    }
}

// Test 1: Mine with POW_SHA256
void test_mine_sha256(Blockchain& blockchain, Wallet& miner) {
    print_header("Test 1: Mining with POW_SHA256 (Bitcoin-style)");

    print_mining("Creating POW_SHA256 block...");
    Block lastBlock = blockchain.getLatestBlock();
    Block block(lastBlock.getIndex() + 1,
                lastBlock.getHash(),
                BlockType::POW_SHA256);

    block.setMinerAddress(miner.getAddress());
    block.setMinerPublicKey(miner.getPublicKey());

    // Set difficulty (low for testing)
    double difficulty = 2.0;  // 2 leading zeros

    print_mining("Mining block with difficulty " + std::to_string(difficulty) + "...");
    auto start = std::chrono::high_resolution_clock::now();

    block.mineBlock(difficulty);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    print_info("Mining completed in " + std::to_string(duration) + " ms");
    print_block_info(block);

    // Verify hash meets target
    std::string hash = block.getHash();
    size_t leading_zeros = 0;
    for (char c : hash) {
        if (c == '0') leading_zeros++;
        else break;
    }

    if (leading_zeros >= static_cast<size_t>(difficulty)) {
        print_success("Block hash meets difficulty target (" + std::to_string(leading_zeros) + " zeros)");
    } else {
        print_fail("Block hash doesn't meet difficulty (" + std::to_string(leading_zeros) + " < " + std::to_string(difficulty) + ")");
    }

    // Verify work receipt
    if (!block.getWorkReceiptHash().empty() && block.getWorkReceiptHash().length() == 64) {
        print_success("Work receipt generated");
    } else {
        print_fail("Work receipt missing or invalid");
    }

    // Add block to blockchain
    print_info("Adding block to blockchain...");
    uint32_t block_index = block.getIndex();
    bool added = blockchain.addBlock(block);

    if (added) {
        print_success("Block added to blockchain");
    } else {
        print_fail("Failed to add block to blockchain");
        return;
    }

    // Verify block is in chain
    if (verify_block_in_chain(blockchain, block_index)) {
        print_success("Block verified in blockchain at index " + std::to_string(block_index));
    } else {
        print_fail("Block not found in blockchain");
    }

    // Verify coinbase reward
    try {
        Block saved_block = blockchain.getBlock(block_index);
        double reward = get_coinbase_reward(saved_block);
        if (reward > 0) {
            print_success("Coinbase reward received: " + std::to_string(reward) + " GXC");
        } else {
            print_fail("No coinbase reward found");
        }

        // Verify miner received reward
        bool miner_in_output = false;
        for (const auto& tx : saved_block.getTransactions()) {
            if (tx.isCoinbaseTransaction()) {
                for (const auto& output : tx.getOutputs()) {
                    if (output.address == miner.getAddress()) {
                        miner_in_output = true;
                        print_success("Miner address found in coinbase outputs");
                        break;
                    }
                }
            }
        }
        if (!miner_in_output) {
            print_fail("Miner address not found in coinbase outputs");
        }
    } catch (...) {
        print_fail("Failed to retrieve block from blockchain");
    }
}

// Test 2: Mine with POW_ETHASH
void test_mine_ethash(Blockchain& blockchain, Wallet& miner) {
    print_header("Test 2: Mining with POW_ETHASH (Ethereum-style)");

    print_mining("Creating POW_ETHASH block...");
    Block lastBlock = blockchain.getLatestBlock();
    Block block(lastBlock.getIndex() + 1,
                lastBlock.getHash(),
                BlockType::POW_ETHASH);

    block.setMinerAddress(miner.getAddress());
    block.setMinerPublicKey(miner.getPublicKey());

    // Set difficulty (low for testing)
    double difficulty = 2.0;  // 2 leading zeros

    print_mining("Mining block with difficulty " + std::to_string(difficulty) + "...");
    auto start = std::chrono::high_resolution_clock::now();

    block.mineBlock(difficulty);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    print_info("Mining completed in " + std::to_string(duration) + " ms");
    print_block_info(block);

    // Verify hash meets target
    std::string hash = block.getHash();
    size_t leading_zeros = 0;
    for (char c : hash) {
        if (c == '0') leading_zeros++;
        else break;
    }

    if (leading_zeros >= static_cast<size_t>(difficulty)) {
        print_success("Block hash meets difficulty target (" + std::to_string(leading_zeros) + " zeros)");
    } else {
        print_fail("Block hash doesn't meet difficulty (" + std::to_string(leading_zeros) + " < " + std::to_string(difficulty) + ")");
    }

    // Verify work receipt
    if (!block.getWorkReceiptHash().empty() && block.getWorkReceiptHash().length() == 64) {
        print_success("Work receipt generated");
    } else {
        print_fail("Work receipt missing or invalid");
    }

    // Add block to blockchain
    print_info("Adding block to blockchain...");
    uint32_t block_index = block.getIndex();
    bool added = blockchain.addBlock(block);

    if (added) {
        print_success("Block added to blockchain");
    } else {
        print_fail("Failed to add block to blockchain");
        return;
    }

    // Verify block is in chain
    if (verify_block_in_chain(blockchain, block_index)) {
        print_success("Block verified in blockchain at index " + std::to_string(block_index));
    } else {
        print_fail("Block not found in blockchain");
    }

    // Verify coinbase reward
    try {
        Block saved_block = blockchain.getBlock(block_index);
        double reward = get_coinbase_reward(saved_block);
        if (reward > 0) {
            print_success("Coinbase reward received: " + std::to_string(reward) + " GXC");
        } else {
            print_fail("No coinbase reward found");
        }

        // Verify miner received reward
        bool miner_in_output = false;
        for (const auto& tx : saved_block.getTransactions()) {
            if (tx.isCoinbaseTransaction()) {
                for (const auto& output : tx.getOutputs()) {
                    if (output.address == miner.getAddress()) {
                        miner_in_output = true;
                        print_success("Miner address found in coinbase outputs");
                        break;
                    }
                }
            }
        }
        if (!miner_in_output) {
            print_fail("Miner address not found in coinbase outputs");
        }
    } catch (...) {
        print_fail("Failed to retrieve block from blockchain");
    }
}

// Test 3: Mine with POW_GXHASH
void test_mine_gxhash(Blockchain& blockchain, Wallet& miner) {
    print_header("Test 3: Mining with POW_GXHASH (ASIC-resistant)");

    print_mining("Creating POW_GXHASH block...");
    Block lastBlock = blockchain.getLatestBlock();
    Block block(lastBlock.getIndex() + 1,
                lastBlock.getHash(),
                BlockType::POW_GXHASH);

    block.setMinerAddress(miner.getAddress());
    block.setMinerPublicKey(miner.getPublicKey());

    // Set difficulty (low for testing - GXHash is slow)
    double difficulty = 2.0;  // 2 leading zeros

    print_mining("Mining block with difficulty " + std::to_string(difficulty) + "...");
    print_info("Note: GXHash is intentionally slow (memory-hard, ASIC-resistant)");
    auto start = std::chrono::high_resolution_clock::now();

    block.mineBlock(difficulty);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    print_info("Mining completed in " + std::to_string(duration) + " ms");
    print_block_info(block);

    // Verify hash meets target
    std::string hash = block.getHash();
    size_t leading_zeros = 0;
    for (char c : hash) {
        if (c == '0') leading_zeros++;
        else break;
    }

    if (leading_zeros >= static_cast<size_t>(difficulty)) {
        print_success("Block hash meets difficulty target (" + std::to_string(leading_zeros) + " zeros)");
    } else {
        print_fail("Block hash doesn't meet difficulty (" + std::to_string(leading_zeros) + " < " + std::to_string(difficulty) + ")");
    }

    // Verify work receipt
    if (!block.getWorkReceiptHash().empty() && block.getWorkReceiptHash().length() == 64) {
        print_success("Work receipt generated");
    } else {
        print_fail("Work receipt missing or invalid");
    }

    // Add block to blockchain
    print_info("Adding block to blockchain...");
    uint32_t block_index = block.getIndex();
    bool added = blockchain.addBlock(block);

    if (added) {
        print_success("Block added to blockchain");
    } else {
        print_fail("Failed to add block to blockchain");
        return;
    }

    // Verify block is in chain
    if (verify_block_in_chain(blockchain, block_index)) {
        print_success("Block verified in blockchain at index " + std::to_string(block_index));
    } else {
        print_fail("Block not found in blockchain");
    }

    // Verify coinbase reward
    try {
        Block saved_block = blockchain.getBlock(block_index);
        double reward = get_coinbase_reward(saved_block);
        if (reward > 0) {
            print_success("Coinbase reward received: " + std::to_string(reward) + " GXC");
        } else {
            print_fail("No coinbase reward found");
        }

        // Verify miner received reward
        bool miner_in_output = false;
        for (const auto& tx : saved_block.getTransactions()) {
            if (tx.isCoinbaseTransaction()) {
                for (const auto& output : tx.getOutputs()) {
                    if (output.address == miner.getAddress()) {
                        miner_in_output = true;
                        print_success("Miner address found in coinbase outputs");
                        break;
                    }
                }
            }
        }
        if (!miner_in_output) {
            print_fail("Miner address not found in coinbase outputs");
        }
    } catch (...) {
        print_fail("Failed to retrieve block from blockchain");
    }
}

// Test 4: Verify blockchain state
void test_blockchain_state(Blockchain& blockchain) {
    print_header("Test 4: Blockchain State Verification");

    size_t chain_length = blockchain.getHeight() + 1;  // height is 0-based, so +1 for count
    print_info("Total blocks in chain: " + std::to_string(chain_length));

    // Should have genesis + 3 mined blocks
    if (chain_length >= 4) {
        print_success("Blockchain has expected number of blocks (>= 4)");
    } else {
        print_fail("Blockchain missing blocks (expected >= 4, got " + std::to_string(chain_length) + ")");
    }

    // Verify all blocks are valid
    bool all_valid = true;
    for (size_t i = 0; i < chain_length; i++) {
        try {
            Block block = blockchain.getBlock(i);
            if (!blockchain.validateBlock(block)) {
                print_fail("Block " + std::to_string(i) + " validation failed");
                all_valid = false;
            }
        } catch (...) {
            print_fail("Block " + std::to_string(i) + " not found");
            all_valid = false;
        }
    }

    if (all_valid) {
        print_success("All blocks in chain are valid");
    }

    // Verify we have all 3 algorithm types
    bool has_sha256 = false;
    bool has_ethash = false;
    bool has_gxhash = false;

    for (size_t i = 0; i < chain_length; i++) {
        try {
            Block block = blockchain.getBlock(i);
            switch (block.getBlockType()) {
                case BlockType::POW_SHA256: has_sha256 = true; break;
                case BlockType::POW_ETHASH: has_ethash = true; break;
                case BlockType::POW_GXHASH: has_gxhash = true; break;
                default: break;
            }
        } catch (...) {
            // Skip blocks that can't be retrieved
        }
    }

    if (has_sha256) {
        print_success("Blockchain contains POW_SHA256 blocks");
    } else {
        print_fail("No POW_SHA256 blocks found");
    }

    if (has_ethash) {
        print_success("Blockchain contains POW_ETHASH blocks");
    } else {
        print_fail("No POW_ETHASH blocks found");
    }

    if (has_gxhash) {
        print_success("Blockchain contains POW_GXHASH blocks");
    } else {
        print_fail("No POW_GXHASH blocks found");
    }
}

int main() {
    print_header("GXC Mining Rewards Test Suite");
    std::cout << "Testing all 3 mining algorithms with reward validation\n";
    std::cout << "Algorithms: POW_SHA256, POW_ETHASH, POW_GXHASH\n";

    // Note: Using low difficulty for testing (hardcoded in test)

    // Create blockchain
    print_info("Initializing blockchain...");
    Blockchain blockchain;

    // Create miner wallet
    print_info("Creating miner wallet...");
    Wallet miner;
    std::cout << "Miner Address: " << miner.getAddress().substr(0, 20) << "...\n";

    // Run tests
    test_mine_sha256(blockchain, miner);
    test_mine_ethash(blockchain, miner);
    test_mine_gxhash(blockchain, miner);
    test_blockchain_state(blockchain);

    // Summary
    print_header("Test Summary");
    std::cout << "Tests Passed: " << GREEN << tests_passed << RESET << "\n";
    std::cout << "Tests Failed: " << RED << tests_failed << RESET << "\n";
    std::cout << "Total Tests:  " << (tests_passed + tests_failed) << "\n";

    if (tests_failed == 0) {
        std::cout << "\n" << GREEN << "✅ ALL TESTS PASSED!" << RESET << "\n";
        std::cout << GREEN << "All 3 mining algorithms work correctly and miners receive rewards." << RESET << "\n";
        return 0;
    } else {
        std::cout << "\n" << RED << "❌ SOME TESTS FAILED" << RESET << "\n";
        std::cout << RED << "Please review failed tests above." << RESET << "\n";
        return 1;
    }
}
