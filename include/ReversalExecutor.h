#pragma once

#include <string>
#include "FraudDetection.h"
#include "ProofGenerator.h"
#include "ReversalFeePool.h"

// Forward declarations
class Blockchain;
class ProofGenerator;
class ReversalFeePool;

/**
 * Reversal Executor
 * 
 * Executes approved reversals by:
 * 1. Validating proof
 * 2. Creating reversal transaction
 * 3. Debiting current holder
 * 4. Crediting original victim
 * 5. Paying fee from system pool
 * 6. Adding to blockchain
 */
class ReversalExecutor {
private:
    Blockchain* blockchain;
    ProofGenerator* proofGenerator;
    ReversalFeePool* feePool;
    
    // Configuration
    static constexpr uint64_t STANDARD_FEE = 1000; // 0.00001 GXC
    
    /**
     * Calculate reversal fee
     */
    uint64_t calculateFee(uint64_t amount);
    
    /**
     * Debit account
     */
    bool debitAccount(const std::string& address, uint64_t amount);
    
    /**
     * Credit account
     */
    bool creditAccount(const std::string& address, uint64_t amount);
    
    /**
     * Create reversal transaction
     */
    ReversalTransaction createReversalTransaction(const ProofOfFeasibility& proof);
    
    /**
     * Log reversal execution
     */
    void logReversal(const ReversalTransaction& rtx, const ProofOfFeasibility& proof);
    
public:
    ReversalExecutor(Blockchain* bc, ProofGenerator* pg, ReversalFeePool* fp);
    ~ReversalExecutor();
    
    /**
     * Validate reversal feasibility (protocol-level, independent of admin)
     * 
     * Checks mathematical constraints:
     * 1. Proof is cryptographically valid
     * 2. Current holder has sufficient balance
     * 3. Taint score is above threshold
     * 4. Within reversal time window
     * 5. System pool has funds for fees
     * 
     * @param proof Proof of Feasibility to validate
     * @param errorMessage Output parameter for error details
     * @return true if feasible, false otherwise
     */
    bool validateFeasibility(const ProofOfFeasibility& proof, std::string& errorMessage);
    
    /**
     * Execute a reversal
     * 
     * This method performs protocol-level validation before execution.
     * Admin approval is NOT checked here - that's handled separately.
     * 
     * @param proof Validated Proof of Feasibility
     * @return true if successful, false otherwise
     */
    bool executeReversal(const ProofOfFeasibility& proof);
    
    /**
     * Get standard fee
     */
    uint64_t getStandardFee() const { return STANDARD_FEE; }
};
