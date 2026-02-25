#include "../include/ReversalFeePool.h"
#include "../include/Logger.h"
#include <sstream>
#include <algorithm>

ReversalFeePool::ReversalFeePool() 
    : current_balance(0), total_funded(0), total_spent(0), total_reversals(0),
      min_balance_warning(100000000), // 1 GXC (100M satoshis)
      min_fee_amount(1000) {           // 0.00001 GXC
    LOG_INFO("Reversal Fee Pool created");
}

ReversalFeePool::~ReversalFeePool() {
    LOG_INFO("Reversal Fee Pool destroyed");
}

void ReversalFeePool::initialize(const std::string& address) {
    pool_address = address;
    LOG_INFO("Reversal Fee Pool initialized with address: " + address);
    LOG_INFO("Dashboard owners should send funds to this address to fund reversals");
}

bool ReversalFeePool::hasSufficientFunds(uint64_t fee_amount) const {
    return current_balance >= fee_amount;
}

bool ReversalFeePool::isBalanceLow() const {
    return current_balance < min_balance_warning;
}

void ReversalFeePool::updateBalanceFromBlockchain(uint64_t new_balance) {
    uint64_t old_balance = current_balance;
    current_balance = new_balance;
    
    LOG_INFO("Reversal Fee Pool balance updated: " + 
             std::to_string(old_balance) + " -> " + std::to_string(new_balance));
    
    if (isBalanceLow()) {
        LOG_WARNING("Reversal Fee Pool balance is LOW: " + std::to_string(current_balance) + 
                   " (threshold: " + std::to_string(min_balance_warning) + ")");
        LOG_WARNING("Dashboard owners should fund the pool at address: " + pool_address);
    }
}

uint64_t ReversalFeePool::depositTransactionFeeSplit(const std::string& tx_hash,
                                                     uint64_t fee_amount,
                                                     double split_percentage) {
    // Calculate pool share (10-20% of transaction fee)
    uint64_t pool_share = static_cast<uint64_t>(fee_amount * split_percentage);
    
    if (pool_share > 0) {
        // Update balance
        current_balance += pool_share;
        total_funded += pool_share;
        
        // Record as automatic funding
        FundingRecord record;
        record.tx_hash = tx_hash;
        record.from_address = "SYSTEM_TX_FEE_SPLIT";
        record.amount = pool_share;
        record.timestamp = std::time(nullptr);
        record.note = "Automatic deposit from transaction fee (" + 
                     std::to_string(split_percentage * 100) + "% of " + 
                     std::to_string(fee_amount) + ")";
        
        funding_history.push_back(record);
        
        LOG_DEBUG("System Pool: Deposited " + std::to_string(pool_share) + 
                 " from TX fee split (TX: " + tx_hash + ")");
    }
    
    return pool_share;
}

uint64_t ReversalFeePool::depositReversalExecutionFee(const std::string& reversal_tx_hash,
                                                      uint64_t recovered_amount,
                                                      double fee_percentage) {
    // Calculate execution fee (0.1-0.3% of recovered amount)
    uint64_t execution_fee = static_cast<uint64_t>(recovered_amount * fee_percentage);
    
    if (execution_fee > 0) {
        // Update balance
        current_balance += execution_fee;
        total_funded += execution_fee;
        
        // Record as automatic funding
        FundingRecord record;
        record.tx_hash = reversal_tx_hash;
        record.from_address = "SYSTEM_REVERSAL_FEE";
        record.amount = execution_fee;
        record.timestamp = std::time(nullptr);
        record.note = "Reversal execution fee (" + 
                     std::to_string(fee_percentage * 100) + "% of " + 
                     std::to_string(recovered_amount) + " recovered)";
        
        funding_history.push_back(record);
        
        LOG_INFO("System Pool: Deposited " + std::to_string(execution_fee) + 
                " from reversal execution fee (Reversal: " + reversal_tx_hash + ")");
    }
    
    return execution_fee;
}

