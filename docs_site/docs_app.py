#!/usr/bin/env python3
"""
GXC Blockchain Documentation Site
https://docs.gxc.network
"""

import os
from flask import Flask, render_template, send_from_directory, jsonify
from flask_socketio import SocketIO

import secrets

# Flask app - use relative paths (api.py changes directory)
app = Flask(__name__, template_folder='templates', static_folder='static')
app.config['SECRET_KEY'] = secrets.token_hex(32)

# Detect if running on Vercel (serverless)
IS_VERCEL_SERVERLESS = os.environ.get('VERCEL', '0') == '1' or 'vercel' in os.environ.get('VERCEL_URL', '').lower()

# SocketIO - only use if not on Vercel (serverless doesn't support WebSockets well)
if not IS_VERCEL_SERVERLESS:
    socketio = SocketIO(app, cors_allowed_origins="*", async_mode='threading')
else:
    socketio = None

# Detect environment
IS_VERCEL = os.environ.get('VERCEL', '0') == '1' or 'vercel' in os.environ.get('VERCEL_URL', '').lower()
EXPLORER_URL = os.environ.get('EXPLORER_URL', 'https://gxc-blockchain.vercel.app' if IS_VERCEL else 'http://localhost:3000')
FORUM_URL = os.environ.get('FORUM_URL', 'https://gxc-blockchain-forum.vercel.app' if IS_VERCEL else 'http://localhost:3001')
MARKET_MAKER_URL = os.environ.get('MARKET_MAKER_URL', 'https://gxc-blockchain-market-maker.vercel.app' if IS_VERCEL else 'http://localhost:4000')

@app.context_processor
def inject_urls():
    return {
        'explorer_url': EXPLORER_URL,
        'forum_url': FORUM_URL,
        'market_maker_url': MARKET_MAKER_URL
    }

@app.route('/')
def index():
    """Documentation home page"""
    return render_template('docs_index.html')

@app.route('/mining')
def mining_docs():
    """Mining documentation"""
    return render_template('docs_mining.html')

@app.route('/api')
def api_docs():
    """API documentation"""
    return render_template('docs_api.html')

@app.route('/wallet')
def wallet_docs():
    """Wallet documentation"""
    return render_template('docs_wallet.html')

@app.route('/governance')
def governance_docs():
    """Governance documentation"""
    return render_template('docs_governance.html')

@app.route('/staking')
def staking_docs():
    """Staking documentation"""
    return render_template('docs_staking.html')

if __name__ == '__main__':
    print("=" * 60)
    print("GXC Blockchain Documentation Site")
    print("=" * 60)
    print(f"URL: http://localhost:5003")
    print("=" * 60)
    if socketio:
        socketio.run(app, host='0.0.0.0', port=5003, debug=False)
    else:
        app.run(host='0.0.0.0', port=5003, debug=False)

