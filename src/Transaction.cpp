#include "../include/transaction.h"
#include "../include/HashUtils.h"
#include <sstream>

Transaction::Transaction(const std::vector<TransactionInput>& inputsIn, 
                         const std::vector<TransactionOutput>& outputsIn,
                         const std::string& prevTxHashIn)
    : inputs(inputsIn), outputs(outputsIn), prevTxHash(prevTxHashIn), isGoldBacked(false) {
    timestamp = std::time(nullptr);
    txHash = calculateHash();
}

Transaction::Transaction(const std::vector<TransactionInput>& inputsIn, 
                         const std::vector<TransactionOutput>& outputsIn,
                         const std::string& prevTxHashIn,
                         const std::string& popReferenceIn)
    : inputs(inputsIn), outputs(outputsIn), prevTxHash(prevTxHashIn), 
      popReference(popReferenceIn), isGoldBacked(true) {
    timestamp = std::time(nullptr);
    txHash = calculateHash();
}

std::string Transaction::calculateHash() const {
    std::stringstream ss;
    
    // Add all inputs
    for (const auto& input : inputs) {
        ss << input.txHash << input.outputIndex;
    }
    
    // Add all outputs
    for (const auto& output : outputs) {
        ss << output.address << output.amount << output.script;
    }
    
    // Add timestamp and previous transaction hash
    ss << timestamp << prevTxHash;
    
    // For gold-backed transactions, include the PoP reference
    if (isGoldBacked) {
        ss << popReference;
    }
    
    return keccak256(ss.str());
}

bool Transaction::verifyTransaction() const {
    // In a real implementation, we would verify the signatures of all inputs
    // For simplicity, we'll just return true
    return true;
}

void Transaction::signInputs(const std::string& privateKey) {
    // In a real implementation, we would sign all inputs with the private key
    // For simplicity, we'll just set a dummy signature
    for (auto& input : inputs) {
        input.signature = "SIGNED_WITH_" + privateKey;
    }
}
