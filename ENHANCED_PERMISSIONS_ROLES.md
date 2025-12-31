# Enhanced Permissions and Roles System

## Current Roles (6)

### Existing Roles:
1. `super_admin` - All permissions
2. `fraud_admin` - Fraud management + team
3. `fraud_approver` - Can approve/reject fraud reports
4. `fraud_reviewer` - Can view fraud reports
5. `verifier` - Can verify MM applications
6. `reviewer` - Can review MM applications

---

## Proposed Additional Roles (15+)

### 1. Security & Compliance Roles

#### `security_officer`
**Purpose**: Monitor security threats, manage security policies
**Permissions**:
- `view_security_logs` - View all security events
- `manage_security_policies` - Update security settings
- `block_addresses` - Block suspicious addresses
- `unblock_addresses` - Unblock addresses after review
- `view_threat_intelligence` - Access threat data
- `manage_ip_blacklist` - Block/unblock IP addresses
- `view_failed_login_attempts` - Monitor login security

#### `compliance_officer`
**Purpose**: Ensure regulatory compliance, manage KYC/AML
**Permissions**:
- `view_kyc_documents` - Access KYC documents
- `approve_kyc` - Approve KYC verification
- `reject_kyc` - Reject KYC verification
- `generate_compliance_reports` - Create regulatory reports
- `manage_aml_rules` - Configure AML detection rules
- `flag_suspicious_activity` - Mark suspicious transactions
- `export_compliance_data` - Export data for regulators

#### `risk_analyst`
**Purpose**: Analyze risk patterns, fraud trends
**Permissions**:
- `view_fraud_analytics` - Access fraud statistics
- `view_risk_scores` - See risk assessments
- `generate_risk_reports` - Create risk analysis reports
- `view_transaction_patterns` - Analyze transaction behavior
- `configure_risk_thresholds` - Set risk parameters
- `view_all_alerts` - See all system alerts

### 2. Financial & Trading Roles

#### `financial_auditor`
**Purpose**: Audit financial transactions, verify balances
**Permissions**:
- `view_all_transactions` - Access all transaction history
- `view_all_balances` - See all wallet balances
- `generate_financial_reports` - Create financial reports
- `audit_market_maker_trades` - Review MM trading activity
- `verify_reserves` - Check reserve balances
- `export_financial_data` - Export financial records

#### `trading_supervisor`
**Purpose**: Monitor trading activity, detect manipulation
**Permissions**:
- `view_trading_activity` - Monitor all trades
- `suspend_trading` - Halt trading for investigation
- `resume_trading` - Resume trading after review
- `investigate_manipulation` - Review suspicious trading
- `view_order_books` - Access order book data
- `manage_trading_limits` - Set trading restrictions

#### `market_maker_manager`
**Purpose**: Manage market maker operations
**Permissions**:
- `view_all_mm_applications` - See all MM applications
- `approve_mm_applications` - Approve MM applications
- `reject_mm_applications` - Reject MM applications
- `suspend_market_maker` - Suspend MM privileges
- `reinstate_market_maker` - Restore MM privileges
- `view_mm_performance` - Monitor MM metrics
- `manage_mm_limits` - Set MM trading limits

### 3. Technical & Operations Roles

#### `system_administrator`
**Purpose**: Manage system infrastructure, configurations
**Permissions**:
- `manage_nodes` - Add/remove blockchain nodes
- `configure_consensus` - Adjust consensus parameters
- `manage_network` - Configure network settings
- `view_system_health` - Monitor system status
- `restart_services` - Restart system components
- `manage_backups` - Configure backup systems
- `update_software` - Deploy system updates

#### `database_administrator`
**Purpose**: Manage database, data integrity
**Permissions**:
- `manage_database` - Database administration
- `backup_database` - Create database backups
- `restore_database` - Restore from backups
- `optimize_database` - Performance tuning
- `view_database_logs` - Access database logs
- `manage_data_retention` - Configure data policies

#### `api_manager`
**Purpose**: Manage API access, rate limits
**Permissions**:
- `manage_api_keys` - Create/revoke API keys
- `configure_rate_limits` - Set API rate limits
- `view_api_usage` - Monitor API usage
- `block_api_access` - Block abusive API users
- `manage_api_endpoints` - Enable/disable endpoints
- `view_api_logs` - Access API request logs

### 4. Support & Customer Service Roles

