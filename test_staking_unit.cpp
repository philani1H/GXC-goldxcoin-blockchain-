/**
 * GXC Blockchain - Standalone Staking Unit Test
 * Compile with: g++ -std=c++17 -I/workspace/include -I/workspace/third_party test_staking_unit.cpp -o test_staking_unit -lgtest -lgtest_main -lpthread
 */

#include <gtest/gtest.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <random>

// Minimal definitions for testing (copied from headers)

struct TransactionInput {
    std::string txHash;
    uint32_t outputIndex;
    std::string signature;
    double amount;
    std::string publicKey;
};

struct TransactionOutput {
    std::string address;
    double amount;
    std::string script;
};

enum class TransactionType {
    NORMAL = 0,
    STAKE = 1,
    UNSTAKE = 2
};

// Simplified Validator for testing
class Validator {
public:
    static constexpr double BETA = 0.5;
    static constexpr double MIN_STAKE = 100.0;
    static constexpr uint32_t MIN_STAKING_DAYS = 14;
    static constexpr uint32_t MAX_STAKING_DAYS = 365;
    
private:
    std::string address;
    double stakeAmount;
    uint32_t stakingDays;
    bool isActive;
    
public:
    Validator() : stakeAmount(0), stakingDays(0), isActive(false) {}
    
    Validator(const std::string& addr, double stake, uint32_t days)
        : address(addr), stakeAmount(stake), stakingDays(days), isActive(true) {}
    
    std::string getAddress() const { return address; }
    double getStakeAmount() const { return stakeAmount; }
    bool getIsActive() const { return isActive; }
    
    bool hasMinimumStake() const { return stakeAmount >= MIN_STAKE; }
    
    bool isStakingPeriodValid() const {
        return stakingDays >= MIN_STAKING_DAYS && stakingDays <= MAX_STAKING_DAYS;
    }
    
    double getTimeWeight() const {
        return std::pow(static_cast<double>(stakingDays) / 365.0, BETA);
    }
    
    double getWeightedStake() const {
        if (!isActive) return 0.0;
        return stakeAmount * getTimeWeight();
    }
    
    void removeStake(double amount) {
        if (amount > stakeAmount) amount = stakeAmount;
        stakeAmount -= amount;
        if (stakeAmount < MIN_STAKE) isActive = false;
    }
};

// Simplified Transaction for testing
class Transaction {
private:
    std::string txHash;
    TransactionType type;
    std::vector<TransactionInput> inputs;
    std::vector<TransactionOutput> outputs;
    std::string prevTxHash;
    double referencedAmount;
    std::string senderAddress;
    std::string receiverAddress;
    double fee;
    bool isCoinbase;
    
public:
    Transaction() : type(TransactionType::NORMAL), referencedAmount(0), fee(0), isCoinbase(false) {}
    
    Transaction(const std::vector<TransactionInput>& ins, 
                const std::vector<TransactionOutput>& outs,
                const std::string& prevHash)
        : type(TransactionType::NORMAL), inputs(ins), outputs(outs), 
          prevTxHash(prevHash), referencedAmount(0), fee(0), isCoinbase(false) {
        if (!inputs.empty()) {
            referencedAmount = inputs[0].amount;
        }
    }
    
    // Getters
    const std::vector<TransactionInput>& getInputs() const { return inputs; }
    const std::vector<TransactionOutput>& getOutputs() const { return outputs; }
    std::string getPrevTxHash() const { return prevTxHash; }
    double getReferencedAmount() const { return referencedAmount; }
    TransactionType getType() const { return type; }
    double getFee() const { return fee; }
    bool isCoinbaseTransaction() const { return isCoinbase; }
    
    // Setters
    void setType(TransactionType t) { type = t; }
    void setReferencedAmount(double amt) { referencedAmount = amt; }
    void setFee(double f) { fee = f; }
    
    double getTotalInputAmount() const {
        double total = 0;
        for (const auto& inp : inputs) total += inp.amount;
        return total;
    }
    
    double getTotalOutputAmount() const {
        double total = 0;
        for (const auto& out : outputs) total += out.amount;
        return total;
    }
    
    // TRACEABILITY FORMULA VERIFICATION
    // Ti.Inputs[0].txHash == Ti.PrevTxHash
    // Ti.Inputs[0].amount == Ti.ReferencedAmount
    bool verifyTraceabilityFormula() const {
        if (isCoinbase) return true;
        if (inputs.empty()) return false;
        
        // Check hash match
        if (inputs[0].txHash != prevTxHash) {
            std::cout << "Traceability FAIL: input.txHash != prevTxHash" << std::endl;
            return false;
        }
        
        // Check amount match
        if (std::abs(inputs[0].amount - referencedAmount) > 0.00000001) {
            std::cout << "Traceability FAIL: input.amount != referencedAmount" << std::endl;
            return false;
        }
        
        return true;
    }
};

