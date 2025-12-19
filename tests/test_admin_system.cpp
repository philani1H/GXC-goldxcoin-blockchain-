#include "../include/MarketMakerAdmin.h"
#include <iostream>
#include <cassert>

int tests_passed = 0;
int tests_failed = 0;

void printTestResult(const std::string& testName, bool passed) {
    if (passed) {
        std::cout << "✅ PASS: " << testName << std::endl;
        tests_passed++;
    } else {
        std::cout << "❌ FAIL: " << testName << std::endl;
        tests_failed++;
    }
}

// Test 1: First Admin Creation
void test_first_admin_creation() {
    std::cout << "\n=== Test 1: First Admin Creation ===" << std::endl;
    
    MarketMakerAdmin admin;
    
    // Try to login with first admin
    try {
        std::string sessionToken = admin.adminLogin(
            "Philani-GXC.Foundation",
            "GXC$ecure2025!Philani#Foundation@Admin"
        );
        printTestResult("First admin login successful", !sessionToken.empty());
    } catch (const std::exception& e) {
        printTestResult("First admin login successful", false);
        std::cout << "  Error: " << e.what() << std::endl;
    }
}

// Test 2: Create Additional Admin
void test_create_admin() {
    std::cout << "\n=== Test 2: Create Additional Admin ===" << std::endl;
    
    MarketMakerAdmin admin;
    
    // Login as super admin
    std::string superAdminToken = admin.adminLogin(
        "Philani-GXC.Foundation",
        "GXC$ecure2025!Philani#Foundation@Admin"
    );
    
    // Create verifier admin
    try {
        std::string newAdminId = admin.createAdmin(
            "admin_philani_gxc_foundation",
            "john_verifier",
            "SecurePass123!",
            "verifier",
            {"view_applications", "verify_license", "check_reputation"}
        );
        printTestResult("Create verifier admin", !newAdminId.empty());
        
        // Try to login with new admin
        std::string verifierToken = admin.adminLogin("john_verifier", "SecurePass123!");
        printTestResult("New admin can login", !verifierToken.empty());
    } catch (const std::exception& e) {
        printTestResult("Create verifier admin", false);
        std::cout << "  Error: " << e.what() << std::endl;
    }
}

// Test 3: Change Password
void test_change_password() {
    std::cout << "\n=== Test 3: Change Password ===" << std::endl;
    
    MarketMakerAdmin admin;
    
    // Create admin
    admin.createAdmin(
        "admin_philani_gxc_foundation",
        "test_user",
        "OldPassword123!",
        "verifier",
        {"view_applications"}
    );
    
    // Login
    admin.adminLogin("test_user", "OldPassword123!");
    
    // Change password
    try {
        bool changed = admin.changePassword(
            "ADMIN_*",  // Will be extracted from session
            "OldPassword123!",
            "NewPassword456!"
        );
        printTestResult("Password changed", changed);
        
        // Try old password (should fail)
        try {
            admin.adminLogin("test_user", "OldPassword123!");
            printTestResult("Old password rejected", false);
        } catch (...) {
            printTestResult("Old password rejected", true);
        }
        
        // Try new password (should work)
        std::string token = admin.adminLogin("test_user", "NewPassword456!");
        printTestResult("New password works", !token.empty());
    } catch (const std::exception& e) {
        printTestResult("Password changed", false);
        std::cout << "  Error: " << e.what() << std::endl;
    }
}

// Test 4: Update Permissions
void test_update_permissions() {
    std::cout << "\n=== Test 4: Update Permissions ===" << std::endl;
    
    MarketMakerAdmin admin;
    
    // Create admin with limited permissions
    std::string adminId = admin.createAdmin(
        "admin_philani_gxc_foundation",
        "limited_user",
        "Password123!",
        "verifier",
        {"view_applications"}
    );
    
    // Update permissions
    try {
        bool updated = admin.updateAdminPermissions(
            "admin_philani_gxc_foundation",
            adminId,
            {"view_applications", "verify_license", "check_reputation", "review_financial"}
        );
        printTestResult("Permissions updated", updated);
    } catch (const std::exception& e) {
        printTestResult("Permissions updated", false);
        std::cout << "  Error: " << e.what() << std::endl;
    }
}

