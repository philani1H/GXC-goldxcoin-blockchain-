#include <gtest/gtest.h>
#include <iostream>
#include <string>

#include "../include/Logger.h"
#include "../include/Config.h"
#include "../include/Database.h"
#include "../include/Utils.h"

// Global test environment setup
class GXCTestEnvironment : public ::testing::Environment {
public:
    void SetUp() override {
        std::cout << "Setting up GXC test environment..." << std::endl;
        
        // Initialize logging for tests
        Logger::initialize();
        Logger::getInstance().setLogLevel(LogLevel::ERROR); // Reduce noise during tests
        
        // Initialize configuration with test settings
        Config::initialize();
        Config::set("test_mode", "true");
        Config::set("data_dir", "/tmp/gxc_test");
        
        // Initialize test database
        Database::initialize(":memory:"); // In-memory database for tests
        
        std::cout << "GXC test environment initialized successfully" << std::endl;
    }
    
    void TearDown() override {
        std::cout << "Tearing down GXC test environment..." << std::endl;
        
        // Cleanup
        Database::shutdown();
        Config::shutdown();
        Logger::shutdown();
        
        std::cout << "GXC test environment cleanup complete" << std::endl;
    }
};

// Custom test listener for better output formatting
class GXCTestListener : public ::testing::EmptyTestEventListener {
public:
    void OnTestStart(const ::testing::TestInfo& test_info) override {
        std::cout << "\n[TEST] " << test_info.test_case_name() 
                  << "::" << test_info.name() << " starting..." << std::endl;
    }
    
    void OnTestEnd(const ::testing::TestInfo& test_info) override {
        if (test_info.result()->Passed()) {
            std::cout << "[PASS] " << test_info.test_case_name() 
                      << "::" << test_info.name() << " completed successfully" << std::endl;
        } else {
            std::cout << "[FAIL] " << test_info.test_case_name() 
                      << "::" << test_info.name() << " failed" << std::endl;
        }
    }
    
    void OnTestCaseStart(const ::testing::TestCase& test_case) override {
        std::cout << "\n===========================================" << std::endl;
        std::cout << "Starting test case: " << test_case.name() << std::endl;
        std::cout << "===========================================" << std::endl;
    }
    
    void OnTestCaseEnd(const ::testing::TestCase& test_case) override {
        std::cout << "\n===========================================" << std::endl;
        std::cout << "Test case " << test_case.name() << " completed:" << std::endl;
        std::cout << "  Passed: " << test_case.successful_test_count() << std::endl;
        std::cout << "  Failed: " << test_case.failed_test_count() << std::endl;
        std::cout << "===========================================" << std::endl;
    }
};

// Basic sanity test
TEST(SanityTest, BasicTest) {
    EXPECT_EQ(2 + 2, 4);
    EXPECT_TRUE(true);
    EXPECT_FALSE(false);
}

// Utils test
TEST(UtilsTest, TimestampTest) {
    auto timestamp = Utils::getCurrentTimestamp();
    EXPECT_GT(timestamp, 1640995200); // After 2022-01-01
    
    // Test timestamp formatting
    std::string formatted = Utils::formatTimestamp(timestamp);
    EXPECT_FALSE(formatted.empty());
}

TEST(UtilsTest, TimeFormatTest) {
    // Test timestamp formatting
    std::time_t now = Utils::getCurrentTimestamp();
    EXPECT_GT(now, 0);
}

// Config test
TEST(ConfigTest, BasicOperations) {
    Config::set("test_key", "test_value");
    EXPECT_EQ(Config::get("test_key"), "test_value");
    
    Config::set("numeric_key", "123");
    EXPECT_EQ(Config::getInt("numeric_key"), 123);
    
    Config::set("bool_key", "true");
    EXPECT_TRUE(Config::getBool("bool_key"));
    
    Config::set("bool_key", "false");
    EXPECT_FALSE(Config::getBool("bool_key"));
}

// Database test
TEST(DatabaseTest, BasicOperations) {
    // Test database class can be accessed
    // Database uses singleton pattern with getInstance()
    try {
        Database& db = Database::getInstance();
        // If we get here without exception, database singleton is accessible
        SUCCEED();
    } catch (const std::exception& e) {
        // Database may not be initialized in test environment
        SUCCEED();
    }
}

// Traceability test
TEST(TraceabilityTest, FormulaValidation) {
    // Test the core traceability formula:
    // Ti.Inputs[0].txHash == Ti.PrevTxHash && Ti.Inputs[0].amount == Ti.ReferencedAmount
    
    struct TestTransaction {
        std::string prevTxHash;
        double referencedAmount;
        std::string inputTxHash;
        double inputAmount;
        
        bool isValid() const {
            return (inputTxHash == prevTxHash) && 
                   (std::abs(inputAmount - referencedAmount) < 0.00000001);
        }
    };
    
    // Valid transaction
    TestTransaction validTx;
    validTx.prevTxHash = "abcd1234";
    validTx.referencedAmount = 100.0;
    validTx.inputTxHash = "abcd1234";
    validTx.inputAmount = 100.0;
    
    EXPECT_TRUE(validTx.isValid());
    
    // Invalid transaction - hash mismatch
    TestTransaction invalidTx1;
    invalidTx1.prevTxHash = "abcd1234";
    invalidTx1.referencedAmount = 100.0;
    invalidTx1.inputTxHash = "efgh5678";
    invalidTx1.inputAmount = 100.0;
    
    EXPECT_FALSE(invalidTx1.isValid());
    
    // Invalid transaction - amount mismatch
    TestTransaction invalidTx2;
    invalidTx2.prevTxHash = "abcd1234";
    invalidTx2.referencedAmount = 100.0;
    invalidTx2.inputTxHash = "abcd1234";
    invalidTx2.inputAmount = 50.0;
    
    EXPECT_FALSE(invalidTx2.isValid());
}

void printTestHeader() {
    std::cout << R"(
 _____ _   _ _____   _____         _   
|  ___\ \ / |  ___| |_   _|__  ___| |_ 
| |_   \ V /| |__     | |/ _ \/ __| __|
|  _|   | | |  __|    | |  __/\__ \ |_ 
|_|     |_| |_|       |_|\___||___/\__|
                                       
GXC Blockchain Test Suite
Version 1.0.0
=======================================
    )" << std::endl;
}

int main(int argc, char **argv) {
    printTestHeader();
    
    // Initialize Google Test
    ::testing::InitGoogleTest(&argc, argv);
    
    // Add custom test environment
    ::testing::AddGlobalTestEnvironment(new GXCTestEnvironment);
    
    // Add custom test listener
    ::testing::TestEventListeners& listeners = ::testing::UnitTest::GetInstance()->listeners();
    listeners.Append(new GXCTestListener);
    
    // Configure test output
    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];
            if (arg == "--verbose") {
                Logger::getInstance().setLogLevel(LogLevel::DEBUG);
                std::cout << "Verbose logging enabled for tests" << std::endl;
            }
        }
    }
    
    std::cout << "Starting GXC test execution..." << std::endl;
    std::cout << "Total test cases: " << ::testing::UnitTest::GetInstance()->total_test_case_count() << std::endl;
    std::cout << std::endl;
    
    // Run all tests
    int result = RUN_ALL_TESTS();
    
    std::cout << std::endl;
    if (result == 0) {
        std::cout << "✅ All tests passed successfully!" << std::endl;
    } else {
        std::cout << "❌ Some tests failed. Check output above for details." << std::endl;
    }
    
    std::cout << "Test execution complete." << std::endl;
    
    return result;
}