// Simplified ValidatorSelector for testing
class ValidatorSelector {
private:
    std::vector<Validator> validators;
    
public:
    void addValidator(const Validator& v) {
        validators.push_back(v);
    }
    
    const std::vector<Validator>& getValidators() const {
        return validators;
    }
    
    Validator selectValidator() const {
        double totalWeight = 0;
        for (const auto& v : validators) {
            if (v.getIsActive()) totalWeight += v.getWeightedStake();
        }
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0, totalWeight);
        double threshold = dis(gen);
        
        double cumWeight = 0;
        for (const auto& v : validators) {
            if (v.getIsActive()) {
                cumWeight += v.getWeightedStake();
                if (cumWeight >= threshold) return v;
            }
        }
        
        return validators.back();
    }
};

// ==================== TESTS ====================

TEST(ValidatorTest, MinimumStake) {
    Validator v("GXCtest", 100.0, 30);
    EXPECT_TRUE(v.hasMinimumStake());
    EXPECT_TRUE(v.getIsActive());
    
    Validator v2("GXCtest2", 50.0, 30);
    EXPECT_FALSE(v2.hasMinimumStake());
}

TEST(ValidatorTest, StakingPeriod) {
    Validator v1("addr1", 100.0, 30);
    EXPECT_TRUE(v1.isStakingPeriodValid());
    
    Validator v2("addr2", 100.0, 7); // Too short
    EXPECT_FALSE(v2.isStakingPeriodValid());
    
    Validator v3("addr3", 100.0, 500); // Too long
    EXPECT_FALSE(v3.isStakingPeriodValid());
}

TEST(ValidatorTest, RemoveStake) {
    Validator v("GXCtest", 200.0, 30);
    
    v.removeStake(50.0);
    EXPECT_EQ(v.getStakeAmount(), 150.0);
    EXPECT_TRUE(v.getIsActive());
    
    v.removeStake(100.0);
    EXPECT_EQ(v.getStakeAmount(), 50.0);
    EXPECT_FALSE(v.getIsActive()); // Below minimum
}

TEST(ValidatorTest, WeightedStake) {
    Validator v("GXCtest", 100.0, 365);
    double weighted = v.getWeightedStake();
    EXPECT_NEAR(weighted, 100.0, 0.01); // Full year = weight 1.0
    
    Validator v2("GXCtest2", 100.0, 91); // ~3 months
    double weighted2 = v2.getWeightedStake();
    EXPECT_LT(weighted2, weighted); // Less time = less weight
}

TEST(TraceabilityTest, ValidFormula) {
    std::vector<TransactionInput> inputs;
    TransactionInput inp;
    inp.txHash = "prev_tx_123";
    inp.outputIndex = 0;
    inp.amount = 100.0;
    inputs.push_back(inp);
    
    std::vector<TransactionOutput> outputs;
    TransactionOutput out;
    out.address = "recipient";
    out.amount = 99.0;
    outputs.push_back(out);
    
    // Correct: prevTxHash matches input[0].txHash
    Transaction tx(inputs, outputs, "prev_tx_123");
    tx.setReferencedAmount(100.0);
    
    EXPECT_TRUE(tx.verifyTraceabilityFormula());
}

TEST(TraceabilityTest, InvalidFormula_HashMismatch) {
    std::vector<TransactionInput> inputs;
    TransactionInput inp;
    inp.txHash = "correct_hash";
    inp.outputIndex = 0;
    inp.amount = 100.0;
    inputs.push_back(inp);
    
    std::vector<TransactionOutput> outputs;
    TransactionOutput out;
    out.address = "recipient";
    out.amount = 99.0;
    outputs.push_back(out);
    
    // WRONG: prevTxHash doesn't match input[0].txHash
    Transaction tx(inputs, outputs, "wrong_hash");
    tx.setReferencedAmount(100.0);
    
    EXPECT_FALSE(tx.verifyTraceabilityFormula());
}

