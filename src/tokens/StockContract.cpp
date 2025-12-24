/**
 * Stock Contract Models Implementation
 * Supports all 3 legitimate asset models
 */

#include "../include/StockContract.h"
#include "../include/Logger.h"
#include <sstream>
#include <algorithm>

// ========================================
// MODEL 1: SYNTHETIC EQUITY
// ========================================

StockContract StockContract::createSyntheticEquity(
    const std::string& ticker,
    const std::string& companyName,
    const std::string& exchange,
    uint64_t tokenSupply,
    const std::string& priceSource
) {
    StockContract contract;
    
    // Asset classification
    contract.assetType = AssetType::SYNTHETIC_EQUITY;
    contract.settlementType = SettlementType::CASH_CRYPTO;
    contract.legalOwnership = false;
    contract.votingRights = false;
    contract.dividendRights = false;
    contract.redemptionRights = false;
    
    // Basic info
    contract.ticker = ticker;
    contract.companyName = companyName;
    contract.exchange = exchange;
    contract.totalShares = tokenSupply;  // Arbitrary supply
    contract.priceOracle = priceSource;
    
    // No custodian or issuer fields
    contract.custodian = "";
    contract.issuerAddress = "";
    
    LOG_INFO("Created SYNTHETIC EQUITY contract for " + ticker + 
             " with " + std::to_string(tokenSupply) + " tokens");
    
    return contract;
}

// ========================================
// MODEL 2: CUSTODIAL-BACKED
// ========================================

StockContract StockContract::createCustodialBacked(
    const std::string& ticker,
    const std::string& companyName,
    const std::string& exchange,
    uint64_t sharesHeld,
    const std::string& custodian,
    const std::string& proofOfReservesUrl,
    const std::string& auditFrequency,
    bool votingRights,
    bool dividendRights
) {
    StockContract contract;
    
    // Asset classification
    contract.assetType = AssetType::CUSTODIAL_BACKED;
    contract.settlementType = SettlementType::PHYSICAL_REDEMPTION;
    contract.legalOwnership = true;
    contract.votingRights = votingRights;
    contract.dividendRights = dividendRights;
    contract.redemptionRights = true;
    
    // Basic info
    contract.ticker = ticker;
    contract.companyName = companyName;
    contract.exchange = exchange;
    contract.totalShares = sharesHeld;  // MUST equal shares held
    
    // Custodial-specific fields
    contract.custodian = custodian;
    contract.proofOfReservesUrl = proofOfReservesUrl;
    contract.auditFrequency = auditFrequency;
    contract.lastAuditDate = std::time(nullptr);
    
    // No issuer fields
    contract.issuerAddress = "";
    
    LOG_INFO("Created CUSTODIAL-BACKED contract for " + ticker + 
             " with " + std::to_string(sharesHeld) + " shares held by " + custodian);
    
    return contract;
}

// ========================================
// MODEL 3: ISSUER-AUTHORIZED
// ========================================

StockContract StockContract::createIssuerAuthorized(
    const std::string& ticker,
    const std::string& companyName,
    const std::string& exchange,
    uint64_t sharesIssued,
    const std::string& issuerAddress,
    const std::string& capTableUrl,
    const std::string& shareholderRegistryUrl
) {
    StockContract contract;
    
    // Asset classification
    contract.assetType = AssetType::ISSUER_AUTHORIZED;
    contract.settlementType = SettlementType::COMPANY_REGISTRY;
    contract.legalOwnership = true;
    contract.votingRights = true;
    contract.dividendRights = true;
    contract.redemptionRights = false;  // Already on cap table
    
    // Basic info
    contract.ticker = ticker;
    contract.companyName = companyName;
    contract.exchange = exchange;
    contract.totalShares = sharesIssued;
    
    // Issuer-specific fields
    contract.issuerAddress = issuerAddress;
    contract.capTableUrl = capTableUrl;
    contract.shareholderRegistryUrl = shareholderRegistryUrl;
    
    // No custodian fields
    contract.custodian = "";
    
    LOG_INFO("Created ISSUER-AUTHORIZED contract for " + ticker + 
             " with " + std::to_string(sharesIssued) + " shares issued by " + issuerAddress);
    
    return contract;
}

// ========================================
// DISCLAIMER GENERATION
// ========================================

