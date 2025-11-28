#!/usr/bin/env python3
"""
GXC GPU Mining Pool (Ethash)
https://gpu-pool.gxc.network
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
        pool_name='gpu-pool',
        algorithm='ethash',
        port=4444,
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
    print("GXC GPU Mining Pool (Ethash)")
    print("=" * 60)
    print(f"Stratum: stratum+tcp://gpu-pool.gxc.network:4444")
    print(f"Dashboard: http://localhost:5002")
    print(f"Algorithm: Ethash (GPU)")
    print("=" * 60)
    
    pool.run(host='0.0.0.0', port=5002, debug=False)

