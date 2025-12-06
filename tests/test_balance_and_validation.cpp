/**
 * Test suite for balance deduction and transaction validation
 * 
 * These tests verify that:
 * 1. Sending coins deducts from sender balance
 * 2. Transactions are validated against UTXO set
 * 3. Staking requires real on-chain balance
 * 4. Double-spend protection works
 * 5. Coinbase rewards are spendable
 */

#include "../include/blockchain.h"
#include "../include/Wallet.h"
#include "../include/Config.h"
#include "../include/Logger.h"
#include \u003cassert.h\u003e
#include \u003ciostream\u003e

void test_coinbase_creates_spendable_utxo() {
    std::cout \u003c\u003c "Test 1: Coinbase creates spendable UTXO..." \u003c\u003c std::endl;
    
    Blockchain blockchain;
    blockchain.initialize();
    
    // Create a wallet
    Wallet minerWallet;
    std::string minerAddress = minerWallet.getAddress();
    
    // Mine a block (creates coinbase)
    Block block(1, blockchain.getLatestBlock().getHash(), minerAddress);
    block.setBlockType(BlockType::POW_SHA256);
    
    // Add block to chain
    assert(blockchain.addBlock(block) && "Failed to add block with coinbase");
    
    // Verify balance increased
    double balance = blockchain.getBalance(minerAddress);
    assert(balance \u003e 0 && "Coinbase reward not added to balance");
    
    // Verify UTXO exists
    const auto& utxoSet = blockchain.getUtxoSet();
    bool foundCoinbaseUtxo = false;
    for (const auto& [key, utxo] : utxoSet) {
        if (utxo.address == minerAddress) {
            foundCoinbaseUtxo = true;
            break;
        }
    }
    assert(foundCoinbaseUtxo && "Coinbase UTXO not found in UTXO set");
    
    std::cout \u003c\u003c "  ✅ PASSED: Coinbase creates spendable UTXO" \u003c\u003c std::endl;
}

void test_sending_deducts_balance() {
    std::cout \u003c\u003c "Test 2: Sending coins deducts sender balance..." \u003c\u003c std::endl;
    
    Blockchain blockchain;
    blockchain.initialize();
    
    // Create wallets
    Wallet senderWallet;
    Wallet receiverWallet;
    std::string senderAddress = senderWallet.getAddress();
    std::string receiverAddress = receiverWallet.getAddress();
    
    // Mine a block to sender
    Block block1(1, blockchain.getLatestBlock().getHash(), senderAddress);
    block1.setBlockType(BlockType::POW_SHA256);
    blockchain.addBlock(block1);
    
    double initialBalance = blockchain.getBalance(senderAddress);
    assert(initialBalance \u003e 0 && "Sender has no initial balance");
    
    // Create and send transaction
    double sendAmount = 10.0;
    double fee = 0.001;
    const auto& utxoSet = blockchain.getUtxoSet();
    Transaction tx = senderWallet.createTransaction(receiverAddress, sendAmount, utxoSet, fee);
    
    // Add transaction to mempool
    assert(blockchain.addTransaction(tx) && "Failed to add transaction to mempool");
    
    // Mine a block with the transaction
    Block block2(2, blockchain.getLatestBlock().getHash(), senderAddress);
    block2.setBlockType(BlockType::POW_SHA256);
    block2.addTransaction(tx);
    blockchain.addBlock(block2);
    
    // Verify sender balance decreased
    double finalBalance = blockchain.getBalance(senderAddress);
    double expectedDecrease = sendAmount + fee;
    
    // Account for new coinbase reward in block2
    double blockReward = 50.0; // Default block reward
    double expectedFinalBalance = initialBalance - expectedDecrease + blockReward;
    
    assert(std::abs(finalBalance - expectedFinalBalance) \u003c 0.01 && 
           "Sender balance not correctly deducted");
    
    // Verify receiver balance increased
    double receiverBalance = blockchain.getBalance(receiverAddress);
    assert(std::abs(receiverBalance - sendAmount) \u003c 0.01 && 
           "Receiver balance not correctly increased");
    
    std::cout \u003c\u003c "  ✅ PASSED: Sending deducts balance correctly" \u003c\u003c std::endl;
}

