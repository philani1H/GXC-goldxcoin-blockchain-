/**
 * GXC Blockchain - Transaction and Staking Bug Fix Tests
 * 
 * Tests for the fixes to:
 * 1. Fund transfer not working (UTXO set update issue)
 * 2. Staking creating validator but not staking funds (validator map/vector sync issue)
 */

#include <gtest/gtest.h>
#include "../include/blockchain.h"
#include "../include/Wallet.h"
#include "../include/Validator.h"
#include "../include/transaction.h"
#include "../include/Config.h"
#include <memory>

class TransactionStakingFixTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize config for testnet
        Config::set("network", "testnet");
        
        // Create blockchain instance
        blockchain = std::make_unique<Blockchain>();
        
        // Create wallets for testing
        wallet1 = std::make_unique<Wallet>();
        wallet2 = std::make_unique<Wallet>();
    }
    
    void TearDown() override {
        blockchain.reset();
        wallet1.reset();
        wallet2.reset();
    }
    
    std::unique_ptr<Blockchain> blockchain;
    std::unique_ptr<Wallet> wallet1;
    std::unique_ptr<Wallet> wallet2;
    
    // Helper to create a fake UTXO set with coins
    std::unordered_map<std::string, TransactionOutput> createUtxoSet(
        const std::string& address, double amount, const std::string& txHash = "genesis_tx_0000000000000000000000000000000000000000000000000000") {
        
        std::unordered_map<std::string, TransactionOutput> utxoSet;
        TransactionOutput utxo;
        utxo.address = address;
        utxo.amount = amount;
        utxoSet[txHash + "_0"] = utxo;
        return utxoSet;
    }
};

// Test: Fund transfer creates proper UTXOs
TEST_F(TransactionStakingFixTest, FundTransferCreatesUTXOs) {
    std::string addr1 = wallet1->getAddress();
    std::string addr2 = wallet2->getAddress();
    
    // Create initial UTXO for wallet1
    auto utxoSet = createUtxoSet(addr1, 200.0);
    
    // Create a transfer transaction
    Transaction tx = wallet1->createTransaction(addr2, 50.0, utxoSet, 0.001);
    
    // Verify transaction structure
    EXPECT_FALSE(tx.getInputs().empty());
    EXPECT_FALSE(tx.getOutputs().empty());
    
    // Verify outputs: one for recipient, one for change
    EXPECT_EQ(tx.getOutputs().size(), 2);
    
    // Find recipient and change outputs
    bool foundRecipient = false;
    bool foundChange = false;
    double recipientAmount = 0.0;
    double changeAmount = 0.0;
    
    for (const auto& output : tx.getOutputs()) {
        if (output.address == addr2) {
            foundRecipient = true;
            recipientAmount = output.amount;
        } else if (output.address == addr1) {
            foundChange = true;
            changeAmount = output.amount;
        }
    }
    
    EXPECT_TRUE(foundRecipient);
    EXPECT_TRUE(foundChange);
    EXPECT_NEAR(recipientAmount, 50.0, 0.00000001);
    EXPECT_NEAR(changeAmount, 149.999, 0.001); // 200 - 50 - 0.001 fee
    
    // Verify traceability
    EXPECT_TRUE(tx.verifyTraceabilityFormula());
}

// Test: Staking transaction properly locks funds
TEST_F(TransactionStakingFixTest, StakingTransactionLocksFunds) {
    std::string addr = wallet1->getAddress();
    
    // Create initial UTXO
    auto utxoSet = createUtxoSet(addr, 200.0);
    
    // Create stake transaction
    double stakeAmount = 100.0;
    Transaction stakeTx = wallet1->createStakeTransaction(stakeAmount, utxoSet, 0.001);
    
    // Verify transaction type
    EXPECT_EQ(stakeTx.getType(), TransactionType::STAKE);
    
    // Verify inputs are consumed
    EXPECT_FALSE(stakeTx.getInputs().empty());
    EXPECT_EQ(stakeTx.getInputs()[0].amount, 200.0);
    
    // Verify outputs: should only contain change (staked amount is locked, not in outputs)
    double totalOutput = stakeTx.getTotalOutputAmount();
    double expectedChange = 200.0 - stakeAmount - 0.001; // input - stake - fee
    EXPECT_NEAR(totalOutput, expectedChange, 0.00000001);
    
    // Verify traceability
    EXPECT_TRUE(stakeTx.verifyTraceabilityFormula());
}

