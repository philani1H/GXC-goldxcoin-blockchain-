#include "../../include/Governance.h"
#include "../../include/Logger.h"
#include "../../include/Utils.h"
#include <algorithm>
#include <numeric>

Governance::Governance(Blockchain* blockchain) 
    : blockchain(blockchain), votingPeriod(7 * 24 * 3600), // 7 days in seconds
      proposalFee(100.0), quorumThreshold(0.6), passThreshold(0.51) {
    LOG_GOVERNANCE(LogLevel::INFO, "Governance system initialized");
}

Governance::~Governance() = default;

bool Governance::submitProposal(const Proposal& proposal) {
    // Validate proposal
    if (!isValidProposal(proposal)) {
        LOG_GOVERNANCE(LogLevel::ERROR, "Invalid proposal submitted: " + proposal.title);
        return false;
    }
    
    // Check if proposer has enough balance for fee
    if (!hasEnoughBalance(proposal.proposer, proposalFee)) {
        LOG_GOVERNANCE(LogLevel::ERROR, "Insufficient balance for proposal fee: " + proposal.proposer);
        return false;
    }
    
    // Create proposal with unique ID
    Proposal newProposal = proposal;
    newProposal.id = generateProposalId();
    newProposal.status = ProposalStatus::ACTIVE;
    newProposal.submitTime = Utils::getCurrentTimestamp();
    newProposal.votingEndTime = newProposal.submitTime + votingPeriod;
    newProposal.totalVotes = 0;
    newProposal.yesVotes = 0;
    newProposal.noVotes = 0;
    
    // Add to proposals map
    {
        std::lock_guard<std::mutex> lock(proposalsMutex);
        proposals[newProposal.id] = newProposal;
    }
    
    // Deduct proposal fee
    deductProposalFee(proposal.proposer, proposalFee);
    
    LOG_GOVERNANCE(LogLevel::INFO, "Proposal submitted: " + newProposal.id + " - " + newProposal.title);
    
    return true;
}

bool Governance::vote(const std::string& proposalId, const std::string& voter, bool support, double weight) {
    std::lock_guard<std::mutex> lock(proposalsMutex);
    
    // Find proposal
    auto it = proposals.find(proposalId);
    if (it == proposals.end()) {
        LOG_GOVERNANCE(LogLevel::ERROR, "Proposal not found: " + proposalId);
        return false;
    }
    
    Proposal& proposal = it->second;
    
    // Check if proposal is active
    if (proposal.status != ProposalStatus::ACTIVE) {
        LOG_GOVERNANCE(LogLevel::ERROR, "Proposal not active for voting: " + proposalId);
        return false;
    }
    
    // Check if voting period has ended
    if (Utils::getCurrentTimestamp() > proposal.votingEndTime) {
        LOG_GOVERNANCE(LogLevel::ERROR, "Voting period ended for proposal: " + proposalId);
        finalizeProposal(proposalId);
        return false;
    }
    
    // Check if voter has already voted
    if (proposal.voters.find(voter) != proposal.voters.end()) {
        LOG_GOVERNANCE(LogLevel::ERROR, "Voter already voted: " + voter);
        return false;
    }
    
    // Validate voter's voting power
    double votingPower = calculateVotingPower(voter);
    if (votingPower <= 0) {
        LOG_GOVERNANCE(LogLevel::ERROR, "No voting power: " + voter);
        return false;
    }
    
    // Use provided weight or calculated voting power
    double actualWeight = (weight > 0) ? std::min(weight, votingPower) : votingPower;
    
    // Record vote
    Vote vote;
    vote.voter = voter;
    vote.support = support;
    vote.weight = actualWeight;
    vote.timestamp = Utils::getCurrentTimestamp();
    
    proposal.votes.push_back(vote);
    proposal.voters.insert(voter);
    proposal.totalVotes += actualWeight;
    
    if (support) {
        proposal.yesVotes += actualWeight;
    } else {
        proposal.noVotes += actualWeight;
    }
    
    LOG_GOVERNANCE(LogLevel::INFO, "Vote recorded for proposal " + proposalId + 
                                  " by " + voter + " (support: " + (support ? "YES" : "NO") + 
                                  ", weight: " + Utils::formatAmount(actualWeight, 2) + ")");
    
    return true;
}

