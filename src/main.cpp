#include <iostream>
#include <memory>
#include "../include/Blockchain.h"
#include "../include/Wallet.h"
#include "../include/ProofOfPrice.h"
#include "../include/CrossChainBridge.h"

int main() {
    std::cout << "Initializing GXC Blockchain..." << std::endl;
    
    // Create the blockchain
    std::shared_ptr<Blockchain> blockchain = std::make_shared<Blockchain>();
    
    // Create wallets
    Wallet minerWallet;
    Wallet userWallet;
    
    std::cout << "Miner address: " << minerWallet.getAddress() << std::endl;
    std::cout << "User address: " << userWallet.getAddress() << std::endl;
    
    // Create validators
    Validator validator1(minerWallet.getAddress(), 1000.0, 30);
    Validator validator2(userWallet.getAddress(), 500.0, 60);
    
    // Register validators
    blockchain->registerValidator(validator1);
    blockchain->registerValidator(validator2);
    
    // Create PoP oracle
    std::shared_ptr<ProofOfPrice> popOracle = std::make_shared<ProofOfPrice>();
    
    // Add oracles
    Oracle oracle1("oracle1", "oracle1_public_key");
    Oracle oracle2("oracle2", "oracle2_public_key");
    Oracle oracle3("oracle3", "oracle3_public_key");
    
    popOracle->addOracle(oracle1);
    popOracle->addOracle(oracle2);
    popOracle->addOracle(oracle3);
    
    // Submit price data
    PriceSubmission submission1 = oracle1.submitPrice(2000.0, "oracle1_private_key");
    PriceSubmission submission2 = oracle2.submitPrice(2010.0, "oracle2_private_key");
    PriceSubmission submission3 = oracle3.submitPrice(1990.0, "oracle3_private_key");
    
    popOracle->submitPrice(submission1);
    popOracle->submitPrice(submission2);
    popOracle->submitPrice(submission3);
    
    // Get aggregated price data
    PriceData priceData = popOracle->getCurrentPrice();
    std::cout << "Current gold price: $" << priceData.price << std::endl;
    
    // Create cross-chain bridge
    std::shared_ptr<CrossChainBridge> bridge = std::make_shared<CrossChainBridge>(2);
    
    // Add bridge validators
    BridgeValidator bridgeValidator1;
    bridgeValidator1.id = "bridge_validator1";
    bridgeValidator1.publicKey = "bridge_validator1_public_key";
    bridgeValidator1.collateral = 1000.0;
    bridgeValidator1.isActive = true;
    
    BridgeValidator bridgeValidator2;
    bridgeValidator2.id = "bridge_validator2";
    bridgeValidator2.publicKey = "bridge_validator2_public_key";
    bridgeValidator2.collateral = 1500.0;
    bridgeValidator2.isActive = true;
    
    bridge->addValidator(bridgeValidator1);
    bridge->addValidator(bridgeValidator2);
    
    // Mine some blocks
    std::cout << "Mining blocks..." << std::endl;
    
    for (int i = 0; i < 10; i++) {
        // Create a new block
        Block newBlock = blockchain->createBlock(
            i % 4 == 3 ? BlockType::POS : (i % 2 == 0 ? BlockType::POW_SHA256 : BlockType::POW_ETHASH),
            minerWallet.getAddress()
        );
        
        // Mine the block
        newBlock.mineBlock(blockchain->getDifficulty());
        
        // Add the block to the chain
        blockchain->addBlock(newBlock);
        
        std::cout << "Block #" << i + 1 << " mined with hash: " << newBlock.getHash().substr(0, 10) << "..." << std::endl;
    }
    
    std::cout << "Blockchain initialized with " << blockchain->getChainLength() << " blocks." << std::endl;
    
    return 0;
}