std::string StockContract::getDisclaimer() const {
    std::ostringstream oss;
    
    switch (assetType) {
        case AssetType::SYNTHETIC_EQUITY:
            oss << "⚠️ SYNTHETIC INSTRUMENT\n\n"
                << "This token tracks the price of " << ticker << " but DOES NOT represent legal ownership.\n\n"
                << "• NO voting rights\n"
                << "• NO dividend rights (unless MM pays)\n"
                << "• NO redemption for real shares\n"
                << "• Settlement in cash/crypto only\n\n"
                << "This is a derivative for price exposure only.";
            break;
            
        case AssetType::CUSTODIAL_BACKED:
            oss << "✅ CUSTODIAL-BACKED TOKEN\n\n"
                << "This token is backed 1:1 by real " << ticker << " shares held in custody.\n\n"
                << "• Legal ownership: YES\n"
                << "• Voting rights: " << (votingRights ? "YES" : "NO") << "\n"
                << "• Dividend rights: " << (dividendRights ? "YES" : "NO") << "\n"
                << "• Redeemable for real shares: YES\n\n"
                << "Custodian: " << custodian << "\n"
                << "Proof of Reserves: " << proofOfReservesUrl;
            break;
            
        case AssetType::ISSUER_AUTHORIZED:
            oss << "✅ ISSUER-AUTHORIZED TOKEN\n\n"
                << "This token is issued directly by " << companyName << " and represents real shares.\n\n"
                << "• Legal ownership: YES\n"
                << "• Voting rights: YES\n"
                << "• Dividend rights: YES\n"
                << "• Part of company cap table: YES\n\n"
                << "This is a legally recognized security.";
            break;
    }
    
    return oss.str();
}

// ========================================
// VALIDATION
// ========================================

bool StockContract::isValidStockContract() const {
    // Basic validation
    if (ticker.empty() || companyName.empty() || exchange.empty()) {
        return false;
    }
    
    if (totalShares == 0) {
        return false;
    }
    
    // Asset-type specific validation
    switch (assetType) {
        case AssetType::SYNTHETIC_EQUITY:
            // Synthetic: No special requirements
            // Token supply is arbitrary
            return true;
            
        case AssetType::CUSTODIAL_BACKED:
            // Custodial: Must have custodian and proof of reserves
            if (custodian.empty() || proofOfReservesUrl.empty()) {
                LOG_ERROR("Custodial-backed contract missing custodian or proof of reserves");
                return false;
            }
            // Token supply must be reasonable (not arbitrary)
            if (totalShares > 1000000000) {  // 1 billion max
                LOG_ERROR("Custodial-backed contract has unrealistic share count");
                return false;
            }
            return true;
            
        case AssetType::ISSUER_AUTHORIZED:
            // Issuer: Must have issuer address and cap table
            if (issuerAddress.empty() || capTableUrl.empty()) {
                LOG_ERROR("Issuer-authorized contract missing issuer or cap table");
                return false;
            }
            return true;
    }
    
    return false;
}

// ========================================
// SERIALIZATION
// ========================================

std::string StockContract::serialize() const {
    std::ostringstream oss;
    oss << "{"
        << "\"ticker\":\"" << ticker << "\","
        << "\"companyName\":\"" << companyName << "\","
        << "\"exchange\":\"" << exchange << "\","
        << "\"assetType\":" << static_cast<int>(assetType) << ","
        << "\"settlementType\":" << static_cast<int>(settlementType) << ","
        << "\"legalOwnership\":" << (legalOwnership ? "true" : "false") << ","
        << "\"votingRights\":" << (votingRights ? "true" : "false") << ","
        << "\"dividendRights\":" << (dividendRights ? "true" : "false") << ","
        << "\"redemptionRights\":" << (redemptionRights ? "true" : "false") << ","
        << "\"totalShares\":" << totalShares << ","
        << "\"custodian\":\"" << custodian << "\","
        << "\"issuerAddress\":\"" << issuerAddress << "\","
        << "\"currentPrice\":" << currentPrice.price
        << "}";
    return oss.str();
}

// ========================================
// CORPORATE ACTIONS (HANDLE DILUTION)
// ========================================

bool StockContract::executeSplit(double ratio) {
    LOG_INFO("Executing " + std::to_string(ratio) + ":1 split for " + ticker);
    
    switch (assetType) {
        case AssetType::SYNTHETIC_EQUITY:
            // Synthetic: Price adjusts automatically via price feed
            // Token supply does NOT change
            // We track VALUE, not cap table structure
            LOG_INFO("Synthetic equity: Price will adjust via feed, supply unchanged");
            return true;
            
        case AssetType::CUSTODIAL_BACKED: {
            // Custodial: Must mint new tokens proportionally
            // Because we represent real shares 1:1
            uint64_t oldSupply = totalShares;
            totalShares = static_cast<uint64_t>(totalShares * ratio);
            
            // Adjust all balances
            for (auto& [address, balance] : shareBalances) {
                balance = static_cast<uint64_t>(balance * ratio);
            }
            
            LOG_INFO("Custodial-backed: Minted new tokens. Supply: " + 
                    std::to_string(oldSupply) + " -> " + std::to_string(totalShares));
            return true;
        }
            
        case AssetType::ISSUER_AUTHORIZED: {
            // Issuer: Company handles split on cap table
            // Blockchain reflects the change
            uint64_t oldSupply2 = totalShares;
            totalShares = static_cast<uint64_t>(totalShares * ratio);
            
            for (auto& [address, balance] : shareBalances) {
                balance = static_cast<uint64_t>(balance * ratio);
            }
            
            LOG_INFO("Issuer-authorized: Split executed on cap table. Supply: " + 
                    std::to_string(oldSupply2) + " -> " + std::to_string(totalShares));
            return true;
        }
    }
    
    return false;
}

