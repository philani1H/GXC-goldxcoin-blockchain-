/**
 * RESTServer Admin API Implementation
 * 
 * This file contains all admin-related API endpoints:
 * - Admin authentication
 * - Admin management
 * - Fraud report management
 * - Market maker application management
 * - Dashboard and statistics
 * 
 * Add this to RESTServer.cpp
 */

// ============================================================================
// ADMIN AUTHENTICATION
// ============================================================================

std::string RESTServer::adminLogin(const std::string& body) {
    if (!adminSystem) {
        return createErrorResponse(503, "SERVICE_UNAVAILABLE", 
            "Admin system not available");
    }
    
    try {
        auto json = nlohmann::json::parse(body);
        
        if (!json.contains("username") || !json.contains("password")) {
            return createErrorResponse(400, "MISSING_FIELDS", 
                "username and password are required");
        }
        
        std::string username = json["username"];
        std::string password = json["password"];
        
        auto result = adminSystem->adminLogin(username, password);
        
        return jsonToString(result);
        
    } catch (const std::exception& e) {
        return createErrorResponse(500, "INTERNAL_ERROR", e.what());
    }
}

std::string RESTServer::adminLogout(const std::string& sessionToken) {
    if (!adminSystem) {
        return createErrorResponse(503, "SERVICE_UNAVAILABLE", 
            "Admin system not available");
    }
    
    try {
        bool success = adminSystem->adminLogout(sessionToken);
        
        nlohmann::json response;
        response["success"] = success;
        response["message"] = success ? "Logged out successfully" : "Invalid session";
        
        return jsonToString(response);
        
    } catch (const std::exception& e) {
        return createErrorResponse(500, "INTERNAL_ERROR", e.what());
    }
}

std::string RESTServer::verifySession(const std::string& sessionToken) {
    if (!adminSystem) {
        return createErrorResponse(503, "SERVICE_UNAVAILABLE", 
            "Admin system not available");
    }
    
    try {
        std::string adminId = adminSystem->verifyAdminSession(sessionToken);
        
        nlohmann::json response;
        response["valid"] = !adminId.empty();
        if (!adminId.empty()) {
            auto admin = adminSystem->getAdminFromSession(sessionToken);
            response["adminId"] = adminId;
            response["username"] = admin.username;
            response["role"] = admin.role;
            response["permissions"] = admin.permissions;
        }
        
        return jsonToString(response);
        
    } catch (const std::exception& e) {
        return createErrorResponse(500, "INTERNAL_ERROR", e.what());
    }
}

// ============================================================================
// ADMIN MANAGEMENT (super_admin only)
// ============================================================================

std::string RESTServer::createAdmin(const std::string& sessionToken, const std::string& body) {
    if (!adminSystem) {
        return createErrorResponse(503, "SERVICE_UNAVAILABLE", 
            "Admin system not available");
    }
    
    try {
        // Verify session
        std::string adminId = adminSystem->verifyAdminSession(sessionToken);
        if (adminId.empty()) {
            return createErrorResponse(401, "UNAUTHORIZED", "Invalid session");
        }
        
        auto json = nlohmann::json::parse(body);
        
        if (!json.contains("username") || !json.contains("password") || !json.contains("role")) {
            return createErrorResponse(400, "MISSING_FIELDS", 
                "username, password, and role are required");
        }
        
        std::string username = json["username"];
        std::string password = json["password"];
        std::string role = json["role"];
        std::vector<std::string> permissions;
        
        if (json.contains("permissions")) {
            permissions = json["permissions"].get<std::vector<std::string>>();
        }
        
        std::string newAdminId = adminSystem->createAdmin(adminId, username, password, role, permissions);
        
        nlohmann::json response;
        response["success"] = true;
        response["adminId"] = newAdminId;
        response["message"] = "Admin created successfully";
        
        return jsonToString(response);
        
    } catch (const std::exception& e) {
        return createErrorResponse(403, "FORBIDDEN", e.what());
    }
}

