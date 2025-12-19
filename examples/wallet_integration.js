/**
 * GXC Wallet Integration Examples
 * Complete working examples for wallet developers
 */

const crypto = require('crypto');
const fetch = require('node-fetch');

// Configuration
const NODE_URL = 'http://localhost:8545';
const NETWORK = 'testnet'; // or 'mainnet'

/**
 * Make RPC call to GXC node
 */
async function rpc(method, params = []) {
  const response = await fetch(NODE_URL, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({
      jsonrpc: '2.0',
      method: method,
      params: params,
      id: 1
    })
  });
  
  const data = await response.json();
  
  if (data.error) {
    throw new Error(`RPC Error: ${data.error.message}`);
  }
  
  return data.result;
}

/**
 * Example 1: Send Transaction
 */
async function example1_sendTransaction() {
  console.log('\n=== Example 1: Send Transaction ===\n');
  
  try {
    // Send 10 GXC to recipient
    const txid = await rpc('sendtoaddress', [
      'tGXC1234567890abcdef...',  // recipient address
      10.0                         // amount in GXC
    ]);
    
    console.log(`✅ Transaction sent!`);
    console.log(`   TX ID: ${txid}`);
    
    // Get transaction details
    const tx = await rpc('gettransaction', [txid]);
    console.log(`   Confirmations: ${tx.confirmations}`);
    console.log(`   Fee: ${tx.fee} GXC`);
    
  } catch (error) {
    console.error(`❌ Error: ${error.message}`);
  }
}

/**
 * Example 2: Check Balance
 */
async function example2_checkBalance() {
  console.log('\n=== Example 2: Check Balance ===\n');
  
  try {
    const address = 'tGXC1234567890abcdef...';
    
    // Get spendable balance
    const balance = await rpc('getbalance', [address]);
    console.log(`✅ Spendable Balance: ${balance} GXC`);
    
    // Get staking info
    const stakingInfo = await rpc('getStakingInfo', [address]);
    console.log(`   Staked Amount: ${stakingInfo.staked_amount} GXC`);
    console.log(`   Total Balance: ${stakingInfo.total_balance} GXC`);
    console.log(`   Pending Rewards: ${stakingInfo.pending_rewards} GXC`);
    
  } catch (error) {
    console.error(`❌ Error: ${error.message}`);
  }
}

/**
 * Example 3: Register as Validator
 */
async function example3_registerValidator() {
  console.log('\n=== Example 3: Register as Validator ===\n');
  
  try {
    const address = 'tGXC1234567890abcdef...';
    const stakeAmount = 100.0;  // Minimum 100 GXC
    const stakingDays = 30;     // 14-365 days
    
    // Register validator
    const result = await rpc('registerValidator', [
      address,
      stakeAmount,
      stakingDays
    ]);
    
    console.log(`✅ Validator registered!`);
    console.log(`   Address: ${result.address}`);
    console.log(`   Stake: ${result.stake_amount} GXC`);
    console.log(`   Status: ${result.status}`);
    console.log(`   Stake TX: ${result.stake_tx}`);
    console.log(`   Message: ${result.message}`);
    
    // Check validator status
    const info = await rpc('getValidatorInfo', [address]);
    console.log(`\n   Validator Info:`);
    console.log(`   - Is Pending: ${info.is_pending}`);
    console.log(`   - Is Active: ${info.is_active}`);
    console.log(`   - Weighted Stake: ${info.weighted_stake}`);
    
  } catch (error) {
    console.error(`❌ Error: ${error.message}`);
  }
}

/**
 * Example 4: Add More Stake
 */
async function example4_addStake() {
  console.log('\n=== Example 4: Add More Stake ===\n');
  
  try {
    const address = 'tGXC1234567890abcdef...';
    const additionalAmount = 50.0;
    
    // Add stake
    const result = await rpc('addStake', [
      address,
      additionalAmount
    ]);
    
    console.log(`✅ Stake added!`);
    console.log(`   Additional Amount: ${result.additional_amount} GXC`);
    console.log(`   TX ID: ${result.txid}`);
    
    // Check updated info
    const info = await rpc('getValidatorInfo', [address]);
    console.log(`   Total Stake: ${info.stake_amount} GXC`);
    
  } catch (error) {
    console.error(`❌ Error: ${error.message}`);
  }
}

