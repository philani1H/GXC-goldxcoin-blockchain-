#!/usr/bin/env python3
"""
GXC Testnet Miner with GUI - Real-time mining with visual feedback
Similar to gxhash_miner.py but for testnet mining
"""

import os
import sys
import requests
import hashlib
import time
import json
import threading
import signal
import webbrowser
import tkinter as tk
from tkinter import ttk, scrolledtext, messagebox
from datetime import datetime
from typing import Optional, Dict, Any, List

# Import centralized network configuration
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from config.railway_config import (
    get_network, get_rpc_url, get_miner_address, get_network_config,
    RAILWAY_NODE_URL, LOCAL_NODE_URL, CURRENT_NETWORK
)

# Network-aware configuration
NETWORK = get_network()
MINER_ADDRESS = get_miner_address()
RPC_URL = get_rpc_url()
ACTIVE_NODE_TYPE = None

class TestnetMinerGUI:
    """GXC Testnet Miner with GUI - Real-time visual feedback"""
    
    def __init__(self, root):
        self.root = root
        self.root.title(f"GXC Miner - {NETWORK.upper()}")
        self.root.geometry("1000x750")
        self.root.minsize(900, 650)
        self.root.configure(bg="#0f172a")
        
        # Mining parameters
        self.rpc_url = RPC_URL
        self.wallet_address = tk.StringVar(value=MINER_ADDRESS)
        
        # Mining state
        self.mining = False
        self.node_connected = False
        self.hash_rate = 0
        self.total_hashes = 0
        self.blocks_found = 0
        self.blocks_attempted = 0
        # NO LOCAL EARNINGS TRACKING - Get from blockchain only
        self.last_balance_check = 0.0  # Track last known balance for comparison only
        self.start_time = None
        self.shutdown = False
        
        # Setup signal handlers
        signal.signal(signal.SIGINT, self._signal_handler)
        signal.signal(signal.SIGTERM, self._signal_handler)
        
        self.setup_ui()
        self.log(f"🚀 GXC {NETWORK.upper()} Miner initialized", "INFO")
        self.log(f"📍 Network: {NETWORK.upper()}", "INFO")
        self.log(f"📍 Address prefix: {CURRENT_NETWORK['address_prefix']}", "INFO")
        # Get actual block reward from blockchain (will be fetched after connection)
        self.log(f"📍 Block reward: Will fetch from blockchain...", "INFO")
        
        # Auto-connect to blockchain in background (wait 2 seconds for node to be ready)
        self.root.after(2000, self.auto_connect)
        
        # Start stats update timer
        self.root.after(1000, self.update_stats_timer)
        
        # Auto-refresh balance periodically if wallet address is set
        self.root.after(10000, self.auto_refresh_balance)
    
    def _signal_handler(self, signum, frame):
        """Handle shutdown signals"""
        self.shutdown = True
        self.mining = False
        self.root.quit()
    
    def auto_connect(self):
        """Auto-connect to blockchain node in background"""
        self.log("🔌 Auto-connecting to blockchain node...", "INFO")
        self.status_label.config(text="● Connecting...", fg="#fbbf24")
        
        def connect():
            global RPC_URL, ACTIVE_NODE_TYPE
            
            # Test LOCAL first (since user says it's running) - same as command miner
            # Try multiple times with different URLs
            local_urls = [
                LOCAL_NODE_URL,
                LOCAL_NODE_URL.replace('localhost', '127.0.0.1'),
                'http://127.0.0.1:18332'
            ]
            
            for url in local_urls:
                try:
                    response = requests.post(url, json={
                        "jsonrpc": "2.0",
                        "method": "getblockcount",
                        "params": [],
                        "id": 1
                    }, timeout=3)
                    response.raise_for_status()
                    result = response.json()
                    
                    if 'error' not in result and result.get('result') is not None:
                        height = result.get('result')
                        # Get blockchain info for block reward
                        try:
                            info_response = requests.post(url, json={
                                "jsonrpc": "2.0",
                                "method": "getblockchaininfo",
                                "params": [],
                                "id": 1
                            }, timeout=3)
                            info_result = info_response.json().get('result', {})
                            block_reward = info_result.get('block_reward') or info_result.get('reward', 50.0)
                        except:
                            block_reward = 50.0
                    self.node_connected = True
                    RPC_URL = url
                    ACTIVE_NODE_TYPE = "local"
                    self.root.after(0, lambda h=height, r=block_reward: self.log(f"✅ Connected to local node", "SUCCESS"))
                    self.root.after(0, lambda h=height, r=block_reward: self.log(f"📊 Current height: {h}", "INFO"))
                    self.root.after(0, lambda r=block_reward: self.log(f"💰 Block reward: {r} GXC", "INFO"))
                    self.root.after(0, lambda: self.status_label.config(text="● Connected (Local)", fg="#86efac"))
                    
                    # Auto-refresh balance if wallet address is set
                    if self.wallet_address.get():
                        self.root.after(1000, self.refresh_balance_from_blockchain)
                    return
                except requests.exceptions.ConnectionError:
                    continue
                except Exception:
                    continue
            
            # Try Railway as fallback - same as command miner
            try:
                response = requests.post(RAILWAY_NODE_URL, json={
                    "jsonrpc": "2.0",
                    "method": "getblockcount",
                    "params": [],
                    "id": 1
                }, timeout=10)
                response.raise_for_status()
                result = response.json()
                
                if 'error' not in result and result.get('result') is not None:
                    height = result.get('result')
                    # Get blockchain info for block reward
                    try:
                        info_response = requests.post(RAILWAY_NODE_URL, json={
                            "jsonrpc": "2.0",
                            "method": "getblockchaininfo",
                            "params": [],
                            "id": 1
                        }, timeout=10)
                        info_result = info_response.json().get('result', {})
                        block_reward = info_result.get('block_reward') or info_result.get('reward', 50.0)
                    except:
                        block_reward = 50.0
                    self.node_connected = True
                    RPC_URL = RAILWAY_NODE_URL
                    ACTIVE_NODE_TYPE = "railway"
                    self.root.after(0, lambda: self.log(f"✅ Connected to Railway node", "SUCCESS"))
                    self.root.after(0, lambda h=height: self.log(f"📊 Current height: {h}", "INFO"))
                    self.root.after(0, lambda r=block_reward: self.log(f"💰 Block reward: {r} GXC", "INFO"))
                    self.root.after(0, lambda: self.status_label.config(text="● Connected (Railway)", fg="#86efac"))
                    
                    # Auto-refresh balance if wallet address is set
                    if self.wallet_address.get():
                        self.root.after(1000, self.refresh_balance_from_blockchain)
                    return
            except Exception:
                pass
            
            self.node_connected = False
            self.root.after(0, lambda: self.log("⚠️ Could not connect. Make sure node is running:", "WARNING"))
            self.root.after(0, lambda: self.log(f"   python testnet_blockchain_node.py", "INFO"))
            self.root.after(0, lambda: self.log(f"   Node should be at: {LOCAL_NODE_URL}", "INFO"))
            self.root.after(0, lambda: self.status_label.config(text="● Disconnected", fg="#fca5a5"))
        
        threading.Thread(target=connect, daemon=True).start()
    
    def setup_ui(self):
        """Setup the user interface"""
        # Header with status
        header = tk.Frame(self.root, bg="#7c3aed", height=50)
        header.pack(fill="x")
        header.pack_propagate(False)
        
        title_text = f"⚡ GXC {NETWORK.upper()} MINER"
        tk.Label(header, text=title_text, 
                font=("Arial", 16, "bold"), bg="#7c3aed", fg="#ffffff").pack(side="left", padx=15, pady=10)
        
        self.status_label = tk.Label(header, text="● Connecting...", 
                                     font=("Arial", 9, "bold"), 
                                     bg="#7c3aed", fg="#fbbf24")
        self.status_label.pack(side="right", padx=15)
        
        # Compact stats row
        stats = tk.Frame(self.root, bg="#0f172a")
        stats.pack(fill="x", padx=8, pady=8)
        
        self.create_mini_stat(stats, "Hash Rate", "0 H/s", "#3b82f6", 0)
        self.create_mini_stat(stats, "Balance", "0.000000 GXC", "#10b981", 1)
        self.create_mini_stat(stats, "Blocks", "0", "#8b5cf6", 2)
        self.create_mini_stat(stats, "Earned", "0.000000 GXC", "#f59e0b", 3)
        
        # Main container with notebook tabs
        notebook = ttk.Notebook(self.root)
        notebook.pack(fill="both", expand=True, padx=8, pady=(0,8))
        
        # Style for notebook
        style = ttk.Style()
        style.theme_use('default')
        style.configure('TNotebook', background='#0f172a', borderwidth=0)
        style.configure('TNotebook.Tab', background='#1e293b', foreground='#94a3b8', 
                       padding=[12, 6], font=('Arial', 9, 'bold'))
        style.map('TNotebook.Tab', background=[('selected', '#7c3aed')], 
                 foreground=[('selected', '#ffffff')])
        
        # Tab 1: Mining
        mining_tab = tk.Frame(notebook, bg="#1e293b")
        notebook.add(mining_tab, text="⚡ Mining")
        self.setup_mining_tab(mining_tab)
        
        # Tab 2: Transactions
        transactions_tab = tk.Frame(notebook, bg="#1e293b")
        notebook.add(transactions_tab, text="💰 Transactions")
        self.setup_transactions_tab(transactions_tab)
        
        # Tab 3: Activity Log
        log_tab = tk.Frame(notebook, bg="#1e293b")
        notebook.add(log_tab, text="📊 Activity")
        
        self.log_text = scrolledtext.ScrolledText(log_tab, font=("Consolas", 8),
                                                  bg="#0f172a", fg="#10b981",
                                                  wrap="word", relief="flat",
                                                  insertbackground="#10b981")
        self.log_text.pack(fill="both", expand=True, padx=8, pady=8)
        
        # Configure log colors
        self.log_text.tag_config("INFO", foreground="#3b82f6")
        self.log_text.tag_config("SUCCESS", foreground="#10b981")
        self.log_text.tag_config("WARNING", foreground="#f59e0b")
        self.log_text.tag_config("ERROR", foreground="#ef4444")
        
        # Tab 4: Stats
        stats_tab = tk.Frame(notebook, bg="#1e293b")
        notebook.add(stats_tab, text="📈 Stats")
        self.create_stat_display(stats_tab)
    
    def create_mini_stat(self, parent, label, value, color, column):
        """Create mini stat widget"""
        frame = tk.Frame(parent, bg="#1e293b", relief="flat", bd=0)
        frame.grid(row=0, column=column, padx=4, sticky="ew", pady=4)
        parent.grid_columnconfigure(column, weight=1)
        
        tk.Label(frame, text=label, font=("Arial", 7), 
                bg="#1e293b", fg="#94a3b8").pack()
        
        label_obj = tk.Label(frame, text=value, font=("Arial", 12, "bold"), 
                            bg="#1e293b", fg=color)
        label_obj.pack()
        
        setattr(self, f"{label.lower().replace(' ', '_')}_label", label_obj)
    
    def setup_mining_tab(self, parent):
        """Setup mining tab"""
        # Wallet section
        wallet_frame = tk.LabelFrame(parent, text="Wallet Address", font=("Arial", 9, "bold"),
                                     bg="#1e293b", fg="#e2e8f0", relief="flat")
        wallet_frame.pack(fill="x", padx=8, pady=8)
        
        tk.Label(wallet_frame, text="GXC Address (for mining rewards):", font=("Arial", 8), 
                bg="#1e293b", fg="#cbd5e1").pack(anchor="w", padx=8, pady=(8,2))
        
        entry_frame = tk.Frame(wallet_frame, bg="#1e293b")
        entry_frame.pack(fill="x", padx=8, pady=(0,4))
        
        tk.Entry(entry_frame, textvariable=self.wallet_address, font=("Arial", 9), 
                bg="#334155", fg="#ffffff", relief="flat", 
                insertbackground="#ffffff").pack(side="left", fill="x", expand=True, ipady=4)
        
        refresh_btn = tk.Button(entry_frame, text="🔄", command=self.refresh_balance_from_blockchain,
                              font=("Arial", 8), bg="#3b82f6", fg="#ffffff",
                              relief="flat", cursor="hand2", width=3)
        refresh_btn.pack(side="right", padx=(4,0))
        
        # Balance display
        self.wallet_balance_label = tk.Label(wallet_frame, text="Balance: Loading...", 
                                            font=("Arial", 8, "bold"), 
                                            bg="#1e293b", fg="#10b981")
        self.wallet_balance_label.pack(anchor="w", padx=8, pady=(4,0))
        
        tk.Label(wallet_frame, text=f"💰 Mining rewards: {CURRENT_NETWORK['block_reward']} GXC per block", 
                font=("Arial", 7), bg="#1e293b", fg="#94a3b8").pack(anchor="w", padx=8, pady=(4,8))
        
        # Control buttons
        btn_frame = tk.Frame(parent, bg="#1e293b")
        btn_frame.pack(fill="x", padx=8, pady=(0,8))
        
        self.start_btn = tk.Button(btn_frame, text="⚡ START MINING", command=self.start_mining,
                                   font=("Arial", 10, "bold"), bg="#10b981", fg="#ffffff",
                                   relief="flat", cursor="hand2")
        self.start_btn.pack(side="left", fill="x", expand=True, padx=(0,4), ipady=8)
        
        self.stop_btn = tk.Button(btn_frame, text="⏸ STOP", command=self.stop_mining,
                                  font=("Arial", 10, "bold"), bg="#ef4444", fg="#ffffff",
                                  relief="flat", state="disabled", cursor="hand2")
        self.stop_btn.pack(side="left", fill="x", expand=True, padx=4, ipady=8)
        
        reconnect_btn = tk.Button(btn_frame, text="🔄 Reconnect", command=self.auto_connect,
                                  font=("Arial", 9, "bold"), bg="#3b82f6", fg="#ffffff",
                                  relief="flat", cursor="hand2")
        reconnect_btn.pack(side="left", fill="x", expand=True, padx=(4,0), ipady=8)
        
        # Network info
        info_frame = tk.LabelFrame(parent, text="Network Info", font=("Arial", 9, "bold"),
                                   bg="#1e293b", fg="#e2e8f0", relief="flat")
        info_frame.pack(fill="x", padx=8, pady=(0,8))
        
        info_text = f"Network: {NETWORK.upper()} | Prefix: {CURRENT_NETWORK['address_prefix']} | Block Time: {CURRENT_NETWORK['block_time']}s"
        tk.Label(info_frame, text=info_text, font=("Arial", 8), 
                bg="#1e293b", fg="#cbd5e1").pack(padx=8, pady=8)
    
    def create_stat_display(self, parent):
        """Create detailed stats display"""
        stats_container = tk.Frame(parent, bg="#1e293b")
        stats_container.pack(fill="both", expand=True, padx=8, pady=8)
        
        # Create stats grid
        stats = [
            ("Total Hashes", "total_hashes_label", "0"),
            ("Blocks Found", "blocks_found_label", "0"),
            ("Blocks Attempted", "blocks_attempted_label", "0"),
            ("Success Rate", "success_rate_label", "0%"),
            ("Total Earned", "total_earned_label", "0.000000 GXC"),
            ("Current Balance", "current_balance_label", "0.000000 GXC"),
            ("Mining Time", "mining_time_label", "00:00:00"),
        ]
        
        row = 0
        col = 0
        for label, attr, default in stats:
            frame = tk.LabelFrame(stats_container, text=label, font=("Arial", 8, "bold"),
                                 bg="#1e293b", fg="#cbd5e1", relief="flat")
            frame.grid(row=row, column=col, padx=8, pady=8, sticky="ew")
            
            label_obj = tk.Label(frame, text=default, font=("Arial", 14, "bold"),
                                bg="#1e293b", fg="#10b981")
            label_obj.pack(padx=8, pady=8)
            
            setattr(self, attr, label_obj)
            
            col += 1
            if col >= 2:
                col = 0
                row += 1
        
        stats_container.grid_columnconfigure(0, weight=1)
        stats_container.grid_columnconfigure(1, weight=1)
    
    def log(self, message, level="INFO"):
        """Add message to log"""
        timestamp = datetime.now().strftime("%H:%M:%S")
        log_entry = f"[{timestamp}] {message}\n"
        
        self.log_text.insert("end", log_entry, level)
        self.log_text.see("end")
    
    def rpc_call(self, method, params=None, show_errors=True):
        """Make RPC call to blockchain node - uses same logic as command miner"""
        global RPC_URL, ACTIVE_NODE_TYPE
        
        if params is None:
            params = []
        
        payload = {
            "jsonrpc": "2.0",
            "method": method,
            "params": params,
            "id": 1
        }
        
        try:
            # Use same timeout logic as command miner
            timeout = 10 if ACTIVE_NODE_TYPE == "railway" else 5
            response = requests.post(RPC_URL, json=payload, timeout=timeout)
            response.raise_for_status()
            result = response.json()
            
            if 'error' in result:
                if show_errors:
                    self.log(f"❌ RPC Error: {result['error']}", "ERROR")
                return None
            
            return result.get('result')
        except Exception as e:
            if show_errors:
                self.log(f"❌ RPC Error: {str(e)[:60]}", "ERROR")
            return None
    
    def mine_block(self):
        """Mine a single block"""
        address = self.wallet_address.get()
        if not address:
            return False
        
        self.log(f"⛏️  Mining block to: {address[:20]}...", "INFO")
        
        # Get block template
        template = self.rpc_call("getblocktemplate", [{"algorithm": "sha256"}], show_errors=False)
        
        if not template:
            self.log("❌ Failed to get block template", "ERROR")
            return False
        
        height = template.get('height', 1)
        prev_hash = template.get('previousblockhash', '0' * 64)
        
        self.log(f"📦 Height: {height}, Previous: {prev_hash[:16]}...", "INFO")
        
        # Mining loop
        nonce = 0
        start_time = time.time()
        last_update = time.time()
        
        while self.mining and not self.shutdown:
            # Create block hash
            block_string = f"{height}{prev_hash}{nonce}{address}{int(time.time())}"
            block_hash = hashlib.sha256(block_string.encode()).hexdigest()
            
            self.total_hashes += 1
            
            # Update hash rate every second
            if time.time() - last_update >= 1.0:
                elapsed = time.time() - start_time
                if elapsed > 0:
                    self.hash_rate = self.total_hashes / elapsed
                last_update = time.time()
            
            # Check if valid (testnet - easy difficulty)
            if block_hash.startswith('0'):
                # Get block reward from template or blockchain info
                block_reward = template.get('coinbasevalue') or template.get('coinbase_value') or 12.5
                if not block_reward:
                    try:
                        blockchain_info = self.rpc_call("getblockchaininfo", [], show_errors=False)
                        if blockchain_info:
                            block_reward = blockchain_info.get('block_reward') or blockchain_info.get('reward') or CURRENT_NETWORK.get('block_reward', 12.5)
                    except:
                        block_reward = CURRENT_NETWORK.get('block_reward', 12.5)
                
                # Create coinbase transaction to pay the miner
                coinbase_tx = {
                    'hash': hashlib.sha256(f"coinbase_{height}_{address}_{int(time.time())}".encode()).hexdigest(),
                    'txid': hashlib.sha256(f"coinbase_{height}_{address}_{int(time.time())}".encode()).hexdigest(),
                    'type': 'coinbase',
                    'is_coinbase': True,
                    'coinbase': True,
                    'inputs': [],
                    'outputs': [{
                        'address': address,
                        'amount': block_reward,
                        'value': block_reward
                    }],
                    'value': block_reward,
                    'amount': block_reward,
                    'timestamp': int(time.time()),
                    'block_number': height,
                    'from': 'coinbase',
                    'to': address
                }
                
                # Prepare block data with coinbase transaction
                block_data = {
                    'hash': block_hash,
                    'previousblockhash': prev_hash,
                    'height': height,
                    'nonce': nonce,
                    'miner': address,
                    'miner_address': address,
                    'timestamp': int(time.time()),
                    'difficulty': template.get('difficulty', 0.1),
                    'transactions': [coinbase_tx],  # Include coinbase transaction
                    'transaction_count': 1
                }
                
                self.log(f"✅ Found valid hash: {block_hash[:16]}...", "SUCCESS")
                self.log(f"📤 Submitting block...", "INFO")
                
                result = self.rpc_call("submitblock", [block_data], show_errors=False)
                elapsed = time.time() - start_time
                
                if result is None or (isinstance(result, dict) and result.get('success')):
                    # Get reward from blockchain - NO HARDCODED VALUES
                    reward = None
                    try:
                        # Try to get from blockchain info
                        blockchain_info = self.rpc_call("getblockchaininfo", [], show_errors=False)
                        if blockchain_info:
                            reward = blockchain_info.get('block_reward') or blockchain_info.get('reward') or CURRENT_NETWORK.get('block_reward')
                    except:
                        pass
                    
                    if not reward:
                        reward = CURRENT_NETWORK.get('block_reward', 0.0)  # Use network config as last resort
                    
                    self.blocks_found += 1
                    # DO NOT increment total_earned locally - get from blockchain
                    
                    self.log(f"🎉 BLOCK MINED! Height: {height}", "SUCCESS")
                    if reward > 0:
                        self.log(f"💰 Expected reward: {reward} GXC (will verify from blockchain)", "SUCCESS")
                    self.log(f"⏳ Waiting for blockchain confirmation...", "INFO")
                    self.log(f"⏱️  Time: {elapsed:.2f} seconds", "INFO")
                    
                    # Show explorer URLs (but don't auto-open browser)
                    explorer_url = f"http://localhost:3000/block/{height}"
                    tx_url = f"http://localhost:3000/tx/{block_hash}"
                    self.log(f"🔗 Block Explorer: {explorer_url}", "INFO")
                    self.log(f"🔗 Transaction Explorer: {tx_url}", "INFO")
                    
                    # Verify block was accepted by checking if it exists on chain
                    def verify_block():
                        try:
                            # Wait a bit for block to be processed
                            time.sleep(3)
                            # Try to get the block from chain
                            block_check = self.rpc_call("getblock", [height, False], show_errors=False)
                            if block_check:
                                self.log(f"✅ Block {height} confirmed on blockchain", "SUCCESS")
                                # Check if block has our address in coinbase
                                block_full = self.rpc_call("getblock", [height, True], show_errors=False)
                                if block_full and isinstance(block_full, dict):
                                    txs = block_full.get('transactions', block_full.get('tx', []))
                                    reward_found = False
                                    for tx in txs:
                                        if isinstance(tx, dict):
                                            # Check if this is coinbase transaction
                                            is_coinbase = tx.get('is_coinbase') or tx.get('coinbase') or tx.get('type') == 'coinbase'
                                            if is_coinbase:
                                                # Check outputs for coinbase
                                                outputs = tx.get('outputs', tx.get('vout', []))
                                                if not outputs and isinstance(tx.get('to'), str):
                                                    # Simple format: direct to address
                                                    if tx.get('to') == address:
                                                        amount = tx.get('value', tx.get('amount', 0.0))
                                                        if amount > 0:
                                                            self.log(f"✅ Coinbase reward found: {amount} GXC to {address[:20]}...", "SUCCESS")
                                                            reward_found = True
                                                else:
                                                    # Check each output
                                                    for output in outputs:
                                                        if isinstance(output, dict):
                                                            out_addr = output.get('address', output.get('scriptPubKey', {}).get('addresses', [None])[0] if isinstance(output.get('scriptPubKey', {}).get('addresses'), list) else None)
                                                            if out_addr == address:
                                                                amount = output.get('value', output.get('amount', 0.0))
                                                                if amount > 0:
                                                                    self.log(f"✅ Coinbase reward found: {amount} GXC to {address[:20]}...", "SUCCESS")
                                                                    reward_found = True
                                                                    break
                                            if reward_found:
                                                break
                                    if not reward_found:
                                        self.log(f"⚠️ Coinbase transaction not found in block {height} for address {address[:20]}...", "WARNING")
                            else:
                                self.log(f"⚠️ Block {height} not yet confirmed on chain", "WARNING")
                        except Exception as e:
                            self.log(f"⚠️ Could not verify block: {e}", "WARNING")
                    
                    # Verify block in background
                    threading.Thread(target=verify_block, daemon=True).start()
                    
                    # Refresh balance and transactions from blockchain to get REAL data
                    # Wait a bit for block to be processed, then verify from blockchain
                    self.root.after(5000, self.refresh_balance_from_blockchain)  # Wait 5 seconds for block processing
                    self.root.after(15000, self.refresh_balance_from_blockchain)  # Refresh again after 15 seconds to get confirmed data
                    self.root.after(30000, self.refresh_balance_from_blockchain)  # Refresh again after 30 seconds
                    
                    return True
                else:
                    self.log(f"❌ Block rejected: {result}", "ERROR")
                    return False
            
            nonce += 1
            
            # Timeout after 60 seconds
            if time.time() - start_time > 60:
                self.log(f"⏱️  Timeout after 60 seconds", "WARNING")
                return False
        
        return False
    
    def start_mining(self):
        """Start mining"""
        if not self.wallet_address.get():
            messagebox.showerror("Error", "Enter your GXC wallet address first!")
            return
        
        if not self.node_connected:
            messagebox.showerror("Error", "Blockchain node not connected!\n\nStart the blockchain node first.")
            return
        
        self.mining = True
        self.start_btn.config(state="disabled")
        self.stop_btn.config(state="normal")
        self.start_time = time.time()
        self.log("⚡ Mining started!", "SUCCESS")
        self.log(f"📧 Mining to: {self.wallet_address.get()[:30]}...", "INFO")
        
        # Start mining in background thread
        def mine_loop():
            while self.mining and not self.shutdown:
                self.blocks_attempted += 1
                self.log(f"📊 Attempt #{self.blocks_attempted}", "INFO")
                
                if self.mine_block():
                    self.log(f"✅ Total blocks mined: {self.blocks_found}", "SUCCESS")
                    time.sleep(2)
                else:
                    time.sleep(5)
                
                if not self.mining:
                    break
        
        threading.Thread(target=mine_loop, daemon=True).start()
    
    def stop_mining(self):
        """Stop mining"""
        self.mining = False
        self.start_btn.config(state="normal")
        self.stop_btn.config(state="disabled")
        self.log("⏸️  Mining stopped", "WARNING")
        
        success_rate = (self.blocks_found / self.blocks_attempted * 100) if self.blocks_attempted > 0 else 0
        self.log(f"📊 Statistics:", "INFO")
        self.log(f"   Blocks attempted: {self.blocks_attempted}", "INFO")
        self.log(f"   Blocks mined: {self.blocks_found}", "INFO")
        self.log(f"   Success rate: {success_rate:.1f}%", "INFO")
        # Refresh from blockchain to get real earnings
        if self.wallet_address.get():
            self.refresh_balance_from_blockchain()
        self.log(f"   Total hashes: {self.total_hashes:,}", "INFO")
    
    def update_stats_timer(self):
        """Update stats periodically"""
        if self.mining:
            # Update hash rate display
            if self.hash_rate >= 1e6:
                hr = f"{self.hash_rate/1e6:.2f} MH/s"
            elif self.hash_rate >= 1e3:
                hr = f"{self.hash_rate/1e3:.2f} KH/s"
            else:
                hr = f"{self.hash_rate:.0f} H/s"
            
            self.hash_rate_label.config(text=hr)
            # Balance and earnings will be updated from blockchain refresh, not here
            self.blocks_label.config(text=f"{self.blocks_found}")
            
            # Update detailed stats
            if hasattr(self, 'total_hashes_label'):
                self.total_hashes_label.config(text=f"{self.total_hashes:,}")
            if hasattr(self, 'blocks_found_label'):
                self.blocks_found_label.config(text=f"{self.blocks_found}")
            if hasattr(self, 'blocks_attempted_label'):
                self.blocks_attempted_label.config(text=f"{self.blocks_attempted}")
            
            success_rate = (self.blocks_found / self.blocks_attempted * 100) if self.blocks_attempted > 0 else 0
            if hasattr(self, 'success_rate_label'):
                self.success_rate_label.config(text=f"{success_rate:.1f}%")
            
            # Earnings and balance updated from blockchain only
            
            # Update mining time
            if self.start_time and hasattr(self, 'mining_time_label'):
                elapsed = time.time() - self.start_time
                hours = int(elapsed // 3600)
                minutes = int((elapsed % 3600) // 60)
                seconds = int(elapsed % 60)
                self.mining_time_label.config(text=f"{hours:02d}:{minutes:02d}:{seconds:02d}")
        
        self.root.after(1000, self.update_stats_timer)
    
    def setup_transactions_tab(self, parent):
        """Setup transactions display tab"""
        # Header with refresh button
        header_frame = tk.Frame(parent, bg="#1e293b")
        header_frame.pack(fill="x", padx=8, pady=8)
        
        tk.Label(header_frame, text="Mining Rewards & Transactions", 
                font=("Arial", 10, "bold"), bg="#1e293b", fg="#e2e8f0").pack(side="left")
        
        tk.Button(header_frame, text="🔄 Refresh", command=self.refresh_balance_from_blockchain,
                 font=("Arial", 8), bg="#3b82f6", fg="#ffffff",
                 relief="flat", cursor="hand2").pack(side="right", padx=4)
        
        # Transactions list with scrollbar
        list_frame = tk.Frame(parent, bg="#1e293b")
        list_frame.pack(fill="both", expand=True, padx=8, pady=(0,8))
        
        # Create treeview for transactions
        columns = ("Type", "Amount", "Status", "Time", "Hash", "Explorer")
        self.tx_tree = ttk.Treeview(list_frame, columns=columns, show="headings", height=15)
        
        # Configure columns
        self.tx_tree.heading("Type", text="Type")
        self.tx_tree.heading("Amount", text="Amount (GXC)")
        self.tx_tree.heading("Status", text="Status")
        self.tx_tree.heading("Time", text="Time")
        self.tx_tree.heading("Hash", text="Transaction Hash")
        self.tx_tree.heading("Explorer", text="Explorer Link")
        
        self.tx_tree.column("Type", width=120)
        self.tx_tree.column("Amount", width=120)
        self.tx_tree.column("Status", width=100)
        self.tx_tree.column("Time", width=150)
        self.tx_tree.column("Hash", width=150)
        self.tx_tree.column("Explorer", width=200)
        
        # Scrollbar
        scrollbar = ttk.Scrollbar(list_frame, orient="vertical", command=self.tx_tree.yview)
        self.tx_tree.configure(yscrollcommand=scrollbar.set)
        
        self.tx_tree.pack(side="left", fill="both", expand=True)
        scrollbar.pack(side="right", fill="y")
        
        # Bind double-click to open in explorer (only when user clicks)
        self.tx_tree.bind("<Double-1>", self.open_transaction_in_explorer)
        
        # Add right-click context menu for explorer link
        self.tx_tree.bind("<Button-3>", self.show_explorer_context_menu)
        
        # Style treeview
        style = ttk.Style()
        style.theme_use('default')
        style.configure("Treeview", background="#0f172a", foreground="#e2e8f0", 
                       fieldbackground="#0f172a", rowheight=25)
        style.configure("Treeview.Heading", background="#334155", foreground="#ffffff")
        style.map("Treeview", background=[("selected", "#7c3aed")])
    
    def add_transaction_to_list(self, tx: Dict):
        """Add a single transaction to the transactions list"""
        tx_type = tx.get('type', 'unknown')
        tx_hash = tx.get('hash', '')[:16] + '...' if tx.get('hash') else 'N/A'
        full_hash = tx.get('hash', '')
        
        # Determine transaction type display
        if tx_type == 'coinbase':
            type_display = "💰 Mining Reward"
            amount = tx.get('amount', 0)
        else:
            type_display = "⬇️ Received"
            amount = tx.get('amount', 0)
        
        # Format amount
        amount_str = f"{amount:.8f}" if amount >= 0 else f"-{abs(amount):.8f}"
        
        # Status
        confirmations = tx.get('confirmations', 0)
        if confirmations >= 6:
            status = "✅ Confirmed"
        elif confirmations > 0:
            status = f"⏳ {confirmations}/6"
        else:
            status = "⏳ Pending"
        
        # Time
        timestamp = tx.get('timestamp', 0)
        if timestamp:
            try:
                dt = datetime.fromtimestamp(timestamp)
                time_str = dt.strftime("%Y-%m-%d %H:%M:%S")
            except:
                time_str = "Unknown"
        else:
            time_str = "Unknown"
        
        # Explorer URL
        explorer_url = tx.get('explorer_url', '')
        if not explorer_url and full_hash:
            explorer_url = f"http://localhost:3000/tx/{full_hash}"
        
        # Insert at the top (newest first)
        item = self.tx_tree.insert("", 0, values=(
            type_display,
            amount_str,
            status,
            time_str,
            tx_hash,
            explorer_url  # Store URL for clicking
        ), tags=(full_hash, explorer_url))
        
        # Color code by type
        if tx_type == 'coinbase':
            self.tx_tree.set(item, "Type", "💰 Mining Reward")
    
    def calculate_total_earned_from_blockchain(self, transactions: List[Dict]) -> float:
        """Calculate total earned from actual blockchain coinbase transactions"""
        total = 0.0
        for tx in transactions:
            # Only count confirmed coinbase transactions (mining rewards)
            if tx.get('type') == 'coinbase' or tx.get('is_coinbase') or tx.get('isCoinbase'):
                confirmations = tx.get('confirmations', 0)
                # Only count if confirmed (1+ confirmations)
                if confirmations >= 1:
                    amount = tx.get('amount') or tx.get('value') or 0.0
                    if amount > 0:
                        total += amount
        return total
    
    def update_balance_display(self, balance: float = None):
        """Update balance displays with REAL blockchain data"""
        if balance is None:
            balance = self.last_balance_check
        
        # Store for comparison only, don't modify locally
        self.last_balance_check = balance
        balance_str = f"{balance:.8f} GXC"
        
        # Update main balance label
        if hasattr(self, 'balance_label'):
            self.balance_label.config(text=balance_str)
        
        # Update wallet balance label if it exists
        if hasattr(self, 'wallet_balance_label'):
            self.wallet_balance_label.config(text=f"Balance: {balance_str}")
        
        # Update stats label if it exists
        if hasattr(self, 'current_balance_label'):
            self.current_balance_label.config(text=balance_str)
    
    def update_earnings_display(self, total_earned: float):
        """Update earnings display with REAL blockchain data"""
        if hasattr(self, 'earned_label'):
            self.earned_label.config(text=f"{total_earned:.8f} GXC")
        
        if hasattr(self, 'total_earned_label'):
            self.total_earned_label.config(text=f"{total_earned:.8f} GXC")
    
    def get_address_transactions(self, address: str, limit: int = 100) -> List[Dict]:
        """Get transactions for an address from blockchain"""
        try:
            # Try multiple RPC methods
            methods = [
                ("getaddresstransactions", [address, limit]),
                ("listtransactions", [address, limit]),
                ("gettransactions", [address, limit]),
                ("getaddresshistory", [address]),
                ("searchrawtransactions", [address]),
            ]
            
            for method, params in methods:
                result = self.rpc_call(method, params, show_errors=False)
                if result:
                    if isinstance(result, list):
                        # Normalize transaction format
                        normalized = []
                        for tx in result:
                            if isinstance(tx, dict):
                                normalized.append(tx)
                            else:
                                # Try to convert if it's a different format
                                normalized.append({'hash': str(tx), 'value': 0.0})
                        return normalized
                    elif isinstance(result, dict):
                        if 'transactions' in result:
                            return result['transactions']
                        elif 'tx' in result:
                            return result['tx']
                        elif 'items' in result:
                            return result['items']
            return []
        except Exception as e:
            self.log(f"⚠️ Error getting transactions: {e}", "WARNING")
            return []
    
    def refresh_balance_from_blockchain(self):
        """Refresh balance and transactions from blockchain - REAL DATA ONLY"""
        if not self.wallet_address.get() or not self.node_connected:
            return
        
        address = self.wallet_address.get().strip()
        if not address:
            return
        
        self.log("🔄 Refreshing balance and transactions from blockchain...", "INFO")
        
        def fetch():
            try:
                # Try multiple methods to get balance
                balance = 0.0
                balance_methods = [
                    ("getbalance", [address]),
                    ("getaddressbalance", [address]),
                    ("getaccountbalance", [address]),
                ]
                
                for method, params in balance_methods:
                    result = self.rpc_call(method, params, show_errors=False)
                    if result is not None:
                        if isinstance(result, dict):
                            balance = float(result.get('balance', result.get('amount', 0.0)))
                        else:
                            balance = float(result) if result else 0.0
                        if balance > 0:
                            break
                
                # Also try to get balance from UTXOs
                if balance == 0.0:
                    try:
                        utxos_result = self.rpc_call("listunspent", [0, 9999999, [address]], show_errors=False)
                        if utxos_result and isinstance(utxos_result, list):
                            balance = sum(float(utxo.get('amount', utxo.get('value', 0.0))) for utxo in utxos_result)
                    except:
                        pass
                
                # Update balance display with REAL blockchain data
                self.root.after(0, lambda: self.update_balance_display(balance))
                self.root.after(0, lambda: self.log(f"💵 Balance from blockchain: {balance:.8f} GXC", "INFO"))
                
                # Get transactions from blockchain - REAL DATA ONLY
                transactions = self.get_address_transactions(address, limit=100)
                
                # Also try to get transactions from recent blocks if direct method fails
                if not transactions:
                    try:
                        # Get recent blocks and check for transactions to this address
                        blockchain_info = self.rpc_call("getblockchaininfo", [], show_errors=False)
                        if blockchain_info:
                            current_height = blockchain_info.get('blocks', blockchain_info.get('height', 0))
                            # Check last 10 blocks for transactions
                            for height in range(max(0, current_height - 10), current_height + 1):
                                block_data = self.rpc_call("getblock", [height, True], show_errors=False)
                                if block_data and isinstance(block_data, dict):
                                    block_txs = block_data.get('transactions', block_data.get('tx', []))
                                    for tx in block_txs:
                                        # Check if transaction involves this address
                                        tx_to = tx.get('to', tx.get('to_address', ''))
                                        tx_from = tx.get('from', tx.get('from_address', ''))
                                        # Check outputs for coinbase
                                        outputs = tx.get('outputs', tx.get('vout', []))
                                        for output in outputs:
                                            if isinstance(output, dict):
                                                out_addr = output.get('address', output.get('scriptPubKey', {}).get('addresses', [None])[0] if isinstance(output.get('scriptPubKey', {}).get('addresses'), list) else None)
                                                if out_addr == address:
                                                    # This is a transaction to our address
                                                    tx_data = {
                                                        'hash': tx.get('hash', tx.get('txid', '')),
                                                        'type': 'coinbase' if tx.get('is_coinbase') or tx.get('coinbase') else 'transfer',
                                                        'is_coinbase': tx.get('is_coinbase') or tx.get('coinbase') or False,
                                                        'amount': output.get('value', output.get('amount', 0.0)),
                                                        'value': output.get('value', output.get('amount', 0.0)),
                                                        'block_number': height,
                                                        'confirmations': max(0, current_height - height + 1),
                                                        'timestamp': block_data.get('time', block_data.get('timestamp', int(time.time())))
                                                    }
                                                    transactions.append(tx_data)
                    except Exception as e:
                        self.root.after(0, lambda: self.log(f"⚠️ Could not fetch transactions from blocks: {e}", "WARNING"))
                
                # Calculate total earned from ACTUAL blockchain transactions
                total_earned = self.calculate_total_earned_from_blockchain(transactions)
                
                # If still 0, try calculating from balance (if balance > 0)
                if total_earned == 0.0 and balance > 0:
                    # Balance exists but no transactions found - might be coinbase rewards
                    # Try to get block count to estimate
                    try:
                        blockchain_info = self.rpc_call("getblockchaininfo", [], show_errors=False)
                        if blockchain_info:
                            blocks = blockchain_info.get('blocks', 0)
                            # This is just for display - actual balance is what matters
                            total_earned = balance  # Use balance as earned if no transactions found
                    except:
                        pass
                
                # Update UI with REAL earnings from blockchain
                self.root.after(0, lambda: self.update_earnings_display(total_earned))
                self.root.after(0, lambda: self.log(f"💰 Total earned (from blockchain): {total_earned:.8f} GXC", "INFO"))
                
                # Log transaction count for debugging
                if transactions:
                    self.root.after(0, lambda: self.log(f"📋 Found {len(transactions)} transactions for address", "INFO"))
                else:
                    self.root.after(0, lambda: self.log(f"⚠️ No transactions found for address {address[:20]}...", "WARNING"))
                
                # Update transactions display if method exists
                if hasattr(self, 'update_transactions_display'):
                    self.root.after(0, lambda: self.update_transactions_display(transactions))
                
            except Exception as e:
                import traceback
                error_msg = f"❌ Failed to fetch from blockchain: {e}\n{traceback.format_exc()}"
                self.root.after(0, lambda: self.log(error_msg, "ERROR"))
        
        threading.Thread(target=fetch, daemon=True).start()
    
    def auto_refresh_balance(self):
        """Auto-refresh balance from blockchain every 30 seconds"""
        if self.wallet_address.get() and self.node_connected:
            self.refresh_balance_from_blockchain()
        
        # Schedule next refresh
        self.root.after(30000, self.auto_refresh_balance)  # Every 30 seconds
    
    def open_transaction_in_explorer(self, event):
        """Open transaction in blockchain explorer (only when user clicks)"""
        selection = self.tx_tree.selection()
        if selection:
            item = self.tx_tree.item(selection[0])
            values = item.get('values', [])
            tags = item.get('tags', [])
            
            # Get explorer URL from values or tags
            explorer_url = None
            if len(values) > 5:
                explorer_url = values[5]  # Explorer column
            elif len(tags) > 1:
                explorer_url = tags[1]  # URL in tags
            
            # Fallback: construct URL from hash
            if not explorer_url and len(tags) > 0:
                tx_hash = tags[0]
                if tx_hash:
                    explorer_url = f"http://localhost:3000/tx/{tx_hash}"
            
            if explorer_url:
                webbrowser.open(explorer_url)
                self.log(f"🌐 Opened transaction in explorer: {explorer_url}", "INFO")
            else:
                self.log("⚠️ No explorer URL available for this transaction", "WARNING")
    
    def show_explorer_context_menu(self, event):
        """Show context menu for explorer link"""
        selection = self.tx_tree.selection()
        if selection:
            item = self.tx_tree.item(selection[0])
            values = item.get('values', [])
            tags = item.get('tags', [])
            
            explorer_url = None
            if len(values) > 5:
                explorer_url = values[5]
            elif len(tags) > 1:
                explorer_url = tags[1]
            
            if explorer_url:
                menu = tk.Menu(self.root, tearoff=0)
                menu.add_command(label="Open in Explorer", command=lambda: webbrowser.open(explorer_url))
                menu.add_command(label="Copy URL", command=lambda: self.root.clipboard_clear() or self.root.clipboard_append(explorer_url))
                try:
                    menu.tk_popup(event.x_root, event.y_root)
                finally:
                    menu.grab_release()


def main():
    """Main entry point"""
    root = tk.Tk()
    app = TestnetMinerGUI(root)
    root.mainloop()


if __name__ == "__main__":
    main()