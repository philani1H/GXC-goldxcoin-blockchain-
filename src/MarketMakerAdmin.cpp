#include "../include/MarketMakerAdmin.h"
#include <random>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>

// Constructor
MarketMakerAdmin::MarketMakerAdmin() {
    // Create default super admin
    AdminUser superAdmin;
    superAdmin.adminId = "admin_super_001";
    superAdmin.username = "gxc_admin";
    superAdmin.passwordHash = hashPassword("change_this_password");
    superAdmin.role = "super_admin";
    superAdmin.permissions = {"all"};
    superAdmin.isActive = true;
    superAdmin.createdAt = std::time(nullptr);
    superAdmin.lastLoginAt = 0;
    
    admins[superAdmin.adminId] = superAdmin;
}

// Generate unique application ID
std::string MarketMakerAdmin::generateApplicationId() {
    std::stringstream ss;
    ss << "APP_" << std::time(nullptr) << "_" << (rand() % 10000);
    return ss.str();
}

// Generate unique admin ID
std::string MarketMakerAdmin::generateAdminId() {
    std::stringstream ss;
    ss << "ADMIN_" << std::time(nullptr) << "_" << (rand() % 10000);
    return ss.str();
}

// Generate unique log ID
std::string MarketMakerAdmin::generateLogId() {
    std::stringstream ss;
    ss << "LOG_" << std::time(nullptr) << "_" << (rand() % 10000);
    return ss.str();
}

// Hash password
std::string MarketMakerAdmin::hashPassword(const std::string& password) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)password.c_str(), password.length(), hash);
    
    std::stringstream ss;
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}

// Verify admin permission
bool MarketMakerAdmin::verifyAdminPermission(const std::string& adminId, const std::string& permission) {
    auto it = admins.find(adminId);
    if (it == admins.end() || !it->second.isActive) {
        return false;
    }
    
    // Super admin has all permissions
    if (it->second.role == "super_admin") {
        return true;
    }
    
    // Check specific permission
    for (const auto& perm : it->second.permissions) {
        if (perm == permission || perm == "all") {
            return true;
        }
    }
    
    return false;
}

// Log action
void MarketMakerAdmin::logAction(const std::string& adminId, const std::string& action,
                                  const std::string& applicationId, const std::string& details) {
    AuditLogEntry entry;
    entry.logId = generateLogId();
    entry.adminId = adminId;
    entry.action = action;
    entry.applicationId = applicationId;
    entry.details = details;
    entry.timestamp = std::time(nullptr);
    entry.ipAddress = "0.0.0.0"; // TODO: Get real IP
    
    auditLog.push_back(entry);
}

// Create admin user
std::string MarketMakerAdmin::createAdmin(
    const std::string& superAdminId,
    const std::string& username,
    const std::string& password,
    const std::string& role,
    const std::vector<std::string>& permissions
) {
    if (!verifyAdminPermission(superAdminId, "create_admin")) {
        throw std::runtime_error("Permission denied: Only super admin can create admins");
    }
    
    AdminUser newAdmin;
    newAdmin.adminId = generateAdminId();
    newAdmin.username = username;
    newAdmin.passwordHash = hashPassword(password);
    newAdmin.role = role;
    newAdmin.permissions = permissions;
    newAdmin.isActive = true;
    newAdmin.createdAt = std::time(nullptr);
    newAdmin.lastLoginAt = 0;
    
    admins[newAdmin.adminId] = newAdmin;
    
    logAction(superAdminId, "create_admin", "", "Created admin: " + username);
    
    return newAdmin.adminId;
}

// Admin login
std::string MarketMakerAdmin::adminLogin(const std::string& username, const std::string& password) {
    std::string passwordHash = hashPassword(password);
    
    for (auto& [adminId, admin] : admins) {
        if (admin.username == username && admin.passwordHash == passwordHash && admin.isActive) {
            admin.lastLoginAt = std::time(nullptr);
            
            // Generate session token
            std::stringstream ss;
            ss << "SESSION_" << adminId << "_" << std::time(nullptr);
            
            logAction(adminId, "login", "", "Admin logged in");
            
            return ss.str();
        }
    }
    
    throw std::runtime_error("Invalid username or password");
}

