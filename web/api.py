#!/usr/bin/env python3
"""
Vercel serverless function wrapper for GXC Forum
"""
import sys
import os

# Add web directory to path
web_path = os.path.dirname(__file__)
sys.path.insert(0, web_path)

# Change to web directory for templates/static
if os.path.exists(web_path):
    os.chdir(web_path)

# Auto-migrate on first import (Vercel cold start)
try:
    from auto_migrate import auto_migrate
    auto_migrate()
except Exception as e:
    print(f"Note: Auto-migration: {e}")

# Import the Flask app
from forum import app, init_database

# Initialize database on cold start
try:
    init_database()
except Exception as e:
    print(f"Note: Database initialization: {e}")

# Vercel automatically handles WSGI apps
# Just export the app

