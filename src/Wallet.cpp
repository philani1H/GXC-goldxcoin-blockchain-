#include "../include/Wallet.h"
#include "../include/HashUtils.h"
#include "../include/Crypto.h"
#include "../include/Config.h"

#include <random>
#include <sstream>
#include <iomanip>
#include <fstream>

Wallet::Wallet() {
    generateKeyPair();
}

void Wallet::generateKeyPair() {
    // Use proper secp256k1 ECDSA key generation
    Crypto::KeyPair keyPair = Crypto::generateKeyPair();
    
    privateKey = keyPair.privateKey;
    publicKey = keyPair.publicKey;
    
    // Generate address from public key (testnet detection would be done elsewhere)
    // For now, generate mainnet address
    address = Crypto::generateAddress(publicKey, Config::isTestnet());
    
    // Initialize last transaction hash
    lastTxHash = "";
}

bool Wallet::saveToFile(const std::string& filepath) const {
    try {
        std::ofstream file(filepath);
        if (!file.is_open()) {
            return false;
        }

        file << privateKey << "\n";
        file << publicKey << "\n";
        file << address << "\n";
        file << lastTxHash << "\n";

        return true;
    } catch (...) {
        return false;
    }
}

bool Wallet::loadFromFile(const std::string& filepath) {
    try {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            return false;
        }

        std::string line;
        if (std::getline(file, line)) privateKey = line;
        if (std::getline(file, line)) publicKey = line;
        if (std::getline(file, line)) address = line;
        if (std::getline(file, line)) lastTxHash = line;

        // Re-derive address to match current network configuration
        // This ensures that if we are on testnet, we use the testnet address format
        // for the same keypair, even if the file stored the mainnet format.
        if (!publicKey.empty()) {
            address = Crypto::generateAddress(publicKey, Config::isTestnet());
        }

        return !privateKey.empty() && !publicKey.empty() && !address.empty();
    } catch (...) {
        return false;
    }
}

void Wallet::signTransaction(Transaction& tx) {
    tx.signInputs(privateKey);
}

Transaction Wallet::createTransaction(const std::string& recipientAddress, double amount, 
                                     const std::unordered_map<std::string, TransactionOutput>& utxoSet) {
    // Find unspent outputs to use as inputs
    std::vector<TransactionInput> inputs;
    std::vector<TransactionOutput> outputs;
    
    double availableAmount = 0.0;
    
    // Traceability Requirement: Prioritize UTXO from last transaction (chaining)
    if (!lastTxHash.empty()) {
        for (const auto& [utxoKey, utxo] : utxoSet) {
            if (utxo.address == address) {
                size_t sepPos = utxoKey.find('_');
                if (sepPos != std::string::npos) {
                    std::string txHash = utxoKey.substr(0, sepPos);
                    if (txHash == lastTxHash) {
                        // Found previous transaction output, use it first
                        uint32_t outputIndex = std::stoul(utxoKey.substr(sepPos + 1));

                        TransactionInput input;
                        input.txHash = txHash;
                        input.outputIndex = outputIndex;
                        input.amount = utxo.amount; // Critical for traceability
                        inputs.push_back(input);

                        availableAmount += utxo.amount;
                        // Don't break immediately, keep collecting outputs from the last tx if multiple exist
                        // break;
                    }
                }
            }
        }
    }

    // Find other unspent outputs if needed
    if (availableAmount < amount) {
        for (const auto& [utxoKey, utxo] : utxoSet) {
            if (utxo.address == address) {
                // Parse the UTXO key to get the transaction hash and output index
                size_t sepPos = utxoKey.find('_');
                if (sepPos != std::string::npos) {
                    std::string txHash = utxoKey.substr(0, sepPos);
                    uint32_t outputIndex = std::stoul(utxoKey.substr(sepPos + 1));

                    // Check if already added (avoid duplicates)
                    bool alreadyAdded = false;
                    for (const auto& inp : inputs) {
                        if (inp.txHash == txHash && inp.outputIndex == outputIndex) {
                            alreadyAdded = true;
                            break;
                        }
                    }
                    if (alreadyAdded) continue;

                    // Create an input
                    TransactionInput input;
                    input.txHash = txHash;
                    input.outputIndex = outputIndex;
                    input.amount = utxo.amount;
                    inputs.push_back(input);

                    availableAmount += utxo.amount;

                    // If we have enough funds, break
                    if (availableAmount >= amount) {
                        break;
                    }
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
            size_t sepPos = utxoKey.find('_');
            if (sepPos != std::string::npos) {
                std::string txHash = utxoKey.substr(0, sepPos);
                uint32_t outputIndex = std::stoul(utxoKey.substr(sepPos + 1));
                
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

