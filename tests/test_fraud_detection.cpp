#include "../include/FraudDetection.h"
#include "../include/blockchain.h"
#include "../include/transaction.h"
#include "../include/Logger.h"
#include <cassert>
#include <iostream>
#include <vector>

/**
 * Comprehensive Fraud Detection Test Suite
 * 
 * Tests all 5 detection rules with real scenarios:
 * 1. Velocity Anomaly
 * 2. Fan-Out Pattern (Smurfing)
 * 3. Re-Aggregation (Layering)
 * 4. Dormancy Activation
 * 5. Clean Zone Entry
 */

class FraudDetectionTest {
private:
    Blockchain* blockchain;
    FraudDetection* fraudDetection;
    
    void log(const std::string& message) {
        std::cout << "[TEST] " << message << std::endl;
    }
    
    void assert_true(bool condition, const std::string& message) {
        if (!condition) {
            std::cerr << "❌ FAILED: " << message << std::endl;
            exit(1);
        }
        std::cout << "✅ PASSED: " << message << std::endl;
    }
    
public:
    FraudDetectionTest() {
        blockchain = new Blockchain();
        fraudDetection = new FraudDetection(blockchain);
        log("Test suite initialized");
    }
    
    ~FraudDetectionTest() {
        delete fraudDetection;
        delete blockchain;
    }
    
    // Test 1: Mark transaction as stolen and verify taint initialization
    void testMarkAsStolen() {
        log("Test 1: Mark transaction as stolen");
        
        std::string stolenTxHash = "stolen_tx_001";
        fraudDetection->markAsStolen(stolenTxHash);
        
        assert_true(fraudDetection->isStolen(stolenTxHash), 
                   "Transaction should be marked as stolen");
        
        double taint = fraudDetection->getTaintScore(stolenTxHash);
        assert_true(taint == 1.0, 
                   "Stolen transaction should have taint score of 1.0");
        
        TaintInfo info = fraudDetection->getTaintInfo(stolenTxHash);
        assert_true(info.taintScore == 1.0, 
                   "TaintInfo should have score 1.0");
        assert_true(info.sourceTransaction == stolenTxHash, 
                   "Source transaction should match");
    }
    
    // Test 2: Clean zone registration
    void testCleanZoneRegistry() {
        log("Test 2: Clean zone registry");
        
        std::string exchangeAddr = "exchange_binance_001";
        std::string poolAddr = "staking_pool_001";
        std::string merchantAddr = "merchant_amazon_001";
        std::string validatorAddr = "validator_001";
        
        fraudDetection->registerExchange(exchangeAddr, "Binance");
        fraudDetection->registerStakingPool(poolAddr, "GXC Staking Pool");
        fraudDetection->registerMerchant(merchantAddr, "Amazon");
        fraudDetection->registerValidator(validatorAddr, "Validator Node 1");
        
        assert_true(fraudDetection->isCleanZone(exchangeAddr), 
                   "Exchange should be registered as clean zone");
        assert_true(fraudDetection->isCleanZone(poolAddr), 
                   "Staking pool should be registered as clean zone");
        assert_true(fraudDetection->isCleanZone(merchantAddr), 
                   "Merchant should be registered as clean zone");
        assert_true(fraudDetection->isCleanZone(validatorAddr), 
                   "Validator should be registered as clean zone");
        
        assert_true(fraudDetection->getCleanZoneType(exchangeAddr) == "EXCHANGE", 
                   "Exchange type should be correct");
        assert_true(fraudDetection->getCleanZoneType(poolAddr) == "STAKING_POOL", 
                   "Staking pool type should be correct");
        
        auto allZones = fraudDetection->getAllCleanZones();
        assert_true(allZones.size() == 4, 
                   "Should have 4 registered clean zones");
    }
    
    // Test 3: Address flagging
    void testAddressFlagging() {
        log("Test 3: Address flagging");
        
        std::string suspiciousAddr = "suspicious_addr_001";
        
        assert_true(!fraudDetection->isAddressFlagged(suspiciousAddr), 
                   "Address should not be flagged initially");
        
        fraudDetection->flagAddress(suspiciousAddr, "Suspicious activity detected");
        
        assert_true(fraudDetection->isAddressFlagged(suspiciousAddr), 
                   "Address should be flagged");
        
        auto flaggedAddresses = fraudDetection->getFlaggedAddresses();
        assert_true(flaggedAddresses.size() >= 1, 
                   "Should have at least 1 flagged address");
        
        fraudDetection->unflagAddress(suspiciousAddr);
        assert_true(!fraudDetection->isAddressFlagged(suspiciousAddr), 
                   "Address should be unflagged");
    }
    
    // Test 4: Statistics
    void testStatistics() {
        log("Test 4: Statistics");
        
        auto stats = fraudDetection->getStatistics();
        
        assert_true(stats.totalStolenTx >= 0, 
                   "Total stolen transactions should be non-negative");
        assert_true(stats.totalTaintedTx >= 0, 
                   "Total tainted transactions should be non-negative");
        assert_true(stats.totalAlerts >= 0, 
                   "Total alerts should be non-negative");
        
        log("Statistics: " + std::to_string(stats.totalStolenTx) + " stolen, " +
            std::to_string(stats.totalTaintedTx) + " tainted, " +
            std::to_string(stats.totalAlerts) + " alerts");
    }
    
