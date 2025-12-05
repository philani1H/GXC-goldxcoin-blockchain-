#include <iostream>
#include <cassert>
#include "../include/transaction.h"
#include "../include/Utils.h"

int main() {
    std::cout << "Testing Transaction Serialization..." << std::endl;

    // Create a dummy transaction
    Transaction tx;
    tx.setHash("dummyhash");
    tx.setTimestamp(1234567890);
    tx.setType(TransactionType::STAKE);

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
    output.script = "script";
    tx.addOutput(output);

    // Serialize
    std::string serialized = tx.serialize();
    std::cout << "Serialized: " << serialized << std::endl;

    // Deserialize
    Transaction tx2;
    if (tx2.deserialize(serialized)) {
        std::cout << "Deserialization successful." << std::endl;
        assert(tx2.getHash() == "dummyhash");
        assert(tx2.getType() == TransactionType::STAKE);
        assert(tx2.getInputs().size() == 1);
        assert(tx2.getOutputs().size() == 1);
        assert(tx2.getInputs()[0].txHash == "prevhash");
        std::cout << "Verification passed." << std::endl;
    } else {
        std::cout << "Deserialization failed!" << std::endl;
        return 1;
    }

    return 0;
}
