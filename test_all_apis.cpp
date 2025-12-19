/**
 * Comprehensive API Test Suite
 * Tests all RPC APIs to ensure they work for developers
 */

#include "include/RPCAPI.h"
#include "include/blockchain.h"
#include "include/Wallet.h"
#include "include/Validator.h"
#include "include/Config.h"
#include <iostream>
#include <iomanip>
#include <memory>

void print_section(const std::string& title) {
    std::cout << "\n" << std::string(70, '=') << "\n";
    std::cout << "  " << title << "\n";
    std::cout << std::string(70, '=') << "\n";
}

void print_test(const std::string& name, bool passed) {
    std::cout << (passed ? "✅" : "❌") << " " << name << "\n";
}

class APITester {
private:
    std::unique_ptr<Blockchain> blockchain;
    std::unique_ptr<RPCAPI> rpcapi;
    std::unique_ptr<Wallet> wallet1;
    std::unique_ptr<Wallet> wallet2;
    
public:
    APITester() {
        // Initialize testnet
        Config::set("testnet", "true");
        Config::set("network", "testnet");
        
        // Create blockchain
        blockchain = std::make_unique<Blockchain>();
        
        // Create RPC API
        rpcapi = std::make_unique<RPCAPI>(blockchain.get(), nullptr);
        
        // Create test wallets
        wallet1 = std::make_unique<Wallet>();
        wallet2 = std::make_unique<Wallet>();
        
        std::cout << "✅ Test environment initialized\n";
        std::cout << "   Wallet 1: " << wallet1->getAddress().substr(0, 30) << "...\n";
        std::cout << "   Wallet 2: " << wallet2->getAddress().substr(0, 30) << "...\n";
    }
    
    bool testBlockchainInfoAPIs() {
        print_section("TEST 1: Blockchain Info APIs");
        
        bool allPassed = true;
        
        try {
            // Test getblockchaininfo
            JsonValue result = rpcapi->getBlockchainInfo(JsonValue());
            bool hasChain = result.contains("chain");
            bool hasBlocks = result.contains("blocks");
            print_test("getblockchaininfo", hasChain && hasBlocks);
            allPassed &= (hasChain && hasBlocks);
            
            if (hasChain) {
                std::cout << "   Chain: " << result["chain"].getString() << "\n";
            }
            if (hasBlocks) {
                std::cout << "   Height: " << result["blocks"].getInt() << "\n";
            }
            
        } catch (const std::exception& e) {
            print_test("getblockchaininfo", false);
            std::cout << "   Error: " << e.what() << "\n";
            allPassed = false;
        }
        
        try {
            // Test getblockcount
            JsonValue result = rpcapi->getBlockCount(JsonValue());
            bool isNumber = result.isInt();
            print_test("getblockcount", isNumber);
            allPassed &= isNumber;
            
            if (isNumber) {
                std::cout << "   Block count: " << result.getInt() << "\n";
            }
            
        } catch (const std::exception& e) {
            print_test("getblockcount", false);
            std::cout << "   Error: " << e.what() << "\n";
            allPassed = false;
        }
        
        return allPassed;
    }
    
