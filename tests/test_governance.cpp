#include <gtest/gtest.h>
#include "Governance.h"

TEST(GovernanceTest, ProposalCreation) {
    Proposal p("proposer", "title", "desc", {});
    EXPECT_TRUE(p.isValidProposal());
}