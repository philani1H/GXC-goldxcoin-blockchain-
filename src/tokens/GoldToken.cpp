#include "GoldToken.h"
#include <iostream>

GoldToken::GoldToken() : tokenId("GXC-G"), symbol("GXC-G"), name("GXC Gold Token"), decimals(8), totalSupply(0), totalReserves(0), transfersEnabled(true) {}

bool GoldToken::mint(const std::string& to, double goldGrams, const std::string&) {
    if (!transfersEnabled) return false;
    balances[to] += goldGrams;
    totalSupply += goldGrams;
    totalReserves += goldGrams;
    emitMintEvent(to, goldGrams, "vault");
    return true;
}
bool GoldToken::burn(const std::string& from, double goldGrams, const std::string&) {
    if (balances[from] < goldGrams) return false;
    balances[from] -= goldGrams;
    totalSupply -= goldGrams;
    totalReserves -= goldGrams;
    emitBurnEvent(from, goldGrams, "redemption");
    return true;
}
bool GoldToken::transfer(const std::string& from, const std::string& to, double amount) {
    if (!transfersEnabled || balances[from] < amount) return false;
    balances[from] -= amount;
    balances[to] += amount;
    emitTransferEvent(from, to, amount);
    return true;
}
bool GoldToken::addGoldReserve(const GoldReserve& reserve) {
    reserves[reserve.vaultId] = reserve;
    totalReserves += reserve.goldGrams;
    return true;
}
bool GoldToken::removeGoldReserve(const std::string& vaultId, const std::string&) {
    auto it = reserves.find(vaultId);
    if (it != reserves.end()) { totalReserves -= it->second.goldGrams; reserves.erase(it); return true; }
    return false;
}
bool GoldToken::auditReserves(const std::string& vaultId, const std::string&) {
    return reserves.count(vaultId) > 0;
}
std::vector<GoldReserve> GoldToken::getActiveReserves() const {
    std::vector<GoldReserve> active;
    for (const auto& [_, r] : reserves) if (r.isActive) active.push_back(r);
    return active;
}
void GoldToken::addAuthorizedIssuer(const std::string& issuer) { authorizedIssuers.push_back(issuer); }
void GoldToken::removeAuthorizedIssuer(const std::string& issuer) { authorizedIssuers.erase(std::remove(authorizedIssuers.begin(), authorizedIssuers.end(), issuer), authorizedIssuers.end()); }
bool GoldToken::isAuthorizedIssuer(const std::string& issuer) const { return std::find(authorizedIssuers.begin(), authorizedIssuers.end(), issuer) != authorizedIssuers.end(); }
void GoldToken::addAuthorizedAuditor(const std::string& auditor) { authorizedAuditors.push_back(auditor); }
void GoldToken::removeAuthorizedAuditor(const std::string& auditor) { authorizedAuditors.erase(std::remove(authorizedAuditors.begin(), authorizedAuditors.end(), auditor), authorizedAuditors.end()); }
bool GoldToken::isAuthorizedAuditor(const std::string& auditor) const { return std::find(authorizedAuditors.begin(), authorizedAuditors.end(), auditor) != authorizedAuditors.end(); }
void GoldToken::updatePriceReference(const std::string& ref) { lastPriceReference = ref; lastPriceUpdate = std::time(nullptr); }
std::string GoldToken::getCurrentPriceReference() const { return lastPriceReference; }
bool GoldToken::isPriceDataFresh(uint32_t maxAgeSeconds) const { return std::time(nullptr) - lastPriceUpdate < maxAgeSeconds; }
std::string GoldToken::getLastTransactionHash(const std::string& address) const { auto it = lastTxHash.find(address); return it != lastTxHash.end() ? it->second : ""; }
bool GoldToken::verifyTransactionChain(const std::string&, uint32_t) const { return true; }
GoldTokenTransfer GoldToken::createChainedTransfer(const std::string& from, const std::string& to, double amount, const std::string& popRef) { GoldTokenTransfer t{from, to, amount, "txHash", getLastTransactionHash(from), popRef, std::time(nullptr)}; transferHistory.push_back(t); lastTxHash[to] = t.txHash; return t; }
void GoldToken::blacklistAddress(const std::string& address) { blacklistedAddresses[address] = true; }
void GoldToken::unblacklistAddress(const std::string& address) { blacklistedAddresses[address] = false; }
bool GoldToken::isBlacklisted(const std::string& address) const { auto it = blacklistedAddresses.find(address); return it != blacklistedAddresses.end() && it->second; }
void GoldToken::enableTransfers() { transfersEnabled = true; }
void GoldToken::disableTransfers() { transfersEnabled = false; }
double GoldToken::balanceOf(const std::string& address) const { auto it = balances.find(address); return it != balances.end() ? it->second : 0.0; }
double GoldToken::getReserveRatio() const { return totalReserves ? totalSupply / totalReserves : 0.0; }
bool GoldToken::isFullyBacked() const { return getReserveRatio() >= 1.0; }
std::vector<GoldTokenTransfer> GoldToken::getTransferHistory(const std::string& address) const { std::vector<GoldTokenTransfer> result; for (const auto& t : transferHistory) if (t.from == address || t.to == address) result.push_back(t); return result; }
std::vector<GoldTokenTransfer> GoldToken::getAllTransfers() const { return transferHistory; }
GoldTokenTransfer GoldToken::getTransfer(const std::string& txHash) const { for (const auto& t : transferHistory) if (t.txHash == txHash) return t; return {}; }
std::string GoldToken::requestRedemption(const std::string& holder, double, const std::string&) { return "redemptionId"; }
bool GoldToken::processRedemption(const std::string&, const std::string&) { return true; }
std::vector<std::string> GoldToken::getPendingRedemptions() const { return {}; }
bool GoldToken::verifyReserves() const { return isFullyBacked(); }
std::string GoldToken::generateReserveProof() const { return "reserve_proof"; }
bool GoldToken::validateReserveProof(const std::string&) const { return true; }
void GoldToken::emitMintEvent(const std::string& to, double amount, const std::string& vaultId) { std::cout << "[GoldToken] Minted " << amount << " to " << to << " from vault " << vaultId << std::endl; }
void GoldToken::emitBurnEvent(const std::string& from, double amount, const std::string& redemptionId) { std::cout << "[GoldToken] Burned " << amount << " from " << from << " for redemption " << redemptionId << std::endl; }
void GoldToken::emitTransferEvent(const std::string& from, const std::string& to, double amount) { std::cout << "[GoldToken] Transferred " << amount << " from " << from << " to " << to << std::endl; }
void GoldToken::emitReserveAuditEvent(const std::string& vaultId, double reserves, const std::string& auditor) { std::cout << "[GoldToken] Reserve audit for vault " << vaultId << ": " << reserves << " by " << auditor << std::endl; }
std::string GoldToken::serialize() const { return "<goldtoken_serialized>"; }
GoldToken GoldToken::deserialize(const std::string&) { return GoldToken(); }