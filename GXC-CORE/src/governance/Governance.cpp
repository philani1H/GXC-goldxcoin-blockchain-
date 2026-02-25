#include "../../include/Governance.h"
#include "../../include/Logger.h"
#include "../../include/Utils.h"
#include <algorithm>
#include <numeric>
#include <mutex>
#include <random>

static std::mutex proposalsMutex;

GovernanceSystem::GovernanceSystem() {
    LOG_CORE(LogLevel::INFO, "Governance system initialized");
}

std::string GovernanceSystem::submitProposal(const std::string& proposer, const std::string& title,
                                              const std::string& description, const std::vector<ParameterChange>& changes) {
    Proposal proposal(proposer, title, description, changes);
    
    if (!isValidProposal(proposal)) {
        LOG_CORE(LogLevel::ERROR, "Invalid proposal submitted: " + title);
        return "";
    }
    
    std::lock_guard<std::mutex> lock(proposalsMutex);
    std::string proposalId = proposal.getId();
    proposals[proposalId] = proposal;
    
    LOG_CORE(LogLevel::INFO, "Proposal submitted: " + proposalId + " - " + title);
    return proposalId;
}

bool GovernanceSystem::cancelProposal(const std::string& proposalId, const std::string& canceller) {
    std::lock_guard<std::mutex> lock(proposalsMutex);
    auto it = proposals.find(proposalId);
    if (it != proposals.end()) {
        Proposal& proposal = it->second;
        if (proposal.getProposer() == canceller) {
            proposal.cancel();
            return true;
        }
    }
    return false;
}

void GovernanceSystem::updateProposalStatus(const std::string& proposalId) {
    std::lock_guard<std::mutex> lock(proposalsMutex);
    auto it = proposals.find(proposalId);
    if (it != proposals.end()) {
        Proposal& proposal = it->second;
        proposal.updateStatus();
    }
}

bool GovernanceSystem::castVote(const std::string& voter, const std::string& proposalId, VoteType voteType, 
                                 double stakeAmount, uint32_t stakingDays) {
    std::lock_guard<std::mutex> lock(proposalsMutex);
    
    auto it = proposals.find(proposalId);
    if (it == proposals.end()) {
        LOG_CORE(LogLevel::ERROR, "Proposal not found: " + proposalId);
        return false;
    }
    
    Proposal& proposal = it->second;
    
    if (!proposal.isVotingOpen()) {
        LOG_CORE(LogLevel::ERROR, "Voting not open for proposal: " + proposalId);
        return false;
    }
    
    if (proposal.hasVoted(voter)) {
        LOG_CORE(LogLevel::ERROR, "Voter already voted: " + voter);
        return false;
    }
    
    double voteWeight = calculateVoteWeight(stakeAmount, stakingDays);
    
    Vote vote;
    vote.voter = voter;
    vote.proposalId = proposalId;
    vote.voteType = voteType;
    vote.voteWeight = voteWeight;
    vote.timestamp = std::time(nullptr);
    
    if (proposal.addVote(vote)) {
        proposalVotes[proposalId].push_back(vote);
        voterLastActivity[voter] = std::time(nullptr);
        return true;
    }
    
    return false;
}

void GovernanceSystem::tallyVotes(const std::string& proposalId) {
    std::lock_guard<std::mutex> lock(proposalsMutex);
    auto it = proposals.find(proposalId);
    if (it != proposals.end()) {
        Proposal& proposal = it->second;
        proposal.tallyVotes();
    }
}

bool GovernanceSystem::executeProposal(const std::string& proposalId) {
    std::lock_guard<std::mutex> lock(proposalsMutex);
    
    auto it = proposals.find(proposalId);
    if (it == proposals.end()) {
        LOG_CORE(LogLevel::ERROR, "Proposal not found: " + proposalId);
        return false;
    }
    
    Proposal& proposal = it->second;
    
    if (!proposal.canExecute()) {
        LOG_CORE(LogLevel::ERROR, "Proposal cannot be executed: " + proposalId);
        return false;
    }
    
    applyParameterChanges(proposal.getParameterChanges());
    proposal.markExecuted("exec_tx_" + proposalId);
    
    LOG_CORE(LogLevel::INFO, "Proposal executed: " + proposalId);
    return true;
}

void GovernanceSystem::applyParameterChanges(const std::vector<ParameterChange>& changes) {
    for (const auto& change : changes) {
        setParameter(change.paramType, change.newValue);
        LOG_CORE(LogLevel::INFO, "Applied parameter change: " + std::to_string(static_cast<int>(change.paramType)) + 
                                  " = " + std::to_string(change.newValue));
    }
}

