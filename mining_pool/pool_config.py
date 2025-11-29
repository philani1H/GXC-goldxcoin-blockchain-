#!/usr/bin/env python3
"""
GXC Mining Pool Configuration
Public endpoints for third-party miners
"""

import os

# Pool Public Endpoints (for third-party miners)
# These should be set to your public pool URLs

# General Pool (All Algorithms)
GENERAL_POOL_STRATUM_URL = os.environ.get(
    'GENERAL_POOL_STRATUM_URL',
    os.environ.get('POOL_STRATUM_URL', 'stratum+tcp://pool.gxc.network:3333')
)
GENERAL_POOL_DASHBOARD_URL = os.environ.get(
    'GENERAL_POOL_DASHBOARD_URL',
    os.environ.get('POOL_DASHBOARD_URL', 'https://pool.gxc.network')
)
GENERAL_POOL_HOST = os.environ.get('GENERAL_POOL_HOST', '0.0.0.0')  # Listen on all interfaces
GENERAL_POOL_STRATUM_PORT = int(os.environ.get('GENERAL_POOL_STRATUM_PORT', 3333))
GENERAL_POOL_DASHBOARD_PORT = int(os.environ.get('GENERAL_POOL_DASHBOARD_PORT', 6000))

# GPU Pool (Ethash)
GPU_POOL_STRATUM_URL = os.environ.get(
    'GPU_POOL_STRATUM_URL',
    'stratum+tcp://gpu-pool.gxc.network:3334'
)
GPU_POOL_DASHBOARD_URL = os.environ.get(
    'GPU_POOL_DASHBOARD_URL',
    'https://gpu-pool.gxc.network'
)
GPU_POOL_HOST = os.environ.get('GPU_POOL_HOST', '0.0.0.0')
GPU_POOL_STRATUM_PORT = int(os.environ.get('GPU_POOL_STRATUM_PORT', 3334))
GPU_POOL_DASHBOARD_PORT = int(os.environ.get('GPU_POOL_DASHBOARD_PORT', 6001))

# ASIC Pool (SHA-256)
ASIC_POOL_STRATUM_URL = os.environ.get(
    'ASIC_POOL_STRATUM_URL',
    'stratum+tcp://asic-pool.gxc.network:3335'
)
ASIC_POOL_DASHBOARD_URL = os.environ.get(
    'ASIC_POOL_DASHBOARD_URL',
    'https://asic-pool.gxc.network'
)
ASIC_POOL_HOST = os.environ.get('ASIC_POOL_HOST', '0.0.0.0')
ASIC_POOL_STRATUM_PORT = int(os.environ.get('ASIC_POOL_STRATUM_PORT', 3335))
ASIC_POOL_DASHBOARD_PORT = int(os.environ.get('ASIC_POOL_DASHBOARD_PORT', 6002))

# Railway Node URL (Testnet) - Default for all services
RAILWAY_NODE_URL = os.environ.get(
    'RAILWAY_NODE_URL',
    'https://gxc-chain112-blockchain-node-production.up.railway.app'
)

# Blockchain Node (Railway Testnet) - Use RAILWAY_NODE_URL
BLOCKCHAIN_NODE_URL = os.environ.get(
    'BLOCKCHAIN_NODE_URL',
    os.environ.get('RAILWAY_NODE_URL', RAILWAY_NODE_URL)
)

def get_pool_stratum_url(pool_type='general'):
    """Get Stratum URL for pool type"""
    if pool_type == 'gpu':
        return GPU_POOL_STRATUM_URL
    elif pool_type == 'asic':
        return ASIC_POOL_STRATUM_URL
    else:
        return GENERAL_POOL_STRATUM_URL

def get_pool_dashboard_url(pool_type='general'):
    """Get dashboard URL for pool type"""
    if pool_type == 'gpu':
        return GPU_POOL_DASHBOARD_URL
    elif pool_type == 'asic':
        return ASIC_POOL_DASHBOARD_URL
    else:
        return GENERAL_POOL_DASHBOARD_URL

def print_pool_config(pool_type='general'):
    """Print pool configuration"""
    print("=" * 60)
    print(f"GXC {pool_type.upper()} Mining Pool Configuration")
    print("=" * 60)
    
    if pool_type == 'gpu':
        print(f"Stratum URL: {GPU_POOL_STRATUM_URL}")
        print(f"Dashboard: {GPU_POOL_DASHBOARD_URL}")
        print(f"Stratum Port: {GPU_POOL_STRATUM_PORT}")
        print(f"Dashboard Port: {GPU_POOL_DASHBOARD_PORT}")
    elif pool_type == 'asic':
        print(f"Stratum URL: {ASIC_POOL_STRATUM_URL}")
        print(f"Dashboard: {ASIC_POOL_DASHBOARD_URL}")
        print(f"Stratum Port: {ASIC_POOL_STRATUM_PORT}")
        print(f"Dashboard Port: {ASIC_POOL_DASHBOARD_PORT}")
    else:
        print(f"Stratum URL: {GENERAL_POOL_STRATUM_URL}")
        print(f"Dashboard: {GENERAL_POOL_DASHBOARD_URL}")
        print(f"Stratum Port: {GENERAL_POOL_STRATUM_PORT}")
        print(f"Dashboard Port: {GENERAL_POOL_DASHBOARD_PORT}")
    
    print(f"Blockchain Node: {BLOCKCHAIN_NODE_URL}")
    print(f"Railway Node URL: {RAILWAY_NODE_URL}")
    print(f"Host: {GENERAL_POOL_HOST} (0.0.0.0 = public access)")
    print("=" * 60)

if __name__ == '__main__':
    print_pool_config('general')
    print()
    print_pool_config('gpu')
    print()
    print_pool_config('asic')