// Test 5: Deactivate/Reactivate Admin
void test_deactivate_reactivate() {
    std::cout << "\n=== Test 5: Deactivate/Reactivate Admin ===" << std::endl;
    
    MarketMakerAdmin admin;
    
    // Create admin
    std::string adminId = admin.createAdmin(
        "admin_philani_gxc_foundation",
        "temp_user",
        "Password123!",
        "verifier",
        {"view_applications"}
    );
    
    // Deactivate
    try {
        bool deactivated = admin.deactivateAdmin(
            "admin_philani_gxc_foundation",
            adminId
        );
        printTestResult("Admin deactivated", deactivated);
        
        // Try to login (should fail)
        try {
            admin.adminLogin("temp_user", "Password123!");
            printTestResult("Deactivated admin cannot login", false);
        } catch (...) {
            printTestResult("Deactivated admin cannot login", true);
        }
        
        // Reactivate
        bool reactivated = admin.reactivateAdmin(
            "admin_philani_gxc_foundation",
            adminId
        );
        printTestResult("Admin reactivated", reactivated);
        
        // Try to login (should work)
        std::string token = admin.adminLogin("temp_user", "Password123!");
        printTestResult("Reactivated admin can login", !token.empty());
    } catch (const std::exception& e) {
        printTestResult("Deactivate/reactivate", false);
        std::cout << "  Error: " << e.what() << std::endl;
    }
}

// Test 6: Remove Admin
void test_remove_admin() {
    std::cout << "\n=== Test 6: Remove Admin ===" << std::endl;
    
    MarketMakerAdmin admin;
    
    // Create admin
    std::string adminId = admin.createAdmin(
        "admin_philani_gxc_foundation",
        "remove_me",
        "Password123!",
        "verifier",
        {"view_applications"}
    );
    
    // Remove admin
    try {
        bool removed = admin.removeAdmin(
            "admin_philani_gxc_foundation",
            adminId
        );
        printTestResult("Admin removed", removed);
        
        // Try to login (should fail)
        try {
            admin.adminLogin("remove_me", "Password123!");
            printTestResult("Removed admin cannot login", false);
        } catch (...) {
            printTestResult("Removed admin cannot login", true);
        }
    } catch (const std::exception& e) {
        printTestResult("Remove admin", false);
        std::cout << "  Error: " << e.what() << std::endl;
    }
}

// Test 7: Submit Application
void test_submit_application() {
    std::cout << "\n=== Test 7: Submit Application ===" << std::endl;
    
    MarketMakerAdmin admin;
    
    try {
        std::string appId = admin.submitApplication(
            "tGXC_goldman_sachs",
            "Goldman Sachs Trading Desk",
            "MM-12345-US",
            "SEC",
            "USA",
            "mm-support@goldmansachs.com",
            "+1-212-555-0100",
            "https://www.goldmansachs.com",
            "0xabc123",
            "0xdef456",
            "0xghi789",
            "0xjkl012"
        );
        printTestResult("Application submitted", !appId.empty());
        
        // Check application status
        json status = admin.getApplicationStatus(appId);
        printTestResult("Application status retrieved", status["status"] == "PENDING");
    } catch (const std::exception& e) {
        printTestResult("Application submitted", false);
        std::cout << "  Error: " << e.what() << std::endl;
    }
}

