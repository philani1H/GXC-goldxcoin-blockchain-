/**
 * GXC Blockchain - Staking Unit Tests
 * 
 * Tests for:
 * - STAKE transaction creation
 * - UNSTAKE transaction creation
 * - Traceability formula enforcement
 * - UTXO management during staking
 * - Validator selection (PoS)
 */

#include <gtest/gtest.h>
#include "../include/blockchain.h"
#include "../include/Wallet.h"
#include "../include/Validator.h"
#include "../include/transaction.h"
#include <cmath>

class StakingTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a fresh wallet for testing
        wallet = std::make_unique<Wallet>();
    }
    
    void TearDown() override {
        wallet.reset();
    }
    
    std::unique_ptr<Wallet> wallet;
    
    // Helper to create a fake UTXO set with coins
    std::unordered_map<std::string, TransactionOutput> createUtxoSet(
        const std::string& address, double amount, const std::string& txHash = "genesis_tx_hash") {
        
        std::unordered_map<std::string, TransactionOutput> utxoSet;
        TransactionOutput utxo;
        utxo.address = address;
        utxo.amount = amount;
        utxoSet[txHash + "_0"] = utxo;
        return utxoSet;
    }
};

// Test: Validator creation with minimum stake
TEST_F(StakingTest, ValidatorMinimumStake) {
    Validator v("GXCtest123", 100.0, 30);
    
    EXPECT_TRUE(v.hasMinimumStake());
    EXPECT_TRUE(v.isStakingPeriodValid());
    EXPECT_TRUE(v.getIsActive());
    EXPECT_EQ(v.getStakeAmount(), 100.0);
}

// Test: Validator below minimum stake
TEST_F(StakingTest, ValidatorBelowMinimumStake) {
    Validator v("GXCtest123", 50.0, 30); // Below 100 GXC minimum
    
    EXPECT_FALSE(v.hasMinimumStake());
}

// Test: Validator staking period validation
TEST_F(StakingTest, ValidatorStakingPeriod) {
    // Valid period (14-365 days)
    Validator v1("GXCtest1", 100.0, 30);
    EXPECT_TRUE(v1.isStakingPeriodValid());
    
    // Too short (< 14 days)
    Validator v2("GXCtest2", 100.0, 7);
    EXPECT_FALSE(v2.isStakingPeriodValid());
    
    // Too long (> 365 days)
    Validator v3("GXCtest3", 100.0, 500);
    EXPECT_FALSE(v3.isStakingPeriodValid());
}

// Test: Validator removeStake
TEST_F(StakingTest, ValidatorRemoveStake) {
    Validator v("GXCtest123", 200.0, 30);
    
    // Remove 50 GXC
    v.removeStake(50.0);
    EXPECT_EQ(v.getStakeAmount(), 150.0);
    EXPECT_TRUE(v.getIsActive());
    
    // Remove more to go below minimum
    v.removeStake(100.0);
    EXPECT_EQ(v.getStakeAmount(), 50.0);
    EXPECT_FALSE(v.getIsActive()); // Should be deactivated
}

// Test: Weighted stake calculation
TEST_F(StakingTest, WeightedStakeCalculation) {
    Validator v("GXCtest123", 100.0, 365); // Full year
    
    double weighted = v.getWeightedStake();
    EXPECT_GT(weighted, 0.0);
    
    // With beta = 0.5, time weight for 365 days = (365/365)^0.5 = 1.0
    double expectedWeight = 100.0 * 1.0; // stake * time_weight
    EXPECT_NEAR(weighted, expectedWeight, 0.01);
}

// Test: Transaction traceability formula
TEST_F(StakingTest, TraceabilityFormula) {
    // Create a transaction that follows the traceability formula
    std::vector<TransactionInput> inputs;
    TransactionInput input;
    input.txHash = "prev_tx_hash_123";
    input.outputIndex = 0;
    input.amount = 100.0;
    inputs.push_back(input);
    
    std::vector<TransactionOutput> outputs;
    TransactionOutput output;
    output.address = "recipient_address";
    output.amount = 99.0;
    outputs.push_back(output);
    
    // Create transaction with correct prevTxHash (should match inputs[0].txHash)
    Transaction tx(inputs, outputs, "prev_tx_hash_123");
    tx.setReferencedAmount(100.0);
    
    // Verify traceability formula
    // Ti.Inputs[0].txHash == Ti.PrevTxHash
    EXPECT_EQ(tx.getInputs()[0].txHash, tx.getPrevTxHash());
    
    // Ti.Inputs[0].amount == Ti.ReferencedAmount
    EXPECT_NEAR(tx.getInputs()[0].amount, tx.getReferencedAmount(), 0.00000001);
    
    // Full traceability validation
    EXPECT_TRUE(tx.verifyTraceabilityFormula());
}

// Test: Transaction traceability failure
TEST_F(StakingTest, TraceabilityFormulaFailure) {
    std::vector<TransactionInput> inputs;
    TransactionInput input;
    input.txHash = "correct_hash";
    input.outputIndex = 0;
    input.amount = 100.0;
    inputs.push_back(input);
    
    std::vector<TransactionOutput> outputs;
    TransactionOutput output;
    output.address = "recipient";
    output.amount = 99.0;
    outputs.push_back(output);
    
    // Create transaction with WRONG prevTxHash
    Transaction tx(inputs, outputs, "wrong_hash"); // Mismatch!
    tx.setReferencedAmount(100.0);
    
    // Traceability should fail
    EXPECT_FALSE(tx.verifyTraceabilityFormula());
}

