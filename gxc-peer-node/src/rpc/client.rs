use anyhow::{Result, Context};
use serde::{Deserialize, Serialize};
use serde_json::json;
use log::{debug, error};

use crate::blockchain::block::Block;

/// RPC client for communicating with GXC main node
pub struct RpcClient {
    url: String,
    client: reqwest::Client,
}

#[derive(Debug, Serialize)]
struct RpcRequest {
    jsonrpc: String,
    method: String,
    params: serde_json::Value,
    id: u32,
}

#[derive(Debug, Deserialize)]
struct RpcResponse<T> {
    result: Option<T>,
    error: Option<RpcError>,
    id: u32,
}

#[derive(Debug, Deserialize)]
struct RpcError {
    code: i32,
    message: String,
}

impl RpcClient {
    pub fn new(url: String) -> Self {
        Self {
            url,
            client: reqwest::Client::new(),
        }
    }
    
    /// Make RPC call to node
    async fn call<T: for<'de> Deserialize<'de>>(
        &self,
        method: &str,
        params: serde_json::Value,
    ) -> Result<T> {
        let request = RpcRequest {
            jsonrpc: "2.0".to_string(),
            method: method.to_string(),
            params,
            id: 1,
        };
        
        debug!("RPC call: {}", method);
        
        let response = self.client
            .post(&self.url)
            .json(&request)
            .send()
            .await
            .context("Failed to send RPC request")?;
        
        let rpc_response: RpcResponse<T> = response
            .json()
            .await
            .context("Failed to parse RPC response")?;
        
        if let Some(error) = rpc_response.error {
            anyhow::bail!("RPC error {}: {}", error.code, error.message);
        }
        
        rpc_response.result
            .ok_or_else(|| anyhow::anyhow!("RPC response missing result"))
    }
    
    /// Get blockchain info
    pub async fn get_info(&self) -> Result<BlockchainInfo> {
        self.call("getinfo", json!([])).await
    }
    
    /// Get block count (height)
    pub async fn get_block_count(&self) -> Result<u32> {
        self.call("getblockcount", json!([])).await
    }
    
    /// Get block by height
    pub async fn get_block(&self, height: u32) -> Result<Block> {
        self.call("getblock", json!([height])).await
    }
    
    /// Get block by hash
    pub async fn get_block_by_hash(&self, hash: &str) -> Result<Block> {
        self.call("getblock", json!([hash])).await
    }
    
    /// Get transaction by hash
    pub async fn get_transaction(&self, hash: &str) -> Result<serde_json::Value> {
        self.call("gettransaction", json!([hash])).await
    }
    
    /// Get balance for address
    pub async fn get_balance(&self, address: &str) -> Result<f64> {
        self.call("getbalance", json!([address])).await
    }
    
    /// Submit block to node
    pub async fn submit_block(&self, block: &Block) -> Result<()> {
        let _: serde_json::Value = self.call("submitblock", json!([block])).await?;
        Ok(())
    }
    
    /// Get mempool transactions
    pub async fn get_mempool(&self) -> Result<Vec<String>> {
        self.call("getrawmempool", json!([])).await
    }
    
    /// Get peer info
    pub async fn get_peer_info(&self) -> Result<Vec<PeerInfo>> {
        self.call("getpeerinfo", json!([])).await
    }
    
    /// Get mining info
    pub async fn get_mining_info(&self) -> Result<MiningInfo> {
        self.call("getmininginfo", json!([])).await
    }
    
    /// Check if node is healthy
    pub async fn health_check(&self) -> Result<bool> {
        match self.get_block_count().await {
            Ok(_) => Ok(true),
            Err(e) => {
                error!("Health check failed: {}", e);
                Ok(false)
            }
        }
    }
}

#[derive(Debug, Deserialize)]
pub struct BlockchainInfo {
    pub chain: String,
    pub blocks: u32,
    pub headers: u32,
    pub bestblockhash: String,
    pub difficulty: f64,
    pub mediantime: u64,
    pub verificationprogress: f64,
    pub chainwork: String,
}

#[derive(Debug, Deserialize)]
pub struct PeerInfo {
    pub id: u32,
    pub addr: String,
    pub version: u32,
    pub subver: String,
    pub inbound: bool,
    pub startingheight: u32,
    pub synced_blocks: u32,
}

#[derive(Debug, Deserialize)]
pub struct MiningInfo {
    pub blocks: u32,
    pub currentblocksize: u64,
    pub currentblocktx: u32,
    pub difficulty: f64,
    pub networkhashps: f64,
    pub pooledtx: u32,
    pub chain: String,
}

#[cfg(test)]
mod tests {
    use super::*;
    
    #[tokio::test]
    async fn test_rpc_client_creation() {
        let client = RpcClient::new("http://localhost:18332".to_string());
        assert_eq!(client.url, "http://localhost:18332");
    }
}
