#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "transaction.h"

class Wallet {
private:
    std::string privateKey;
    std::string publicKey;
    std::string address;
    std::string lastTxHash; // For chained TX hashes
    std::unordered_map<std::string, TransactionOutput> unspentOutputs;
    
    // Imported addresses (third-party wallets) - watch-only unless private key imported
    std::unordered_map<std::string, std::string> importedAddresses;  // address -> label
    std::unordered_map<std::string, std::string> importedPrivateKeys;  // address -> privateKey
    std::unordered_map<std::string, std::string> importedPublicKeys;   // address -> publicKey

public:
    // Constructor
    Wallet();

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
    
    // ============= THIRD-PARTY WALLET SUPPORT =============
    
    // Import private key for third-party wallet (full control)
    bool importPrivateKey(const std::string& privateKeyHex, const std::string& label = "");
    
    // Import address for watch-only (no signing capability)
    bool importAddress(const std::string& addr, const std::string& label = "");
    
    // Check if wallet controls an address (either main or imported)
    bool controlsAddress(const std::string& addr) const;
    
    // Check if wallet can sign for an address (has private key)
    bool canSignForAddress(const std::string& addr) const;
    
    // Get private key for an address (main or imported)
    std::string getPrivateKeyForAddress(const std::string& addr) const;
    
    // Get public key for an address (main or imported)
    std::string getPublicKeyForAddress(const std::string& addr) const;
    
    // Get all controlled addresses (main + imported with keys)
    std::vector<std::string> getAllControlledAddresses() const;
    
    // Get all watched addresses (imported without keys)
    std::vector<std::string> getWatchOnlyAddresses() const;
    
    // Switch active address (for multi-address support)
    bool setActiveAddress(const std::string& addr);
    
    // Create transaction from specific address
    Transaction createTransactionFrom(const std::string& fromAddress,
                                      const std::string& toAddress, 
                                      double amount,
                                      const std::unordered_map<std::string, TransactionOutput>& utxoSet,
                                      double fee = 0.001);
    
    // Create staking transaction from specific address
    Transaction createStakeTransactionFrom(const std::string& fromAddress,
                                           double stakeAmount, 
                                           const std::unordered_map<std::string, TransactionOutput>& utxoSet,
                                           double fee = 0.001);
    
    // ============= END THIRD-PARTY WALLET SUPPORT =============
    
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
