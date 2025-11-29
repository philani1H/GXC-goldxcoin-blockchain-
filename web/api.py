#!/usr/bin/env python3
"""
Vercel serverless function wrapper for GXC Blockchain Explorer
"""
import sys
import os

# Add web directory to path
web_path = os.path.dirname(__file__)
sys.path.insert(0, web_path)

# Change to web directory for templates/static
if os.path.exists(web_path):
    os.chdir(web_path)

# Set Railway URL environment variable if not set (for Vercel deployment)
if not os.environ.get('BLOCKCHAIN_NODE_URL') and not os.environ.get('RAILWAY_NODE_URL'):
    os.environ['RAILWAY_NODE_URL'] = 'https://gxc-chain112-blockchain-node-production.up.railway.app'
    os.environ['BLOCKCHAIN_NODE_URL'] = 'https://gxc-chain112-blockchain-node-production.up.railway.app'

# Import the Flask app from blockchain_explorer
from blockchain_explorer import app, explorer

# Initialize explorer database on cold start
try:
    explorer.init_database()
except Exception as e:
    print(f"Note: Explorer database initialization: {e}")

# Vercel automatically handles WSGI apps
# Just export the app

