#!/usr/bin/env python3
"""
Vercel serverless function wrapper for GXC Wallet Service
"""
import sys
import os

# Add api directory to path
api_path = os.path.dirname(__file__)
sys.path.insert(0, api_path)

# Change to api directory for templates
if os.path.exists(api_path):
    os.chdir(api_path)

# Set Vercel environment variables
os.environ['VERCEL'] = '1'
if 'DATABASE_PATH' not in os.environ:
    os.environ['DATABASE_PATH'] = '/tmp/gxc_wallets.db'
if 'BACKUP_STORAGE_PATH' not in os.environ:
    os.environ['BACKUP_STORAGE_PATH'] = '/tmp/wallet_backups/'

# Set Railway URL environment variable if not set (for Vercel deployment)
if not os.environ.get('BLOCKCHAIN_NODE_URL') and not os.environ.get('RAILWAY_NODE_URL'):
    os.environ['RAILWAY_NODE_URL'] = 'https://gxc-chain112-blockchain-node-production.up.railway.app'
    os.environ['BLOCKCHAIN_NODE_URL'] = 'https://gxc-chain112-blockchain-node-production.up.railway.app'

# Import the Flask app
from wallet_service import app

# Vercel automatically handles WSGI apps
# Just export the app

