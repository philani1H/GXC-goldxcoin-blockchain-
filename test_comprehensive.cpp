/**
 * Comprehensive test for GXC blockchain fixes
 * Tests fund transfer, staking, and traceability
 */

#include "include/blockchain.h"
#include "include/Wallet.h"
#include "include/Validator.h"
#include "include/transaction.h"
#include "include/Config.h"
#include <iostream>
#include <iomanip>
#include <cmath>

void print_section(const std::string& title) {
    std::cout << "\n" << std::string(70, '=') << "\n";
    std::cout << "  " << title << "\n";
    std::cout << std::string(70, '=') << "\n";
}

void print_success(const std::string& message) {
    std::cout << "✅ " << message << "\n";
}

void print_error(const std::string& message) {
    std::cout << "❌ " << message << "\n";
}

void print_info(const std::string& message) {
    std::cout << "ℹ️  " << message << "\n";
}

bool test_traceability() {
    print_section("TRACEABILITY FORMULA TEST");
    
    print_info("GXC Traceability Formula:");
    std::cout << "   Ti.Inputs[0].txHash == Ti.PrevTxHash\n";
    std::cout << "   Ti.Inputs[0].amount == Ti.ReferencedAmount\n\n";
    
    // Initialize config for testnet
    Config::set("testnet", "true");
    Config::set("network", "testnet");
    
    // Create wallets
    Wallet wallet1, wallet2;
    print_success("Wallet 1: " + wallet1.getAddress().substr(0, 30) + "...");
    print_success("Wallet 2: " + wallet2.getAddress().substr(0, 30) + "...");
    
    // Create fake UTXO set for testing
    std::unordered_map<std::string, TransactionOutput> utxoSet;
    
    // Add genesis UTXO for wallet1
    // UTXO key format: txHash_outputIndex (outputIndex must be numeric)
    std::string genesisTxHash = "0000000000000000000000000000000000000000000000000000000000000001";
    TransactionOutput genesisUtxo;
    genesisUtxo.address = wallet1.getAddress();
    genesisUtxo.amount = 200.0;
    utxoSet[genesisTxHash + "_0"] = genesisUtxo;
    
    print_success("Genesis UTXO created: 200 GXC for wallet1");
    
    // Test 1: Create first transaction
    print_section("TEST 1: First Transaction (Traceability)");
    
    try {
        Transaction tx1 = wallet1.createTransaction(wallet2.getAddress(), 50.0, utxoSet, 0.001);
        print_success("Transaction 1 created");
        
        // Verify traceability
        print_info("Verifying traceability formula...");
        std::cout << "   TX Hash: " << tx1.getHash().substr(0, 16) << "...\n";
        std::cout << "   PrevTxHash: " << tx1.getPrevTxHash().substr(0, 16) << "...\n";
        std::cout << "   Inputs[0].txHash: " << tx1.getInputs()[0].txHash.substr(0, 16) << "...\n";
        std::cout << "   Inputs[0].amount: " << tx1.getInputs()[0].amount << " GXC\n";
        std::cout << "   ReferencedAmount: " << tx1.getReferencedAmount() << " GXC\n";
        
        // Check formula
        bool hashMatch = (tx1.getInputs()[0].txHash == tx1.getPrevTxHash());
        bool amountMatch = (std::abs(tx1.getInputs()[0].amount - tx1.getReferencedAmount()) < 0.00000001);
        
        if (hashMatch && amountMatch) {
            print_success("✓ Traceability formula VALID");
            std::cout << "   ✓ Inputs[0].txHash == PrevTxHash: " << (hashMatch ? "true" : "false") << "\n";
            std::cout << "   ✓ Inputs[0].amount == ReferencedAmount: " << (amountMatch ? "true" : "false") << "\n";
        } else {
            print_error("✗ Traceability formula INVALID");
            std::cout << "   ✗ Hash match: " << (hashMatch ? "true" : "false") << "\n";
            std::cout << "   ✗ Amount match: " << (amountMatch ? "true" : "false") << "\n";
            return false;
        }
        
        // Verify transaction structure
        print_info("Transaction structure:");
        std::cout << "   Inputs: " << tx1.getInputs().size() << "\n";
        std::cout << "   Outputs: " << tx1.getOutputs().size() << "\n";
        std::cout << "   Total input: " << tx1.getTotalInputAmount() << " GXC\n";
        std::cout << "   Total output: " << tx1.getTotalOutputAmount() << " GXC\n";
        std::cout << "   Fee: " << tx1.getFee() << " GXC\n";
        
        // Verify balance equation: inputs = outputs + fee
        double inputTotal = tx1.getTotalInputAmount();
        double outputTotal = tx1.getTotalOutputAmount();
        double fee = tx1.getFee();
        
        if (std::abs(inputTotal - (outputTotal + fee)) < 0.00000001) {
            print_success("✓ Balance equation valid: inputs = outputs + fee");
        } else {
            print_error("✗ Balance equation invalid");
            return false;
        }
        
        // Update UTXO set for next transaction
        utxoSet.clear();
        for (size_t i = 0; i < tx1.getOutputs().size(); i++) {
            std::string utxoKey = tx1.getHash() + "_" + std::to_string(i);
            utxoSet[utxoKey] = tx1.getOutputs()[i];
        }
        
    } catch (const std::exception& e) {
        print_error("Transaction 1 failed: " + std::string(e.what()));
        return false;
    }
    
    // Test 2: Create second transaction (chained)
    print_section("TEST 2: Chained Transaction (Traceability)");
    
    try {
        Transaction tx2 = wallet2.createTransaction(wallet1.getAddress(), 20.0, utxoSet, 0.001);
        print_success("Transaction 2 created");
        
        // Verify traceability
        print_info("Verifying traceability formula...");
        std::cout << "   TX Hash: " << tx2.getHash().substr(0, 16) << "...\n";
        std::cout << "   PrevTxHash: " << tx2.getPrevTxHash().substr(0, 16) << "...\n";
        std::cout << "   Inputs[0].txHash: " << tx2.getInputs()[0].txHash.substr(0, 16) << "...\n";
        
        // Check formula
        bool hashMatch = (tx2.getInputs()[0].txHash == tx2.getPrevTxHash());
        bool amountMatch = (std::abs(tx2.getInputs()[0].amount - tx2.getReferencedAmount()) < 0.00000001);
        
        if (hashMatch && amountMatch) {
            print_success("✓ Traceability formula VALID for TX2");
        } else {
            print_error("✗ Traceability formula INVALID for TX2");
            return false;
        }
        
    } catch (const std::exception& e) {
        print_error("Transaction 2 failed: " + std::string(e.what()));
        return false;
    }
    
    // Test 3: Staking transaction
    print_section("TEST 3: Staking Transaction (Traceability)");
    
    // Reset UTXO set with fresh funds
    utxoSet.clear();
    TransactionOutput freshUtxo;
    freshUtxo.address = wallet1.getAddress();
    freshUtxo.amount = 200.0;
    std::string freshTxHash = "0000000000000000000000000000000000000000000000000000000000000002";
    utxoSet[freshTxHash + "_0"] = freshUtxo;
    
    try {
        Transaction stakeTx = wallet1.createStakeTransaction(100.0, utxoSet, 0.001);
        print_success("Stake transaction created");
        
        // Verify type
        if (stakeTx.getType() == TransactionType::STAKE) {
            print_success("✓ Transaction type is STAKE");
        } else {
            print_error("✗ Transaction type is not STAKE");
            return false;
        }
        
        // Verify traceability
        print_info("Verifying stake transaction traceability...");
        std::cout << "   PrevTxHash: " << stakeTx.getPrevTxHash().substr(0, 16) << "...\n";
        std::cout << "   Inputs[0].txHash: " << stakeTx.getInputs()[0].txHash.substr(0, 16) << "...\n";
        std::cout << "   Inputs[0].amount: " << stakeTx.getInputs()[0].amount << " GXC\n";
        std::cout << "   ReferencedAmount: " << stakeTx.getReferencedAmount() << " GXC\n";
        
        // Check formula
        bool hashMatch = (stakeTx.getInputs()[0].txHash == stakeTx.getPrevTxHash());
        bool amountMatch = (std::abs(stakeTx.getInputs()[0].amount - stakeTx.getReferencedAmount()) < 0.00000001);
        
        if (hashMatch && amountMatch) {
            print_success("✓ Stake transaction traceability VALID");
        } else {
            print_error("✗ Stake transaction traceability INVALID");
            return false;
        }
        
        // Verify stake amount calculation
        double totalInput = stakeTx.getTotalInputAmount();
        double totalOutput = stakeTx.getTotalOutputAmount();
        double fee = stakeTx.getFee();
        double stakedAmount = totalInput - totalOutput - fee;
        
        print_info("Stake calculation:");
        std::cout << "   Total input: " << totalInput << " GXC\n";
        std::cout << "   Total output: " << totalOutput << " GXC\n";
        std::cout << "   Fee: " << fee << " GXC\n";
        std::cout << "   Staked amount: " << stakedAmount << " GXC\n";
        
        if (std::abs(stakedAmount - 100.0) < 0.001) {
            print_success("✓ Stake amount correct: " + std::to_string(stakedAmount) + " GXC");
        } else {
            print_error("✗ Stake amount incorrect: Expected 100 GXC, got " + std::to_string(stakedAmount) + " GXC");
            return false;
        }
        
        // Verify outputs (should only be change, not staked amount)
        print_info("Verifying stake transaction outputs...");
        std::cout << "   Number of outputs: " << stakeTx.getOutputs().size() << "\n";
        
        if (stakeTx.getOutputs().size() == 1) {
            double changeAmount = stakeTx.getOutputs()[0].amount;
            double expectedChange = 200.0 - 100.0 - 0.001;
            
            if (std::abs(changeAmount - expectedChange) < 0.001) {
                print_success("✓ Change output correct: " + std::to_string(changeAmount) + " GXC");
            } else {
                print_error("✗ Change output incorrect");
                return false;
            }
        }
        
    } catch (const std::exception& e) {
        print_error("Stake transaction failed: " + std::string(e.what()));
        return false;
    }
    
    // Test 4: Verify amount matching with different amounts
    print_section("TEST 4: Amount Matching Verification");
    
    // Reset UTXO set with different amount
    utxoSet.clear();
    TransactionOutput testUtxo;
    testUtxo.address = wallet1.getAddress();
    testUtxo.amount = 150.5; // Different amount
    std::string testTxHash = "0000000000000000000000000000000000000000000000000000000000000003";
    utxoSet[testTxHash + "_0"] = testUtxo;
    
    try {
        Transaction testTx = wallet1.createTransaction(wallet2.getAddress(), 30.0, utxoSet, 0.001);
        print_success("Test transaction created with 150.5 GXC input");
        
        // Verify the referenced amount matches the input amount
        print_info("Verifying amount matching...");
        std::cout << "   Input amount: " << testTx.getInputs()[0].amount << " GXC\n";
        std::cout << "   Referenced amount: " << testTx.getReferencedAmount() << " GXC\n";
        
        double inputAmount = testTx.getInputs()[0].amount;
        double refAmount = testTx.getReferencedAmount();
        
        if (std::abs(inputAmount - refAmount) < 0.00000001) {
            print_success("✓ Amount matching VERIFIED: " + std::to_string(inputAmount) + " == " + std::to_string(refAmount));
        } else {
            print_error("✗ Amount matching FAILED");
            std::cout << "   Expected: " << inputAmount << " GXC\n";
            std::cout << "   Got: " << refAmount << " GXC\n";
            std::cout << "   Difference: " << std::abs(inputAmount - refAmount) << " GXC\n";
            return false;
        }
        
        // Verify the formula holds
        if (testTx.verifyTraceabilityFormula()) {
            print_success("✓ Traceability formula verified with amount " + std::to_string(inputAmount) + " GXC");
        } else {
            print_error("✗ Traceability formula failed");
            return false;
        }
        
    } catch (const std::exception& e) {
        print_error("Amount matching test failed: " + std::string(e.what()));
        return false;
    }
    
    print_section("ALL TESTS PASSED");
    print_success("Fund transfers work correctly");
    print_success("Traceability formula is enforced");
    print_success("Transaction chains are preserved");
    print_success("Staking transactions maintain traceability");
    print_success("Amount matching verified with different values");
    
    return true;
}

int main() {
    std::cout << "\n" << std::string(70, '=') << "\n";
    std::cout << "  GXC BLOCKCHAIN COMPREHENSIVE TEST\n";
    std::cout << "  Testing: Fund Transfer, Staking, and Traceability\n";
    std::cout << std::string(70, '=') << "\n";
    
    try {
        bool success = test_traceability();
        
        if (success) {
            std::cout << "\n" << std::string(70, '=') << "\n";
            std::cout << "  ✅ ALL TESTS PASSED\n";
            std::cout << std::string(70, '=') << "\n";
            return 0;
        } else {
            std::cout << "\n" << std::string(70, '=') << "\n";
            std::cout << "  ❌ TESTS FAILED\n";
            std::cout << std::string(70, '=') << "\n";
            return 1;
        }
    } catch (const std::exception& e) {
        print_error("Test suite failed with exception: " + std::string(e.what()));
        return 1;
    }
}
