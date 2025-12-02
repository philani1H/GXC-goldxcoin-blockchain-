#!/usr/bin/env python3
"""
GXC GXHash Miner - Python Implementation with GUI
Connects directly to blockchain node and mines real blocks
Shows all mining rewards and transactions automatically
"""

import tkinter as tk
from tkinter import ttk, scrolledtext, messagebox
import threading
import time
import hashlib
import json
import requests
import sys
import signal
import webbrowser
from typing import Optional, Dict, Any, List
from datetime import datetime
import os

# Import centralized network configuration
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))
try:
    from config.railway_config import (
        get_network, get_rpc_url, get_rest_url, get_miner_address, get_network_config
    )
    USE_CENTRAL_CONFIG = True
except ImportError:
    USE_CENTRAL_CONFIG = False
    print("[WARNING] Central config not found, using default configuration")

class BlockchainClient:
    """Client for connecting to GXC blockchain node"""
    
    def __init__(self, rpc_url: str = None, rest_url: str = None):
        if USE_CENTRAL_CONFIG:
            self.rpc_url = rpc_url or get_rpc_url()
            self.rest_url = rest_url or get_rest_url()
            self.network = get_network()
        else:
            # Fallback to default configuration
            RAILWAY_NODE_URL = "https://gxc-chain112-blockchain-node-production.up.railway.app"
            self.rpc_url = rpc_url or os.environ.get('BLOCKCHAIN_RPC_URL', os.environ.get('RAILWAY_NODE_URL', RAILWAY_NODE_URL))
            self.rest_url = rest_url or os.environ.get('BLOCKCHAIN_REST_URL', self.rpc_url)
            self.network = os.environ.get('GXC_NETWORK', 'testnet')
        self.session = requests.Session()
        self.session.timeout = 10
        
    def rpc_call(self, method: str, params: list = None) -> Optional[Dict]:
        """Make RPC call to blockchain node"""
        if params is None:
            params = []
            
        payload = {
            "jsonrpc": "2.0",
            "method": method,
            "params": params,
            "id": 1
        }
        
        try:
            response = self.session.post(self.rpc_url, json=payload, timeout=30)
            response.raise_for_status()
            result = response.json()
            
            # Check for errors
            if 'error' in result:
                error = result['error']
                error_msg = error.get('message', 'Unknown error') if isinstance(error, dict) else str(error)
                error_code = error.get('code', -1) if isinstance(error, dict) else -1
                print(f"[RPC ERROR] {method}: {error_msg} (code: {error_code})")
                
                # Return error info for debugging
                if error_code == -32602:  # Invalid params
                    print(f"[RPC] Invalid params for {method}: {params}")
                elif error_code == -32603:  # Internal error
                    print(f"[RPC] Internal error for {method}")
                
                return None
                
            return result.get('result')
        except requests.exceptions.Timeout:
            print(f"[RPC TIMEOUT] {method} timed out after 30s")
            return None
        except Exception as e:
            print(f"[RPC EXCEPTION] {method}: {e}")
            return None
    
    def rest_call(self, endpoint: str) -> Optional[Dict]:
        """Make REST API call to blockchain node"""
        try:
            url = f"{self.rest_url}{endpoint}"
            response = self.session.get(url)
            response.raise_for_status()
            return response.json()
        except Exception as e:
            return None
    
    def get_blockchain_info(self) -> Optional[Dict]:
        """Get blockchain information"""
        return self.rpc_call("getblockchaininfo")
    
    def get_mining_info(self) -> Optional[Dict]:
        """Get mining information"""
        return self.rpc_call("getmininginfo")
    
    def get_block_template(self, algorithm: str = "gxhash") -> Optional[Dict]:
        """Get block template for mining"""
        # Try GXC-specific method first
        result = self.rpc_call("gxc_getBlockTemplate", [{"algorithm": algorithm}])
        if result:
            return result
        # Fallback to standard method with algorithm parameter
        result = self.rpc_call("getblocktemplate", [{"algorithm": algorithm}])
        if result:
            return result
        # Last resort: try without parameters (for backward compatibility)
        return self.rpc_call("getblocktemplate", [])
    
    def submit_block(self, block_data: Dict) -> Optional[str]:
        """Submit mined block to blockchain, returns transaction hash"""
        # Try multiple submission methods for compatibility
        methods = [
            ("gxc_submitBlock", [block_data]),  # GXC-specific method
            ("submitblock", [block_data]),  # Direct object (preferred)
            ("submitblock", [json.dumps(block_data)]),  # Standard JSON-RPC string
        ]
        
        for method, params in methods:
            try:
                result = self.rpc_call(method, params)
                
                # Check if submission was successful
                # None typically means success in RPC implementations
                if result is None:
                    return block_data.get('hash') or block_data.get('blockHash')
                
                # If result is True or a hash string, block was accepted
                if result is True or (isinstance(result, str) and len(result) > 0):
                    return block_data.get('hash') or block_data.get('blockHash')
                
                # If result is a dict with success status
                if isinstance(result, dict):
                    if result.get('accepted', False) or result.get('success', False):
                        return block_data.get('hash') or block_data.get('blockHash')
                    # Check for error message
                    if 'error' in result:
                        error_msg = result.get('error', 'Unknown error')
                        if isinstance(error_msg, dict):
                            error_msg = error_msg.get('message', str(error_msg))
                        print(f"[WARNING] Block submission error ({method}): {error_msg}")
                        # Continue to next method
                        continue
                
                # If result is False or empty string, try next method
                if result is False or (isinstance(result, str) and ('rejected' in result.lower() or 'error' in result.lower())):
                    print(f"[WARNING] Block rejected by {method}: {result}")
                    continue
                    
            except Exception as e:
                # Continue to next method on error
                print(f"[WARNING] Exception in {method}: {e}")
                continue
        
        # All methods failed
        print(f"[ERROR] All block submission methods failed for block {block_data.get('hash', 'unknown')[:16]}...")
        return None
    
    def get_difficulty(self) -> Optional[float]:
        """Get current mining difficulty"""
        info = self.get_mining_info()
        if info:
            return info.get('difficulty') or info.get('currentDifficulty')
        return None
    
    def get_address_balance(self, address: str) -> float:
        """Get balance for an address - Use RPC for reliability"""
        # Try RPC methods first (more reliable)
        methods = [
            ("gxc_getBalance", [address]),
            ("getbalance", [address]),
            ("getaddressbalance", [address]),
        ]
        
        for method, params in methods:
            try:
                result = self.rpc_call(method, params)
                if result is not None:
                    if isinstance(result, (int, float)):
                        return float(result)
                    elif isinstance(result, dict):
                        balance = result.get('balance') or result.get('amount') or 0.0
                        return float(balance)
            except:
                continue
        
        # Fallback to REST API
        try:
            result = self.rest_call(f"/api/v1/address/{address}/balance")
            if result:
                return float(result.get('balance', 0.0))
        except:
            pass
        return 0.0
    
    def get_current_block_height(self) -> int:
        """Get current blockchain height"""
        info = self.get_blockchain_info()
        if info:
            return info.get('blocks') or info.get('height') or 0
        return 0
    
    def get_address_transactions(self, address: str, limit: int = 50) -> List[Dict]:
        """Get transactions for an address - Use RPC for reliability"""
        # Try RPC methods first (more reliable)
        methods = [
            ("gxc_getTransactionsByAddress", [address, limit]),
            ("getaddresstransactions", [address, limit]),
            ("listtransactions", [address, limit]),
        ]
        
        for method, params in methods:
            try:
                result = self.rpc_call(method, params)
                if result is not None:
                    if isinstance(result, list):
                        return result
                    elif isinstance(result, dict):
                        txs = result.get('transactions') or result.get('txs') or []
                        if isinstance(txs, list):
                            return txs
            except:
                continue
        
        # Fallback to REST API
        try:
            result = self.rest_call(f"/api/v1/address/{address}/transactions?limit={limit}")
            if isinstance(result, list):
                return result
            elif isinstance(result, dict) and 'transactions' in result:
                return result['transactions']
        except:
            pass
        return []
    
    def verify_block_confirmed(self, block_hash: str, block_height: int) -> bool:
        """Verify that a block is confirmed on the blockchain"""
        try:
            # Get current blockchain height
            current_height = self.get_current_block_height()
            if current_height == 0:
                return False
            
            # Check if block is at least 6 blocks deep (confirmed)
            if current_height < block_height + 6:
                return False
            
            # Try to get the block by hash to verify it exists
            block_info = self.rpc_call("getblock", [block_hash])
            if block_info:
                # Verify block height matches
                block_height_from_node = block_info.get('height') or block_info.get('block_number') or 0
                if block_height_from_node == block_height:
                    confirmations = current_height - block_height + 1
                    return confirmations >= 6
            return False
        except:
            return False


