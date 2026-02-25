#pragma once

#include "blockchain.h"
#include "json.hpp"
#include <string>
#include <vector>

using json = nlohmann::json;

/**
 * Explorer API
 * Provides transparent access to all blockchain data
 * Users can see everything: blocks, transactions, validators, stocks, etc.
 */
class ExplorerAPI {
private:
    Blockchain* blockchain;
    
public:
    ExplorerAPI(Blockchain* bc) : blockchain(bc) {}
    
    // ===== BLOCK EXPLORER =====
    
    /**
     * Get latest blocks
     * Returns: Last N blocks with full details
     */
    json getLatestBlocks(int limit = 10);
    
    /**
     * Get block by height or hash
     * Returns: Complete block details
     */
    json getBlock(const std::string& blockIdentifier);
    
    /**
     * Get block transactions
     * Returns: All transactions in a block
     */
    json getBlockTransactions(const std::string& blockIdentifier);
    
    /**
     * Search blocks
     * Returns: Blocks matching criteria
     */
    json searchBlocks(const std::string& query);
    
    // ===== TRANSACTION EXPLORER =====
    
    /**
     * Get transaction by hash
     * Returns: Complete transaction details with traceability
     */
    json getTransaction(const std::string& txHash);
    
    /**
     * Get transaction chain (traceability)
     * Returns: Complete chain from genesis to this transaction
     */
    json getTransactionChain(const std::string& txHash);
    
    /**
     * Get latest transactions
     * Returns: Last N transactions across all blocks
     */
    json getLatestTransactions(int limit = 20);
    
    /**
     * Get transactions by address
     * Returns: All transactions involving this address
     */
    json getAddressTransactions(const std::string& address, int limit = 50);
    
    /**
     * Search transactions
     * Returns: Transactions matching criteria
     */
    json searchTransactions(const std::string& query);
    
    // ===== ADDRESS EXPLORER =====
    
    /**
     * Get address details
     * Returns: Balance, transaction count, etc.
     */
    json getAddressDetails(const std::string& address);
    
    /**
     * Get address balance
     * Returns: Current balance and UTXOs
     */
    json getAddressBalance(const std::string& address);
    
    /**
     * Get address UTXOs
     * Returns: All unspent transaction outputs
     */
    json getAddressUtxos(const std::string& address);
    
    /**
     * Get richest addresses
     * Returns: Top N addresses by balance
     */
    json getRichestAddresses(int limit = 100);
    
    // ===== VALIDATOR EXPLORER =====
    
    /**
     * Get all validators
     * Returns: Complete list of validators with details
     */
    json getAllValidators();
    
    /**
     * Get validator details
     * Returns: Validator info, stake, blocks mined, etc.
     */
    json getValidatorDetails(const std::string& address);
    
    /**
     * Get validator blocks
     * Returns: All blocks mined by this validator
     */
    json getValidatorBlocks(const std::string& address, int limit = 50);
    
    /**
     * Get top validators
     * Returns: Top N validators by stake
     */
    json getTopValidators(int limit = 20);
    
    // ===== STOCK CONTRACT EXPLORER =====
    
    /**
     * Get all stock contracts
     * Returns: All deployed stocks
     */
    json getAllStocks();
    
    /**
     * Get stock details
     * Returns: Stock info, price, market cap, etc.
     */
    json getStockDetails(const std::string& ticker);
    
    /**
     * Get stock transactions
     * Returns: All buy/sell transactions for this stock
     */
    json getStockTransactions(const std::string& ticker, int limit = 50);
    
    /**
     * Get stock holders
     * Returns: All addresses holding this stock
     */
    json getStockHolders(const std::string& ticker);
    
    /**
     * Get market maker stocks
     * Returns: All stocks deployed by this market maker
     */
    json getMarketMakerStocks(const std::string& makerAddress);
    
    // ===== STATISTICS =====
    
    /**
     * Get blockchain statistics
     * Returns: Total blocks, transactions, addresses, etc.
     */
    json getBlockchainStats();
    
    /**
     * Get network statistics
     * Returns: Hashrate, difficulty, validators, etc.
     */
    json getNetworkStats();
    
    /**
     * Get supply statistics
     * Returns: Total supply, circulating supply, burned, etc.
     */
    json getSupplyStats();
    
    /**
     * Get market statistics
     * Returns: Stock market stats, trading volume, etc.
     */
    json getMarketStats();
    
    // ===== SEARCH =====
    
    /**
     * Universal search
     * Searches blocks, transactions, addresses, validators, stocks
     * Returns: All matching results
     */
    json search(const std::string& query);
    
    // ===== CHARTS =====
    
    /**
     * Get block time chart
     * Returns: Block times over time
     */
    json getBlockTimeChart(int days = 7);
    
    /**
     * Get transaction volume chart
     * Returns: Transaction count over time
     */
    json getTransactionVolumeChart(int days = 7);
    
    /**
     * Get validator distribution chart
     * Returns: Stake distribution among validators
     */
    json getValidatorDistributionChart();
    
    /**
     * Get stock price chart
     * Returns: Stock price history
     */
    json getStockPriceChart(const std::string& ticker, int days = 30);
};