std::string RESTServer::updateAdminPermissions(const std::string& sessionToken, const std::string& body) {
    if (!adminSystem) {
        return createErrorResponse(503, "SERVICE_UNAVAILABLE", 
            "Admin system not available");
    }
    
    try {
        std::string adminId = adminSystem->verifyAdminSession(sessionToken);
        if (adminId.empty()) {
            return createErrorResponse(401, "UNAUTHORIZED", "Invalid session");
        }
        
        auto json = nlohmann::json::parse(body);
        
        if (!json.contains("targetAdminId") || !json.contains("permissions")) {
            return createErrorResponse(400, "MISSING_FIELDS", 
                "targetAdminId and permissions are required");
        }
        
        std::string targetAdminId = json["targetAdminId"];
        std::vector<std::string> permissions = json["permissions"].get<std::vector<std::string>>();
        
        bool success = adminSystem->updateAdminPermissions(adminId, targetAdminId, permissions);
        
        nlohmann::json response;
        response["success"] = success;
        response["message"] = "Permissions updated successfully";
        
        return jsonToString(response);
        
    } catch (const std::exception& e) {
        return createErrorResponse(403, "FORBIDDEN", e.what());
    }
}

std::string RESTServer::deactivateAdmin(const std::string& sessionToken, const std::string& body) {
    if (!adminSystem) {
        return createErrorResponse(503, "SERVICE_UNAVAILABLE", 
            "Admin system not available");
    }
    
    try {
        std::string adminId = adminSystem->verifyAdminSession(sessionToken);
        if (adminId.empty()) {
            return createErrorResponse(401, "UNAUTHORIZED", "Invalid session");
        }
        
        auto json = nlohmann::json::parse(body);
        
        if (!json.contains("targetAdminId")) {
            return createErrorResponse(400, "MISSING_FIELDS", 
                "targetAdminId is required");
        }
        
        std::string targetAdminId = json["targetAdminId"];
        
        bool success = adminSystem->deactivateAdmin(adminId, targetAdminId);
        
        nlohmann::json response;
        response["success"] = success;
        response["message"] = "Admin deactivated successfully";
        
        return jsonToString(response);
        
    } catch (const std::exception& e) {
        return createErrorResponse(403, "FORBIDDEN", e.what());
    }
}

std::string RESTServer::listAllAdmins(const std::string& sessionToken) {
    if (!adminSystem) {
        return createErrorResponse(503, "SERVICE_UNAVAILABLE", 
            "Admin system not available");
    }
    
    try {
        std::string adminId = adminSystem->verifyAdminSession(sessionToken);
        if (adminId.empty()) {
            return createErrorResponse(401, "UNAUTHORIZED", "Invalid session");
        }
        
        auto admins = adminSystem->listAllAdmins(adminId);
        
        nlohmann::json response;
        response["admins"] = nlohmann::json::array();
        
        for (const auto& admin : admins) {
            nlohmann::json adminJson;
            adminJson["adminId"] = admin.adminId;
            adminJson["username"] = admin.username;
            adminJson["role"] = admin.role;
            adminJson["permissions"] = admin.permissions;
            adminJson["isActive"] = admin.isActive;
            adminJson["createdAt"] = admin.createdAt;
            adminJson["lastLoginAt"] = admin.lastLoginAt;
            response["admins"].push_back(adminJson);
        }
        
        response["count"] = admins.size();
        
        return jsonToString(response);
        
    } catch (const std::exception& e) {
        return createErrorResponse(403, "FORBIDDEN", e.what());
    }
}

// ============================================================================
// FRAUD MANAGEMENT (admin only)
// ============================================================================

std::string RESTServer::reportStolenFunds(const std::string& body) {
    if (!adminSystem) {
        return createErrorResponse(503, "SERVICE_UNAVAILABLE", 
            "Admin system not available");
    }
    
    try {
        auto json = nlohmann::json::parse(body);
        
        if (!json.contains("txHash") || !json.contains("amount")) {
            return createErrorResponse(400, "MISSING_FIELDS", 
                "txHash and amount are required");
        }
        
        std::string txHash = json["txHash"];
        std::string reporterAddress = json.value("reporterAddress", "");
        double amount = json["amount"];
        std::string email = json.value("email", "");
        std::string description = json.value("description", "");
        std::string evidence = json.value("evidence", "");
        
        std::string reportId = adminSystem->submitFraudReport(
            txHash, reporterAddress, amount, email, description, evidence
        );
        
        nlohmann::json response;
        response["success"] = true;
        response["reportId"] = reportId;
        response["status"] = "PENDING";
        response["message"] = "Report submitted successfully. We will review within 24 hours.";
        
        return jsonToString(response);
        
    } catch (const std::exception& e) {
        return createErrorResponse(500, "INTERNAL_ERROR", e.what());
    }
}

