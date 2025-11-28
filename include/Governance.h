#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <ctime>
#include <cstdint>
#include <mutex>

enum class VoteType {
    FOR,
    AGAINST,
    ABSTAIN
};

enum class ProposalStatus {
    ACTIVE,
    PASSED,
    FAILED,
    EXECUTED,
    CANCELLED
};

enum class ParameterType {
    TARGET_INFLATION_RATE,
    TARGET_PRICE_RATIO,
    ADJUSTMENT_K1,
    ADJUSTMENT_K2,
    ADJUSTMENT_K3,
    BASE_BURN_RATE,
    MIN_STAKE,
    ORACLE_THRESHOLD,
    BRIDGE_THRESHOLD,
    BLOCK_REWARD,
    DIFFICULTY_ADJUSTMENT
};

struct ParameterChange {
    ParameterType paramType;
    double oldValue;
    double newValue;
    std::string description;
};

struct Vote {
    std::string voter;
    std::string proposalId;
    VoteType voteType;
    double voteWeight;
    std::time_t timestamp;
    std::string signature;
};

class Proposal {
private:
    std::string id;
    std::string proposer;
    std::string title;
    std::string description;
    std::vector<ParameterChange> parameterChanges;
    std::time_t createdAt;
    std::time_t votingEndsAt;
    ProposalStatus status;
    
    // Vote tallies
    double forVotes;
    double againstVotes;
    double abstainVotes;
    double totalVotes;
    
    // Thresholds
    double quorumThreshold;
    double passThreshold;
    
    // Execution
    bool executed;
    std::time_t executedAt;
    std::string executionTxHash;
    
public:
    // Constructors
    Proposal();
    Proposal(const std::string& proposerIn, const std::string& titleIn, 
             const std::string& descriptionIn, const std::vector<ParameterChange>& changesIn,
             uint32_t votingPeriodDays = 7);
    
    // Voting
    bool addVote(const Vote& vote);
    bool hasVoted(const std::string& voter) const;
    void tallyVotes();
    
    // Status management
    void updateStatus();
    bool canExecute() const;
    void markExecuted(const std::string& txHash);
    void cancel();
    
    // Validation
    bool isValidProposal() const;
    bool isVotingOpen() const;
    bool hasQuorum() const;
    bool hasPassed() const;
    
    // Getters
    std::string getId() const { return id; }
    std::string getProposer() const { return proposer; }
    std::string getTitle() const { return title; }
    std::string getDescription() const { return description; }
    const std::vector<ParameterChange>& getParameterChanges() const { return parameterChanges; }
    std::time_t getCreatedAt() const { return createdAt; }
    std::time_t getVotingEndsAt() const { return votingEndsAt; }
    ProposalStatus getStatus() const { return status; }
    double getForVotes() const { return forVotes; }
    double getAgainstVotes() const { return againstVotes; }
    double getAbstainVotes() const { return abstainVotes; }
    double getTotalVotes() const { return totalVotes; }
    double getQuorumThreshold() const { return quorumThreshold; }
    double getPassThreshold() const { return passThreshold; }
    bool isExecuted() const { return executed; }
    std::time_t getExecutedAt() const { return executedAt; }
    std::string getExecutionTxHash() const { return executionTxHash; }
    
    // Setters
    void setQuorumThreshold(double threshold) { quorumThreshold = threshold; }
    void setPassThreshold(double threshold) { passThreshold = threshold; }
    
    // Results
    double getApprovalRatio() const;
    double getParticipationRatio(double totalStake) const;
    
    // Serialization
    std::string serialize() const;
    static Proposal deserialize(const std::string& data);
    
    // Constants
    static const double DEFAULT_QUORUM_THRESHOLD; // 15%
    static const double DEFAULT_PASS_THRESHOLD;   // 60%
    static const uint32_t DEFAULT_VOTING_PERIOD;  // 7 days
    static const double MINIMUM_PROPOSAL_STAKE;   // 1000 GXC
};

class GovernanceSystem {
private:
    mutable std::mutex proposalsMutex;
    std::unordered_map<std::string, Proposal> proposals;
    std::unordered_map<std::string, std::vector<Vote>> proposalVotes;
    std::unordered_map<std::string, std::time_t> voterLastActivity;
    
    // System parameters
    std::unordered_map<ParameterType, double> parameters;
    
    // Access control
    std::unordered_map<std::string, bool> authorizedProposers;
    
public:
    // Constructor
    GovernanceSystem();
    
    // Proposal management
    std::string submitProposal(const std::string& proposer, const std::string& title,
                              const std::string& description, const std::vector<ParameterChange>& changes);
    bool cancelProposal(const std::string& proposalId, const std::string& canceller);
    void updateProposalStatus(const std::string& proposalId);
    
    // Voting
    bool castVote(const std::string& voter, const std::string& proposalId, VoteType voteType, 
                  double stakeAmount, uint32_t stakingDays);
    void tallyVotes(const std::string& proposalId);
    
    // Execution
    bool executeProposal(const std::string& proposalId);
    void applyParameterChanges(const std::vector<ParameterChange>& changes);
    
    // Parameter management
    double getParameter(ParameterType paramType) const;
    void setParameter(ParameterType paramType, double value);
    std::vector<ParameterChange> getPendingChanges() const;
    
    // Queries
    std::vector<Proposal> getActiveProposals() const;
    std::vector<Proposal> getProposalHistory() const;
    Proposal getProposal(const std::string& proposalId) const;
    std::vector<Vote> getProposalVotes(const std::string& proposalId) const;
    
    // Access control
    void authorizeProposer(const std::string& address);
    void revokeProposer(const std::string& address);
    bool isAuthorizedProposer(const std::string& address) const;
    
    // Statistics
    uint32_t getTotalProposals() const;
    uint32_t getActiveProposalCount() const;
    double getAverageParticipation() const;
    
    // Validation
    bool isValidParameterChange(const ParameterChange& change) const;
    bool isValidProposal(const Proposal& proposal) const;
    
    // Vote weight calculation
    double calculateVoteWeight(double stakeAmount, uint32_t stakingDays) const;
    
    // Constants
    static const uint32_t MAX_ACTIVE_PROPOSALS = 10;
    static const uint32_t MAX_VOTING_PERIOD_DAYS = 30;
    static const uint32_t MIN_VOTING_PERIOD_DAYS = 3;
};