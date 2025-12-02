#!/usr/bin/env python3
"""
GXC Testnet Miner with GUI - Real-time mining with visual feedback
Complete implementation with real-time confirmation tracking
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

# Network Configuration
RAILWAY_NODE_URL = "https://gxc-chain112-blockchain-node-production.up.railway.app"
LOCAL_NODE_URL = "http://localhost:8545"

# Network settings
NETWORK_CONFIG = {
    'testnet': {
        'name': 'testnet',
        'address_prefix': 'tGXC',
        'block_reward': 50.0,
        'block_time': 60,
        'difficulty': 0.1
    },
    'mainnet': {
        'name': 'mainnet',
        'address_prefix': 'GXC',
        'block_reward': 50.0,
        'block_time': 600,
        'difficulty': 1.0
    }
}

# Current network
NETWORK = os.environ.get('GXC_NETWORK', 'testnet')
CURRENT_NETWORK = NETWORK_CONFIG.get(NETWORK, NETWORK_CONFIG['testnet'])
RPC_URL = os.environ.get('BLOCKCHAIN_RPC_URL', RAILWAY_NODE_URL)
MINER_ADDRESS = os.environ.get('GXC_MINER_ADDRESS', '')
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
        self.shares_accepted = 0
        self.shares_rejected = 0
        self.total_earned = 0.0
        self.last_balance_check = 0.0
        self.start_time = None
        self.shutdown = False
        self.pending_transactions = {}
        self.confirmation_threads = []

        # Setup signal handlers
        signal.signal(signal.SIGINT, self._signal_handler)
        signal.signal(signal.SIGTERM, self._signal_handler)

        self.setup_ui()
        self.log(f"GXC {NETWORK.upper()} Miner initialized", "INFO")
        self.log(f"Network: {NETWORK.upper()}", "INFO")
        self.log(f"Address prefix: {CURRENT_NETWORK['address_prefix']}", "INFO")
        self.log(f"Block reward: Will fetch from blockchain...", "INFO")

        # Auto-connect to blockchain in background
        self.root.after(2000, self.auto_connect)

        # Start stats update timer
        self.root.after(1000, self.update_stats_timer)

        # Auto-refresh balance periodically
        self.root.after(10000, self.auto_refresh_balance)

        # Start confirmation tracker
        self.root.after(5000, self.update_pending_confirmations)

    def _signal_handler(self, signum, frame):
        """Handle shutdown signals"""
        self.shutdown = True
        self.mining = False
        self.root.quit()

    def auto_connect(self):
        """Auto-connect to blockchain node in background"""
        self.log("Connecting to blockchain node...", "INFO")
        self.status_label.config(text="Connecting...", fg="#fbbf24")

        def connect():
            global RPC_URL, ACTIVE_NODE_TYPE

            # Try LOCAL first
            local_urls = [
                LOCAL_NODE_URL,
                LOCAL_NODE_URL.replace('localhost', '127.0.0.1'),
                'http://localhost:8080',
                'http://127.0.0.1:8080',
                'http://localhost:18332',
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
                        block_reward = self._get_block_reward(url)
                        self.node_connected = True
                        RPC_URL = url
                        ACTIVE_NODE_TYPE = "local"
                        self.rpc_url = url
                        self.root.after(0, lambda h=height: self.log(f"Connected to local node at height {h}", "SUCCESS"))
                        self.root.after(0, lambda r=block_reward: self.log(f"Block reward: {r} GXC", "INFO"))
                        self.root.after(0, lambda: self.status_label.config(text="Connected (Local)", fg="#86efac"))
                        if self.wallet_address.get():
                            self.root.after(1000, self.refresh_balance_from_blockchain)
                        return
                except requests.exceptions.ConnectionError:
                    continue
                except Exception:
                    continue

            # Try Railway as fallback
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
                    block_reward = self._get_block_reward(RAILWAY_NODE_URL)
                    self.node_connected = True
                    RPC_URL = RAILWAY_NODE_URL
                    ACTIVE_NODE_TYPE = "railway"
                    self.rpc_url = RAILWAY_NODE_URL
                    self.root.after(0, lambda h=height: self.log(f"Connected to Railway node at height {h}", "SUCCESS"))
                    self.root.after(0, lambda r=block_reward: self.log(f"Block reward: {r} GXC", "INFO"))
                    self.root.after(0, lambda: self.status_label.config(text="Connected (Railway)", fg="#86efac"))
                    if self.wallet_address.get():
                        self.root.after(1000, self.refresh_balance_from_blockchain)
                    return
            except Exception:
                pass

            self.node_connected = False
            self.root.after(0, lambda: self.log("Could not connect. Make sure node is running.", "WARNING"))
            self.root.after(0, lambda: self.status_label.config(text="Disconnected", fg="#fca5a5"))

        threading.Thread(target=connect, daemon=True).start()

    def _get_block_reward(self, url: str) -> float:
        """Get block reward from blockchain"""
        try:
            response = requests.post(url, json={
                "jsonrpc": "2.0",
                "method": "getblockchaininfo",
                "params": [],
                "id": 1
            }, timeout=5)
            result = response.json().get('result', {})
            return result.get('block_reward') or result.get('reward', CURRENT_NETWORK['block_reward'])
        except:
            return CURRENT_NETWORK['block_reward']

    def setup_ui(self):
        """Setup the user interface"""
        # Header with status
        header = tk.Frame(self.root, bg="#7c3aed", height=50)
        header.pack(fill="x")
        header.pack_propagate(False)

        title_text = f"GXC {NETWORK.upper()} MINER"
        tk.Label(header, text=title_text,
                 font=("Arial", 16, "bold"), bg="#7c3aed", fg="#ffffff").pack(side="left", padx=15, pady=10)

        self.status_label = tk.Label(header, text="Connecting...",
                                     font=("Arial", 9, "bold"),
                                     bg="#7c3aed", fg="#fbbf24")
        self.status_label.pack(side="right", padx=15)

        stats = tk.Frame(self.root, bg="#0f172a")
        stats.pack(fill="x", padx=8, pady=8)

        self.create_mini_stat(stats, "Hash Rate", "0 H/s", "#3b82f6", 0)
        self.create_mini_stat(stats, "Balance", "0.000000 GXC", "#10b981", 1)
        self.create_mini_stat(stats, "Blocks", "0", "#8b5cf6", 2)
        self.create_mini_stat(stats, "Accepted", "0", "#22c55e", 3)
        self.create_mini_stat(stats, "Rejected", "0", "#ef4444", 4)
        self.create_mini_stat(stats, "Earned", "0.000000 GXC", "#f59e0b", 5)

        # Main container with notebook tabs
        notebook = ttk.Notebook(self.root)
        notebook.pack(fill="both", expand=True, padx=8, pady=(0, 8))

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
        notebook.add(mining_tab, text="Mining")
        self.setup_mining_tab(mining_tab)

        # Tab 2: Transactions
        transactions_tab = tk.Frame(notebook, bg="#1e293b")
        notebook.add(transactions_tab, text="Transactions")
        self.setup_transactions_tab(transactions_tab)

        # Tab 3: Activity Log
        log_tab = tk.Frame(notebook, bg="#1e293b")
        notebook.add(log_tab, text="Activity")

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
        notebook.add(stats_tab, text="Stats")
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
                 bg="#1e293b", fg="#cbd5e1").pack(anchor="w", padx=8, pady=(8, 2))

        entry_frame = tk.Frame(wallet_frame, bg="#1e293b")
        entry_frame.pack(fill="x", padx=8, pady=(0, 4))

        tk.Entry(entry_frame, textvariable=self.wallet_address, font=("Arial", 9),
                 bg="#334155", fg="#ffffff", relief="flat",
                 insertbackground="#ffffff").pack(side="left", fill="x", expand=True, ipady=4)

        refresh_btn = tk.Button(entry_frame, text="Refresh", command=self.refresh_balance_from_blockchain,
                                font=("Arial", 8), bg="#3b82f6", fg="#ffffff",
                                relief="flat", cursor="hand2", width=8)
        refresh_btn.pack(side="right", padx=(4, 0))

        # Balance display
        self.wallet_balance_label = tk.Label(wallet_frame, text="Balance: Loading...",
                                             font=("Arial", 8, "bold"),
                                             bg="#1e293b", fg="#10b981")
        self.wallet_balance_label.pack(anchor="w", padx=8, pady=(4, 0))

        tk.Label(wallet_frame, text=f"Mining rewards: {CURRENT_NETWORK['block_reward']} GXC per block",
                 font=("Arial", 7), bg="#1e293b", fg="#94a3b8").pack(anchor="w", padx=8, pady=(4, 8))

        # Control buttons
        btn_frame = tk.Frame(parent, bg="#1e293b")
        btn_frame.pack(fill="x", padx=8, pady=(0, 8))

        self.start_btn = tk.Button(btn_frame, text="START MINING", command=self.start_mining,
                                   font=("Arial", 10, "bold"), bg="#10b981", fg="#ffffff",
                                   relief="flat", cursor="hand2")
        self.start_btn.pack(side="left", fill="x", expand=True, padx=(0, 4), ipady=8)

        self.stop_btn = tk.Button(btn_frame, text="STOP", command=self.stop_mining,
                                  font=("Arial", 10, "bold"), bg="#ef4444", fg="#ffffff",
                                  relief="flat", state="disabled", cursor="hand2")
        self.stop_btn.pack(side="left", fill="x", expand=True, padx=4, ipady=8)

        reconnect_btn = tk.Button(btn_frame, text="Reconnect", command=self.auto_connect,
                                  font=("Arial", 9, "bold"), bg="#3b82f6", fg="#ffffff",
                                  relief="flat", cursor="hand2")
        reconnect_btn.pack(side="left", fill="x", expand=True, padx=(4, 0), ipady=8)

        # Network info
        info_frame = tk.LabelFrame(parent, text="Network Info", font=("Arial", 9, "bold"),
                                   bg="#1e293b", fg="#e2e8f0", relief="flat")
        info_frame.pack(fill="x", padx=8, pady=(0, 8))

        info_text = f"Network: {NETWORK.upper()} | Prefix: {CURRENT_NETWORK['address_prefix']} | Block Time: {CURRENT_NETWORK['block_time']}s"
        tk.Label(info_frame, text=info_text, font=("Arial", 8),
                 bg="#1e293b", fg="#cbd5e1").pack(padx=8, pady=8)

        # Pending Confirmations Section
        pending_frame = tk.LabelFrame(parent, text="Pending Confirmations", font=("Arial", 9, "bold"),
                                      bg="#1e293b", fg="#e2e8f0", relief="flat")
        pending_frame.pack(fill="x", padx=8, pady=(0, 8))

        self.pending_label = tk.Label(pending_frame, text="No pending transactions",
                                      font=("Arial", 8), bg="#1e293b", fg="#94a3b8")
        self.pending_label.pack(padx=8, pady=8)

    def setup_transactions_tab(self, parent):
        """Setup transactions display tab"""
        # Header with refresh button
        header_frame = tk.Frame(parent, bg="#1e293b")
        header_frame.pack(fill="x", padx=8, pady=8)

        tk.Label(header_frame, text="Mining Rewards & Transactions",
                 font=("Arial", 10, "bold"), bg="#1e293b", fg="#e2e8f0").pack(side="left")

        tk.Button(header_frame, text="Refresh", command=self.refresh_balance_from_blockchain,
                  font=("Arial", 8), bg="#3b82f6", fg="#ffffff",
                  relief="flat", cursor="hand2").pack(side="right", padx=4)

        # Transactions list with scrollbar
        list_frame = tk.Frame(parent, bg="#1e293b")
        list_frame.pack(fill="both", expand=True, padx=8, pady=(0, 8))

        # Create treeview for transactions
        columns = ("Type", "Amount", "Status", "Confirmations", "Time", "Hash")
        self.tx_tree = ttk.Treeview(list_frame, columns=columns, show="headings", height=15)

        # Configure columns
        self.tx_tree.heading("Type", text="Type")
        self.tx_tree.heading("Amount", text="Amount (GXC)")
        self.tx_tree.heading("Status", text="Status")
        self.tx_tree.heading("Confirmations", text="Confirmations")
        self.tx_tree.heading("Time", text="Time")
        self.tx_tree.heading("Hash", text="Transaction Hash")

        self.tx_tree.column("Type", width=120)
        self.tx_tree.column("Amount", width=100)
        self.tx_tree.column("Status", width=100)
        self.tx_tree.column("Confirmations", width=100)
        self.tx_tree.column("Time", width=150)
        self.tx_tree.column("Hash", width=200)

        # Scrollbar
        scrollbar = ttk.Scrollbar(list_frame, orient="vertical", command=self.tx_tree.yview)
        self.tx_tree.configure(yscrollcommand=scrollbar.set)

        self.tx_tree.pack(side="left", fill="both", expand=True)
        scrollbar.pack(side="right", fill="y")

        # Bind double-click to open in explorer
        self.tx_tree.bind("<Double-1>", self.open_transaction_in_explorer)
        self.tx_tree.bind("<Button-3>", self.show_explorer_context_menu)

        # Style treeview
        style = ttk.Style()
        style.configure("Treeview", background="#0f172a", foreground="#e2e8f0",
                        fieldbackground="#0f172a", rowheight=25)
        style.configure("Treeview.Heading", background="#334155", foreground="#ffffff")
        style.map("Treeview", background=[("selected", "#7c3aed")])

    def create_stat_display(self, parent):
        """Create detailed stats display"""
        stats_container = tk.Frame(parent, bg="#1e293b")
        stats_container.pack(fill="both", expand=True, padx=8, pady=8)

        stats = [
            ("Total Hashes", "total_hashes_label", "0"),
            ("Blocks Found", "blocks_found_label", "0"),
            ("Blocks Attempted", "blocks_attempted_label", "0"),
            ("Success Rate", "success_rate_label", "0%"),
            ("Shares Accepted", "shares_accepted_label", "0"),
            ("Shares Rejected", "shares_rejected_label", "0"),
            ("Total Earned", "total_earned_label", "0.000000 GXC"),
            ("Current Balance", "current_balance_label", "0.000000 GXC"),
            ("Mining Time", "mining_time_label", "00:00:00"),
            ("Pending Confirms", "pending_confirms_label", "0"),
        ]

        row = 0
        col = 0
        for label, attr, default in stats:
            frame = tk.LabelFrame(stats_container, text=label, font=("Arial", 8, "bold"),
                                  bg="#1e293b", fg="#cbd5e1", relief="flat")
            frame.grid(row=row, column=col, padx=8, pady=8, sticky="ew")

            if "Accepted" in label:
                fg_color = "#22c55e"
            elif "Rejected" in label:
                fg_color = "#ef4444"
            else:
                fg_color = "#10b981"

            label_obj = tk.Label(frame, text=default, font=("Arial", 14, "bold"),
                                 bg="#1e293b", fg=fg_color)
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
        try:
            timestamp = datetime.now().strftime("%H:%M:%S")
            log_entry = f"[{timestamp}] {message}\n"
            self.log_text.insert("end", log_entry, level)
            self.log_text.see("end")
        except Exception as e:
            print(f"[LOG ERROR] {e}: {message}")

    def rpc_call(self, method, params=None, show_errors=True):
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
            timeout = 10 if ACTIVE_NODE_TYPE == "railway" else 5
            response = requests.post(self.rpc_url, json=payload, timeout=timeout)
            response.raise_for_status()
            result = response.json()

            if 'error' in result:
                if show_errors:
                    self.log(f"RPC Error: {result['error']}", "ERROR")
                return None

            return result.get('result')
        except Exception as e:
            if show_errors:
                self.log(f"RPC Error: {str(e)[:60]}", "ERROR")
            return None

    def get_current_height(self) -> int:
        """Get current blockchain height"""
        result = self.rpc_call("getblockcount", [], show_errors=False)
        return result if result else 0

    def add_pending_transaction(self, tx_hash: str, block_height: int, amount: float):
        """Add a transaction to pending confirmations tracker"""
        self.pending_transactions[tx_hash] = {
            'hash': tx_hash,
            'block_height': block_height,
            'amount': amount,
            'confirmations': 0,
            'timestamp': time.time(),
            'status': 'pending'
        }
        self.update_pending_display()

    def update_pending_confirmations(self):
        """Update confirmations for pending transactions in real-time"""
        if not self.node_connected or not self.pending_transactions:
            self.root.after(5000, self.update_pending_confirmations)
            return

        def update():
            try:
                current_height = self.get_current_height()
                if current_height == 0:
                    return

                updated = False
                to_remove = []

                for tx_hash, tx_info in list(self.pending_transactions.items()):
                    block_height = tx_info['block_height']
                    new_confirmations = max(0, current_height - block_height + 1)
                    old_confirmations = tx_info['confirmations']

                    if new_confirmations != old_confirmations:
                        tx_info['confirmations'] = new_confirmations
                        updated = True

                        # Log confirmation updates
                        if new_confirmations == 1:
                            self.root.after(0, lambda h=tx_hash[:16], c=new_confirmations:
                                            self.log(f"TX {h}... - 1 confirmation", "INFO"))
                        elif new_confirmations == 6:
                            self.root.after(0, lambda h=tx_hash[:16], a=tx_info['amount']:
                                            self.log(f"TX {h}... CONFIRMED! +{a} GXC", "SUCCESS"))
                            tx_info['status'] = 'confirmed'
                            to_remove.append(tx_hash)
                        elif new_confirmations < 6:
                            self.root.after(0, lambda h=tx_hash[:16], c=new_confirmations:
                                            self.log(f"TX {h}... - {c}/6 confirmations", "INFO"))

                # Remove fully confirmed transactions
                for tx_hash in to_remove:
                    del self.pending_transactions[tx_hash]

                if updated:
                    self.root.after(0, self.update_pending_display)
                    self.root.after(0, self.update_transactions_display_confirmations)

            except Exception as e:
                self.root.after(0, lambda err=str(e): self.log(f"Confirmation update error: {err}", "WARNING"))

        threading.Thread(target=update, daemon=True).start()
        self.root.after(5000, self.update_pending_confirmations)

    def update_pending_display(self):
        """Update the pending confirmations display"""
        if not self.pending_transactions:
            self.pending_label.config(text="No pending transactions")
            if hasattr(self, 'pending_confirms_label'):
                self.pending_confirms_label.config(text="0")
            return

        pending_text = []
        for tx_hash, tx_info in self.pending_transactions.items():
            confirmations = tx_info['confirmations']
            amount = tx_info['amount']
            pending_text.append(f"{tx_hash[:12]}... | {amount} GXC | {confirmations}/6 confirmations")

        self.pending_label.config(text="\n".join(pending_text))
        if hasattr(self, 'pending_confirms_label'):
            self.pending_confirms_label.config(text=str(len(self.pending_transactions)))

    def update_transactions_display_confirmations(self):
        """Update confirmation counts in the transactions treeview"""
        for item in self.tx_tree.get_children():
            values = list(self.tx_tree.item(item, 'values'))
            tags = self.tx_tree.item(item, 'tags')

            if tags and len(tags) > 0:
                tx_hash = tags[0]
                if tx_hash in self.pending_transactions:
                    tx_info = self.pending_transactions[tx_hash]
                    confirmations = tx_info['confirmations']

                    # Update status and confirmations columns
                    if confirmations >= 6:
                        values[2] = "Confirmed"
                        values[3] = f"{confirmations}"
                    else:
                        values[2] = "Pending"
                        values[3] = f"{confirmations}/6"

                    self.tx_tree.item(item, values=tuple(values))

    def mine_block(self):
        """Mine a single block"""
        address = self.wallet_address.get()
        if not address:
            return False

        self.log(f"Mining block to: {address[:20]}...", "INFO")

        # Get block template
        template = self.rpc_call("getblocktemplate", [{"algorithm": "sha256"}], show_errors=False)

        if not template:
            self.log("Failed to get block template", "ERROR")
            return False

        height = template.get('height', 1)
        prev_hash = template.get('previousblockhash', '0' * 64)

        self.log(f"Height: {height}, Previous: {prev_hash[:16]}...", "INFO")

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
                # Get block reward from template
                block_reward = template.get('block_reward') or template.get('reward') or template.get('coinbase_value')

                if not block_reward and template.get('coinbasevalue'):
                    block_reward = template.get('coinbasevalue') / 100000000.0

                if not block_reward:
                    try:
                        blockchain_info = self.rpc_call("getblockchaininfo", [], show_errors=False)
                        if blockchain_info:
                            block_reward = blockchain_info.get('block_reward') or blockchain_info.get('reward') or CURRENT_NETWORK.get('block_reward', 50.0)
                    except:
                        block_reward = CURRENT_NETWORK.get('block_reward', 50.0)

                # Create coinbase transaction
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

                # Prepare block data
                block_data = {
                    'hash': block_hash,
                    'previousblockhash': prev_hash,
                    'height': height,
                    'nonce': nonce,
                    'miner': address,
                    'miner_address': address,
                    'timestamp': int(time.time()),
                    'difficulty': template.get('difficulty', 0.1),
                    'transactions': [coinbase_tx],
                    'transaction_count': 1
                }

                self.log(f"Found valid hash: {block_hash[:16]}...", "SUCCESS")
                self.log(f"Submitting block...", "INFO")

                result = self.rpc_call("submitblock", [block_data], show_errors=False)
                elapsed = time.time() - start_time

                # Handle different result types
                success = False
                if result is None:
                    success = True
                elif isinstance(result, dict):
                    if result.get('success') or result.get('accepted'):
                        success = True
                    elif result.get('error'):
                        self.log(f"Block submission error: {result.get('error')}", "ERROR")
                        self.shares_rejected += 1
                        return False
                elif isinstance(result, str):
                    if 'rejected' in result.lower() or 'error' in result.lower():
                        self.log(f"Block rejected: {result}", "ERROR")
                        self.shares_rejected += 1
                        return False
                    success = True
                elif result is False:
                    self.log(f"Block submission returned False", "ERROR")
                    self.shares_rejected += 1
                    return False
                else:
                    success = True

                if success:
                    self.blocks_found += 1
                    self.shares_accepted += 1
                    tx_hash = coinbase_tx['hash']

                    self.log(f"BLOCK MINED! Height: {height}", "SUCCESS")
                    self.log(f"Expected reward: {block_reward} GXC", "SUCCESS")
                    self.log(f"Time: {elapsed:.2f} seconds", "INFO")

                    # Add to pending confirmations for real-time tracking
                    self.add_pending_transaction(tx_hash, height, block_reward)

                    # Add to transaction list
                    self.add_transaction_to_list({
                        'hash': tx_hash,
                        'type': 'coinbase',
                        'amount': block_reward,
                        'confirmations': 0,
                        'timestamp': int(time.time()),
                        'block_number': height
                    })

                    # Refresh balance after delays
                    self.root.after(5000, self.refresh_balance_from_blockchain)
                    self.root.after(15000, self.refresh_balance_from_blockchain)
                    self.root.after(30000, self.refresh_balance_from_blockchain)

                    return True
                else:
                    self.shares_rejected += 1
                    self.log(f"Block rejected: {result}", "ERROR")
                    return False

            nonce += 1

            # Timeout after 60 seconds
            if time.time() - start_time > 60:
                self.log(f"Timeout after 60 seconds", "WARNING")
                return False

        return False

    def add_transaction_to_list(self, tx: Dict):
        """Add a single transaction to the transactions list"""
        tx_type = tx.get('type', 'unknown')
        tx_hash = tx.get('hash', '')
        short_hash = tx_hash[:16] + '...' if tx_hash else 'N/A'

        # Determine transaction type display
        if tx_type == 'coinbase':
            type_display = "Mining Reward"
            amount = tx.get('amount', 0)
        else:
            type_display = "Received"
            amount = tx.get('amount', 0)

        # Format amount
        amount_str = f"{amount:.8f}"

        # Status and confirmations
        confirmations = tx.get('confirmations', 0)
        if confirmations >= 6:
            status = "Confirmed"
            confirm_str = f"{confirmations}"
        elif confirmations > 0:
            status = "Pending"
            confirm_str = f"{confirmations}/6"
        else:
            status = "Pending"
            confirm_str = "0/6"

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

        # Insert at the top (newest first)
        try:
            self.tx_tree.insert("", 0, values=(
                type_display,
                amount_str,
                status,
                confirm_str,
                time_str,
                short_hash
            ), tags=(tx_hash,))
        except Exception as e:
            print(f"[TX INSERT ERROR] {e}")

    def update_transactions_display(self, transactions: List[Dict]):
        """Update transactions treeview"""
        # Clear existing items
        for item in self.tx_tree.get_children():
            self.tx_tree.delete(item)

        # Sort by timestamp (newest first)
        sorted_txs = sorted(transactions, key=lambda x: x.get('timestamp', 0), reverse=True)

        for tx in sorted_txs:
            self.add_transaction_to_list(tx)

    def calculate_total_earned_from_blockchain(self, transactions: List[Dict]) -> float:
        """Calculate total earned from actual blockchain coinbase transactions"""
        total = 0.0
        for tx in transactions:
            if tx.get('type') == 'coinbase' or tx.get('is_coinbase') or tx.get('isCoinbase'):
                confirmations = tx.get('confirmations', 0)
                if confirmations >= 6:
                    amount = tx.get('amount') or tx.get('value') or 0.0
                    if amount > 0:
                        total += amount
        return total

    def update_balance_display(self, balance: float = None):
        """Update balance displays with REAL blockchain data"""
        if balance is None:
            balance = self.last_balance_check

        self.last_balance_check = balance
        balance_str = f"{balance:.6f} GXC"

        if hasattr(self, 'balance_label'):
            self.balance_label.config(text=balance_str)

        if hasattr(self, 'wallet_balance_label'):
            self.wallet_balance_label.config(text=f"Balance: {balance_str}")

        if hasattr(self, 'current_balance_label'):
            self.current_balance_label.config(text=balance_str)

    def update_earnings_display(self, total_earned: float):
        """Update earnings display with REAL blockchain data"""
        self.total_earned = total_earned
        earned_str = f"{total_earned:.6f} GXC"

        if hasattr(self, 'earned_label'):
            self.earned_label.config(text=earned_str)

        if hasattr(self, 'total_earned_label'):
            self.total_earned_label.config(text=earned_str)

    def get_address_transactions(self, address: str, limit: int = 100) -> List[Dict]:
        """Get transactions for an address from blockchain"""
        try:
            methods = [
                ("gxc_getTransactionsByAddress", [address, limit]),
                ("getaddresstransactions", [address, limit]),
                ("listtransactions", [address, limit]),
                ("gettransactions", [address, limit]),
            ]

            for method, params in methods:
                result = self.rpc_call(method, params, show_errors=False)
                if result:
                    if isinstance(result, list):
                        return result
                    elif isinstance(result, dict):
                        if 'transactions' in result:
                            return result['transactions']
            return []
        except Exception as e:
            self.log(f"Error getting transactions: {e}", "WARNING")
            return []

    def refresh_balance_from_blockchain(self):
        """Refresh balance and transactions from blockchain"""
        if not self.wallet_address.get() or not self.node_connected:
            return

        address = self.wallet_address.get().strip()
        if not address:
            return

        self.log("Refreshing balance from blockchain...", "INFO")

        def fetch():
            try:
                # Get balance
                balance = 0.0
                balance_methods = [
                    ("gxc_getBalance", [address]),
                    ("getbalance", [address]),
                    ("getaddressbalance", [address]),
                ]

                for method, params in balance_methods:
                    result = self.rpc_call(method, params, show_errors=False)
                    if result is not None:
                        if isinstance(result, (int, float)):
                            balance = float(result)
                            break
                        elif isinstance(result, dict):
                            balance = float(result.get('balance', result.get('amount', 0.0)))
                            if balance > 0:
                                break

                self.root.after(0, lambda b=balance: self.update_balance_display(b))
                self.root.after(0, lambda b=balance: self.log(f"Balance: {b:.8f} GXC", "INFO"))

                # Get transactions
                transactions = self.get_address_transactions(address, limit=100)

                # Calculate total earned
                total_earned = self.calculate_total_earned_from_blockchain(transactions)

                # Update earnings display
                self.root.after(0, lambda te=total_earned: self.update_earnings_display(te))
                self.root.after(0, lambda te=total_earned: self.log(f"Total earned: {te:.8f} GXC", "INFO"))

                if transactions:
                    self.root.after(0, lambda txs=transactions: self.update_transactions_display(txs))
                    self.root.after(0, lambda txs=transactions: self.log(f"Found {len(txs)} transactions", "INFO"))

            except Exception as e:
                self.root.after(0, lambda err=str(e): self.log(f"Failed to fetch from blockchain: {err}", "ERROR"))

        threading.Thread(target=fetch, daemon=True).start()

    def auto_refresh_balance(self):
        """Auto-refresh balance from blockchain every 30 seconds"""
        if self.wallet_address.get() and self.node_connected:
            self.refresh_balance_from_blockchain()
        self.root.after(30000, self.auto_refresh_balance)

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
        self.log("Mining started!", "SUCCESS")
        self.log(f"Mining to: {self.wallet_address.get()[:30]}...", "INFO")

        def mine_loop():
            while self.mining and not self.shutdown:
                self.blocks_attempted += 1
                self.log(f"Attempt #{self.blocks_attempted}", "INFO")

                if self.mine_block():
                    self.log(f"Total blocks mined: {self.blocks_found}", "SUCCESS")
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
        self.log("Mining stopped", "WARNING")

        success_rate = (self.blocks_found / self.blocks_attempted * 100) if self.blocks_attempted > 0 else 0
        self.log(f"Statistics:", "INFO")
        self.log(f"   Blocks attempted: {self.blocks_attempted}", "INFO")
        self.log(f"   Blocks mined: {self.blocks_found}", "INFO")
        self.log(f"   Shares accepted: {self.shares_accepted}", "INFO")
        self.log(f"   Shares rejected: {self.shares_rejected}", "INFO")
        self.log(f"   Success rate: {success_rate:.1f}%", "INFO")
        self.log(f"   Total hashes: {self.total_hashes:,}", "INFO")

        if self.wallet_address.get():
            self.refresh_balance_from_blockchain()

    def update_stats_timer(self):
        """Update stats periodically"""
        if self.mining:
            # Update hash rate display
            if self.hash_rate >= 1e6:
                hr = f"{self.hash_rate / 1e6:.2f} MH/s"
            elif self.hash_rate >= 1e3:
                hr = f"{self.hash_rate / 1e3:.2f} KH/s"
            else:
                hr = f"{self.hash_rate:.0f} H/s"

            self.hash_rate_label.config(text=hr)
            self.blocks_label.config(text=f"{self.blocks_found}")

            if hasattr(self, 'accepted_label'):
                self.accepted_label.config(text=f"{self.shares_accepted}")
            if hasattr(self, 'rejected_label'):
                self.rejected_label.config(text=f"{self.shares_rejected}")

            # Update detailed stats
            if hasattr(self, 'total_hashes_label'):
                self.total_hashes_label.config(text=f"{self.total_hashes:,}")
            if hasattr(self, 'blocks_found_label'):
                self.blocks_found_label.config(text=f"{self.blocks_found}")
            if hasattr(self, 'blocks_attempted_label'):
                self.blocks_attempted_label.config(text=f"{self.blocks_attempted}")
            if hasattr(self, 'shares_accepted_label'):
                self.shares_accepted_label.config(text=f"{self.shares_accepted}")
            if hasattr(self, 'shares_rejected_label'):
                self.shares_rejected_label.config(text=f"{self.shares_rejected}")

            success_rate = (self.blocks_found / self.blocks_attempted * 100) if self.blocks_attempted > 0 else 0
            if hasattr(self, 'success_rate_label'):
                self.success_rate_label.config(text=f"{success_rate:.1f}%")

            # Update mining time
            if self.start_time and hasattr(self, 'mining_time_label'):
                elapsed = time.time() - self.start_time
                hours = int(elapsed // 3600)
                minutes = int((elapsed % 3600) // 60)
                seconds = int(elapsed % 60)
                self.mining_time_label.config(text=f"{hours:02d}:{minutes:02d}:{seconds:02d}")

        self.root.after(1000, self.update_stats_timer)

    def open_transaction_in_explorer(self, event):
        """Open transaction in blockchain explorer"""
        selection = self.tx_tree.selection()
        if selection:
            item = self.tx_tree.item(selection[0])
            tags = item.get('tags', [])

            if tags and len(tags) > 0:
                tx_hash = tags[0]
                explorer_url = f"http://localhost:3000/tx/{tx_hash}"
                webbrowser.open(explorer_url)
                self.log(f"Opened transaction in explorer: {explorer_url}", "INFO")

    def show_explorer_context_menu(self, event):
        """Show context menu for explorer link"""
        selection = self.tx_tree.selection()
        if selection:
            item = self.tx_tree.item(selection[0])
            tags = item.get('tags', [])

            if tags and len(tags) > 0:
                tx_hash = tags[0]
                explorer_url = f"http://localhost:3000/tx/{tx_hash}"

                menu = tk.Menu(self.root, tearoff=0)
                menu.add_command(label="Open in Explorer", command=lambda: webbrowser.open(explorer_url))
                menu.add_command(label="Copy URL", command=lambda: self.copy_to_clipboard(explorer_url))
                menu.add_command(label="Copy Hash", command=lambda: self.copy_to_clipboard(tx_hash))
                try:
                    menu.tk_popup(event.x_root, event.y_root)
                finally:
                    menu.grab_release()

    def copy_to_clipboard(self, text):
        """Copy text to clipboard"""
        self.root.clipboard_clear()
        self.root.clipboard_append(text)


def main():
    """Main entry point"""
    root = tk.Tk()
    app = TestnetMinerGUI(root)
    root.mainloop()


if __name__ == "__main__":
    main()
