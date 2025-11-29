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
        self.total_earned = 0.0
        self.current_balance = 0.0
        self.start_time = None
        self.shutdown = False
        
        # Setup signal handlers
        signal.signal(signal.SIGINT, self._signal_handler)
        signal.signal(signal.SIGTERM, self._signal_handler)
        
        self.setup_ui()
        self.log(f"🚀 GXC {NETWORK.upper()} Miner initialized", "INFO")
        self.log(f"📍 Network: {NETWORK.upper()}", "INFO")
        self.log(f"📍 Address prefix: {CURRENT_NETWORK['address_prefix']}", "INFO")
        self.log(f"📍 Block reward: {CURRENT_NETWORK['block_reward']} GXC", "INFO")
        
        # Auto-connect to blockchain in background (wait 2 seconds for node to be ready)
        self.root.after(2000, self.auto_connect)
        
        # Start stats update timer
        self.root.after(1000, self.update_stats_timer)
    
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
                        self.node_connected = True
                        RPC_URL = url
                        ACTIVE_NODE_TYPE = "local"
                        self.root.after(0, lambda h=height: self.log(f"✅ Connected to local node", "SUCCESS"))
                        self.root.after(0, lambda h=height: self.log(f"📊 Current height: {h}", "INFO"))
                        self.root.after(0, lambda: self.status_label.config(text="● Connected (Local)", fg="#86efac"))
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
                    self.node_connected = True
                    RPC_URL = RAILWAY_NODE_URL
                    ACTIVE_NODE_TYPE = "railway"
                    self.root.after(0, lambda: self.log(f"✅ Connected to Railway node", "SUCCESS"))
                    self.root.after(0, lambda: self.log(f"📊 Current height: {height}", "INFO"))
                    self.root.after(0, lambda: self.status_label.config(text="● Connected (Railway)", fg="#86efac"))
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
        
        # Tab 2: Activity Log
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
        
        # Tab 3: Stats
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
        
        tk.Entry(wallet_frame, textvariable=self.wallet_address, font=("Arial", 9), 
                bg="#334155", fg="#ffffff", relief="flat", 
                insertbackground="#ffffff").pack(fill="x", padx=8, pady=(0,4), ipady=4)
        
        tk.Label(wallet_frame, text=f"💰 Mining rewards: {CURRENT_NETWORK['block_reward']} GXC per block", 
                font=("Arial", 7), bg="#1e293b", fg="#94a3b8").pack(anchor="w", padx=8, pady=(0,8))
        
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
                # Prepare block data
                block_data = {
                    'hash': block_hash,
                    'previousblockhash': prev_hash,
                    'height': height,
                    'nonce': nonce,
                    'miner': address,
                    'timestamp': int(time.time()),
                    'difficulty': template.get('difficulty', 0.1),
                    'transactions': []
                }
                
                self.log(f"✅ Found valid hash: {block_hash[:16]}...", "SUCCESS")
                self.log(f"📤 Submitting block...", "INFO")
                
                result = self.rpc_call("submitblock", [block_data], show_errors=False)
                elapsed = time.time() - start_time
                
                if result is None or (isinstance(result, dict) and result.get('success')):
                    reward = CURRENT_NETWORK['block_reward']
                    self.blocks_found += 1
                    self.total_earned += reward
                    self.current_balance += reward
                    
                    self.log(f"🎉 BLOCK MINED! Height: {height}", "SUCCESS")
                    self.log(f"💰 Reward: {reward} GXC", "SUCCESS")
                    self.log(f"⏱️  Time: {elapsed:.2f} seconds", "INFO")
                    
                    # Open explorer to view block (optional - user can disable)
                    explorer_url = f"http://localhost:3000/block/{height}"
                    self.root.after(2000, lambda url=explorer_url: webbrowser.open(url))
                    
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
        self.log(f"   Total earned: {self.total_earned:.8f} GXC", "INFO")
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
            self.balance_label.config(text=f"{self.current_balance:.8f} GXC")
            self.blocks_label.config(text=f"{self.blocks_found}")
            self.earned_label.config(text=f"{self.total_earned:.8f} GXC")
            
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
            
            if hasattr(self, 'total_earned_label'):
                self.total_earned_label.config(text=f"{self.total_earned:.8f} GXC")
            if hasattr(self, 'current_balance_label'):
                self.current_balance_label.config(text=f"{self.current_balance:.8f} GXC")
            
            # Update mining time
            if self.start_time and hasattr(self, 'mining_time_label'):
                elapsed = time.time() - self.start_time
                hours = int(elapsed // 3600)
                minutes = int((elapsed % 3600) // 60)
                seconds = int(elapsed % 60)
                self.mining_time_label.config(text=f"{hours:02d}:{minutes:02d}:{seconds:02d}")
        
        self.root.after(1000, self.update_stats_timer)


def main():
    """Main entry point"""
    root = tk.Tk()
    app = TestnetMinerGUI(root)
    root.mainloop()


if __name__ == "__main__":
    main()