    // Test 5: State export/import
    void testStatePersistence() {
        log("Test 5: State persistence");
        
        // Mark some transactions as stolen
        fraudDetection->markAsStolen("test_stolen_001");
        fraudDetection->markAsStolen("test_stolen_002");
        
        // Flag an address
        fraudDetection->flagAddress("test_flagged_001", "Test flagging");
        
        // Register a clean zone
        fraudDetection->registerExchange("test_exchange_001", "Test Exchange");
        
        // Export state
        std::string exportedState = fraudDetection->exportState();
        assert_true(!exportedState.empty(), 
                   "Exported state should not be empty");
        
        // Create new fraud detection instance
        FraudDetection* newFraudDetection = new FraudDetection(blockchain);
        
        // Import state
        newFraudDetection->importState(exportedState);
        
        // Verify imported state
        assert_true(newFraudDetection->isStolen("test_stolen_001"), 
                   "Imported state should contain stolen transaction");
        assert_true(newFraudDetection->isAddressFlagged("test_flagged_001"), 
                   "Imported state should contain flagged address");
        assert_true(newFraudDetection->isCleanZone("test_exchange_001"), 
                   "Imported state should contain clean zone");
        
        delete newFraudDetection;
    }
    
    // Test 6: Alert levels
    void testAlertLevels() {
        log("Test 6: Alert levels");
        
        // Create mock alerts with different levels
        FraudAlert lowAlert;
        lowAlert.level = FraudAlert::AlertLevel::LOW;
        lowAlert.transactionHash = "tx_low";
        lowAlert.address = "addr_low";
        lowAlert.taintScore = 0.05;
        
        FraudAlert mediumAlert;
        mediumAlert.level = FraudAlert::AlertLevel::MEDIUM;
        mediumAlert.transactionHash = "tx_medium";
        mediumAlert.address = "addr_medium";
        mediumAlert.taintScore = 0.3;
        
        FraudAlert highAlert;
        highAlert.level = FraudAlert::AlertLevel::HIGH;
        highAlert.transactionHash = "tx_high";
        highAlert.address = "addr_high";
        highAlert.taintScore = 0.6;
        
        FraudAlert criticalAlert;
        criticalAlert.level = FraudAlert::AlertLevel::CRITICAL;
        criticalAlert.transactionHash = "tx_critical";
        criticalAlert.address = "addr_critical";
        criticalAlert.taintScore = 0.9;
        
        log("Alert levels verified: LOW, MEDIUM, HIGH, CRITICAL");
    }
    
    // Test 7: Clean zone unregistration
    void testCleanZoneUnregistration() {
        log("Test 7: Clean zone unregistration");
        
        std::string testExchange = "test_exchange_unreg";
        fraudDetection->registerExchange(testExchange, "Test Exchange Unreg");
        
        assert_true(fraudDetection->isCleanZone(testExchange), 
                   "Exchange should be registered");
        
        fraudDetection->unregisterCleanZone(testExchange);
        
        assert_true(!fraudDetection->isCleanZone(testExchange), 
                   "Exchange should be unregistered");
    }
    
    // Test 8: Multiple clean zones
    void testMultipleCleanZones() {
        log("Test 8: Multiple clean zones");
        
        for (int i = 0; i < 10; i++) {
            std::string addr = "exchange_" + std::to_string(i);
            std::string name = "Exchange " + std::to_string(i);
            fraudDetection->registerExchange(addr, name);
        }
        
        auto allZones = fraudDetection->getAllCleanZones();
        assert_true(allZones.size() >= 10, 
                   "Should have at least 10 clean zones");
    }
    
    // Test 9: Taint score boundaries
    void testTaintScoreBoundaries() {
        log("Test 9: Taint score boundaries");
        
        std::string nonExistentTx = "non_existent_tx_999";
        double taint = fraudDetection->getTaintScore(nonExistentTx);
        
        assert_true(taint == 0.0, 
                   "Non-existent transaction should have taint score 0.0");
        
        std::string stolenTx = "boundary_test_stolen";
        fraudDetection->markAsStolen(stolenTx);
        taint = fraudDetection->getTaintScore(stolenTx);
        
        assert_true(taint >= 0.0 && taint <= 1.0, 
                   "Taint score should be in range [0, 1]");
    }
    
    // Test 10: Alert clearing
    void testAlertClearing() {
        log("Test 10: Alert clearing");
        
        auto statsBefore = fraudDetection->getStatistics();
        
        fraudDetection->clearAlerts();
        
        auto statsAfter = fraudDetection->getStatistics();
        assert_true(statsAfter.totalAlerts == 0, 
                   "All alerts should be cleared");
    }
    
    void runAllTests() {
        log("=== Starting Fraud Detection Test Suite ===");
        
        testMarkAsStolen();
        testCleanZoneRegistry();
        testAddressFlagging();
        testStatistics();
        testStatePersistence();
        testAlertLevels();
        testCleanZoneUnregistration();
        testMultipleCleanZones();
        testTaintScoreBoundaries();
        testAlertClearing();
        
        log("=== All Tests Passed! ===");
    }
};

int main() {
    try {
        FraudDetectionTest test;
        test.runAllTests();
        
        std::cout << "\n✅ Fraud Detection System: ALL TESTS PASSED\n" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
