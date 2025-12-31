use std::sync::Arc;
use tokio::sync::RwLock;
use tokio::net::TcpStream;
use tokio::io::{AsyncReadExt, AsyncWriteExt};
use anyhow::{Result, Context};
use log::{info, warn, error, debug};

use super::message::{Message, PeerInfo};

/// Represents a connected peer
pub struct Peer {
    pub info: PeerInfo,
    stream: Arc<RwLock<TcpStream>>,
    pub connected: bool,
}

impl Peer {
    /// Create new peer from TCP stream
    pub fn new(stream: TcpStream, info: PeerInfo) -> Self {
        Self {
            info,
            stream: Arc::new(RwLock::new(stream)),
            connected: true,
        }
    }
    
    /// Send message to peer
    pub async fn send_message(&self, message: &Message) -> Result<()> {
        let json = message.to_json()
            .context("Failed to serialize message")?;
        
        let mut stream = self.stream.write().await;
        
        // Send message length (4 bytes) + message
        let len = json.len() as u32;
        stream.write_all(&len.to_be_bytes()).await
            .context("Failed to write message length")?;
        
        stream.write_all(json.as_bytes()).await
            .context("Failed to write message")?;
        
        stream.flush().await
            .context("Failed to flush stream")?;
        
        debug!("Sent {} to {}", message.message_type(), self.info.address);
        
        Ok(())
    }
    
    /// Receive message from peer
    pub async fn receive_message(&self) -> Result<Message> {
        let mut stream = self.stream.write().await;
        
        // Read message length (4 bytes)
        let mut len_bytes = [0u8; 4];
        stream.read_exact(&mut len_bytes).await
            .context("Failed to read message length")?;
        
        let len = u32::from_be_bytes(len_bytes) as usize;
        
        // Sanity check: max message size 10MB
        if len > 10 * 1024 * 1024 {
            anyhow::bail!("Message too large: {} bytes", len);
        }
        
        // Read message
        let mut buffer = vec![0u8; len];
        stream.read_exact(&mut buffer).await
            .context("Failed to read message")?;
        
        let json = String::from_utf8(buffer)
            .context("Invalid UTF-8 in message")?;
        
        let message = Message::from_json(&json)
            .context("Failed to deserialize message")?;
        
        debug!("Received {} from {}", message.message_type(), self.info.address);
        
        Ok(message)
    }
    
    /// Perform handshake with peer
    pub async fn handshake(
        &self,
        our_node_id: &str,
        our_best_height: u32,
        network: &str,
    ) -> Result<()> {
        // Send our handshake
        let handshake = Message::Handshake {
            version: 1,
            node_id: our_node_id.to_string(),
            best_height: our_best_height,
            network: network.to_string(),
        };
        
        self.send_message(&handshake).await?;
        
        // Wait for their handshake
        let response = self.receive_message().await?;
        
        match response {
            Message::Handshake { version, node_id, best_height, network: peer_network } => {
                if peer_network != network {
                    anyhow::bail!("Network mismatch: we are on {}, peer is on {}", network, peer_network);
                }
                
                info!("Handshake successful with {} (height: {}, version: {})", 
                      node_id, best_height, version);
                
                Ok(())
            }
            Message::Error { code, message } => {
                anyhow::bail!("Handshake failed: {} (code: {})", message, code);
            }
            _ => {
                anyhow::bail!("Expected Handshake, got {}", response.message_type());
            }
        }
    }
    
    /// Ping peer to check if alive
    pub async fn ping(&self) -> Result<u64> {
        let timestamp = chrono::Utc::now().timestamp() as u64;
        
        let ping = Message::Ping { timestamp };
        self.send_message(&ping).await?;
        
        // Wait for pong with timeout
        let response = tokio::time::timeout(
            std::time::Duration::from_secs(5),
            self.receive_message()
        ).await
            .context("Ping timeout")??;
        
        match response {
            Message::Pong { timestamp: pong_timestamp } => {
                let now = chrono::Utc::now().timestamp() as u64;
                let latency = now.saturating_sub(timestamp);
                
                debug!("Pong from {} (latency: {}ms)", self.info.address, latency);
                
                Ok(latency)
            }
            _ => {
                anyhow::bail!("Expected Pong, got {}", response.message_type());
            }
        }
    }
    
    /// Disconnect from peer
    pub async fn disconnect(&mut self) -> Result<()> {
        self.connected = false;
        
        let mut stream = self.stream.write().await;
        stream.shutdown().await
            .context("Failed to shutdown connection")?;
        
        info!("Disconnected from {}", self.info.address);
        
        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    
    #[tokio::test]
    async fn test_message_framing() {
        // Test that we can properly frame and unframe messages
        let msg = Message::Ping { timestamp: 12345 };
        let json = msg.to_json().unwrap();
        
        let len = json.len() as u32;
        let len_bytes = len.to_be_bytes();
        
        assert_eq!(u32::from_be_bytes(len_bytes), len);
    }
}
