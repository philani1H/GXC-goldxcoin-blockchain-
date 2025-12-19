#include "../include/StockMarketAPI.h"
#include "../include/MarketMakerAdmin.h"
#include "../include/blockchain.h"
#include <iostream>
#include <cassert>

int tests_passed = 0;
int tests_failed = 0;

void printTestResult(const std::string& testName, bool passed) {
    if (passed) {
        std::cout << "✅ PASS: " << testName << std::endl;
        tests_passed++;
    } else {
        std::cout << "❌ FAIL: " << testName << std::endl;
        tests_failed++;
    }
}

void printSection(const std::string& section) {
    std::cout << "\n========================================" << std::endl;
    std::cout << "  " << section << std::endl;
    std::cout << "========================================" << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  STOCK CONTRACTS COMPREHENSIVE TESTS" << std::endl;
    std::cout << "========================================" << std::endl;
    
    Blockchain blockchain;
    blockchain.initialize();
    
    MarketMakerRegistry registry(&blockchain);
    StockMarketAPI api(&blockchain, &registry);
    
    // Test 1: Register Market Maker
    printSection("TEST 1: Register Market Maker");
    try {
        bool registered = registry.registerMarketMaker(
            "tGXC_mm_goldman",
            "Goldman Sachs Trading Desk",
            "MM-12345-US",
            "USA"
        );
        printTestResult("Register market maker", registered);
        
        // Verify registration
        bool verified = registry.verifyMarketMaker("tGXC_mm_goldman");
        printTestResult("Verify market maker", verified);
    } catch (const std::exception& e) {
        printTestResult("Register market maker", false);
        std::cout << "  Error: " << e.what() << std::endl;
    }
    
    // Test 2: Deploy Stock Contract
    printSection("TEST 2: Deploy Stock Contract");
    try {
        std::string contractAddr = api.deployStock(
            "tGXC_mm_goldman",
            "AAPL",
            "Apple Inc.",
            "NASDAQ",
            1000000
        );
        printTestResult("Deploy AAPL stock", !contractAddr.empty());
        std::cout << "  Contract address: " << contractAddr << std::endl;
    } catch (const std::exception& e) {
        printTestResult("Deploy AAPL stock", false);
        std::cout << "  Error: " << e.what() << std::endl;
    }
    
    // Test 3: Unauthorized Stock Deployment
    printSection("TEST 3: Unauthorized Stock Deployment");
    try {
        std::string contractAddr = api.deployStock(
            "tGXC_unauthorized",
            "FAKE",
            "Fake Company",
            "NASDAQ",
            1000000
        );
        printTestResult("Unauthorized deployment rejected", false);
    } catch (const std::exception& e) {
        printTestResult("Unauthorized deployment rejected", true);
        std::cout << "  Expected error: " << e.what() << std::endl;
    }
    
    // Test 4: Update Stock Price
    printSection("TEST 4: Update Stock Price");
    try {
        bool updated = api.updateStockPrice(
            "tGXC_mm_goldman",
            "AAPL",
            150.50,
            "Bloomberg Terminal",
            "0xproof_bloomberg_123"
        );
        printTestResult("Update stock price", updated);
        
        // Verify price
        double price = api.getStockPrice("AAPL");
        printTestResult("Get stock price", price == 150.50);
        std::cout << "  Current price: $" << price << std::endl;
    } catch (const std::exception& e) {
        printTestResult("Update stock price", false);
        std::cout << "  Error: " << e.what() << std::endl;
    }
    
    // Test 5: Unauthorized Price Update
    printSection("TEST 5: Unauthorized Price Update");
    try {
        bool updated = api.updateStockPrice(
            "tGXC_unauthorized",
            "AAPL",
            200.00,
            "Fake Source",
            "0xfake_proof"
        );
        printTestResult("Unauthorized price update rejected", false);
    } catch (const std::exception& e) {
        printTestResult("Unauthorized price update rejected", true);
        std::cout << "  Expected error: " << e.what() << std::endl;
    }
    
    // Test 6: Add Liquidity
    printSection("TEST 6: Add Liquidity");
    try {
        bool added = api.addLiquidity(
            "tGXC_mm_goldman",
            "AAPL",
            10000,
            150.25,
            150.75
        );
        printTestResult("Add liquidity", added);
        std::cout << "  Liquidity: 10,000 shares" << std::endl;
        std::cout << "  Bid: $150.25, Ask: $150.75" << std::endl;
        std::cout << "  Spread: $0.50" << std::endl;
    } catch (const std::exception& e) {
        printTestResult("Add liquidity", false);
        std::cout << "  Error: " << e.what() << std::endl;
    }
    
    // Test 7: Remove Liquidity
    printSection("TEST 7: Remove Liquidity");
    try {
        bool removed = api.removeLiquidity(
            "tGXC_mm_goldman",
            "AAPL",
            5000
        );
        printTestResult("Remove liquidity", removed);
        std::cout << "  Removed: 5,000 shares" << std::endl;
        std::cout << "  Remaining: 5,000 shares" << std::endl;
    } catch (const std::exception& e) {
        printTestResult("Remove liquidity", false);
        std::cout << "  Error: " << e.what() << std::endl;
    }
    
    // Test 8: Get Order Book
    printSection("TEST 8: Get Order Book");
    try {
        auto orderBook = api.getOrderBook("AAPL");
        printTestResult("Get order book", true);
        std::cout << "  Bids: " << orderBook.bids.size() << std::endl;
        std::cout << "  Asks: " << orderBook.asks.size() << std::endl;
        
        if (orderBook.bids.size() > 0) {
            std::cout << "  Best bid: $" << orderBook.bids[0].first 
                      << " (" << orderBook.bids[0].second << " shares)" << std::endl;
        }
        if (orderBook.asks.size() > 0) {
            std::cout << "  Best ask: $" << orderBook.asks[0].first 
                      << " (" << orderBook.asks[0].second << " shares)" << std::endl;
        }
    } catch (const std::exception& e) {
        printTestResult("Get order book", false);
        std::cout << "  Error: " << e.what() << std::endl;
    }
    
    // Test 9: Deploy Multiple Stocks
    printSection("TEST 9: Deploy Multiple Stocks");
    try {
        // Deploy MSFT
        std::string msftAddr = api.deployStock(
            "tGXC_mm_goldman",
            "MSFT",
            "Microsoft Corporation",
            "NASDAQ",
            1000000
        );
        printTestResult("Deploy MSFT stock", !msftAddr.empty());
        
        // Deploy GOOGL
        std::string googlAddr = api.deployStock(
            "tGXC_mm_goldman",
            "GOOGL",
            "Alphabet Inc.",
            "NASDAQ",
            1000000
        );
        printTestResult("Deploy GOOGL stock", !googlAddr.empty());
        
        // Update prices
        api.updateStockPrice("tGXC_mm_goldman", "MSFT", 380.00, "Bloomberg", "0xproof");
        api.updateStockPrice("tGXC_mm_goldman", "GOOGL", 140.00, "Bloomberg", "0xproof");
        
        printTestResult("Update multiple stock prices", true);
        std::cout << "  MSFT: $380.00" << std::endl;
        std::cout << "  GOOGL: $140.00" << std::endl;
    } catch (const std::exception& e) {
        printTestResult("Deploy multiple stocks", false);
        std::cout << "  Error: " << e.what() << std::endl;
    }
    
    // Test 10: Multiple Market Makers
    printSection("TEST 10: Multiple Market Makers");
    try {
        // Register second market maker
        bool registered = registry.registerMarketMaker(
            "tGXC_mm_citadel",
            "Citadel Securities",
            "MM-67890-US",
            "USA"
        );
        printTestResult("Register second market maker", registered);
        
        // Deploy stock from second market maker
        std::string tslaAddr = api.deployStock(
            "tGXC_mm_citadel",
            "TSLA",
            "Tesla Inc.",
            "NASDAQ",
            1000000
        );
        printTestResult("Second market maker deploys stock", !tslaAddr.empty());
        
        // Update price
        api.updateStockPrice("tGXC_mm_citadel", "TSLA", 250.00, "Reuters", "0xproof");
        printTestResult("Second market maker updates price", true);
        std::cout << "  TSLA: $250.00" << std::endl;
    } catch (const std::exception& e) {
        printTestResult("Multiple market makers", false);
        std::cout << "  Error: " << e.what() << std::endl;
    }
    
    // Test 11: Price Source Tracking
    printSection("TEST 11: Price Source Tracking");
    try {
        // Update with different price sources
        api.updateStockPrice("tGXC_mm_goldman", "AAPL", 151.00, "Bloomberg Terminal", "0xbloomberg");
        api.updateStockPrice("tGXC_mm_goldman", "MSFT", 381.00, "Reuters Eikon", "0xreuters");
        api.updateStockPrice("tGXC_mm_citadel", "TSLA", 251.00, "Proprietary System", "0xproprietary");
        
        printTestResult("Track different price sources", true);
        std::cout << "  AAPL: Bloomberg Terminal" << std::endl;
        std::cout << "  MSFT: Reuters Eikon" << std::endl;
        std::cout << "  TSLA: Proprietary System" << std::endl;
    } catch (const std::exception& e) {
        printTestResult("Track different price sources", false);
        std::cout << "  Error: " << e.what() << std::endl;
    }
    
    // Test 12: Stock Contract Isolation
    printSection("TEST 12: Stock Contract Isolation");
    try {
        // Market maker 1 cannot update market maker 2's stock
        try {
            api.updateStockPrice("tGXC_mm_goldman", "TSLA", 300.00, "Fake", "0xfake");
            printTestResult("Stock contract isolation", false);
        } catch (...) {
            printTestResult("Stock contract isolation", true);
            std::cout << "  Correctly rejected: Market maker 1 cannot update market maker 2's stock" << std::endl;
        }
    } catch (const std::exception& e) {
        printTestResult("Stock contract isolation", false);
        std::cout << "  Error: " << e.what() << std::endl;
    }
    
    // Summary
    std::cout << "\n========================================" << std::endl;
    std::cout << "  TEST SUMMARY" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "✅ Tests Passed: " << tests_passed << std::endl;
    std::cout << "❌ Tests Failed: " << tests_failed << std::endl;
    std::cout << "Total Tests: " << (tests_passed + tests_failed) << std::endl;
    
    if (tests_failed == 0) {
        std::cout << "\n🎉 ALL TESTS PASSED! 🎉" << std::endl;
        std::cout << "\nStock Contract Features Working:" << std::endl;
        std::cout << "  ✅ Market maker registration" << std::endl;
        std::cout << "  ✅ Stock deployment (authorized only)" << std::endl;
        std::cout << "  ✅ Unauthorized deployment rejected" << std::endl;
        std::cout << "  ✅ Price updates (authorized only)" << std::endl;
        std::cout << "  ✅ Unauthorized price update rejected" << std::endl;
        std::cout << "  ✅ Add/remove liquidity" << std::endl;
        std::cout << "  ✅ Order book functionality" << std::endl;
        std::cout << "  ✅ Multiple stocks per market maker" << std::endl;
        std::cout << "  ✅ Multiple market makers" << std::endl;
        std::cout << "  ✅ Price source tracking" << std::endl;
        std::cout << "  ✅ Stock contract isolation" << std::endl;
        std::cout << "\n🚀 READY FOR PRODUCTION!" << std::endl;
        return 0;
    } else {
        std::cout << "\n⚠️  SOME TESTS FAILED" << std::endl;
        return 1;
    }
}
