#!/usr/bin/env python3
"""
Vercel serverless function wrapper for GXC Market Maker
"""
import sys
import os

# Add market_maker directory to path
market_maker_path = os.path.dirname(__file__)
sys.path.insert(0, market_maker_path)

# Change to market_maker directory for templates
if os.path.exists(market_maker_path):
    os.chdir(market_maker_path)

# Import the Flask app
from stock_market_maker import app

# Vercel automatically handles WSGI apps
# Just export the app

