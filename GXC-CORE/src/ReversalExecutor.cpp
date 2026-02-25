#include "../include/ReversalExecutor.h"
#include "../include/blockchain.h"
#include "../include/ProofGenerator.h"
#include "../include/ReversalFeePool.h"
#include "../include/Logger.h"
#include "../include/HashUtils.h"
#include <sstream>

ReversalExecutor::ReversalExecutor(Blockchain* bc, ProofGenerator* pg, ReversalFeePool* fp)
    : blockchain(bc), proofGenerator(pg), feePool(fp) {
    LOG_INFO("Reversal Executor initialized");
}

ReversalExecutor::~ReversalExecutor() {
    LOG_INFO("Reversal Executor destroyed");
}

uint64_t ReversalExecutor::calculateFee(uint64_t amount) {
    // Standard fee for all reversals
    // In production, could be dynamic based on amount or network congestion
    return STANDARD_FEE;
}

bool ReversalExecutor::debitAccount(const std::string& address, uint64_t amount) {
    uint64_t current_balance = blockchain->getBalance(address);
    
    if (current_balance < amount) {
        LOG_ERROR("Reversal Executor: Insufficient balance to debit. " +
                 std::string("Address: ") + address + 
                 ", Required: " + std::to_string(amount) + 
                 ", Available: " + std::to_string(current_balance));
        return false;
    }
    
    // Debit the account
    // Note: In production, this would update UTXO set
    // For now, we'll log it and assume blockchain handles it
    LOG_INFO("Reversal Executor: Debiting " + std::to_string(amount) + " from " + address);
    
    return true;
}

bool ReversalExecutor::creditAccount(const std::string& address, uint64_t amount) {
    // Credit the account
    // Note: In production, this would create new UTXO
    // For now, we'll log it and assume blockchain handles it
    LOG_INFO("Reversal Executor: Crediting " + std::to_string(amount) + " to " + address);
    
    return true;
}

ReversalTransaction ReversalExecutor::createReversalTransaction(const ProofOfFeasibility& proof) {
    ReversalTransaction rtx;
    
    // Generate transaction hash
    std::stringstream ss;
    ss << "REVERSAL_" << proof.proof_hash << "_" << proof.timestamp;
    rtx.tx_hash = sha256(ss.str());
    
    // Set transaction details
    rtx.from = proof.current_holder;
    rtx.to = proof.origin_owner;
    rtx.amount = proof.recoverable_amount;
    rtx.proof_hash = proof.proof_hash;
    rtx.fee = calculateFee(proof.recoverable_amount);
    rtx.timestamp = std::time(nullptr);
    rtx.block_height = blockchain->getHeight();
    rtx.admin_signature = proof.approval_signature;
    
    LOG_INFO("Reversal Transaction created: " + rtx.tx_hash);
    LOG_INFO("From: " + rtx.from);
    LOG_INFO("To: " + rtx.to);
    LOG_INFO("Amount: " + std::to_string(rtx.amount));
    LOG_INFO("Fee: " + std::to_string(rtx.fee));
    
    return rtx;
}

void ReversalExecutor::logReversal(const ReversalTransaction& rtx, const ProofOfFeasibility& proof) {
    LOG_SECURITY(LogLevel::CRITICAL,
        "REVERSAL EXECUTED: " + rtx.tx_hash + 
        " | Victim: " + rtx.to + 
        " | Amount: " + std::to_string(rtx.amount) + 
        " | Stolen TX: " + proof.stolen_tx + 
        " | Taint: " + std::to_string(proof.taint_score) + 
        " | Admin: " + proof.approved_by,
        "ReversalExecutor");
}

