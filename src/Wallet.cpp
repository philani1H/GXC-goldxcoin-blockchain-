#include "../include/Wallet.h"
#include "../include/HashUtils.h"
#include "../include/Crypto.h"
#include "../include/Config.h"
#include "../include/Utils.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>

Wallet::Wallet() {
    generateKeyPair();
}

void Wallet::generateKeyPair() {
    // Use proper secp256k1 ECDSA key generation
    Crypto::KeyPair keyPair = Crypto::generateKeyPair();
    privateKey = keyPair.privateKey;
    publicKey = keyPair.publicKey;
    
    // Generate address from public key based on network configuration
    // This fixes the issue where wallet generated Mainnet addresses (GXC...)
    // while running on Testnet (expecting tGXC...), causing "Insufficient funds"
    // because UTXOs were not recognized as belonging to the wallet.
    bool isTestnet = Config::isTestnet();
    address = Crypto::generateAddress(publicKey, isTestnet);
    
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
        if (std::getline(file, line)) privateKey = Utils::trim(line);
        if (std::getline(file, line)) publicKey = Utils::trim(line);
        if (std::getline(file, line)) address = Utils::trim(line);
        if (std::getline(file, line)) lastTxHash = Utils::trim(line);
        
        // Re-derive address based on current network configuration to ensure correctness
        // regardless of what's in the file (e.g. if file was from Mainnet but running Testnet)
        bool isTestnet = Config::isTestnet();
        address = Crypto::generateAddress(publicKey, isTestnet);
        
        return !privateKey.empty() && !publicKey.empty() && !address.empty();
    } catch (...) {
        return false;
    }
}

void Wallet::signTransaction(Transaction& tx) {
    tx.signInputs(privateKey);
}

Transaction Wallet::createTransaction(const std::string& recipientAddress, double amount,
                                     const std::unordered_map<std::string, UTXO>& utxoSet) {
    // Find unspent outputs to use as inputs
    std::vector<TransactionInput> inputs;
    std::vector<TransactionOutput> outputs;
    double availableAmount = 0.0;
    double fee = 0.001; // Standard transaction fee
    double requiredTotal = amount + fee;
    
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
                    }
                }
            }
        }
    }
    
    // Find other unspent outputs if needed
    if (availableAmount < requiredTotal) {
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
                    if (availableAmount >= requiredTotal) {
                        break;
                    }
                }
            }
        }
    }
    
    // Check if we have enough funds
    if (availableAmount < requiredTotal) {
        throw std::runtime_error("Insufficient funds (Amount + Fee required: " + 
                               std::to_string(requiredTotal) + ")");
    }
    
    // Create the output to the recipient
    TransactionOutput recipientOutput;
    recipientOutput.address = recipientAddress;
    recipientOutput.amount = amount;
    outputs.push_back(recipientOutput);
    
    // If there's change, create an output back to our address
    if (availableAmount > requiredTotal) {
        TransactionOutput changeOutput;
        changeOutput.address = address;
        changeOutput.amount = availableAmount - requiredTotal;
        outputs.push_back(changeOutput);
    }
    
    // Create the transaction
    // Explicitly set prevTxHash to inputs[0].txHash to ensure traceability
    std::string correctPrevTxHash = !inputs.empty() ? inputs[0].txHash : lastTxHash;
    Transaction tx(inputs, outputs, correctPrevTxHash);
    tx.setFee(fee);
    
    // Sign the inputs
    tx.signInputs(privateKey);
    
    // Update last transaction hash
    lastTxHash = tx.getHash();
    
    return tx;
}

Transaction Wallet::createStakeTransaction(double stakeAmount,
                                          const std::unordered_map<std::string, UTXO>& utxoSet) {
    // Find unspent outputs to use as inputs
    std::vector<TransactionInput> inputs;
    std::vector<TransactionOutput> outputs;
    double availableAmount = 0.0;
    double fee = 0.001; // Standard fee
    double requiredTotal = stakeAmount + fee;
    
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
                    }
                }
            }
        }
    }
    
    // Find other unspent outputs if needed
    if (availableAmount < requiredTotal) {
        for (const auto& [utxoKey, utxo] : utxoSet) {
            if (utxo.address == address) {
                // Parse the UTXO key
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
                    if (availableAmount >= requiredTotal) {
                        break;
                    }
                }
            }
        }
    }
    
    // Check if we have enough funds
    if (availableAmount < requiredTotal) {
        throw std::runtime_error("Insufficient funds for stake + fee");
    }
    
    // Create NO output for the stake amount (it is burned/locked)
    // Only create change output if there is excess
    if (availableAmount > requiredTotal) {
        TransactionOutput changeOutput;
        changeOutput.address = address;
        changeOutput.amount = availableAmount - requiredTotal;
        outputs.push_back(changeOutput);
    }
    
    // Create the transaction
    // Explicitly set prevTxHash to inputs[0].txHash to ensure traceability
    std::string correctPrevTxHash = !inputs.empty() ? inputs[0].txHash : lastTxHash;
    Transaction tx(inputs, outputs, correctPrevTxHash);
    tx.setType(TransactionType::STAKE);
    tx.setFee(fee);
    
    // Explicitly set ReferencedAmount to match Input[0].amount for traceability
    if (!inputs.empty()) {
        tx.setReferencedAmount(inputs[0].amount);
    }
    
    // Sign the inputs
    tx.signInputs(privateKey);
    
    // Update last transaction hash
    lastTxHash = tx.getHash();
    
    return tx;
}

