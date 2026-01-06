use anyhow::{Result, bail};
use log::{info, warn, error};

use super::block::Block;

/// Block validator for GXC blockchain
pub struct BlockValidator {
    network: String,
    min_difficulty: f64,
}

impl BlockValidator {
    pub fn new(network: String) -> Self {
        let min_difficulty = if network == "testnet" { 1.0 } else { 1000.0 };
        
        Self {
            network,
            min_difficulty,
        }
    }
    
    /// Validate a block completely
    pub fn validate_block(&self, block: &Block, previous_block: Option<&Block>) -> Result<()> {
        info!("Validating block at height {}", block.height);
        
        // 1. Verify proof-of-work
        if !block.verify_proof_of_work() {
            bail!("Invalid proof-of-work for block {}", block.height);
        }
        
        // 2. Verify work receipt
        if !block.verify_work_receipt() {
            bail!("Invalid work receipt for block {}", block.height);
        }
        
        // 3. Verify merkle root
        if !block.verify_merkle_root() {
            bail!("Invalid merkle root for block {}", block.height);
        }
        
        // 4. Verify coinbase transaction
        if !block.verify_coinbase() {
            bail!("Invalid coinbase transaction for block {}", block.height);
        }
        
        // 5. Verify difficulty meets minimum
        if block.difficulty < self.min_difficulty {
            bail!(
                "Block difficulty {} below minimum {} for {}",
                block.difficulty,
                self.min_difficulty,
                self.network
            );
        }
        
        // 6. Verify timestamp is reasonable
        let now = chrono::Utc::now().timestamp() as u64;
        if block.time > now + 7200 {
            bail!("Block timestamp too far in future");
        }
        
        // 7. Verify previous block link
        if let Some(prev) = previous_block {
            if block.previousblockhash != prev.hash {
                bail!(
                    "Previous hash mismatch: expected {}, got {}",
                    prev.hash,
                    block.previousblockhash
                );
            }
            
            if block.height != prev.height + 1 {
                bail!(
                    "Height mismatch: expected {}, got {}",
                    prev.height + 1,
                    block.height
                );
            }
            
            if block.time <= prev.time {
                bail!("Block timestamp must be greater than previous block");
            }
        }
        
        // 8. Verify transactions
        for (i, tx) in block.tx.iter().enumerate() {
            if i == 0 {
                // Coinbase must be first
                if !tx.is_coinbase {
                    bail!("First transaction must be coinbase");
                }
            } else {
                // Non-coinbase transactions
                if tx.is_coinbase {
                    bail!("Coinbase must be first transaction only");
                }
                
                // Verify transaction has valid amounts
                if tx.amount <= 0.0 {
                    bail!("Transaction {} has invalid amount", tx.hash);
                }
                
                if tx.fee < 0.0 {
                    bail!("Transaction {} has negative fee", tx.hash);
                }
            }
        }
        
        info!("✅ Block {} validated successfully", block.height);
        
        Ok(())
    }
    
    /// Quick validation (for initial sync)
    pub fn quick_validate(&self, block: &Block) -> Result<()> {
        // Only check critical fields
        if !block.verify_proof_of_work() {
            bail!("Invalid proof-of-work");
        }
        
        if !block.verify_work_receipt() {
            bail!("Invalid work receipt");
        }
        
        if block.difficulty < self.min_difficulty {
            bail!("Difficulty too low");
        }
        
        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::blockchain::block::Transaction;
    
    #[test]
    fn test_block_validation() {
        let validator = BlockValidator::new("testnet".to_string());
        
        let coinbase = Transaction {
            hash: "coinbase_hash".to_string(),
            from: String::new(),
            to: "miner_address".to_string(),
            amount: 50.0,
            fee: 0.0,
            timestamp: 1234567890,
            signature: String::new(),
            is_coinbase: true,
        };
        
        let mut block = Block {
            height: 1,
            hash: "0000abc123".to_string(),
            previous_hash: "genesis".to_string(),
            merkle_root: String::new(),
            timestamp: 1234567890,
            nonce: 12345,
            difficulty: 4.0,
            miner: "test_miner".to_string(),
            work_receipt: String::new(),
            transactions: vec![coinbase],
        };
        
        // Calculate merkle root and work receipt
        block.merkle_root = block.calculate_merkle_root();
        block.work_receipt = block.compute_work_receipt();
        
        // Should validate
        assert!(validator.validate_block(&block, None).is_ok());
    }
}
