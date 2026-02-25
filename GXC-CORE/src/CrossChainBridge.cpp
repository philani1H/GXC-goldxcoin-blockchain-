#include "../include/CrossChainBridge.h"
#include "../include/HashUtils.h"
#include <sstream>

CrossChainBridge::CrossChainBridge(uint32_t requiredSignaturesIn)
    : requiredSignatures(requiredSignaturesIn) {
    
    // Initialize confirmation blocks for different chains
    confirmationBlocks[ChainType::BITCOIN] = 6;
    confirmationBlocks[ChainType::ETHEREUM] = 12;
    confirmationBlocks[ChainType::BSC] = 15;
    confirmationBlocks[ChainType::SOLANA] = 32;
    confirmationBlocks[ChainType::POLKADOT] = 4;
    confirmationBlocks[ChainType::GXC] = 6;
}

void CrossChainBridge::addValidator(const BridgeValidator& validator) {
    validators.push_back(validator);
}

std::string CrossChainBridge::initiateTransfer(ChainType sourceChain, const std::string& sourceAsset, 
                                             double amount, const std::string& destinationAddress,
                                             const std::string& txHash) {
    // Create a transfer ID
    std::stringstream ss;
    ss << static_cast<int>(sourceChain) << sourceAsset << amount << destinationAddress << txHash << std::time(nullptr);
    std::string transferId = keccak256(ss.str());
    
    // Create a new transfer
    Transfer transfer;
    transfer.transferId = transferId;
    transfer.sourceChain = sourceChain;
    transfer.sourceAsset = sourceAsset;
    transfer.amount = amount;
    transfer.destinationAddress = destinationAddress;
    transfer.txHash = txHash;
    transfer.status = "PENDING";
    
    // Store the transfer
    transfers[transferId] = transfer;
    
    return transferId;
}

bool CrossChainBridge::verifyAndSignTransfer(const std::string& transferId, const std::string& validatorId, 
                                           const std::string& signature) {
    // Check if the transfer exists
    auto it = transfers.find(transferId);
    if (it == transfers.end()) {
        return false;
    }
    
    Transfer& transfer = it->second;
    
    // Check if the validator is authorized
    bool isAuthorized = false;
    for (const auto& validator : validators) {
        if (validator.id == validatorId && validator.isActive) {
            isAuthorized = true;
            break;
        }
    }
    
    if (!isAuthorized) {
        return false;
    }
    
    // Verify the transfer on the source chain
    if (!verifyTransferOnSourceChain(transfer.sourceChain, transfer.txHash, 
                                   transfer.sourceAsset, transfer.amount, 
                                   transfer.destinationAddress)) {
        return false;
    }
    
    // Add the signature
    transfer.signatures.push_back(signature);
    
    // Check if we have enough signatures to execute the transfer
    if (transfer.signatures.size() >= requiredSignatures) {
        executeTransfer(transferId);
    }
    
    return true;
}

bool CrossChainBridge::executeTransfer(const std::string& transferId) {
    // Check if the transfer exists
    auto it = transfers.find(transferId);
    if (it == transfers.end()) {
        return false;
    }
    
    Transfer& transfer = it->second;
    
    // Check if we have enough signatures
    if (transfer.signatures.size() < requiredSignatures) {
        return false;
    }
    
    // Execute the transfer (mint or release assets)
    // In a real implementation, this would interact with the blockchain
    
    // Update the transfer status
    transfer.status = "COMPLETED";
    
    return true;
}

bool CrossChainBridge::verifyTransferOnSourceChain(ChainType sourceChain, const std::string& txHash,
                                                 const std::string& sourceAsset, double amount,
                                                 const std::string& destinationAddress) {
    // In a real implementation, this would verify the transaction on the source chain
    // For simplicity, we'll just return true
    return true;
}

Transfer CrossChainBridge::getTransfer(const std::string& transferId) const {
    auto it = transfers.find(transferId);
    if (it != transfers.end()) {
        return it->second;
    }
    
    // Return an empty transfer if not found
    Transfer emptyTransfer;
    emptyTransfer.transferId = "";
    emptyTransfer.status = "NOT_FOUND";
    return emptyTransfer;
}
