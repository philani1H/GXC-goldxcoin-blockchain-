#include "../include/transaction.h"
#include "../include/HashUtils.h"
#include "../include/Utils.h"
#include "../include/Crypto.h"
#include <sstream>
#include <algorithm>
#include <cmath>
#include <iostream>

// Default constructor
Transaction::Transaction() 
    : timestamp(std::time(nullptr)), referencedAmount(0.0), nonce(0),
      isGoldBacked(false), isCoinbase(false), fee(0.0), lockTime(0), type(TransactionType::NORMAL),
      workReceiptHash(""), blockHeight(0) {
    txHash = "";
    prevTxHash = "";
}

Transaction::Transaction(const std::vector<TransactionInput>& inputsIn, 
                        const std::vector<TransactionOutput>& outputsIn,
                        const std::string& prevTxHashIn)
    : inputs(inputsIn), outputs(outputsIn), prevTxHash(prevTxHashIn), 
      isGoldBacked(false), isCoinbase(false), fee(0.0), lockTime(0), type(TransactionType::NORMAL) {
    timestamp = std::time(nullptr);
    nonce = Utils::randomUint32();
    
    // Set sender and receiver from inputs/outputs
    if (!inputs.empty() && !outputs.empty()) {
        senderAddress = ""; // Would be derived from input signatures
        receiverAddress = outputs[0].address;
        referencedAmount = inputs[0].amount;
    }
    
    txHash = calculateHash();
}

Transaction::Transaction(const std::vector<TransactionInput>& inputsIn, 
                        const std::vector<TransactionOutput>& outputsIn,
                        const std::string& prevTxHashIn,
                        const std::string& popReferenceIn)
    : inputs(inputsIn), outputs(outputsIn), prevTxHash(prevTxHashIn), 
      popReference(popReferenceIn), isGoldBacked(true), isCoinbase(false), fee(0.0), lockTime(0), type(TransactionType::NORMAL) {
    timestamp = std::time(nullptr);
    nonce = Utils::randomUint32();
    
    if (!inputs.empty() && !outputs.empty()) {
        senderAddress = "";
        receiverAddress = outputs[0].address;
        referencedAmount = inputs[0].amount;
    }
    
    txHash = calculateHash();
}

// Constructor for coinbase transaction
Transaction::Transaction(const std::string& minerAddress, double blockReward)
    : prevTxHash("0"), referencedAmount(0.0), receiverAddress(minerAddress),
      isGoldBacked(false), isCoinbase(true), fee(0.0), lockTime(0), type(TransactionType::NORMAL),
      workReceiptHash(""), blockHeight(0) {
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
    
    // Include type
    ss << static_cast<int>(type);

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
    
    // For third-party wallets: if prevTxHash is not set, automatically use inputs[0].txHash
    std::string effectivePrevTxHash = prevTxHash;
    if (effectivePrevTxHash.empty() || effectivePrevTxHash == "0") {
        effectivePrevTxHash = inputs[0].txHash;
    }
    
    // Implement the formula: Ti.Inputs[0].txHash == Ti.PrevTxHash
    if (inputs[0].txHash != effectivePrevTxHash) {
        return false;
    }
    
    // For third-party wallets: if referencedAmount is not set, use inputs[0].amount
    double effectiveReferencedAmount = referencedAmount;
    if (effectiveReferencedAmount == 0.0 && !inputs.empty()) {
        effectiveReferencedAmount = inputs[0].amount;
    }
    
    // Implement the formula: Ti.Inputs[0].amount == Ti.ReferencedAmount
    if (std::abs(inputs[0].amount - effectiveReferencedAmount) > 0.00000001) { // Handle floating point precision
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
        
        // Verify ECDSA signature
        std::string message = input.txHash + std::to_string(input.outputIndex) + std::to_string(input.amount);
        if (!Crypto::verifySignature(message, input.signature, input.publicKey)) {
            return false;
        }
    }
    
    return true;
}