    bool testBalanceAPIs() {
        print_section("TEST 2: Balance & UTXO APIs");
        
        bool allPassed = true;
        std::string address = wallet1->getAddress();
        
        try {
            // Test getbalance
            JsonValue params;
            params.push_back(JsonValue(address));
            JsonValue balance = rpcapi->getBalance(params);
            
            bool isNumber = balance.isDouble() || balance.isInt();
            print_test("getbalance", isNumber);
            allPassed &= isNumber;
            
            if (isNumber) {
                std::cout << "   Balance: " << balance.getDouble() << " GXC\n";
            }
            
        } catch (const std::exception& e) {
            print_test("getbalance", false);
            std::cout << "   Error: " << e.what() << "\n";
            allPassed = false;
        }
        
        try {
            // Test listunspent
            JsonValue params;
            params.push_back(JsonValue(address));
            JsonValue utxos = rpcapi->listUnspent(params);
            
            bool isArray = utxos.isArray();
            print_test("listunspent", isArray);
            allPassed &= isArray;
            
            if (isArray) {
                std::cout << "   UTXOs: " << utxos.size() << "\n";
            }
            
        } catch (const std::exception& e) {
            print_test("listunspent", false);
            std::cout << "   Error: " << e.what() << "\n";
            allPassed = false;
        }
        
        try {
            // Test getStakingInfo
            JsonValue params;
            params.push_back(JsonValue(address));
            JsonValue info = rpcapi->getStakingInfo(params);
            
            bool hasFields = info.contains("spendable_balance") && 
                           info.contains("staked_amount") && 
                           info.contains("total_balance");
            print_test("getStakingInfo", hasFields);
            allPassed &= hasFields;
            
            if (hasFields) {
                std::cout << "   Spendable: " << info["spendable_balance"].getDouble() << " GXC\n";
                std::cout << "   Staked: " << info["staked_amount"].getDouble() << " GXC\n";
                std::cout << "   Total: " << info["total_balance"].getDouble() << " GXC\n";
            }
            
        } catch (const std::exception& e) {
            print_test("getStakingInfo", false);
            std::cout << "   Error: " << e.what() << "\n";
            allPassed = false;
        }
        
        return allPassed;
    }
    
    bool testValidatorAPIs() {
        print_section("TEST 3: Validator APIs");
        
        bool allPassed = true;
        
        try {
            // Test getValidators
            JsonValue result = rpcapi->getValidators(JsonValue());
            
            bool isArray = result.isArray();
            print_test("getValidators", isArray);
            allPassed &= isArray;
            
            if (isArray) {
                std::cout << "   Validators: " << result.size() << "\n";
            }
            
        } catch (const std::exception& e) {
            print_test("getValidators", false);
            std::cout << "   Error: " << e.what() << "\n";
            allPassed = false;
        }
        
        try {
            // Test getValidatorInfo (should fail for non-validator)
            JsonValue params;
            params.push_back(JsonValue(wallet1->getAddress()));
            
            try {
                JsonValue info = rpcapi->getValidatorInfo(params);
                print_test("getValidatorInfo (non-validator)", false);
                allPassed = false;
            } catch (const std::exception& e) {
                // Expected to fail for non-validator
                print_test("getValidatorInfo (error handling)", true);
                std::cout << "   Expected error: " << e.what() << "\n";
            }
            
        } catch (const std::exception& e) {
            print_test("getValidatorInfo", false);
            std::cout << "   Error: " << e.what() << "\n";
            allPassed = false;
        }
        
        return allPassed;
    }
    
    bool testFeeAPIs() {
        print_section("TEST 4: Fee Estimation APIs");
        
        bool allPassed = true;
        
        try {
            // Test estimateFee
            JsonValue result = rpcapi->estimateFee(JsonValue());
            
            bool hasFields = result.contains("recommended_fee") && 
                           result.contains("base_fee");
            print_test("estimateFee", hasFields);
            allPassed &= hasFields;
            
            if (hasFields) {
                std::cout << "   Recommended: " << result["recommended_fee"].getDouble() << " GXC\n";
                std::cout << "   Base: " << result["base_fee"].getDouble() << " GXC\n";
            }
            
        } catch (const std::exception& e) {
            print_test("estimateFee", false);
            std::cout << "   Error: " << e.what() << "\n";
            allPassed = false;
        }
        
        return allPassed;
    }
    
