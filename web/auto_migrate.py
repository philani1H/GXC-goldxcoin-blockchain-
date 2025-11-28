#!/usr/bin/env python3
"""
Auto-migration for Vercel deployment
Automatically detects and migrates forum data when deployed
"""

import os
import sys

# Detect Vercel environment
IS_VERCEL = os.environ.get('VERCEL', '0') == '1' or 'vercel' in os.environ.get('VERCEL_URL', '').lower()
USE_POSTGRES = os.environ.get('USE_POSTGRES', 'false').lower() == 'true'
DATABASE_URL = os.environ.get('DATABASE_URL', '')

def should_auto_migrate():
    """Check if auto-migration should run"""
    return IS_VERCEL and USE_POSTGRES and DATABASE_URL

def auto_migrate():
    """Automatically migrate forum data if needed"""
    if not should_auto_migrate():
        return False
    
    print("🔍 Checking for auto-migration...")
    
    try:
        from forum_db import get_db
        
        db = get_db()
        
        # Initialize schema (this will trigger auto-migration if needed)
        db.init_schema()
        
        print("✅ Auto-migration check complete")
        return True
        
    except Exception as e:
        print(f"⚠️  Auto-migration check failed: {e}")
        import traceback
        traceback.print_exc()
        return False

if __name__ == '__main__':
    print("="*50)
    print("GXC Forum Auto-Migration")
    print("="*50)
    print()
    print(f"Environment: {'Vercel' if IS_VERCEL else 'Local'}")
    print(f"Use PostgreSQL: {USE_POSTGRES}")
    print(f"Database URL: {'Set' if DATABASE_URL else 'Not set'}")
    print()
    
    if auto_migrate():
        print("✅ Auto-migration successful")
        sys.exit(0)
    else:
        print("ℹ️  No migration needed or migration skipped")
        sys.exit(0)