// ========================================
// STUB IMPLEMENTATIONS FOR REMAINING METHODS
// ========================================

StockContract::StockContract() 
    : assetType(AssetType::SYNTHETIC_EQUITY),
      settlementType(SettlementType::CASH_CRYPTO),
      legalOwnership(false),
      votingRights(false),
      dividendRights(false),
      redemptionRights(false),
      lastAuditDate(0),
      totalShares(0),
      tradingHalted(false),
      marketCap(0),
      dividendYield(0.0) {
}

bool StockContract::updatePrice(double newPrice, std::time_t timestamp, const std::string& popHash, const std::string& source) {
    currentPrice = {newPrice, timestamp, popHash, source};
    priceHistory.push_back(currentPrice);
    return true;
}

std::vector<StockPrice> StockContract::getPriceHistory(uint32_t) const { return priceHistory; }
bool StockContract::isPriceDataFresh(uint32_t maxAgeSeconds) const { return std::time(nullptr) - currentPrice.timestamp < maxAgeSeconds; }

bool StockContract::issueShares(const std::string& to, uint64_t shares) {
    shareBalances[to] += shares;
    totalShares += shares;
    return true;
}

bool StockContract::transferShares(const std::string& from, const std::string& to, uint64_t shares) {
    if (shareBalances[from] < shares) return false;
    shareBalances[from] -= shares;
    shareBalances[to] += shares;
    return true;
}

bool StockContract::burnShares(const std::string& from, uint64_t shares) {
    if (shareBalances[from] < shares) return false;
    shareBalances[from] -= shares;
    totalShares -= shares;
    return true;
}

uint64_t StockContract::proposeCorporateAction(ActionType actionType, double value, const std::string& proofHash, const std::string& description) {
    CorporateAction action{corporateActions.size(), actionType, std::time(nullptr), value, proofHash, description, false, 0};
    corporateActions.push_back(action);
    return action.id;
}

bool StockContract::executeCorporateAction(uint64_t actionId) {
    if (actionId >= corporateActions.size()) return false;
    corporateActions[actionId].executed = true;
    corporateActions[actionId].executedAt = std::time(nullptr);
    return true;
}

bool StockContract::voteCorporateAction(uint64_t, const std::string&, bool) { return true; }
std::vector<CorporateAction> StockContract::getPendingActions() const { return corporateActions; }
bool StockContract::distributeDividend(double, const std::string&) { return true; }
double StockContract::calculateDividendPayment(const std::string&) const { return 0.0; }
bool StockContract::claimDividend(const std::string&) { return true; }
bool StockContract::executeMerger(double, const std::string&) { return true; }

void StockContract::addAuthorizedIssuer(const std::string& issuer) { authorizedIssuers.push_back(issuer); }
void StockContract::removeAuthorizedIssuer(const std::string&) {}
bool StockContract::isAuthorizedIssuer(const std::string& issuer) const {
    return std::find(authorizedIssuers.begin(), authorizedIssuers.end(), issuer) != authorizedIssuers.end();
}

void StockContract::addAuthorizedExecutor(const std::string& executor) { authorizedExecutors.push_back(executor); }
void StockContract::removeAuthorizedExecutor(const std::string&) {}
bool StockContract::isAuthorizedExecutor(const std::string& executor) const {
    return std::find(authorizedExecutors.begin(), authorizedExecutors.end(), executor) != authorizedExecutors.end();
}

void StockContract::addAuthorizedOracle(const std::string& oracle) { authorizedOracles.push_back(oracle); }
void StockContract::removeAuthorizedOracle(const std::string&) {}
bool StockContract::isAuthorizedOracle(const std::string& oracle) const {
    return std::find(authorizedOracles.begin(), authorizedOracles.end(), oracle) != authorizedOracles.end();
}

