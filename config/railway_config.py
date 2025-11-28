#!/usr/bin/env python3
"""
GXC Railway Configuration
Central configuration for all services connecting to Railway node
"""

import os

# Railway Node URL
RAILWAY_NODE_URL = "https://gxc-chain112-blockchain-node-production.up.railway.app"

# API Endpoints
RPC_URL = os.environ.get('BLOCKCHAIN_RPC_URL', RAILWAY_NODE_URL)
REST_URL = os.environ.get('BLOCKCHAIN_REST_URL', f"{RAILWAY_NODE_URL}:8080")
WEBSOCKET_URL = os.environ.get('BLOCKCHAIN_WS_URL', f"{RAILWAY_NODE_URL}:8081")

# Network Configuration
NETWORK = os.environ.get('GXC_NETWORK', 'testnet')
NETWORK_CONFIG = {
    'testnet': {
        'address_prefix': 'tGXC',
        'block_time': 60,  # seconds
        'difficulty': 0.1,
        'block_reward': 12.5,
        'rpc_port': 18332,
    },
    'mainnet': {
        'address_prefix': 'GXC',
        'block_time': 600,  # seconds (10 minutes)
        'difficulty': 1.0,
        'block_reward': 50.0,
        'rpc_port': 8332,
    }
}

# Get current network config
CURRENT_NETWORK = NETWORK_CONFIG.get(NETWORK, NETWORK_CONFIG['testnet'])

# Mining Configuration
DEFAULT_MINER_ADDRESS = os.environ.get('MINER_ADDRESS', 'tGXC9fab7317231b966af85ac453e168c0932')

# Explorer Configuration
EXPLORER_URL = os.environ.get('EXPLORER_URL', 'http://localhost:3000')

# Wallet Configuration
WALLET_API_URL = os.environ.get('WALLET_API_URL', 'http://localhost:5000')

# Database Paths
DATABASE_PATHS = {
    'testnet': 'gxc_testnet_data/blockchain.db',
    'mainnet': 'gxc_data/blockchain.db',
    'wallets': 'gxc_wallets.db',
    'wallets_testnet': 'gxc_wallets_testnet.db',
    'explorer': 'gxc_explorer.db',
}

def get_rpc_url():
    """Get RPC URL for current network"""
    return RPC_URL

def get_rest_url():
    """Get REST API URL for current network"""
    return REST_URL

def get_websocket_url():
    """Get WebSocket URL for current network"""
    return WEBSOCKET_URL

def get_network_config():
    """Get configuration for current network"""
    return CURRENT_NETWORK

def get_miner_address():
    """Get default miner address"""
    return DEFAULT_MINER_ADDRESS

def print_config():
    """Print current configuration"""
    print("=" * 60)
    print("GXC Railway Configuration")
    print("=" * 60)
    print(f"Network: {NETWORK}")
    print(f"RPC URL: {RPC_URL}")
    print(f"REST URL: {REST_URL}")
    print(f"WebSocket URL: {WEBSOCKET_URL}")
    print(f"Miner Address: {DEFAULT_MINER_ADDRESS}")
    print(f"Block Time: {CURRENT_NETWORK['block_time']} seconds")
    print(f"Block Reward: {CURRENT_NETWORK['block_reward']} GXC")
    print(f"Difficulty: {CURRENT_NETWORK['difficulty']}")
    print("=" * 60)

if __name__ == '__main__':
    print_config()