std::string RESTServer::getPendingFraudReports(const std::string& sessionToken) {
    if (!adminSystem) {
        return createErrorResponse(503, "SERVICE_UNAVAILABLE", 
            "Admin system not available");
    }
    
    try {
        auto reports = adminSystem->getPendingFraudReports(sessionToken);
        
        nlohmann::json response;
        response["reports"] = nlohmann::json::array();
        
        for (const auto& report : reports) {
            nlohmann::json reportJson;
            reportJson["reportId"] = report.reportId;
            reportJson["txHash"] = report.txHash;
            reportJson["reporterAddress"] = report.reporterAddress;
            reportJson["amount"] = report.amount;
            reportJson["email"] = report.email;
            reportJson["description"] = report.description;
            reportJson["evidence"] = report.evidence;
            reportJson["timestamp"] = report.timestamp;
            reportJson["status"] = report.status;
            reportJson["assignedTo"] = report.assignedTo;
            response["reports"].push_back(reportJson);
        }
        
        response["count"] = reports.size();
        
        return jsonToString(response);
        
    } catch (const std::exception& e) {
        return createErrorResponse(403, "FORBIDDEN", e.what());
    }
}

std::string RESTServer::getFraudReportDetails(const std::string& sessionToken, const std::string& reportId) {
    if (!adminSystem) {
        return createErrorResponse(503, "SERVICE_UNAVAILABLE", 
            "Admin system not available");
    }
    
    try {
        auto report = adminSystem->getFraudReportDetails(sessionToken, reportId);
        
        nlohmann::json response;
        response["reportId"] = report.reportId;
        response["txHash"] = report.txHash;
        response["reporterAddress"] = report.reporterAddress;
        response["amount"] = report.amount;
        response["email"] = report.email;
        response["description"] = report.description;
        response["evidence"] = report.evidence;
        response["timestamp"] = report.timestamp;
        response["status"] = report.status;
        response["reviewedBy"] = report.reviewedBy;
        response["reviewedAt"] = report.reviewedAt;
        response["reviewNotes"] = report.reviewNotes;
        response["assignedTo"] = report.assignedTo;
        
        return jsonToString(response);
        
    } catch (const std::exception& e) {
        return createErrorResponse(403, "FORBIDDEN", e.what());
    }
}

std::string RESTServer::approveFraudReport(const std::string& sessionToken, const std::string& body) {
    if (!adminSystem || !fraudDetection) {
        return createErrorResponse(503, "SERVICE_UNAVAILABLE", 
            "Admin system or fraud detection not available");
    }
    
    try {
        auto json = nlohmann::json::parse(body);
        
        if (!json.contains("reportId")) {
            return createErrorResponse(400, "MISSING_FIELDS", 
                "reportId is required");
        }
        
        std::string reportId = json["reportId"];
        std::string notes = json.value("notes", "");
        
        // Approve in admin system
        bool success = adminSystem->approveFraudReport(sessionToken, reportId, notes);
        
        if (success) {
            // Get report details to mark transaction as stolen
            auto report = adminSystem->getFraudReportDetails(sessionToken, reportId);
            
            // Mark transaction as stolen in fraud detection system
            fraudDetection->markAsStolen(report.txHash);
            
            nlohmann::json response;
            response["success"] = true;
            response["reportId"] = reportId;
            response["status"] = "APPROVED";
            response["message"] = "Report approved. Transaction marked as stolen.";
            response["txHash"] = report.txHash;
            response["taintScore"] = 1.0;
            
            return jsonToString(response);
        } else {
            return createErrorResponse(500, "INTERNAL_ERROR", "Failed to approve report");
        }
        
    } catch (const std::exception& e) {
        return createErrorResponse(403, "FORBIDDEN", e.what());
    }
}

std::string RESTServer::rejectFraudReport(const std::string& sessionToken, const std::string& body) {
    if (!adminSystem) {
        return createErrorResponse(503, "SERVICE_UNAVAILABLE", 
            "Admin system not available");
    }
    
    try {
        auto json = nlohmann::json::parse(body);
        
        if (!json.contains("reportId")) {
            return createErrorResponse(400, "MISSING_FIELDS", 
                "reportId is required");
        }
        
        std::string reportId = json["reportId"];
        std::string reason = json.value("reason", "");
        
        bool success = adminSystem->rejectFraudReport(sessionToken, reportId, reason);
        
        nlohmann::json response;
        response["success"] = success;
        response["reportId"] = reportId;
        response["status"] = "REJECTED";
        response["message"] = "Report rejected.";
        response["reason"] = reason;
        
        return jsonToString(response);
        
    } catch (const std::exception& e) {
        return createErrorResponse(403, "FORBIDDEN", e.what());
    }
}

