#pragma once

#include <string>
#include <vector>
#include <ctime>
#include <cstdint>

struct TransactionInput {
    std::string txHash;      // Reference to previous transaction
    uint32_t outputIndex;    // Index of the output in the referenced transaction
    std::string signature;   // Signature to verify ownership
    double amount;           // Amount from the referenced output (for traceability)
    std::string publicKey;   // Public key for verification
};

struct TransactionOutput {
    std::string address;     // Recipient address
    double amount;           // Amount of GXC
    std::string script;      // Output script (can be used for smart contracts)
};

enum class TransactionType {
    NORMAL = 0,
    STAKE = 1,
    UNSTAKE = 2
};

class Transaction {
private:
    std::string txHash;
    TransactionType type;
    std::vector<TransactionInput> inputs;
    std::vector<TransactionOutput> outputs;
    std::time_t timestamp;

    // Enhanced Traceability System - Implementing Your Formula
    std::string prevTxHash;       // Hash of sender's previous transaction (Ti-1)
    double referencedAmount;      // Amount transferred from Ti-1
    std::string senderAddress;    // Address of the sender
    std::string receiverAddress;  // Primary receiver address
    uint32_t nonce;              // For transaction uniqueness

    // Special transaction types
    std::string popReference;     // Reference to Proof of Price (for gold-backed tokens)
    bool isGoldBacked;           // Flag for gold-backed transactions
    bool isCoinbase;             // Flag for coinbase transactions
    std::string chainReference;  // Cross-chain reference

    // Transaction metadata
    double fee;                  // Transaction fee
    std::string memo;            // Optional memo field
    uint32_t lockTime;           // Time lock for the transaction

public:
    // Default constructor
    Transaction();

    // Constructor for regular transaction
    Transaction(const std::vector<TransactionInput>& inputsIn,
                const std::vector<TransactionOutput>& outputsIn,
                const std::string& prevTxHashIn);

    // Constructor for gold-backed transaction
    Transaction(const std::vector<TransactionInput>& inputsIn,
                const std::vector<TransactionOutput>& outputsIn,
                const std::string& prevTxHashIn,
                const std::string& popReferenceIn);

    // Constructor for coinbase transaction
    Transaction(const std::string& minerAddress, double blockReward);

    // Core transaction operations
    std::string calculateHash() const;
    bool verifyTransaction() const;
    void signInputs(const std::string& privateKey);

    // Traceability Verification - Implementing Your Formula
    // Ti.Inputs[0].txHash == Ti.PrevTxHash
    // Ti.Inputs[0].amount == Ti.ReferencedAmount
    bool verifyTraceabilityFormula() const;
    bool validateInputReference() const;
    bool isTraceabilityValid() const;

    // Enhanced validation
    bool validateAmountConsistency() const;
    bool validateInputOutputBalance() const;
    bool validateSignatures() const;

    // Getters
    std::string getHash() const { return txHash; }
    std::time_t getTimestamp() const { return timestamp; }
    const std::vector<TransactionInput>& getInputs() const { return inputs; }
    const std::vector<TransactionOutput>& getOutputs() const { return outputs; }
    std::string getPrevTxHash() const { return prevTxHash; }
    double getReferencedAmount() const { return referencedAmount; }
    std::string getSenderAddress() const { return senderAddress; }
    std::string getReceiverAddress() const { return receiverAddress; }
    uint32_t getNonce() const { return nonce; }
    std::string getPopReference() const { return popReference; }
    bool isGoldBackedTransaction() const { return isGoldBacked; }
    bool isCoinbaseTransaction() const { return isCoinbase; }
    double getFee() const { return fee; }
    std::string getMemo() const { return memo; }
    uint32_t getLockTime() const { return lockTime; }
    TransactionType getType() const { return type; }

    // Setters
    void setHash(const std::string& hash) { txHash = hash; }
    void setTimestamp(std::time_t ts) { timestamp = ts; }
    void setCoinbaseTransaction(bool isCoinbaseTx) { isCoinbase = isCoinbaseTx; }
    void setPrevTxHash(const std::string& hash) { prevTxHash = hash; }
    void setReferencedAmount(double amount) { referencedAmount = amount; }
    void setSenderAddress(const std::string& addr) { senderAddress = addr; }
    void setReceiverAddress(const std::string& addr) { receiverAddress = addr; }
    void setNonce(uint32_t n) { nonce = n; }
    void setFee(double f) { fee = f; }
    void setMemo(const std::string& m) { memo = m; }
    void setLockTime(uint32_t time) { lockTime = time; }
    void setType(TransactionType t) { type = t; }

    // Utility functions
    double getTotalInputAmount() const;
    double getTotalOutputAmount() const;
    bool isValid() const;
    std::string serialize() const;
    bool deserialize(const std::string& data);
    std::vector<std::string> getInputHashes() const;

    // Transaction type checks
    bool isGenesis() const;
    bool hasValidPrevReference() const;

    // Add input/output methods
    void addInput(const TransactionInput& input);
    void addOutput(const TransactionOutput& output);
    void clearInputs();
    void clearOutputs();
};