TEST(TraceabilityTest, InvalidFormula_AmountMismatch) {
    std::vector<TransactionInput> inputs;
    TransactionInput inp;
    inp.txHash = "hash_123";
    inp.outputIndex = 0;
    inp.amount = 100.0;
    inputs.push_back(inp);
    
    std::vector<TransactionOutput> outputs;
    TransactionOutput out;
    out.address = "recipient";
    out.amount = 99.0;
    outputs.push_back(out);
    
    Transaction tx(inputs, outputs, "hash_123");
    tx.setReferencedAmount(50.0); // WRONG: doesn't match input amount
    
    EXPECT_FALSE(tx.verifyTraceabilityFormula());
}

TEST(TransactionTest, InputOutputBalance) {
    std::vector<TransactionInput> inputs;
    TransactionInput inp;
    inp.txHash = "prev";
    inp.outputIndex = 0;
    inp.amount = 100.0;
    inputs.push_back(inp);
    
    std::vector<TransactionOutput> outputs;
    TransactionOutput out1;
    out1.address = "recipient";
    out1.amount = 50.0;
    outputs.push_back(out1);
    
    TransactionOutput out2;
    out2.address = "change";
    out2.amount = 49.999; // Change
    outputs.push_back(out2);
    
    Transaction tx(inputs, outputs, "prev");
    tx.setFee(0.001);
    
    double inputTotal = tx.getTotalInputAmount();
    double outputTotal = tx.getTotalOutputAmount();
    double fee = tx.getFee();
    
    // inputs = outputs + fee
    EXPECT_NEAR(inputTotal, outputTotal + fee, 0.00001);
}

TEST(StakeTransactionTest, StakeReducesOutputs) {
    // STAKE transaction: staked amount should NOT be in outputs
    std::vector<TransactionInput> inputs;
    TransactionInput inp;
    inp.txHash = "prev";
    inp.outputIndex = 0;
    inp.amount = 200.0;
    inputs.push_back(inp);
    
    // Only change output (staked amount is "locked")
    std::vector<TransactionOutput> outputs;
    TransactionOutput change;
    change.address = "sender";
    change.amount = 99.999; // 200 - 100 stake - 0.001 fee = 99.999
    outputs.push_back(change);
    
    Transaction tx(inputs, outputs, "prev");
    tx.setType(TransactionType::STAKE);
    tx.setFee(0.001);
    
    double stakeAmount = tx.getTotalInputAmount() - tx.getTotalOutputAmount() - tx.getFee();
    EXPECT_NEAR(stakeAmount, 100.0, 0.01);
}

TEST(UnstakeTransactionTest, UnstakeCreatesOutput) {
    // UNSTAKE transaction: creates output for unstaked amount
    std::vector<TransactionInput> inputs;
    TransactionInput dummy;
    dummy.txHash = "stake_ref";
    dummy.outputIndex = 0;
    dummy.amount = 0.0; // Dummy input for unstake
    inputs.push_back(dummy);
    
    std::vector<TransactionOutput> outputs;
    TransactionOutput unstaked;
    unstaked.address = "staker";
    unstaked.amount = 100.0; // Coins returned from stake
    outputs.push_back(unstaked);
    
    Transaction tx(inputs, outputs, "stake_ref");
    tx.setType(TransactionType::UNSTAKE);
    
    EXPECT_EQ(tx.getType(), TransactionType::UNSTAKE);
    EXPECT_EQ(tx.getTotalOutputAmount(), 100.0);
}

TEST(ValidatorSelectorTest, WeightedSelection) {
    ValidatorSelector selector;
    
    Validator v1("addr1", 100.0, 30);
    Validator v2("addr2", 200.0, 30);
    Validator v3("addr3", 300.0, 30);
    
    selector.addValidator(v1);
    selector.addValidator(v2);
    selector.addValidator(v3);
    
    // Run many selections
    int counts[3] = {0, 0, 0};
    for (int i = 0; i < 1000; i++) {
        Validator v = selector.selectValidator();
        if (v.getAddress() == "addr1") counts[0]++;
        else if (v.getAddress() == "addr2") counts[1]++;
        else if (v.getAddress() == "addr3") counts[2]++;
    }
    
    // Higher stake = selected more often
    std::cout << "Selection counts: addr1=" << counts[0] 
              << ", addr2=" << counts[1] 
              << ", addr3=" << counts[2] << std::endl;
    
    EXPECT_GT(counts[2], counts[0]); // addr3 (300) > addr1 (100)
    EXPECT_GT(counts[1], counts[0]); // addr2 (200) > addr1 (100)
}

// Main
int main(int argc, char** argv) {
    std::cout << "========================================" << std::endl;
    std::cout << "GXC BLOCKCHAIN STAKING UNIT TESTS" << std::endl;
    std::cout << "========================================" << std::endl;
    
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
