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
    
    // Generate address from public key based on network configuration
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

        return !privateKey.empty() && !publicKey.empty() && !address.empty();
    } catch (...) {
        return false;
    }
}

void Wallet::signTransaction(Transaction& tx) {
    tx.signInputs(privateKey);
}

Transaction Wallet::createTransaction(const std::string& recipientAddress, double amount, 
                                     const std::unordered_map<std::string, TransactionOutput>& utxoSet,
                                     double fee) {
    // Use default fee if not specified
    if (fee <= 0) {
        fee = 0.001; // Default minimum fee
    }
    
    double totalRequired = amount + fee;
    
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
                    }
                }
            }
        }
    }

    // Find other unspent outputs if needed
    if (availableAmount < totalRequired) {
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
                    if (availableAmount >= totalRequired) {
                        break;
                    }
                }
            }
        }
    }
    
    // Check if we have enough funds (amount + fee)
    if (availableAmount < totalRequired) {
        throw std::runtime_error("Insufficient funds: need " + std::to_string(totalRequired) + 
                                " GXC (amount: " + std::to_string(amount) + " + fee: " + std::to_string(fee) + 
                                "), have: " + std::to_string(availableAmount) + " GXC");
    }
    
    // Create the output to the recipient
    TransactionOutput recipientOutput;
    recipientOutput.address = recipientAddress;
    recipientOutput.amount = amount;
    outputs.push_back(recipientOutput);
    
    // Calculate change (input - output - fee)
    double change = availableAmount - amount - fee;
    
    // If there's change, create an output back to our address
    if (change > 0.00000001) { // Use epsilon for floating point comparison
        TransactionOutput changeOutput;
        changeOutput.address = address;
        changeOutput.amount = change;
        outputs.push_back(changeOutput);
    }
    
    // TRACEABILITY FORMULA ENFORCEMENT:
    // Ti.Inputs[0].txHash == Ti.PrevTxHash
    // Ti.Inputs[0].amount == Ti.ReferencedAmount
    //
    // The prevTxHash MUST equal inputs[0].txHash for the formula to hold
    std::string prevTxHash;
    double referencedAmount = 0.0;
    
    if (!inputs.empty()) {
        // CRITICAL: prevTxHash must match the first input's txHash
        prevTxHash = inputs[0].txHash;
        referencedAmount = inputs[0].amount;
    } else {
        // No inputs means genesis-like transaction (shouldn't happen for normal sends)
        prevTxHash = "0";
        referencedAmount = 0.0;
    }
    
    // Create the transaction with traceability-compliant prevTxHash
    Transaction tx(inputs, outputs, prevTxHash);
    tx.setFee(fee);
    tx.setSenderAddress(address);
    tx.setReceiverAddress(recipientAddress);
    tx.setReferencedAmount(referencedAmount);
    
    // Sign the inputs
    tx.signInputs(privateKey);
    
    // Update last transaction hash
    lastTxHash = tx.getHash();
    
    return tx;
}

Transaction Wallet::createStakeTransaction(double stakeAmount, 
                                          const std::unordered_map<std::string, TransactionOutput>& utxoSet,
                                          double fee) {
    // STAKE transaction: consume UTXOs, lock the staked amount
    // Only create change output - no output for the staked amount (it's locked)
    
    if (fee <= 0) {
        fee = 0.001; // Default minimum fee
    }
    
    double totalRequired = stakeAmount + fee;
    
    std::vector<TransactionInput> inputs;
    std::vector<TransactionOutput> outputs;
    
    double availableAmount = 0.0;
    
    // Find unspent outputs belonging to this wallet
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
                
                availableAmount += utxo.amount;
                
                if (availableAmount >= totalRequired) {
                    break;
                }
            }
        }
    }
    
    if (availableAmount < totalRequired) {
        throw std::runtime_error("Insufficient funds for staking: need " + std::to_string(totalRequired) + 
                                " GXC, have: " + std::to_string(availableAmount) + " GXC");
    }
    
    // Calculate change = totalInput - stakeAmount - fee
    // The staked amount is NOT an output - it's "burned" into the stake pool
    double change = availableAmount - stakeAmount - fee;
    
    // Only create change output if there's change
    if (change > 0.00000001) {
        TransactionOutput changeOutput;
        changeOutput.address = address;
        changeOutput.amount = change;
        outputs.push_back(changeOutput);
    }
    
    // TRACEABILITY FORMULA ENFORCEMENT:
    // Ti.Inputs[0].txHash == Ti.PrevTxHash
    // Ti.Inputs[0].amount == Ti.ReferencedAmount
    std::string prevTxHash;
    double referencedAmount = 0.0;
    
    if (!inputs.empty()) {
        // CRITICAL: prevTxHash must match the first input's txHash
        prevTxHash = inputs[0].txHash;
        referencedAmount = inputs[0].amount;
    } else {
        prevTxHash = "0";
        referencedAmount = 0.0;
    }
    
    Transaction tx(inputs, outputs, prevTxHash);
    tx.setType(TransactionType::STAKE);
    tx.setFee(fee);
    tx.setSenderAddress(address);
    tx.setReceiverAddress(address); // Staking is self-referential
    tx.setMemo("Stake: " + std::to_string(stakeAmount) + " GXC");
    tx.setReferencedAmount(referencedAmount);
    
    // Sign the inputs
    tx.signInputs(privateKey);
    
    // Update last transaction hash
    lastTxHash = tx.getHash();
    
    return tx;
}

Transaction Wallet::createUnstakeTransaction(double unstakeAmount, double fee) {
    // UNSTAKE transaction: creates new spendable coins from staked balance
    // This is a special transaction with no inputs (funds come from stake pool)
    
    if (fee <= 0) {
        fee = 0.0; // No fee for unstaking (no inputs to pay from)
    }
    
    std::vector<TransactionInput> inputs;
    std::vector<TransactionOutput> outputs;
    
    // Create a dummy input to link to previous transaction for signing
    // This provides proof of ownership
    TransactionInput dummyInput;
    dummyInput.txHash = lastTxHash.empty() ? "stake_withdrawal" : lastTxHash;
    dummyInput.outputIndex = 0;
    dummyInput.amount = 0.0; // No actual UTXO being spent
    inputs.push_back(dummyInput);
    
    // Create output for the unstaked amount
    TransactionOutput unstakeOutput;
    unstakeOutput.address = address;
    unstakeOutput.amount = unstakeAmount;
    outputs.push_back(unstakeOutput);
    
    // Create the unstake transaction
    Transaction tx(inputs, outputs, lastTxHash);
    tx.setType(TransactionType::UNSTAKE);
    tx.setFee(fee);
    tx.setSenderAddress(address);
    tx.setReceiverAddress(address);
    tx.setMemo("Unstake: " + std::to_string(unstakeAmount) + " GXC");
    tx.setReferencedAmount(0.0); // No referenced amount for unstake
    
    // Sign to prove ownership
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