/**
 * Example 5: Unstake
 */
async function example5_unstake() {
  console.log('\n=== Example 5: Unstake ===\n');
  
  try {
    const address = 'tGXC1234567890abcdef...';
    const unstakeAmount = 50.0;
    
    // Unstake
    const result = await rpc('unstake', [
      address,
      unstakeAmount
    ]);
    
    console.log(`✅ Unstake successful!`);
    console.log(`   Amount: ${result.amount} GXC`);
    console.log(`   Remaining Stake: ${result.remaining_stake} GXC`);
    console.log(`   TX ID: ${result.txid}`);
    console.log(`   Message: ${result.message}`);
    
  } catch (error) {
    console.error(`❌ Error: ${error.message}`);
  }
}

/**
 * Example 6: List All Validators
 */
async function example6_listValidators() {
  console.log('\n=== Example 6: List All Validators ===\n');
  
  try {
    const validators = await rpc('getValidators', []);
    
    console.log(`✅ Found ${validators.length} validators\n`);
    
    validators.slice(0, 5).forEach((v, i) => {
      console.log(`${i + 1}. ${v.address.substring(0, 20)}...`);
      console.log(`   Stake: ${v.stake_amount} GXC`);
      console.log(`   Weighted: ${v.weighted_stake}`);
      console.log(`   Active: ${v.is_active}`);
      console.log(`   Blocks: ${v.blocks_produced}`);
      console.log(`   APY: ${v.apy.toFixed(2)}%\n`);
    });
    
  } catch (error) {
    console.error(`❌ Error: ${error.message}`);
  }
}

/**
 * Example 7: Verify Transaction Traceability
 */
async function example7_verifyTraceability() {
  console.log('\n=== Example 7: Verify Traceability ===\n');
  
  try {
    const txid = '0x1234567890abcdef...';
    
    // Get transaction
    const tx = await rpc('gettransaction', [txid]);
    
    console.log(`Transaction: ${txid.substring(0, 16)}...`);
    console.log(`   Type: ${tx.type}`);
    console.log(`   Amount: ${tx.amount} GXC`);
    
    // Verify traceability formula
    if (tx.inputs && tx.inputs.length > 0) {
      const hashMatch = tx.inputs[0].txHash === tx.prevTxHash;
      const amountMatch = Math.abs(tx.inputs[0].amount - tx.referencedAmount) < 0.00000001;
      
      console.log(`\n   Traceability Formula:`);
      console.log(`   ✓ Inputs[0].txHash == PrevTxHash: ${hashMatch}`);
      console.log(`   ✓ Inputs[0].amount == ReferencedAmount: ${amountMatch}`);
      
      if (hashMatch && amountMatch) {
        console.log(`\n   ✅ Traceability VALID`);
      } else {
        console.log(`\n   ❌ Traceability INVALID`);
      }
    }
    
  } catch (error) {
    console.error(`❌ Error: ${error.message}`);
  }
}

/**
 * Example 8: Trace Transaction Chain
 */
async function example8_traceChain() {
  console.log('\n=== Example 8: Trace Transaction Chain ===\n');
  
  try {
    const txid = '0x1234567890abcdef...';
    
    console.log(`Tracing transaction chain...\n`);
    
    let currentTx = txid;
    let depth = 0;
    const maxDepth = 10;
    
    while (currentTx && currentTx !== '0' && depth < maxDepth) {
      const tx = await rpc('gettransaction', [currentTx]);
      
      console.log(`${depth + 1}. TX: ${currentTx.substring(0, 16)}...`);
      console.log(`   Amount: ${tx.amount} GXC`);
      console.log(`   Type: ${tx.type}`);
      console.log(`   PrevTx: ${tx.prevTxHash.substring(0, 16)}...`);
      
      currentTx = tx.prevTxHash;
      depth++;
    }
    
    console.log(`\n✅ Traced ${depth} transactions`);
    
  } catch (error) {
    console.error(`❌ Error: ${error.message}`);
  }
}

/**
 * Example 9: Estimate Fee
 */