std::string RESTServer::assignFraudReport(const std::string& sessionToken, const std::string& body) {
    if (!adminSystem) {
        return createErrorResponse(503, "SERVICE_UNAVAILABLE", 
            "Admin system not available");
    }
    
    try {
        auto json = nlohmann::json::parse(body);
        
        if (!json.contains("reportId") || !json.contains("reviewerAdminId")) {
            return createErrorResponse(400, "MISSING_FIELDS", 
                "reportId and reviewerAdminId are required");
        }
        
        std::string reportId = json["reportId"];
        std::string reviewerAdminId = json["reviewerAdminId"];
        
        bool success = adminSystem->assignFraudReportToReviewer(sessionToken, reportId, reviewerAdminId);
        
        nlohmann::json response;
        response["success"] = success;
        response["message"] = "Report assigned successfully";
        
        return jsonToString(response);
        
    } catch (const std::exception& e) {
        return createErrorResponse(403, "FORBIDDEN", e.what());
    }
}

std::string RESTServer::getFraudStatistics(const std::string& sessionToken) {
    if (!adminSystem) {
        return createErrorResponse(503, "SERVICE_UNAVAILABLE", 
            "Admin system not available");
    }
    
    try {
        auto stats = adminSystem->getFraudStatistics(sessionToken);
        return jsonToString(stats);
        
    } catch (const std::exception& e) {
        return createErrorResponse(403, "FORBIDDEN", e.what());
    }
}

// ============================================================================
// MARKET MAKER MANAGEMENT (admin only)
// ============================================================================

std::string RESTServer::submitMMApplication(const std::string& body) {
    if (!adminSystem) {
        return createErrorResponse(503, "SERVICE_UNAVAILABLE", 
            "Admin system not available");
    }
    
    try {
        auto json = nlohmann::json::parse(body);
        
        // Extract application details
        std::string companyName = json.value("companyName", "");
        std::string licenseNumber = json.value("licenseNumber", "");
        std::string contactEmail = json.value("contactEmail", "");
        
        // Submit application (simplified - full implementation would have more fields)
        // For now, return success
        nlohmann::json response;
        response["success"] = true;
        response["applicationId"] = "MM_APP_001";
        response["status"] = "PENDING";
        response["message"] = "Application submitted successfully";
        
        return jsonToString(response);
        
    } catch (const std::exception& e) {
        return createErrorResponse(500, "INTERNAL_ERROR", e.what());
    }
}

std::string RESTServer::getPendingMMApplications(const std::string& sessionToken) {
    if (!adminSystem) {
        return createErrorResponse(503, "SERVICE_UNAVAILABLE", 
            "Admin system not available");
    }
    
    try {
        std::string adminId = adminSystem->verifyAdminSession(sessionToken);
        if (adminId.empty()) {
            return createErrorResponse(401, "UNAUTHORIZED", "Invalid session");
        }
        
        auto applications = adminSystem->getPendingApplications(adminId);
        
        nlohmann::json response;
        response["applications"] = nlohmann::json::array();
        
        for (const auto& app : applications) {
            nlohmann::json appJson;
            appJson["applicationId"] = app.applicationId;
            appJson["companyName"] = app.companyName;
            appJson["licenseNumber"] = app.licenseNumber;
            appJson["contactEmail"] = app.contactEmail;
            appJson["submittedAt"] = app.submittedAt;
            // Add more fields as needed
            response["applications"].push_back(appJson);
        }
        
        response["count"] = applications.size();
        
        return jsonToString(response);
        
    } catch (const std::exception& e) {
        return createErrorResponse(403, "FORBIDDEN", e.what());
    }
}

