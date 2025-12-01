#include "../include/Wallet.h"
#include "../include/HashUtils.h"
#include <random>
#include <sstream>
#include <iomanip>

Wallet::Wallet() {
    generateKeyPair();
}

void Wallet::generateKeyPair() {
    // Generate a random private key (256 bits)
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;
    
    std::stringstream ss;
    for (int i = 0; i < 4; i++) {
        ss << std::hex << std::setw(16) << std::setfill('0') << dis(gen);
    }
    
    privateKey = ss.str();
    
    // In a real implementation, we would derive the public key from the private key
    // using elliptic curve cryptography (secp256k1)
    publicKey = "PUBLIC_KEY_FOR_" + privateKey;
    
    // Generate address from public key
    std::string hash = ripemd160(sha256(publicKey));
    address = "GXC" + hash.substr(0, 34); // Simplified address format
    
    // Initialize last transaction hash
    lastTxHash = "";
}

Transaction Wallet::createTransaction(const std::string& recipientAddress, double amount, 
                                     const std::unordered_map<std::string, TransactionOutput>& utxoSet) {
    // Find unspent outputs to use as inputs
    std::vector<TransactionInput> inputs;
    std::vector<TransactionOutput> outputs;
    
    double availableAmount = 0.0;
    
    // Find unspent outputs belonging to this wallet
    for (const auto& [utxoKey, utxo] : utxoSet) {
        if (utxo.address == address) {
            // Parse the UTXO key to get the transaction hash and output index
            size_t colonPos = utxoKey.find(':');
            if (colonPos != std::string::npos) {
                std::string txHash = utxoKey.substr(0, colonPos);
                uint32_t outputIndex = std::stoul(utxoKey.substr(colonPos + 1));
                
                // Create an input
                TransactionInput input;
                input.txHash = txHash;
                input.outputIndex = outputIndex;
                inputs.push_back(input);
                
                availableAmount += utxo.amount;
                
                // If we have enough funds, break
                if (availableAmount >= amount) {
                    break;
                }
            }
        }
    }
    
    // Check if we have enough funds
    if (availableAmount < amount) {
        throw std::runtime_error("Insufficient funds");
    }
    
    // Create the output to the recipient
    TransactionOutput recipientOutput;
    recipientOutput.address = recipientAddress;
    recipientOutput.amount = amount;
    outputs.push_back(recipientOutput);
    
    // If there's change, create an output back to our address
    if (availableAmount > amount) {
        TransactionOutput changeOutput;
        changeOutput.address = address;
        changeOutput.amount = availableAmount - amount;
        outputs.push_back(changeOutput);
    }
    
    // Create the transaction
    Transaction tx(inputs, outputs, lastTxHash);
    
    // Sign the inputs
    tx.signInputs(privateKey);
    
    // Update last transaction hash
    lastTxHash = tx.getHash();
    
    return tx;
}

Transaction Wallet::createGoldBackedTransaction(const std::string& recipientAddress, double amount, 
                                              const std::unordered_map<std::string, TransactionOutput>& utxoSet,
                                              const std::string& popReference) {
    // Similar to createTransaction, but with the addition of the PoP reference
    std::vector<TransactionInput> inputs;
    std::vector<TransactionOutput> outputs;
    
    double availableAmount = 0.0;
    
    // Find unspent outputs belonging to this wallet
    for (const auto& [utxoKey, utxo] : utxoSet) {
        if (utxo.address == address) {
            // Parse the UTXO key to get the transaction hash and output index
            size_t colonPos = utxoKey.find(':');
            if (colonPos != std::string::npos) {
                std::string txHash = utxoKey.substr(0, colonPos);
                uint32_t outputIndex = std::stoul(utxoKey.substr(colonPos + 1));
                
                // Create an input
                TransactionInput input;
                input.txHash = txHash;
                input.outputIndex = outputIndex;
                inputs.push_back(input);
                
                availableAmount += utxo.amount;
                
                // If we have enough funds, break
                if (availableAmount >= amount) {
                    break;
                }
            }
        }
    }
    
    // Check if we have enough funds
    if (availableAmount < amount) {
        throw std::runtime_error("Insufficient funds");
    }
    
    // Create the output to the recipient
    TransactionOutput recipientOutput;
    recipientOutput.address = recipientAddress;
    recipientOutput.amount = amount;
    outputs.push_back(recipientOutput);
    
    // If there's change, create an output back to our address
    if (availableAmount > amount) {
        TransactionOutput changeOutput;
        changeOutput.address = address;
        changeOutput.amount = availableAmount - amount;
        outputs.push_back(changeOutput);
    }
    
    // Create the gold-backed transaction with PoP reference
    Transaction tx(inputs, outputs, lastTxHash, popReference);
    
    // Sign the inputs
    tx.signInputs(privateKey);
    
    // Update last transaction hash
    lastTxHash = tx.getHash();
    
    return tx;
}

double Wallet::getBalance(const std::unordered_map<std::string, TransactionOutput>& utxoSet) const {
    double balance = 0.0;
    
    // Sum up all unspent outputs belonging to this wallet
    for (const auto& [utxoKey, utxo] : utxoSet) {
        if (utxo.address == address) {
            balance += utxo.amount;
        }
    }
    
    return balance;
}

