#!/usr/bin/env python3
"""
Vercel serverless function wrapper for General mining pool
"""
import sys
import os

# Add mining_pool directory to path
pool_path = os.path.dirname(__file__)
sys.path.insert(0, pool_path)

# Change to mining_pool directory for templates/static
if os.path.exists(pool_path):
    os.chdir(pool_path)

# Import the Flask app
from general_pool import app