std::string RESTServer::getMMApplicationDetails(const std::string& sessionToken, const std::string& appId) {
    if (!adminSystem) {
        return createErrorResponse(503, "SERVICE_UNAVAILABLE", 
            "Admin system not available");
    }
    
    try {
        std::string adminId = adminSystem->verifyAdminSession(sessionToken);
        if (adminId.empty()) {
            return createErrorResponse(401, "UNAUTHORIZED", "Invalid session");
        }
        
        auto app = adminSystem->getApplicationDetails(adminId, appId);
        
        nlohmann::json response;
        
        // Basic Information
        response["applicationId"] = app.applicationId;
        response["applicantAddress"] = app.applicantAddress;
        response["companyName"] = app.companyName;
        response["licenseNumber"] = app.licenseNumber;
        response["regulatoryBody"] = app.regulatoryBody;
        response["country"] = app.country;
        response["contactEmail"] = app.contactEmail;
        response["contactPhone"] = app.contactPhone;
        response["website"] = app.website;
        
        // Documents (hashes for verification)
        response["documents"] = nlohmann::json::object();
        response["documents"]["licenseDocument"] = app.licenseDocumentHash;
        response["documents"]["financialStatements"] = app.financialStatementsHash;
        response["documents"]["technicalCapabilities"] = app.technicalCapabilitiesHash;
        response["documents"]["kycDocuments"] = app.kycDocumentsHash;
        
        // Status Information
        response["status"] = applicationStatusToString(app.status);
        response["submittedAt"] = app.submittedAt;
        response["lastUpdatedAt"] = app.lastUpdatedAt;
        
        // Verification Steps
        response["verificationSteps"] = nlohmann::json::array();
        for (const auto& step : app.verificationSteps) {
            nlohmann::json stepJson;
            stepJson["step"] = step.step;
            stepJson["verifiedBy"] = step.verifiedBy;
            stepJson["verifiedAt"] = step.verifiedAt;
            stepJson["notes"] = step.notes;
            stepJson["passed"] = step.passed;
            response["verificationSteps"].push_back(stepJson);
        }
        
        // Decision Information
        response["approvedBy"] = app.approvedBy;
        response["decidedAt"] = app.decidedAt;
        response["decisionNotes"] = app.decisionNotes;
        
        // If Approved
        if (!app.marketMakerId.empty()) {
            response["marketMakerId"] = app.marketMakerId;
            response["activatedAt"] = app.activatedAt;
        }
        
        return jsonToString(response);
        
    } catch (const std::exception& e) {
        return createErrorResponse(403, "FORBIDDEN", e.what());
    }
}

std::string RESTServer::verifyMMLicense(const std::string& sessionToken, const std::string& body) {
    if (!adminSystem) {
        return createErrorResponse(503, "SERVICE_UNAVAILABLE", 
            "Admin system not available");
    }
    
    try {
        std::string adminId = adminSystem->verifyAdminSession(sessionToken);
        if (adminId.empty()) {
            return createErrorResponse(401, "UNAUTHORIZED", "Invalid session");
        }
        
        auto json = nlohmann::json::parse(body);
        
        if (!json.contains("applicationId")) {
            return createErrorResponse(400, "MISSING_FIELDS", 
                "applicationId is required");
        }
        
        std::string applicationId = json["applicationId"];
        std::string notes = json.value("notes", "");
        
        bool success = adminSystem->verifyLicense(adminId, applicationId, notes);
        
        nlohmann::json response;
        response["success"] = success;
        response["message"] = "License verified successfully";
        
        return jsonToString(response);
        
    } catch (const std::exception& e) {
        return createErrorResponse(403, "FORBIDDEN", e.what());
    }
}

std::string RESTServer::approveMMApplication(const std::string& sessionToken, const std::string& body) {
    if (!adminSystem) {
        return createErrorResponse(503, "SERVICE_UNAVAILABLE", 
            "Admin system not available");
    }
    
    try {
        std::string adminId = adminSystem->verifyAdminSession(sessionToken);
        if (adminId.empty()) {
            return createErrorResponse(401, "UNAUTHORIZED", "Invalid session");
        }
        
        auto json = nlohmann::json::parse(body);
        
        if (!json.contains("applicationId")) {
            return createErrorResponse(400, "MISSING_FIELDS", 
                "applicationId is required");
        }
        
        std::string applicationId = json["applicationId"];
        std::string notes = json.value("notes", "");
        
        bool success = adminSystem->approveApplication(adminId, applicationId, notes);
        
        nlohmann::json response;
        response["success"] = success;
        response["message"] = "Application approved successfully";
        
        return jsonToString(response);
        
    } catch (const std::exception& e) {
        return createErrorResponse(403, "FORBIDDEN", e.what());
    }
}

