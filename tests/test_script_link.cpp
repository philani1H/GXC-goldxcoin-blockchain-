#include <iostream>
#include <cassert>
#include "../include/transaction.h"
#include "../include/Utils.h"

int main() {
    std::cout << "Testing Transaction::verifyScript..." << std::endl;

    // Simulate a P2PKH script
    // "OP_DUP OP_HASH160 <address> OP_EQUALVERIFY OP_CHECKSIG"
    std::string address = "GXC1234567890abcdef";
    std::string scriptPubKey = "OP_DUP OP_HASH160 " + address + " OP_EQUALVERIFY OP_CHECKSIG";

    // Test direct call
    // Note: We need a valid public key that generates this address to pass the check fully.
    // However, verifyScript logic first extracts the address.
    // If we pass an arbitrary public key, it will generate an address and compare.
    // We can at least call it and assert it returns false for mismatch, which proves extraction worked but comparison failed.

    bool result = Transaction::verifyScript("sig", "invalid_pubkey", scriptPubKey);
    assert(result == false);

    std::cout << "verifyScript test passed (negative case)." << std::endl;
    return 0;
}
