#include <iostream>
#include <cassert>
#include "../include/transaction.h"
#include "../include/Utils.h"

int main() {
    std::cout << "Testing Transaction Serialization with Delimiter Safety..." << std::endl;

    // Create a dummy transaction
    Transaction tx;
    tx.setHash("dummyhash");
    tx.setTimestamp(1234567890);
    tx.setType(TransactionType::STAKE);
    tx.setMemo("Test|Memo|With|Pipes"); // This should be hex encoded

    TransactionInput input;
    input.txHash = "prevhash";
    input.outputIndex = 0;
    input.signature = "sig";
    input.amount = 100.0;
    input.publicKey = "pubkey";
    tx.addInput(input);

    TransactionOutput output;
    output.address = "addr";
    output.amount = 99.0;
    output.script = "OP_DUP|OP_HASH160|DATA"; // This should be hex encoded
    tx.addOutput(output);

    // Serialize
    std::string serialized = tx.serialize();
    std::cout << "Serialized: " << serialized << std::endl;

    // Check if pipes are present in encoded fields (simple check)
    // Actually we expect it NOT to break the pipe structure.

    // Deserialize
    Transaction tx2;
    if (tx2.deserialize(serialized)) {
        std::cout << "Deserialization successful." << std::endl;
        assert(tx2.getMemo() == "Test|Memo|With|Pipes");
        assert(tx2.getOutputs()[0].script == "OP_DUP|OP_HASH160|DATA");
        assert(tx2.getType() == TransactionType::STAKE);
        std::cout << "Verification passed." << std::endl;
    } else {
        std::cout << "Deserialization failed!" << std::endl;
        return 1;
    }

    return 0;
}
