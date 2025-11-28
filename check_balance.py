#!/usr/bin/env python3
"""
Check balance for GXC addresses
"""

import sqlite3
import sys
from datetime import datetime

def check_balance(address, network='testnet'):
    """Check balance for an address"""
    
    # Determine database path
    if network == 'testnet':
        db_path = 'gxc_testnet_data/blockchain.db'
        reward_per_block = 12.5
    else:
        db_path = 'gxc_data/blockchain.db'
        reward_per_block = 50.0
    
    try:
        conn = sqlite3.connect(db_path)
        cursor = conn.cursor()
        
        # Get blocks mined by this address
        cursor.execute('''
            SELECT height, hash, timestamp 
            FROM blocks 
            WHERE miner = ? 
            ORDER BY height
        ''', (address,))
        
        blocks = cursor.fetchall()
        
        if not blocks:
            print(f"\n❌ No blocks found for address: {address}")
            print(f"   Network: {network}")
            return
        
        # Calculate balance
        total_blocks = len(blocks)
        total_balance = total_blocks * reward_per_block
        
        # Get blockchain info
        cursor.execute('SELECT MAX(height) FROM blocks')
        total_height = cursor.fetchone()[0] or 0
        
        conn.close()
        
        # Display results
        print("\n" + "="*70)
        print(f"💰 BALANCE CHECK - {network.upper()}")
        print("="*70)
        
        print(f"\n📍 Address:")
        print(f"   {address}")
        
        print(f"\n💎 Balance:")
        print(f"   {total_balance} GXC")
        
        print(f"\n⛏️  Mining Statistics:")
        print(f"   Blocks mined: {total_blocks}")
        print(f"   Reward per block: {reward_per_block} GXC")
        print(f"   Total blockchain height: {total_height}")
        print(f"   Your share: {(total_blocks/total_height*100) if total_height > 0 else 0:.1f}%")
        
        print(f"\n📊 Block History:")
        print(f"   First block: #{blocks[0][0]}")
        print(f"   Latest block: #{blocks[-1][0]}")
        
        # Show recent blocks
        print(f"\n🔗 Recent Blocks (last 5):")
        for height, block_hash, timestamp in blocks[-5:]:
            dt = datetime.fromtimestamp(timestamp)
            print(f"   Block #{height}: {block_hash[:16]}... ({dt.strftime('%Y-%m-%d %H:%M:%S')})")
        
        print("\n" + "="*70 + "\n")
        
        return {
            'address': address,
            'balance': total_balance,
            'blocks_mined': total_blocks,
            'network': network
        }
        
    except Exception as e:
        print(f"\n❌ Error: {e}")
        return None

def main():
    """Main function"""
    
    if len(sys.argv) < 2:
        print("\nUsage: python3 check_balance.py <address> [network]")
        print("\nExamples:")
        print("  python3 check_balance.py tGXC9fab7317231b966af85ac453e168c0932 testnet")
        print("  python3 check_balance.py GXC21d83ca607604df4f86bd60761e3c4ee30 mainnet")
        print()
        
        # Show default addresses
        print("Your addresses:")
        print("  Testnet: tGXC9fab7317231b966af85ac453e168c0932")
        print("  Mainnet: GXC21d83ca607604df4f86bd60761e3c4ee30")
        print()
        
        # Check testnet by default
        print("Checking testnet address...")
        check_balance('tGXC9fab7317231b966af85ac453e168c0932', 'testnet')
        return
    
    address = sys.argv[1]
    
    # Auto-detect network from address prefix
    if address.startswith('tGXC'):
        network = 'testnet'
    elif address.startswith('GXC'):
        network = 'mainnet'
    else:
        print(f"❌ Invalid address format: {address}")
        print("   Testnet addresses start with 'tGXC'")
        print("   Mainnet addresses start with 'GXC'")
        return
    
    # Override network if specified
    if len(sys.argv) > 2:
        network = sys.argv[2]
    
    check_balance(address, network)

if __name__ == "__main__":
    main()