// Submit application
std::string MarketMakerAdmin::submitApplication(
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
) {
    MarketMakerApplication app;
    app.applicationId = generateApplicationId();
    app.applicantAddress = applicantAddress;
    app.companyName = companyName;
    app.licenseNumber = licenseNumber;
    app.regulatoryBody = regulatoryBody;
    app.country = country;
    app.contactEmail = contactEmail;
    app.contactPhone = contactPhone;
    app.website = website;
    app.licenseDocumentHash = licenseDocumentHash;
    app.financialStatementsHash = financialStatementsHash;
    app.technicalCapabilitiesHash = technicalCapabilitiesHash;
    app.kycDocumentsHash = kycDocumentsHash;
    app.status = ApplicationStatus::PENDING;
    app.submittedAt = std::time(nullptr);
    app.lastUpdatedAt = std::time(nullptr);
    
    applications[app.applicationId] = app;
    
    return app.applicationId;
}

// Get application status
json MarketMakerAdmin::getApplicationStatus(const std::string& applicationId) {
    auto it = applications.find(applicationId);
    if (it == applications.end()) {
        throw std::runtime_error("Application not found");
    }
    
    json result;
    result["applicationId"] = it->second.applicationId;
    result["companyName"] = it->second.companyName;
    result["status"] = applicationStatusToString(it->second.status);
    result["submittedAt"] = it->second.submittedAt;
    result["lastUpdatedAt"] = it->second.lastUpdatedAt;
    
    return result;
}

// Get pending applications
std::vector<MarketMakerApplication> MarketMakerAdmin::getPendingApplications(const std::string& adminId) {
    if (!verifyAdminPermission(adminId, "view_applications")) {
        throw std::runtime_error("Permission denied");
    }
    
    std::vector<MarketMakerApplication> pending;
    for (const auto& [id, app] : applications) {
        if (app.status == ApplicationStatus::PENDING || 
            app.status == ApplicationStatus::UNDER_REVIEW) {
            pending.push_back(app);
        }
    }
    
    return pending;
}

// Verify license
bool MarketMakerAdmin::verifyLicense(
    const std::string& adminId,
    const std::string& applicationId,
    bool passed,
    const std::string& notes,
    const std::string& proofDocumentHash
) {
    if (!verifyAdminPermission(adminId, "verify_license")) {
        throw std::runtime_error("Permission denied");
    }
    
    auto it = applications.find(applicationId);
    if (it == applications.end()) {
        throw std::runtime_error("Application not found");
    }
    
    VerificationStep step;
    step.stepName = "License Verification";
    step.status = ApplicationStatus::LICENSE_VERIFIED;
    step.verifiedBy = adminId;
    step.verifiedAt = std::time(nullptr);
    step.notes = notes;
    step.proofDocument = proofDocumentHash;
    step.passed = passed;
    
    it->second.verificationSteps.push_back(step);
    it->second.lastUpdatedAt = std::time(nullptr);
    
    if (passed) {
        it->second.status = ApplicationStatus::LICENSE_VERIFIED;
    } else {
        it->second.status = ApplicationStatus::REJECTED;
    }
    
    logAction(adminId, "verify_license", applicationId, 
              "License verification: " + std::string(passed ? "PASSED" : "FAILED"));
    
    return true;
}

// Check company reputation
bool MarketMakerAdmin::checkCompanyReputation(
    const std::string& adminId,
    const std::string& applicationId,
    bool passed,
    const std::string& notes,
    const std::string& proofDocumentHash
) {
    if (!verifyAdminPermission(adminId, "check_reputation")) {
        throw std::runtime_error("Permission denied");
    }
    
    auto it = applications.find(applicationId);
    if (it == applications.end()) {
        throw std::runtime_error("Application not found");
    }
    
    VerificationStep step;
    step.stepName = "Company Reputation Check";
    step.status = ApplicationStatus::REPUTATION_CHECKED;
    step.verifiedBy = adminId;
    step.verifiedAt = std::time(nullptr);
    step.notes = notes;
    step.proofDocument = proofDocumentHash;
    step.passed = passed;
    
    it->second.verificationSteps.push_back(step);
    it->second.lastUpdatedAt = std::time(nullptr);
    
    if (passed) {
        it->second.status = ApplicationStatus::REPUTATION_CHECKED;
    } else {
        it->second.status = ApplicationStatus::REJECTED;
    }
    
    logAction(adminId, "check_reputation", applicationId,
              "Reputation check: " + std::string(passed ? "PASSED" : "FAILED"));
    
    return true;
}