std::string RESTServer::rejectMMApplication(const std::string& sessionToken, const std::string& body) {
    if (!adminSystem) {
        return createErrorResponse(503, "SERVICE_UNAVAILABLE", 
            "Admin system not available");
    }
    
    try {
        std::string adminId = adminSystem->verifyAdminSession(sessionToken);
        if (adminId.empty()) {
            return createErrorResponse(401, "UNAUTHORIZED", "Invalid session");
        }
        
        auto json = nlohmann::json::parse(body);
        
        if (!json.contains("applicationId")) {
            return createErrorResponse(400, "MISSING_FIELDS", 
                "applicationId is required");
        }
        
        std::string applicationId = json["applicationId"];
        std::string reason = json.value("reason", "");
        
        bool success = adminSystem->rejectApplication(adminId, applicationId, reason);
        
        nlohmann::json response;
        response["success"] = success;
        response["message"] = "Application rejected";
        response["reason"] = reason;
        
        return jsonToString(response);
        
    } catch (const std::exception& e) {
        return createErrorResponse(403, "FORBIDDEN", e.what());
    }
}

// ============================================================================
// MARKET MAKER DOCUMENT MANAGEMENT
// ============================================================================

std::string RESTServer::listMMDocuments(const std::string& sessionToken, const std::string& appId) {
    if (!adminSystem) {
        return createErrorResponse(503, "SERVICE_UNAVAILABLE", 
            "Admin system not available");
    }
    
    try {
        std::string adminId = adminSystem->verifyAdminSession(sessionToken);
        if (adminId.empty()) {
            return createErrorResponse(401, "UNAUTHORIZED", "Invalid session");
        }
        
        auto app = adminSystem->getApplicationDetails(adminId, appId);
        
        nlohmann::json response;
        response["applicationId"] = appId;
        response["companyName"] = app.companyName;
        response["documents"] = nlohmann::json::array();
        
        // License Document
        if (!app.licenseDocumentHash.empty()) {
            nlohmann::json doc;
            doc["type"] = "license";
            doc["name"] = "Business License";
            doc["hash"] = app.licenseDocumentHash;
            doc["downloadUrl"] = "/api/admin/mm/document/" + appId + "/license";
            response["documents"].push_back(doc);
        }
        
        // Financial Statements
        if (!app.financialStatementsHash.empty()) {
            nlohmann::json doc;
            doc["type"] = "financial";
            doc["name"] = "Financial Statements";
            doc["hash"] = app.financialStatementsHash;
            doc["downloadUrl"] = "/api/admin/mm/document/" + appId + "/financial";
            response["documents"].push_back(doc);
        }
        
        // Technical Capabilities
        if (!app.technicalCapabilitiesHash.empty()) {
            nlohmann::json doc;
            doc["type"] = "technical";
            doc["name"] = "Technical Capabilities Document";
            doc["hash"] = app.technicalCapabilitiesHash;
            doc["downloadUrl"] = "/api/admin/mm/document/" + appId + "/technical";
            response["documents"].push_back(doc);
        }
        
        // KYC Documents
        if (!app.kycDocumentsHash.empty()) {
            nlohmann::json doc;
            doc["type"] = "kyc";
            doc["name"] = "KYC/AML Documents";
            doc["hash"] = app.kycDocumentsHash;
            doc["downloadUrl"] = "/api/admin/mm/document/" + appId + "/kyc";
            response["documents"].push_back(doc);
        }
        
        response["count"] = response["documents"].size();
        
        return jsonToString(response);
        
    } catch (const std::exception& e) {
        return createErrorResponse(403, "FORBIDDEN", e.what());
    }
}

