#!/usr/bin/env python3
"""
GXC Wallet - Complete Wallet CLI Tool
Allows testing all blockchain functions
"""

import os
import sys
import json
import hashlib
import requests
import argparse
from datetime import datetime

# Configuration
NODE_URL = os.environ.get('GXC_NODE_URL', 'http://localhost:8332')
WALLET_FILE = os.path.expanduser('~/.gxc_wallet.json')

class Colors:
    GREEN = '\033[92m'
    RED = '\033[91m'
    YELLOW = '\033[93m'
    BLUE = '\033[94m'
    CYAN = '\033[96m'
    END = '\033[0m'

class GXCWallet:
    def __init__(self, node_url=None):
        self.addresses = {}
        self.node_url = node_url or NODE_URL
        self.load_wallet()
    
    def load_wallet(self):
        """Load wallet from file"""
        if os.path.exists(WALLET_FILE):
            try:
                with open(WALLET_FILE, 'r') as f:
                    data = json.load(f)
                    self.addresses = data.get('addresses', {})
                print(f"{Colors.GREEN}✓ Wallet loaded: {len(self.addresses)} addresses{Colors.END}")
            except Exception as e:
                print(f"{Colors.RED}✗ Error loading wallet: {e}{Colors.END}")
        else:
            print(f"{Colors.YELLOW}⚠ No wallet file found, creating new wallet{Colors.END}")
    
    def save_wallet(self):
        """Save wallet to file"""
        try:
            data = {
                'addresses': self.addresses,
                'created': datetime.now().isoformat()
            }
            with open(WALLET_FILE, 'w') as f:
                json.dump(data, f, indent=2)
            print(f"{Colors.GREEN}✓ Wallet saved{Colors.END}")
        except Exception as e:
            print(f"{Colors.RED}✗ Error saving wallet: {e}{Colors.END}")
    
    def rpc_call(self, method, params=None):
        """Make RPC call to node"""
        if params is None:
            params = []
        
        payload = {
            "jsonrpc": "2.0",
            "method": method,
            "params": params,
            "id": 1
        }
        
        try:
            response = requests.post(self.node_url, json=payload, timeout=10)
            if response.status_code == 200:
                result = response.json()
                if 'error' in result and result['error']:
                    return None, result['error']
                return result.get('result'), None
            return None, f"HTTP {response.status_code}"
        except requests.exceptions.ConnectionError:
            return None, "Cannot connect to node. Is it running?"
        except Exception as e:
            return None, str(e)
    
    def create_address(self, label=None):
        """Create new address"""
        result, error = self.rpc_call("getnewaddress")
        
        if error:
            print(f"{Colors.RED}✗ Error: {error}{Colors.END}")
            return None
        
        address = result
        if not label:
            label = f"Address {len(self.addresses) + 1}"
        
        self.addresses[address] = {
            'label': label,
            'created': datetime.now().isoformat()
        }
        self.save_wallet()
        
        print(f"{Colors.GREEN}✓ New address created:{Colors.END}")
        print(f"  Address: {Colors.CYAN}{address}{Colors.END}")
        print(f"  Label: {label}")
        
        return address
    
    def import_address(self, address, label=None):
        """Import existing address"""
        # Validate address
        result, error = self.rpc_call("validateaddress", [address])
        
        if error:
            print(f"{Colors.RED}✗ Error: {error}{Colors.END}")
            return False
        
        if not result.get('isvalid'):
            print(f"{Colors.RED}✗ Invalid address{Colors.END}")
            return False
        
        if not label:
            label = f"Imported {len(self.addresses) + 1}"
        
        self.addresses[address] = {
            'label': label,
            'imported': datetime.now().isoformat()
        }
        self.save_wallet()
        
        print(f"{Colors.GREEN}✓ Address imported:{Colors.END}")
        print(f"  Address: {Colors.CYAN}{address}{Colors.END}")
        print(f"  Label: {label}")
        
        return True
    
    def list_addresses(self):
        """List all addresses"""
        if not self.addresses:
            print(f"{Colors.YELLOW}No addresses in wallet{Colors.END}")
            return
        
        print(f"\n{Colors.BLUE}Wallet Addresses:{Colors.END}")
        print("=" * 80)
        
        for i, (address, info) in enumerate(self.addresses.items(), 1):
            # Get balance
            balance, error = self.rpc_call("getbalance", [address])
            balance_str = f"{balance:.6f} GXC" if balance is not None else "N/A"
            
            print(f"\n{i}. {Colors.CYAN}{address}{Colors.END}")
            print(f"   Label: {info.get('label', 'N/A')}")
            print(f"   Balance: {Colors.GREEN}{balance_str}{Colors.END}")
            if 'created' in info:
                print(f"   Created: {info['created']}")
            if 'imported' in info:
                print(f"   Imported: {info['imported']}")
        
        print()
    
    def get_balance(self, address=None):
        """Get balance for address or all addresses"""
        if address:
            result, error = self.rpc_call("getbalance", [address])
            
            if error:
                print(f"{Colors.RED}✗ Error: {error}{Colors.END}")
                return
            
            print(f"\n{Colors.BLUE}Balance:{Colors.END}")
            print(f"  Address: {Colors.CYAN}{address}{Colors.END}")
            print(f"  Balance: {Colors.GREEN}{result:.6f} GXC{Colors.END}\n")
        else:
            # Get balance for all addresses
            total = 0
            print(f"\n{Colors.BLUE}All Balances:{Colors.END}")
            print("=" * 80)
            
            for addr in self.addresses:
                result, error = self.rpc_call("getbalance", [addr])
                if result is not None:
                    total += result
                    label = self.addresses[addr].get('label', 'N/A')
                    print(f"  {label}: {Colors.GREEN}{result:.6f} GXC{Colors.END}")
            
            print("=" * 80)
            print(f"  Total: {Colors.GREEN}{total:.6f} GXC{Colors.END}\n")
    
    def send(self, from_address, to_address, amount):
        """Send transaction"""
        print(f"\n{Colors.BLUE}Sending Transaction:{Colors.END}")
        print(f"  From: {Colors.CYAN}{from_address}{Colors.END}")
        print(f"  To: {Colors.CYAN}{to_address}{Colors.END}")
        print(f"  Amount: {Colors.GREEN}{amount} GXC{Colors.END}")
        
        # Confirm
        confirm = input(f"\n{Colors.YELLOW}Confirm? (yes/no): {Colors.END}")
        if confirm.lower() != 'yes':
            print(f"{Colors.YELLOW}Transaction cancelled{Colors.END}")
            return
        
        result, error = self.rpc_call("sendtoaddress", [to_address, amount, from_address])
        
        if error:
            print(f"{Colors.RED}✗ Error: {error}{Colors.END}")
            return
        
        print(f"{Colors.GREEN}✓ Transaction sent!{Colors.END}")
        print(f"  TX Hash: {Colors.CYAN}{result}{Colors.END}\n")
        
        return result
    
    def stake(self, address, amount, days):
        """Register as validator / stake coins"""
        print(f"\n{Colors.BLUE}Staking:{Colors.END}")
        print(f"  Address: {Colors.CYAN}{address}{Colors.END}")
        print(f"  Amount: {Colors.GREEN}{amount} GXC{Colors.END}")
        print(f"  Days: {days}")
        
        # Confirm
        confirm = input(f"\n{Colors.YELLOW}Confirm staking? (yes/no): {Colors.END}")
        if confirm.lower() != 'yes':
            print(f"{Colors.YELLOW}Staking cancelled{Colors.END}")
            return
        
        result, error = self.rpc_call("registervalidator", [address, amount, days])
        
        if error:
            print(f"{Colors.RED}✗ Error: {error}{Colors.END}")
            return
        
        print(f"{Colors.GREEN}✓ Staking successful!{Colors.END}\n")
        return result
    
    def get_staking_info(self, address):
        """Get staking information"""
        result, error = self.rpc_call("getstakinginfo", [address])
        
        if error:
            print(f"{Colors.RED}✗ Error: {error}{Colors.END}")
            return
        
        print(f"\n{Colors.BLUE}Staking Info:{Colors.END}")
        print(f"  Address: {Colors.CYAN}{address}{Colors.END}")
        print(json.dumps(result, indent=2))
        print()
    
    def get_transaction(self, txid):
        """Get transaction details"""
        result, error = self.rpc_call("gettransaction", [txid])
        
        if error:
            print(f"{Colors.RED}✗ Error: {error}{Colors.END}")
            return
        
        print(f"\n{Colors.BLUE}Transaction Details:{Colors.END}")
        print(json.dumps(result, indent=2))
        print()
    
    def node_info(self):
        """Get node information"""
        # Get blockchain info
        blockchain_info, error = self.rpc_call("getblockchaininfo")
        
        if error:
            print(f"{Colors.RED}✗ Error: {error}{Colors.END}")
            return
        
        # Get mining info
        mining_info, _ = self.rpc_call("getmininginfo")
        
        # Get mempool info
        mempool_info, _ = self.rpc_call("getmempoolinfo")
        
        print(f"\n{Colors.BLUE}Node Information:{Colors.END}")
        print("=" * 80)
        
        if blockchain_info:
            print(f"\n{Colors.CYAN}Blockchain:{Colors.END}")
            print(f"  Height: {blockchain_info.get('blocks', 'N/A')}")
            print(f"  Difficulty: {blockchain_info.get('difficulty', 'N/A')}")
            print(f"  Best Block: {blockchain_info.get('bestblockhash', 'N/A')[:16]}...")
        
        if mining_info:
            print(f"\n{Colors.CYAN}Mining:{Colors.END}")
            print(f"  Network Hashrate: {mining_info.get('networkhashps', 'N/A')}")
            print(f"  Pooled Transactions: {mining_info.get('pooledtx', 'N/A')}")
        
        if mempool_info:
            print(f"\n{Colors.CYAN}Mempool:{Colors.END}")
            print(f"  Pending Transactions: {mempool_info.get('size', 'N/A')}")
            print(f"  Memory Usage: {mempool_info.get('usage', 'N/A')} bytes")
        
        print()