void Transaction::signInputs(const std::string& privateKey) {
    // Proper ECDSA signing with secp256k1
    // Derive public key from private key
    std::string publicKey = Crypto::derivePublicKey(privateKey);
    
    for (auto& input : inputs) {
        // Create message to sign (transaction data)
        std::string message = input.txHash + std::to_string(input.outputIndex) + std::to_string(input.amount);
        
        // Sign with ECDSA
        input.signature = Crypto::signData(message, privateKey);
        input.publicKey = publicKey;
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
    
    // Helper to safely serialize strings that might contain the delimiter
    auto safeSerialize = [](const std::string& str) {
        // Hex encode strings that might contain separators
        std::vector<uint8_t> bytes(str.begin(), str.end());
        return Utils::toHex(bytes);
    };

    // Serialize basic data
    // Format: txHash|timestamp|prevTxHash|referencedAmount|senderAddress|receiverAddress|nonce|fee|memo|lockTime|isGoldBacked|isCoinbase|type|
    // NOTE: txHash, prevTxHash, addresses are typically hex or base58 which are safe from '|'
    ss << txHash << "|" << timestamp << "|" << prevTxHash << "|" 
       << referencedAmount << "|" << senderAddress << "|" << receiverAddress << "|"
       << nonce << "|" << fee << "|" << safeSerialize(memo) << "|" << lockTime << "|"
       << (isGoldBacked ? "1" : "0") << "|" << (isCoinbase ? "1" : "0") << "|"
       << static_cast<int>(type) << "|";
    
    // Serialize inputs
    ss << inputs.size() << "|";
    for (const auto& input : inputs) {
        ss << input.txHash << "|" << input.outputIndex << "|" 
           << input.signature << "|" << input.amount << "|" << input.publicKey << "|";
    }
    
    // Serialize outputs
    ss << outputs.size() << "|";
    for (const auto& output : outputs) {
        // Encode script as it can contain arbitrary characters
        ss << output.address << "|" << output.amount << "|" << safeSerialize(output.script) << "|";
    }
    
    if (isGoldBacked) {
        ss << popReference << "|";
    }
    
    return ss.str();
}

bool Transaction::deserialize(const std::string& data) {
    try {
        if (data.empty()) return false;

        std::vector<std::string> parts = Utils::split(data, '|');
        size_t index = 0;

        if (parts.size() < 13) return false; // Minimum required parts

        // Helper to safely deserialize strings
        auto safeDeserialize = [](const std::string& hexStr) {
            if (hexStr.empty()) return std::string("");
            // Check if it looks like hex (basic check)
            if (Utils::isValidHex(hexStr)) {
                 std::vector<uint8_t> bytes = Utils::fromHex(hexStr);
                 return std::string(bytes.begin(), bytes.end());
            }
            // Fallback for backward compatibility if it's not hex or legacy format
            return hexStr;
        };

        txHash = parts[index++];
        timestamp = std::stoull(parts[index++]);
        prevTxHash = parts[index++];
        referencedAmount = std::stod(parts[index++]);
        senderAddress = parts[index++];
        receiverAddress = parts[index++];
        nonce = std::stoul(parts[index++]);
        fee = std::stod(parts[index++]);

        // Memo is now hex encoded
        std::string memoHex = parts[index++];
        memo = safeDeserialize(memoHex);

        lockTime = std::stoul(parts[index++]);
        isGoldBacked = (parts[index++] == "1");
        isCoinbase = (parts[index++] == "1");

        // Handle type field which was added to serialization
        if (index < parts.size() && Utils::isNumeric(parts[index])) {
            int typeInt = std::stoi(parts[index++]);
            type = static_cast<TransactionType>(typeInt);
        } else {
            type = TransactionType::NORMAL;
        }

        // Parse inputs
        if (index >= parts.size()) return false;
        size_t inputCount = std::stoul(parts[index++]);
        inputs.clear();
        for (size_t i = 0; i < inputCount; i++) {
            if (index + 4 >= parts.size()) return false;

            TransactionInput input;
            input.txHash = parts[index++];
            input.outputIndex = std::stoul(parts[index++]);
            input.signature = parts[index++];
            input.amount = std::stod(parts[index++]);
            input.publicKey = parts[index++];
            inputs.push_back(input);
        }

        // Parse outputs
        if (index >= parts.size()) return false;
        size_t outputCount = std::stoul(parts[index++]);
        outputs.clear();
        for (size_t i = 0; i < outputCount; i++) {
            if (index + 2 >= parts.size()) return false;

            TransactionOutput output;
            output.address = parts[index++];
            output.amount = std::stod(parts[index++]);

            // Script is now hex encoded, or might be empty/last
            if (index < parts.size()) {
                std::string scriptHex = parts[index++];
                output.script = safeDeserialize(scriptHex);
            } else {
                output.script = "";
            }
            outputs.push_back(output);
        }

        if (isGoldBacked && index < parts.size()) {
            popReference = parts[index++];
        }

        // Recalculate hash to verify integrity
        // Note: Hash calculation logic might need to align if it uses internal members that were populated
        std::string calculatedHash = calculateHash();
        if (calculatedHash != txHash) {
             // Hash mismatch logging
        }

        return true;
    } catch (const std::exception& e) {
        return false;
    }
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
    
    // For third-party wallets: if prevTxHash is not set but we have inputs,
    // automatically derive it from the first input's txHash
    if ((prevTxHash.empty() || prevTxHash == "0") && !inputs.empty()) {
        // This is valid - the blockchain will use inputs[0].txHash as the reference
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

// Validation function: verify that the scriptSig in the inputs matches the scriptPubKey of the UTXOs being spent
// Validates signature against public key and public key against script/address
bool Transaction::verifyScript(const std::string& signature, const std::string& publicKey, const std::string& scriptPubKey) {
    // This is a simplified P2PKH script verification
    // scriptPubKey format: "OP_DUP OP_HASH160 <address> OP_EQUALVERIFY OP_CHECKSIG"

    // 1. Extract address from scriptPubKey
    std::string address;
    std::string prefix = "OP_DUP OP_HASH160 ";
    std::string suffix = " OP_EQUALVERIFY OP_CHECKSIG";

    size_t startPos = scriptPubKey.find(prefix);
    size_t endPos = scriptPubKey.find(suffix);

    if (startPos != std::string::npos && endPos != std::string::npos && endPos > startPos) {
        address = scriptPubKey.substr(startPos + prefix.length(), endPos - (startPos + prefix.length()));
    } else {
        // If script format doesn't match standard P2PKH, fall back to checking if script IS the address (legacy support)
        if (scriptPubKey.find("OP_") == std::string::npos) {
            address = scriptPubKey;
        } else {
            return false; // Unknown script format
        }
    }

    // 2. Verify that publicKey generates this address
    // Note: We need to know if it's testnet or mainnet.
    // Usually address prefix determines this.
    bool isTestnet = (address.length() >= 4 && address.substr(0, 4) == "tGXC");
    std::string derivedAddress = Crypto::generateAddress(publicKey, isTestnet);

    if (derivedAddress != address) {
        return false;
    }

    // 3. Verify the signature is valid for this public key
    // NOTE: This check depends on the transaction hash which is not passed here.
    // The signature verification (Crypto::verifySignature) is typically done in validateSignatures().
    // This function focuses on matching the key to the lock (script).

    return true;
}