void test_insufficient_funds_rejected() {
    std::cout \u003c\u003c "Test 3: Transaction with insufficient funds rejected..." \u003c\u003c std::endl;
    
    Blockchain blockchain;
    blockchain.initialize();
    
    // Create wallets
    Wallet senderWallet;
    Wallet receiverWallet;
    std::string senderAddress = senderWallet.getAddress();
    std::string receiverAddress = receiverWallet.getAddress();
    
    // Don't mine any blocks - sender has 0 balance
    double balance = blockchain.getBalance(senderAddress);
    assert(balance == 0 && "Sender should have 0 balance");
    
    // Try to create transaction (should throw)
    bool exceptionThrown = false;
    try {
        const auto& utxoSet = blockchain.getUtxoSet();
        Transaction tx = senderWallet.createTransaction(receiverAddress, 10.0, utxoSet, 0.001);
    } catch (const std::runtime_error& e) {
        exceptionThrown = true;
        std::string errorMsg = e.what();
        assert(errorMsg.find("Insufficient funds") != std::string::npos && 
               "Wrong error message");
    }
    
    assert(exceptionThrown && "Transaction with insufficient funds should throw exception");
    
    std::cout \u003c\u003c "  ✅ PASSED: Insufficient funds rejected" \u003c\u003c std::endl;
}

void test_double_spend_rejected() {
    std::cout \u003c\u003c "Test 4: Double-spend attempt rejected..." \u003c\u003c std::endl;
    
    Blockchain blockchain;
    blockchain.initialize();
    
    // Create wallets
    Wallet senderWallet;
    Wallet receiver1Wallet;
    Wallet receiver2Wallet;
    std::string senderAddress = senderWallet.getAddress();
    std::string receiver1Address = receiver1Wallet.getAddress();
    std::string receiver2Address = receiver2Wallet.getAddress();
    
    // Mine a block to sender
    Block block1(1, blockchain.getLatestBlock().getHash(), senderAddress);
    block1.setBlockType(BlockType::POW_SHA256);
    blockchain.addBlock(block1);
    
    double balance = blockchain.getBalance(senderAddress);
    assert(balance \u003e 0 && "Sender has no balance");
    
    // Create first transaction
    const auto& utxoSet = blockchain.getUtxoSet();
    Transaction tx1 = senderWallet.createTransaction(receiver1Address, 10.0, utxoSet, 0.001);
    
    // Add first transaction to mempool
    assert(blockchain.addTransaction(tx1) && "Failed to add first transaction");
    
    // Try to create second transaction using same UTXOs (double-spend)
    // This should fail because wallet will see UTXOs are already spent in mempool
    bool secondTxFailed = false;
    try {
        // Get fresh UTXO set (but tx1 is in mempool, so balance check should fail)
        const auto& utxoSet2 = blockchain.getUtxoSet();
        Transaction tx2 = senderWallet.createTransaction(receiver2Address, 10.0, utxoSet2, 0.001);
        
        // If we got here, try to add to mempool (should fail validation)
        if (!blockchain.addTransaction(tx2)) {
            secondTxFailed = true;
        }
    } catch (const std::runtime_error& e) {
        // Expected: insufficient funds because first tx is pending
        secondTxFailed = true;
    }
    
    assert(secondTxFailed && "Double-spend should be rejected");
    
    std::cout \u003c\u003c "  ✅ PASSED: Double-spend rejected" \u003c\u003c std::endl;
}

