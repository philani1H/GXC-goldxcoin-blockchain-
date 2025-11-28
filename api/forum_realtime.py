#!/usr/bin/env python3
"""
Vercel serverless function wrapper for GXC Forum Real-Time Server
"""
import sys
import os

# Add web directory to path
web_path = os.path.join(os.path.dirname(__file__), '..', 'web')
sys.path.insert(0, web_path)

# Change to web directory
if os.path.exists(web_path):
    os.chdir(web_path)

# Set Vercel environment
os.environ['VERCEL'] = '1'

# Import the Flask app and socketio
from forum_realtime_server import app, socketio

# Export for Vercel
# Vercel will handle the WSGI app

