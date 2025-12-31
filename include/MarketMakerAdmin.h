#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <ctime>
#include <memory>
#include "../third_party/json/json.hpp"

using json = nlohmann::json;

/**
 * Market Maker Application Status
 */
enum class ApplicationStatus {
    PENDING,              // Application submitted, awaiting review
    UNDER_REVIEW,         // GXC team reviewing application
    LICENSE_VERIFIED,     // License verified with regulatory body
    REPUTATION_CHECKED,   // Company reputation checked
    FINANCIAL_REVIEWED,   // Financial standing reviewed
    TECHNICAL_VERIFIED,   // Technical capabilities verified
    KYC_AML_COMPLETED,    // KYC/AML compliance completed
    APPROVED,             // Application approved
    REJECTED,             // Application rejected
    REVOKED               // Authorization revoked
};

/**
 * Verification Step
 */
struct VerificationStep {
    std::string stepName;
    ApplicationStatus status;
    std::string verifiedBy;      // Admin who verified
    std::time_t verifiedAt;
    std::string notes;
    std::string proofDocument;   // Hash of verification document
    bool passed;
};

/**
 * Market Maker Application
 */
struct MarketMakerApplication {
    std::string applicationId;
    std::string applicantAddress;
    std::string companyName;
    std::string licenseNumber;
    std::string regulatoryBody;  // SEC, FCA, MAS, etc.
    std::string country;
    std::string contactEmail;
    std::string contactPhone;
    std::string website;
    
    // Documents
    std::string licenseDocumentHash;
    std::string financialStatementsHash;
    std::string technicalCapabilitiesHash;
    std::string kycDocumentsHash;
    
    // Application status
    ApplicationStatus status;
    std::time_t submittedAt;
    std::time_t lastUpdatedAt;
    
    // Verification steps
    std::vector<VerificationStep> verificationSteps;
    
    // Final decision
    std::string approvedBy;      // Admin who approved/rejected
    std::time_t decidedAt;
    std::string decisionNotes;
    
    // If approved
    std::string marketMakerId;
    std::time_t activatedAt;
};

/**
 * Admin User
 */
struct AdminUser {
    std::string adminId;
    std::string username;
    std::string passwordHash;
    std::string role;            // "super_admin", "fraud_admin", "fraud_approver", "fraud_reviewer", "verifier", "reviewer"
    std::vector<std::string> permissions;
    bool isActive;
    std::time_t createdAt;
    std::time_t lastLoginAt;
    std::string sessionToken;    // Current session token
    std::time_t sessionExpiry;   // Session expiration time
};

/**
 * Fraud Report (for admin management)
 * 
 * IMPORTANT: Admin approval validates FACTS (fraud legitimacy), not execution.
 * The protocol independently validates feasibility and executes automatically.
 * 
 * Status Flow:
 * 1. PENDING: Awaiting admin review
 * 2. FACTS_APPROVED: Admin confirmed fraud is legitimate
 * 3. FACTS_REJECTED: Admin determined fraud claim is invalid
 * 4. PROTOCOL_VALIDATING: System checking mathematical feasibility
 * 5. EXECUTED: Protocol validated and executed reversal
 * 6. INFEASIBLE: Protocol determined reversal is mathematically impossible
 */
struct FraudReport {
    std::string reportId;
    std::string txHash;
    std::string reporterAddress;
    double amount;
    std::string email;
    std::string description;
    std::string evidence;
    uint64_t timestamp;
    
    // Admin fact validation (NOT execution approval)
    std::string factsStatus;     // "PENDING", "FACTS_APPROVED", "FACTS_REJECTED"
    std::string reviewedBy;      // Admin ID who reviewed facts
    uint64_t reviewedAt;
    std::string reviewNotes;     // Admin's notes on fraud legitimacy
    std::string assignedTo;      // Admin ID assigned to review
    
    // Protocol execution status (independent of admin)
    std::string executionStatus; // "NOT_STARTED", "VALIDATING", "EXECUTED", "INFEASIBLE"
    std::string proofHash;       // Hash of generated proof
    uint64_t validatedAt;        // When protocol validated
    uint64_t executedAt;         // When reversal executed
    uint64_t recoveredAmount;    // Amount actually recovered
    std::string executionNotes;  // Protocol's notes on feasibility/execution
};

/**
 * Audit Log Entry
 */
struct AuditLogEntry {
    std::string logId;
    std::string adminId;
    std::string action;          // "verify_license", "approve_application", etc.
    std::string applicationId;
    std::string details;
    std::time_t timestamp;
    std::string ipAddress;
};

/**
 * Market Maker Admin API
 * Handles market maker application verification and approval
 */
class MarketMakerAdmin {
private:
    std::unordered_map<std::string, MarketMakerApplication> applications;
    std::unordered_map<std::string, AdminUser> admins;
    std::unordered_map<std::string, FraudReport> fraudReports;
    std::unordered_map<std::string, std::string> sessionTokens; // token -> adminId
    std::vector<AuditLogEntry> auditLog;
    
    // Generate unique IDs
    std::string generateApplicationId();
    std::string generateAdminId();
    std::string generateLogId();
    std::string generateReportId();
    std::string generateSessionToken();
    