async function example9_estimateFee() {
  console.log('\n=== Example 9: Estimate Fee ===\n');
  
  try {
    const feeInfo = await rpc('estimateFee', []);
    
    console.log(`✅ Fee Estimation:`);
    console.log(`   Recommended: ${feeInfo.recommended_fee} GXC`);
    console.log(`   Base Fee: ${feeInfo.base_fee} GXC`);
    console.log(`   Max Fee: ${feeInfo.max_fee} GXC`);
    console.log(`   Pending TXs: ${feeInfo.pending_transactions}`);
    console.log(`   Dynamic Fees: ${feeInfo.dynamic_fees_enabled}`);
    
    console.log(`\n   Fee Tiers:`);
    console.log(`   - Low: ${feeInfo.fee_tiers.low} GXC`);
    console.log(`   - Medium: ${feeInfo.fee_tiers.medium} GXC`);
    console.log(`   - High: ${feeInfo.fee_tiers.high} GXC`);
    
  } catch (error) {
    console.error(`❌ Error: ${error.message}`);
  }
}

/**
 * Example 10: Monitor Validator Status
 */
async function example10_monitorValidator() {
  console.log('\n=== Example 10: Monitor Validator Status ===\n');
  
  try {
    const address = 'tGXC1234567890abcdef...';
    
    // Get validator info
    const info = await rpc('getValidatorInfo', [address]);
    
    console.log(`✅ Validator Status:`);
    console.log(`   Address: ${info.address.substring(0, 30)}...`);
    console.log(`   Stake: ${info.stake_amount} GXC`);
    console.log(`   Status: ${info.status}`);
    console.log(`   Active: ${info.is_active}`);
    console.log(`   Pending: ${info.is_pending}`);
    
    console.log(`\n   Performance:`);
    console.log(`   - Blocks Produced: ${info.blocks_produced}`);
    console.log(`   - Missed Blocks: ${info.missed_blocks}`);
    console.log(`   - Uptime: ${(info.uptime * 100).toFixed(2)}%`);
    
    console.log(`\n   Rewards:`);
    console.log(`   - Total Rewards: ${info.total_rewards} GXC`);
    console.log(`   - Pending Rewards: ${info.pending_rewards} GXC`);
    console.log(`   - APY: ${info.apy.toFixed(2)}%`);
    
    console.log(`\n   Staking:`);
    console.log(`   - Staking Days: ${info.staking_days}`);
    console.log(`   - Weighted Stake: ${info.weighted_stake}`);
    console.log(`   - Time Weight: ${info.time_weight.toFixed(4)}`);
    
  } catch (error) {
    console.error(`❌ Error: ${error.message}`);
  }
}

/**
 * Run all examples
 */
async function runAllExamples() {
  console.log('╔════════════════════════════════════════════════════════════╗');
  console.log('║         GXC Wallet Integration Examples                   ║');
  console.log('╚════════════════════════════════════════════════════════════╝');
  
  // Check node connection
  try {
    const info = await rpc('getblockchaininfo', []);
    console.log(`\n✅ Connected to GXC node`);
    console.log(`   Network: ${info.chain}`);
    console.log(`   Height: ${info.blocks}`);
    console.log(`   Version: ${info.version}`);
  } catch (error) {
    console.error(`\n❌ Cannot connect to node at ${NODE_URL}`);
    console.error(`   Please start the node first: ./build/gxc-node`);
    return;
  }
  
  // Run examples (uncomment to run)
  // await example1_sendTransaction();
  // await example2_checkBalance();
  // await example3_registerValidator();
  // await example4_addStake();
  // await example5_unstake();
  // await example6_listValidators();
  // await example7_verifyTraceability();
  // await example8_traceChain();
  // await example9_estimateFee();
  // await example10_monitorValidator();
  
  console.log('\n✅ Examples ready to run!');
  console.log('   Uncomment the examples you want to test.\n');
}

// Run if executed directly
if (require.main === module) {
  runAllExamples().catch(console.error);
}

// Export for use in other modules
module.exports = {
  rpc,
  example1_sendTransaction,
  example2_checkBalance,
  example3_registerValidator,
  example4_addStake,
  example5_unstake,
  example6_listValidators,
  example7_verifyTraceability,
  example8_traceChain,
  example9_estimateFee,
  example10_monitorValidator
};