Transaction Wallet::createUnstakeTransaction(double unstakeAmount,
                                            const std::unordered_map<std::string, UTXO>& utxoSet) {
    // UNSTAKE Transaction Strategy:
    // We want to link to the last transaction for traceability.
    // We should consume a UTXO if available to maintain the "chain" of ownership,
    // even if we are net-gaining funds.
    // Input: One valid UTXO (preferably the last one from this wallet).
    // Output: Input.Amount + UnstakeAmount - Fee.
    
    std::vector<TransactionInput> inputs;
    std::vector<TransactionOutput> outputs;
    double inputAmount = 0.0;
    
    // Find the last UTXO for traceability
    bool foundLast = false;
    if (!lastTxHash.empty()) {
        for (const auto& [utxoKey, utxo] : utxoSet) {
            if (utxo.address == address) {
                size_t sepPos = utxoKey.find('_');
                if (sepPos != std::string::npos) {
                    std::string txHash = utxoKey.substr(0, sepPos);
                    if (txHash == lastTxHash) {
                        uint32_t outputIndex = std::stoul(utxoKey.substr(sepPos + 1));
                        TransactionInput input;
                        input.txHash = txHash;
                        input.outputIndex = outputIndex;
                        input.amount = utxo.amount;
                        inputs.push_back(input);
                        inputAmount += utxo.amount;
                        foundLast = true;
                        break; // Just need one link
                    }
                }
            }
        }
    }
    
    // If no "last" UTXO found (maybe spent or fresh wallet?), try ANY UTXO
    if (!foundLast) {
        for (const auto& [utxoKey, utxo] : utxoSet) {
            if (utxo.address == address) {
                size_t sepPos = utxoKey.find('_');
                if (sepPos != std::string::npos) {
                    std::string txHash = utxoKey.substr(0, sepPos);
                    uint32_t outputIndex = std::stoul(utxoKey.substr(sepPos + 1));
                    TransactionInput input;
                    input.txHash = txHash;
                    input.outputIndex = outputIndex;
                    input.amount = utxo.amount;
                    inputs.push_back(input);
                    inputAmount += utxo.amount;
                    foundLast = true;
                    break;
                }
            }
        }
    }
    
    // Fallback: Create a dummy input pointing to lastTxHash with 0 amount.
    if (!foundLast) {
        TransactionInput input;
        input.txHash = lastTxHash.empty() ? "0" : lastTxHash;
        input.outputIndex = 0; // Dummy
        input.amount = 0.0;
        inputs.push_back(input);
        // inputAmount is 0
    }
    
    double fee = 0.001;
    double totalOutput = inputAmount + unstakeAmount - fee;
    
    if (totalOutput < 0) {
        // Should not happen unless unstakeAmount is tiny and input is 0 and fee is high
        throw std::runtime_error("Unstake amount too small to cover fee");
    }
    
    TransactionOutput output;
    output.address = address;
    output.amount = totalOutput;
    outputs.push_back(output);
    
    // Explicitly set prevTxHash to inputs[0].txHash to ensure traceability
    std::string correctPrevTxHash = !inputs.empty() ? inputs[0].txHash : lastTxHash;
    Transaction tx(inputs, outputs, correctPrevTxHash);
    tx.setType(TransactionType::UNSTAKE);
    tx.setFee(fee);
    
    // Explicitly set ReferencedAmount to match Input[0].amount for traceability
    if (!inputs.empty()) {
        tx.setReferencedAmount(inputs[0].amount);
    }
    
    tx.signInputs(privateKey);
    lastTxHash = tx.getHash();
    
    return tx;
}

Transaction Wallet::createGoldBackedTransaction(const std::string& recipientAddress, double amount,
                                               const std::unordered_map<std::string, UTXO>& utxoSet,
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

double Wallet::getBalance(const std::unordered_map<std::string, UTXO>& utxoSet) const {
    double balance = 0.0;
    
    // Sum up all unspent outputs belonging to this wallet
    for (const auto& [utxoKey, utxo] : utxoSet) {
        if (utxo.address == address) {
            balance += utxo.amount;
        }
    }
    
    return balance;
}