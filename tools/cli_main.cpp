#include <iostream>
int main(int argc, char* argv[]) {
    std::cout << "GXC CLI - Command Line Interface\n";
    if (argc < 2) {
        std::cout << "Usage: gxc-cli <command> [options]\n";
        std::cout << "Commands: stake, wallet, help\n";
        return 0;
    }
    std::string cmd = argv[1];
    if (cmd == "stake") {
        std::cout << "[CLI] Staking command executed.\n";
    } else if (cmd == "wallet") {
        std::cout << "[CLI] Wallet command executed.\n";
    } else {
        std::cout << "Unknown command: " << cmd << "\n";
    }
    return 0;
}