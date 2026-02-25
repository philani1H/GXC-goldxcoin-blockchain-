#include <iostream>
#include "blockchain.h"
#include "transaction.h"
int main(int argc, char* argv[]) {
    Blockchain blockchain;
    Transaction tx;
    blockchain.addTransaction(tx);
    std::cout << "GXC TX - Transaction Tool\n";
    std::cout << "Transaction submitted. Hash: " << tx.getHash() << "\n";
    return 0;
}