// Test 8: Verification Workflow
void test_verification_workflow() {
    std::cout << "\n=== Test 8: Verification Workflow ===" << std::endl;
    
    MarketMakerAdmin admin;
    
    // Submit application
    std::string appId = admin.submitApplication(
        "tGXC_citadel",
        "Citadel Securities",
        "MM-67890-US",
        "SEC",
        "USA",
        "mm@citadel.com",
        "+1-312-555-0200",
        "https://www.citadel.com",
        "0xabc123",
        "0xdef456",
        "0xghi789",
        "0xjkl012"
    );
    
    // Step 1: Verify license
    try {
        bool verified = admin.verifyLicense(
            "admin_philani_gxc_foundation",
            appId,
            true,
            "Verified with SEC. License active.",
            "0xproof_license"
        );
        printTestResult("Step 1: License verified", verified);
    } catch (const std::exception& e) {
        printTestResult("Step 1: License verified", false);
        std::cout << "  Error: " << e.what() << std::endl;
    }
    
    // Step 2: Check reputation
    try {
        bool checked = admin.checkCompanyReputation(
            "admin_philani_gxc_foundation",
            appId,
            true,
            "Excellent reputation. No issues.",
            "0xproof_reputation"
        );
        printTestResult("Step 2: Reputation checked", checked);
    } catch (const std::exception& e) {
        printTestResult("Step 2: Reputation checked", false);
        std::cout << "  Error: " << e.what() << std::endl;
    }
    
    // Step 3: Review financial
    try {
        bool reviewed = admin.reviewFinancialStanding(
            "admin_philani_gxc_foundation",
            appId,
            true,
            "Strong financial position.",
            "0xproof_financial"
        );
        printTestResult("Step 3: Financial reviewed", reviewed);
    } catch (const std::exception& e) {
        printTestResult("Step 3: Financial reviewed", false);
        std::cout << "  Error: " << e.what() << std::endl;
    }
    
    // Step 4: Verify technical
    try {
        bool verified = admin.verifyTechnicalCapabilities(
            "admin_philani_gxc_foundation",
            appId,
            true,
            "Proven infrastructure.",
            "0xproof_technical"
        );
        printTestResult("Step 4: Technical verified", verified);
    } catch (const std::exception& e) {
        printTestResult("Step 4: Technical verified", false);
        std::cout << "  Error: " << e.what() << std::endl;
    }
    
    // Step 5: Complete KYC/AML
    try {
        bool completed = admin.completeKycAmlCheck(
            "admin_philani_gxc_foundation",
            appId,
            true,
            "KYC/AML verified.",
            "0xproof_kyc"
        );
        printTestResult("Step 5: KYC/AML completed", completed);
    } catch (const std::exception& e) {
        printTestResult("Step 5: KYC/AML completed", false);
        std::cout << "  Error: " << e.what() << std::endl;
    }
}

// Test 9: Approve Application
void test_approve_application() {
    std::cout << "\n=== Test 9: Approve Application ===" << std::endl;
    
    MarketMakerAdmin admin;
    
    // Submit and verify application
    std::string appId = admin.submitApplication(
        "tGXC_janestreet",
        "Jane Street Capital",
        "MM-11111-US",
        "SEC",
        "USA",
        "mm@janestreet.com",
        "+1-212-555-0300",
        "https://www.janestreet.com",
        "0xabc123",
        "0xdef456",
        "0xghi789",
        "0xjkl012"
    );
    
    // Complete all verification steps
    admin.verifyLicense("admin_philani_gxc_foundation", appId, true, "Verified", "0xproof1");
    admin.checkCompanyReputation("admin_philani_gxc_foundation", appId, true, "Checked", "0xproof2");
    admin.reviewFinancialStanding("admin_philani_gxc_foundation", appId, true, "Reviewed", "0xproof3");
    admin.verifyTechnicalCapabilities("admin_philani_gxc_foundation", appId, true, "Verified", "0xproof4");
    admin.completeKycAmlCheck("admin_philani_gxc_foundation", appId, true, "Completed", "0xproof5");
    
    // Approve
    try {
        bool approved = admin.approveApplication(
            "admin_philani_gxc_foundation",
            appId,
            "All steps passed. Approved."
        );
        printTestResult("Application approved", approved);
        
        // Check status
        json status = admin.getApplicationStatus(appId);
        printTestResult("Application status is APPROVED", status["status"] == "APPROVED");
    } catch (const std::exception& e) {
        printTestResult("Application approved", false);
        std::cout << "  Error: " << e.what() << std::endl;
    }
}