std::vector<Proposal> Governance::getActiveProposals() const {
    std::lock_guard<std::mutex> lock(proposalsMutex);
    
    std::vector<Proposal> activeProposals;
    auto currentTime = Utils::getCurrentTimestamp();
    
    for (const auto& pair : proposals) {
        const Proposal& proposal = pair.second;
        if (proposal.status == ProposalStatus::ACTIVE && currentTime <= proposal.votingEndTime) {
            activeProposals.push_back(proposal);
        }
    }
    
    return activeProposals;
}

std::vector<Proposal> Governance::getProposalsByStatus(ProposalStatus status) const {
    std::lock_guard<std::mutex> lock(proposalsMutex);
    
    std::vector<Proposal> filteredProposals;
    
    for (const auto& pair : proposals) {
        if (pair.second.status == status) {
            filteredProposals.push_back(pair.second);
        }
    }
    
    return filteredProposals;
}

Proposal Governance::getProposal(const std::string& proposalId) const {
    std::lock_guard<std::mutex> lock(proposalsMutex);
    
    auto it = proposals.find(proposalId);
    if (it != proposals.end()) {
        return it->second;
    }
    
    throw std::runtime_error("Proposal not found: " + proposalId);
}

bool Governance::executeProposal(const std::string& proposalId) {
    std::lock_guard<std::mutex> lock(proposalsMutex);
    
    auto it = proposals.find(proposalId);
    if (it == proposals.end()) {
        LOG_GOVERNANCE(LogLevel::ERROR, "Proposal not found for execution: " + proposalId);
        return false;
    }
    
    Proposal& proposal = it->second;
    
    // Check if proposal passed
    if (proposal.status != ProposalStatus::PASSED) {
        LOG_GOVERNANCE(LogLevel::ERROR, "Proposal not in passed status: " + proposalId);
        return false;
    }
    
    try {
        // Execute based on proposal type
        bool executed = false;
        
        switch (proposal.type) {
            case ProposalType::PARAMETER_CHANGE:
                executed = executeParameterChange(proposal);
                break;
                
            case ProposalType::SYSTEM_UPGRADE:
                executed = executeSystemUpgrade(proposal);
                break;
                
            case ProposalType::TREASURY_ALLOCATION:
                executed = executeTreasuryAllocation(proposal);
                break;
                
            case ProposalType::EMERGENCY_ACTION:
                executed = executeEmergencyAction(proposal);
                break;
                
            case ProposalType::CUSTOM:
                executed = executeCustomProposal(proposal);
                break;
                
            default:
                LOG_GOVERNANCE(LogLevel::ERROR, "Unknown proposal type: " + std::to_string(static_cast<int>(proposal.type)));
                return false;
        }
        
        if (executed) {
            proposal.status = ProposalStatus::EXECUTED;
            proposal.executionTime = Utils::getCurrentTimestamp();
            
            LOG_GOVERNANCE(LogLevel::INFO, "Proposal executed successfully: " + proposalId);
            return true;
        } else {
            proposal.status = ProposalStatus::EXECUTION_FAILED;
            LOG_GOVERNANCE(LogLevel::ERROR, "Proposal execution failed: " + proposalId);
            return false;
        }
        
    } catch (const std::exception& e) {
        proposal.status = ProposalStatus::EXECUTION_FAILED;
        LOG_GOVERNANCE(LogLevel::ERROR, "Proposal execution error: " + std::string(e.what()));
        return false;
    }
}

