#!/usr/bin/env python3
"""
GXC ASIC Mining Pool (SHA-256)
https://asic-pool.gxc.network
"""

import os
import sys

# Add current directory to path
current_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, current_dir)

from pool_base import MiningPool

# Create pool instance (for Vercel deployment)
try:
    pool = MiningPool(
        pool_name='asic-pool',
        algorithm='sha256',
        port=3333,
        rpc_url=os.environ.get('BLOCKCHAIN_NODE_URL', 'http://localhost:8545')
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
    print(f"Stratum: stratum+tcp://asic-pool.gxc.network:3333")
    print(f"Dashboard: http://localhost:5001")
    print(f"Algorithm: SHA-256 (ASIC)")
    print("=" * 60)
    
    pool.run(host='0.0.0.0', port=5001, debug=False)