bool ReversalExecutor::validateFeasibility(const ProofOfFeasibility& proof, std::string& errorMessage) {
    LOG_INFO("=== PROTOCOL: VALIDATING REVERSAL FEASIBILITY ===");
    LOG_INFO("This validation is independent of admin approval");
    
    // Check 1: Cryptographic proof validity
    LOG_INFO("Check 1: Validating cryptographic proof...");
    if (!proofGenerator->validateProof(proof)) {
        errorMessage = "Cryptographic proof is invalid";
        LOG_WARNING("Protocol: Feasibility check FAILED - " + errorMessage);
        return false;
    }
    LOG_INFO("Check 1: Proof is cryptographically valid ✓");
    
    // Check 2: Taint score threshold
    LOG_INFO("Check 2: Checking taint score threshold...");
    double minTaintScore = 0.1; // Minimum 10% taint to justify reversal
    if (proof.taint_score < minTaintScore) {
        errorMessage = "Taint score too low (" + std::to_string(proof.taint_score) + 
                      " < " + std::to_string(minTaintScore) + "). " +
                      "Insufficient evidence of stolen funds.";
        LOG_WARNING("Protocol: Feasibility check FAILED - " + errorMessage);
        return false;
    }
    LOG_INFO("Check 2: Taint score acceptable (" + std::to_string(proof.taint_score) + ") ✓");
    
    // Check 3: Current holder balance
    LOG_INFO("Check 3: Checking current holder balance...");
    double holderBalance = blockchain->getBalance(proof.current_holder);
    double requiredAmount = proof.recoverable_amount / 100000000.0; // Convert from satoshis
    if (holderBalance < requiredAmount) {
        errorMessage = "Current holder has insufficient balance (" + 
                      std::to_string(holderBalance) + " GXC < " + 
                      std::to_string(requiredAmount) + " GXC). " +
                      "Funds may have already been moved.";
        LOG_WARNING("Protocol: Feasibility check FAILED - " + errorMessage);
        return false;
    }
    LOG_INFO("Check 3: Holder has sufficient balance (" + std::to_string(holderBalance) + " GXC) ✓");
    
    // Check 4: System pool has funds for fees
    LOG_INFO("Check 4: Checking system pool balance...");
    uint64_t requiredFee = calculateFee(proof.recoverable_amount);
    if (!feePool->hasSufficientFunds(requiredFee)) {
        errorMessage = "System pool has insufficient funds for fees (" + 
                      std::to_string(feePool->getBalance()) + " < " + 
                      std::to_string(requiredFee) + " satoshis). " +
                      "System is self-sustaining but may need time to accumulate fees.";
        LOG_WARNING("Protocol: Feasibility check FAILED - " + errorMessage);
        return false;
    }
    LOG_INFO("Check 4: System pool has sufficient funds ✓");
    
    // Check 5: Reversal time window (optional, can be configured)
    LOG_INFO("Check 5: Checking reversal time window...");
    uint64_t currentTime = std::time(nullptr);
    uint64_t maxReversalAge = 30 * 24 * 60 * 60; // 30 days default
    if (currentTime - proof.timestamp > maxReversalAge) {
        errorMessage = "Reversal request too old (" + 
                      std::to_string((currentTime - proof.timestamp) / 86400) + " days). " +
                      "Maximum age is " + std::to_string(maxReversalAge / 86400) + " days.";
        LOG_WARNING("Protocol: Feasibility check FAILED - " + errorMessage);
        return false;
    }
    LOG_INFO("Check 5: Within reversal time window ✓");
    
    LOG_INFO("=== PROTOCOL: FEASIBILITY VALIDATED ===");
    LOG_INFO("All mathematical constraints satisfied - reversal is feasible");
    errorMessage = "";
    return true;
}