void Governance::finalizeProposal(const std::string& proposalId) {
    auto it = proposals.find(proposalId);
    if (it == proposals.end()) {
        return;
    }
    
    Proposal& proposal = it->second;
    
    if (proposal.status != ProposalStatus::ACTIVE) {
        return;
    }
    
    // Calculate total eligible voting power
    double totalEligibleVotes = calculateTotalVotingPower();
    
    // Check quorum
    double turnout = proposal.totalVotes / totalEligibleVotes;
    if (turnout < quorumThreshold) {
        proposal.status = ProposalStatus::REJECTED;
        proposal.rejectionReason = "Insufficient quorum (turnout: " + 
                                  Utils::formatAmount(turnout * 100, 2) + "%)";
    } else {
        // Check if proposal passed
        double supportRatio = proposal.yesVotes / proposal.totalVotes;
        if (supportRatio >= passThreshold) {
            proposal.status = ProposalStatus::PASSED;
        } else {
            proposal.status = ProposalStatus::REJECTED;
            proposal.rejectionReason = "Insufficient support (support: " + 
                                     Utils::formatAmount(supportRatio * 100, 2) + "%)";
        }
    }
    
    proposal.finalizationTime = Utils::getCurrentTimestamp();
    
    LOG_GOVERNANCE(LogLevel::INFO, "Proposal finalized: " + proposalId + 
                                  " - Status: " + proposalStatusToString(proposal.status));
}

void Governance::updateActiveProposals() {
    std::lock_guard<std::mutex> lock(proposalsMutex);
    
    auto currentTime = Utils::getCurrentTimestamp();
    
    for (auto& pair : proposals) {
        Proposal& proposal = pair.second;
        
        // Finalize expired active proposals
        if (proposal.status == ProposalStatus::ACTIVE && currentTime > proposal.votingEndTime) {
            finalizeProposal(pair.first);
        }
    }
}

GovernanceStats Governance::getStats() const {
    std::lock_guard<std::mutex> lock(proposalsMutex);
    
    GovernanceStats stats;
    stats.totalProposals = proposals.size();
    stats.activeProposals = 0;
    stats.passedProposals = 0;
    stats.rejectedProposals = 0;
    stats.executedProposals = 0;
    stats.totalVoters = 0;
    
    std::set<std::string> uniqueVoters;
    
    for (const auto& pair : proposals) {
        const Proposal& proposal = pair.second;
        
        switch (proposal.status) {
            case ProposalStatus::ACTIVE:
                stats.activeProposals++;
                break;
            case ProposalStatus::PASSED:
                stats.passedProposals++;
                break;
            case ProposalStatus::REJECTED:
                stats.rejectedProposals++;
                break;
            case ProposalStatus::EXECUTED:
                stats.executedProposals++;
                break;
            default:
                break;
        }
        
        // Count unique voters
        for (const auto& voter : proposal.voters) {
            uniqueVoters.insert(voter);
        }
    }
    
    stats.totalVoters = uniqueVoters.size();
    
    return stats;
}

bool Governance::isValidProposal(const Proposal& proposal) const {
    // Basic validation
    if (proposal.title.empty() || proposal.description.empty()) {
        return false;
    }
    
    if (proposal.title.length() > 200 || proposal.description.length() > 5000) {
        return false;
    }
    
    if (proposal.proposer.empty()) {
        return false;
    }
    
    // Type-specific validation
    switch (proposal.type) {
        case ProposalType::PARAMETER_CHANGE:
            return validateParameterChangeProposal(proposal);
            
        case ProposalType::SYSTEM_UPGRADE:
            return validateSystemUpgradeProposal(proposal);
            
        case ProposalType::TREASURY_ALLOCATION:
            return validateTreasuryAllocationProposal(proposal);
            
        case ProposalType::EMERGENCY_ACTION:
            return validateEmergencyActionProposal(proposal);
            
        case ProposalType::CUSTOM:
            return validateCustomProposal(proposal);
            
        default:
            return false;
    }
}

