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

class BlockchainClient:
    """Client for connecting to GXC blockchain node"""
    
    def __init__(self, rpc_url: str = None, rest_url: str = None):
        # Use Railway URL from environment, fallback to localhost for local development
        RAILWAY_NODE_URL = "https://gxc-chain112-blockchain-node-production.up.railway.app"
        self.rpc_url = rpc_url or os.environ.get('BLOCKCHAIN_RPC_URL', os.environ.get('RAILWAY_NODE_URL', RAILWAY_NODE_URL))
        self.rest_url = rest_url or os.environ.get('BLOCKCHAIN_REST_URL', self.rpc_url)
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
            response = self.session.post(self.rpc_url, json=payload)
            response.raise_for_status()
            result = response.json()
            
            if 'error' in result:
                return None
                
            return result.get('result')
        except Exception as e:
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
        result = self.rpc_call("gxc_getBlockTemplate", [{"algorithm": algorithm}])
        if result:
            return result
        return self.rpc_call("getblocktemplate")
    
    def submit_block(self, block_data: Dict) -> Optional[str]:
        """Submit mined block to blockchain, returns transaction hash"""
        result = self.rpc_call("submitblock", [json.dumps(block_data)])
        if result is None:
            result = self.rpc_call("gxc_submitBlock", [block_data])
        
        if result is not False:
            # Return block hash as transaction ID
            return block_data.get('hash') or block_data.get('blockHash')
        return None
    
    def get_difficulty(self) -> Optional[float]:
        """Get current mining difficulty"""
        info = self.get_mining_info()
        if info:
            return info.get('difficulty') or info.get('currentDifficulty')
        return None
    
    def get_address_balance(self, address: str) -> float:
        """Get balance for an address"""
        try:
            result = self.rest_call(f"/api/v1/address/{address}/balance")
            if result:
                return float(result.get('balance', 0.0))
        except:
            pass
        return 0.0
    
    def get_address_transactions(self, address: str, limit: int = 50) -> List[Dict]:
        """Get transactions for an address"""
        try:
            result = self.rest_call(f"/api/v1/address/{address}/transactions?limit={limit}")
            if isinstance(result, list):
                return result
            elif isinstance(result, dict) and 'transactions' in result:
                return result['transactions']
        except:
            pass
        return []


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
        self.total_earned = 0.0
        self.current_balance = 0.0
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
        
        tk.Entry(wallet_frame, textvariable=self.wallet_address, font=("Arial", 9), 
                bg="#334155", fg="#ffffff", relief="flat", 
                insertbackground="#ffffff").pack(fill="x", padx=8, pady=(0,4), ipady=4)
        
        tk.Label(wallet_frame, text="💰 Mining rewards are automatically credited to this address", 
                font=("Arial", 7), bg="#1e293b", fg="#94a3b8").pack(anchor="w", padx=8, pady=(0,8))
        
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
        columns = ("Type", "Amount", "Status", "Time", "Hash")
        self.tx_tree = ttk.Treeview(list_frame, columns=columns, show="headings", height=15)
        
        # Configure columns
        self.tx_tree.heading("Type", text="Type")
        self.tx_tree.heading("Amount", text="Amount (GXC)")
        self.tx_tree.heading("Status", text="Status")
        self.tx_tree.heading("Time", text="Time")
        self.tx_tree.heading("Hash", text="Transaction Hash")
        
        self.tx_tree.column("Type", width=120)
        self.tx_tree.column("Amount", width=120)
        self.tx_tree.column("Status", width=100)
        self.tx_tree.column("Time", width=150)
        self.tx_tree.column("Hash", width=200)
        
        # Scrollbar
        scrollbar = ttk.Scrollbar(list_frame, orient="vertical", command=self.tx_tree.yview)
        self.tx_tree.configure(yscrollcommand=scrollbar.set)
        
        self.tx_tree.pack(side="left", fill="both", expand=True)
        scrollbar.pack(side="right", fill="y")
        
        # Bind double-click to open in explorer
        self.tx_tree.bind("<Double-1>", self.open_transaction_in_explorer)
        
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
    
    def refresh_transactions(self):
        """Refresh transactions from blockchain"""
        if not self.wallet_address.get() or not self.client:
            return
        
        address = self.wallet_address.get().strip()
        if not address:
            return
        
        self.log("🔄 Refreshing transactions...", "INFO")
        
        def fetch():
            try:
                transactions = self.client.get_address_transactions(address, limit=100)
                balance = self.client.get_address_balance(address)
                
                self.root.after(0, lambda: self.update_transactions_display(transactions))
                self.root.after(0, lambda: self.update_balance_display(balance))
            except Exception as e:
                self.root.after(0, lambda: self.log(f"❌ Failed to fetch transactions: {e}", "ERROR"))
        
        threading.Thread(target=fetch, daemon=True).start()
    
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
            
            # Insert into treeview
            item = self.tx_tree.insert("", "end", values=(
                type_display,
                amount_str,
                status,
                time_str,
                tx_hash
            ), tags=(tx.get('hash', ''),))
            
            # Color code by type
            if tx_type == 'coinbase':
                self.tx_tree.set(item, "Type", "💰 Mining Reward")
    
    def update_balance_display(self, balance: float):
        """Update balance displays"""
        self.current_balance = balance
        balance_str = f"{balance:.8f} GXC"
        self.balance_label.config(text=balance_str)
        if hasattr(self, 'current_balance_stats_label'):
            self.current_balance_stats_label.config(text=balance_str)
    
    def open_transaction_in_explorer(self, event):
        """Open transaction in blockchain explorer"""
        selection = self.tx_tree.selection()
        if selection:
            item = self.tx_tree.item(selection[0])
            tags = item.get('tags', [])
            if tags and tags[0]:
                tx_hash = tags[0]
                url = f"{self.explorer_url}/transactions?hash={tx_hash}"
                webbrowser.open(url)
                self.log(f"🌐 Opened transaction in explorer: {tx_hash[:16]}...", "INFO")
    
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
            'coinbase_value': template.get('coinbasevalue') or template.get('coinbaseValue', 12.5)
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
                reward_amount = work.get('coinbase_value', 12.5)
                
                self.root.after(0, lambda: self.log(f"🎉 Block found by thread {thread_id}!", "SUCCESS"))
                self.root.after(0, lambda: self.log(f"   Hash: {hash_result[:32]}...", "INFO"))
                self.root.after(0, lambda: self.log(f"   Reward: {reward_amount} GXC", "SUCCESS"))
                
                block = {
                    'version': work['version'],
                    'previousblockhash': work['prev_hash'],
                    'merkleroot': work['merkle_root'],
                    'timestamp': work['timestamp'],
                    'bits': work['bits'],
                    'nonce': nonce - 1,
                    'hash': hash_result,
                    'height': work['height'],
                    'transactions': work.get('transactions', [])
                }
                
                if self.wallet_address.get():
                    coinbase = {
                        'type': 'coinbase',
                        'hash': hash_result,  # Use block hash as transaction ID
                        'inputs': [],
                        'outputs': [{
                            'address': self.wallet_address.get(),
                            'amount': reward_amount
                        }],
                        'timestamp': int(time.time())
                    }
                    block['transactions'].insert(0, coinbase)
                
                tx_hash = self.client.submit_block(block)
                
                if tx_hash:
                    self.blocks_found += 1
                    self.shares_accepted += 1
                    self.total_earned += reward_amount
                    
                    self.root.after(0, lambda: self.log("✅ Block submitted and accepted!", "SUCCESS"))
                    self.root.after(0, lambda: self.log(f"💰 +{reward_amount} GXC credited to your wallet", "SUCCESS"))
                    
                    # Show explorer link
                    explorer_url = f"{self.explorer_url}/transactions?hash={tx_hash}"
                    self.root.after(0, lambda: self.show_explorer_link(tx_hash, explorer_url, reward_amount))
                    
                    # Refresh transactions after a delay
                    self.root.after(3000, self.refresh_transactions)
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
        """Show dialog with explorer link"""
        msg = f"✅ Block mined successfully!\n\n"
        msg += f"💰 Reward: {amount} GXC\n"
        msg += f"🔐 Transaction: {tx_hash[:16]}...\n\n"
        msg += f"View on blockchain explorer?"
        
        if messagebox.askyesno("Block Mined!", msg):
            webbrowser.open(url)
            self.log(f"🌐 Opened transaction in explorer", "INFO")
    
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
        self.earned_label.config(text=f"{self.total_earned:.8f} GXC")
        
        # Update stats tab
        try:
            self.total_hash_label.config(text=f"{self.total_hashes:,}")
            self.total_blocks_label.config(text=f"{self.blocks_found}")
            self.total_accepted_label.config(text=f"{self.shares_accepted}")
            self.total_rejected_label.config(text=f"{self.shares_rejected}")
            self.total_earned_stats_label.config(text=f"{self.total_earned:.8f} GXC")
            self.balance_label.config(text=f"{self.current_balance:.8f} GXC")
            if hasattr(self, 'current_balance_stats_label'):
                self.current_balance_stats_label.config(text=f"{self.current_balance:.8f} GXC")
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
        self.log(f"💰 Total Earned: {self.total_earned:.8f} GXC", "INFO")


def main():
    """Main entry point for the miner"""
    root = tk.Tk()
    app = GXCMiner(root)
    root.mainloop()


if __name__ == "__main__":
    main()