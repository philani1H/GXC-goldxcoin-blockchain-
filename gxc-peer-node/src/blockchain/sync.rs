use std::sync::Arc;
use tokio::sync::RwLock;
use anyhow::{Result, Context};
use log::{info, warn, error};

use super::block::Block;
use super::validator::BlockValidator;
use crate::rpc::RpcClient;

/// Blockchain synchronization manager
pub struct BlockchainSync {
    blocks: Arc<RwLock<Vec<Block>>>,
    validator: BlockValidator,
    rpc_client: Arc<RpcClient>,
}

impl BlockchainSync {
    pub fn new(network: String, rpc_url: String) -> Self {
        Self {
            blocks: Arc::new(RwLock::new(Vec::new())),
            validator: BlockValidator::new(network),
            rpc_client: Arc::new(RpcClient::new(rpc_url)),
        }
    }
    
    /// Get current blockchain height
    pub async fn get_height(&self) -> u32 {
        let blocks = self.blocks.read().await;
        blocks.len() as u32
    }
    
    /// Get block by height
    pub async fn get_block(&self, height: u32) -> Option<Block> {
        let blocks = self.blocks.read().await;
        blocks.get(height as usize).cloned()
    }
    
    /// Get latest block
    pub async fn get_latest_block(&self) -> Option<Block> {
        let blocks = self.blocks.read().await;
        blocks.last().cloned()
    }
    
    /// Add block to chain
    pub async fn add_block(&self, block: Block) -> Result<()> {
        // Get previous block for validation
        let previous_block = if block.height > 0 {
            self.get_block(block.height - 1).await
        } else {
            None
        };
        
        // Validate block
        self.validator.validate_block(&block, previous_block.as_ref())
            .context("Block validation failed")?;
        
        // Add to chain
        let mut blocks = self.blocks.write().await;
        
        // Ensure we're adding blocks in order
        if block.height as usize != blocks.len() {
            anyhow::bail!(
                "Block height mismatch: expected {}, got {}",
                blocks.len(),
                block.height
            );
        }
        
        blocks.push(block.clone());
        
        info!("✅ Block {} added to chain", block.height);
        
        Ok(())
    }
    
    /// Sync blockchain from main node
    pub async fn sync_from_node(&self) -> Result<()> {
        info!("Starting blockchain sync...");
        
        // Get node's best height
        let node_height = self.rpc_client.get_block_count().await
            .context("Failed to get node height")?;
        
        let our_height = self.get_height().await;
        
        if node_height <= our_height {
            info!("Already synced (our: {}, node: {})", our_height, node_height);
            return Ok(());
        }
        
        info!("Syncing from height {} to {}", our_height, node_height);
        
        // Fetch blocks in batches
        const BATCH_SIZE: u32 = 100;
        
        for start_height in (our_height..node_height).step_by(BATCH_SIZE as usize) {
            let end_height = std::cmp::min(start_height + BATCH_SIZE, node_height);
            
            info!("Fetching blocks {} to {}", start_height, end_height);
            
            for height in start_height..end_height {
                match self.rpc_client.get_block(height).await {
                    Ok(block) => {
                        if let Err(e) = self.add_block(block).await {
                            error!("Failed to add block {}: {}", height, e);
                            return Err(e);
                        }
                    }
                    Err(e) => {
                        error!("Failed to fetch block {}: {}", height, e);
                        return Err(e);
                    }
                }
            }
            
            let progress = ((end_height as f64 / node_height as f64) * 100.0) as u32;
            info!("Sync progress: {}%", progress);
        }
        
        info!("✅ Blockchain sync complete");
        
        Ok(())
    }
    
    /// Verify entire blockchain
    pub async fn verify_chain(&self) -> Result<()> {
        info!("Verifying blockchain...");
        
        let blocks = self.blocks.read().await;
        
        for (i, block) in blocks.iter().enumerate() {
            let previous_block = if i > 0 {
                Some(&blocks[i - 1])
            } else {
                None
            };
            
            self.validator.validate_block(block, previous_block)
                .context(format!("Block {} validation failed", i))?;
        }
        
        info!("✅ Blockchain verified ({} blocks)", blocks.len());
        
        Ok(())
    }
    
    /// Get blockchain statistics
    pub async fn get_stats(&self) -> BlockchainStats {
        let blocks = self.blocks.read().await;
        
        let total_blocks = blocks.len();
        let total_transactions: usize = blocks.iter()
            .map(|b| b.transactions.len())
            .sum();
        
        let avg_difficulty = if !blocks.is_empty() {
            blocks.iter().map(|b| b.difficulty).sum::<f64>() / blocks.len() as f64
        } else {
            0.0
        };
        
        BlockchainStats {
            height: total_blocks as u32,
            total_transactions,
            avg_difficulty,
        }
    }
}

#[derive(Debug)]
pub struct BlockchainStats {
    pub height: u32,
    pub total_transactions: usize,
    pub avg_difficulty: f64,
}

#[cfg(test)]
mod tests {
    use super::*;
    
    #[tokio::test]
    async fn test_blockchain_sync() {
        let sync = BlockchainSync::new(
            "testnet".to_string(),
            "http://localhost:18332".to_string()
        );
        
        assert_eq!(sync.get_height().await, 0);
    }
}
