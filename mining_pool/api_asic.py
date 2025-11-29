#!/usr/bin/env python3
"""
Vercel serverless function wrapper for ASIC mining pool
"""
import sys
import os

# Set Vercel environment
os.environ['VERCEL'] = '1'

# Set Railway URL environment variables if not set (for Vercel deployment)
if not os.environ.get('BLOCKCHAIN_NODE_URL') and not os.environ.get('RAILWAY_NODE_URL'):
    os.environ['RAILWAY_NODE_URL'] = 'https://gxc-chain112-blockchain-node-production.up.railway.app'
    os.environ['BLOCKCHAIN_NODE_URL'] = 'https://gxc-chain112-blockchain-node-production.up.railway.app'

# Add mining_pool directory to path
pool_path = os.path.dirname(__file__)
sys.path.insert(0, pool_path)

# Change to mining_pool directory for templates/static
if os.path.exists(pool_path):
    os.chdir(pool_path)

# Set database path for Vercel
if 'DATABASE_PATH' not in os.environ:
    os.environ['DATABASE_PATH'] = '/tmp/asic_pool.db'

try:
    # Import the Flask app
    from asic_pool import app
except Exception as e:
    # Fallback if import fails
    from flask import Flask
    app = Flask(__name__)
    
    @app.route('/')
    def error():
        return f"ASIC Pool initialization error: {str(e)}", 500
    
    @app.route('/api/stats')
    def api_stats():
        return {'error': 'ASIC Pool not initialized', 'details': str(e)}, 500

