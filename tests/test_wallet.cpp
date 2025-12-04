#include <gtest/gtest.h>
#include "../include/Wallet.h"
#include "../include/transaction.h"

TEST(WalletTest, Creation) {
    Wallet w;
    
    // Wallet should have an address after creation
    EXPECT_FALSE(w.getAddress().empty());
    
    // With empty UTXO set, balance should be 0
    std::unordered_map<std::string, TransactionOutput> emptyUtxoSet;
    EXPECT_EQ(w.getBalance(emptyUtxoSet), 0.0);
}

TEST(WalletTest, AddressFormat) {
    Wallet w;
    std::string address = w.getAddress();
    
    // Address should start with correct prefix
    // Testnet: tGXC, Mainnet: GXC
    bool validPrefix = (address.substr(0, 4) == "tGXC" || address.substr(0, 3) == "GXC");
    EXPECT_TRUE(validPrefix);
}

TEST(WalletTest, KeyGeneration) {
    Wallet w;
    
    // Wallet should have a valid address (implying keys were generated)
    EXPECT_FALSE(w.getAddress().empty());
    EXPECT_TRUE(w.getAddress().length() > 10);
}

TEST(WalletTest, BalanceWithUtxo) {
    Wallet w;
    std::string address = w.getAddress();
    
    // Create a UTXO set with some coins for this wallet
    std::unordered_map<std::string, TransactionOutput> utxoSet;
    
    TransactionOutput output;
    output.address = address;
    output.amount = 100.0;
    output.script = "P2PKH";
    
    utxoSet["tx123_0"] = output;
    
    // Balance should reflect the UTXO
    EXPECT_EQ(w.getBalance(utxoSet), 100.0);
}
