#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "transaction.h"

class Wallet {
private:
    std::string privateKey;
    std::string publicKey;
    std::string address;
    std::string lastTxHash; // For chained TX hashes
    std::unordered_map<std::string, TransactionOutput> unspentOutputs;

public:
    // Constructor
    Wallet();
    
    // Create wallet from private key
    bool createFromPrivateKey(const std::string& privateKeyHex);
    
    // Set wallet address (for use with imported keys)
    void setAddress(const std::string& addr) { address = addr; }
    void setPublicKey(const std::string& pubKey) { publicKey = pubKey; }
    void setPrivateKey(const std::string& privKey) { privateKey = privKey; }

    // Generate a new key pair
    void generateKeyPair();

    // Persistence
    bool saveToFile(const std::string& filepath) const;
    bool loadFromFile(const std::string& filepath);

    // Create a transaction (with optional fee, defaults to 0.001)
    Transaction createTransaction(const std::string& recipientAddress, double amount,
                                 const std::unordered_map<std::string, TransactionOutput>& utxoSet,
                                 double fee = 0.001);

    // Create a staking transaction (locks coins in stake pool)
    Transaction createStakeTransaction(double stakeAmount, 
                                       const std::unordered_map<std::string, TransactionOutput>& utxoSet,
                                       double fee = 0.001);

    // Create an unstaking transaction (releases coins from stake pool)
    Transaction createUnstakeTransaction(double unstakeAmount, double fee = 0.0);

    // Sign a transaction
    void signTransaction(Transaction& tx);

    // Create a gold-backed transaction
    Transaction createGoldBackedTransaction(const std::string& recipientAddress, double amount,
                                          const std::unordered_map<std::string, TransactionOutput>& utxoSet,
                                          const std::string& popReference);

    // Calculate wallet balance
    double getBalance(const std::unordered_map<std::string, TransactionOutput>& utxoSet) const;

    // Update last transaction hash
    void updateLastTxHash(const std::string& txHash) { lastTxHash = txHash; }

    // Getters
    std::string getAddress() const { return address; }
    std::string getPublicKey() const { return publicKey; }
    std::string getLastTxHash() const { return lastTxHash; }
    
    // Static address validation methods
    static bool isValidAddress(const std::string& address) {
        // GXC addresses start with "GXC" (mainnet) or "tGXC" (testnet)
        if (address.length() < 30) return false;
        return address.substr(0, 3) == "GXC" || address.substr(0, 4) == "tGXC";
    }
    
    static bool isTestnetAddress(const std::string& address) {
        return address.length() >= 4 && address.substr(0, 4) == "tGXC";
    }
    
    static bool isMainnetAddress(const std::string& address) {
        return address.length() >= 3 && address.substr(0, 3) == "GXC" && 
               (address.length() < 4 || address[3] != 'C' || address.substr(0,4) != "tGXC");
    }
};