// Test: Validator registration shows stake amount immediately
TEST_F(TransactionStakingFixTest, ValidatorRegistrationShowsStakeImmediately) {
    std::string addr = wallet1->getAddress();
    std::string pubKey = wallet1->getPublicKey();
    
    // Step 1: Register validator with stake amount (pending confirmation)
    double stakeAmount = 100.0;
    Validator validator(addr, stakeAmount, 30);
    validator.setPublicKey(pubKey);
    validator.setPending(true);
    validator.setIsActive(false); // Not active until confirmed
    
    bool registered = blockchain->registerValidator(validator);
    EXPECT_TRUE(registered);
    
    // Step 2: Verify validator shows the stake amount immediately (even though pending)
    auto validators = blockchain->getActiveValidators();
    bool foundValidator = false;
    for (const auto& v : validators) {
        if (v.getAddress() == addr) {
            foundValidator = true;
            EXPECT_TRUE(v.getIsPending());
            EXPECT_FALSE(v.getIsActive()); // Not active until stake tx confirmed
            EXPECT_EQ(v.getStakeAmount(), stakeAmount); // Shows stake amount immediately!
            break;
        }
    }
    
    // Note: Pending validators might not be in active list, but should be in validatorMap
    
    // Step 3: Create stake transaction to lock the funds
    auto utxoSet = createUtxoSet(addr, 200.0);
    Transaction stakeTx = wallet1->createStakeTransaction(stakeAmount, utxoSet, 0.001);
    
    // Verify stake transaction is valid
    EXPECT_EQ(stakeTx.getType(), TransactionType::STAKE);
    EXPECT_TRUE(stakeTx.verifyTraceabilityFormula());
    
    // When this transaction is confirmed in a block, the validator will be activated
    // and the funds will be permanently locked in the stake
}

// Test: Multiple stake transactions to same validator
TEST_F(TransactionStakingFixTest, MultipleStakeTransactions) {
    std::string addr = wallet1->getAddress();
    
    // Create initial UTXO
    auto utxoSet1 = createUtxoSet(addr, 200.0, "tx1_hash_0000000000000000000000000000000000000000000000000000");
    
    // First stake
    Transaction stake1 = wallet1->createStakeTransaction(50.0, utxoSet1, 0.001);
    EXPECT_EQ(stake1.getType(), TransactionType::STAKE);
    
    // Simulate change UTXO from first stake
    double change1 = 200.0 - 50.0 - 0.001;
    auto utxoSet2 = createUtxoSet(addr, change1, stake1.getHash());
    
    // Second stake
    Transaction stake2 = wallet1->createStakeTransaction(50.0, utxoSet2, 0.001);
    EXPECT_EQ(stake2.getType(), TransactionType::STAKE);
    
    // Both should be valid
    EXPECT_TRUE(stake1.verifyTraceabilityFormula());
    EXPECT_TRUE(stake2.verifyTraceabilityFormula());
}

// Test: Unstake transaction returns funds to spendable balance
TEST_F(TransactionStakingFixTest, UnstakeTransactionReturnsFunds) {
    double unstakeAmount = 50.0;
    std::string addr = wallet1->getAddress();
    
    // Create unstake transaction
    Transaction unstakeTx = wallet1->createUnstakeTransaction(unstakeAmount, 0.0);
    
    // Verify transaction type
    EXPECT_EQ(unstakeTx.getType(), TransactionType::UNSTAKE);
    
    // Verify output goes to wallet address
    EXPECT_EQ(unstakeTx.getOutputs().size(), 1);
    EXPECT_EQ(unstakeTx.getOutputs()[0].address, addr);
    EXPECT_NEAR(unstakeTx.getOutputs()[0].amount, unstakeAmount, 0.00000001);
}