def main():
    parser = argparse.ArgumentParser(
        description='GXC Wallet - Complete Wallet CLI Tool',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Create new address
  %(prog)s create --label "My Address"
  
  # Import existing address
  %(prog)s import tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf --label "Mining Address"
  
  # List all addresses
  %(prog)s list
  
  # Get balance
  %(prog)s balance
  %(prog)s balance tGXC3tz6MsTbP2NCMvc33JixQdkQi6tf
  
  # Send transaction
  %(prog)s send FROM_ADDRESS TO_ADDRESS 10.0
  
  # Stake coins
  %(prog)s stake ADDRESS 100.0 365
  
  # Get staking info
  %(prog)s staking ADDRESS
  
  # Get transaction
  %(prog)s tx TRANSACTION_HASH
  
  # Node info
  %(prog)s info
        """
    )
    
    parser.add_argument('command', help='Command to execute')
    parser.add_argument('args', nargs='*', help='Command arguments')
    parser.add_argument('--label', help='Label for address')
    parser.add_argument('--node', help='Node URL', default=NODE_URL)
    
    args = parser.parse_args()
    
    wallet = GXCWallet(node_url=args.node)
    
    # Execute command
    if args.command == 'create':
        wallet.create_address(args.label)
    
    elif args.command == 'import':
        if not args.args:
            print(f"{Colors.RED}✗ Address required{Colors.END}")
            return
        wallet.import_address(args.args[0], args.label)
    
    elif args.command == 'list':
        wallet.list_addresses()
    
    elif args.command == 'balance':
        address = args.args[0] if args.args else None
        wallet.get_balance(address)
    
    elif args.command == 'send':
        if len(args.args) < 3:
            print(f"{Colors.RED}✗ Usage: send FROM_ADDRESS TO_ADDRESS AMOUNT{Colors.END}")
            return
        wallet.send(args.args[0], args.args[1], float(args.args[2]))
    
    elif args.command == 'stake':
        if len(args.args) < 3:
            print(f"{Colors.RED}✗ Usage: stake ADDRESS AMOUNT DAYS{Colors.END}")
            return
        wallet.stake(args.args[0], float(args.args[1]), int(args.args[2]))
    
    elif args.command == 'staking':
        if not args.args:
            print(f"{Colors.RED}✗ Address required{Colors.END}")
            return
        wallet.get_staking_info(args.args[0])
    
    elif args.command == 'tx':
        if not args.args:
            print(f"{Colors.RED}✗ Transaction hash required{Colors.END}")
            return
        wallet.get_transaction(args.args[0])
    
    elif args.command == 'info':
        wallet.node_info()
    
    else:
        print(f"{Colors.RED}✗ Unknown command: {args.command}{Colors.END}")
        parser.print_help()

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print(f"\n{Colors.YELLOW}Interrupted{Colors.END}")
        sys.exit(0)
    except Exception as e:
        print(f"{Colors.RED}✗ Error: {e}{Colors.END}")
        sys.exit(1)