#### `customer_support_lead`
**Purpose**: Handle escalated customer issues
**Permissions**:
- `view_user_accounts` - Access user account info
- `unlock_accounts` - Unlock frozen accounts
- `reset_passwords` - Reset user passwords
- `view_support_tickets` - Access all tickets
- `escalate_issues` - Escalate to higher level
- `refund_transactions` - Process refunds (if applicable)
- `view_user_activity` - See user transaction history

#### `customer_support_agent`
**Purpose**: Handle basic customer inquiries
**Permissions**:
- `view_assigned_tickets` - See assigned tickets only
- `respond_to_tickets` - Reply to support tickets
- `view_basic_user_info` - Limited user data access
- `create_support_notes` - Add notes to tickets
- `escalate_to_lead` - Escalate complex issues

### 5. Content & Communication Roles

#### `content_moderator`
**Purpose**: Moderate user-generated content, comments
**Permissions**:
- `view_user_content` - Access user posts/comments
- `remove_content` - Delete inappropriate content
- `ban_users` - Ban users for violations
- `unban_users` - Unban users after review
- `manage_content_policies` - Update content rules
- `view_moderation_queue` - See flagged content

#### `communications_manager`
**Purpose**: Manage announcements, notifications
**Permissions**:
- `create_announcements` - Post system announcements
- `send_notifications` - Send user notifications
- `manage_email_templates` - Update email templates
- `view_notification_logs` - See notification history
- `manage_communication_channels` - Configure channels

### 6. Blockchain-Specific Roles

#### `validator_manager`
**Purpose**: Manage validator nodes, staking
**Permissions**:
- `approve_validators` - Approve new validators
- `remove_validators` - Remove misbehaving validators
- `view_validator_performance` - Monitor validator metrics
- `manage_staking_pools` - Configure staking pools
- `adjust_validator_rewards` - Modify reward parameters
- `slash_validators` - Penalize bad validators

#### `governance_admin`
**Purpose**: Manage on-chain governance, proposals
**Permissions**:
- `create_proposals` - Create governance proposals
- `approve_proposals` - Approve proposals for voting
- `reject_proposals` - Reject invalid proposals
- `view_voting_results` - See voting outcomes
- `execute_proposals` - Execute approved proposals
- `manage_voting_parameters` - Configure voting rules

#### `token_manager`
**Purpose**: Manage tokens, token economics
**Permissions**:
- `create_tokens` - Create new tokens
- `mint_tokens` - Mint additional tokens (if allowed)
- `burn_tokens` - Burn tokens from circulation
- `manage_token_policies` - Configure token rules
- `view_token_metrics` - Monitor token statistics
- `freeze_tokens` - Freeze tokens in addresses

### 7. Audit & Reporting Roles

#### `internal_auditor`
**Purpose**: Conduct internal audits, compliance checks
**Permissions**:
- `view_all_audit_logs` - Access complete audit trail
- `generate_audit_reports` - Create audit reports
- `view_admin_actions` - See all admin activities
- `export_audit_data` - Export audit logs
- `configure_audit_rules` - Set audit parameters
- `view_system_changes` - Track configuration changes

#### `external_auditor`
**Purpose**: Limited access for external auditors
**Permissions**:
- `view_financial_records` - Read-only financial data
- `view_compliance_reports` - Access compliance docs
- `export_audit_data` - Export specific data sets
- `view_transaction_history` - Read-only transaction data
- **Note**: No write permissions, read-only access

#### `report_analyst`
**Purpose**: Generate business intelligence reports
**Permissions**:
- `generate_reports` - Create various reports
- `view_analytics_dashboard` - Access BI dashboard
- `export_report_data` - Export report data
- `schedule_reports` - Set up automated reports
- `view_historical_data` - Access historical metrics

---

## Granular Permissions List (100+)

### Admin Management (10)
- `create_admin` - Create new admin users
- `update_admin` - Modify admin details
- `delete_admin` - Remove admin users
- `view_admins` - List all admins
- `assign_permissions` - Grant permissions
- `revoke_permissions` - Remove permissions
- `change_admin_role` - Modify admin roles
- `reset_admin_password` - Reset admin passwords
- `activate_admin` - Activate admin accounts
- `deactivate_admin` - Deactivate admin accounts

