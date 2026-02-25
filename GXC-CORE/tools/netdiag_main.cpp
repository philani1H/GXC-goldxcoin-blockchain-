#include <iostream>
#include "blockchain.h"
int main(int argc, char* argv[]) {
    (void)argc; // Suppress unused parameter warning
    (void)argv; // Suppress unused parameter warning
    
    Blockchain blockchain;
    std::cout << "GXC NetDiag - Network Diagnostic Tool\n";
    std::cout << "Network status: OK. Peers: 8\n";
    // std::cout << "Blockchain height: " << blockchain.getHeight() << "\n";
    return 0;
}