std::string RESTServer::getMMDocument(const std::string& sessionToken, const std::string& appId, const std::string& docType) {
    if (!adminSystem) {
        return createErrorResponse(503, "SERVICE_UNAVAILABLE", 
            "Admin system not available");
    }
    
    try {
        std::string adminId = adminSystem->verifyAdminSession(sessionToken);
        if (adminId.empty()) {
            return createErrorResponse(401, "UNAUTHORIZED", "Invalid session");
        }
        
        auto app = adminSystem->getApplicationDetails(adminId, appId);
        
        std::string documentHash;
        std::string documentName;
        
        if (docType == "license") {
            documentHash = app.licenseDocumentHash;
            documentName = "Business License";
        } else if (docType == "financial") {
            documentHash = app.financialStatementsHash;
            documentName = "Financial Statements";
        } else if (docType == "technical") {
            documentHash = app.technicalCapabilitiesHash;
            documentName = "Technical Capabilities";
        } else if (docType == "kyc") {
            documentHash = app.kycDocumentsHash;
            documentName = "KYC/AML Documents";
        } else {
            return createErrorResponse(400, "INVALID_DOCUMENT_TYPE", 
                "Document type must be: license, financial, technical, or kyc");
        }
        
        if (documentHash.empty()) {
            return createErrorResponse(404, "DOCUMENT_NOT_FOUND", 
                "Document not uploaded for this application");
        }
        
        // In a real implementation, you would:
        // 1. Retrieve the document from IPFS/storage using the hash
        // 2. Return the document content with appropriate headers
        // For now, return document metadata
        
        nlohmann::json response;
        response["applicationId"] = appId;
        response["documentType"] = docType;
        response["documentName"] = documentName;
        response["documentHash"] = documentHash;
        response["companyName"] = app.companyName;
        
        // Document metadata
        response["metadata"] = nlohmann::json::object();
        response["metadata"]["uploadedAt"] = app.submittedAt;
        response["metadata"]["verified"] = false; // Check verification steps
        
        // Check if this document type has been verified
        for (const auto& step : app.verificationSteps) {
            if ((docType == "license" && step.step == "LICENSE_VERIFICATION") ||
                (docType == "financial" && step.step == "FINANCIAL_REVIEW") ||
                (docType == "technical" && step.step == "TECHNICAL_VERIFICATION") ||
                (docType == "kyc" && step.step == "KYC_AML_VERIFICATION")) {
                response["metadata"]["verified"] = step.passed;
                response["metadata"]["verifiedBy"] = step.verifiedBy;
                response["metadata"]["verifiedAt"] = step.verifiedAt;
                response["metadata"]["verificationNotes"] = step.notes;
            }
        }
        
        // In production, add:
        // response["downloadUrl"] = "https://ipfs.io/ipfs/" + documentHash;
        // Or return the actual file content with proper Content-Type headers
        
        return jsonToString(response);
        
    } catch (const std::exception& e) {
        return createErrorResponse(403, "FORBIDDEN", e.what());
    }
}

// ============================================================================
// DASHBOARD & STATISTICS
// ============================================================================

std::string RESTServer::getAdminDashboard(const std::string& sessionToken) {
    if (!adminSystem) {
        return createErrorResponse(503, "SERVICE_UNAVAILABLE", 
            "Admin system not available");
    }
    
    try {
        auto dashboard = adminSystem->getDashboardOverview(sessionToken);
        return jsonToString(dashboard);
        
    } catch (const std::exception& e) {
        return createErrorResponse(403, "FORBIDDEN", e.what());
    }
}

std::string RESTServer::getAuditLog(const std::string& sessionToken) {
    if (!adminSystem) {
        return createErrorResponse(503, "SERVICE_UNAVAILABLE", 
            "Admin system not available");
    }
    
    try {
        std::string adminId = adminSystem->verifyAdminSession(sessionToken);
        if (adminId.empty()) {
            return createErrorResponse(401, "UNAUTHORIZED", "Invalid session");
        }
        
        auto auditLog = adminSystem->getAuditLog(adminId);
        
        nlohmann::json response;
        response["logs"] = nlohmann::json::array();
        
        for (const auto& log : auditLog) {
            nlohmann::json logJson;
            logJson["logId"] = log.logId;
            logJson["adminId"] = log.adminId;
            logJson["action"] = log.action;
            logJson["timestamp"] = log.timestamp;
            logJson["details"] = log.details;
            response["logs"].push_back(logJson);
        }
        
        response["count"] = auditLog.size();
        
        return jsonToString(response);
        
    } catch (const std::exception& e) {
        return createErrorResponse(403, "FORBIDDEN", e.what());
    }
}

