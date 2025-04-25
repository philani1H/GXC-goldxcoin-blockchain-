#pragma once
#include <string>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>

// Simple SHA256 helper
static std::string sha256(const std::string &input) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, input.c_str(), input.size());
    SHA256_Final(hash, &ctx);

    std::ostringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    return ss.str();
}

struct Transaction {
    std::string sender;
    std::string receiver;
    double amount;
    std::string prevTxHash; // for chained traceability
    std::string txHash;

    Transaction(const std::string& s,
                const std::string& r,
                double a,
                const std::string& prev)
      : sender(s), receiver(r), amount(a), prevTxHash(prev)
    {
        txHash = calculateHash();
    }

    std::string calculateHash() const {
        std::ostringstream ss;
        ss << sender << receiver << amount << prevTxHash;
        return sha256(ss.str());
    }
};
