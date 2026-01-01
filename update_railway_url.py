#!/usr/bin/env python3
"""
Update Railway URL in all Python files
Run this script whenever you need to change the Railway node URL
"""

import os
import re
import sys

# Current Railway URL
RAILWAY_URL = "https://gxc-chain112-blockchain-node-production.up.railway.app"

# Files to update
FILES_TO_UPDATE = [
    'mine_testnet_fixed.py',
    'mine_testnet.py',
    'api/wallet_service.py',
    'web/blockchain_explorer.py',
    'mining_pool/pool_base.py',
    'mining/gxhash_miner.py',
    'config/railway_config.py',
]

# URL patterns to replace
PATTERNS = [
    (r'http://localhost:18332', RAILWAY_URL),
    (r'http://localhost:8545', RAILWAY_URL),
    (r'http://localhost:8332', RAILWAY_URL),
    (r'https://gxc-[a-z0-9-]+\.up\.railway\.app', RAILWAY_URL),
]

def update_file(filepath):
    """Update Railway URL in a file"""
    if not os.path.exists(filepath):
        print(f"⚠️  File not found: {filepath}")
        return False
    
    try:
        with open(filepath, 'r') as f:
            content = f.read()
        
        original_content = content
        updated = False
        
        # Apply all patterns
        for pattern, replacement in PATTERNS:
            new_content = re.sub(pattern, replacement, content)
            if new_content != content:
                content = new_content
                updated = True
        
        if updated:
            with open(filepath, 'w') as f:
                f.write(content)
            print(f"✅ Updated: {filepath}")
            return True
        else:
            print(f"ℹ️  No changes: {filepath}")
            return False
            
    except Exception as e:
        print(f"❌ Error updating {filepath}: {e}")
        return False

def main():
    """Main function"""
    global RAILWAY_URL

    print("=" * 60)
    print("GXC Railway URL Updater")
    print("=" * 60)
    print(f"\nNew Railway URL: {RAILWAY_URL}\n")

    if len(sys.argv) > 1:
        # Custom URL provided
        RAILWAY_URL = sys.argv[1]
        print(f"Using custom URL: {RAILWAY_URL}\n")
    
    updated_count = 0
    total_count = len(FILES_TO_UPDATE)
    
    for filepath in FILES_TO_UPDATE:
        if update_file(filepath):
            updated_count += 1
    
    print("\n" + "=" * 60)
    print(f"Updated {updated_count}/{total_count} files")
    print("=" * 60)
    
    if updated_count > 0:
        print("\n✅ Railway URL updated successfully!")
        print("\nNext steps:")
        print("1. Test connection: python test_railway_node.py")
        print("2. Start mining: python mine_testnet_fixed.py")
        print("3. Check wallet: python api/wallet_service.py")
    else:
        print("\nℹ️  No files needed updating")

if __name__ == '__main__':
    main()
