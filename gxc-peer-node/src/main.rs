mod p2p;
mod blockchain;
mod rpc;
mod config;

use clap::{Parser, Subcommand};
use anyhow::Result;
use log::{info, error};
use std::path::PathBuf;

use config::Config;
use blockchain::BlockchainSync;
use p2p::P2PNetwork;

#[derive(Parser)]
#[command(name = "gxc-peer-node")]
#[command(about = "GXC Blockchain Peer Node", long_about = None)]
struct Cli {
    /// Config file path
    #[arg(short, long, value_name = "FILE")]
    config: Option<PathBuf>,
    
    /// Network (mainnet or testnet)
    #[arg(short, long)]
    network: Option<String>,
    
    /// RPC node URL
    #[arg(short, long)]
    rpc_url: Option<String>,
    
    /// P2P listen port
    #[arg(short, long)]
    port: Option<u16>,
    
    #[command(subcommand)]
    command: Option<Commands>,
}

#[derive(Subcommand)]
enum Commands {
    /// Start the peer node
    Start,
    
    /// Sync blockchain from main node
    Sync,
    
    /// Verify blockchain integrity
    Verify,
    
    /// Show blockchain stats
    Stats,
    
    /// Generate default config file
    InitConfig {
        /// Output path for config file
        #[arg(short, long, default_value = "config.toml")]
        output: PathBuf,
    },
    
    /// Connect to a specific peer
    Connect {
        /// Peer address (host:port)
        address: String,
    },
}

#[tokio::main]
async fn main() -> Result<()> {
    // Initialize logger
    env_logger::Builder::from_env(env_logger::Env::default().default_filter_or("info"))
        .init();
    
    let cli = Cli::parse();
    
    // Load or create config
    let mut config = if let Some(config_path) = cli.config {
        Config::load(&config_path)?
    } else {
        Config::default()
    };
    
    // Override with CLI args
    if let Some(network) = cli.network {
        config.network.name = network;
    }
    
    if let Some(rpc_url) = cli.rpc_url {
        config.rpc.node_url = rpc_url;
    }
    
    if let Some(port) = cli.port {
        config.p2p.listen_port = port;
    }
    
    // Execute command
    match cli.command {
        Some(Commands::Start) => start_node(config).await,
        Some(Commands::Sync) => sync_blockchain(config).await,
        Some(Commands::Verify) => verify_blockchain(config).await,
        Some(Commands::Stats) => show_stats(config).await,
        Some(Commands::InitConfig { output }) => init_config(output),
        Some(Commands::Connect { address }) => connect_peer(config, address).await,
        None => start_node(config).await, // Default: start node
    }
}

async fn start_node(config: Config) -> Result<()> {
    info!("Starting GXC Peer Node");
    info!("Network: {}", config.network.name);
    info!("Node ID: {}", config.node.node_id);
    info!("P2P Listen: {}:{}", config.p2p.listen_addr, config.p2p.listen_port);
    info!("RPC Node: {}", config.rpc.node_url);
    
    // Initialize blockchain sync
    let blockchain = BlockchainSync::new(
        config.network.name.clone(),
        config.rpc.node_url.clone(),
    );
    
    // Sync blockchain
    info!("Syncing blockchain...");
    if let Err(e) = blockchain.sync_from_node().await {
        error!("Failed to sync blockchain: {}", e);
        return Err(e);
    }
    
    // Initialize P2P network
    let listen_addr = format!("{}:{}", config.p2p.listen_addr, config.p2p.listen_port);
    let p2p = P2PNetwork::new(
        config.node.node_id.clone(),
        listen_addr,
        config.network.name.clone(),
    );
    
    // Connect to bootstrap peers
    for peer_addr in &config.p2p.bootstrap_peers {
        info!("Connecting to bootstrap peer: {}", peer_addr);
        
        let parts: Vec<&str> = peer_addr.split(':').collect();
        if parts.len() == 2 {
            let host = parts[0];
            let port: u16 = parts[1].parse().unwrap_or(18333);
            
            if let Err(e) = p2p.connect_to_peer(host, port).await {
                error!("Failed to connect to {}: {}", peer_addr, e);
            }
        }
    }
    
    // Update P2P network with current height
    let height = blockchain.get_height().await;
    p2p.update_best_height(height).await;
    
    info!("✅ Peer node started successfully");
    info!("Current height: {}", height);
    info!("Connected peers: {}", p2p.peer_count().await);
    
    // Start P2P listener
    p2p.start().await?;
    
    Ok(())
}

async fn sync_blockchain(config: Config) -> Result<()> {
    info!("Syncing blockchain from {}", config.rpc.node_url);
    
    let blockchain = BlockchainSync::new(
        config.network.name.clone(),
        config.rpc.node_url.clone(),
    );
    
    blockchain.sync_from_node().await?;
    
    let height = blockchain.get_height().await;
    info!("✅ Sync complete. Height: {}", height);
    
    Ok(())
}

async fn verify_blockchain(config: Config) -> Result<()> {
    info!("Verifying blockchain...");
    
    let blockchain = BlockchainSync::new(
        config.network.name.clone(),
        config.rpc.node_url.clone(),
    );
    
    // Load existing blockchain
    blockchain.sync_from_node().await?;
    
    // Verify
    blockchain.verify_chain().await?;
    
    info!("✅ Blockchain verification complete");
    
    Ok(())
}

async fn show_stats(config: Config) -> Result<()> {
    let blockchain = BlockchainSync::new(
        config.network.name.clone(),
        config.rpc.node_url.clone(),
    );
    
    blockchain.sync_from_node().await?;
    
    let stats = blockchain.get_stats().await;
    
    println!("\n╔════════════════════════════════════════╗");
    println!("║       GXC Blockchain Statistics       ║");
    println!("╠════════════════════════════════════════╣");
    println!("║ Height:            {:>18} ║", stats.height);
    println!("║ Total Transactions: {:>17} ║", stats.total_transactions);
    println!("║ Avg Difficulty:    {:>18.2} ║", stats.avg_difficulty);
    println!("╚════════════════════════════════════════╝\n");
    
    Ok(())
}

fn init_config(output: PathBuf) -> Result<()> {
    info!("Creating default config at {:?}", output);
    
    Config::create_default(&output)?;
    
    info!("✅ Config file created");
    info!("Edit the file and run: gxc-peer-node --config {:?} start", output);
    
    Ok(())
}

async fn connect_peer(config: Config, address: String) -> Result<()> {
    info!("Connecting to peer: {}", address);
    
    let parts: Vec<&str> = address.split(':').collect();
    if parts.len() != 2 {
        anyhow::bail!("Invalid address format. Use host:port");
    }
    
    let host = parts[0];
    let port: u16 = parts[1].parse()?;
    
    let listen_addr = format!("{}:{}", config.p2p.listen_addr, config.p2p.listen_port);
    let p2p = P2PNetwork::new(
        config.node.node_id.clone(),
        listen_addr,
        config.network.name.clone(),
    );
    
    p2p.connect_to_peer(host, port).await?;
    
    info!("✅ Connected to peer");
    info!("Peer count: {}", p2p.peer_count().await);
    
    // Keep connection alive
    tokio::signal::ctrl_c().await?;
    
    Ok(())
}