    bool testTransactionCreation() {
        print_section("TEST 5: Transaction Creation");
        
        bool allPassed = true;
        
        try {
            // Create fake UTXO for testing
            std::unordered_map<std::string, TransactionOutput> utxoSet;
            TransactionOutput utxo;
            utxo.address = wallet1->getAddress();
            utxo.amount = 200.0;
            std::string txHash = "0000000000000000000000000000000000000000000000000000000000000001";
            utxoSet[txHash + "_0"] = utxo;
            
            // Test regular transaction
            Transaction tx = wallet1->createTransaction(
                wallet2->getAddress(),
                50.0,
                utxoSet,
                0.001
            );
            
            bool hasInputs = !tx.getInputs().empty();
            bool hasOutputs = !tx.getOutputs().empty();
            bool validTrace = tx.verifyTraceabilityFormula();
            
            print_test("createTransaction", hasInputs && hasOutputs);
            print_test("Transaction has inputs", hasInputs);
            print_test("Transaction has outputs", hasOutputs);
            print_test("Traceability valid", validTrace);
            
            allPassed &= (hasInputs && hasOutputs && validTrace);
            
            if (hasInputs && hasOutputs) {
                std::cout << "   Inputs: " << tx.getInputs().size() << "\n";
                std::cout << "   Outputs: " << tx.getOutputs().size() << "\n";
                std::cout << "   Total input: " << tx.getTotalInputAmount() << " GXC\n";
                std::cout << "   Total output: " << tx.getTotalOutputAmount() << " GXC\n";
                std::cout << "   Fee: " << tx.getFee() << " GXC\n";
            }
            
        } catch (const std::exception& e) {
            print_test("createTransaction", false);
            std::cout << "   Error: " << e.what() << "\n";
            allPassed = false;
        }
        
        return allPassed;
    }
    
    bool testStakeTransaction() {
        print_section("TEST 6: Stake Transaction Creation");
        
        bool allPassed = true;
        
        try {
            // Create fake UTXO for testing
            std::unordered_map<std::string, TransactionOutput> utxoSet;
            TransactionOutput utxo;
            utxo.address = wallet1->getAddress();
            utxo.amount = 200.0;
            std::string txHash = "0000000000000000000000000000000000000000000000000000000000000002";
            utxoSet[txHash + "_0"] = utxo;
            
            // Test stake transaction
            Transaction stakeTx = wallet1->createStakeTransaction(
                100.0,
                utxoSet,
                0.001
            );
            
            bool isStake = (stakeTx.getType() == TransactionType::STAKE);
            bool hasInputs = !stakeTx.getInputs().empty();
            bool validTrace = stakeTx.verifyTraceabilityFormula();
            
            print_test("createStakeTransaction", isStake && hasInputs);
            print_test("Transaction type is STAKE", isStake);
            print_test("Traceability valid", validTrace);
            
            allPassed &= (isStake && hasInputs && validTrace);
            
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
                print_test("Staked amount correct", correctAmount);
                allPassed &= correctAmount;
            }
            
        } catch (const std::exception& e) {
            print_test("createStakeTransaction", false);
            std::cout << "   Error: " << e.what() << "\n";
            allPassed = false;
        }
        
