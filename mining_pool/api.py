#!/usr/bin/env python3
"""
Vercel serverless function wrapper for General mining pool
"""
import sys
import os

# Set Vercel environment
os.environ['VERCEL'] = '1'

# Add mining_pool directory to path
pool_path = os.path.dirname(__file__)
sys.path.insert(0, pool_path)

# Change to mining_pool directory for templates/static
if os.path.exists(pool_path):
    os.chdir(pool_path)

# Set database path for Vercel
if 'DATABASE_PATH' not in os.environ:
    os.environ['DATABASE_PATH'] = '/tmp/pool.db'

try:
    # Import the Flask app
    from general_pool import app
except Exception as e:
    # Fallback if import fails
    from flask import Flask
    app = Flask(__name__)
    
    @app.route('/')
    def error():
        return f"Pool initialization error: {str(e)}", 500
    
    @app.route('/api/stats')
    def api_stats():
        return {'error': 'Pool not initialized', 'details': str(e)}, 500

