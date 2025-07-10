// Proposals.cpp
// Stub implementation for governance proposals.

#include "Governance.h"
#include <iostream>

Proposal::Proposal(const std::string& proposerIn, const std::string& titleIn, const std::string& descriptionIn, const std::vector<ParameterChange>& changesIn, uint32_t votingPeriodDays)
    : proposer(proposerIn), title(titleIn), description(descriptionIn), parameterChanges(changesIn), createdAt(std::time(nullptr)), votingEndsAt(createdAt + votingPeriodDays * 86400), status(ProposalStatus::ACTIVE), forVotes(0), againstVotes(0), abstainVotes(0), totalVotes(0), quorumThreshold(DEFAULT_QUORUM_THRESHOLD), passThreshold(DEFAULT_PASS_THRESHOLD), executed(false), executedAt(0) {}

bool Proposal::addVote(const Vote& vote) {
    if (!isVotingOpen()) return false;
    if (hasVoted(vote.voter)) return false;
    switch (vote.voteType) {
        case VoteType::FOR: forVotes += vote.voteWeight; break;
        case VoteType::AGAINST: againstVotes += vote.voteWeight; break;
        case VoteType::ABSTAIN: abstainVotes += vote.voteWeight; break;
    }
    totalVotes += vote.voteWeight;
    return true;
}
bool Proposal::hasVoted(const std::string&) const { return false; }
void Proposal::tallyVotes() { updateStatus(); }
void Proposal::updateStatus() {
    if (executed) status = ProposalStatus::EXECUTED;
    else if (std::time(nullptr) > votingEndsAt) status = hasPassed() ? ProposalStatus::PASSED : ProposalStatus::FAILED;
    else status = ProposalStatus::ACTIVE;
}
bool Proposal::canExecute() const { return status == ProposalStatus::PASSED && !executed; }
void Proposal::markExecuted(const std::string& txHash) { executed = true; executedAt = std::time(nullptr); executionTxHash = txHash; status = ProposalStatus::EXECUTED; }
void Proposal::cancel() { status = ProposalStatus::CANCELLED; }
bool Proposal::isValidProposal() const { return !title.empty() && !description.empty(); }
bool Proposal::isVotingOpen() const { return status == ProposalStatus::ACTIVE && std::time(nullptr) < votingEndsAt; }
bool Proposal::hasQuorum() const { return totalVotes >= quorumThreshold; }
bool Proposal::hasPassed() const { return forVotes >= passThreshold * totalVotes; }
double Proposal::getApprovalRatio() const { return totalVotes ? forVotes / totalVotes : 0.0; }
double Proposal::getParticipationRatio(double totalStake) const { return totalStake ? totalVotes / totalStake : 0.0; }
std::string Proposal::serialize() const { return "<proposal_serialized>"; }
Proposal Proposal::deserialize(const std::string&) { return Proposal("","","",{}); }