#!/usr/bin/env python3
"""
Vercel serverless function wrapper for documentation site
"""
import sys
import os

# Add docs_site directory to path
docs_path = os.path.dirname(__file__)
sys.path.insert(0, docs_path)

# Change to docs_site directory for templates/static
if os.path.exists(docs_path):
    os.chdir(docs_path)

# Import the Flask app
from docs_app import app