void test_staking_requires_balance() {
    std::cout \u003c\u003c "Test 5: Staking requires real on-chain balance..." \u003c\u003c std::endl;
    
    Blockchain blockchain;
    blockchain.initialize();
    
    // Create wallet
    Wallet stakerWallet;
    std::string stakerAddress = stakerWallet.getAddress();
    
    // Register as validator (with 0 stake initially)
    Validator validator(stakerAddress, 0.0, 30);
    blockchain.registerValidator(validator);
    
    // Try to stake without any balance (should fail)
    bool stakeFailed = false;
    try {
        const auto& utxoSet = blockchain.getUtxoSet();
        Transaction stakeTx = stakerWallet.createStakeTransaction(100.0, utxoSet, 0.001);
    } catch (const std::runtime_error& e) {
        stakeFailed = true;
        std::string errorMsg = e.what();
        assert(errorMsg.find("Insufficient funds") != std::string::npos && 
               "Wrong error message for stake without balance");
    }
    
    assert(stakeFailed && "Staking without balance should fail");
    
    // Mine a block to get balance
    Block block1(1, blockchain.getLatestBlock().getHash(), stakerAddress);
    block1.setBlockType(BlockType::POW_SHA256);
    blockchain.addBlock(block1);
    
    double balance = blockchain.getBalance(stakerAddress);
    assert(balance \u003e= 100.0 && "Insufficient balance for staking");
    
    // Now staking should succeed
    const auto& utxoSet = blockchain.getUtxoSet();
    Transaction stakeTx = stakerWallet.createStakeTransaction(100.0, utxoSet, 0.001);
    assert(blockchain.addTransaction(stakeTx) && "Staking with sufficient balance should succeed");
    
    std::cout \u003c\u003c "  ✅ PASSED: Staking requires real balance" \u003c\u003c std::endl;
}

void test_stake_without_validator_registration_rejected() {
    std::cout \u003c\u003c "Test 6: Stake without validator registration rejected..." \u003c\u003c std::endl;
    
    Blockchain blockchain;
    blockchain.initialize();
    
    // Create wallet
    Wallet stakerWallet;
    std::string stakerAddress = stakerWallet.getAddress();
    
    // Mine a block to get balance
    Block block1(1, blockchain.getLatestBlock().getHash(), stakerAddress);
    block1.setBlockType(BlockType::POW_SHA256);
    blockchain.addBlock(block1);
    
    double balance = blockchain.getBalance(stakerAddress);
    assert(balance \u003e= 100.0 && "Insufficient balance");
    
    // Try to stake WITHOUT registering as validator first
    const auto& utxoSet = blockchain.getUtxoSet();
    Transaction stakeTx = stakerWallet.createStakeTransaction(100.0, utxoSet, 0.001);
    
    // Transaction creation succeeds, but validation should fail
    bool validationFailed = !blockchain.addTransaction(stakeTx);
    
    assert(validationFailed && "Stake without validator registration should be rejected");
    
    std::cout \u003c\u003c "  ✅ PASSED: Stake without validator registration rejected" \u003c\u003c std::endl;
}

int main() {
    std::cout \u003c\u003c "========================================" \u003c\u003c std::endl;
    std::cout \u003c\u003c "Balance and Validation Test Suite" \u003c\u003c std::endl;
    std::cout \u003c\u003c "========================================" \u003c\u003c std::endl;
    std::cout \u003c\u003c std::endl;
    
    try {
        test_coinbase_creates_spendable_utxo();
        test_sending_deducts_balance();
        test_insufficient_funds_rejected();
        test_double_spend_rejected();
        test_staking_requires_balance();
        test_stake_without_validator_registration_rejected();
        
        std::cout \u003c\u003c std::endl;
        std::cout \u003c\u003c "========================================" \u003c\u003c std::endl;
        std::cout \u003c\u003c "ALL TESTS PASSED ✅" \u003c\u003c std::endl;
        std::cout \u003c\u003c "========================================" \u003c\u003c std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr \u003c\u003c std::endl;
        std::cerr \u003c\u003c "❌ TEST FAILED: " \u003c\u003c e.what() \u003c\u003c std::endl;
        return 1;
    }
}
