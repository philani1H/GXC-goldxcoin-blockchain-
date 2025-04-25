#pragma once

#include <string>
#include <vector>
#include <ctime>

struct TransactionInput {
    std::string txHash;      // Reference to previous transaction
    uint32_t outputIndex;    // Index of the output in the referenced transaction
    std::string signature;   // Signature to verify ownership
};

struct TransactionOutput {
    std::string address;     // Recipient address
    double amount;           // Amount of GXC
    std::string script;      // Output script (can be used for smart contracts)
};

class Transaction {
private:
    std::string txHash;
    std::vector<TransactionInput> inputs;
    std::vector<TransactionOutput> outputs;
    std::time_t timestamp;
    std::string prevTxHash;  // Hash of sender's previous transaction (for chained TX hashes)
    std::string popReference; // Reference to Proof of Price (for gold-backed tokens)
    bool isGoldBacked;       // Flag for gold-backed transactions

public:
    // Constructor for regular transaction
    Transaction(const std::vector<TransactionInput>& inputsIn, 
                const std::vector<TransactionOutput>& outputsIn,
                const std::string& prevTxHashIn);
    
    // Constructor for gold-backed transaction
    Transaction(const std::vector<TransactionInput>& inputsIn, 
                const std::vector<TransactionOutput>& outputsIn,
                const std::string& prevTxHashIn,
                const std::string& popReferenceIn);
    
    // Calculate transaction hash
    std::string calculateHash() const;
    
    // Verify transaction
    bool verifyTransaction() const;
    
    // Sign transaction inputs
    void signInputs(const std::string& privateKey);
    
    // Getters
    std::string getHash() const { return txHash; }
    std::time_t getTimestamp() const { return timestamp; }
    const std::vector<TransactionInput>& getInputs() const { return inputs; }
    const std::vector<TransactionOutput>& getOutputs() const { return outputs; }
    std::string getPrevTxHash() const { return prevTxHash; }
    std::string getPopReference() const { return popReference; }
    bool isGoldBackedTransaction() const { return isGoldBacked; }
};
