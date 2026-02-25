#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <ctime>

/**
 * Reversal Fee Pool
 * 
 * Manages the system pool used to fund fraud detection and reversal operations.
 * 
 * SELF-SUSTAINING MODEL:
 * The pool is automatically funded through transaction fee splits, not external wallets.
 * 
 * Funding Sources:
 * 1. Transaction Fee Split: 10-20% of all transaction fees go to system pool
 * 2. Reversal Execution Fee: 0.1-0.3% of recovered funds go to system pool
 * 
 * Key Principles:
 * 1. Pool receives automatic deposits from every transaction
 * 2. No manual funding required from dashboard owners
 * 3. System is self-sustaining through fee economics
 * 4. Pool balance grows with network activity
 * 5. Admins approve FACTS (fraud legitimacy), not execution
 * 6. Protocol validates feasibility and executes automatically
 */

struct FundingRecord {
    std::string tx_hash;           // Transaction hash of funding
    std::string from_address;      // Who funded (dashboard owner)
    uint64_t amount;               // Amount funded
    uint64_t timestamp;            // When funded
    std::string note;              // Optional note
    
    FundingRecord() : amount(0), timestamp(0) {}
};

struct ReversalFeeRecord {
    std::string reversal_tx_hash;  // Reversal transaction hash
    uint64_t fee_amount;           // Fee paid
    uint64_t timestamp;            // When paid
    std::string victim_address;    // Who was helped
    std::string admin_id;          // Admin who approved
    
    ReversalFeeRecord() : fee_amount(0), timestamp(0) {}
};

class ReversalFeePool {
private:
    // Pool wallet address (like any normal address)
    std::string pool_address;
    
    // Current balance (tracked separately for quick access)
    uint64_t current_balance;
    
    // Funding history
    std::vector<FundingRecord> funding_history;
    
    // Fee spending history
    std::vector<ReversalFeeRecord> fee_history;
    
    // Statistics
    uint64_t total_funded;         // Total amount ever funded
    uint64_t total_spent;          // Total fees ever paid
    uint32_t total_reversals;      // Total reversals funded
    
    // Configuration
    uint64_t min_balance_warning;  // Warn when balance below this
    uint64_t min_fee_amount;       // Minimum fee per reversal
    
public:
    ReversalFeePool();
    ~ReversalFeePool();
    
    // ===== INITIALIZATION =====
    
    /**
     * Initialize pool with a wallet address
     * This address is where dashboard owners send funds
     */
    void initialize(const std::string& address);
    
    /**
     * Get the pool wallet address
     * Dashboard owners send funds to this address
     */
    std::string getPoolAddress() const { return pool_address; }
    
    // ===== BALANCE MANAGEMENT =====
    
    /**
     * Get current pool balance
     */
    uint64_t getBalance() const { return current_balance; }
    
    /**
     * Check if pool has sufficient funds for a fee
     */
    bool hasSufficientFunds(uint64_t fee_amount) const;
    
    /**
     * Check if balance is below warning threshold
     */
    bool isBalanceLow() const;
    
    /**
     * Update balance from blockchain
     * Call this to sync pool balance with actual blockchain balance
     */
    void updateBalanceFromBlockchain(uint64_t new_balance);
    
    // ===== AUTOMATIC FUNDING (Self-Sustaining) =====
    
    /**
     * Deposit transaction fee split to pool
     * Called automatically when processing each transaction
     * 
     * @param tx_hash Transaction hash
     * @param fee_amount Total transaction fee
     * @param split_percentage Percentage going to pool (10-20%)
     * @return Amount deposited to pool
     */
    uint64_t depositTransactionFeeSplit(const std::string& tx_hash,
                                        uint64_t fee_amount,
                                        double split_percentage = 0.15);
    
    /**
     * Deposit reversal execution fee to pool
     * Called when a reversal is successfully executed
     * 
     * @param reversal_tx_hash Reversal transaction hash
     * @param recovered_amount Amount recovered
     * @param fee_percentage Fee percentage (0.1-0.3%)
     * @return Amount deposited to pool
     */
    uint64_t depositReversalExecutionFee(const std::string& reversal_tx_hash,
                                         uint64_t recovered_amount,
                                         double fee_percentage = 0.002);
    
    /**
     * Record a manual funding transaction (legacy/emergency)
     * Called when dashboard owner manually sends funds to pool address
     */
    void recordFunding(const std::string& tx_hash, 
                      const std::string& from_address,
                      uint64_t amount,
                      const std::string& note = "");
    
    /**
     * Get funding history
     */
    std::vector<FundingRecord> getFundingHistory() const { return funding_history; }
    
    /**
     * Get total amount funded
     */
    uint64_t getTotalFunded() const { return total_funded; }
    
    // ===== FEE DEDUCTION (System Use) =====
    
    /**
     * Deduct fee for a reversal transaction
     * Returns true if successful, false if insufficient funds
     */
    bool deductFee(const std::string& reversal_tx_hash,
                   uint64_t fee_amount,
                   const std::string& victim_address,
                   const std::string& admin_id);
    
    /**
     * Get fee spending history
     */
    std::vector<ReversalFeeRecord> getFeeHistory() const { return fee_history; }
    
    /**
     * Get total fees spent
     */
    uint64_t getTotalSpent() const { return total_spent; }
    
    // ===== STATISTICS =====
    
    /**
     * Get total number of reversals funded
     */
    uint32_t getTotalReversals() const { return total_reversals; }
    
    /**
     * Get average fee per reversal
     */
    uint64_t getAverageFee() const;
    
    /**
     * Get pool statistics
     */
    struct PoolStats {
        std::string pool_address;
        uint64_t current_balance;
        uint64_t total_funded;
        uint64_t total_spent;
        uint32_t total_reversals;
        uint64_t average_fee;
        bool is_balance_low;
        uint32_t funding_count;
        uint64_t last_funding_amount;
        uint64_t last_funding_timestamp;
    };
    
    PoolStats getStatistics() const;
    
    // ===== CONFIGURATION =====
    
    /**
     * Set minimum balance warning threshold
     */
    void setMinBalanceWarning(uint64_t amount) { min_balance_warning = amount; }
    
    /**
     * Get minimum balance warning threshold
     */
    uint64_t getMinBalanceWarning() const { return min_balance_warning; }
    
    /**
     * Set minimum fee amount
     */
    void setMinFeeAmount(uint64_t amount) { min_fee_amount = amount; }
    
    /**
     * Get minimum fee amount
     */
    uint64_t getMinFeeAmount() const { return min_fee_amount; }
    
    // ===== PERSISTENCE =====
    
    /**
     * Export pool state for persistence
     */
    std::string exportState() const;
    
    /**
     * Import pool state from persistence
     */
    void importState(const std::string& state);
};
