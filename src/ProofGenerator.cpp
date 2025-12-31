#include "../include/ProofGenerator.h"
#include "../include/blockchain.h"
#include "../include/FraudDetection.h"
#include "../include/Logger.h"
#include "../include/HashUtils.h"
#include <sstream>

ProofGenerator::ProofGenerator(Blockchain* bc, FraudDetection* fd)
    : blockchain(bc), fraudDetection(fd) {
    LOG_INFO("Proof Generator initialized");
}

ProofGenerator::~ProofGenerator() {
    LOG_INFO("Proof Generator destroyed");
}

uint64_t ProofGenerator::calculateRecoverableAmount(const std::string& current_tx,
                                                    const std::string& current_holder,
                                                    double taint_score) {
    // Get current balance of holder
    uint64_t current_balance = blockchain->getBalance(current_holder);
    
    // Calculate recoverable amount: R = τ × Balance
    uint64_t recoverable = static_cast<uint64_t>(taint_score * current_balance);
    
    LOG_INFO("Recoverable amount calculated: " + std::to_string(recoverable) + 
             " (taint: " + std::to_string(taint_score) + 
             ", balance: " + std::to_string(current_balance) + ")");
    
    return recoverable;
}

bool ProofGenerator::verifyTracePath(const std::vector<std::string>& path) {
    if (path.empty()) {
        LOG_ERROR("Proof validation failed: Empty trace path");
        return false;
    }
    
    if (path.size() > MAX_TRACE_HOPS) {
        LOG_ERROR("Proof validation failed: Trace path too long (" + 
                 std::to_string(path.size()) + " > " + std::to_string(MAX_TRACE_HOPS) + ")");
        return false;
    }
    
    // Verify each transaction in path exists
    for (const auto& tx_hash : path) {
        auto tx = blockchain->getTransaction(tx_hash);
        if (!tx) {
            LOG_ERROR("Proof validation failed: Transaction not found in path: " + tx_hash);
            return false;
        }
    }
    
    // Verify path connectivity (each tx connects to next)
    for (size_t i = 0; i < path.size() - 1; i++) {
        auto tx1 = blockchain->getTransaction(path[i]);
        auto tx2 = blockchain->getTransaction(path[i + 1]);
        
        if (!tx1 || !tx2) {
            LOG_ERROR("Proof validation failed: Missing transaction in path");
            return false;
        }
        
        // Check if tx1 outputs connect to tx2 inputs
        // (Simplified check - in production, verify UTXO chain)
        bool connected = false;
        std::string tx1_receiver = tx1->getReceiverAddress();
        std::string tx2_sender = tx2->getSenderAddress();
        
        if (tx1_receiver == tx2_sender) {
            connected = true;
        }
        
        if (!connected) {
            LOG_WARNING("Proof validation: Weak connection between " + path[i] + " and " + path[i+1]);
            // Don't fail - taint can propagate through mixing
        }
    }
    
    return true;
}

bool ProofGenerator::isWithinReversalWindow(const std::string& stolen_tx) {
    auto tx = blockchain->getTransaction(stolen_tx);
    if (!tx) {
        LOG_ERROR("Proof validation failed: Stolen transaction not found: " + stolen_tx);
        return false;
    }
    
    // Get block height of stolen transaction (confirmed by miners)
    uint32_t stolen_height = blockchain->getTransactionBlockHeight(stolen_tx);
    
    if (stolen_height == 0) {
        LOG_ERROR("Proof validation failed: Stolen transaction not confirmed in any block");
        return false;
    }
    
    uint32_t current_height = blockchain->getHeight();
    uint32_t age = current_height - stolen_height;
    
    if (age > REVERSAL_WINDOW) {
        LOG_ERROR("Proof validation failed: Transaction too old (" + 
                 std::to_string(age) + " blocks > " + std::to_string(REVERSAL_WINDOW) + ")");
        return false;
    }
    
    LOG_INFO("Transaction age: " + std::to_string(age) + " blocks (within window)");
    return true;
}

std::string ProofGenerator::generateProofHash(const ProofOfFeasibility& proof) {
    std::stringstream ss;
    ss << proof.proof_type;
    ss << proof.stolen_tx;
    ss << proof.current_tx;
    for (const auto& tx : proof.trace_path) {
        ss << tx;
    }
    ss << proof.taint_score;
    ss << proof.recoverable_amount;
    ss << proof.origin_owner;
    ss << proof.current_holder;
    ss << proof.approved_by;
    ss << proof.approval_signature;
    ss << proof.timestamp;
    ss << proof.block_height;
    
    return sha256(ss.str());
}

