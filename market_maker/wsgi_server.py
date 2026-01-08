#!/usr/bin/env python3
"""
Production WSGI Server for GXC Admin & Market Maker API
Uses Waitress for production-grade serving
"""

import os
from waitress import serve
from admin_api import app

def run_production_server():
    """Run production server with Waitress"""

    # Server configuration
    host = os.environ.get('SERVER_HOST', '0.0.0.0')
    port = int(os.environ.get('SERVER_PORT', '5002'))
    threads = int(os.environ.get('SERVER_THREADS', '4'))

    print("=" * 80)
    print("GXC Admin & Market Maker API - Production Server")
    print("=" * 80)
    print(f"Database: {os.environ.get('ADMIN_DB_PATH', 'gxc_admin.db')}")
    print(f"Server: Waitress WSGI Server")
    print(f"Host: {host}")
    print(f"Port: {port}")
    print(f"Threads: {threads}")
    print("=" * 80)
    print("API Endpoints:")
    print("  POST /api/v1/admin/create - Create admin")
    print("  POST /api/v1/admin/login - Admin login")
    print("  GET  /api/v1/admin/list - List admins")
    print("  POST /api/v1/marketmaker/apply - Submit application")
    print("  GET  /api/v1/marketmaker/status - Check status")
    print("  GET  /api/v1/admin/applications/pending - Get pending")
    print("  POST /api/v1/admin/verify/<type> - Verify step")
    print("  POST /api/v1/admin/approve - Approve application")
    print("  POST /api/v1/admin/reject - Reject application")
    print("=" * 80)
    print("Server is ready and listening...")
    print("Press CTRL+C to stop")
    print("=" * 80)

    # Serve with Waitress
    serve(
        app,
        host=host,
        port=port,
        threads=threads,
        url_scheme='http',
        channel_timeout=120,
        connection_limit=1000,
        cleanup_interval=30,
        clear_untrusted_proxy_headers=True
    )

if __name__ == '__main__':
    run_production_server()
