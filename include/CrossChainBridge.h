#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "Validator.h"

enum class ChainType {
    GXC,
    BITCOIN,
    ETHEREUM,
    BSC,
    SOLANA,
    POLKADOT
};

struct BridgeValidator {
    std::string id;
    std::string publicKey;
    double collateral;
    bool isActive;
};

struct Transfer {
    std::string transferId;
    ChainType sourceChain;
    std::string sourceAsset;
    double amount;
    std::string destinationAddress;
    std::string txHash;
    std::string status;
    std::vector<std::string> signatures;
};

class CrossChainBridge {
private:
    std::vector<BridgeValidator> validators;
    std::unordered_map<std::string, Transfer> transfers;
    uint32_t requiredSignatures;
    std::unordered_map<ChainType, uint32_t> confirmationBlocks;
    
public:
    // Constructor
    CrossChainBridge(uint32_t requiredSignaturesIn);
    
    // Add a validator
    void addValidator(const BridgeValidator& validator);
    
    // Initiate a transfer
    std::string initiateTransfer(ChainType sourceChain, const std::string& sourceAsset, 
                               double amount, const std::string& destinationAddress,
                               const std::string& txHash);
    
    // Verify and sign a transfer
    bool verifyAndSignTransfer(const std::string& transferId, const std::string& validatorId, 
                              const std::string& signature);
    
    // Execute a transfer (mint or release assets)
    bool executeTransfer(const std::string& transferId);
    
    // Verify transfer on source chain
    bool verifyTransferOnSourceChain(ChainType sourceChain, const std::string& txHash,
                                   const std::string& sourceAsset, double amount,
                                   const std::string& destinationAddress);
    
    // Getters
    Transfer getTransfer(const std::string& transferId) const;
    uint32_t getRequiredSignatures() const { return requiredSignatures; }
};
