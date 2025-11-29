#!/usr/bin/env python3
"""
GXC ASIC Mining Pool (SHA-256)
Public endpoint for third-party ASIC miners
"""

import os
import sys

# Add current directory to path
current_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, current_dir)

from pool_base import MiningPool
from pool_config import (
    ASIC_POOL_STRATUM_URL,
    ASIC_POOL_DASHBOARD_URL,
    ASIC_POOL_HOST,
    ASIC_POOL_STRATUM_PORT,
    ASIC_POOL_DASHBOARD_PORT,
    BLOCKCHAIN_NODE_URL,
    RAILWAY_NODE_URL
)

# Create pool instance (for Vercel deployment)
try:
    pool = MiningPool(
        pool_name='asic-pool',
        algorithm='sha256',
        port=ASIC_POOL_STRATUM_PORT,
        rpc_url=BLOCKCHAIN_NODE_URL
    )
    # Expose Flask app for Vercel
    app = pool.app
except Exception as e:
    # If pool creation fails, create a minimal Flask app
    from flask import Flask
    app = Flask(__name__)
    @app.route('/')
    def error():
        return f"Pool initialization error: {str(e)}", 500

if __name__ == '__main__':
    print("=" * 60)
    print("GXC ASIC Mining Pool (SHA-256)")
    print("=" * 60)
    print(f"Public Stratum Endpoint: {ASIC_POOL_STRATUM_URL}")
    print(f"Public Dashboard: {ASIC_POOL_DASHBOARD_URL}")
    print(f"Stratum Port: {ASIC_POOL_STRATUM_PORT} (listening on {ASIC_POOL_HOST})")
    print(f"Dashboard Port: {ASIC_POOL_DASHBOARD_PORT}")
    print(f"Algorithm: SHA-256 (ASIC)")
    print(f"Blockchain Node: {BLOCKCHAIN_NODE_URL}")
    print("=" * 60)
    print()
    print("✅ Pool is PUBLIC - Third-party ASIC miners can connect!")
    print(f"   Connect with: {ASIC_POOL_STRATUM_URL}")
    print("=" * 60)
    
    pool.run(host=ASIC_POOL_HOST, port=ASIC_POOL_DASHBOARD_PORT, debug=False, allow_unsafe_werkzeug=True)

