use serde::{Deserialize, Serialize};

/// P2P message types for GXC network
#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(tag = "type")]
pub enum Message {
    /// Handshake message to establish connection
    Handshake {
        version: u32,
        node_id: String,
        best_height: u32,
        network: String, // "mainnet" or "testnet"
    },
    
    /// Request peer list
    GetPeers,
    
    /// Response with peer list
    Peers {
        peers: Vec<PeerInfo>,
    },
    
    /// Request blocks starting from height
    GetBlocks {
        start_height: u32,
        count: u32,
    },
    
    /// Response with blocks
    Blocks {
        blocks: Vec<BlockData>,
    },
    
    /// Request specific block by hash
    GetBlock {
        hash: String,
    },
    
    /// Response with single block
    Block {
        block: BlockData,
    },
    
    /// Announce new block
    NewBlock {
        block: BlockData,
    },
    
    /// Request transaction by hash
    GetTransaction {
        hash: String,
    },
    
    /// Response with transaction
    Transaction {
        tx: TransactionData,
    },
    
    /// Announce new transaction
    NewTransaction {
        tx: TransactionData,
    },
    
    /// Ping to keep connection alive
    Ping {
        timestamp: u64,
    },
    
    /// Pong response
    Pong {
        timestamp: u64,
    },
    
    /// Error message
    Error {
        code: u32,
        message: String,
    },
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PeerInfo {
    pub address: String,
    pub port: u16,
    pub node_id: String,
    pub version: u32,
    pub best_height: u32,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct BlockData {
    pub height: u32,
    pub hash: String,
    pub previous_hash: String,
    pub merkle_root: String,
    pub timestamp: u64,
    pub nonce: u64,
    pub difficulty: f64,
    pub miner: String,
    pub work_receipt: String,
    pub transactions: Vec<TransactionData>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct TransactionData {
    pub hash: String,
    pub from: String,
    pub to: String,
    pub amount: f64,
    pub fee: f64,
    pub timestamp: u64,
    pub signature: String,
    pub is_coinbase: bool,
}

impl Message {
    /// Serialize message to JSON
    pub fn to_json(&self) -> Result<String, serde_json::Error> {
        serde_json::to_string(self)
    }
    
    /// Deserialize message from JSON
    pub fn from_json(json: &str) -> Result<Self, serde_json::Error> {
        serde_json::from_str(json)
    }
    
    /// Get message type as string
    pub fn message_type(&self) -> &str {
        match self {
            Message::Handshake { .. } => "Handshake",
            Message::GetPeers => "GetPeers",
            Message::Peers { .. } => "Peers",
            Message::GetBlocks { .. } => "GetBlocks",
            Message::Blocks { .. } => "Blocks",
            Message::GetBlock { .. } => "GetBlock",
            Message::Block { .. } => "Block",
            Message::NewBlock { .. } => "NewBlock",
            Message::GetTransaction { .. } => "GetTransaction",
            Message::Transaction { .. } => "Transaction",
            Message::NewTransaction { .. } => "NewTransaction",
            Message::Ping { .. } => "Ping",
            Message::Pong { .. } => "Pong",
            Message::Error { .. } => "Error",
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    
    #[test]
    fn test_message_serialization() {
        let msg = Message::Handshake {
            version: 1,
            node_id: "test-node".to_string(),
            best_height: 100,
            network: "testnet".to_string(),
        };
        
        let json = msg.to_json().unwrap();
        let decoded = Message::from_json(&json).unwrap();
        
        assert_eq!(msg.message_type(), decoded.message_type());
    }
}