class GXCMiner:
    """GXC GXHash Miner with GUI - Connects to real blockchain"""
    
    def __init__(self, root):
        self.root = root
        self.root.title("GXC Miner Pro - GXHash")
        self.root.geometry("900x650")
        self.root.minsize(800, 600)
        self.root.configure(bg="#0f172a")
        
        # Blockchain client - auto-detect endpoints
        self.client = None
        RAILWAY_NODE_URL = "https://gxc-chain112-blockchain-node-production.up.railway.app"
        self.rpc_url = os.environ.get('BLOCKCHAIN_RPC_URL', os.environ.get('RAILWAY_NODE_URL', RAILWAY_NODE_URL))
        self.rest_url = os.environ.get('BLOCKCHAIN_REST_URL', self.rpc_url)
        self.explorer_url = "https://gxc-blockchain.vercel.app"
        
        # Mining parameters
        self.wallet_address = tk.StringVar()
        self.threads = tk.IntVar(value=os.cpu_count() or 4)
        
        # Mining state
        self.mining = False
        self.node_connected = False
        self.hash_rate = 0
        self.total_hashes = 0
        self.blocks_found = 0
        self.shares_accepted = 0
        self.shares_rejected = 0
        # NO LOCAL EARNINGS TRACKING - Get from blockchain only
        self.last_balance_check = 0.0  # Track last known balance for comparison
        self.current_job = None
        self.job_lock = threading.Lock()
        self.shutdown = False
        self.transactions = []  # Store transaction history
        
        # Setup signal handlers
        signal.signal(signal.SIGINT, self._signal_handler)
        signal.signal(signal.SIGTERM, self._signal_handler)
        
        self.setup_ui()
        self.log("🚀 GXC GXHash Miner initialized", "INFO")
        
        # Auto-connect to blockchain in background
        self.root.after(500, self.auto_connect)
        
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
        
        def connect():
            try:
                # Try default localhost endpoints
                self.client = BlockchainClient(self.rpc_url, self.rest_url)
                info = self.client.get_blockchain_info()
                
                if info:
                    self.node_connected = True
                    self.status_label.config(text="● Connected", fg="#86efac")
                    self.root.after(0, lambda: self.log("✅ Auto-connected to blockchain", "SUCCESS"))
                    self.root.after(0, lambda: self.log(f"📊 Chain: {info.get('chain', 'unknown')}", "INFO"))
                    self.root.after(0, lambda: self.log(f"📊 Blocks: {info.get('blocks', 0)}", "INFO"))
                    
                    # Auto-refresh balance if wallet address is set
                    if self.wallet_address.get():
                        self.root.after(1000, self.refresh_balance_only)
                else:
                    # Try alternative endpoints (testnet ports)
                    alt_endpoints = [
                        ("http://127.0.0.1:8545", "http://127.0.0.1:8080"),
                        ("http://localhost:18332", "http://localhost:8080"),  # Testnet port
                    ]
                    
                    for rpc, rest in alt_endpoints:
                        self.client = BlockchainClient(rpc, rest)
                        info = self.client.get_blockchain_info()
                        if info:
                            self.rpc_url = rpc
                            self.rest_url = rest
                            self.node_connected = True
                            self.status_label.config(text="● Connected", fg="#86efac")
                            self.root.after(0, lambda: self.log("✅ Auto-connected to blockchain", "SUCCESS"))
                            return
                    
                    self.node_connected = False
                    self.status_label.config(text="● Disconnected", fg="#fca5a5")
                    self.root.after(0, lambda: self.log("⚠️ Could not auto-connect. Start blockchain node first.", "WARNING"))
            except Exception as e:
                self.node_connected = False
                self.status_label.config(text="● Disconnected", fg="#fca5a5")
                self.root.after(0, lambda: self.log("⚠️ Auto-connect failed. Start blockchain node first.", "WARNING"))
        
        threading.Thread(target=connect, daemon=True).start()
    
    def auto_refresh_balance(self):
        """Auto-refresh balance from blockchain every 30 seconds"""
        if self.wallet_address.get() and self.client and self.node_connected:
            self.refresh_balance_only()
        
        # Schedule next refresh
        self.root.after(30000, self.auto_refresh_balance)  # Every 30 seconds
    
    def setup_ui(self):
        # Header with status
        header = tk.Frame(self.root, bg="#7c3aed", height=50)
        header.pack(fill="x")
        header.pack_propagate(False)
        
        tk.Label(header, text="⚡ GXC MINER PRO - GXHash", 
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
        
        # Wallet section (endpoint hidden - auto-detected)
        wallet_frame = tk.LabelFrame(mining_tab, text="Wallet Address", font=("Arial", 9, "bold"),
                                     bg="#1e293b", fg="#e2e8f0", relief="flat")
        wallet_frame.pack(fill="x", padx=8, pady=8)
        
        tk.Label(wallet_frame, text="GXC Address (for mining rewards):", font=("Arial", 8), 
                bg="#1e293b", fg="#cbd5e1").pack(anchor="w", padx=8, pady=(8,2))
        
        entry_frame = tk.Frame(wallet_frame, bg="#1e293b")
        entry_frame.pack(fill="x", padx=8, pady=(0,4))
        
        tk.Entry(entry_frame, textvariable=self.wallet_address, font=("Arial", 9), 
                bg="#334155", fg="#ffffff", relief="flat", 
                insertbackground="#ffffff").pack(side="left", fill="x", expand=True, ipady=4)
        
        refresh_btn = tk.Button(entry_frame, text="🔄", command=self.refresh_balance_only,
                              font=("Arial", 8), bg="#3b82f6", fg="#ffffff",
                              relief="flat", cursor="hand2", width=3)
        refresh_btn.pack(side="right", padx=(4,0))
        
        # Balance display
        self.wallet_balance_label = tk.Label(wallet_frame, text="Balance: Loading...", 
                                            font=("Arial", 8, "bold"), 
                                            bg="#1e293b", fg="#10b981")
        self.wallet_balance_label.pack(anchor="w", padx=8, pady=(4,0))
        
        tk.Label(wallet_frame, text="💰 Mining rewards are automatically credited to this address", 
                font=("Arial", 7), bg="#1e293b", fg="#94a3b8").pack(anchor="w", padx=8, pady=(4,8))
        
        # Settings
        settings_frame = tk.LabelFrame(mining_tab, text="Settings", font=("Arial", 9, "bold"),
                                       bg="#1e293b", fg="#e2e8f0", relief="flat")
        settings_frame.pack(fill="x", padx=8, pady=(0,8))
        
        left_col = tk.Frame(settings_frame, bg="#1e293b")
        left_col.pack(side="left", fill="both", expand=True, padx=(8,4), pady=8)
        
        right_col = tk.Frame(settings_frame, bg="#1e293b")
        right_col.pack(side="right", fill="both", expand=True, padx=(4,8), pady=8)
        
        # Threads
        tk.Label(left_col, text="Mining Threads:", font=("Arial", 8), 
                bg="#1e293b", fg="#cbd5e1").pack(anchor="w", pady=(0,2))
        
        self.thread_label = tk.Label(left_col, text=f"Threads: {self.threads.get()}", 
                                     font=("Arial", 8), bg="#1e293b", fg="#cbd5e1")
        self.thread_label.pack(anchor="w", pady=(0,2))
        
        thread_scale = tk.Scale(left_col, from_=1, to=os.cpu_count() or 16, variable=self.threads,
                               orient="horizontal", bg="#1e293b", fg="#ffffff",
                               troughcolor="#334155", highlightthickness=0, showvalue=0,
                               command=lambda v: self.thread_label.config(text=f"Threads: {int(float(v))}"))
        thread_scale.pack(fill="x", pady=(0,8))
        
        # Algorithm info
        tk.Label(right_col, text="Algorithm: GXHash", font=("Arial", 8, "bold"), 
                bg="#1e293b", fg="#10b981").pack(anchor="w", pady=(0,2))
        
        tk.Label(right_col, text="CPU-Optimized Mining", font=("Arial", 7), 
                bg="#1e293b", fg="#94a3b8").pack(anchor="w", pady=(0,8))
        
        # Control buttons
        btn_frame = tk.Frame(mining_tab, bg="#1e293b")
        btn_frame.pack(fill="x", padx=8, pady=(0,8))
        
        self.start_btn = tk.Button(btn_frame, text="⚡ START MINING", command=self.start_mining,
                                   font=("Arial", 10, "bold"), bg="#10b981", fg="#ffffff",
                                   relief="flat", cursor="hand2")
        self.start_btn.pack(side="left", fill="x", expand=True, padx=(0,4), ipady=8)
        
        self.stop_btn = tk.Button(btn_frame, text="⏸ STOP", command=self.stop_mining,
                                  font=("Arial", 10, "bold"), bg="#ef4444", fg="#ffffff",
                                  relief="flat", state="disabled", cursor="hand2")
        self.stop_btn.pack(side="left", fill="x", expand=True, padx=4, ipady=8)
        
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
        
    def setup_transactions_tab(self, parent):
        """Setup transactions display tab"""
        # Header with refresh button
        header_frame = tk.Frame(parent, bg="#1e293b")
        header_frame.pack(fill="x", padx=8, pady=8)
        
        tk.Label(header_frame, text="Mining Rewards & Transactions", 
                font=("Arial", 10, "bold"), bg="#1e293b", fg="#e2e8f0").pack(side="left")
        
        tk.Button(header_frame, text="🔄 Refresh", command=self.refresh_transactions,
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
        
    def create_mini_stat(self, parent, title, value, color, col):
        card = tk.Frame(parent, bg=color, relief="flat")
        card.grid(row=0, column=col, padx=2, sticky="nsew")
        parent.grid_columnconfigure(col, weight=1)
        
        tk.Label(card, text=title, font=("Arial", 7), 
                bg=color, fg="#ffffff").pack(pady=(6,0))
        
        value_label = tk.Label(card, text=value, font=("Arial", 10, "bold"), 
                              bg=color, fg="#ffffff")
        value_label.pack(pady=(0,6))
        
        if title == "Hash Rate":
            self.hashrate_label = value_label
        elif title == "Balance":
            self.balance_label = value_label
        elif title == "Blocks":
            self.blocks_label = value_label
        elif title == "Earned":
            self.earned_label = value_label
            
    def create_stat_display(self, parent):
        container = tk.Frame(parent, bg="#1e293b")
        container.pack(fill="both", expand=True, padx=8, pady=8)
        
        stats_data = [
            ("Total Hashes", "0", "#3b82f6"),
            ("Blocks Found", "0", "#10b981"),
            ("Shares Accepted", "0", "#8b5cf6"),
            ("Shares Rejected", "0", "#f59e0b"),
            ("Total Earned", "0.000000 GXC", "#ef4444"),
            ("Current Balance", "0.000000 GXC", "#06b6d4")
        ]
        
        for i, (label, value, color) in enumerate(stats_data):
            row = i // 2
            col = i % 2
            
            card = tk.Frame(container, bg=color, relief="flat")
            card.grid(row=row, column=col, padx=4, pady=4, sticky="nsew")
            container.grid_columnconfigure(col, weight=1)
            container.grid_rowconfigure(row, weight=1)
            
            tk.Label(card, text=label, font=("Arial", 9), 
                    bg=color, fg="#ffffff").pack(pady=(15,5))
            
            val_label = tk.Label(card, text=value, font=("Arial", 12, "bold"), 
                                bg=color, fg="#ffffff")
            val_label.pack(pady=(0,15))
            
            # Store references
            if label == "Total Hashes":
                self.total_hash_label = val_label
            elif label == "Blocks Found":
                self.total_blocks_label = val_label
            elif label == "Shares Accepted":
                self.total_accepted_label = val_label
            elif label == "Shares Rejected":
                self.total_rejected_label = val_label
            elif label == "Total Earned":
                self.total_earned_stats_label = val_label
            elif label == "Current Balance":
                self.current_balance_stats_label = val_label
                
    def log(self, message, level="INFO"):
        timestamp = datetime.now().strftime("%H:%M:%S")
        self.log_text.insert("end", f"[{timestamp}] {message}\n", level)
        self.log_text.see("end")
    
    def calculate_total_earned_from_blockchain(self, transactions: List[Dict]) -> float:
        """Calculate total earned from actual blockchain coinbase transactions"""
        total = 0.0
        current_height = self.client.get_current_block_height() if self.client else 0
        
        for tx in transactions:
            # Only count confirmed coinbase transactions (mining rewards)
            if tx.get('type') == 'coinbase' or tx.get('is_coinbase') or tx.get('isCoinbase'):
                confirmations = tx.get('confirmations', 0)
                block_number = tx.get('block_number') or tx.get('block') or 0
                
                # Calculate confirmations if not provided
                if confirmations == 0 and block_number > 0 and current_height > 0:
                    confirmations = max(0, current_height - block_number + 1)
                
                # Only count if confirmed (6+ confirmations for safety)
                if confirmations >= 6:
                    amount = tx.get('amount') or tx.get('value') or 0.0
                    if amount > 0:
                        total += amount
                        self.log(f"✅ Confirmed coinbase: {amount} GXC (block {block_number}, {confirmations} confirmations)", "SUCCESS")
        return total
    
    def refresh_transactions(self):
        """Refresh transactions and balance from blockchain - REAL DATA ONLY"""
        if not self.wallet_address.get() or not self.client:
            return
        
        address = self.wallet_address.get().strip()
        if not address:
            return
        
        self.log("🔄 Refreshing balance and transactions from blockchain...", "INFO")
        
        def fetch():
            try:
                # Get current blockchain height to detect resets
                current_height = self.client.get_current_block_height()
                
                # Get balance from blockchain - REAL DATA ONLY
                balance = self.client.get_address_balance(address)
                if balance is None:
                    balance = 0.0
                
                # Check for significant balance drop (possible chain reset)
                if hasattr(self, 'last_known_balance') and self.last_known_balance > 0:
                    balance_drop = self.last_known_balance - balance
                    if balance_drop > 100:  # Significant drop (>100 GXC)
                        self.root.after(0, lambda: self.log(
                            f"⚠️ WARNING: Balance dropped from {self.last_known_balance:.8f} to {balance:.8f} GXC "
                            f"(drop: {balance_drop:.8f} GXC). Possible chain reorganization or node reset!",
                            "WARNING"))
                        self.root.after(0, lambda: self.log(
                            f"⚠️ Current blockchain height: {current_height}. Check if node reset or chain reorganized.",
                            "WARNING"))
                
                self.last_known_balance = balance
                
                # Update balance display with REAL blockchain data
                self.root.after(0, lambda: self.update_balance_display(balance))
                self.root.after(0, lambda: self.log(f"💵 Balance from blockchain: {balance:.8f} GXC", "INFO"))
                
                # Get transactions from blockchain - REAL DATA ONLY
                transactions = self.client.get_address_transactions(address, limit=100)
                
                # Calculate total earned from ACTUAL blockchain transactions (only confirmed)
                total_earned = self.calculate_total_earned_from_blockchain(transactions)
                
                # Check for transaction count drop (possible chain reset)
                if hasattr(self, 'last_transaction_count') and self.last_transaction_count > 0:
                    tx_drop = self.last_transaction_count - len(transactions)
                    if tx_drop > 5:  # Significant drop (>5 transactions)
                        self.root.after(0, lambda: self.log(
                            f"⚠️ WARNING: Transaction count dropped from {self.last_transaction_count} to {len(transactions)}. "
                            f"Possible chain reorganization or node reset!",
                            "WARNING"))
                
                self.last_transaction_count = len(transactions)
                
                # Update UI with REAL earnings from blockchain
                self.root.after(0, lambda: self.update_earnings_display(total_earned))
                self.root.after(0, lambda: self.update_transactions_display(transactions))
                self.root.after(0, lambda: self.log(f"📊 Loaded {len(transactions)} transactions", "INFO"))
                self.root.after(0, lambda: self.log(f"💰 Total earned (from blockchain, confirmed only): {total_earned:.8f} GXC", "INFO"))
                self.root.after(0, lambda: self.log(f"📏 Current blockchain height: {current_height}", "INFO"))
            except Exception as e:
                self.root.after(0, lambda: self.log(f"❌ Failed to fetch from blockchain: {e}", "ERROR"))
                import traceback
                self.root.after(0, lambda: self.log(f"❌ Error details: {traceback.format_exc()}", "ERROR"))
        
        threading.Thread(target=fetch, daemon=True).start()
    
    def refresh_balance_only(self):
        """Refresh only balance from blockchain (faster)"""
        if not self.wallet_address.get() or not self.client:
            return
        
        address = self.wallet_address.get().strip()
        if not address:
            return
        
        def fetch():
            try:
                balance = self.client.get_address_balance(address)
                self.root.after(0, lambda: self.update_balance_display(balance))
            except Exception as e:
                self.root.after(0, lambda: self.log(f"❌ Failed to fetch balance: {e}", "ERROR"))
        
        threading.Thread(target=fetch, daemon=True).start()
    
    def add_transaction_to_list(self, tx: Dict):
        """Add a single transaction to the transactions list"""
        tx_type = tx.get('type', 'unknown')
        tx_hash = tx.get('hash', '')[:16] + '...' if tx.get('hash') else 'N/A'
        full_hash = tx.get('hash', '')
        
        # Determine transaction type display
        if tx_type == 'coinbase':
            type_display = "💰 Mining Reward"
            amount = tx.get('amount', 0)
        elif tx.get('to') == self.wallet_address.get().strip():
            type_display = "⬇️ Received"
            amount = tx.get('amount', 0)
        else:
            type_display = "⬆️ Sent"
            amount = -tx.get('amount', 0)
        
        # Format amount
        amount_str = f"{amount:.8f}" if amount >= 0 else f"-{abs(amount):.8f}"
        
        # Calculate confirmations if not provided
        confirmations = tx.get('confirmations', 0)
        if confirmations == 0 and (tx.get('block_number') or tx.get('block')):
            # Calculate confirmations: current_height - tx_block + 1
            current_height = self.client.get_current_block_height()
            tx_block = tx.get('block_number') or tx.get('block') or 0
            if tx_block > 0:
                confirmations = max(0, current_height - tx_block + 1)
        
        # Status with confirmation display
        if confirmations >= 6:
            status = f"✅ Confirmed ({confirmations})"
        elif confirmations > 0:
            status = f"⏳ {confirmations}/6 confirmations"
        else:
            status = "⏳ Pending (0 confirmations)"
        
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
            explorer_url = f"{self.explorer_url}/tx/{full_hash}"
        
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
    
    def update_transactions_display(self, transactions: List[Dict]):
        """Update transactions treeview"""
        # Clear existing items
        for item in self.tx_tree.get_children():
            self.tx_tree.delete(item)
        
        # Sort by timestamp (newest first)
        sorted_txs = sorted(transactions, key=lambda x: x.get('timestamp', 0), reverse=True)
        
        for tx in sorted_txs:
            tx_type = tx.get('type', 'unknown')
            tx_hash = tx.get('hash', '')[:16] + '...' if tx.get('hash') else 'N/A'
            full_hash = tx.get('hash', '')
            
            # Determine transaction type display
            if tx_type == 'coinbase':
                type_display = "💰 Mining Reward"
                amount = tx.get('amount', 0)
            elif tx.get('to') == self.wallet_address.get().strip():
                type_display = "⬇️ Received"
                amount = tx.get('amount', 0)
            else:
                type_display = "⬆️ Sent"
                amount = -tx.get('amount', 0)
            
            # Format amount
            amount_str = f"{amount:.8f}" if amount >= 0 else f"-{abs(amount):.8f}"
            
            # Calculate confirmations if not provided
            confirmations = tx.get('confirmations', 0)
            if confirmations == 0 and tx.get('block_number') or tx.get('block'):
                # Calculate confirmations: current_height - tx_block + 1
                current_height = self.client.get_current_block_height()
                tx_block = tx.get('block_number') or tx.get('block') or 0
                if tx_block > 0:
                    confirmations = max(0, current_height - tx_block + 1)
            
            # Status with confirmation display
            if confirmations >= 6:
                status = f"✅ Confirmed ({confirmations})"
            elif confirmations > 0:
                status = f"⏳ {confirmations}/6 confirmations"
            else:
                status = "⏳ Pending (0 confirmations)"
            
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
                explorer_url = f"{self.explorer_url}/tx/{full_hash}"
            
            # Insert into treeview
            item = self.tx_tree.insert("", "end", values=(
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
    
    def update_balance_display(self, balance: float):
        """Update balance display with REAL blockchain data"""
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
        if hasattr(self, 'current_balance_stats_label'):
            self.current_balance_stats_label.config(text=balance_str)
    
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
                    explorer_url = f"{self.explorer_url}/tx/{tx_hash}"
            
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
    
    def get_work(self) -> Optional[Dict]:
        """Get mining work from blockchain"""
        if not self.client:
            return None
            
        template = self.client.get_block_template()
        
        if not template:
            return None
        
        work = {
            'job_id': template.get('job_id') or f"job_{int(time.time())}",
            'version': template.get('version', 1),
            'prev_hash': template.get('previousblockhash') or template.get('prevHash', '0' * 64),
            'merkle_root': template.get('merkleroot') or template.get('merkleRoot', ''),
            'timestamp': template.get('curtime') or template.get('timestamp', int(time.time())),
            'bits': template.get('bits') or template.get('target', '0000ffff00000000000000000000000000000000000000000000000000000000'),
            'difficulty': template.get('difficulty', 1),
            'height': template.get('height', 0),
            'transactions': template.get('transactions', []),
            'coinbase_value': template.get('coinbasevalue') or template.get('coinbaseValue')  # Get from blockchain, no fallback
        }
        
        return work
    
    def compute_gxhash(self, block_header: str, nonce: int, transactions: list = None) -> str:
        """Compute GXHash - GXC's custom CPU-friendly algorithm"""
        data = f"{block_header}{nonce}"
        hash_result = hashlib.sha256(data.encode()).hexdigest()
        
        if transactions:
            tx_data = "".join([str(tx.get('hash', '')) for tx in transactions[:10]])
            hash_result = hashlib.sha256((hash_result + tx_data).encode()).hexdigest()
        
        for round_num in range(8):
            hash_result = hashlib.sha256((hash_result + str(round_num) + str(nonce)).encode()).hexdigest()
        
        return hash_result
    
    def check_difficulty(self, hash_value: str, target: str) -> bool:
        """Check if hash meets difficulty target"""
        try:
            target_int = int(target, 16) if target.startswith('0x') else int(target, 16)
            hash_int = int(hash_value, 16)
            return hash_int <= target_int
        except:
            target_zeros = len(target) - len(target.lstrip('0'))
            hash_zeros = len(hash_value) - len(hash_value.lstrip('0'))
            return hash_zeros >= target_zeros
    
    def update_work(self):
        """Update mining work from blockchain"""
        work = self.get_work()
        if work:
            with self.job_lock:
                self.current_job = work
            return True
        return False
    
    def mine_thread(self, thread_id: int):
        """Mining thread worker"""
        nonce_start = thread_id * (2**32 // self.threads.get())
        nonce = nonce_start
        hash_count = 0
        last_update = time.time()
        
        while self.mining and not self.shutdown:
            with self.job_lock:
                work = self.current_job
            
            if not work:
                time.sleep(0.1)
                continue
            
            block_header = (
                f"{work['version']:08x}"
                f"{work['prev_hash']}"
                f"{work['merkle_root']}"
                f"{work['timestamp']:08x}"
                f"{work['bits']}"
                f"{nonce:08x}"
            )
            
            hash_result = self.compute_gxhash(block_header, nonce, work.get('transactions', []))
            hash_count += 1
            nonce += 1
            self.total_hashes += 1
            
            if self.check_difficulty(hash_result, work['bits']):
                # Get reward from blockchain template - NO HARDCODED VALUES
                reward_amount = work.get('coinbase_value') or work.get('coinbaseValue')
                if not reward_amount:
                    # Try to get from blockchain info
                    try:
                        blockchain_info = self.client.get_blockchain_info()
                        if blockchain_info:
                            reward_amount = blockchain_info.get('block_reward') or blockchain_info.get('reward')
                    except:
                        pass
                
                # If still no reward, log warning but continue
                if not reward_amount:
                    self.root.after(0, lambda: self.log(f"⚠️ Warning: Could not get reward amount from blockchain", "WARNING"))
                    reward_amount = 0.0  # Don't assume, let blockchain determine
                
                self.root.after(0, lambda: self.log(f"🎉 Block found by thread {thread_id}!", "SUCCESS"))
                self.root.after(0, lambda: self.log(f"   Hash: {hash_result[:32]}...", "INFO"))
                if reward_amount > 0:
                    self.root.after(0, lambda: self.log(f"   Reward: {reward_amount} GXC (from blockchain)", "SUCCESS"))
                
                # Build complete block with all required fields for testnet validation
                block = {
                    'version': work['version'],
                    'previousblockhash': work['prev_hash'],
                    'merkleroot': work['merkle_root'],
                    'timestamp': work['timestamp'],
                    'bits': work['bits'],
                    'nonce': nonce - 1,
                    'hash': hash_result,
                    'blockHash': hash_result,  # Alternative field name
                    'height': work['height'],
                    'blockNumber': work['height'],
                    'transactions': work.get('transactions', []),
                    'transactionCount': len(work.get('transactions', [])),
                    'difficulty': work.get('difficulty', 1),
                    'algorithm': 'gxhash',
                    'network': self.client.network if hasattr(self.client, 'network') else 'testnet'
                }
                
                # Add coinbase transaction if wallet address is set and reward is known
                if self.wallet_address.get() and reward_amount > 0:
                    coinbase = {
                        'type': 'coinbase',
                        'hash': hash_result,  # Use block hash as transaction ID
                        'txHash': hash_result,
                        'inputs': [],
                        'outputs': [{
                            'address': self.wallet_address.get(),
                            'amount': reward_amount
                        }],
                        'timestamp': int(time.time()),
                        'value': reward_amount,
                        'isCoinbase': True
                    }
                    block['transactions'].insert(0, coinbase)
                    block['transactionCount'] = len(block['transactions'])
                
                # Ensure merkle root is calculated if not provided
                if not block.get('merkleroot') or block['merkleroot'] == '':
                    # Calculate merkle root from transactions
                    tx_hashes = [tx.get('hash', tx.get('txHash', '')) for tx in block['transactions']]
                    if tx_hashes:
                        merkle_data = ''.join(tx_hashes)
                        block['merkleroot'] = hashlib.sha256(merkle_data.encode()).hexdigest()
                
                # Submit block with proper error handling
                try:
                    tx_hash = self.client.submit_block(block)
                except Exception as e:
                    self.root.after(0, lambda: self.log(f"❌ Block submission error: {e}", "ERROR"))
                    tx_hash = None
                
                if tx_hash:
                    self.blocks_found += 1
                    self.shares_accepted += 1
                    # DO NOT increment total_earned locally - get from blockchain
                    
                    self.root.after(0, lambda: self.log("✅ Block submitted and accepted!", "SUCCESS"))
                    self.root.after(0, lambda: self.log(f"⏳ Waiting for blockchain confirmation...", "INFO"))
                    if reward_amount > 0:
                        self.root.after(0, lambda: self.log(f"💰 Expected reward: {reward_amount} GXC (will verify from blockchain)", "INFO"))
                    
                    # Show explorer URL (but don't open browser automatically)
                    explorer_url = f"{self.explorer_url}/tx/{tx_hash}"
                    block_url = f"{self.explorer_url}/block/{work['height']}"
                    self.root.after(0, lambda: self.log(f"🔗 Block Explorer: {block_url}", "INFO"))
                    self.root.after(0, lambda: self.log(f"🔗 Transaction Explorer: {explorer_url}", "INFO"))
                    
                    # Refresh balance and transactions from blockchain to get REAL data
                    # Wait a bit for block to be processed, then verify from blockchain
                    # Verify block is actually confirmed on chain
                    import time
                    time.sleep(2)  # Wait for block to be processed
                    
                    # Verify block confirmation
                    block_hash = block_data.get('hash') or block_data.get('blockHash')
                    if block_hash and block_height:
                        is_confirmed = self.client.verify_block_confirmed(block_hash, block_height)
                        if not is_confirmed:
                            self.root.after(0, lambda: self.log(f"⚠️ Block {block_height} not yet confirmed (may be orphaned)", "WARNING"))
                    self.root.after(5000, self.refresh_transactions)  # Wait 5 seconds for block processing
                    self.root.after(15000, self.refresh_transactions)  # Refresh again after 15 seconds to get confirmed data
                else:
                    self.shares_rejected += 1
                    self.root.after(0, lambda: self.log("❌ Block rejected", "ERROR"))
                
                self.update_work()
                nonce = nonce_start + (thread_id * (2**32 // self.threads.get()))
            
            if time.time() - last_update >= 1.0:
                self.hash_rate = hash_count / (time.time() - last_update)
                hash_count = 0
                last_update = time.time()
                self.root.after(0, self.update_stats)
    
    def show_explorer_link(self, tx_hash: str, url: str, amount: float):
        """Show dialog with explorer link (user must click to open)"""
        msg = f"✅ Block mined successfully!\n\n"
        msg += f"💰 Reward: {amount} GXC\n"
        msg += f"🔐 Transaction: {tx_hash[:16]}...\n\n"
        msg += f"Explorer URL:\n{url}\n\n"
        msg += f"Click the link in the Transactions tab to view in explorer."
        
        messagebox.showinfo("Block Mined!", msg)
        self.log(f"🔗 Explorer URL: {url}", "INFO")
    
    def update_stats(self):
        """Update UI statistics"""
        rate = self.hash_rate
        if rate >= 1e9:
            hr = f"{rate/1e9:.2f} GH/s"
        elif rate >= 1e6:
            hr = f"{rate/1e6:.2f} MH/s"
        elif rate >= 1e3:
            hr = f"{rate/1e3:.2f} KH/s"
        else:
            hr = f"{rate:.0f} H/s"
            
        self.hashrate_label.config(text=hr)
        self.blocks_label.config(text=f"{self.blocks_found}")
        # Earnings will be updated from blockchain refresh, not here
        
        # Update stats tab
        try:
            self.total_hash_label.config(text=f"{self.total_hashes:,}")
            self.total_blocks_label.config(text=f"{self.blocks_found}")
            self.total_accepted_label.config(text=f"{self.shares_accepted}")
            self.total_rejected_label.config(text=f"{self.shares_rejected}")
            # Earnings and balance updated from blockchain only
        except:
            pass
        
        # Auto-refresh balance and transactions periodically
        if self.wallet_address.get() and self.client and self.mining:
            self.root.after(30000, self.refresh_transactions)  # Every 30 seconds
    
    def start_mining(self):
        if not self.wallet_address.get():
            self.log("❌ Enter GXC wallet address first", "ERROR")
            messagebox.showerror("Error", "Enter your GXC wallet address first!")
            return
        
        if not self.node_connected:
            self.log("❌ Blockchain node not connected", "ERROR")
            messagebox.showerror("Error", "Blockchain node not connected!\n\nStart the blockchain node first.")
            return
            
        if not self.client:
            self.log("❌ Blockchain client not initialized", "ERROR")
            return
        
        if not self.update_work():
            self.log("❌ Failed to get work from blockchain", "ERROR")
            messagebox.showerror("Error", "Failed to get work from blockchain!")
            return
        
        self.mining = True
        self.start_btn.config(state="disabled")
        self.stop_btn.config(state="normal")
        self.log("⚡ Mining started!", "SUCCESS")
        self.log(f"🧵 Using {self.threads.get()} threads", "INFO")
        self.log(f"📧 Mining to: {self.wallet_address.get()[:20]}...", "INFO")
        
        # Initial balance and transaction refresh
        self.refresh_transactions()
        
        # Start mining threads
        self.miner_threads = []
        for i in range(self.threads.get()):
            thread = threading.Thread(target=self.mine_thread, args=(i,), daemon=True)
            thread.start()
            self.miner_threads.append(thread)
        
        # Monitor and update work periodically
        self.monitor_work()
        
    def monitor_work(self):
        """Monitor and update work periodically"""
        if self.mining:
            self.update_work()
            self.root.after(30000, self.monitor_work)
        
    def stop_mining(self):
        self.mining = False
        self.start_btn.config(state="normal")
        self.stop_btn.config(state="disabled")
        self.log("⏸️ Mining stopped", "WARNING")
        self.log(f"📊 {self.total_hashes:,} hashes, {self.blocks_found} blocks", "INFO")
        self.log(f"📊 Accepted: {self.shares_accepted}, Rejected: {self.shares_rejected}", "INFO")
        # Refresh from blockchain to get real earnings
        if self.wallet_address.get():
            self.refresh_transactions()


def main():
    """Main entry point for the miner"""
    root = tk.Tk()
    app = GXCMiner(root)
    root.mainloop()


if __name__ == "__main__":
    main()