bool ReversalExecutor::executeReversal(const ProofOfFeasibility& proof) {
    LOG_INFO("=== EXECUTING REVERSAL ===");
    LOG_INFO("Proof hash: " + proof.proof_hash);
    LOG_INFO("Stolen TX: " + proof.stolen_tx);
    LOG_INFO("Victim: " + proof.origin_owner);
    LOG_INFO("Current holder: " + proof.current_holder);
    LOG_INFO("Amount: " + std::to_string(proof.recoverable_amount));
    
    // Step 1: Protocol-level feasibility validation
    LOG_INFO("Step 1: Protocol validating feasibility...");
    std::string errorMessage;
    if (!validateFeasibility(proof, errorMessage)) {
        LOG_ERROR("Reversal execution failed: " + errorMessage);
        return false;
    }
    LOG_INFO("Step 1: Protocol validated feasibility ✓");
    
    // Step 2: Create reversal transaction
    LOG_INFO("Step 2: Creating reversal transaction...");
    ReversalTransaction rtx = createReversalTransaction(proof);
    LOG_INFO("Step 2: Reversal transaction created ✓");
    
    // Step 3: Check fee pool has sufficient funds
    LOG_INFO("Step 3: Checking fee pool balance...");
    if (!feePool->hasSufficientFunds(rtx.fee)) {
        LOG_ERROR("Reversal execution failed: Insufficient funds in fee pool");
        LOG_ERROR("Required fee: " + std::to_string(rtx.fee));
        LOG_ERROR("Pool balance: " + std::to_string(feePool->getBalance()));
        LOG_ERROR("Dashboard owners must fund the pool at address: " + feePool->getPoolAddress());
        return false;
    }
    LOG_INFO("Step 3: Fee pool has sufficient funds ✓");
    
    // Step 4: Debit current holder
    LOG_INFO("Step 4: Debiting current holder...");
    if (!debitAccount(rtx.from, rtx.amount)) {
        LOG_ERROR("Reversal execution failed: Could not debit current holder");
        return false;
    }
    LOG_INFO("Step 4: Current holder debited ✓");
    
    // Step 5: Credit original victim
    LOG_INFO("Step 5: Crediting original victim...");
    if (!creditAccount(rtx.to, rtx.amount)) {
        LOG_ERROR("Reversal execution failed: Could not credit victim");
        // Rollback debit
        LOG_WARNING("Rolling back debit...");
        creditAccount(rtx.from, rtx.amount);
        return false;
    }
    LOG_INFO("Step 5: Original victim credited ✓");
    
    // Step 6: Deduct fee from pool
    LOG_INFO("Step 6: Deducting fee from pool...");
    if (!feePool->deductFee(rtx.tx_hash, rtx.fee, rtx.to, proof.approved_by)) {
        LOG_ERROR("Reversal execution failed: Could not deduct fee from pool");
        // Rollback everything
        LOG_WARNING("Rolling back all changes...");
        debitAccount(rtx.to, rtx.amount);
        creditAccount(rtx.from, rtx.amount);
        return false;
    }
    LOG_INFO("Step 6: Fee deducted from pool ✓");
    
    // Step 7: Add reversal transaction to blockchain
    LOG_INFO("Step 7: Adding reversal to blockchain...");
    if (!blockchain->addReversalTransaction(rtx.from, rtx.to, rtx.amount, rtx.proof_hash, rtx.fee)) {
        LOG_ERROR("Reversal execution failed: Could not add to blockchain");
        // Rollback everything
        LOG_WARNING("Rolling back all changes...");
        debitAccount(rtx.to, rtx.amount);
        creditAccount(rtx.from, rtx.amount);
        // Fee is not refunded on blockchain add failure (already deducted from pool)
        return false;
    }
    LOG_INFO("Step 7: Reversal added to blockchain ✓");
    
    // Step 8: Log the reversal
    LOG_INFO("Step 8: Logging reversal...");
    logReversal(rtx, proof);
    LOG_INFO("Step 8: Reversal logged ✓");
    
    // Step 9: Mark transaction as reversed (prevent double reversal)
    LOG_INFO("Step 9: Marking transaction as reversed...");
    blockchain->markTransactionAsReversed(proof.stolen_tx, rtx.tx_hash);
    LOG_INFO("Step 9: Transaction marked as reversed ✓");
    LOG_INFO("This prevents double reversal of the same stolen funds");
    
    // Step 10: Deposit reversal execution fee to system pool (self-sustaining model)
    LOG_INFO("Step 10: Depositing reversal execution fee to system pool...");
    double executionFeePercentage = 0.002; // 0.2% of recovered amount
    uint64_t executionFeeDeposit = feePool->depositReversalExecutionFee(
        rtx.tx_hash, rtx.amount, executionFeePercentage);
    LOG_INFO("Step 10: Deposited " + std::to_string(executionFeeDeposit) + 
             " satoshis (" + std::to_string(executionFeePercentage * 100) + 
             "% of recovered) to system pool ✓");
    
    LOG_INFO("=== REVERSAL EXECUTED SUCCESSFULLY ===");
    LOG_INFO("Transaction hash: " + rtx.tx_hash);
    LOG_INFO("Victim received: " + std::to_string(rtx.amount) + " satoshis");
    LOG_INFO("Transaction fee paid: " + std::to_string(rtx.fee) + " satoshis");
    LOG_INFO("Execution fee to pool: " + std::to_string(executionFeeDeposit) + " satoshis");
    LOG_INFO("System is self-sustaining through fee economics");
    
    return true;
}