double GovernanceSystem::getParameter(ParameterType paramType) const {
    auto it = parameters.find(paramType);
    if (it != parameters.end()) {
        return it->second;
    }
    return 0.0;
}

void GovernanceSystem::setParameter(ParameterType paramType, double value) {
    parameters[paramType] = value;
}

std::vector<ParameterChange> GovernanceSystem::getPendingChanges() const {
    // Return pending changes from active proposals
    std::vector<ParameterChange> changes;
    for (const auto& pair : proposals) {
        const Proposal& proposal = pair.second;
        if (proposal.getStatus() == ProposalStatus::PASSED && !proposal.isExecuted()) {
            const auto& proposalChanges = proposal.getParameterChanges();
            changes.insert(changes.end(), proposalChanges.begin(), proposalChanges.end());
        }
    }
    return changes;
}

std::vector<Proposal> GovernanceSystem::getActiveProposals() const {
    std::lock_guard<std::mutex> lock(proposalsMutex);
    std::vector<Proposal> active;
    for (const auto& pair : proposals) {
        if (pair.second.getStatus() == ProposalStatus::ACTIVE) {
            active.push_back(pair.second);
        }
    }
    return active;
}

std::vector<Proposal> GovernanceSystem::getProposalHistory() const {
    std::lock_guard<std::mutex> lock(proposalsMutex);
    std::vector<Proposal> history;
    for (const auto& pair : proposals) {
        history.push_back(pair.second);
    }
    return history;
}

Proposal GovernanceSystem::getProposal(const std::string& proposalId) const {
    std::lock_guard<std::mutex> lock(proposalsMutex);
    auto it = proposals.find(proposalId);
    if (it != proposals.end()) {
        return it->second;
    }
    throw std::runtime_error("Proposal not found: " + proposalId);
}

std::vector<Vote> GovernanceSystem::getProposalVotes(const std::string& proposalId) const {
    std::lock_guard<std::mutex> lock(proposalsMutex);
    auto it = proposalVotes.find(proposalId);
    if (it != proposalVotes.end()) {
        return it->second;
    }
    return std::vector<Vote>();
}

void GovernanceSystem::authorizeProposer(const std::string& address) {
    authorizedProposers[address] = true;
}

void GovernanceSystem::revokeProposer(const std::string& address) {
    authorizedProposers.erase(address);
}

bool GovernanceSystem::isAuthorizedProposer(const std::string& address) const {
    auto it = authorizedProposers.find(address);
    return it != authorizedProposers.end() && it->second;
}

uint32_t GovernanceSystem::getTotalProposals() const {
    std::lock_guard<std::mutex> lock(proposalsMutex);
    return static_cast<uint32_t>(proposals.size());
}

uint32_t GovernanceSystem::getActiveProposalCount() const {
    std::lock_guard<std::mutex> lock(proposalsMutex);
    uint32_t count = 0;
    for (const auto& pair : proposals) {
        if (pair.second.getStatus() == ProposalStatus::ACTIVE) {
            count++;
        }
    }
    return count;
}

double GovernanceSystem::getAverageParticipation() const {
    std::lock_guard<std::mutex> lock(proposalsMutex);
    if (proposals.empty()) return 0.0;
    
    double total = 0.0;
    for (const auto& pair : proposals) {
        total += pair.second.getTotalVotes();
    }
    return total / proposals.size();
}

bool GovernanceSystem::isValidParameterChange(const ParameterChange& change) const {
    // Basic validation
    if (change.newValue < 0) return false;
    
    // Type-specific validation could be added here
    return true;
}

bool GovernanceSystem::isValidProposal(const Proposal& proposal) const {
    if (proposal.getTitle().empty() || proposal.getDescription().empty()) {
        return false;
    }
    
    if (proposal.getProposer().empty()) {
        return false;
    }
    
    // Validate parameter changes
    for (const auto& change : proposal.getParameterChanges()) {
        if (!isValidParameterChange(change)) {
            return false;
        }
    }
    
    return true;
}

double GovernanceSystem::calculateVoteWeight(double stakeAmount, uint32_t stakingDays) const {
    // Simple formula: base weight from stake, bonus from staking duration
    double baseWeight = stakeAmount;
    double durationBonus = 1.0 + (stakingDays / 365.0) * 0.1; // 10% bonus per year
    return baseWeight * durationBonus;
}
