#include "../include/transaction.h"
#include "../include/HashUtils.h"
#include "../include/Utils.h"
#include <sstream>
#include <algorithm>
#include <cmath>

// Default constructor
Transaction::Transaction() 
    : timestamp(std::time(nullptr)), referencedAmount(0.0), nonce(0),
      isGoldBacked(false), isCoinbase(false), fee(0.0), lockTime(0) {
    txHash = "";
    prevTxHash = "";
}

Transaction::Transaction(const std::vector<TransactionInput>& inputsIn, 
                        const std::vector<TransactionOutput>& outputsIn,
                        const std::string& prevTxHashIn)
    : inputs(inputsIn), outputs(outputsIn), prevTxHash(prevTxHashIn), 
      isGoldBacked(false), isCoinbase(false), fee(0.0), lockTime(0) {
    timestamp = std::time(nullptr);
    nonce = Utils::randomUint32();
    
    // Set sender and receiver from inputs/outputs
    if (!inputs.empty() && !outputs.empty()) {
        senderAddress = ""; // Would be derived from input signatures
        receiverAddress = outputs[0].address;
        referencedAmount = getTotalInputAmount();
    }
    
    txHash = calculateHash();
}

Transaction::Transaction(const std::vector<TransactionInput>& inputsIn, 
                        const std::vector<TransactionOutput>& outputsIn,
                        const std::string& prevTxHashIn,
                        const std::string& popReferenceIn)
    : inputs(inputsIn), outputs(outputsIn), prevTxHash(prevTxHashIn), 
      popReference(popReferenceIn), isGoldBacked(true), isCoinbase(false), fee(0.0), lockTime(0) {
    timestamp = std::time(nullptr);
    nonce = Utils::randomUint32();
    
    if (!inputs.empty() && !outputs.empty()) {
        senderAddress = "";
        receiverAddress = outputs[0].address;
        referencedAmount = getTotalInputAmount();
    }
    
    txHash = calculateHash();
}

// Constructor for coinbase transaction
Transaction::Transaction(const std::string& minerAddress, double blockReward)
    : prevTxHash("0"), referencedAmount(0.0), receiverAddress(minerAddress),
      isGoldBacked(false), isCoinbase(true), fee(0.0), lockTime(0) {
    timestamp = std::time(nullptr);
    nonce = Utils::randomUint32();
    
    // Coinbase transaction has no inputs
    inputs.clear();
    
    // Single output to miner
    TransactionOutput output;
    output.address = minerAddress;
    output.amount = blockReward;
    output.script = "OP_DUP OP_HASH160 " + minerAddress + " OP_EQUALVERIFY OP_CHECKSIG";
    outputs.push_back(output);
    
    txHash = calculateHash();
}

std::string Transaction::calculateHash() const {
    std::stringstream ss;
    
    // Include inputs
    for (const auto& input : inputs) {
        ss << input.txHash << input.outputIndex << input.signature << input.amount << input.publicKey;
    }
    
    // Include outputs
    for (const auto& output : outputs) {
        ss << output.address << output.amount << output.script;
    }
    
    // Include all transaction data for comprehensive hash
    ss << timestamp << prevTxHash << referencedAmount << senderAddress 
       << receiverAddress << nonce << fee << memo << lockTime;
    
    // Include special fields
    if (isGoldBacked) {
        ss << popReference;
    }
    
    if (isCoinbase) {
        ss << "COINBASE";
    }
    
    return keccak256(ss.str());
}

// Core Traceability Verification - Implementing Your Formula
bool Transaction::verifyTraceabilityFormula() const {
    if (isCoinbase || isGenesis()) {
        return true; // Genesis and coinbase transactions don't need traceability
    }
    
    if (inputs.empty()) {
        return false;
    }
    
    // Implement the formula: Ti.Inputs[0].txHash == Ti.PrevTxHash
    if (inputs[0].txHash != prevTxHash) {
        return false;
    }
    
    // Implement the formula: Ti.Inputs[0].amount == Ti.ReferencedAmount
    if (std::abs(inputs[0].amount - referencedAmount) > 0.00000001) { // Handle floating point precision
        return false;
    }
    
    return true;
}

bool Transaction::validateInputReference() const {
    if (isCoinbase || isGenesis()) {
        return true;
    }
    
    // Ensure all inputs reference valid previous transactions
    for (const auto& input : inputs) {
        if (input.txHash.empty() || input.txHash == "0") {
            return false;
        }
        
        if (input.amount <= 0) {
            return false;
        }
    }
    
    return true;
}

bool Transaction::isTraceabilityValid() const {
    return verifyTraceabilityFormula() && validateInputReference() && 
           validateAmountConsistency() && hasValidPrevReference();
}