### Fraud Detection (15)
- `view_fraud_reports` - See fraud reports
- `create_fraud_report` - Submit fraud reports
- `approve_fraud_report` - Approve reports
- `reject_fraud_report` - Reject reports
- `assign_fraud_report` - Assign to reviewers
- `view_fraud_statistics` - See fraud metrics
- `mark_transaction_stolen` - Flag stolen transactions
- `unmark_transaction_stolen` - Unflag transactions
- `view_taint_scores` - See taint calculations
- `configure_fraud_rules` - Modify detection rules
- `view_fraud_alerts` - See fraud alerts
- `dismiss_fraud_alerts` - Dismiss false positives
- `flag_address` - Flag suspicious addresses
- `unflag_address` - Unflag addresses
- `manage_clean_zones` - Register exchanges/pools

### Market Maker Management (12)
- `view_mm_applications` - See MM applications
- `approve_mm_application` - Approve applications
- `reject_mm_application` - Reject applications
- `verify_mm_license` - Verify licenses
- `verify_mm_financial` - Verify financials
- `verify_mm_technical` - Verify tech capabilities
- `verify_mm_kyc` - Verify KYC/AML
- `view_mm_documents` - Access documents
- `download_mm_documents` - Download documents
- `suspend_market_maker` - Suspend MM
- `reinstate_market_maker` - Reinstate MM
- `view_mm_performance` - Monitor MM metrics

### Security & Compliance (20)
- `view_security_logs` - Access security logs
- `manage_security_policies` - Update security
- `block_address` - Block addresses
- `unblock_address` - Unblock addresses
- `block_ip` - Block IP addresses
- `unblock_ip` - Unblock IP addresses
- `view_kyc_documents` - Access KYC docs
- `approve_kyc` - Approve KYC
- `reject_kyc` - Reject KYC
- `view_aml_alerts` - See AML alerts
- `configure_aml_rules` - Set AML rules
- `generate_compliance_report` - Create reports
- `export_compliance_data` - Export data
- `view_threat_intelligence` - Access threat data
- `manage_sanctions_list` - Update sanctions
- `view_failed_logins` - Monitor login attempts
- `reset_user_password` - Reset passwords
- `unlock_user_account` - Unlock accounts
- `freeze_user_account` - Freeze accounts
- `unfreeze_user_account` - Unfreeze accounts

### Financial Operations (15)
- `view_all_transactions` - See all transactions
- `view_all_balances` - See all balances
- `view_transaction_details` - Detailed transaction info
- `generate_financial_report` - Create financial reports
- `export_financial_data` - Export financial data
- `audit_transactions` - Audit transaction history
- `verify_reserves` - Check reserve balances
- `manage_fees` - Configure transaction fees
- `view_fee_revenue` - See fee income
- `process_refund` - Issue refunds
- `reverse_transaction` - Reverse transactions (if allowed)
- `view_wallet_balances` - See wallet balances
- `freeze_funds` - Freeze funds in wallet
- `unfreeze_funds` - Unfreeze funds
- `transfer_funds` - Admin fund transfers

### Trading & Market Operations (12)
- `view_trading_activity` - Monitor trades
- `view_order_books` - Access order books
- `suspend_trading` - Halt trading
- `resume_trading` - Resume trading
- `cancel_orders` - Cancel user orders
- `investigate_manipulation` - Review suspicious trades
- `manage_trading_limits` - Set trading limits
- `view_market_data` - Access market data
- `configure_trading_pairs` - Add/remove pairs
- `manage_liquidity` - Manage liquidity pools
- `view_trading_fees` - See trading fees
- `adjust_trading_fees` - Modify fee structure

### System Administration (18)
- `manage_nodes` - Manage blockchain nodes
- `configure_consensus` - Adjust consensus
- `manage_network` - Configure network
- `view_system_health` - Monitor system
- `restart_services` - Restart components
- `manage_backups` - Configure backups
- `restore_backup` - Restore from backup
- `update_software` - Deploy updates
- `view_system_logs` - Access system logs
- `configure_logging` - Set log levels
- `manage_database` - Database admin
- `optimize_database` - Database tuning
- `manage_cache` - Configure caching
- `clear_cache` - Clear cache
- `manage_api_keys` - API key management
- `configure_rate_limits` - Set rate limits
- `view_api_usage` - Monitor API usage
- `manage_api_endpoints` - Enable/disable APIs

### Blockchain Operations (10)
- `approve_validator` - Approve validators
- `remove_validator` - Remove validators
- `view_validator_performance` - Monitor validators
- `manage_staking_pools` - Configure staking
- `slash_validator` - Penalize validators
- `create_proposal` - Create governance proposals
- `approve_proposal` - Approve proposals
- `execute_proposal` - Execute proposals
- `manage_tokens` - Token management
- `mint_tokens` - Mint new tokens

