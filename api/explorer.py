#!/usr/bin/env python3
"""
Vercel serverless function wrapper for GXC Blockchain Explorer
"""
import sys
import os

# Add web directory to path
web_path = os.path.join(os.path.dirname(__file__), '..', 'web')
sys.path.insert(0, web_path)

# Change to web directory for templates/static
if os.path.exists(web_path):
    os.chdir(web_path)

# Import the Flask app
from blockchain_explorer import app

# Vercel automatically handles WSGI apps
# Just export the app

