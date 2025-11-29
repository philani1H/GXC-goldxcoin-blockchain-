#!/usr/bin/env python3
"""
GXC General Mining Pool (All Algorithms)
Public endpoint for third-party miners
"""

import os
import sys

# Add current directory to path
current_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, current_dir)

from pool_base import MiningPool
from pool_config import (
    GENERAL_POOL_STRATUM_URL,
    GENERAL_POOL_DASHBOARD_URL,
    GENERAL_POOL_HOST,
    GENERAL_POOL_STRATUM_PORT,
    GENERAL_POOL_DASHBOARD_PORT,
    BLOCKCHAIN_NODE_URL
)

# Create pool instance (for Vercel deployment)
try:
    pool = MiningPool(
        pool_name='pool',
        algorithm='all',
        port=GENERAL_POOL_STRATUM_PORT,
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
    print("GXC General Mining Pool (All Algorithms)")
    print("=" * 60)
    print(f"Public Stratum Endpoint: {GENERAL_POOL_STRATUM_URL}")
    print(f"Public Dashboard: {GENERAL_POOL_DASHBOARD_URL}")
    print(f"Stratum Port: {GENERAL_POOL_STRATUM_PORT} (listening on {GENERAL_POOL_HOST})")
    print(f"Dashboard Port: {GENERAL_POOL_DASHBOARD_PORT}")
    print(f"Algorithms: SHA-256, Ethash, GXHash")
    print(f"Blockchain Node: {BLOCKCHAIN_NODE_URL}")
    print("=" * 60)
    print()
    print("✅ Pool is PUBLIC - Third-party miners can connect!")
    print(f"   Connect with: {GENERAL_POOL_STRATUM_URL}")
    print("=" * 60)
    
    pool.run(host=GENERAL_POOL_HOST, port=GENERAL_POOL_DASHBOARD_PORT, debug=False, allow_unsafe_werkzeug=True)