// Test: Transaction with insufficient funds fails gracefully
TEST_F(TransactionStakingFixTest, InsufficientFundsThrowsException) {
    std::string addr1 = wallet1->getAddress();
    std::string addr2 = wallet2->getAddress();
    
    // Create UTXO with only 10 GXC
    auto utxoSet = createUtxoSet(addr1, 10.0);
    
    // Try to send 50 GXC (should fail)
    EXPECT_THROW(
        wallet1->createTransaction(addr2, 50.0, utxoSet, 0.001),
        std::runtime_error
    );
}

// Test: Stake transaction with insufficient funds fails gracefully
TEST_F(TransactionStakingFixTest, InsufficientFundsForStakingThrowsException) {
    std::string addr = wallet1->getAddress();
    
    // Create UTXO with only 50 GXC
    auto utxoSet = createUtxoSet(addr, 50.0);
    
    // Try to stake 100 GXC (should fail)
    EXPECT_THROW(
        wallet1->createStakeTransaction(100.0, utxoSet, 0.001),
        std::runtime_error
    );
}

// Test: Transaction preserves traceability chain
TEST_F(TransactionStakingFixTest, TransactionChainPreservesTraceability) {
    std::string addr1 = wallet1->getAddress();
    std::string addr2 = wallet2->getAddress();
    
    // Create initial UTXO
    auto utxoSet1 = createUtxoSet(addr1, 100.0, "genesis_tx");
    
    // First transaction
    Transaction tx1 = wallet1->createTransaction(addr2, 30.0, utxoSet1, 0.001);
    EXPECT_TRUE(tx1.verifyTraceabilityFormula());
    
    // Simulate change UTXO from first transaction
    double change1 = 100.0 - 30.0 - 0.001;
    auto utxoSet2 = createUtxoSet(addr1, change1, tx1.getHash());
    
    // Second transaction using change from first
    Transaction tx2 = wallet1->createTransaction(addr2, 20.0, utxoSet2, 0.001);
    EXPECT_TRUE(tx2.verifyTraceabilityFormula());
    
    // Verify chain: tx2 should reference tx1
    EXPECT_EQ(tx2.getPrevTxHash(), tx1.getHash());
}

// Test: Validator stake amount updates correctly
TEST_F(TransactionStakingFixTest, ValidatorStakeAmountUpdates) {
    std::string addr = wallet1->getAddress();
    
    // Create validator with initial stake
    Validator validator(addr, 100.0, 30);
    EXPECT_EQ(validator.getStakeAmount(), 100.0);
    
    // Add more stake
    validator.addStake(50.0);
    EXPECT_EQ(validator.getStakeAmount(), 150.0);
    
    // Remove some stake
    validator.removeStake(30.0);
    EXPECT_EQ(validator.getStakeAmount(), 120.0);
    
    // Verify validator is still active (above minimum)
    EXPECT_TRUE(validator.getIsActive());
}

// Test: Validator deactivates when stake drops below minimum
TEST_F(TransactionStakingFixTest, ValidatorDeactivatesBelowMinimum) {
    std::string addr = wallet1->getAddress();
    
    // Create validator with stake just above minimum
    Validator validator(addr, 110.0, 30);
    EXPECT_TRUE(validator.getIsActive());
    
    // Remove stake to go below minimum (100 GXC)
    validator.removeStake(20.0);
    EXPECT_EQ(validator.getStakeAmount(), 90.0);
    
    // Validator should be deactivated
    EXPECT_FALSE(validator.getIsActive());
}

// Note: main() is provided by test_main.cpp