        return allPassed;
    }
    
    bool testUnstakeTransaction() {
        print_section("TEST 7: Unstake Transaction Creation");
        
        bool allPassed = true;
        
        try {
            // Test unstake transaction
            Transaction unstakeTx = wallet1->createUnstakeTransaction(50.0, 0.0);
            
            bool isUnstake = (unstakeTx.getType() == TransactionType::UNSTAKE);
            bool hasOutputs = !unstakeTx.getOutputs().empty();
            
            print_test("createUnstakeTransaction", isUnstake && hasOutputs);
            print_test("Transaction type is UNSTAKE", isUnstake);
            
            allPassed &= (isUnstake && hasOutputs);
            
            if (hasOutputs) {
                std::cout << "   Outputs: " << unstakeTx.getOutputs().size() << "\n";
                std::cout << "   Amount: " << unstakeTx.getOutputs()[0].amount << " GXC\n";
                std::cout << "   To: " << unstakeTx.getOutputs()[0].address.substr(0, 30) << "...\n";
            }
            
        } catch (const std::exception& e) {
            print_test("createUnstakeTransaction", false);
            std::cout << "   Error: " << e.what() << "\n";
            allPassed = false;
        }
        
        return allPassed;
    }
    
    bool testTraceability() {
        print_section("TEST 8: Traceability Verification");
        
        bool allPassed = true;
        
        try {
            // Create transaction chain
            std::unordered_map<std::string, TransactionOutput> utxoSet;
            TransactionOutput utxo;
            utxo.address = wallet1->getAddress();
            utxo.amount = 200.0;
            std::string txHash = "0000000000000000000000000000000000000000000000000000000000000003";
            utxoSet[txHash + "_0"] = utxo;
            
            // TX1
            Transaction tx1 = wallet1->createTransaction(
                wallet2->getAddress(),
                50.0,
                utxoSet,
                0.001
            );
            
            // Verify TX1 traceability
            bool tx1Valid = tx1.verifyTraceabilityFormula();
            print_test("TX1 traceability", tx1Valid);
            allPassed &= tx1Valid;
            
            if (tx1Valid) {
                std::cout << "   TX1 PrevTxHash: " << tx1.getPrevTxHash().substr(0, 16) << "...\n";
                std::cout << "   TX1 Inputs[0].txHash: " << tx1.getInputs()[0].txHash.substr(0, 16) << "...\n";
                std::cout << "   TX1 Inputs[0].amount: " << tx1.getInputs()[0].amount << " GXC\n";
                std::cout << "   TX1 ReferencedAmount: " << tx1.getReferencedAmount() << " GXC\n";
                
                bool hashMatch = (tx1.getInputs()[0].txHash == tx1.getPrevTxHash());
                bool amountMatch = (std::abs(tx1.getInputs()[0].amount - tx1.getReferencedAmount()) < 0.00000001);
                
                print_test("Hash matching", hashMatch);
                print_test("Amount matching", amountMatch);
                allPassed &= (hashMatch && amountMatch);
            }
            
        } catch (const std::exception& e) {
            print_test("Traceability verification", false);
            std::cout << "   Error: " << e.what() << "\n";
            allPassed = false;
        }
        
        return allPassed;
    }
    
    void runAllTests() {
        std::cout << "\n╔════════════════════════════════════════════════════════════╗\n";
        std::cout << "║         GXC API Comprehensive Test Suite                  ║\n";
        std::cout << "╚════════════════════════════════════════════════════════════╝\n";
        
        int totalTests = 0;
        int passedTests = 0;
        
        // Run all test suites
        bool test1 = testBlockchainInfoAPIs();
        totalTests++; if (test1) passedTests++;
        
        bool test2 = testBalanceAPIs();
        totalTests++; if (test2) passedTests++;
        
        bool test3 = testValidatorAPIs();
        totalTests++; if (test3) passedTests++;
        
        bool test4 = testFeeAPIs();
        totalTests++; if (test4) passedTests++;
        
        bool test5 = testTransactionCreation();
        totalTests++; if (test5) passedTests++;
        
        bool test6 = testStakeTransaction();
        totalTests++; if (test6) passedTests++;
        
        bool test7 = testUnstakeTransaction();
        totalTests++; if (test7) passedTests++;
        
        bool test8 = testTraceability();
        totalTests++; if (test8) passedTests++;
        
        // Print summary
        print_section("TEST SUMMARY");
        std::cout << "Total Test Suites: " << totalTests << "\n";
        std::cout << "Passed: " << passedTests << "\n";
        std::cout << "Failed: " << (totalTests - passedTests) << "\n";
        std::cout << "Success Rate: " << std::fixed << std::setprecision(1) 
                  << (100.0 * passedTests / totalTests) << "%\n";
        
        if (passedTests == totalTests) {
            std::cout << "\n✅ ALL API TESTS PASSED!\n";
        } else {
            std::cout << "\n❌ SOME TESTS FAILED\n";
        }
    }
};

int main() {
    try {
        APITester tester;
        tester.runAllTests();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "❌ Test suite failed: " << e.what() << "\n";
        return 1;
    }
}
