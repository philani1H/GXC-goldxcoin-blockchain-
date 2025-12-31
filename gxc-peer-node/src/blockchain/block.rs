use serde::{Deserialize, Serialize};
use sha2::{Sha256, Digest};

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Block {
    pub height: u32,
    pub hash: String,
    pub previous_hash: String,
    pub merkle_root: String,
    pub timestamp: u64,
    pub nonce: u64,
    pub difficulty: f64,
    pub miner: String,
    pub work_receipt: String,
    pub transactions: Vec<Transaction>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Transaction {
    pub hash: String,
    pub from: String,
    pub to: String,
    pub amount: f64,
    pub fee: f64,
    pub timestamp: u64,
    pub signature: String,
    pub is_coinbase: bool,
}

impl Block {
    /// Compute work receipt for this block
    pub fn compute_work_receipt(&self) -> String {
        let data = format!(
            "{}{}{}{}{}{}",
            self.previous_hash,
            self.merkle_root,
            self.nonce,
            self.miner, // In real implementation, use miner_pubkey
            self.difficulty,
            self.timestamp
        );
        
        let mut hasher = Sha256::new();
        hasher.update(data.as_bytes());
        hex::encode(hasher.finalize())
    }
    
    /// Verify work receipt matches computed value
    pub fn verify_work_receipt(&self) -> bool {
        let computed = self.compute_work_receipt();
        computed == self.work_receipt
    }
    
    /// Verify proof-of-work (hash meets difficulty target)
    pub fn verify_proof_of_work(&self) -> bool {
        let leading_zeros = self.hash.chars()
            .take_while(|&c| c == '0')
            .count();
        
        let required_zeros = self.difficulty as usize;
        
        leading_zeros >= required_zeros
    }
    
    /// Calculate merkle root from transactions
    pub fn calculate_merkle_root(&self) -> String {
        if self.transactions.is_empty() {
            return String::from("0000000000000000000000000000000000000000000000000000000000000000");
        }
        
        let mut hashes: Vec<String> = self.transactions
            .iter()
            .map(|tx| tx.hash.clone())
            .collect();
        
        while hashes.len() > 1 {
            let mut next_level = Vec::new();
            
            for chunk in hashes.chunks(2) {
                let combined = if chunk.len() == 2 {
                    format!("{}{}", chunk[0], chunk[1])
                } else {
                    format!("{}{}", chunk[0], chunk[0])
                };
                
                let mut hasher = Sha256::new();
                hasher.update(combined.as_bytes());
                next_level.push(hex::encode(hasher.finalize()));
            }
            
            hashes = next_level;
        }
        
        hashes[0].clone()
    }
    
    /// Verify merkle root matches transactions
    pub fn verify_merkle_root(&self) -> bool {
        let computed = self.calculate_merkle_root();
        computed == self.merkle_root
    }
    
    /// Verify coinbase transaction is valid
    pub fn verify_coinbase(&self) -> bool {
        if self.transactions.is_empty() {
            return false;
        }
        
        let coinbase = &self.transactions[0];
        
        // Must be coinbase type
        if !coinbase.is_coinbase {
            return false;
        }
        
        // Must be first transaction
        // (already guaranteed by index 0)
        
        // Verify amount is correct for block height
        let expected_reward = Self::calculate_block_reward(self.height);
        
        // Allow small floating point difference
        (coinbase.amount - expected_reward).abs() < 0.00000001
    }
    
    /// Calculate block reward with halving
    pub fn calculate_block_reward(height: u32) -> f64 {
        const INITIAL_REWARD: f64 = 50.0;
        const HALVING_INTERVAL: u32 = 1051200; // ~4 years
        
        let halvings = height / HALVING_INTERVAL;
        let mut reward = INITIAL_REWARD;
        
        for _ in 0..halvings {
            reward /= 2.0;
        }
        
        // Minimum reward
        if reward < 0.00000001 {
            reward = 0.00000001;
        }
        
        reward
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    
    #[test]
    fn test_work_receipt_computation() {
        let block = Block {
            height: 1,
            hash: "00000000abc123".to_string(),
            previous_hash: "genesis".to_string(),
            merkle_root: "merkle".to_string(),
            timestamp: 1234567890,
            nonce: 12345,
            difficulty: 1.0,
            miner: "test_miner".to_string(),
            work_receipt: String::new(),
            transactions: vec![],
        };
        
        let work_receipt = block.compute_work_receipt();
        assert_eq!(work_receipt.len(), 64); // SHA256 hex = 64 chars
    }
    
    #[test]
    fn test_proof_of_work_validation() {
        let block = Block {
            height: 1,
            hash: "0000abc123".to_string(), // 4 leading zeros
            previous_hash: "genesis".to_string(),
            merkle_root: "merkle".to_string(),
            timestamp: 1234567890,
            nonce: 12345,
            difficulty: 4.0, // Requires 4 leading zeros
            miner: "test_miner".to_string(),
            work_receipt: String::new(),
            transactions: vec![],
        };
        
        assert!(block.verify_proof_of_work());
        
        let invalid_block = Block {
            difficulty: 5.0, // Requires 5 leading zeros, but hash only has 4
            ..block
        };
        
        assert!(!invalid_block.verify_proof_of_work());
    }
    
    #[test]
    fn test_block_reward_halving() {
        assert_eq!(Block::calculate_block_reward(0), 50.0);
        assert_eq!(Block::calculate_block_reward(1051200), 25.0);
        assert_eq!(Block::calculate_block_reward(2102400), 12.5);
    }
}
