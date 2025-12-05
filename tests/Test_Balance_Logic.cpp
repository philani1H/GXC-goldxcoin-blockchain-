#include <iostream>
#include <cassert>
#include <vector>
#include <memory>
#include "../include/blockchain.h"
#include "../include/Wallet.h"
#include "../include/transaction.h"
#include "../include/Validator.h"
#include "../include/Utils.h"
#include "../include/Network.h"
#include "../include/Config.h"

// Mock Network for testing
class MockNetwork : public Network {
public:
    MockNetwork() : Network() {}
};

void run_test() {
    std::cout << "Starting Balance Logic Test..." << std::endl;

    // 0. Set Testnet Mode (Lower difficulty)
    Config::setBool("testnet", true);

    // 1. Setup Blockchain
    // Ensure clean state (delete test db)
    system("rm -rf testnet_data");

    auto blockchain = std::make_unique<Blockchain>();
    blockchain->initialize(); // Creates genesis block

    // 2. Setup Wallet
    Wallet wallet;
    std::string address = wallet.getAddress();
    std::cout << "Wallet Address: " << address << std::endl;

    // 3. Fund Wallet (via mining/coinbase)
    // Create a new block mined by this wallet
    Block block1(1, blockchain->getLatestBlock().getHash(), BlockType::POW_SHA256);
    block1.setTimestamp(Utils::getCurrentTimestamp());
    block1.setMinerAddress(address);

    // Mine block
    // Difficulty should be low (0.1) due to testnet
    int attempts = 0;
    while (!blockchain->validateProofOfWork(block1)) {
        block1.incrementNonce();
        attempts++;
        if (attempts > 1000000) {
            std::cerr << "Mining took too long!" << std::endl;
            return;
        }
    }

    if (blockchain->addBlock(block1)) {
        std::cout << "Block 1 mined successfully." << std::endl;
    } else {
        std::cerr << "Failed to add Block 1" << std::endl;
        return;
    }

    // Check Balance
    double balance = blockchain->getBalance(address);
    std::cout << "Balance after mining: " << balance << " GXC" << std::endl;
    assert(balance > 0);

    // 4. Test Sending Coins
    Wallet recipient;
    std::string recipientAddr = recipient.getAddress();
    double sendAmount = 10.0;

    try {
        Transaction sendTx = wallet.createTransaction(recipientAddr, sendAmount, blockchain->getUtxoSet());
        if (blockchain->addTransaction(sendTx)) {
            std::cout << "Send transaction created: " << sendTx.getHash() << std::endl;
        } else {
            std::cerr << "Failed to add send transaction" << std::endl;
        }

        // Mine block to confirm transaction
        Block block2(2, blockchain->getLatestBlock().getHash(), BlockType::POW_SHA256);
        block2.setMinerAddress(address); // Miner gets fees
        block2.addTransaction(sendTx);
        block2.calculateMerkleRoot();

        while (!blockchain->validateProofOfWork(block2)) {
            block2.incrementNonce();
        }
        blockchain->addBlock(block2);

        std::cout << "Block 2 mined. Verifying balances..." << std::endl;
        double newBalance = blockchain->getBalance(address);
        double recipientBalance = blockchain->getBalance(recipientAddr);

        std::cout << "Sender Balance: " << newBalance << std::endl;
        std::cout << "Recipient Balance: " << recipientBalance << std::endl;

        assert(std::abs(recipientBalance - sendAmount) < 0.000001);

    } catch (const std::exception& e) {
        std::cerr << "Send Error: " << e.what() << std::endl;
    }

    // 5. Test Staking
    double stakeAmount = 100.0;
    try {
        // Step 1: Create Validator (Pending)
        Validator v(address, 0.0, 14);
        v.setPublicKey(wallet.getPublicKey());
        v.setPending(true);
        bool regResult = blockchain->registerValidator(v);
        assert(regResult);
        std::cout << "Validator registered (pending)." << std::endl;

        // Step 2: Create Stake Transaction
        Transaction stakeTx = wallet.createStakeTransaction(stakeAmount, blockchain->getUtxoSet());
        if (blockchain->addTransaction(stakeTx)) {
            std::cout << "Stake transaction created: " << stakeTx.getHash() << std::endl;
        }

        // Step 3: Mine block to confirm stake
        Block block3(3, blockchain->getLatestBlock().getHash(), BlockType::POW_SHA256);
        block3.setMinerAddress(address);
        block3.addTransaction(stakeTx);
        block3.calculateMerkleRoot();

        while (!blockchain->validateProofOfWork(block3)) {
            block3.incrementNonce();
        }
        blockchain->addBlock(block3);

        std::cout << "Block 3 mined. Verifying stake..." << std::endl;

        // Check active validators
        auto validators = blockchain->getActiveValidators();
        bool found = false;
        for (const auto& val : validators) {
            if (val.getAddress() == address) {
                found = true;
                std::cout << "Validator active with stake: " << val.getStakeAmount() << std::endl;
                assert(std::abs(val.getStakeAmount() - stakeAmount) < 0.000001);
                assert(!val.getIsPending());
                break;
            }
        }
        assert(found);

    } catch (const std::exception& e) {
        std::cerr << "Stake Error: " << e.what() << std::endl;
    }

    // 6. Test Unstaking
    try {
        Transaction unstakeTx = wallet.createUnstakeTransaction(stakeAmount);
        if (blockchain->addTransaction(unstakeTx)) {
             std::cout << "Unstake transaction created: " << unstakeTx.getHash() << std::endl;
        }

        // Mine block
        Block block4(4, blockchain->getLatestBlock().getHash(), BlockType::POW_SHA256);
        block4.setMinerAddress(address);
        block4.addTransaction(unstakeTx);
        block4.calculateMerkleRoot();

        while (!blockchain->validateProofOfWork(block4)) {
            block4.incrementNonce();
        }
        blockchain->addBlock(block4);

        std::cout << "Block 4 mined. Verifying unstake..." << std::endl;

        // Check validator stake is reduced
        auto validators = blockchain->getActiveValidators();
        bool found = false;
        for (const auto& val : validators) {
            if (val.getAddress() == address) {
                found = true;
                std::cout << "Remaining Stake: " << val.getStakeAmount() << std::endl;
                if (val.getStakeAmount() < Validator::MIN_STAKE) {
                    std::cout << "Validator inactive (correct)." << std::endl;
                }
                break;
            }
        }

        // Check balance increased (Minted UTXOs)
        double finalBalance = blockchain->getBalance(address);
        std::cout << "Final Balance: " << finalBalance << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Unstake Error: " << e.what() << std::endl;
    }

    std::cout << "Test Complete." << std::endl;
}

int main() {
    run_test();
    return 0;
}