double Governance::calculateVotingPower(const std::string& address) const {
    // In a real implementation, voting power could be based on:
    // 1. Token holdings
    // 2. Staking duration
    // 3. Historical participation
    // 4. Delegation
    
    // Simplified implementation: use token balance
    double balance = getAddressBalance(address);
    
    // Apply voting power formula (e.g., square root to reduce whale influence)
    return std::sqrt(balance);
}

double Governance::calculateTotalVotingPower() const {
    // Calculate total voting power of all eligible voters
    // In a real implementation, would iterate through all accounts
    return 1000000.0; // Simplified
}

bool Governance::hasEnoughBalance(const std::string& address, double amount) const {
    double balance = getAddressBalance(address);
    return balance >= amount;
}

double Governance::getAddressBalance(const std::string& address) const {
    // In a real implementation, would query the blockchain for balance
    // Simplified implementation
    return 1000.0;
}

void Governance::deductProposalFee(const std::string& address, double amount) {
    // In a real implementation, would create a transaction to deduct the fee
    LOG_GOVERNANCE(LogLevel::INFO, "Deducted proposal fee from " + address + 
                                  ": " + Utils::formatAmount(amount, 2) + " GXC");
}

std::string Governance::generateProposalId() {
    static uint64_t counter = 0;
    auto timestamp = Utils::getCurrentTimestamp();
    return "prop_" + std::to_string(timestamp) + "_" + std::to_string(++counter);
}

bool Governance::validateParameterChangeProposal(const Proposal& proposal) const {
    // Validate parameter change proposals
    if (proposal.parameters.empty()) {
        return false;
    }
    
    // Check for valid parameter names and values
    for (const auto& param : proposal.parameters) {
        if (param.first.empty() || param.second.empty()) {
            return false;
        }
        
        // Validate specific parameters
        if (param.first == "block_reward" || param.first == "difficulty_adjustment") {
            try {
                double value = std::stod(param.second);
                if (value <= 0) {
                    return false;
                }
            } catch (...) {
                return false;
            }
        }
    }
    
    return true;
}

bool Governance::validateSystemUpgradeProposal(const Proposal& proposal) const {
    // Validate system upgrade proposals
    auto it = proposal.parameters.find("version");
    if (it == proposal.parameters.end()) {
        return false;
    }
    
    // Check version format (e.g., "1.2.3")
    std::string version = it->second;
    return !version.empty() && version.find('.') != std::string::npos;
}

bool Governance::validateTreasuryAllocationProposal(const Proposal& proposal) const {
    // Validate treasury allocation proposals
    auto amountIt = proposal.parameters.find("amount");
    auto recipientIt = proposal.parameters.find("recipient");
    
    if (amountIt == proposal.parameters.end() || recipientIt == proposal.parameters.end()) {
        return false;
    }
    
    try {
        double amount = std::stod(amountIt->second);
        if (amount <= 0) {
            return false;
        }
    } catch (...) {
        return false;
    }
    
    return !recipientIt->second.empty();
}

bool Governance::validateEmergencyActionProposal(const Proposal& proposal) const {
    // Emergency actions require special validation
    auto actionIt = proposal.parameters.find("action");
    if (actionIt == proposal.parameters.end()) {
        return false;
    }
    
    std::string action = actionIt->second;
    return action == "pause_network" || action == "resume_network" || 
           action == "emergency_patch" || action == "freeze_assets";
}

bool Governance::validateCustomProposal(const Proposal& proposal) const {
    // Custom proposals have minimal validation
    return !proposal.description.empty();
}

bool Governance::executeParameterChange(const Proposal& proposal) {
    LOG_GOVERNANCE(LogLevel::INFO, "Executing parameter change proposal: " + proposal.id);
    
    // Apply parameter changes to the system
    for (const auto& param : proposal.parameters) {
        applyParameterChange(param.first, param.second);
    }
    
    return true;
}

