/**
 * Simple API Test - Tests core functionality without RPC layer
 */

#include "include/blockchain.h"
#include "include/Wallet.h"
#include "include/Validator.h"
#include "include/Config.h"
#include <iostream>
#include <iomanip>

void print_section(const std::string& title) {
    std::cout << "\n" << std::string(70, '=') << "\n";
    std::cout << "  " << title << "\n";
    std::cout << std::string(70, '=') << "\n";
}

void print_test(const std::string& name, bool passed) {
    std::cout << (passed ? "✅" : "❌") << " " << name << "\n";
}

int main() {
    std::cout << "\n╔════════════════════════════════════════════════════════════╗\n";
    std::cout << "║         GXC API Simple Test Suite                         ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════╝\n";
    
    int totalTests = 0;
    int passedTests = 0;
    
    // Initialize testnet
    Config::set("testnet", "true");
    Config::set("network", "testnet");
    
    // TEST 1: Wallet Creation
    print_section("TEST 1: Wallet Creation & Address Generation");
    try {
        Wallet wallet1, wallet2;
        
        bool hasAddress1 = !wallet1.getAddress().empty();
        bool hasAddress2 = !wallet2.getAddress().empty();
        bool isTestnet1 = wallet1.getAddress().substr(0, 4) == "tGXC";
        bool isTestnet2 = wallet2.getAddress().substr(0, 4) == "tGXC";
        
        print_test("Wallet 1 created", hasAddress1);
        print_test("Wallet 2 created", hasAddress2);
        print_test("Wallet 1 testnet address", isTestnet1);
        print_test("Wallet 2 testnet address", isTestnet2);
        
        if (hasAddress1) {
            std::cout << "   Wallet 1: " << wallet1.getAddress().substr(0, 30) << "...\n";
        }
        if (hasAddress2) {
            std::cout << "   Wallet 2: " << wallet2.getAddress().substr(0, 30) << "...\n";
        }
        
        totalTests += 4;
        if (hasAddress1) passedTests++;
        if (hasAddress2) passedTests++;
        if (isTestnet1) passedTests++;
        if (isTestnet2) passedTests++;
        
    } catch (const std::exception& e) {
        std::cout << "❌ Error: " << e.what() << "\n";
        totalTests += 4;
    }
    
    // TEST 2: Transaction Creation
    print_section("TEST 2: Transaction Creation");
    try {
        Wallet wallet1, wallet2;
        
        // Create fake UTXO
        std::unordered_map<std::string, TransactionOutput> utxoSet;
        TransactionOutput utxo;
        utxo.address = wallet1.getAddress();
        utxo.amount = 200.0;
        std::string txHash = "0000000000000000000000000000000000000000000000000000000000000001";
        utxoSet[txHash + "_0"] = utxo;
        
        // Create transaction
        Transaction tx = wallet1.createTransaction(
            wallet2.getAddress(),
            50.0,
            utxoSet,
            0.001
        );
        
        bool hasInputs = !tx.getInputs().empty();
        bool hasOutputs = !tx.getOutputs().empty();
        bool hasHash = !tx.getHash().empty();
        bool validTrace = tx.verifyTraceabilityFormula();
        
        print_test("Transaction created", hasInputs && hasOutputs);
        print_test("Has inputs", hasInputs);
        print_test("Has outputs", hasOutputs);
        print_test("Has hash", hasHash);
        print_test("Traceability valid", validTrace);
        
        if (hasInputs && hasOutputs) {
            std::cout << "   Inputs: " << tx.getInputs().size() << "\n";
            std::cout << "   Outputs: " << tx.getOutputs().size() << "\n";
            std::cout << "   Total input: " << tx.getTotalInputAmount() << " GXC\n";
            std::cout << "   Total output: " << tx.getTotalOutputAmount() << " GXC\n";
            std::cout << "   Fee: " << tx.getFee() << " GXC\n";
            
            // Verify balance equation
            double input = tx.getTotalInputAmount();
            double output = tx.getTotalOutputAmount();
            double fee = tx.getFee();
            bool balanced = (std::abs(input - (output + fee)) < 0.00000001);
            print_test("Balance equation valid", balanced);
            
            totalTests++;
            if (balanced) passedTests++;
        }
        
        totalTests += 5;
        if (hasInputs && hasOutputs) passedTests++;
        if (hasInputs) passedTests++;
        if (hasOutputs) passedTests++;
        if (hasHash) passedTests++;
        if (validTrace) passedTests++;
        
    } catch (const std::exception& e) {
        std::cout << "❌ Error: " << e.what() << "\n";
        totalTests += 5;
    }
    
    // TEST 3: Stake Transaction
    print_section("TEST 3: Stake Transaction Creation");
    try {
        Wallet wallet;
        
        // Create fake UTXO
        std::unordered_map<std::string, TransactionOutput> utxoSet;
        TransactionOutput utxo;
        utxo.address = wallet.getAddress();
        utxo.amount = 200.0;
        std::string txHash = "0000000000000000000000000000000000000000000000000000000000000002";
        utxoSet[txHash + "_0"] = utxo;
        
        // Create stake transaction
        Transaction stakeTx = wallet.createStakeTransaction(100.0, utxoSet, 0.001);
        
        bool isStake = (stakeTx.getType() == TransactionType::STAKE);
        bool hasInputs = !stakeTx.getInputs().empty();
        bool validTrace = stakeTx.verifyTraceabilityFormula();
        
        print_test("Stake transaction created", isStake);
        print_test("Type is STAKE", isStake);
        print_test("Has inputs", hasInputs);
        print_test("Traceability valid", validTrace);
        
        if (hasInputs) {
            double totalInput = stakeTx.getTotalInputAmount();
            double totalOutput = stakeTx.getTotalOutputAmount();
            double fee = stakeTx.getFee();
            double staked = totalInput - totalOutput - fee;
            
            std::cout << "   Total input: " << totalInput << " GXC\n";
            std::cout << "   Total output: " << totalOutput << " GXC\n";
            std::cout << "   Fee: " << fee << " GXC\n";
            std::cout << "   Staked: " << staked << " GXC\n";
            
            bool correctAmount = (std::abs(staked - 100.0) < 0.001);
            print_test("Staked amount correct (100 GXC)", correctAmount);
            
            totalTests++;
            if (correctAmount) passedTests++;
        }
        
        totalTests += 4;
        if (isStake) passedTests++;
        if (isStake) passedTests++;
        if (hasInputs) passedTests++;
        if (validTrace) passedTests++;
        
    } catch (const std::exception& e) {
        std::cout << "❌ Error: " << e.what() << "\n";
        totalTests += 4;
    }
    
    // TEST 4: Unstake Transaction
    print_section("TEST 4: Unstake Transaction Creation");
    try {
        Wallet wallet;
        
        // Create unstake transaction
        Transaction unstakeTx = wallet.createUnstakeTransaction(50.0, 0.0);
        
        bool isUnstake = (unstakeTx.getType() == TransactionType::UNSTAKE);
        bool hasOutputs = !unstakeTx.getOutputs().empty();
        
        print_test("Unstake transaction created", isUnstake);
        print_test("Type is UNSTAKE", isUnstake);
        print_test("Has outputs", hasOutputs);
        
        if (hasOutputs) {
            std::cout << "   Outputs: " << unstakeTx.getOutputs().size() << "\n";
            std::cout << "   Amount: " << unstakeTx.getOutputs()[0].amount << " GXC\n";
            std::cout << "   To: " << unstakeTx.getOutputs()[0].address.substr(0, 30) << "...\n";
            
            bool correctAmount = (std::abs(unstakeTx.getOutputs()[0].amount - 50.0) < 0.001);
            print_test("Unstake amount correct (50 GXC)", correctAmount);
            
            totalTests++;
            if (correctAmount) passedTests++;
        }
        
        totalTests += 3;
        if (isUnstake) passedTests++;
        if (isUnstake) passedTests++;
        if (hasOutputs) passedTests++;
        
    } catch (const std::exception& e) {
        std::cout << "❌ Error: " << e.what() << "\n";
        totalTests += 3;
    }
    
    // TEST 5: Traceability Formula
    print_section("TEST 5: Traceability Formula Verification");
    try {
        Wallet wallet1, wallet2;
        
        // Create fake UTXO
        std::unordered_map<std::string, TransactionOutput> utxoSet;
        TransactionOutput utxo;
        utxo.address = wallet1.getAddress();
        utxo.amount = 150.5;  // Test with decimal
        std::string txHash = "0000000000000000000000000000000000000000000000000000000000000003";
        utxoSet[txHash + "_0"] = utxo;
        
        // Create transaction
        Transaction tx = wallet1.createTransaction(wallet2.getAddress(), 30.0, utxoSet, 0.001);
        
        // Verify traceability components
        bool hashMatch = (tx.getInputs()[0].txHash == tx.getPrevTxHash());
        bool amountMatch = (std::abs(tx.getInputs()[0].amount - tx.getReferencedAmount()) < 0.00000001);
        bool formulaValid = tx.verifyTraceabilityFormula();
        
        print_test("Hash matching", hashMatch);
        print_test("Amount matching", amountMatch);
        print_test("Formula valid", formulaValid);
        
        std::cout << "   PrevTxHash: " << tx.getPrevTxHash().substr(0, 16) << "...\n";
        std::cout << "   Inputs[0].txHash: " << tx.getInputs()[0].txHash.substr(0, 16) << "...\n";
        std::cout << "   Inputs[0].amount: " << tx.getInputs()[0].amount << " GXC\n";
        std::cout << "   ReferencedAmount: " << tx.getReferencedAmount() << " GXC\n";
        
        totalTests += 3;
        if (hashMatch) passedTests++;
        if (amountMatch) passedTests++;
        if (formulaValid) passedTests++;
        
    } catch (const std::exception& e) {
        std::cout << "❌ Error: " << e.what() << "\n";
        totalTests += 3;
    }
    
    // TEST 6: Validator Creation
    print_section("TEST 6: Validator Creation");
    try {
        std::string address = "tGXC1234567890abcdef1234567890abcdef1234567890";
        double stakeAmount = 100.0;
        int stakingDays = 30;
        
        Validator validator(address, stakeAmount, stakingDays);
        
        bool hasAddress = (validator.getAddress() == address);
        bool hasStake = (validator.getStakeAmount() == stakeAmount);
        bool hasDays = (validator.getStakingDays() == stakingDays);
        bool hasWeightedStake = (validator.getWeightedStake() > 0);
        
        print_test("Validator created", hasAddress);
        print_test("Address correct", hasAddress);
        print_test("Stake amount correct", hasStake);
        print_test("Staking days correct", hasDays);
        print_test("Weighted stake calculated", hasWeightedStake);
        
        std::cout << "   Address: " << validator.getAddress().substr(0, 30) << "...\n";
        std::cout << "   Stake: " << validator.getStakeAmount() << " GXC\n";
        std::cout << "   Days: " << validator.getStakingDays() << "\n";
        std::cout << "   Weighted Stake: " << validator.getWeightedStake() << "\n";
        std::cout << "   Time Weight: " << validator.getTimeWeight() << "\n";
        
        totalTests += 5;
        if (hasAddress) passedTests++;
        if (hasAddress) passedTests++;
        if (hasStake) passedTests++;
        if (hasDays) passedTests++;
        if (hasWeightedStake) passedTests++;
        
    } catch (const std::exception& e) {
        std::cout << "❌ Error: " << e.what() << "\n";
        totalTests += 5;
    }
    
    // TEST 7: Blockchain Creation
    print_section("TEST 7: Blockchain Initialization");
    try {
        Blockchain blockchain;
        
        int height = blockchain.getHeight();
        bool hasGenesis = (height >= 0);
        
        print_test("Blockchain created", hasGenesis);
        print_test("Has genesis block", hasGenesis);
        
        std::cout << "   Height: " << height << "\n";
        
        totalTests += 2;
        if (hasGenesis) passedTests++;
        if (hasGenesis) passedTests++;
        
    } catch (const std::exception& e) {
        std::cout << "❌ Error: " << e.what() << "\n";
        totalTests += 2;
    }
    
    // Print summary
    print_section("TEST SUMMARY");
    std::cout << "Total Tests: " << totalTests << "\n";
    std::cout << "Passed: " << passedTests << "\n";
    std::cout << "Failed: " << (totalTests - passedTests) << "\n";
    std::cout << "Success Rate: " << std::fixed << std::setprecision(1) 
              << (100.0 * passedTests / totalTests) << "%\n";
    
    if (passedTests == totalTests) {
        std::cout << "\n✅ ALL API TESTS PASSED!\n";
        std::cout << "\nAPIs Ready for Developers:\n";
        std::cout << "  ✅ Wallet creation\n";
        std::cout << "  ✅ Transaction creation\n";
        std::cout << "  ✅ Stake transactions\n";
        std::cout << "  ✅ Unstake transactions\n";
        std::cout << "  ✅ Traceability verification\n";
        std::cout << "  ✅ Validator creation\n";
        std::cout << "  ✅ Blockchain initialization\n";
        return 0;
    } else {
        std::cout << "\n❌ SOME TESTS FAILED\n";
        return 1;
    }
}