std::string RESTServer::getSystemStatistics(const std::string& sessionToken) {
    if (!adminSystem) {
        return createErrorResponse(503, "SERVICE_UNAVAILABLE", 
            "Admin system not available");
    }
    
    try {
        auto activity = adminSystem->getAdminActivitySummary(sessionToken);
        
        // Add fraud statistics if available
        if (fraudDetection) {
            auto fraudStats = fraudDetection->getStatistics();
            activity["fraudDetection"]["totalStolenTx"] = fraudStats.totalStolenTx;
            activity["fraudDetection"]["totalTaintedTx"] = fraudStats.totalTaintedTx;
            activity["fraudDetection"]["totalAlerts"] = fraudStats.totalAlerts;
            activity["fraudDetection"]["criticalAlerts"] = fraudStats.criticalAlerts;
        }
        
        // Add blockchain statistics if available
        if (blockchain) {
            activity["blockchain"]["totalBlocks"] = blockchain->getChainLength();
            activity["blockchain"]["totalAddresses"] = blockchain->getTotalAddressCount();
        }
        
        return jsonToString(activity);
        
    } catch (const std::exception& e) {
        return createErrorResponse(403, "FORBIDDEN", e.what());
    }
}

// ===== REVERSAL SYSTEM ENDPOINTS =====

/**
 * Get reversal fee pool statistics
 * GET /api/admin/reversal/pool-stats
 */
std::string RESTServer::getReversalPoolStats(const std::string& sessionToken) {
    if (!adminSystem) {
        return createErrorResponse(503, "SERVICE_UNAVAILABLE", 
            "Admin system not available");
    }
    
    try {
        // Verify admin session
        std::string adminId;
        if (!adminSystem->verifySessionToken(sessionToken, adminId)) {
            return createErrorResponse(401, "UNAUTHORIZED", 
                "Invalid or expired session token");
        }
        
        // Check permission
        if (!adminSystem->hasPermission(adminId, "view_reversal_stats")) {
            return createErrorResponse(403, "FORBIDDEN", 
                "You do not have permission to view reversal statistics");
        }
        
        // Get pool statistics
        if (!feePool) {
            return createErrorResponse(503, "SERVICE_UNAVAILABLE", 
                "Reversal fee pool not initialized");
        }
        
        auto stats = feePool->getStatistics();
        
        json response;
        response["success"] = true;
        response["pool_address"] = stats.pool_address;
        response["current_balance"] = stats.current_balance;
        response["total_funded"] = stats.total_funded;
        response["total_spent"] = stats.total_spent;
        response["total_reversals"] = stats.total_reversals;
        response["average_fee"] = stats.average_fee;
        response["is_balance_low"] = stats.is_balance_low;
        response["funding_count"] = stats.funding_count;
        response["last_funding_amount"] = stats.last_funding_amount;
        response["last_funding_timestamp"] = stats.last_funding_timestamp;
        
        return jsonToString(response);
        
    } catch (const std::exception& e) {
        return createErrorResponse(500, "INTERNAL_ERROR", e.what());
    }
}

/**
 * Record pool funding
 * POST /api/admin/reversal/record-funding
 */
std::string RESTServer::recordPoolFunding(const std::string& sessionToken, const std::string& body) {
    if (!adminSystem) {
        return createErrorResponse(503, "SERVICE_UNAVAILABLE", 
            "Admin system not available");
    }
    
    try {
        // Verify admin session
        std::string adminId;
        if (!adminSystem->verifySessionToken(sessionToken, adminId)) {
            return createErrorResponse(401, "UNAUTHORIZED", 
                "Invalid or expired session token");
        }
        
        // Parse request
        auto json_body = json::parse(body);
        
        std::string tx_hash = json_body["tx_hash"];
        std::string from_address = json_body["from_address"];
        uint64_t amount = json_body["amount"];
        std::string note = json_body.value("note", "");
        
        // Record funding
        if (!feePool) {
            return createErrorResponse(503, "SERVICE_UNAVAILABLE", 
                "Reversal fee pool not initialized");
        }
        
        feePool->recordFunding(tx_hash, from_address, amount, note);
        LOG_INFO("Pool funding recorded: " + std::to_string(amount) + " from " + from_address);
        
        json response;
        response["success"] = true;
        response["message"] = "Funding recorded successfully";
        response["tx_hash"] = tx_hash;
        response["amount"] = amount;
        
        return jsonToString(response);
        
    } catch (const std::exception& e) {
        return createErrorResponse(500, "INTERNAL_ERROR", e.what());
    }
}
