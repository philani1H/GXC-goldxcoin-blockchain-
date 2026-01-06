#!/usr/bin/env python3
"""
GXC Railway Configuration
Central configuration for all services connecting to Railway node
Network-aware configuration supporting both testnet and mainnet
"""

import os

# Network Configuration - Determines which network to use
NETWORK = os.environ.get('GXC_NETWORK', 'testnet').lower()

# Network-specific configuration
NETWORK_CONFIG = {
    'testnet': {
        'address_prefix': 'tGXC',
        'block_time': 120,  # seconds (2 minutes)
        'difficulty': 0.1,
        'block_reward': 50.0,  # Initial block reward (halves every 1,051,200 blocks)
        'rpc_port': 18332,
        'railway_url': 'https://gxc-chain112-blockchain-node-production.up.railway.app',
        'local_url': 'http://localhost:18332',
        'network_name': 'GXC Testnet',
        'chain_id': 'GXC_TESTNET',
        'currency': 'tGXC',
    },
    'mainnet': {
        'address_prefix': 'GXC',
        'block_time': 600,  # seconds (10 minutes)
        'difficulty': 1.0,
        'block_reward': 50.0,
        'rpc_port': 8332,
        'railway_url': None,  # Set when mainnet Railway node is deployed
        'local_url': 'http://localhost:8332',
        'network_name': 'GXC Mainnet',
        'chain_id': 'GXC',
        'currency': 'GXC',
    }
}

# Get current network config
CURRENT_NETWORK = NETWORK_CONFIG.get(NETWORK, NETWORK_CONFIG['testnet'])

# Network-aware Railway Node URLs
RAILWAY_NODE_URL = CURRENT_NETWORK.get('railway_url') or NETWORK_CONFIG['testnet']['railway_url']
LOCAL_NODE_URL = CURRENT_NETWORK['local_url']

# API Endpoints - Network-aware URLs (Railway uses single PORT, no separate ports)
RPC_URL = os.environ.get('BLOCKCHAIN_RPC_URL', os.environ.get('RAILWAY_NODE_URL', RAILWAY_NODE_URL))
REST_URL = os.environ.get('BLOCKCHAIN_REST_URL', os.environ.get('BLOCKCHAIN_RPC_URL', RPC_URL))
WEBSOCKET_URL = os.environ.get('BLOCKCHAIN_WS_URL', os.environ.get('BLOCKCHAIN_RPC_URL', RPC_URL))

# Mining Configuration - Network-aware default addresses
if NETWORK == 'testnet':
    DEFAULT_MINER_ADDRESS = os.environ.get('MINER_ADDRESS', 'tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf')
else:
    DEFAULT_MINER_ADDRESS = os.environ.get('MINER_ADDRESS', 'GXC9fab7317231b966af85ac453e168c0932')

# Explorer Configuration
EXPLORER_URL = os.environ.get('EXPLORER_URL', 'http://localhost:3000')

# Wallet Configuration
WALLET_API_URL = os.environ.get('WALLET_API_URL', 'http://localhost:5000')

# Database Paths - Network-aware database paths
base_db_path = '/tmp' if os.path.exists('/tmp') else '.'
DATABASE_PATHS = {
    'blockchain': {
        'testnet': os.path.join(base_db_path, 'gxc_testnet_data', 'blockchain.db'),
        'mainnet': os.path.join(base_db_path, 'gxc_data', 'blockchain.db'),
    },
    'wallets': {
        'testnet': os.path.join(base_db_path, f'gxc_wallets_testnet.db'),
        'mainnet': os.path.join(base_db_path, f'gxc_wallets.db'),
    },
    'explorer': {
        'testnet': os.path.join(base_db_path, 'gxc_explorer_testnet.db'),
        'mainnet': os.path.join(base_db_path, 'gxc_explorer_mainnet.db'),
    },
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

def get_network():
    """Get current network name"""
    return NETWORK

def get_miner_address():
    """Get default miner address for current network"""
    return DEFAULT_MINER_ADDRESS

def get_database_path(db_type='blockchain', network=None):
    """
    Get database path for specified database type and network
    
    Args:
        db_type: 'blockchain', 'wallets', or 'explorer'
        network: 'testnet' or 'mainnet' (defaults to current network)
    
    Returns:
        Database file path
    """
    if network is None:
        network = NETWORK
    
    if db_type not in DATABASE_PATHS:
        raise ValueError(f"Unknown database type: {db_type}")
    
    if network not in DATABASE_PATHS[db_type]:
        raise ValueError(f"Unknown network: {network}")
    
    return DATABASE_PATHS[db_type][network]

def get_wallet_database_path():
    """Get wallet database path for current network"""
    return get_database_path('wallets', NETWORK)

def get_blockchain_database_path():
    """Get blockchain database path for current network"""
    return get_database_path('blockchain', NETWORK)

def get_explorer_database_path():
    """Get explorer database path for current network"""
    return get_database_path('explorer', NETWORK)

def get_network_info():
    """Get complete network information dictionary"""
    return {
        'network': NETWORK.upper(),
        'network_name': CURRENT_NETWORK['network_name'],
        'chain_id': CURRENT_NETWORK['chain_id'],
        'address_prefix': CURRENT_NETWORK['address_prefix'],
        'currency': CURRENT_NETWORK['currency'],
        'block_time': CURRENT_NETWORK['block_time'],
        'block_reward': CURRENT_NETWORK['block_reward'],
        'difficulty': CURRENT_NETWORK['difficulty'],
        'rpc_url': RPC_URL,
        'rest_url': REST_URL,
        'websocket_url': WEBSOCKET_URL,
        'is_testnet': NETWORK == 'testnet',
        'is_mainnet': NETWORK == 'mainnet',
    }

def print_config():
    """Print current configuration"""
    print("=" * 60)
    print("GXC Railway Configuration")
    print("=" * 60)
    print(f"Network: {NETWORK.upper()}")
    print(f"Network Name: {CURRENT_NETWORK['network_name']}")
    print(f"Chain ID: {CURRENT_NETWORK['chain_id']}")
    print(f"Address Prefix: {CURRENT_NETWORK['address_prefix']}")
    print(f"Currency: {CURRENT_NETWORK['currency']}")
    print(f"RPC URL: {RPC_URL}")
    print(f"REST URL: {REST_URL}")
    print(f"WebSocket URL: {WEBSOCKET_URL}")
    print(f"Miner Address: {DEFAULT_MINER_ADDRESS}")
    print(f"Block Time: {CURRENT_NETWORK['block_time']} seconds")
    print(f"Block Reward: {CURRENT_NETWORK['block_reward']} GXC")
    print(f"Difficulty: {CURRENT_NETWORK['difficulty']}")
    print(f"Wallet DB: {get_wallet_database_path()}")
    print(f"Blockchain DB: {get_blockchain_database_path()}")
    print(f"Explorer DB: {get_explorer_database_path()}")
    print("=" * 60)

if __name__ == '__main__':
    print_config()