// Review financial standing
bool MarketMakerAdmin::reviewFinancialStanding(
    const std::string& adminId,
    const std::string& applicationId,
    bool passed,
    const std::string& notes,
    const std::string& proofDocumentHash
) {
    if (!verifyAdminPermission(adminId, "review_financial")) {
        throw std::runtime_error("Permission denied");
    }
    
    auto it = applications.find(applicationId);
    if (it == applications.end()) {
        throw std::runtime_error("Application not found");
    }
    
    VerificationStep step;
    step.stepName = "Financial Standing Review";
    step.status = ApplicationStatus::FINANCIAL_REVIEWED;
    step.verifiedBy = adminId;
    step.verifiedAt = std::time(nullptr);
    step.notes = notes;
    step.proofDocument = proofDocumentHash;
    step.passed = passed;
    
    it->second.verificationSteps.push_back(step);
    it->second.lastUpdatedAt = std::time(nullptr);
    
    if (passed) {
        it->second.status = ApplicationStatus::FINANCIAL_REVIEWED;
    } else {
        it->second.status = ApplicationStatus::REJECTED;
    }
    
    logAction(adminId, "review_financial", applicationId,
              "Financial review: " + std::string(passed ? "PASSED" : "FAILED"));
    
    return true;
}

// Verify technical capabilities
bool MarketMakerAdmin::verifyTechnicalCapabilities(
    const std::string& adminId,
    const std::string& applicationId,
    bool passed,
    const std::string& notes,
    const std::string& proofDocumentHash
) {
    if (!verifyAdminPermission(adminId, "verify_technical")) {
        throw std::runtime_error("Permission denied");
    }
    
    auto it = applications.find(applicationId);
    if (it == applications.end()) {
        throw std::runtime_error("Application not found");
    }
    
    VerificationStep step;
    step.stepName = "Technical Capabilities Verification";
    step.status = ApplicationStatus::TECHNICAL_VERIFIED;
    step.verifiedBy = adminId;
    step.verifiedAt = std::time(nullptr);
    step.notes = notes;
    step.proofDocument = proofDocumentHash;
    step.passed = passed;
    
    it->second.verificationSteps.push_back(step);
    it->second.lastUpdatedAt = std::time(nullptr);
    
    if (passed) {
        it->second.status = ApplicationStatus::TECHNICAL_VERIFIED;
    } else {
        it->second.status = ApplicationStatus::REJECTED;
    }
    
    logAction(adminId, "verify_technical", applicationId,
              "Technical verification: " + std::string(passed ? "PASSED" : "FAILED"));
    
    return true;
}

// Complete KYC/AML check
bool MarketMakerAdmin::completeKycAmlCheck(
    const std::string& adminId,
    const std::string& applicationId,
    bool passed,
    const std::string& notes,
    const std::string& proofDocumentHash
) {
    if (!verifyAdminPermission(adminId, "complete_kyc_aml")) {
        throw std::runtime_error("Permission denied");
    }
    
    auto it = applications.find(applicationId);
    if (it == applications.end()) {
        throw std::runtime_error("Application not found");
    }
    
    VerificationStep step;
    step.stepName = "KYC/AML Compliance Check";
    step.status = ApplicationStatus::KYC_AML_COMPLETED;
    step.verifiedBy = adminId;
    step.verifiedAt = std::time(nullptr);
    step.notes = notes;
    step.proofDocument = proofDocumentHash;
    step.passed = passed;
    
    it->second.verificationSteps.push_back(step);
    it->second.lastUpdatedAt = std::time(nullptr);
    
    if (passed) {
        it->second.status = ApplicationStatus::KYC_AML_COMPLETED;
    } else {
        it->second.status = ApplicationStatus::REJECTED;
    }
    
    logAction(adminId, "complete_kyc_aml", applicationId,
              "KYC/AML check: " + std::string(passed ? "PASSED" : "FAILED"));
    
    return true;
}

