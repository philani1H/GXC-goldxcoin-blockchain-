use std::collections::HashMap;
use std::sync::Arc;
use tokio::sync::RwLock;
use tokio::net::{TcpListener, TcpStream};
use anyhow::{Result, Context};
use log::{info, warn, error, debug};

use super::peer::Peer;
use super::message::{Message, PeerInfo, BlockData};

/// P2P Network manager
pub struct P2PNetwork {
    node_id: String,
    listen_addr: String,
    network: String, // "mainnet" or "testnet"
    peers: Arc<RwLock<HashMap<String, Arc<Peer>>>>,
    best_height: Arc<RwLock<u32>>,
}

impl P2PNetwork {
    /// Create new P2P network
    pub fn new(node_id: String, listen_addr: String, network: String) -> Self {
        Self {
            node_id,
            listen_addr,
            network,
            peers: Arc::new(RwLock::new(HashMap::new())),
            best_height: Arc::new(RwLock::new(0)),
        }
    }
    
    /// Start listening for incoming connections
    pub async fn start(&self) -> Result<()> {
        let listener = TcpListener::bind(&self.listen_addr).await
            .context("Failed to bind listener")?;
        
        info!("P2P network listening on {}", self.listen_addr);
        
        loop {
            match listener.accept().await {
                Ok((stream, addr)) => {
                    info!("New connection from {}", addr);
                    
                    let peers = self.peers.clone();
                    let node_id = self.node_id.clone();
                    let best_height = *self.best_height.read().await;
                    let network = self.network.clone();
                    
                    tokio::spawn(async move {
                        if let Err(e) = Self::handle_connection(
                            stream,
                            addr.to_string(),
                            peers,
                            &node_id,
                            best_height,
                            &network,
                        ).await {
                            error!("Connection error: {}", e);
                        }
                    });
                }
                Err(e) => {
                    error!("Failed to accept connection: {}", e);
                }
            }
        }
    }
    
    /// Handle incoming connection
    async fn handle_connection(
        stream: TcpStream,
        addr: String,
        peers: Arc<RwLock<HashMap<String, Arc<Peer>>>>,
        node_id: &str,
        best_height: u32,
        network: &str,
    ) -> Result<()> {
        let peer_info = PeerInfo {
            address: addr.clone(),
            port: 0, // Will be updated from handshake
            node_id: String::new(),
            version: 1,
            best_height: 0,
        };
        
        let peer = Arc::new(Peer::new(stream, peer_info));
        
        // Perform handshake
        peer.handshake(node_id, best_height, network).await?;
        
        // Add to peer list
        {
            let mut peers_lock = peers.write().await;
            peers_lock.insert(addr.clone(), peer.clone());
        }
        
        info!("Peer {} added to network", addr);
        
        // Handle messages from this peer
        loop {
            match peer.receive_message().await {
                Ok(message) => {
                    debug!("Received {} from {}", message.message_type(), addr);
                    
                    // Handle message
                    if let Err(e) = Self::handle_message(message, &peer).await {
                        error!("Error handling message: {}", e);
                    }
                }
                Err(e) => {
                    warn!("Connection closed with {}: {}", addr, e);
                    break;
                }
            }
        }
        
        // Remove peer
        {
            let mut peers_lock = peers.write().await;
            peers_lock.remove(&addr);
        }
        
        info!("Peer {} removed from network", addr);
        
        Ok(())
    }
    
    /// Handle received message
    async fn handle_message(message: Message, peer: &Arc<Peer>) -> Result<()> {
        match message {
            Message::Ping { timestamp } => {
                // Respond with pong
                let pong = Message::Pong { timestamp };
                peer.send_message(&pong).await?;
            }
            
            Message::GetPeers => {
                // Send peer list (not implemented yet)
                let peers = Message::Peers { peers: vec![] };
                peer.send_message(&peers).await?;
            }
            
            Message::GetBlocks { start_height, count } => {
                debug!("Peer requested blocks from {} (count: {})", start_height, count);
                // TODO: Fetch blocks from blockchain and send
            }
            
            Message::NewBlock { block } => {
                info!("Received new block at height {}", block.height);
                // TODO: Validate and add block to blockchain
            }
            
            Message::NewTransaction { tx } => {
                debug!("Received new transaction {}", tx.hash);
                // TODO: Validate and add to mempool
            }
            
            _ => {
                debug!("Unhandled message type: {}", message.message_type());
            }
        }
        
        Ok(())
    }
    
    /// Connect to a peer
    pub async fn connect_to_peer(&self, address: &str, port: u16) -> Result<()> {
        let addr = format!("{}:{}", address, port);
        
        info!("Connecting to peer {}", addr);
        
        let stream = TcpStream::connect(&addr).await
            .context("Failed to connect to peer")?;
        
        let peer_info = PeerInfo {
            address: address.to_string(),
            port,
            node_id: String::new(),
            version: 1,
            best_height: 0,
        };
        
        let peer = Arc::new(Peer::new(stream, peer_info));
        
        // Perform handshake
        let best_height = *self.best_height.read().await;
        peer.handshake(&self.node_id, best_height, &self.network).await?;
        
        // Add to peer list
        {
            let mut peers = self.peers.write().await;
            peers.insert(addr.clone(), peer.clone());
        }
        
        info!("Connected to peer {}", addr);
        
        // Start message handler
        let peers = self.peers.clone();
        let addr_clone = addr.clone();
        tokio::spawn(async move {
            loop {
                match peer.receive_message().await {
                    Ok(message) => {
                        if let Err(e) = Self::handle_message(message, &peer).await {
                            error!("Error handling message: {}", e);
                        }
                    }
                    Err(e) => {
                        warn!("Connection closed with {}: {}", addr_clone, e);
                        break;
                    }
                }
            }
            
            // Remove peer
            let mut peers_lock = peers.write().await;
            peers_lock.remove(&addr_clone);
        });
        
        Ok(())
    }
    
    /// Broadcast message to all peers
    pub async fn broadcast(&self, message: &Message) -> Result<()> {
        let peers = self.peers.read().await;
        
        for (addr, peer) in peers.iter() {
            if let Err(e) = peer.send_message(message).await {
                warn!("Failed to send to {}: {}", addr, e);
            }
        }
        
        Ok(())
    }
    
    /// Get number of connected peers
    pub async fn peer_count(&self) -> usize {
        self.peers.read().await.len()
    }
    
    /// Update best height
    pub async fn update_best_height(&self, height: u32) {
        let mut best_height = self.best_height.write().await;
        *best_height = height;
    }
    
    /// Request blocks from peers
    pub async fn request_blocks(&self, start_height: u32, count: u32) -> Result<()> {
        let message = Message::GetBlocks { start_height, count };
        self.broadcast(&message).await
    }
    
    /// Announce new block to network
    pub async fn announce_block(&self, block: BlockData) -> Result<()> {
        let message = Message::NewBlock { block };
        self.broadcast(&message).await
    }
}
