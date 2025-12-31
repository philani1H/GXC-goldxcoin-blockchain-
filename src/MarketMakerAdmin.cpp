#include "../include/MarketMakerAdmin.h"
#include "../include/Logger.h"
#include "../include/FraudDetection.h"
#include <random>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <openssl/sha.h>

// Forward declare hashPassword
std::string hashPassword(const std::string& password);

// Constructor
MarketMakerAdmin::MarketMakerAdmin() {
    // Create first super admin: Philani-GXC.Foundation
    AdminUser superAdmin;
    superAdmin.adminId = "admin_philani_gxc_foundation";
    superAdmin.username = "Philani-GXC.Foundation";
    superAdmin.passwordHash = hashPassword("GXC$ecure2025!Philani#Foundation@Admin");
    superAdmin.role = "super_admin";
    superAdmin.permissions = {"all"};
    superAdmin.isActive = true;
    superAdmin.createdAt = std::time(nullptr);
    superAdmin.lastLoginAt = 0;
    
    admins[superAdmin.adminId] = superAdmin;
    
    std::cout << "========================================" << std::endl;
    std::cout << "GXC Market Maker Admin System Initialized" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "First Admin Created:" << std::endl;
    std::cout << "  Username: Philani-GXC.Foundation" << std::endl;
    std::cout << "  Password: GXC$ecure2025!Philani#Foundation@Admin" << std::endl;
    std::cout << "  Role: super_admin" << std::endl;
    std::cout << "  Admin ID: admin_philani_gxc_foundation" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "IMPORTANT: Save this password securely!" << std::endl;
    std::cout << "You can change it after first login." << std::endl;
    std::cout << "========================================" << std::endl;
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

// Hash password (helper function)
std::string hashPassword(const std::string& password) {
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
json MarketMakerAdmin::adminLogin(const std::string& username, const std::string& password) {
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

// Change password
bool MarketMakerAdmin::changePassword(
    const std::string& adminId,
    const std::string& oldPassword,
    const std::string& newPassword
) {
    auto it = admins.find(adminId);
    if (it == admins.end()) {
        throw std::runtime_error("Admin not found");
    }
    
    // Verify old password
    if (it->second.passwordHash != hashPassword(oldPassword)) {
        throw std::runtime_error("Incorrect old password");
    }
    
    // Update password
    it->second.passwordHash = hashPassword(newPassword);
    
    logAction(adminId, "change_password", "", "Password changed");
    
    return true;
}

// Update admin permissions
bool MarketMakerAdmin::updateAdminPermissions(
    const std::string& superAdminId,
    const std::string& targetAdminId,
    const std::vector<std::string>& newPermissions
) {
    if (!verifyAdminPermission(superAdminId, "update_permissions")) {
        throw std::runtime_error("Permission denied: Only super admin can update permissions");
    }
    
    auto it = admins.find(targetAdminId);
    if (it == admins.end()) {
        throw std::runtime_error("Target admin not found");
    }
    
    it->second.permissions = newPermissions;
    
    logAction(superAdminId, "update_permissions", "", 
              "Updated permissions for " + it->second.username);
    
    return true;
}

// Deactivate admin
bool MarketMakerAdmin::deactivateAdmin(
    const std::string& superAdminId,
    const std::string& targetAdminId
) {
    if (!verifyAdminPermission(superAdminId, "deactivate_admin")) {
        throw std::runtime_error("Permission denied: Only super admin can deactivate admins");
    }
    
    auto it = admins.find(targetAdminId);
    if (it == admins.end()) {
        throw std::runtime_error("Target admin not found");
    }
    
    if (targetAdminId == superAdminId) {
        throw std::runtime_error("Cannot deactivate yourself");
    }
    
    it->second.isActive = false;
    
    logAction(superAdminId, "deactivate_admin", "", 
              "Deactivated admin: " + it->second.username);
    
    return true;
}

// Reactivate admin
bool MarketMakerAdmin::reactivateAdmin(
    const std::string& superAdminId,
    const std::string& targetAdminId
) {
    if (!verifyAdminPermission(superAdminId, "reactivate_admin")) {
        throw std::runtime_error("Permission denied: Only super admin can reactivate admins");
    }
    
    auto it = admins.find(targetAdminId);
    if (it == admins.end()) {
        throw std::runtime_error("Target admin not found");
    }
    
    it->second.isActive = true;
    
    logAction(superAdminId, "reactivate_admin", "", 
              "Reactivated admin: " + it->second.username);
    
    return true;
}

// Remove admin
bool MarketMakerAdmin::removeAdmin(
    const std::string& superAdminId,
    const std::string& targetAdminId
) {
    if (!verifyAdminPermission(superAdminId, "remove_admin")) {
        throw std::runtime_error("Permission denied: Only super admin can remove admins");
    }
    
    auto it = admins.find(targetAdminId);
    if (it == admins.end()) {
        throw std::runtime_error("Target admin not found");
    }
    
    if (targetAdminId == superAdminId) {
        throw std::runtime_error("Cannot remove yourself");
    }
    
    std::string username = it->second.username;
    admins.erase(it);
    
    logAction(superAdminId, "remove_admin", "", "Removed admin: " + username);
    
    return true;
}

// List all admins
std::vector<AdminUser> MarketMakerAdmin::listAllAdmins(const std::string& superAdminId) {
    if (!verifyAdminPermission(superAdminId, "list_admins")) {
        throw std::runtime_error("Permission denied: Only super admin can list admins");
    }
    
    std::vector<AdminUser> adminList;
    for (const auto& [id, admin] : admins) {
        adminList.push_back(admin);
    }
    
    return adminList;
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

// ============================================================================
// FRAUD DETECTION MANAGEMENT IMPLEMENTATION
// ============================================================================

std::string MarketMakerAdmin::generateReportId() {
    static uint32_t counter = 1;
    return "FRAUD_REPORT_" + std::to_string(counter++);
}

std::string MarketMakerAdmin::generateSessionToken() {
    // Generate random session token
    std::string token = "SESSION_";
    for (int i = 0; i < 32; i++) {
        token += "0123456789abcdef"[rand() % 16];
    }
    return token;
}

bool MarketMakerAdmin::verifySessionToken(const std::string& sessionToken, std::string& adminId) {
    auto it = sessionTokens.find(sessionToken);
    if (it == sessionTokens.end()) {
        return false;
    }
    
    adminId = it->second;
    
    // Check if session expired
    auto adminIt = admins.find(adminId);
    if (adminIt == admins.end()) {
        sessionTokens.erase(it);
        return false;
    }
    
    if (std::time(nullptr) > adminIt->second.sessionExpiry) {
        sessionTokens.erase(it);
        return false;
    }
    
    return true;
}

// Duplicate adminLogin removed - using the one at line 140

bool MarketMakerAdmin::adminLogout(const std::string& sessionToken) {
    auto it = sessionTokens.find(sessionToken);
    if (it != sessionTokens.end()) {
        std::string adminId = it->second;
        sessionTokens.erase(it);
        
        logAction(adminId, "admin_logout", "", "Admin logged out");
        return true;
    }
    return false;
}

std::string MarketMakerAdmin::verifyAdminSession(const std::string& sessionToken) {
    std::string adminId;
    if (verifySessionToken(sessionToken, adminId)) {
        return adminId;
    }
    return "";
}

AdminUser MarketMakerAdmin::getAdminFromSession(const std::string& sessionToken) {
    std::string adminId = verifyAdminSession(sessionToken);
    if (!adminId.empty()) {
        auto it = admins.find(adminId);
        if (it != admins.end()) {
            return it->second;
        }
    }
    throw std::runtime_error("Invalid session");
}

std::string MarketMakerAdmin::submitFraudReport(
    const std::string& txHash,
    const std::string& reporterAddress,
    double amount,
    const std::string& email,
    const std::string& description,
    const std::string& evidence
) {
    FraudReport report;
    report.reportId = generateReportId();
    report.txHash = txHash;
    report.reporterAddress = reporterAddress;
    report.amount = amount;
    report.email = email;
    report.description = description;
    report.evidence = evidence;
    report.timestamp = std::time(nullptr);
    
    // Admin fact validation fields
    report.factsStatus = "PENDING";
    report.reviewedBy = "";
    report.reviewedAt = 0;
    report.reviewNotes = "";
    report.assignedTo = "";
    
    // Protocol execution fields
    report.executionStatus = "NOT_STARTED";
    report.proofHash = "";
    report.validatedAt = 0;
    report.executedAt = 0;
    report.recoveredAmount = 0;
    report.executionNotes = "";
    
    fraudReports[report.reportId] = report;
    
    // Log submission (no admin ID for public submission)
    logAction("PUBLIC", "fraud_report_submitted", report.reportId, 
              "Fraud report submitted for TX: " + txHash);
    
    return report.reportId;
}

std::vector<FraudReport> MarketMakerAdmin::getPendingFraudReports(const std::string& sessionToken) {
    std::string adminId = verifyAdminSession(sessionToken);
    if (adminId.empty()) {
        throw std::runtime_error("Invalid session");
    }
    
    if (!verifyAdminPermission(adminId, "fraud_reviewer")) {
        throw std::runtime_error("Permission denied: Requires fraud_reviewer permission");
    }
    
    std::vector<FraudReport> pending;
    for (const auto& [id, report] : fraudReports) {
        if (report.factsStatus == "PENDING") {
            pending.push_back(report);
        }
    }
    
    return pending;
}

FraudReport MarketMakerAdmin::getFraudReportDetails(const std::string& sessionToken, const std::string& reportId) {
    std::string adminId = verifyAdminSession(sessionToken);
    if (adminId.empty()) {
        throw std::runtime_error("Invalid session");
    }
    
    if (!verifyAdminPermission(adminId, "fraud_reviewer")) {
        throw std::runtime_error("Permission denied: Requires fraud_reviewer permission");
    }
    
    auto it = fraudReports.find(reportId);
    if (it == fraudReports.end()) {
        throw std::runtime_error("Fraud report not found");
    }
    
    return it->second;
}

bool MarketMakerAdmin::assignFraudReportToReviewer(
    const std::string& sessionToken,
    const std::string& reportId,
    const std::string& reviewerAdminId
) {
    std::string adminId = verifyAdminSession(sessionToken);
    if (adminId.empty()) {
        throw std::runtime_error("Invalid session");
    }
    
    if (!verifyAdminPermission(adminId, "fraud_admin")) {
        throw std::runtime_error("Permission denied: Requires fraud_admin permission");
    }
    
    auto it = fraudReports.find(reportId);
    if (it == fraudReports.end()) {
        throw std::runtime_error("Fraud report not found");
    }
    
    // Verify reviewer exists and has permission
    if (!verifyAdminPermission(reviewerAdminId, "fraud_reviewer")) {
        throw std::runtime_error("Target admin does not have fraud_reviewer permission");
    }
    
    it->second.assignedTo = reviewerAdminId;
    
    logAction(adminId, "fraud_report_assigned", reportId, 
              "Assigned to admin: " + reviewerAdminId);
    
    return true;
}

bool MarketMakerAdmin::approveFraudReport(
    const std::string& sessionToken,
    const std::string& reportId,
    const std::string& notes
) {
    std::string adminId = verifyAdminSession(sessionToken);
    if (adminId.empty()) {
        throw std::runtime_error("Invalid session");
    }
    
    if (!verifyAdminPermission(adminId, "fraud_approver")) {
        throw std::runtime_error("Permission denied: Requires fraud_approver permission");
    }
    
    auto it = fraudReports.find(reportId);
    if (it == fraudReports.end()) {
        throw std::runtime_error("Fraud report not found");
    }
    
    // ADMIN APPROVES FACTS ONLY (not execution)
    // This confirms the fraud claim is legitimate based on evidence
    it->second.factsStatus = "FACTS_APPROVED";
    it->second.reviewedBy = adminId;
    it->second.reviewedAt = std::time(nullptr);
    it->second.reviewNotes = notes;
    it->second.executionStatus = "VALIDATING";
    
    logAction(adminId, "fraud_facts_approved", reportId, 
              "Approved fraud legitimacy for TX: " + it->second.txHash + 
              " - Protocol will now validate feasibility");
    
    LOG_INFO("Admin " + adminId + " approved fraud FACTS for report " + reportId);
    LOG_INFO("Protocol will now independently validate feasibility and execute if possible");
    
    // PROTOCOL VALIDATION AND EXECUTION (independent of admin)
    if (fraudDetection) {
        LOG_INFO("Protocol: Starting independent feasibility validation");
        
        std::string adminSignature = "ADMIN_" + adminId + "_" + std::to_string(std::time(nullptr));
        
        // Generate proof (mathematical analysis)
        ProofOfFeasibility proof = fraudDetection->generateReversalProof(
            it->second.txHash,
            it->second.reporterAddress,
            adminId,
            adminSignature
        );
        
        if (!proof.proof_hash.empty()) {
            it->second.proofHash = proof.proof_hash;
            it->second.validatedAt = std::time(nullptr);
            
            // Protocol validates feasibility independently
            LOG_INFO("Protocol: Proof generated, validating mathematical feasibility");
            
            bool reversalSuccess = fraudDetection->executeReversal(proof);
            
            if (reversalSuccess) {
                // AUTOMATIC EXECUTION (protocol determined it's feasible)
                it->second.executionStatus = "EXECUTED";
                it->second.executedAt = std::time(nullptr);
                it->second.recoveredAmount = proof.recoverable_amount;
                it->second.executionNotes = "Protocol validated feasibility and executed automatically. " +
                                           "Recovered " + std::to_string(proof.recoverable_amount) + " satoshis.";
                
                LOG_INFO("Protocol: Reversal EXECUTED - Recovered " + 
                        std::to_string(proof.recoverable_amount) + " satoshis");
                LOG_INFO("Admin approved FACTS, Protocol validated and executed");
            } else {
                // AUTOMATIC REJECTION (protocol determined it's infeasible)
                it->second.executionStatus = "INFEASIBLE";
                it->second.executionNotes = "Protocol determined reversal is mathematically infeasible. " +
                                           "Possible reasons: insufficient balance, funds already moved, " +
                                           "or taint score below threshold.";
                
                LOG_WARNING("Protocol: Reversal INFEASIBLE - Cannot execute");
                LOG_WARNING("Admin approved FACTS, but Protocol rejected due to mathematical constraints");
            }
        } else {
            // Proof generation failed
            it->second.executionStatus = "INFEASIBLE";
            it->second.executionNotes = "Protocol failed to generate proof. " +
                                       "Transaction may not exist or taint tracking data unavailable.";
            LOG_ERROR("Protocol: Failed to generate reversal proof");
        }
    } else {
        // No fraud detection system available
        it->second.executionStatus = "INFEASIBLE";
        it->second.executionNotes = "Fraud detection system not available.";
        LOG_ERROR("Fraud detection system not initialized");
    }
    
    return true;
}

bool MarketMakerAdmin::rejectFraudReport(
    const std::string& sessionToken,
    const std::string& reportId,
    const std::string& reason
) {
    std::string adminId = verifyAdminSession(sessionToken);
    if (adminId.empty()) {
        throw std::runtime_error("Invalid session");
    }
    
    if (!verifyAdminPermission(adminId, "fraud_approver")) {
        throw std::runtime_error("Permission denied: Requires fraud_approver permission");
    }
    
    auto it = fraudReports.find(reportId);
    if (it == fraudReports.end()) {
        throw std::runtime_error("Fraud report not found");
    }
    
    // Admin rejects FACTS (fraud claim determined to be invalid)
    it->second.factsStatus = "FACTS_REJECTED";
    it->second.reviewedBy = adminId;
    it->second.reviewedAt = std::time(nullptr);
    it->second.reviewNotes = reason;
    it->second.executionStatus = "NOT_STARTED";
    it->second.executionNotes = "Admin determined fraud claim is invalid. No protocol validation performed.";
    
    logAction(adminId, "fraud_facts_rejected", reportId, 
              "Rejected fraud claim as invalid: " + reason);
    
    LOG_INFO("Admin " + adminId + " rejected fraud FACTS for report " + reportId + 
             " - Reason: " + reason);
    
    return true;
}

json MarketMakerAdmin::getFraudStatistics(const std::string& sessionToken) {
    std::string adminId = verifyAdminSession(sessionToken);
    if (adminId.empty()) {
        throw std::runtime_error("Invalid session");
    }
    
    if (!verifyAdminPermission(adminId, "fraud_reviewer")) {
        throw std::runtime_error("Permission denied: Requires fraud_reviewer permission");
    }
    
    uint32_t totalReports = fraudReports.size();
    uint32_t pendingReports = 0;
    uint32_t approvedReports = 0;
    uint32_t rejectedReports = 0;
    double totalAmountReported = 0.0;
    
    for (const auto& [id, report] : fraudReports) {
        if (report.factsStatus == "PENDING") pendingReports++;
        else if (report.factsStatus == "FACTS_APPROVED") approvedReports++;
        else if (report.factsStatus == "FACTS_REJECTED") rejectedReports++;
        
        totalAmountReported += report.amount;
    }
    
    json stats;
    stats["totalReports"] = totalReports;
    stats["pendingReports"] = pendingReports;
    stats["approvedReports"] = approvedReports;
    stats["rejectedReports"] = rejectedReports;
    stats["totalAmountReported"] = totalAmountReported;
    
    return stats;
}

std::vector<FraudReport> MarketMakerAdmin::getAllFraudReports(
    const std::string& sessionToken,
    const std::string& statusFilter
) {
    std::string adminId = verifyAdminSession(sessionToken);
    if (adminId.empty()) {
        throw std::runtime_error("Invalid session");
    }
    
    if (!verifyAdminPermission(adminId, "fraud_reviewer")) {
        throw std::runtime_error("Permission denied: Requires fraud_reviewer permission");
    }
    
    std::vector<FraudReport> reports;
    for (const auto& [id, report] : fraudReports) {
        // Support both old and new status filters for backward compatibility
        bool matchesFilter = (statusFilter == "ALL") ||
                            (report.factsStatus == statusFilter) ||
                            (statusFilter == "PENDING" && report.factsStatus == "PENDING") ||
                            (statusFilter == "APPROVED" && report.factsStatus == "FACTS_APPROVED") ||
                            (statusFilter == "REJECTED" && report.factsStatus == "FACTS_REJECTED");
        
        if (matchesFilter) {
            reports.push_back(report);
        }
    }
    
    return reports;
}

json MarketMakerAdmin::getDashboardOverview(const std::string& sessionToken) {
    std::string adminId = verifyAdminSession(sessionToken);
    if (adminId.empty()) {
        throw std::runtime_error("Invalid session");
    }
    
    json overview;
    
    // Get admin info
    auto adminIt = admins.find(adminId);
    if (adminIt != admins.end()) {
        overview["admin"]["username"] = adminIt->second.username;
        overview["admin"]["role"] = adminIt->second.role;
        overview["admin"]["permissions"] = adminIt->second.permissions;
    }
    
    // Count pending items based on permissions
    if (verifyAdminPermission(adminId, "fraud_reviewer")) {
        uint32_t pendingFraud = 0;
        for (const auto& [id, report] : fraudReports) {
            if (report.factsStatus == "PENDING") pendingFraud++;
        }
        overview["pendingFraudReports"] = pendingFraud;
    }
    
    if (verifyAdminPermission(adminId, "verifier")) {
        uint32_t pendingMM = 0;
        for (const auto& [id, app] : applications) {
            if (app.status == ApplicationStatus::PENDING || 
                app.status == ApplicationStatus::UNDER_REVIEW) {
                pendingMM++;
            }
        }
        overview["pendingMMApplications"] = pendingMM;
    }
    
    // Recent activity
    overview["recentActivity"] = json::array();
    int count = 0;
    for (auto it = auditLog.rbegin(); it != auditLog.rend() && count < 10; ++it, ++count) {
        json activity;
        activity["action"] = it->action;
        activity["timestamp"] = it->timestamp;
        activity["details"] = it->details;
        overview["recentActivity"].push_back(activity);
    }
    
    return overview;
}

json MarketMakerAdmin::getAdminActivitySummary(const std::string& sessionToken) {
    std::string adminId = verifyAdminSession(sessionToken);
    if (adminId.empty()) {
        throw std::runtime_error("Invalid session");
    }
    
    if (!verifyAdminPermission(adminId, "super_admin")) {
        throw std::runtime_error("Permission denied: Requires super_admin permission");
    }
    
    json summary;
    summary["totalAdmins"] = admins.size();
    summary["totalAuditLogs"] = auditLog.size();
    
    // Count active admins
    uint32_t activeAdmins = 0;
    for (const auto& [id, admin] : admins) {
        if (admin.isActive) activeAdmins++;
    }
    summary["activeAdmins"] = activeAdmins;
    
    // Admin activity by role
    std::unordered_map<std::string, uint32_t> roleCount;
    for (const auto& [id, admin] : admins) {
        roleCount[admin.role]++;
    }
    summary["adminsByRole"] = roleCount;
    
    return summary;
}