### Audit & Reporting (10)
- `view_audit_logs` - Access audit logs
- `generate_audit_report` - Create audit reports
- `export_audit_data` - Export audit data
- `view_admin_actions` - See admin activities
- `generate_analytics_report` - Create analytics
- `view_analytics_dashboard` - Access BI dashboard
- `export_report_data` - Export reports
- `schedule_reports` - Automate reports
- `view_historical_data` - Access historical data
- `configure_audit_rules` - Set audit parameters

---

## Permission Hierarchy

```
super_admin (ALL PERMISSIONS)
├── security_officer
│   ├── compliance_officer
│   │   └── risk_analyst
│   └── fraud_admin
│       ├── fraud_approver
│       │   └── fraud_reviewer
│       └── security_analyst
│
├── financial_auditor
│   ├── trading_supervisor
│   │   └── market_maker_manager
│   │       ├── verifier
│   │       └── reviewer
│   └── report_analyst
│
├── system_administrator
│   ├── database_administrator
│   ├── api_manager
│   └── validator_manager
│
├── governance_admin
│   ├── token_manager
│   └── proposal_manager
│
└── customer_support_lead
    ├── customer_support_agent
    └── content_moderator
```

---

## Role Templates

### Template 1: Security Team
```json
{
  "roles": [
    {
      "name": "security_officer",
      "permissions": [
        "view_security_logs",
        "manage_security_policies",
        "block_addresses",
        "unblock_addresses",
        "view_threat_intelligence",
        "manage_ip_blacklist",
        "view_failed_login_attempts"
      ]
    },
    {
      "name": "compliance_officer",
      "permissions": [
        "view_kyc_documents",
        "approve_kyc",
        "reject_kyc",
        "generate_compliance_reports",
        "manage_aml_rules",
        "flag_suspicious_activity",
        "export_compliance_data"
      ]
    }
  ]
}
```

### Template 2: Financial Team
```json
{
  "roles": [
    {
      "name": "financial_auditor",
      "permissions": [
        "view_all_transactions",
        "view_all_balances",
        "generate_financial_reports",
        "audit_market_maker_trades",
        "verify_reserves",
        "export_financial_data"
      ]
    },
    {
      "name": "trading_supervisor",
      "permissions": [
        "view_trading_activity",
        "suspend_trading",
        "resume_trading",
        "investigate_manipulation",
        "view_order_books",
        "manage_trading_limits"
      ]
    }
  ]
}
```

### Template 3: Operations Team
```json
{
  "roles": [
    {
      "name": "system_administrator",
      "permissions": [
        "manage_nodes",
        "configure_consensus",
        "manage_network",
        "view_system_health",
        "restart_services",
        "manage_backups",
        "update_software"
      ]
    },
    {
      "name": "database_administrator",
      "permissions": [
        "manage_database",
        "backup_database",
        "restore_database",
        "optimize_database",
        "view_database_logs",
        "manage_data_retention"
      ]
    }
  ]
}
```

---

## Implementation in Code

### Add to MarketMakerAdmin.h

