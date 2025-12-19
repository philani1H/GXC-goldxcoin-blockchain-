#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <ctime>
#include <memory>
#include "json.hpp"

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
    std::string role;            // "super_admin", "verifier", "reviewer"
    std::vector<std::string> permissions;
    bool isActive;
    std::time_t createdAt;
    std::time_t lastLoginAt;
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
    std::vector<AuditLogEntry> auditLog;
    
    // Generate unique IDs
    std::string generateApplicationId();
    std::string generateAdminId();
    std::string generateLogId();
    
    // Verify admin permissions
    bool verifyAdminPermission(const std::string& adminId, const std::string& permission);
    
    // Log action
    void logAction(const std::string& adminId, const std::string& action, 
                   const std::string& applicationId, const std::string& details);
    
public:
    MarketMakerAdmin();
    
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
     * Returns: Admin session token
     */
    std::string adminLogin(const std::string& username, const std::string& password);
    
    /**
     * Verify admin session
     */
    bool verifyAdminSession(const std::string& sessionToken);
    
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
};

/**
 * Helper functions for status conversion
 */
std::string applicationStatusToString(ApplicationStatus status);
ApplicationStatus stringToApplicationStatus(const std::string& status);