void ReversalFeePool::recordFunding(const std::string& tx_hash, 
                                   const std::string& from_address,
                                   uint64_t amount,
                                   const std::string& note) {
    FundingRecord record;
    record.tx_hash = tx_hash;
    record.from_address = from_address;
    record.amount = amount;
    record.timestamp = std::time(nullptr);
    record.note = note.empty() ? "Manual funding (legacy/emergency)" : note;
    
    funding_history.push_back(record);
    total_funded += amount;
    
    LOG_INFO("Reversal Fee Pool manually funded: " + std::to_string(amount) + 
             " from " + from_address + " (TX: " + tx_hash + ")");
    
    if (!note.empty()) {
        LOG_INFO("Funding note: " + note);
    }
}

bool ReversalFeePool::deductFee(const std::string& reversal_tx_hash,
                               uint64_t fee_amount,
                               const std::string& victim_address,
                               const std::string& admin_id) {
    // Check if sufficient funds
    if (!hasSufficientFunds(fee_amount)) {
        LOG_ERROR("Reversal Fee Pool: Insufficient funds for fee. " +
                 std::string("Required: ") + std::to_string(fee_amount) + 
                 ", Available: " + std::to_string(current_balance));
        return false;
    }
    
    // Check minimum fee
    if (fee_amount < min_fee_amount) {
        LOG_ERROR("Reversal Fee Pool: Fee amount below minimum. " +
                 std::string("Fee: ") + std::to_string(fee_amount) + 
                 ", Minimum: " + std::to_string(min_fee_amount));
        return false;
    }
    
    // Deduct fee
    current_balance -= fee_amount;
    total_spent += fee_amount;
    total_reversals++;
    
    // Record fee payment
    ReversalFeeRecord record;
    record.reversal_tx_hash = reversal_tx_hash;
    record.fee_amount = fee_amount;
    record.timestamp = std::time(nullptr);
    record.victim_address = victim_address;
    record.admin_id = admin_id;
    
    fee_history.push_back(record);
    
    LOG_INFO("Reversal Fee Pool: Fee deducted: " + std::to_string(fee_amount) + 
             " for reversal " + reversal_tx_hash);
    LOG_INFO("Reversal Fee Pool: New balance: " + std::to_string(current_balance));
    
    // Warn if balance is low
    if (isBalanceLow()) {
        LOG_WARNING("Reversal Fee Pool balance is LOW after fee deduction!");
        LOG_WARNING("Dashboard owners should fund the pool at address: " + pool_address);
    }
    
    return true;
}

uint64_t ReversalFeePool::getAverageFee() const {
    if (total_reversals == 0) {
        return 0;
    }
    return total_spent / total_reversals;
}

ReversalFeePool::PoolStats ReversalFeePool::getStatistics() const {
    PoolStats stats;
    stats.pool_address = pool_address;
    stats.current_balance = current_balance;
    stats.total_funded = total_funded;
    stats.total_spent = total_spent;
    stats.total_reversals = total_reversals;
    stats.average_fee = getAverageFee();
    stats.is_balance_low = isBalanceLow();
    stats.funding_count = static_cast<uint32_t>(funding_history.size());
    
    if (!funding_history.empty()) {
        const FundingRecord& last = funding_history.back();
        stats.last_funding_amount = last.amount;
        stats.last_funding_timestamp = last.timestamp;
    } else {
        stats.last_funding_amount = 0;
        stats.last_funding_timestamp = 0;
    }
    
    return stats;
}

