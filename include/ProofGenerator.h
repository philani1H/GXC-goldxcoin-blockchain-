#pragma once

#include <string>
#include <vector>
#include "FraudDetection.h"

// Forward declarations
class Blockchain;
class FraudDetection;

/**
 * Proof Generator
 * 
 * Generates cryptographic Proof of Feasibility (POF) for reversals.
 * 
 * This class proves that a reversal is:
 * 1. Causally valid (funds came from stolen transaction)
 * 2. Mathematically sound (amount ≤ recoverable)
 * 3. Administratively approved (signed by authorized admin)
 * 4. Within finality window (not too old)
 */
class ProofGenerator {
private:
    Blockchain* blockchain;
    FraudDetection* fraudDetection;
    
    // Configuration
    static constexpr uint32_t MAX_TRACE_HOPS = 20;      // Maximum hops to trace
    static constexpr uint32_t REVERSAL_WINDOW = 20000;  // ~30 days in blocks
    static constexpr double MIN_TAINT_THRESHOLD = 0.1;  // Minimum taint to reverse
    
    /**
     * Calculate recoverable amount
     * R = τ(T_k) × Balance(current_holder)
     */
    uint64_t calculateRecoverableAmount(const std::string& current_tx,
                                       const std::string& current_holder,
                                       double taint_score);
    
    /**
     * Verify trace path is valid
     */
    bool verifyTracePath(const std::vector<std::string>& path);
    
    /**
     * Check if within reversal window
     */
    bool isWithinReversalWindow(const std::string& stolen_tx);
    
    /**
     * Generate proof hash
     */
    std::string generateProofHash(const ProofOfFeasibility& proof);
    
public:
    ProofGenerator(Blockchain* bc, FraudDetection* fd);
    ~ProofGenerator();
    
    /**
     * Generate Proof of Feasibility for a reversal
     * 
     * @param stolen_tx Original stolen transaction hash
     * @param current_holder Current holder address
     * @param admin_id Admin ID who is approving
     * @param admin_signature Admin's signature
     * @return ProofOfFeasibility or empty proof if invalid
     */
    ProofOfFeasibility generateProof(const std::string& stolen_tx,
                                    const std::string& current_holder,
                                    const std::string& admin_id,
                                    const std::string& admin_signature);
    
    /**
     * Validate an existing proof
     */
    bool validateProof(const ProofOfFeasibility& proof);
    
    /**
     * Get maximum trace hops
     */
    uint32_t getMaxTraceHops() const { return MAX_TRACE_HOPS; }
    
    /**
     * Get reversal window (in blocks)
     */
    uint32_t getReversalWindow() const { return REVERSAL_WINDOW; }
    
    /**
     * Get minimum taint threshold
     */
    double getMinTaintThreshold() const { return MIN_TAINT_THRESHOLD; }
};