    // Verify admin permissions
    bool verifyAdminPermission(const std::string& adminId, const std::string& permission);
    bool verifySessionToken(const std::string& sessionToken, std::string& adminId);
    
    // Log action
    void logAction(const std::string& adminId, const std::string& action, 
                   const std::string& targetId, const std::string& details);
    
    // Fraud detection system (for reversal execution)
    class FraudDetection* fraudDetection;
    
public:
    MarketMakerAdmin();
    
    // Set fraud detection system (called after initialization)
    void setFraudDetection(class FraudDetection* fd) {
        fraudDetection = fd;
    }
    
    // ===== ADMIN MANAGEMENT =====
    
    /**
     * Create admin user
     * Requires: super_admin permission
     */
    std::string createAdmin(
        const std::string& superAdminId,
        const std::string& username,
        const std::string& password,
        const std::string& role,
        const std::vector<std::string>& permissions
    );
    
    /**
     * Admin login
     * Returns: JSON with session token and admin info
     */
    json adminLogin(const std::string& username, const std::string& password);
    
    /**
     * Admin logout
     */
    bool adminLogout(const std::string& sessionToken);
    
    /**
     * Verify admin session
     * Returns: Admin ID if valid, empty string if invalid
     */
    std::string verifyAdminSession(const std::string& sessionToken);
    
    /**
     * Get admin from session token
     */
    AdminUser getAdminFromSession(const std::string& sessionToken);
    
    /**
     * Get admin info
     */
    AdminUser getAdminInfo(const std::string& adminId);
    
    /**
     * Change admin password
     * Admin can change their own password
     */
    bool changePassword(
        const std::string& adminId,
        const std::string& oldPassword,
        const std::string& newPassword
    );
    
    /**
     * Update admin permissions
     * Requires: super_admin permission
     */
    bool updateAdminPermissions(
        const std::string& superAdminId,
        const std::string& targetAdminId,
        const std::vector<std::string>& newPermissions
    );
    
    /**
     * Deactivate admin
     * Requires: super_admin permission
     */
    bool deactivateAdmin(
        const std::string& superAdminId,
        const std::string& targetAdminId
    );
    
    /**
     * Reactivate admin
     * Requires: super_admin permission
     */
    bool reactivateAdmin(
        const std::string& superAdminId,
        const std::string& targetAdminId
    );
    
    /**
     * Remove admin
     * Requires: super_admin permission
     */
    bool removeAdmin(
        const std::string& superAdminId,
        const std::string& targetAdminId
    );
    
    /**
     * List all admins
     * Requires: super_admin permission
     */
    std::vector<AdminUser> listAllAdmins(const std::string& superAdminId);
    
    // ===== APPLICATION SUBMISSION =====
    
    /**
     * Submit market maker application
     * Called by prospective market maker
     */
    std::string submitApplication(
        const std::string& applicantAddress,
        const std::string& companyName,
        const std::string& licenseNumber,
        const std::string& regulatoryBody,
        const std::string& country,
        const std::string& contactEmail,
        const std::string& contactPhone,
        const std::string& website,
        const std::string& licenseDocumentHash,
        const std::string& financialStatementsHash,
        const std::string& technicalCapabilitiesHash,
        const std::string& kycDocumentsHash
    );
    
    /**
     * Get application status
     * Public - applicant can check their application
     */
    json getApplicationStatus(const std::string& applicationId);
    
    /**
     * Update application documents
     * Applicant can update documents if requested
     */
    bool updateApplicationDocuments(
        const std::string& applicationId,
        const std::string& applicantAddress,
        const std::unordered_map<std::string, std::string>& documentHashes
    );
    
    // ===== ADMIN REVIEW APIS =====
    
    /**
     * Get all pending applications
     * Requires: verifier or reviewer permission
     */
    std::vector<MarketMakerApplication> getPendingApplications(const std::string& adminId);
    
    /**
     * Get application details
     * Requires: verifier or reviewer permission
     */
    MarketMakerApplication getApplicationDetails(
        const std::string& adminId,
        const std::string& applicationId
    );
    
    /**
     * Assign application to admin for review
     * Requires: reviewer permission
     */
    bool assignApplicationToReviewer(
        const std::string& adminId,
        const std::string& applicationId,
        const std::string& reviewerAdminId
    );
    
    // ===== VERIFICATION STEPS =====
    
    /**
     * Step 1: Verify license with regulatory body
     * Requires: verifier permission
     */
    bool verifyLicense(
        const std::string& adminId,
        const std::string& applicationId,
        bool passed,
        const std::string& notes,
        const std::string& proofDocumentHash
    );
    
    /**
     * Step 2: Check company reputation
     * Requires: verifier permission
     */
    bool checkCompanyReputation(
        const std::string& adminId,
        const std::string& applicationId,
        bool passed,
        const std::string& notes,
        const std::string& proofDocumentHash
    );
    
    /**
     * Step 3: Review financial standing
     * Requires: verifier permission
     */
    bool reviewFinancialStanding(
        const std::string& adminId,
        const std::string& applicationId,
        bool passed,
        const std::string& notes,
        const std::string& proofDocumentHash
    );
    