bool Governance::executeSystemUpgrade(const Proposal& proposal) {
    LOG_GOVERNANCE(LogLevel::INFO, "Executing system upgrade proposal: " + proposal.id);
    
    auto versionIt = proposal.parameters.find("version");
    if (versionIt != proposal.parameters.end()) {
        // Schedule system upgrade
        scheduleSystemUpgrade(versionIt->second);
    }
    
    return true;
}

bool Governance::executeTreasuryAllocation(const Proposal& proposal) {
    LOG_GOVERNANCE(LogLevel::INFO, "Executing treasury allocation proposal: " + proposal.id);
    
    auto amountIt = proposal.parameters.find("amount");
    auto recipientIt = proposal.parameters.find("recipient");
    
    if (amountIt != proposal.parameters.end() && recipientIt != proposal.parameters.end()) {
        double amount = std::stod(amountIt->second);
        std::string recipient = recipientIt->second;
        
        // Transfer funds from treasury
        return transferFromTreasury(recipient, amount);
    }
    
    return false;
}

bool Governance::executeEmergencyAction(const Proposal& proposal) {
    LOG_GOVERNANCE(LogLevel::INFO, "Executing emergency action proposal: " + proposal.id);
    
    auto actionIt = proposal.parameters.find("action");
    if (actionIt != proposal.parameters.end()) {
        return executeEmergencyCommand(actionIt->second);
    }
    
    return false;
}

bool Governance::executeCustomProposal(const Proposal& proposal) {
    LOG_GOVERNANCE(LogLevel::INFO, "Executing custom proposal: " + proposal.id);
    
    // Custom proposals require manual implementation
    // This would typically involve calling external systems or contracts
    
    return true; // Simplified
}

void Governance::applyParameterChange(const std::string& parameter, const std::string& value) {
    LOG_GOVERNANCE(LogLevel::INFO, "Applying parameter change: " + parameter + " = " + value);
    
    // Apply changes to blockchain parameters
    if (parameter == "block_reward") {
        blockchain->setBlockReward(std::stod(value));
    } else if (parameter == "difficulty_adjustment") {
        // Apply difficulty adjustment parameter
    }
    // Add more parameter handlers as needed
}

void Governance::scheduleSystemUpgrade(const std::string& version) {
    LOG_GOVERNANCE(LogLevel::INFO, "Scheduled system upgrade to version: " + version);
    // Schedule upgrade for next maintenance window
}

bool Governance::transferFromTreasury(const std::string& recipient, double amount) {
    LOG_GOVERNANCE(LogLevel::INFO, "Transferring " + Utils::formatAmount(amount, 2) + 
                                  " GXC from treasury to " + recipient);
    
    // Create treasury transaction
    Transaction treasuryTx("treasury_address", recipient, amount);
    
    // Add to blockchain
    return blockchain->addPendingTransaction(treasuryTx);
}

bool Governance::executeEmergencyCommand(const std::string& command) {
    LOG_GOVERNANCE(LogLevel::INFO, "Executing emergency command: " + command);
    
    if (command == "pause_network") {
        // Pause network operations
        return true;
    } else if (command == "resume_network") {
        // Resume network operations
        return true;
    } else if (command == "emergency_patch") {
        // Apply emergency patch
        return true;
    } else if (command == "freeze_assets") {
        // Freeze specific assets
        return true;
    }
    
    return false;
}

std::string Governance::proposalStatusToString(ProposalStatus status) {
    switch (status) {
        case ProposalStatus::ACTIVE: return "ACTIVE";
        case ProposalStatus::PASSED: return "PASSED";
        case ProposalStatus::REJECTED: return "REJECTED";
        case ProposalStatus::EXECUTED: return "EXECUTED";
        case ProposalStatus::EXECUTION_FAILED: return "EXECUTION_FAILED";
        default: return "UNKNOWN";
    }
}