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

void printSection(const std::string& section) {
    std::cout << "\n========================================" << std::endl;
    std::cout << "  " << section << std::endl;
    std::cout << "========================================" << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  ADMIN MANAGEMENT COMPREHENSIVE TESTS" << std::endl;
    std::cout << "========================================" << std::endl;
    
    MarketMakerAdmin admin;
    
    // Test 1: First Admin Login
    printSection("TEST 1: First Admin Login");
    try {
        std::string token = admin.adminLogin(
            "Philani-GXC.Foundation",
            "GXC$ecure2025!Philani#Foundation@Admin"
        );
        printTestResult("First admin login", !token.empty());
        std::cout << "  Session token: " << token.substr(0, 30) << "..." << std::endl;
    } catch (const std::exception& e) {
        printTestResult("First admin login", false);
        std::cout << "  Error: " << e.what() << std::endl;
    }
    
    // Test 2: Create Verifier Admin
    printSection("TEST 2: Create Verifier Admin");
    std::string verifierId;
    try {
        verifierId = admin.createAdmin(
            "admin_philani_gxc_foundation",
            "john_verifier",
            "VerifierPass123!",
            "verifier",
            {"view_applications", "verify_license", "check_reputation", 
             "review_financial", "verify_technical", "complete_kyc_aml"}
        );
        printTestResult("Create verifier admin", !verifierId.empty());
        std::cout << "  Verifier ID: " << verifierId << std::endl;
        
        // Test verifier login
        std::string verifierToken = admin.adminLogin("john_verifier", "VerifierPass123!");
        printTestResult("Verifier can login", !verifierToken.empty());
    } catch (const std::exception& e) {
        printTestResult("Create verifier admin", false);
        std::cout << "  Error: " << e.what() << std::endl;
    }
    
    // Test 3: Create Reviewer Admin
    printSection("TEST 3: Create Reviewer Admin");
    std::string reviewerId;
    try {
        reviewerId = admin.createAdmin(
            "admin_philani_gxc_foundation",
            "jane_reviewer",
            "ReviewerPass123!",
            "reviewer",
            {"view_applications", "assign_applications"}
        );
        printTestResult("Create reviewer admin", !reviewerId.empty());
        std::cout << "  Reviewer ID: " << reviewerId << std::endl;
        
        // Test reviewer login
        std::string reviewerToken = admin.adminLogin("jane_reviewer", "ReviewerPass123!");
        printTestResult("Reviewer can login", !reviewerToken.empty());
    } catch (const std::exception& e) {
        printTestResult("Create reviewer admin", false);
        std::cout << "  Error: " << e.what() << std::endl;
    }
    
    // Test 4: Change Password
    printSection("TEST 4: Change Password");
    try {
        // Create test admin
        std::string testAdminId = admin.createAdmin(
            "admin_philani_gxc_foundation",
            "test_password_user",
            "OldPassword123!",
            "verifier",
            {"view_applications"}
        );
        
        // Login with old password
        admin.adminLogin("test_password_user", "OldPassword123!");
        printTestResult("Login with old password", true);
        
        // Change password
        bool changed = admin.changePassword(
            testAdminId,
            "OldPassword123!",
            "NewPassword456!"
        );
        printTestResult("Password changed", changed);
        
        // Try old password (should fail)
        try {
            admin.adminLogin("test_password_user", "OldPassword123!");
            printTestResult("Old password rejected", false);
        } catch (...) {
            printTestResult("Old password rejected", true);
        }
        
        // Try new password (should work)
        std::string newToken = admin.adminLogin("test_password_user", "NewPassword456!");
        printTestResult("New password works", !newToken.empty());
    } catch (const std::exception& e) {
        printTestResult("Change password", false);
        std::cout << "  Error: " << e.what() << std::endl;
    }
    
    // Test 5: Update Permissions
    printSection("TEST 5: Update Permissions");
    try {
        // Create admin with limited permissions
        std::string limitedAdminId = admin.createAdmin(
            "admin_philani_gxc_foundation",
            "limited_user",
            "LimitedPass123!",
            "verifier",
            {"view_applications"}
        );
        printTestResult("Created admin with limited permissions", !limitedAdminId.empty());
        
        // Update permissions
        bool updated = admin.updateAdminPermissions(
            "admin_philani_gxc_foundation",
            limitedAdminId,
            {"view_applications", "verify_license", "check_reputation", 
             "review_financial", "verify_technical", "complete_kyc_aml"}
        );
        printTestResult("Permissions updated", updated);
        std::cout << "  New permissions: 6 permissions granted" << std::endl;
    } catch (const std::exception& e) {
        printTestResult("Update permissions", false);
        std::cout << "  Error: " << e.what() << std::endl;
    }
    
    // Test 6: Deactivate Admin
    printSection("TEST 6: Deactivate Admin");
    try {
        // Create admin to deactivate
        std::string deactivateAdminId = admin.createAdmin(
            "admin_philani_gxc_foundation",
            "deactivate_user",
            "DeactivatePass123!",
            "verifier",
            {"view_applications"}
        );
        
        // Verify can login
        admin.adminLogin("deactivate_user", "DeactivatePass123!");
        printTestResult("Admin can login before deactivation", true);
        
        // Deactivate
        bool deactivated = admin.deactivateAdmin(
            "admin_philani_gxc_foundation",
            deactivateAdminId
        );
        printTestResult("Admin deactivated", deactivated);
        
        // Try to login (should fail)
        try {
            admin.adminLogin("deactivate_user", "DeactivatePass123!");
            printTestResult("Deactivated admin cannot login", false);
        } catch (...) {
            printTestResult("Deactivated admin cannot login", true);
        }
    } catch (const std::exception& e) {
        printTestResult("Deactivate admin", false);
        std::cout << "  Error: " << e.what() << std::endl;
    }
    
    // Test 7: Reactivate Admin
    printSection("TEST 7: Reactivate Admin");
    try {
        // Create and deactivate admin
        std::string reactivateAdminId = admin.createAdmin(
            "admin_philani_gxc_foundation",
            "reactivate_user",
            "ReactivatePass123!",
            "verifier",
            {"view_applications"}
        );
        admin.deactivateAdmin("admin_philani_gxc_foundation", reactivateAdminId);
        
        // Reactivate
        bool reactivated = admin.reactivateAdmin(
            "admin_philani_gxc_foundation",
            reactivateAdminId
        );
        printTestResult("Admin reactivated", reactivated);
        
        // Try to login (should work)
        std::string token = admin.adminLogin("reactivate_user", "ReactivatePass123!");
        printTestResult("Reactivated admin can login", !token.empty());
    } catch (const std::exception& e) {
        printTestResult("Reactivate admin", false);
        std::cout << "  Error: " << e.what() << std::endl;
    }
    
    // Test 8: Remove Admin
    printSection("TEST 8: Remove Admin");
    try {
        // Create admin to remove
        std::string removeAdminId = admin.createAdmin(
            "admin_philani_gxc_foundation",
            "remove_user",
            "RemovePass123!",
            "verifier",
            {"view_applications"}
        );
        
        // Verify can login
        admin.adminLogin("remove_user", "RemovePass123!");
        printTestResult("Admin can login before removal", true);
        
        // Remove
        bool removed = admin.removeAdmin(
            "admin_philani_gxc_foundation",
            removeAdminId
        );
        printTestResult("Admin removed", removed);
        
        // Try to login (should fail)
        try {
            admin.adminLogin("remove_user", "RemovePass123!");
            printTestResult("Removed admin cannot login", false);
        } catch (...) {
            printTestResult("Removed admin cannot login", true);
        }
    } catch (const std::exception& e) {
        printTestResult("Remove admin", false);
        std::cout << "  Error: " << e.what() << std::endl;
    }
    
    // Test 9: List All Admins
    printSection("TEST 9: List All Admins");
    try {
        std::vector<AdminUser> admins = admin.listAllAdmins("admin_philani_gxc_foundation");
        printTestResult("List all admins", admins.size() > 0);
        std::cout << "  Total admins: " << admins.size() << std::endl;
        
        // Display admin list
        for (const auto& a : admins) {
            std::cout << "  - " << a.username << " (" << a.role << ")" 
                      << (a.isActive ? " [ACTIVE]" : " [INACTIVE]") << std::endl;
        }
    } catch (const std::exception& e) {
        printTestResult("List all admins", false);
        std::cout << "  Error: " << e.what() << std::endl;
    }
    
    // Test 10: Permission Enforcement
    printSection("TEST 10: Permission Enforcement");
    try {
        // Create reviewer (limited permissions)
        std::string limitedReviewerId = admin.createAdmin(
            "admin_philani_gxc_foundation",
            "limited_reviewer",
            "LimitedReviewerPass123!",
            "reviewer",
            {"view_applications"}
        );
        
        // Try to create admin (should fail - no permission)
        try {
            admin.createAdmin(
                limitedReviewerId,
                "should_fail",
                "FailPass123!",
                "verifier",
                {"view_applications"}
            );
            printTestResult("Permission enforcement (create admin)", false);
        } catch (...) {
            printTestResult("Permission enforcement (create admin)", true);
            std::cout << "  Correctly rejected: Only super admin can create admins" << std::endl;
        }
        
        // Try to approve application (should fail - no permission)
        std::string appId = admin.submitApplication(
            "tGXC_test",
            "Test Company",
            "MM-TEST",
            "SEC",
            "USA",
            "test@test.com",
            "+1-555-0100",
            "https://test.com",
            "0xabc",
            "0xdef",
            "0xghi",
            "0xjkl"
        );
        
        try {
            admin.approveApplication(limitedReviewerId, appId, "Should fail");
            printTestResult("Permission enforcement (approve)", false);
        } catch (...) {
            printTestResult("Permission enforcement (approve)", true);
            std::cout << "  Correctly rejected: Only super admin can approve" << std::endl;
        }
    } catch (const std::exception& e) {
        printTestResult("Permission enforcement", false);
        std::cout << "  Error: " << e.what() << std::endl;
    }
    
    // Summary
    std::cout << "\n========================================" << std::endl;
    std::cout << "  TEST SUMMARY" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "✅ Tests Passed: " << tests_passed << std::endl;
    std::cout << "❌ Tests Failed: " << tests_failed << std::endl;
    std::cout << "Total Tests: " << (tests_passed + tests_failed) << std::endl;
    
    if (tests_failed == 0) {
        std::cout << "\n🎉 ALL TESTS PASSED! 🎉" << std::endl;
        std::cout << "\nAdmin Management Features Working:" << std::endl;
        std::cout << "  ✅ First admin login" << std::endl;
        std::cout << "  ✅ Create verifier admin" << std::endl;
        std::cout << "  ✅ Create reviewer admin" << std::endl;
        std::cout << "  ✅ Change password" << std::endl;
        std::cout << "  ✅ Update permissions" << std::endl;
        std::cout << "  ✅ Deactivate admin" << std::endl;
        std::cout << "  ✅ Reactivate admin" << std::endl;
        std::cout << "  ✅ Remove admin" << std::endl;
        std::cout << "  ✅ List all admins" << std::endl;
        std::cout << "  ✅ Permission enforcement" << std::endl;
        std::cout << "\n🚀 READY FOR PRODUCTION!" << std::endl;
        return 0;
    } else {
        std::cout << "\n⚠️  SOME TESTS FAILED" << std::endl;
        return 1;
    }
}