    /**
     * Step 4: Verify technical capabilities
     * Requires: verifier permission
     */
    bool verifyTechnicalCapabilities(
        const std::string& adminId,
        const std::string& applicationId,
        bool passed,
        const std::string& notes,
        const std::string& proofDocumentHash
    );
    
    /**
     * Step 5: Complete KYC/AML compliance check
     * Requires: verifier permission
     */
    bool completeKycAmlCheck(
        const std::string& adminId,
        const std::string& applicationId,
        bool passed,
        const std::string& notes,
        const std::string& proofDocumentHash
    );
    
    // ===== APPROVAL/REJECTION =====
    
    /**
     * Approve market maker application
     * Requires: super_admin permission
     * This creates the actual market maker in the registry
     */
    bool approveApplication(
        const std::string& adminId,
        const std::string& applicationId,
        const std::string& notes
    );
    
    /**
     * Reject market maker application
     * Requires: super_admin permission
     */
    bool rejectApplication(
        const std::string& adminId,
        const std::string& applicationId,
        const std::string& reason
    );
    
    /**
     * Request additional information
     * Requires: verifier permission
     */
    bool requestAdditionalInfo(
        const std::string& adminId,
        const std::string& applicationId,
        const std::string& requestedInfo
    );
    
    // ===== REVOCATION =====
    
    /**
     * Revoke market maker authorization
     * Requires: super_admin permission
     */
    bool revokeMarketMaker(
        const std::string& adminId,
        const std::string& marketMakerId,
        const std::string& reason
    );
    
    // ===== REPORTING =====
    
    /**
     * Get all applications (with filters)
     */
    std::vector<MarketMakerApplication> getAllApplications(
        const std::string& adminId,
        ApplicationStatus statusFilter = ApplicationStatus::PENDING
    );
    
    /**
     * Get verification statistics
     */
    json getVerificationStatistics(const std::string& adminId);
    
    /**
     * Get audit log
     * Requires: super_admin permission
     */
    std::vector<AuditLogEntry> getAuditLog(
        const std::string& adminId,
        std::time_t startTime,
        std::time_t endTime
    );
    
    /**
     * Export application data
     * Requires: super_admin permission
     */
    json exportApplicationData(
        const std::string& adminId,
        const std::string& applicationId
    );
    
    // ===== INTEGRATION WITH MARKET MAKER REGISTRY =====
    
    /**
     * Get approved market makers
     */
    std::vector<std::string> getApprovedMarketMakers();
    
    /**
     * Check if market maker is approved
     */
    bool isMarketMakerApproved(const std::string& marketMakerId);
    
    /**
     * Get market maker application history
     */
    std::vector<MarketMakerApplication> getMarketMakerHistory(const std::string& marketMakerId);
    
    // ===== FRAUD DETECTION MANAGEMENT =====
    
    /**
     * Submit fraud report (public - no admin required)
     */
    std::string submitFraudReport(
        const std::string& txHash,
        const std::string& reporterAddress,
        double amount,
        const std::string& email,
        const std::string& description,
        const std::string& evidence
    );
    
    /**
     * Get pending fraud reports
     * Requires: fraud_reviewer permission
     */
    std::vector<FraudReport> getPendingFraudReports(const std::string& sessionToken);
    
    /**
     * Get fraud report details
     * Requires: fraud_reviewer permission
     */
    FraudReport getFraudReportDetails(const std::string& sessionToken, const std::string& reportId);
    
    /**
     * Assign fraud report to reviewer
     * Requires: fraud_admin permission
     */
    bool assignFraudReportToReviewer(
        const std::string& sessionToken,
        const std::string& reportId,
        const std::string& reviewerAdminId
    );
    
    /**
     * Approve fraud report (marks transaction as stolen)
     * Requires: fraud_approver permission
     */
    bool approveFraudReport(
        const std::string& sessionToken,
        const std::string& reportId,
        const std::string& notes
    );
    
    /**
     * Reject fraud report
     * Requires: fraud_approver permission
     */
    bool rejectFraudReport(
        const std::string& sessionToken,
        const std::string& reportId,
        const std::string& reason
    );
    
    /**
     * Get fraud statistics
     * Requires: fraud_reviewer permission
     */
    json getFraudStatistics(const std::string& sessionToken);
    
    /**
     * Get all fraud reports (with filter)
     * Requires: fraud_reviewer permission
     */
    std::vector<FraudReport> getAllFraudReports(
        const std::string& sessionToken,
        const std::string& statusFilter = "ALL"
    );
    
    // ===== ADMIN DASHBOARD =====
    
    /**
     * Get dashboard overview
     * Requires: any admin permission
     */
    json getDashboardOverview(const std::string& sessionToken);
    
    /**
     * Get admin activity summary
     * Requires: super_admin permission
     */
    json getAdminActivitySummary(const std::string& sessionToken);
};

/**
 * Helper functions for status conversion
 */
std::string applicationStatusToString(ApplicationStatus status);
ApplicationStatus stringToApplicationStatus(const std::string& status);
