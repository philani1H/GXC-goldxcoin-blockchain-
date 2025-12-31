use serde::{Deserialize, Serialize};
use std::fs;
use std::path::Path;
use anyhow::{Result, Context};

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Config {
    pub network: NetworkConfig,
    pub p2p: P2PConfig,
    pub rpc: RpcConfig,
    pub node: NodeConfig,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct NetworkConfig {
    pub name: String, // "mainnet" or "testnet"
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct P2PConfig {
    pub listen_addr: String,
    pub listen_port: u16,
    pub max_peers: usize,
    pub bootstrap_peers: Vec<String>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct RpcConfig {
    pub node_url: String,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct NodeConfig {
    pub node_id: String,
    pub data_dir: String,
}

impl Default for Config {
    fn default() -> Self {
        Self {
            network: NetworkConfig {
                name: "testnet".to_string(),
            },
            p2p: P2PConfig {
                listen_addr: "0.0.0.0".to_string(),
                listen_port: 18444,
                max_peers: 50,
                bootstrap_peers: vec![
                    "localhost:18333".to_string(),
                ],
            },
            rpc: RpcConfig {
                node_url: "http://localhost:18332".to_string(),
            },
            node: NodeConfig {
                node_id: format!("gxc-peer-{}", uuid::Uuid::new_v4()),
                data_dir: "./data".to_string(),
            },
        }
    }
}

impl Config {
    /// Load config from file
    pub fn load<P: AsRef<Path>>(path: P) -> Result<Self> {
        let content = fs::read_to_string(path)
            .context("Failed to read config file")?;
        
        let config: Config = toml::from_str(&content)
            .context("Failed to parse config file")?;
        
        Ok(config)
    }
    
    /// Save config to file
    pub fn save<P: AsRef<Path>>(&self, path: P) -> Result<()> {
        let content = toml::to_string_pretty(self)
            .context("Failed to serialize config")?;
        
        fs::write(path, content)
            .context("Failed to write config file")?;
        
        Ok(())
    }
    
    /// Create default config file
    pub fn create_default<P: AsRef<Path>>(path: P) -> Result<()> {
        let config = Config::default();
        config.save(path)
    }
}

// Add uuid dependency
use uuid::Uuid;

#[cfg(test)]
mod tests {
    use super::*;
    
    #[test]
    fn test_default_config() {
        let config = Config::default();
        assert_eq!(config.network.name, "testnet");
        assert_eq!(config.p2p.listen_port, 18444);
    }
}
