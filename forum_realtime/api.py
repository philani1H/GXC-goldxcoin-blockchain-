#!/usr/bin/env python3
"""
Vercel serverless function for GXC Forum Real-Time Server
Standalone deployment
"""
import os

# Set Vercel environment
os.environ['VERCEL'] = '1'
os.environ['FORUM_DATABASE_PATH'] = os.environ.get('FORUM_DATABASE_PATH', '/tmp/gxc_forum.db')

# Import the Flask app and socketio from local file
from forum_realtime_server import app, socketio

# Export for Vercel - Vercel automatically detects Flask apps

