#include <iostream>
#include <cassert>
#include "../include/Wallet.h"
#include "../include/transaction.h"

int main() {
    Wallet wallet;
    wallet.generateKeyPair();

    // Test persistence
    std::string testPath = "test_wallet.dat";
    assert(wallet.saveToFile(testPath));

    Wallet wallet2;
    assert(wallet2.loadFromFile(testPath));
    assert(wallet.getAddress() == wallet2.getAddress());
    assert(wallet.getPublicKey() == wallet2.getPublicKey());

    // Test Transaction Types
    Transaction tx;
    assert(tx.getType() == TransactionType::NORMAL);

    tx.setType(TransactionType::STAKE);
    assert(tx.getType() == TransactionType::STAKE);

    std::cout << "Wallet persistence and Transaction type tests passed!" << std::endl;
    return 0;
}
