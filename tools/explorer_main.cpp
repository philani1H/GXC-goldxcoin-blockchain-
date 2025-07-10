#include <iostream>
#include "blockchain.h"
int main(int argc, char* argv[]) {
    Blockchain blockchain;
    auto block = blockchain.getLatestBlock();
    std::cout << "GXC Explorer - Blockchain Explorer\n";
    std::cout << "Block Height: " << blockchain.getHeight() << ", Hash: " << block.getHash() << ", TxCount: " << block.getTransactions().size() << "\n";
    return 0;
}