void StockContract::setTransferRestriction(const std::string& address, bool restricted) { transferRestrictions[address] = restricted; }
bool StockContract::hasTransferRestriction(const std::string& address) const {
    auto it = transferRestrictions.find(address);
    return it != transferRestrictions.end() && it->second;
}

void StockContract::setKYCStatus(const std::string& address, bool verified) { kycVerified[address] = verified; }
bool StockContract::isKYCVerified(const std::string& address) const {
    auto it = kycVerified.find(address);
    return it != kycVerified.end() && it->second;
}

void StockContract::haltTrading() { tradingHalted = true; }
void StockContract::resumeTrading() { tradingHalted = false; }

std::string StockContract::getLastTransactionHash(const std::string& address) const {
    auto it = userLastStockTx.find(address);
    return it != userLastStockTx.end() ? it->second : "";
}

bool StockContract::verifyTransactionChain(const std::string&, uint32_t) const { return true; }

StockTransfer StockContract::createChainedTransfer(const std::string& from, const std::string& to, uint64_t shares, double price) {
    StockTransfer transfer;
    transfer.from = from;
    transfer.to = to;
    transfer.shares = shares;
    transfer.priceAtTransfer = price;
    transfer.timestamp = std::time(nullptr);
    transferHistory.push_back(transfer);
    return transfer;
}

uint64_t StockContract::balanceOf(const std::string& address) const {
    auto it = shareBalances.find(address);
    return it != shareBalances.end() ? it->second : 0;
}

double StockContract::getMarketCap() const { return currentPrice.price * totalShares; }
std::vector<std::string> StockContract::getShareholders() const {
    std::vector<std::string> holders;
    for (const auto& [address, balance] : shareBalances) {
        if (balance > 0) holders.push_back(address);
    }
    return holders;
}

std::vector<StockTransfer> StockContract::getTransferHistory(const std::string&) const { return transferHistory; }
double StockContract::getVolume24h() const { return 0.0; }
double StockContract::getPriceChange24h() const { return 0.0; }
double StockContract::getAveragePrice(uint32_t) const { return currentPrice.price; }
double StockContract::getVolatility(uint32_t) const { return 0.0; }

void StockContract::emitSharesIssued(const std::string&, uint64_t, double, const std::string&) {}
void StockContract::emitSharesTransferred(const std::string&, const std::string&, uint64_t, const std::string&) {}
void StockContract::emitPriceUpdated(double, std::time_t, const std::string&) {}
void StockContract::emitCorporateActionExecuted(uint64_t, ActionType) {}
void StockContract::emitDividendDistributed(double, double) {}

bool StockContract::isValidTransfer(const std::string&, const std::string&, uint64_t) const { return true; }

StockContract StockContract::deserialize(const std::string&) { return StockContract(); }

// StockIndexContract stubs
StockIndexContract::StockIndexContract(const std::string& nameIn, const std::string& symbolIn) 
    : indexName(nameIn), indexSymbol(symbolIn), indexValue(0.0), lastCalculation(0), totalSupply(0), rebalancingEnabled(false) {}

bool StockIndexContract::addComponent(const std::string&, double) { return true; }
bool StockIndexContract::removeComponent(const std::string&) { return true; }
bool StockIndexContract::changeComponentWeight(const std::string&, double) { return true; }
void StockIndexContract::calculateIndexValue() {}
double StockIndexContract::getTotalWeight() const { return 0.0; }
bool StockIndexContract::mintIndexTokens(const std::string&, uint64_t) { return true; }
bool StockIndexContract::burnIndexTokens(const std::string&, uint64_t) { return true; }
bool StockIndexContract::transferIndexTokens(const std::string&, const std::string&, uint64_t) { return true; }
void StockIndexContract::enableRebalancing() { rebalancingEnabled = true; }
void StockIndexContract::disableRebalancing() { rebalancingEnabled = false; }
bool StockIndexContract::rebalanceIndex() { return true; }
uint64_t StockIndexContract::balanceOf(const std::string&) const { return 0; }
double StockIndexContract::getPerformance(uint32_t) const { return 0.0; }
void StockIndexContract::addAuthorizedManager(const std::string& manager) { authorizedManagers.push_back(manager); }
void StockIndexContract::removeAuthorizedManager(const std::string&) {}
bool StockIndexContract::isAuthorizedManager(const std::string& manager) const {
    return std::find(authorizedManagers.begin(), authorizedManagers.end(), manager) != authorizedManagers.end();
}
bool StockIndexContract::isValidComponent(const std::string&) const { return true; }
std::string StockIndexContract::serialize() const { return "{}"; }
StockIndexContract StockIndexContract::deserialize(const std::string&) { return StockIndexContract("", ""); }