// Approve application
bool MarketMakerAdmin::approveApplication(
    const std::string& adminId,
    const std::string& applicationId,
    const std::string& notes
) {
    if (!verifyAdminPermission(adminId, "approve_application")) {
        throw std::runtime_error("Permission denied: Only super admin can approve");
    }
    
    auto it = applications.find(applicationId);
    if (it == applications.end()) {
        throw std::runtime_error("Application not found");
    }
    
    // Check all verification steps passed
    bool allPassed = true;
    for (const auto& step : it->second.verificationSteps) {
        if (!step.passed) {
            allPassed = false;
            break;
        }
    }
    
    if (!allPassed) {
        throw std::runtime_error("Cannot approve: Not all verification steps passed");
    }
    
    it->second.status = ApplicationStatus::APPROVED;
    it->second.approvedBy = adminId;
    it->second.decidedAt = std::time(nullptr);
    it->second.decisionNotes = notes;
    it->second.marketMakerId = "MM_" + it->second.companyName;
    it->second.activatedAt = std::time(nullptr);
    
    logAction(adminId, "approve_application", applicationId, "Application approved: " + notes);
    
    return true;
}

// Reject application
bool MarketMakerAdmin::rejectApplication(
    const std::string& adminId,
    const std::string& applicationId,
    const std::string& reason
) {
    if (!verifyAdminPermission(adminId, "reject_application")) {
        throw std::runtime_error("Permission denied: Only super admin can reject");
    }
    
    auto it = applications.find(applicationId);
    if (it == applications.end()) {
        throw std::runtime_error("Application not found");
    }
    
    it->second.status = ApplicationStatus::REJECTED;
    it->second.approvedBy = adminId;
    it->second.decidedAt = std::time(nullptr);
    it->second.decisionNotes = reason;
    
    logAction(adminId, "reject_application", applicationId, "Application rejected: " + reason);
    
    return true;
}

// Get verification statistics
json MarketMakerAdmin::getVerificationStatistics(const std::string& adminId) {
    if (!verifyAdminPermission(adminId, "view_statistics")) {
        throw std::runtime_error("Permission denied");
    }
    
    int pending = 0, underReview = 0, approved = 0, rejected = 0;
    
    for (const auto& [id, app] : applications) {
        switch (app.status) {
            case ApplicationStatus::PENDING:
                pending++;
                break;
            case ApplicationStatus::UNDER_REVIEW:
                underReview++;
                break;
            case ApplicationStatus::APPROVED:
                approved++;
                break;
            case ApplicationStatus::REJECTED:
                rejected++;
                break;
            default:
                break;
        }
    }
    
    json stats;
    stats["total"] = applications.size();
    stats["pending"] = pending;
    stats["underReview"] = underReview;
    stats["approved"] = approved;
    stats["rejected"] = rejected;
    
    return stats;
}

// Helper: Convert status to string
std::string applicationStatusToString(ApplicationStatus status) {
    switch (status) {
        case ApplicationStatus::PENDING: return "PENDING";
        case ApplicationStatus::UNDER_REVIEW: return "UNDER_REVIEW";
        case ApplicationStatus::LICENSE_VERIFIED: return "LICENSE_VERIFIED";
        case ApplicationStatus::REPUTATION_CHECKED: return "REPUTATION_CHECKED";
        case ApplicationStatus::FINANCIAL_REVIEWED: return "FINANCIAL_REVIEWED";
        case ApplicationStatus::TECHNICAL_VERIFIED: return "TECHNICAL_VERIFIED";
        case ApplicationStatus::KYC_AML_COMPLETED: return "KYC_AML_COMPLETED";
        case ApplicationStatus::APPROVED: return "APPROVED";
        case ApplicationStatus::REJECTED: return "REJECTED";
        case ApplicationStatus::REVOKED: return "REVOKED";
        default: return "UNKNOWN";
    }
}
