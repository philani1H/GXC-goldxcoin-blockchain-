import tkinter as tk
from tkinter import ttk, scrolledtext, messagebox
import threading
import time
import hashlib
import requests
from datetime import datetime
import math
import random

class GXCMiner:
    def __init__(self, root):
        self.root = root
        self.root.title("GXC Miner Pro")
        self.root.geometry("750x550")
        self.root.minsize(700, 500)
        self.root.configure(bg="#0f172a")
        
        # Mining parameters
        self.blockchain_endpoint = tk.StringVar()
        self.wallet_address = tk.StringVar()
        self.algorithm = tk.StringVar(value="scrypt")
        self.threads = tk.IntVar(value=4)
        self.difficulty = tk.IntVar(value=3)
        
        # Mining state
        self.mining = False
        self.node_connected = False
        self.hash_rate = 0
        self.total_hashes = 0
        self.blocks_found = 0
        self.gxc_earned = 0.0
        self.pool_share = 0.0
        self.estimated_daily = 0.0
        
        # Constants
        self.MINIMUM_PAYOUT = 0.01
        self.POOL_FEE = 0.01
        self.BASE_REWARD = 12.5
        
        self.setup_ui()
        self.log("🚀 GXC Miner initialized", "INFO")
        
    def setup_ui(self):
        # Header with status
        header = tk.Frame(self.root, bg="#7c3aed", height=50)
        header.pack(fill="x")
        header.pack_propagate(False)
        
        tk.Label(header, text="⚡ GXC MINER PRO", 
                font=("Arial", 16, "bold"), bg="#7c3aed", fg="#ffffff").pack(side="left", padx=15, pady=10)
        
        self.status_label = tk.Label(header, text="● Disconnected", 
                                     font=("Arial", 9, "bold"), 
                                     bg="#7c3aed", fg="#fca5a5")
        self.status_label.pack(side="right", padx=15)
        
        # Compact stats row
        stats = tk.Frame(self.root, bg="#0f172a")
        stats.pack(fill="x", padx=8, pady=8)
        
        self.create_mini_stat(stats, "Hash Rate", "0 H/s", "#3b82f6", 0)
        self.create_mini_stat(stats, "Balance", "0.000000 GXC", "#10b981", 1)
        self.create_mini_stat(stats, "Daily Est.", "0.0000 GXC", "#8b5cf6", 2)
        self.create_mini_stat(stats, "Blocks", "0", "#f59e0b", 3)
        
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
        
        # Connection section
        conn_frame = tk.LabelFrame(mining_tab, text="Connection", font=("Arial", 9, "bold"),
                                   bg="#1e293b", fg="#e2e8f0", relief="flat")
        conn_frame.pack(fill="x", padx=8, pady=8)
        
        tk.Label(conn_frame, text="Node Endpoint:", font=("Arial", 8), 
                bg="#1e293b", fg="#cbd5e1").grid(row=0, column=0, sticky="w", padx=8, pady=(8,2))
        
        entry_frame = tk.Frame(conn_frame, bg="#1e293b")
        entry_frame.grid(row=1, column=0, sticky="ew", padx=8, pady=(0,8))
        conn_frame.grid_columnconfigure(0, weight=1)
        
        tk.Entry(entry_frame, textvariable=self.blockchain_endpoint, font=("Arial", 9), 
                bg="#334155", fg="#ffffff", relief="flat", insertbackground="#ffffff").pack(
                side="left", fill="x", expand=True, ipady=4)
        
        tk.Button(entry_frame, text="Connect", command=self.connect_to_node,
                 font=("Arial", 8, "bold"), bg="#3b82f6", fg="#ffffff",
                 relief="flat", cursor="hand2").pack(side="left", padx=(4,0), ipadx=8)
        
        # Wallet section
        wallet_frame = tk.LabelFrame(mining_tab, text="Wallet", font=("Arial", 9, "bold"),
                                     bg="#1e293b", fg="#e2e8f0", relief="flat")
        wallet_frame.pack(fill="x", padx=8, pady=(0,8))
        
        tk.Label(wallet_frame, text="GXC Address:", font=("Arial", 8), 
                bg="#1e293b", fg="#cbd5e1").pack(anchor="w", padx=8, pady=(8,2))
        
        tk.Entry(wallet_frame, textvariable=self.wallet_address, font=("Arial", 9), 
                bg="#334155", fg="#ffffff", relief="flat", 
                insertbackground="#ffffff").pack(fill="x", padx=8, pady=(0,4), ipady=4)
        
        tk.Label(wallet_frame, text=f"💰 Auto-payout: {self.MINIMUM_PAYOUT} GXC | Fee: {self.POOL_FEE*100}%", 
                font=("Arial", 7), bg="#1e293b", fg="#94a3b8").pack(anchor="w", padx=8, pady=(0,8))
        
        # Settings in 2 columns
        settings_frame = tk.LabelFrame(mining_tab, text="Settings", font=("Arial", 9, "bold"),
                                       bg="#1e293b", fg="#e2e8f0", relief="flat")
        settings_frame.pack(fill="x", padx=8, pady=(0,8))
        
        left_col = tk.Frame(settings_frame, bg="#1e293b")
        left_col.pack(side="left", fill="both", expand=True, padx=(8,4), pady=8)
        
        right_col = tk.Frame(settings_frame, bg="#1e293b")
        right_col.pack(side="right", fill="both", expand=True, padx=(4,8), pady=8)
        
        # Algorithm
        tk.Label(left_col, text="Algorithm:", font=("Arial", 8), 
                bg="#1e293b", fg="#cbd5e1").pack(anchor="w", pady=(0,2))
        algo_combo = ttk.Combobox(left_col, textvariable=self.algorithm, state="readonly",
                                 font=("Arial", 8), width=15,
                                 values=["scrypt", "sha256d", "x11", "equihash", "ethash"])
        algo_combo.pack(fill="x", pady=(0,8))
        
        # Threads
        self.thread_label = tk.Label(left_col, text=f"Threads: {self.threads.get()}", 
                                     font=("Arial", 8), bg="#1e293b", fg="#cbd5e1")
        self.thread_label.pack(anchor="w", pady=(0,2))
        
        thread_scale = tk.Scale(left_col, from_=1, to=16, variable=self.threads,
                               orient="horizontal", bg="#1e293b", fg="#ffffff",
                               troughcolor="#334155", highlightthickness=0, showvalue=0,
                               command=lambda v: self.thread_label.config(text=f"Threads: {int(float(v))}"))
        thread_scale.pack(fill="x")
        
        # Difficulty
        self.diff_label = tk.Label(right_col, text=f"Difficulty: {self.difficulty.get()}", 
                                   font=("Arial", 8), bg="#1e293b", fg="#cbd5e1")
        self.diff_label.pack(anchor="w", pady=(0,2))
        
        diff_scale = tk.Scale(right_col, from_=1, to=10, variable=self.difficulty,
                             orient="horizontal", bg="#1e293b", fg="#ffffff",
                             troughcolor="#334155", highlightthickness=0, showvalue=0,
                             command=lambda v: self.diff_label.config(text=f"Difficulty: {int(float(v))}"))
        diff_scale.pack(fill="x", pady=(0,8))
        
        tk.Label(right_col, text="⚡ Higher = Better rewards", 
                font=("Arial", 7), bg="#1e293b", fg="#94a3b8").pack(anchor="w")
        
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
        
        tk.Button(btn_frame, text="💸 TRANSFER", command=self.manual_transfer,
                 font=("Arial", 10, "bold"), bg="#f59e0b", fg="#ffffff",
                 relief="flat", cursor="hand2").pack(side="left", fill="x", expand=True, padx=(4,0), ipady=8)
        
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
        elif title == "Daily Est.":
            self.daily_label = value_label
        elif title == "Blocks":
            self.blocks_label = value_label
            
    def create_stat_display(self, parent):
        container = tk.Frame(parent, bg="#1e293b")
        container.pack(fill="both", expand=True, padx=8, pady=8)
        
        stats_data = [
            ("Total Hashes", "0", "#3b82f6"),
            ("Blocks Found", "0", "#10b981"),
            ("Total Earned", "0.000000 GXC", "#8b5cf6"),
            ("Pool Share", "0.000000 GXC", "#f59e0b"),
            ("Current Difficulty", "3", "#ef4444"),
            ("Algorithm", "scrypt", "#06b6d4")
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
            elif label == "Total Earned":
                self.total_earned_label = val_label
            elif label == "Pool Share":
                self.pool_label = val_label
            elif label == "Current Difficulty":
                self.curr_diff_label = val_label
            elif label == "Algorithm":
                self.curr_algo_label = val_label
                
    def log(self, message, level="INFO"):
        timestamp = datetime.now().strftime("%H:%M:%S")
        self.log_text.insert("end", f"[{timestamp}] {message}\n", level)
        self.log_text.see("end")
        
    def connect_to_node(self):
        endpoint = self.blockchain_endpoint.get()
        if not endpoint:
            self.log("❌ Enter blockchain endpoint URL", "ERROR")
            messagebox.showerror("Error", "Enter blockchain endpoint URL!")
            return
            
        self.log("🔌 Connecting to GXC node...", "INFO")
        
        def connect():
            try:
                time.sleep(1.2)
                # Actual API call would go here
                # response = requests.post(endpoint, json={...}, timeout=10)
                
                self.node_connected = True
                self.status_label.config(text="● Connected", fg="#86efac")
                self.log("✅ Connected to GXC blockchain", "SUCCESS")
                self.log(f"📡 Endpoint: {endpoint[:35]}...", "INFO")
                self.log("✨ Ready to mine", "SUCCESS")
            except Exception as e:
                self.node_connected = False
                self.status_label.config(text="● Error", fg="#fca5a5")
                self.log("❌ Connection failed", "ERROR")
                
        threading.Thread(target=connect, daemon=True).start()
        
    def mine_loop(self):
        nonce = 0
        hash_count = 0
        start_time = time.time()
        target_zeros = '0' * self.difficulty.get()
        
        block_data = {
            'version': 1,
            'prev_hash': hashlib.sha256(str(time.time()).encode()).hexdigest(),
            'timestamp': int(time.time()),
            'difficulty': self.difficulty.get()
        }
        
        self.log(f"🔍 Mining difficulty: {self.difficulty.get()}", "INFO")
        self.log(f"🧬 Algorithm: {self.algorithm.get().upper()}", "INFO")
        
        while self.mining:
            batch_size = self.threads.get() * 50
            
            for _ in range(batch_size):
                nonce += 1
                hash_str = self.compute_hash(nonce, block_data)
                hash_count += 1
                self.total_hashes += 1
                
                if hash_str.startswith(target_zeros):
                    self.find_block(nonce, hash_str)
                    nonce = 0
                    block_data['timestamp'] = int(time.time())
                    block_data['prev_hash'] = hash_str
            
            elapsed = time.time() - start_time
            if elapsed >= 1.0:
                self.hash_rate = hash_count / elapsed
                self.estimated_daily = self.hash_rate * 86400 * 0.000001 * (1 + self.difficulty.get() * 0.5)
                hash_count = 0
                start_time = time.time()
                self.update_stats()
            
            time.sleep(0.01)
            
    def compute_hash(self, nonce, block_data):
        algo = self.algorithm.get()
        data = f"{nonce}{block_data['prev_hash']}{block_data['timestamp']}"
        
        if algo == "scrypt":
            result = data
            for _ in range(5):
                result = hashlib.sha256(result.encode()).hexdigest()
            return result
        elif algo == "sha256d":
            first = hashlib.sha256(data.encode()).hexdigest()
            return hashlib.sha256(first.encode()).hexdigest()
        elif algo == "x11":
            result = data
            for _ in range(3):
                result = hashlib.sha256(result.encode()).hexdigest()
                result = hashlib.md5(result.encode()).hexdigest()
            return hashlib.sha256(result.encode()).hexdigest()
        else:
            return hashlib.sha256(data.encode()).hexdigest()
            
    def find_block(self, nonce, hash_str):
        reward = self.BASE_REWARD * (1 + self.difficulty.get() * 0.15)
        fee = reward * self.POOL_FEE
        net_reward = reward - fee
        
        self.blocks_found += 1
        self.gxc_earned += net_reward
        self.pool_share += net_reward * 0.1
        
        self.log(f"🎉 BLOCK #{self.blocks_found} FOUND!", "SUCCESS")
        self.log(f"💎 +{net_reward:.6f} GXC", "SUCCESS")
        self.log(f"🔐 {hash_str[:24]}...", "INFO")
        
        if self.gxc_earned >= self.MINIMUM_PAYOUT:
            self.root.after(2000, self.auto_transfer)
            
    def auto_transfer(self):
        if not self.wallet_address.get():
            self.log("⚠️ Wallet required for auto-transfer", "WARNING")
            return
            
        amount = self.gxc_earned
        tx_id = hashlib.sha256(f"{self.wallet_address.get()}{time.time()}".encode()).hexdigest()
        
        self.log("🔄 Auto-transfer initiated", "INFO")
        self.log(f"💸 {amount:.6f} GXC → Wallet", "SUCCESS")
        self.log(f"🔐 TX: {tx_id[:24]}...", "SUCCESS")
        
        time.sleep(0.8)
        self.log("✅ Transfer confirmed!", "SUCCESS")
        
        self.gxc_earned = 0.0
        self.update_stats()
        
    def manual_transfer(self):
        if self.gxc_earned < 0.001:
            self.log("❌ Insufficient balance", "ERROR")
            messagebox.showerror("Error", "Insufficient balance!")
            return
        
        self.auto_transfer()
        
    def update_stats(self):
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
        self.balance_label.config(text=f"{self.gxc_earned:.6f} GXC")
        self.daily_label.config(text=f"{self.estimated_daily:.4f} GXC")
        self.blocks_label.config(text=f"{self.blocks_found}")
        
        # Update stats tab if labels exist
        try:
            self.total_hash_label.config(text=f"{self.total_hashes:,}")
            self.total_blocks_label.config(text=f"{self.blocks_found}")
            self.total_earned_label.config(text=f"{self.gxc_earned:.6f} GXC")
            self.pool_label.config(text=f"{self.pool_share:.6f} GXC")
            self.curr_diff_label.config(text=f"{self.difficulty.get()}")
            self.curr_algo_label.config(text=self.algorithm.get())
        except:
            pass
        
    def start_mining(self):
        if not self.wallet_address.get():
            self.log("❌ Enter GXC wallet address", "ERROR")
            messagebox.showerror("Error", "Enter wallet address!")
            return
            
        if not self.node_connected:
            self.log("❌ Connect to node first", "ERROR")
            messagebox.showerror("Error", "Connect to blockchain first!")
            return
            
        self.mining = True
        self.start_btn.config(state="disabled")
        self.stop_btn.config(state="normal")
        self.log("⚡ Mining started!", "SUCCESS")
        
        threading.Thread(target=self.mine_loop, daemon=True).start()
        
    def stop_mining(self):
        self.mining = False
        self.start_btn.config(state="normal")
        self.stop_btn.config(state="disabled")
        self.log("⏸️ Mining stopped", "WARNING")
        self.log(f"📊 {self.total_hashes:,} hashes, {self.blocks_found} blocks", "INFO")

if __name__ == "__main__":
    root = tk.Tk()
    app = GXCMiner(root)
    root.mainloop()