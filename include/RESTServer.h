#pragma once

#include <string>
#include <thread>
#include <atomic>
#include <cstdint>
#include <unordered_map>
#include <nlohmann/json.hpp>

class Blockchain;
class FraudDetection;
class MarketMakerAdmin;

class RESTServer {
private:
    Blockchain* blockchain;
    FraudDetection* fraudDetection;
    MarketMakerAdmin* adminSystem;
    class ReversalFeePool* feePool;
    uint16_t serverPort;
    std::atomic<bool> isRunning;
    std::thread serverThread;
    
    void serverLoop();
    std::string handleRequest(const std::string& method, const std::string& path, const std::string& body);
    std::string getBlockchainInfo();
    std::string getBlock(const std::string& blockId);
    std::string getBlocks();
    std::string getTransaction(const std::string& txHash);
    std::string submitTransaction(const std::string& body);
    std::string getAddressBalance(const std::string& address);
    std::string getAddressTransactions(const std::string& address);
    std::string traceTransaction(const std::string& txHash);
    std::string getMiningInfo();
    std::string getPeerInfo();
    std::string getStats();
    
    // Admin authentication
    std::string adminLogin(const std::string& body);
    std::string adminLogout(const std::string& sessionToken);
    std::string verifySession(const std::string& sessionToken);
    
    // Admin management (super_admin only)
    std::string createAdmin(const std::string& sessionToken, const std::string& body);
    std::string updateAdminPermissions(const std::string& sessionToken, const std::string& body);
    std::string deactivateAdmin(const std::string& sessionToken, const std::string& body);
    std::string listAllAdmins(const std::string& sessionToken);
    
    // Fraud reporting endpoints (public)
    std::string reportStolenFunds(const std::string& body);
    std::string checkTransactionTaint(const std::string& txHash);
    std::string checkAddressFraud(const std::string& address);
    
    // Fraud management endpoints (admin only)
    std::string getPendingFraudReports(const std::string& sessionToken);
    std::string getFraudReportDetails(const std::string& sessionToken, const std::string& reportId);
    std::string approveFraudReport(const std::string& sessionToken, const std::string& body);
    std::string rejectFraudReport(const std::string& sessionToken, const std::string& body);
    std::string assignFraudReport(const std::string& sessionToken, const std::string& body);
    std::string getFraudStatistics(const std::string& sessionToken);
    
    // Market Maker management endpoints (admin only)
    std::string submitMMApplication(const std::string& body);
    std::string getPendingMMApplications(const std::string& sessionToken);
    std::string getMMApplicationDetails(const std::string& sessionToken, const std::string& appId);
    std::string getMMDocument(const std::string& sessionToken, const std::string& appId, const std::string& docType);
    std::string listMMDocuments(const std::string& sessionToken, const std::string& appId);
    std::string verifyMMLicense(const std::string& sessionToken, const std::string& body);
    std::string approveMMApplication(const std::string& sessionToken, const std::string& body);
    std::string rejectMMApplication(const std::string& sessionToken, const std::string& body);
    
    // Dashboard endpoints
    std::string getAdminDashboard(const std::string& sessionToken);
    std::string getAuditLog(const std::string& sessionToken);
    std::string getSystemStatistics(const std::string& sessionToken);
    
    // Reversal system endpoints
    std::string getReversalPoolStats(const std::string& sessionToken);
    std::string recordPoolFunding(const std::string& sessionToken, const std::string& body);
    
    std::string createErrorResponse(int code, const std::string& error, const std::string& message);
    std::string jsonToString(const nlohmann::json& json);
    bool isAuthorizedAdmin(const std::string& adminKey);
    
public:
    RESTServer(Blockchain* blockchain, uint16_t port);
    RESTServer(Blockchain* blockchain, FraudDetection* fraud, MarketMakerAdmin* admin, uint16_t port);
    ~RESTServer();
    
    bool start();
    void stop();
    void processRequests();
    
    // Set components (if not set in constructor)
    void setFraudDetection(FraudDetection* fraud);
    void setReversalFeePool(class ReversalFeePool* pool);
    void setAdminSystem(MarketMakerAdmin* admin);
};