std::string ReversalFeePool::exportState() const {
    std::stringstream ss;
    
    // Pool info
    ss << "POOL_ADDRESS:" << pool_address << "\n";
    ss << "CURRENT_BALANCE:" << current_balance << "\n";
    ss << "TOTAL_FUNDED:" << total_funded << "\n";
    ss << "TOTAL_SPENT:" << total_spent << "\n";
    ss << "TOTAL_REVERSALS:" << total_reversals << "\n";
    ss << "MIN_BALANCE_WARNING:" << min_balance_warning << "\n";
    ss << "MIN_FEE_AMOUNT:" << min_fee_amount << "\n";
    
    // Funding history
    ss << "FUNDING_COUNT:" << funding_history.size() << "\n";
    for (const auto& record : funding_history) {
        ss << "FUNDING:" << record.tx_hash << "|" 
           << record.from_address << "|"
           << record.amount << "|"
           << record.timestamp << "|"
           << record.note << "\n";
    }
    
    // Fee history
    ss << "FEE_COUNT:" << fee_history.size() << "\n";
    for (const auto& record : fee_history) {
        ss << "FEE:" << record.reversal_tx_hash << "|"
           << record.fee_amount << "|"
           << record.timestamp << "|"
           << record.victim_address << "|"
           << record.admin_id << "\n";
    }
    
    return ss.str();
}

void ReversalFeePool::importState(const std::string& state) {
    std::istringstream iss(state);
    std::string line;
    
    funding_history.clear();
    fee_history.clear();
    
    while (std::getline(iss, line)) {
        if (line.empty()) continue;
        
        size_t colonPos = line.find(':');
        if (colonPos == std::string::npos) continue;
        
        std::string key = line.substr(0, colonPos);
        std::string value = line.substr(colonPos + 1);
        
        if (key == "POOL_ADDRESS") {
            pool_address = value;
        } else if (key == "CURRENT_BALANCE") {
            current_balance = std::stoull(value);
        } else if (key == "TOTAL_FUNDED") {
            total_funded = std::stoull(value);
        } else if (key == "TOTAL_SPENT") {
            total_spent = std::stoull(value);
        } else if (key == "TOTAL_REVERSALS") {
            total_reversals = std::stoul(value);
        } else if (key == "MIN_BALANCE_WARNING") {
            min_balance_warning = std::stoull(value);
        } else if (key == "MIN_FEE_AMOUNT") {
            min_fee_amount = std::stoull(value);
        } else if (key == "FUNDING") {
            // Parse funding record
            std::istringstream recordStream(value);
            std::string tx_hash, from_address, amount_str, timestamp_str, note;
            
            std::getline(recordStream, tx_hash, '|');
            std::getline(recordStream, from_address, '|');
            std::getline(recordStream, amount_str, '|');
            std::getline(recordStream, timestamp_str, '|');
            std::getline(recordStream, note);
            
            FundingRecord record;
            record.tx_hash = tx_hash;
            record.from_address = from_address;
            record.amount = std::stoull(amount_str);
            record.timestamp = std::stoull(timestamp_str);
            record.note = note;
            
            funding_history.push_back(record);
        } else if (key == "FEE") {
            // Parse fee record
            std::istringstream recordStream(value);
            std::string reversal_tx_hash, fee_amount_str, timestamp_str, victim_address, admin_id;
            
            std::getline(recordStream, reversal_tx_hash, '|');
            std::getline(recordStream, fee_amount_str, '|');
            std::getline(recordStream, timestamp_str, '|');
            std::getline(recordStream, victim_address, '|');
            std::getline(recordStream, admin_id);
            
            ReversalFeeRecord record;
            record.reversal_tx_hash = reversal_tx_hash;
            record.fee_amount = std::stoull(fee_amount_str);
            record.timestamp = std::stoull(timestamp_str);
            record.victim_address = victim_address;
            record.admin_id = admin_id;
            
            fee_history.push_back(record);
        }
    }
    
    LOG_INFO("Reversal Fee Pool state imported");
    LOG_INFO("Pool address: " + pool_address);
    LOG_INFO("Current balance: " + std::to_string(current_balance));
    LOG_INFO("Total funded: " + std::to_string(total_funded));
    LOG_INFO("Total spent: " + std::to_string(total_spent));
    LOG_INFO("Total reversals: " + std::to_string(total_reversals));
}