```cpp
// Enhanced role definitions
enum class AdminRole {
    SUPER_ADMIN,
    
    // Security & Compliance
    SECURITY_OFFICER,
    COMPLIANCE_OFFICER,
    RISK_ANALYST,
    
    // Fraud Detection
    FRAUD_ADMIN,
    FRAUD_APPROVER,
    FRAUD_REVIEWER,
    
    // Financial
    FINANCIAL_AUDITOR,
    TRADING_SUPERVISOR,
    MARKET_MAKER_MANAGER,
    
    // Technical
    SYSTEM_ADMINISTRATOR,
    DATABASE_ADMINISTRATOR,
    API_MANAGER,
    
    // Support
    CUSTOMER_SUPPORT_LEAD,
    CUSTOMER_SUPPORT_AGENT,
    CONTENT_MODERATOR,
    
    // Blockchain
    VALIDATOR_MANAGER,
    GOVERNANCE_ADMIN,
    TOKEN_MANAGER,
    
    // Audit
    INTERNAL_AUDITOR,
    EXTERNAL_AUDITOR,
    REPORT_ANALYST,
    
    // Market Maker
    VERIFIER,
    REVIEWER
};

// Permission categories
namespace Permissions {
    // Admin Management
    const std::string CREATE_ADMIN = "create_admin";
    const std::string UPDATE_ADMIN = "update_admin";
    const std::string DELETE_ADMIN = "delete_admin";
    const std::string VIEW_ADMINS = "view_admins";
    const std::string ASSIGN_PERMISSIONS = "assign_permissions";
    
    // Fraud Detection
    const std::string VIEW_FRAUD_REPORTS = "view_fraud_reports";
    const std::string APPROVE_FRAUD_REPORT = "approve_fraud_report";
    const std::string REJECT_FRAUD_REPORT = "reject_fraud_report";
    const std::string MARK_TRANSACTION_STOLEN = "mark_transaction_stolen";
    const std::string FLAG_ADDRESS = "flag_address";
    
    // Market Maker
    const std::string VIEW_MM_APPLICATIONS = "view_mm_applications";
    const std::string APPROVE_MM_APPLICATION = "approve_mm_application";
    const std::string VERIFY_MM_LICENSE = "verify_mm_license";
    const std::string VIEW_MM_DOCUMENTS = "view_mm_documents";
    
    // Security
    const std::string VIEW_SECURITY_LOGS = "view_security_logs";
    const std::string BLOCK_ADDRESS = "block_address";
    const std::string MANAGE_SECURITY_POLICIES = "manage_security_policies";
    
    // Financial
    const std::string VIEW_ALL_TRANSACTIONS = "view_all_transactions";
    const std::string VIEW_ALL_BALANCES = "view_all_balances";
    const std::string GENERATE_FINANCIAL_REPORT = "generate_financial_report";
    
    // System
    const std::string MANAGE_NODES = "manage_nodes";
    const std::string CONFIGURE_CONSENSUS = "configure_consensus";
    const std::string RESTART_SERVICES = "restart_services";
    
    // And many more...
}

// Role permission mappings
std::vector<std::string> getRolePermissions(AdminRole role) {
    switch (role) {
        case AdminRole::SUPER_ADMIN:
            return {"all"};
            
        case AdminRole::SECURITY_OFFICER:
            return {
                Permissions::VIEW_SECURITY_LOGS,
                Permissions::MANAGE_SECURITY_POLICIES,
                Permissions::BLOCK_ADDRESS,
                Permissions::VIEW_THREAT_INTELLIGENCE
            };
            
        case AdminRole::FRAUD_ADMIN:
            return {
                Permissions::VIEW_FRAUD_REPORTS,
                Permissions::APPROVE_FRAUD_REPORT,
                Permissions::REJECT_FRAUD_REPORT,
                Permissions::MARK_TRANSACTION_STOLEN,
                Permissions::FLAG_ADDRESS,
                Permissions::ASSIGN_PERMISSIONS
            };
            
        // Add all other roles...
    }
}
```

---

## Benefits of Enhanced Permissions

### 1. **Separation of Duties**
- No single person has too much power
- Reduces insider threat risk
- Meets compliance requirements

### 2. **Principle of Least Privilege**
- Users only get permissions they need
- Reduces attack surface
- Limits damage from compromised accounts

### 3. **Audit Trail**
- Every action tied to specific permission
- Easy to track who did what
- Compliance-ready audit logs

### 4. **Scalability**
- Easy to add new team members
- Clear role definitions
- Flexible permission assignments

### 5. **Compliance**
- Meets SOC 2 requirements
- Satisfies regulatory audits
- Demonstrates security controls

---

## Recommended Implementation Priority

### Phase 1: Critical Security (Immediate)
1. `security_officer` - Security monitoring
2. `compliance_officer` - Regulatory compliance
3. `risk_analyst` - Risk assessment

### Phase 2: Financial Operations (Week 1)
4. `financial_auditor` - Financial oversight
5. `trading_supervisor` - Trading monitoring
6. `market_maker_manager` - MM management

### Phase 3: Technical Operations (Week 2)
7. `system_administrator` - System management
8. `database_administrator` - Database management
9. `api_manager` - API management

### Phase 4: Support & Governance (Week 3)
10. `customer_support_lead` - Customer support
11. `validator_manager` - Validator management
12. `governance_admin` - Governance

### Phase 5: Audit & Reporting (Week 4)
13. `internal_auditor` - Internal audits
14. `external_auditor` - External audits
15. `report_analyst` - Business intelligence

---

## Summary

### Current System
- 6 roles
- ~20 permissions

### Enhanced System
- 25+ roles
- 100+ granular permissions
- Clear hierarchy
- Role templates
- Compliance-ready

### Benefits
- Better security
- Regulatory compliance
- Scalable team management
- Clear audit trail
- Flexible access control

---

**Recommendation**: Implement Phase 1 (Security roles) immediately, then add other phases based on business needs.
