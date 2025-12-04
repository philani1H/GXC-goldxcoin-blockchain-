#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <ctime>

struct GoldReserve {
    std::string vaultId;
    std::string vaultAddress;
    double goldGrams;
    std::string attestationHash;
    std::time_t lastAuditTime;
    std::string auditorSignature;
    bool isActive;
};

struct GoldTokenTransfer {
    std::string from;
    std::string to;
    double amount;
    std::string txHash;
    std::string prevTxHash;
    std::string popReference;
    std::time_t timestamp;
};

class GoldToken {
private:
    std::string tokenId;
    std::string symbol; // "GXC-G"
    std::string name;   // "GXC Gold Token"
    uint8_t decimals;   // 8 (same as GXC)

    // Supply tracking
    double totalSupply;
    double totalReserves;
    std::unordered_map<std::string, double> balances;

    // Reserve management
    std::unordered_map<std::string, GoldReserve> reserves;
    std::vector<std::string> authorizedIssuers;
    std::vector<std::string> authorizedAuditors;

    // Transaction tracking (chained hashes)
    std::unordered_map<std::string, std::string> lastTxHash;
    std::vector<GoldTokenTransfer> transferHistory;

    // PoP integration
    std::string popOracleAddress;
    std::string lastPriceReference;
    std::time_t lastPriceUpdate;

    // Compliance
    std::unordered_map<std::string, bool> blacklistedAddresses;
    bool transfersEnabled;

public:
    // Constructor
    GoldToken();

    // Token operations
    bool mint(const std::string& to, double goldGrams, const std::string& vaultAttestation);
    bool burn(const std::string& from, double goldGrams, const std::string& redemptionCert);
    bool transfer(const std::string& from, const std::string& to, double amount);

    // Reserve management
    bool addGoldReserve(const GoldReserve& reserve);
    bool removeGoldReserve(const std::string& vaultId, const std::string& auditorSignature);
    bool auditReserves(const std::string& vaultId, const std::string& auditorSignature);
    std::vector<GoldReserve> getActiveReserves() const;

    // Authorization
    void addAuthorizedIssuer(const std::string& issuer);
    void removeAuthorizedIssuer(const std::string& issuer);
    bool isAuthorizedIssuer(const std::string& issuer) const;

    void addAuthorizedAuditor(const std::string& auditor);
    void removeAuthorizedAuditor(const std::string& auditor);
    bool isAuthorizedAuditor(const std::string& auditor) const;

    // PoP integration
    void updatePriceReference(const std::string& popReference);
    std::string getCurrentPriceReference() const;
    bool isPriceDataFresh(uint32_t maxAgeSeconds = 3600) const;

    // Transaction chaining
    std::string getLastTransactionHash(const std::string& address) const;
    bool verifyTransactionChain(const std::string& address, uint32_t depth = 100) const;
    GoldTokenTransfer createChainedTransfer(const std::string& from, const std::string& to,
                                           double amount, const std::string& popRef);

    // Compliance
    void blacklistAddress(const std::string& address);
    void unblacklistAddress(const std::string& address);
    bool isBlacklisted(const std::string& address) const;
    void enableTransfers();
    void disableTransfers();

    // Queries
    double balanceOf(const std::string& address) const;
    double getTotalSupply() const { return totalSupply; }
    double getTotalReserves() const { return totalReserves; }
    double getReserveRatio() const;
    bool isFullyBacked() const;

    // Transaction history
    std::vector<GoldTokenTransfer> getTransferHistory(const std::string& address) const;
    std::vector<GoldTokenTransfer> getAllTransfers() const;
    GoldTokenTransfer getTransfer(const std::string& txHash) const;

    // Redemption
    std::string requestRedemption(const std::string& holder, double goldGrams,
                                 const std::string& deliveryAddress);
    bool processRedemption(const std::string& redemptionId, const std::string& vaultSignature);
    std::vector<std::string> getPendingRedemptions() const;

    // Audit and verification
    bool verifyReserves() const;
    std::string generateReserveProof() const;
    bool validateReserveProof(const std::string& proof) const;

    // Events (for logging)
    void emitMintEvent(const std::string& to, double amount, const std::string& vaultId);
    void emitBurnEvent(const std::string& from, double amount, const std::string& redemptionId);
    void emitTransferEvent(const std::string& from, const std::string& to, double amount);
    void emitReserveAuditEvent(const std::string& vaultId, double reserves, const std::string& auditor);

    // Getters
    std::string getTokenId() const { return tokenId; }
    std::string getSymbol() const { return symbol; }
    std::string getName() const { return name; }
    uint8_t getDecimals() const { return decimals; }
    bool areTransfersEnabled() const { return transfersEnabled; }

    // Constants
    static const double GRAMS_PER_TROY_OUNCE; // 31.1035
    static const uint32_t MAX_PRICE_AGE_SECONDS = 3600; // 1 hour
    static const double MIN_RESERVE_RATIO; // 1.0 (100% backed)

    // Serialization
    std::string serialize() const;
    static GoldToken deserialize(const std::string& data);
};

// Gold vault management
class GoldVault {
private:
    std::string vaultId;
    std::string name;
    std::string address;
    std::string publicKey;

    // Inventory
    double totalGoldGrams;
    std::unordered_map<std::string, double> goldBars;

    // Security
    std::vector<std::string> authorizedPersonnel;
    std::string securityLevel;

    // Compliance
    std::string regulatoryLicense;
    std::time_t licenseExpiry;
    std::vector<std::string> insurancePolicies;

public:
    // Constructor
    GoldVault(const std::string& vaultIdIn, const std::string& nameIn, const std::string& addressIn);

    // Gold management
    bool depositGold(const std::string& barId, double grams, const std::string& attestation);
    bool withdrawGold(const std::string& barId, double grams, const std::string& authorization);
    double getTotalGold() const { return totalGoldGrams; }

    // Attestation
    std::string generateAttestation(double grams, const std::string& purpose) const;
    bool verifyAttestation(const std::string& attestation, const std::string& signature) const;

    // Audit
    std::string conductAudit(const std::string& auditorId) const;
    bool validateAudit(const std::string& auditReport, const std::string& auditorSignature) const;

    // Authorization
    void addAuthorizedPersonnel(const std::string& personnel);
    void removeAuthorizedPersonnel(const std::string& personnel);
    bool isAuthorized(const std::string& personnel) const;

    // Getters
    std::string getVaultId() const { return vaultId; }
    std::string getName() const { return name; }
    std::string getAddress() const { return address; }
    std::string getPublicKey() const { return publicKey; }
    std::string getSecurityLevel() const { return securityLevel; }
    bool isLicenseValid() const;

    // Inventory management
    std::vector<std::string> getGoldBars() const;
    double getBarWeight(const std::string& barId) const;

    // Serialization
    std::string serialize() const;
    static GoldVault deserialize(const std::string& data);
};