// Test 10: Reject Application
void test_reject_application() {
    std::cout << "\n=== Test 10: Reject Application ===" << std::endl;
    
    MarketMakerAdmin admin;
    
    // Submit application
    std::string appId = admin.submitApplication(
        "tGXC_rejected",
        "Rejected Company",
        "MM-99999-US",
        "SEC",
        "USA",
        "mm@rejected.com",
        "+1-212-555-0400",
        "https://www.rejected.com",
        "0xabc123",
        "0xdef456",
        "0xghi789",
        "0xjkl012"
    );
    
    // Reject
    try {
        bool rejected = admin.rejectApplication(
            "admin_philani_gxc_foundation",
            appId,
            "Failed financial review."
        );
        printTestResult("Application rejected", rejected);
        
        // Check status
        json status = admin.getApplicationStatus(appId);
        printTestResult("Application status is REJECTED", status["status"] == "REJECTED");
    } catch (const std::exception& e) {
        printTestResult("Application rejected", false);
        std::cout << "  Error: " << e.what() << std::endl;
    }
}

// Test 11: Get Statistics
void test_get_statistics() {
    std::cout << "\n=== Test 11: Get Statistics ===" << std::endl;
    
    MarketMakerAdmin admin;
    
    try {
        json stats = admin.getVerificationStatistics("admin_philani_gxc_foundation");
        printTestResult("Statistics retrieved", stats.contains("total"));
        
        std::cout << "  Total applications: " << stats["total"] << std::endl;
        std::cout << "  Pending: " << stats["pending"] << std::endl;
        std::cout << "  Approved: " << stats["approved"] << std::endl;
        std::cout << "  Rejected: " << stats["rejected"] << std::endl;
    } catch (const std::exception& e) {
        printTestResult("Statistics retrieved", false);
        std::cout << "  Error: " << e.what() << std::endl;
    }
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  ADMIN SYSTEM COMPREHENSIVE TESTS" << std::endl;
    std::cout << "========================================" << std::endl;
    
    test_first_admin_creation();
    test_create_admin();
    test_change_password();
    test_update_permissions();
    test_deactivate_reactivate();
    test_remove_admin();
    test_submit_application();
    test_verification_workflow();
    test_approve_application();
    test_reject_application();
    test_get_statistics();
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "  TEST SUMMARY" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "✅ Tests Passed: " << tests_passed << std::endl;
    std::cout << "❌ Tests Failed: " << tests_failed << std::endl;
    std::cout << "Total Tests: " << (tests_passed + tests_failed) << std::endl;
    
    if (tests_failed == 0) {
        std::cout << "\n🎉 ALL TESTS PASSED! 🎉" << std::endl;
        std::cout << "\nAdmin system is ready for production:" << std::endl;
        std::cout << "  ✅ First admin created (Philani-GXC.Foundation)" << std::endl;
        std::cout << "  ✅ Admin management working" << std::endl;
        std::cout << "  ✅ Password changes working" << std::endl;
        std::cout << "  ✅ Permission updates working" << std::endl;
        std::cout << "  ✅ Application submission working" << std::endl;
        std::cout << "  ✅ 5-step verification working" << std::endl;
        std::cout << "  ✅ Approval/rejection working" << std::endl;
        std::cout << "  ✅ Statistics working" << std::endl;
        return 0;
    } else {
        std::cout << "\n⚠️  SOME TESTS FAILED" << std::endl;
        return 1;
    }
}