// Test: Wallet createTransaction follows traceability
TEST_F(StakingTest, WalletCreateTransactionTraceability) {
    std::string address = wallet->getAddress();
    auto utxoSet = createUtxoSet(address, 200.0, "genesis_tx");
    
    try {
        Transaction tx = wallet->createTransaction("recipient_addr", 50.0, utxoSet, 0.001);
        
        // Check traceability formula
        EXPECT_FALSE(tx.getInputs().empty());
        
        // prevTxHash should match first input's txHash
        EXPECT_EQ(tx.getPrevTxHash(), tx.getInputs()[0].txHash);
        
        // referencedAmount should match first input's amount
        EXPECT_NEAR(tx.getReferencedAmount(), tx.getInputs()[0].amount, 0.00000001);
        
        // Transaction should be valid
        EXPECT_TRUE(tx.verifyTraceabilityFormula());
        
        // Check amounts
        double totalOutput = tx.getTotalOutputAmount();
        double fee = tx.getFee();
        EXPECT_NEAR(totalOutput + fee, 200.0, 0.00000001); // inputs = outputs + fee
        
    } catch (const std::exception& e) {
        FAIL() << "Transaction creation failed: " << e.what();
    }
}

// Test: STAKE transaction creation
TEST_F(StakingTest, CreateStakeTransaction) {
    std::string address = wallet->getAddress();
    auto utxoSet = createUtxoSet(address, 200.0, "genesis_tx");
    
    try {
        double stakeAmount = 100.0;
        double fee = 0.001;
        
        Transaction tx = wallet->createStakeTransaction(stakeAmount, utxoSet, fee);
        
        // Check transaction type
        EXPECT_EQ(tx.getType(), TransactionType::STAKE);
        
        // Check traceability
        EXPECT_TRUE(tx.verifyTraceabilityFormula());
        
        // Check that staked amount is NOT in outputs (it's locked)
        // Only change should be in outputs
        double totalOutput = tx.getTotalOutputAmount();
        double expectedChange = 200.0 - stakeAmount - fee;
        EXPECT_NEAR(totalOutput, expectedChange, 0.00000001);
        
    } catch (const std::exception& e) {
        FAIL() << "Stake transaction creation failed: " << e.what();
    }
}

// Test: STAKE transaction with insufficient funds
TEST_F(StakingTest, CreateStakeTransactionInsufficientFunds) {
    std::string address = wallet->getAddress();
    auto utxoSet = createUtxoSet(address, 50.0, "genesis_tx"); // Only 50 GXC
    
    // Try to stake 100 GXC (should fail)
    EXPECT_THROW(
        wallet->createStakeTransaction(100.0, utxoSet, 0.001),
        std::runtime_error
    );
}

// Test: UNSTAKE transaction creation
TEST_F(StakingTest, CreateUnstakeTransaction) {
    double unstakeAmount = 50.0;
    
    Transaction tx = wallet->createUnstakeTransaction(unstakeAmount, 0.0);
    
    // Check transaction type
    EXPECT_EQ(tx.getType(), TransactionType::UNSTAKE);
    
    // Check output amount equals unstake amount
    EXPECT_EQ(tx.getOutputs().size(), 1);
    EXPECT_NEAR(tx.getOutputs()[0].amount, unstakeAmount, 0.00000001);
    
    // Output should go to wallet address
    EXPECT_EQ(tx.getOutputs()[0].address, wallet->getAddress());
}

// Test: Input-output balance
TEST_F(StakingTest, InputOutputBalance) {
    std::string address = wallet->getAddress();
    auto utxoSet = createUtxoSet(address, 100.0, "genesis_tx");
    
    double sendAmount = 30.0;
    double fee = 0.001;
    
    Transaction tx = wallet->createTransaction("recipient", sendAmount, utxoSet, fee);
    
    double inputTotal = tx.getTotalInputAmount();
    double outputTotal = tx.getTotalOutputAmount();
    
    // Inputs should equal outputs + fee
    EXPECT_NEAR(inputTotal, outputTotal + fee, 0.00000001);
    
    // No coin creation (outputs can't exceed inputs)
    EXPECT_LE(outputTotal, inputTotal);
}

// Test: Coinbase transaction is valid
TEST_F(StakingTest, CoinbaseTransaction) {
    Transaction coinbase("miner_address", 12.5);
    
    EXPECT_TRUE(coinbase.isCoinbaseTransaction());
    EXPECT_TRUE(coinbase.getInputs().empty());
    EXPECT_EQ(coinbase.getOutputs().size(), 1);
    EXPECT_EQ(coinbase.getOutputs()[0].amount, 12.5);
    
    // Coinbase should pass validation
    EXPECT_TRUE(coinbase.verifyTraceabilityFormula()); // Exempt from traceability
}

// Test: ValidatorSelector weighted selection
TEST_F(StakingTest, ValidatorSelector) {
    ValidatorSelector selector;
    
    // Add validators with different stakes
    Validator v1("addr1", 100.0, 30);
    Validator v2("addr2", 200.0, 30);
    Validator v3("addr3", 300.0, 30);
    
    selector.addValidator(v1);
    selector.addValidator(v2);
    selector.addValidator(v3);
    
    // Select a validator
    Validator selected = selector.selectValidator();
    
    // Should return one of the validators
    EXPECT_TRUE(selected.getAddress() == "addr1" || 
                selected.getAddress() == "addr2" || 
                selected.getAddress() == "addr3");
    
    // Run multiple selections to verify weighting
    int counts[3] = {0, 0, 0};
    for (int i = 0; i < 1000; i++) {
        Validator v = selector.selectValidator();
        if (v.getAddress() == "addr1") counts[0]++;
        else if (v.getAddress() == "addr2") counts[1]++;
        else if (v.getAddress() == "addr3") counts[2]++;
    }
    
    // Higher stake should be selected more often
    // addr3 (300) should be selected more than addr1 (100)
    EXPECT_GT(counts[2], counts[0]);
}

// Main function to run tests
int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