bool Transaction::verifyTransaction() const {
    // 1. Basic structure validation
    if (outputs.empty()) {
        return false;
    }
    
    // 2. Coinbase transactions have special rules
    if (isCoinbase) {
        if (!inputs.empty()) {
            return false; // Coinbase should have no inputs
        }
        return outputs.size() == 1 && outputs[0].amount > 0;
    }
    
    // 3. Regular transactions must have inputs
    if (inputs.empty()) {
        return false;
    }
    
    // 4. Verify traceability formula
    if (!isTraceabilityValid()) {
        return false;
    }
    
    // 5. Verify that all output amounts are positive
    for (const auto& output : outputs) {
        if (output.amount <= 0) {
            return false;
        }
    }
    
    // 6. Verify input-output balance
    if (!validateInputOutputBalance()) {
        return false;
    }
    
    // 7. Verify signatures
    if (!validateSignatures()) {
        return false;
    }
    
    // 8. For gold-backed transactions, verify PoP reference
    if (isGoldBacked && popReference.empty()) {
        return false;
    }
    
    return true;
}

bool Transaction::validateAmountConsistency() const {
    if (isCoinbase) {
        return true; // Coinbase creates new money
    }
    
    double inputTotal = getTotalInputAmount();
    double outputTotal = getTotalOutputAmount();
    
    // Input total should equal output total plus fee
    return std::abs(inputTotal - (outputTotal + fee)) < 0.00000001;
}

bool Transaction::validateInputOutputBalance() const {
    if (isCoinbase) {
        return outputs.size() == 1 && outputs[0].amount > 0;
    }
    
    return validateAmountConsistency();
}

bool Transaction::validateSignatures() const {
    if (isCoinbase) {
        return true; // Coinbase doesn't need signatures
    }
    
    // Verify all input signatures
    for (const auto& input : inputs) {
        if (input.signature.empty()) {
            return false;
        }
        
        if (input.publicKey.empty()) {
            return false;
        }
        
        // In a real implementation, would verify cryptographic signature
        // For now, just check that signature exists
    }
    
    return true;
}

void Transaction::signInputs(const std::string& privateKey) {
    // Simplified signing - in practice would use proper cryptographic signing
    for (auto& input : inputs) {
        std::string message = input.txHash + std::to_string(input.outputIndex) + std::to_string(input.amount);
        input.signature = sha256(message + privateKey);
        // In real implementation, would derive public key from private key
        input.publicKey = "PUBKEY_" + privateKey.substr(0, 10);
    }
}

// Utility functions
double Transaction::getTotalInputAmount() const {
    double total = 0.0;
    for (const auto& input : inputs) {
        total += input.amount;
    }
    return total;
}

double Transaction::getTotalOutputAmount() const {
    double total = 0.0;
    for (const auto& output : outputs) {
        total += output.amount;
    }
    return total;
}

bool Transaction::isValid() const {
    return verifyTransaction();
}

std::string Transaction::serialize() const {
    std::stringstream ss;
    
    // Serialize basic data
    ss << txHash << "|" << timestamp << "|" << prevTxHash << "|" 
       << referencedAmount << "|" << senderAddress << "|" << receiverAddress << "|"
       << nonce << "|" << fee << "|" << memo << "|" << lockTime << "|"
       << (isGoldBacked ? "1" : "0") << "|" << (isCoinbase ? "1" : "0") << "|";
    
    // Serialize inputs
    ss << inputs.size() << "|";
    for (const auto& input : inputs) {
        ss << input.txHash << "|" << input.outputIndex << "|" 
           << input.signature << "|" << input.amount << "|" << input.publicKey << "|";
    }
    
    // Serialize outputs
    ss << outputs.size() << "|";
    for (const auto& output : outputs) {
        ss << output.address << "|" << output.amount << "|" << output.script << "|";
    }
    
    if (isGoldBacked) {
        ss << popReference << "|";
    }
    
    return ss.str();
}

bool Transaction::deserialize(const std::string& data) {
    // Simplified deserialization - would need robust parsing in production
    // This is a placeholder implementation
    return !data.empty();
}

std::vector<std::string> Transaction::getInputHashes() const {
    std::vector<std::string> hashes;
    for (const auto& input : inputs) {
        hashes.push_back(input.txHash);
    }
    return hashes;
}

bool Transaction::isGenesis() const {
    return prevTxHash == "0" && !isCoinbase;
}

bool Transaction::hasValidPrevReference() const {
    if (isCoinbase || isGenesis()) {
        return true;
    }
    
    return !prevTxHash.empty() && prevTxHash != "0";
}

// Add/modify transaction data
void Transaction::addInput(const TransactionInput& input) {
    inputs.push_back(input);
    txHash = calculateHash(); // Recalculate hash
}

void Transaction::addOutput(const TransactionOutput& output) {
    outputs.push_back(output);
    txHash = calculateHash(); // Recalculate hash
}

void Transaction::clearInputs() {
    inputs.clear();
    txHash = calculateHash();
}

void Transaction::clearOutputs() {
    outputs.clear();
    txHash = calculateHash();
}
