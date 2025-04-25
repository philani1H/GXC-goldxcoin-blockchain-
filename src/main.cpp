#include <iostream>
#include "blockchain.h"
#include "transaction_tracker.h"
#include "pop_oracle.h"
#include "gold_token.h"

int main() {
    Blockchain gxcChain(4);
    TransactionTracker txTracker;
    PoPOracle oracle(3);
    GoldTokenManager goldMgr;

    // 1) Simulate Proof-of-Price submissions
    oracle.submit({"oracleA", 1950.30});
    oracle.submit({"oracleB", 1948.75});
    oracle.submit({"oracleC", 1951.10});  // triggers aggregate()

    // 2) Mint gold-backed tokens
    goldMgr.mint("Alice",  10.0);
    goldMgr.mint("Bob",     5.5);

    // 3) Create and chain regular transactions
    auto tx1 = txTracker.create("Alice", "Bob",   2.0);
    auto tx2 = txTracker.create("Bob",   "Carol", 1.25);
    gxcChain.addBlock({ tx1, tx2 });

    // 4) Burn gold token & record redemption tx
    goldMgr.burn("Alice", 1.0);
    auto tx3 = txTracker.create("Alice", "Vault", 1.0);
    gxcChain.addBlock({ tx3 });

    // 5) Validate and report
    std::cout << "\nChain valid? "
              << (gxcChain.isValid() ? "Yes" : "No") << "\n";
    std::cout << "Alice GXC-G balance: "
              << goldMgr.balanceOf("Alice") << " g\n";

    return 0;
}
