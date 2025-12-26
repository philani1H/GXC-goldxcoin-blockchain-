#include "../include/Wallet.h"
#include "../include/HashUtils.h"
#include "../include/Crypto.h"
#include "../include/Config.h"

#include <random>
#include <sstream>
#include <iomanip>
#include <fstream>
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
    // IMPORTANT: Iterate through UTXOs in sorted order (oldest first)
    // This helps avoid using immature coinbase outputs
    if (availableAmount < totalRequired) {
        // Convert to vector and sort by UTXO key (which includes txHash)
        // Older transactions will generally have earlier hashes
        std::vector<std::pair<std::string, TransactionOutput>> sortedUTXOs(utxoSet.begin(), utxoSet.end());
        std::sort(sortedUTXOs.begin(), sortedUTXOs.end(), 
                  [](const auto& a, const auto& b) { return a.first < b.first; });
        
        for (const auto& [utxoKey, utxo] : sortedUTXOs) {
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

                    // Note: Coinbase maturity is checked during transaction validation
                    // The blockchain will reject transactions using immature coinbase outputs
                    // We could check here, but it requires blockchain access to determine
                    // if the source transaction is a coinbase and its height
                    // For now, let validation handle it (will be rejected if immature)

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

// ============= THIRD-PARTY WALLET SUPPORT IMPLEMENTATION =============

bool Wallet::importPrivateKey(const std::string& privateKeyHex, const std::string& label) {
    // Validate private key format (should be 64 hex chars = 32 bytes)
    if (privateKeyHex.length() != 64) {
        return false;
    }
    
    // Check if all characters are valid hex
    for (char c : privateKeyHex) {
        if (!isxdigit(c)) {
            return false;
        }
    }
    
    // Derive public key and address from private key
    std::string derivedPubKey = Crypto::derivePublicKey(privateKeyHex);
    if (derivedPubKey.empty()) {
        return false;
    }
    
    // Generate address from public key
    // Detect if testnet based on current wallet or config
    bool isTestnet = Config::isTestnet() || isTestnetAddress(address);
    std::string derivedAddress = Crypto::generateAddress(derivedPubKey, isTestnet);
    
    if (derivedAddress.empty() || !isValidAddress(derivedAddress)) {
        return false;
    }
    
    // Store the imported key
    importedPrivateKeys[derivedAddress] = privateKeyHex;
    importedPublicKeys[derivedAddress] = derivedPubKey;
    importedAddresses[derivedAddress] = label.empty() ? "imported" : label;
    
    return true;
}

bool Wallet::importAddress(const std::string& addr, const std::string& label) {
    // Validate address format
    if (!isValidAddress(addr)) {
        return false;
    }
    
    // Check if already imported with private key (don't downgrade to watch-only)
    if (importedPrivateKeys.find(addr) != importedPrivateKeys.end()) {
        // Already have full control, just update label
        importedAddresses[addr] = label.empty() ? "imported" : label;
        return true;
    }
    
    // Add as watch-only address
    importedAddresses[addr] = label.empty() ? "watch-only" : label;
    
    return true;
}

bool Wallet::controlsAddress(const std::string& addr) const {
    // Check main wallet address
    if (addr == address) {
        return true;
    }
    
    // Check imported addresses (with private keys)
    if (importedPrivateKeys.find(addr) != importedPrivateKeys.end()) {
        return true;
    }
    
    // Watch-only addresses are "controlled" in the sense we can track them
    if (importedAddresses.find(addr) != importedAddresses.end()) {
        return true;
    }
    
    return false;
}

bool Wallet::canSignForAddress(const std::string& addr) const {
    // Check main wallet address
    if (addr == address && !privateKey.empty()) {
        return true;
    }
    
    // Check imported addresses with private keys
    if (importedPrivateKeys.find(addr) != importedPrivateKeys.end()) {
        return true;
    }
    
    return false;
}

std::string Wallet::getPrivateKeyForAddress(const std::string& addr) const {
    // Main wallet address
    if (addr == address) {
        return privateKey;
    }
    
    // Imported address
    auto it = importedPrivateKeys.find(addr);
    if (it != importedPrivateKeys.end()) {
        return it->second;
    }
    
    return "";
}

std::string Wallet::getPublicKeyForAddress(const std::string& addr) const {
    // Main wallet address
    if (addr == address) {
        return publicKey;
    }
    
    // Imported address
    auto it = importedPublicKeys.find(addr);
    if (it != importedPublicKeys.end()) {
        return it->second;
    }
    
    return "";
}

std::vector<std::string> Wallet::getAllControlledAddresses() const {
    std::vector<std::string> addresses;
    
    // Add main wallet address
    if (!address.empty()) {
        addresses.push_back(address);
    }
    
    // Add all imported addresses with private keys
    for (const auto& [addr, key] : importedPrivateKeys) {
        addresses.push_back(addr);
    }
    
    return addresses;
}

std::vector<std::string> Wallet::getWatchOnlyAddresses() const {
    std::vector<std::string> watchOnly;
    
    for (const auto& [addr, label] : importedAddresses) {
        // Only return addresses that don't have private keys
        if (importedPrivateKeys.find(addr) == importedPrivateKeys.end()) {
            watchOnly.push_back(addr);
        }
    }
    
    return watchOnly;
}

bool Wallet::setActiveAddress(const std::string& addr) {
    // Can only switch to addresses we can sign for
    if (!canSignForAddress(addr)) {
        return false;
    }
    
    // If switching to an imported address, update main wallet
    if (addr != address) {
        auto privKeyIt = importedPrivateKeys.find(addr);
        auto pubKeyIt = importedPublicKeys.find(addr);
        
        if (privKeyIt != importedPrivateKeys.end() && pubKeyIt != importedPublicKeys.end()) {
            // Store old main wallet as imported
            if (!address.empty() && !privateKey.empty()) {
                importedPrivateKeys[address] = privateKey;
                importedPublicKeys[address] = publicKey;
                importedAddresses[address] = "previous_main";
            }
            
            // Set new main wallet
            privateKey = privKeyIt->second;
            publicKey = pubKeyIt->second;
            address = addr;
            
            // Remove from imported (now it's main)
            importedPrivateKeys.erase(addr);
            importedPublicKeys.erase(addr);
            importedAddresses.erase(addr);
            
            return true;
        }
    }
    
    return addr == address; // Already active if it's the main address
}

Transaction Wallet::createTransactionFrom(const std::string& fromAddress,
                                          const std::string& toAddress, 
                                          double amount,
                                          const std::unordered_map<std::string, TransactionOutput>& utxoSet,
                                          double fee) {
    // Verify we can sign for this address
    if (!canSignForAddress(fromAddress)) {
        throw std::runtime_error("Cannot sign for address: " + fromAddress);
    }
    
    // Get the private key for signing
    std::string signingKey = getPrivateKeyForAddress(fromAddress);
    if (signingKey.empty()) {
        throw std::runtime_error("No private key found for address: " + fromAddress);
    }
    
    // Use default fee if not specified
    if (fee <= 0) {
        fee = 0.001;
    }
    
    double totalRequired = amount + fee;
    
    std::vector<TransactionInput> inputs;
    std::vector<TransactionOutput> outputs;
    
    double availableAmount = 0.0;
    
    // Find unspent outputs for the specified address
    for (const auto& [utxoKey, utxo] : utxoSet) {
        if (utxo.address == fromAddress) {
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
        throw std::runtime_error("Insufficient funds: need " + std::to_string(totalRequired) + 
                                " GXC, have: " + std::to_string(availableAmount) + " GXC");
    }
    
    // Create output to recipient
    TransactionOutput recipientOutput;
    recipientOutput.address = toAddress;
    recipientOutput.amount = amount;
    outputs.push_back(recipientOutput);
    
    // Calculate change
    double change = availableAmount - amount - fee;
    if (change > 0.00000001) {
        TransactionOutput changeOutput;
        changeOutput.address = fromAddress;
        changeOutput.amount = change;
        outputs.push_back(changeOutput);
    }
    
    // Create transaction
    std::string prevTxHash = inputs.empty() ? "0" : inputs[0].txHash;
    double referencedAmount = inputs.empty() ? 0.0 : inputs[0].amount;
    
    Transaction tx(inputs, outputs, prevTxHash);
    tx.setFee(fee);
    tx.setSenderAddress(fromAddress);
    tx.setReceiverAddress(toAddress);
    tx.setReferencedAmount(referencedAmount);
    
    // Sign with the appropriate private key
    tx.signInputs(signingKey);
    
    return tx;
}

Transaction Wallet::createStakeTransactionFrom(const std::string& fromAddress,
                                               double stakeAmount, 
                                               const std::unordered_map<std::string, TransactionOutput>& utxoSet,
                                               double fee) {
    // Verify we can sign for this address
    if (!canSignForAddress(fromAddress)) {
        throw std::runtime_error("Cannot sign for address: " + fromAddress);
    }
    
    std::string signingKey = getPrivateKeyForAddress(fromAddress);
    if (signingKey.empty()) {
        throw std::runtime_error("No private key found for address: " + fromAddress);
    }
    
    if (fee <= 0) {
        fee = 0.001;
    }
    
    double totalRequired = stakeAmount + fee;
    
    std::vector<TransactionInput> inputs;
    std::vector<TransactionOutput> outputs;
    
    double availableAmount = 0.0;
    
    // Find unspent outputs for the specified address
    for (const auto& [utxoKey, utxo] : utxoSet) {
        if (utxo.address == fromAddress) {
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
    
    // Calculate change (staked amount is locked, not an output)
    double change = availableAmount - stakeAmount - fee;
    if (change > 0.00000001) {
        TransactionOutput changeOutput;
        changeOutput.address = fromAddress;
        changeOutput.amount = change;
        outputs.push_back(changeOutput);
    }
    
    std::string prevTxHash = inputs.empty() ? "0" : inputs[0].txHash;
    double referencedAmount = inputs.empty() ? 0.0 : inputs[0].amount;
    
    Transaction tx(inputs, outputs, prevTxHash);
    tx.setType(TransactionType::STAKE);
    tx.setFee(fee);
    tx.setSenderAddress(fromAddress);
    tx.setReceiverAddress(fromAddress);
    tx.setMemo("Stake: " + std::to_string(stakeAmount) + " GXC");
    tx.setReferencedAmount(referencedAmount);
    
    // Sign with the appropriate private key
    tx.signInputs(signingKey);
    
    return tx;
}

// ============= END THIRD-PARTY WALLET SUPPORT =============