ProofOfFeasibility ProofGenerator::generateProof(const std::string& stolen_tx,
                                                const std::string& current_holder,
                                                const std::string& admin_id,
                                                const std::string& admin_signature) {
    ProofOfFeasibility proof;
    
    LOG_INFO("Generating Proof of Feasibility for reversal");
    LOG_INFO("Stolen TX: " + stolen_tx);
    LOG_INFO("Current holder: " + current_holder);
    LOG_INFO("Admin: " + admin_id);
    
    // 1. Verify stolen transaction exists and is marked as stolen
    if (!fraudDetection->isStolen(stolen_tx)) {
        LOG_ERROR("Proof generation failed: Transaction not marked as stolen");
        return proof; // Empty proof
    }
    
    // 1.5. Check if transaction has already been reversed (prevent double reversal)
    if (blockchain->isTransactionReversed(stolen_tx)) {
        LOG_ERROR("Proof generation failed: Transaction has already been reversed");
        LOG_ERROR("This prevents double reversal of the same stolen funds");
        return proof; // Empty proof
    }
    
    // 2. Check if within reversal window
    if (!isWithinReversalWindow(stolen_tx)) {
        LOG_ERROR("Proof generation failed: Transaction outside reversal window");
        return proof;
    }
    
    // 3. Get stolen transaction details
    auto stolen_tx_obj = blockchain->getTransaction(stolen_tx);
    if (!stolen_tx_obj) {
        LOG_ERROR("Proof generation failed: Stolen transaction not found");
        return proof;
    }
    
    std::string origin_owner = stolen_tx_obj->getSenderAddress();
    
    // 4. Trace tainted flow to current holder
    auto flow_paths = fraudDetection->traceTaintedFlow(stolen_tx, MAX_TRACE_HOPS);
    
    if (flow_paths.empty()) {
        LOG_ERROR("Proof generation failed: No tainted flow paths found");
        return proof;
    }
    
    // 5. Find path to current holder
    FraudDetection::FlowPath selected_path;
    bool found_path = false;
    
    for (const auto& path : flow_paths) {
        if (!path.addresses.empty() && path.addresses.back() == current_holder) {
            selected_path = path;
            found_path = true;
            break;
        }
    }
    
    if (!found_path) {
        LOG_ERROR("Proof generation failed: No path to current holder found");
        return proof;
    }
    
    // 6. Get taint score
    double taint_score = selected_path.finalTaint;
    
    if (taint_score < MIN_TAINT_THRESHOLD) {
        LOG_ERROR("Proof generation failed: Taint score too low (" + 
                 std::to_string(taint_score) + " < " + std::to_string(MIN_TAINT_THRESHOLD) + ")");
        return proof;
    }
    
    // 7. Get current transaction (last in path)
    std::string current_tx = selected_path.transactions.empty() ? "" : selected_path.transactions.back();
    
    if (current_tx.empty()) {
        LOG_ERROR("Proof generation failed: No current transaction found");
        return proof;
    }
    
    // 8. Calculate recoverable amount
    uint64_t recoverable_amount = calculateRecoverableAmount(current_tx, current_holder, taint_score);
    
    if (recoverable_amount == 0) {
        LOG_ERROR("Proof generation failed: Recoverable amount is zero");
        return proof;
    }
    
    // 9. Build proof
    proof.proof_type = "REVERSAL_PROOF";
    proof.stolen_tx = stolen_tx;
    proof.current_tx = current_tx;
    proof.trace_path = selected_path.transactions;
    proof.taint_score = taint_score;
    proof.recoverable_amount = recoverable_amount;
    proof.origin_owner = origin_owner;
    proof.current_holder = current_holder;
    proof.approved_by = admin_id;
    proof.approval_signature = admin_signature;
    proof.timestamp = std::time(nullptr);
    proof.block_height = blockchain->getHeight();
    
    // 10. Generate proof hash
    proof.proof_hash = generateProofHash(proof);
    
    LOG_INFO("Proof of Feasibility generated successfully");
    LOG_INFO("Proof hash: " + proof.proof_hash);
    LOG_INFO("Taint score: " + std::to_string(proof.taint_score));
    LOG_INFO("Recoverable amount: " + std::to_string(proof.recoverable_amount));
    LOG_INFO("Trace path length: " + std::to_string(proof.trace_path.size()));
    
    return proof;
}

bool ProofGenerator::validateProof(const ProofOfFeasibility& proof) {
    LOG_INFO("Validating Proof of Feasibility: " + proof.proof_hash);
    
    // 1. Check proof type
    if (proof.proof_type != "REVERSAL_PROOF") {
        LOG_ERROR("Proof validation failed: Invalid proof type");
        return false;
    }
    
    // 2. Verify stolen transaction is marked as stolen
    if (!fraudDetection->isStolen(proof.stolen_tx)) {
        LOG_ERROR("Proof validation failed: Transaction not marked as stolen");
        return false;
    }
    
    // 3. Verify within reversal window
    if (!isWithinReversalWindow(proof.stolen_tx)) {
        LOG_ERROR("Proof validation failed: Outside reversal window");
        return false;
    }
    
    // 4. Verify trace path
    if (!verifyTracePath(proof.trace_path)) {
        LOG_ERROR("Proof validation failed: Invalid trace path");
        return false;
    }
    
    // 5. Verify taint score
    if (proof.taint_score < MIN_TAINT_THRESHOLD) {
        LOG_ERROR("Proof validation failed: Taint score too low");
        return false;
    }
    
    if (proof.taint_score > 1.0) {
        LOG_ERROR("Proof validation failed: Taint score > 1.0");
        return false;
    }
    
    // 6. Verify recoverable amount
    if (proof.recoverable_amount == 0) {
        LOG_ERROR("Proof validation failed: Recoverable amount is zero");
        return false;
    }
    
    // Verify recoverable amount doesn't exceed current balance
    uint64_t current_balance = blockchain->getBalance(proof.current_holder);
    if (proof.recoverable_amount > current_balance) {
        LOG_ERROR("Proof validation failed: Recoverable amount exceeds current balance");
        return false;
    }
    
    // 7. Verify proof hash
    std::string calculated_hash = generateProofHash(proof);
    if (calculated_hash != proof.proof_hash) {
        LOG_ERROR("Proof validation failed: Proof hash mismatch");
        return false;
    }
    
    // 8. Verify admin signature (simplified - in production use proper crypto)
    if (proof.approval_signature.empty()) {
        LOG_ERROR("Proof validation failed: Missing admin signature");
        return false;
    }
    
    LOG_INFO("Proof validation PASSED");
    return true;
}
