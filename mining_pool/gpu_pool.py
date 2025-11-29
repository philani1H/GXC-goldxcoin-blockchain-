#!/usr/bin/env python3
"""
GXC GPU Mining Pool (Ethash)
Public endpoint for third-party GPU miners
"""

import os
import sys

# Add current directory to path
current_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, current_dir)

from pool_base import MiningPool
from pool_config import (
    GPU_POOL_STRATUM_URL,
    GPU_POOL_DASHBOARD_URL,
    GPU_POOL_HOST,
    GPU_POOL_STRATUM_PORT,
    GPU_POOL_DASHBOARD_PORT,
    BLOCKCHAIN_NODE_URL
)

# Create pool instance (for Vercel deployment)
try:
    pool = MiningPool(
        pool_name='gpu-pool',
        algorithm='ethash',
        port=GPU_POOL_STRATUM_PORT,
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
    print("GXC GPU Mining Pool (Ethash)")
    print("=" * 60)
    print(f"Public Stratum Endpoint: {GPU_POOL_STRATUM_URL}")
    print(f"Public Dashboard: {GPU_POOL_DASHBOARD_URL}")
    print(f"Stratum Port: {GPU_POOL_STRATUM_PORT} (listening on {GPU_POOL_HOST})")
    print(f"Dashboard Port: {GPU_POOL_DASHBOARD_PORT}")
    print(f"Algorithm: Ethash (GPU)")
    print(f"Blockchain Node: {BLOCKCHAIN_NODE_URL}")
    print("=" * 60)
    print()
    print("✅ Pool is PUBLIC - Third-party GPU miners can connect!")
    print(f"   Connect with: {GPU_POOL_STRATUM_URL}")
    print("=" * 60)
    
    pool.run(host=GPU_POOL_HOST, port=GPU_POOL_DASHBOARD_PORT, debug=False, allow_unsafe_werkzeug=True)

