import customtkinter as ctk
import hashlib
import json
import time
import threading
import random
import os
from datetime import datetime
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure

# ==========================================
# PART 1: CRYPTOGRAPHY & DATA STRUCTURES
# ==========================================

class GXCUtils:
    @staticmethod
    def calculate_hash(index, prev_hash, timestamp, data, nonce, difficulty):
        header = f"{index}{prev_hash}{timestamp}{json.dumps(data)}{nonce}{difficulty}"
        return hashlib.sha256(header.encode()).hexdigest()

    @staticmethod
    def get_timestamp():
        return datetime.now().timestamp()

class Transaction:
    def __init__(self, sender, receiver, amount, fee=0.01):
        self.sender = sender
        self.receiver = receiver
        self.amount = amount
        self.fee = fee
        self.timestamp = GXCUtils.get_timestamp()
        self.id = hashlib.sha256(f"{sender}{receiver}{amount}{self.timestamp}".encode()).hexdigest()

    def to_dict(self):
        return {
            "id": self.id,
            "sender": self.sender,
            "receiver": self.receiver,
            "amount": self.amount,
            "fee": self.fee,
            "timestamp": self.timestamp
        }

class Block:
    def __init__(self, index, previous_hash, transactions, difficulty, target_time=10.0):
        self.index = index
        self.previous_hash = previous_hash
        self.transactions = transactions
        self.timestamp = GXCUtils.get_timestamp()
        self.difficulty = difficulty
        self.nonce = 0
        self.hash = ""
        self.target_time = target_time
        self.mining_time = 0
        self.reward = 0

    def mine_block(self, stop_event):
        """ Real PoW: Tries to find a hash starting with Zeros based on difficulty """
        # For simulation speed in Python, we simulate difficulty by "required zeros"
        # In production GXC, this is the raw SHA256 target.
        start_time = time.time()
        prefix_zeros = int(self.difficulty / 1000) # Simple scaling for demo
        if prefix_zeros < 1: prefix_zeros = 1
        target = "0" * prefix_zeros
        
        while True:
            if stop_event.is_set(): return False
            
            self.hash = GXCUtils.calculate_hash(
                self.index, self.previous_hash, self.timestamp, 
                [t.to_dict() for t in self.transactions], self.nonce, self.difficulty
            )
            
            if self.hash.startswith(target):
                self.mining_time = time.time() - start_time
                return True
            else:
                self.nonce += 1

    def calculate_reward(self):
        # EMISSION GUARD LOGIC
        base_reward = 50.0
        if self.mining_time >= self.target_time:
            self.reward = base_reward
        else:
            ratio = self.mining_time / self.target_time
            if ratio < 0.1: ratio = 0.1
            self.reward = base_reward * ratio
        return self.reward

# ==========================================
# PART 2: BLOCKCHAIN MANAGER
# ==========================================

class Blockchain:
    def __init__(self):
        self.chain = []
        self.mempool = []
        self.nodes = set()
        self.difficulty = 1000
        self.total_supply = 0
        self.db_file = "gxc_chain_db.json"
        
        # Load or Create Genesis
        if os.path.exists(self.db_file):
            self.load_chain()
        else:
            self.create_genesis_block()

    def create_genesis_block(self):
        genesis_tx = Transaction("SYSTEM", "GENESIS_WALLET", 0)
        genesis = Block(0, "0", [genesis_tx], 1000)
        genesis.hash = GXCUtils.calculate_hash(0, "0", genesis.timestamp, [genesis_tx.to_dict()], 0, 1000)
        genesis.mining_time = 10.0
        genesis.reward = 50.0
        self.chain.append(genesis)
        self.total_supply += 50.0
        print("GENESIS BLOCK CREATED")

    def get_last_block(self):
        return self.chain[-1]

    def add_transaction(self, sender, receiver, amount):
        tx = Transaction(sender, receiver, amount)
        self.mempool.append(tx)
        return tx

    def adjust_difficulty(self, last_block):
        # PREDICTIVE DIFFICULTY LOGIC
        if len(self.chain) < 2: return last_block.difficulty
        
        prev_block = self.chain[-2]
        
        # Calculate Hashrate Change (Velocity)
        time_delta = last_block.mining_time
        if time_delta == 0: time_delta = 0.001
        
        current_hashrate = last_block.difficulty / time_delta
        
        prev_time_delta = prev_block.mining_time if prev_block.mining_time > 0 else 10.0
        prev_hashrate = prev_block.difficulty / prev_time_delta
        
        # Logic: If hashrate spiked > 10%
        if prev_hashrate > 0:
            change = (current_hashrate - prev_hashrate) / prev_hashrate
            if change > 0.10:
                print(f"[CORE] Hashrate Spike Detected: {change*100:.1f}%")
                return last_block.difficulty * (1.0 + (change * 0.5)) # 0.5 Sensitivity
        
        return last_block.difficulty

    def save_chain(self):
        data = []
        for block in self.chain:
            data.append({
                "index": block.index,
                "hash": block.hash,
                "prev": block.previous_hash,
                "txs": [t.to_dict() for t in block.transactions],
                "diff": block.difficulty,
                "reward": block.reward,
                "time_taken": block.mining_time,
                "timestamp": block.timestamp
            })
        with open(self.db_file, 'w') as f:
            json.dump(data, f, indent=4)
        print("[DB] Blockchain Saved.")

    def load_chain(self):
        # Simplified loader for simulation
        try:
            with open(self.db_file, 'r') as f:
                data = json.load(f)
                for b_data in data:
                    txs = [] # Reconstruct Txs
                    for t in b_data['txs']:
                        txs.append(Transaction(t['sender'], t['receiver'], t['amount'], t['fee']))
                    
                    block = Block(b_data['index'], b_data['prev'], txs, b_data['diff'])
                    block.hash = b_data['hash']
                    block.reward = b_data['reward']
                    block.mining_time = b_data['time_taken']
                    block.timestamp = b_data['timestamp']
                    self.chain.append(block)
                    self.total_supply += block.reward
            print(f"[DB] Loaded {len(self.chain)} blocks.")
        except:
            print("[DB] Error loading. Resetting.")
            self.create_genesis_block()

# ==========================================
# PART 3: THE GUI APPLICATION (ENTERPRISE)
# ==========================================

ctk.set_appearance_mode("Dark")
ctk.set_default_color_theme("dark-blue")

class GXCEnterpriseApp(ctk.CTk):
    def __init__(self):
        super().__init__()
        self.chain = Blockchain()
        self.mining_thread = None
        self.stop_mining = threading.Event()
        self.auto_mine_active = False

        # Window Setup
        self.title("GXC Enterprise Node v3.0 [Mainnet]")
        self.geometry("1280x850")
        
        # Grid Layout
        self.grid_columnconfigure(0, weight=0) # Sidebar
        self.grid_columnconfigure(1, weight=1) # Main
        self.grid_rowconfigure(0, weight=1)

        self.setup_sidebar()
        self.setup_main_view()

    def setup_sidebar(self):
        self.sidebar = ctk.CTkFrame(self, width=250, corner_radius=0)
        self.sidebar.grid(row=0, column=0, sticky="nsew")
        
        # Logo Area
        logo_frame = ctk.CTkFrame(self.sidebar, fg_color="transparent")
        logo_frame.pack(pady=30)
        ctk.CTkLabel(logo_frame, text="GXC", font=("Impact", 40)).pack()
        ctk.CTkLabel(logo_frame, text="ENTERPRISE NODE", font=("Arial", 12, "bold"), text_color="gold").pack()

        # Status Box
        self.status_box = ctk.CTkButton(self.sidebar, text="● SYNCED", fg_color="#1a1a1a", 
                                        border_color="#00FF00", border_width=1, hover=False)
        self.status_box.pack(pady=20, padx=20, fill="x")

        # Navigation
        self.nav_var = ctk.StringVar(value="dashboard")
        
        btn_dash = ctk.CTkRadioButton(self.sidebar, text="Dashboard", variable=self.nav_var, value="dashboard", command=self.switch_tab)
        btn_dash.pack(pady=10, padx=30, anchor="w")
        
        btn_wallet = ctk.CTkRadioButton(self.sidebar, text="Wallet Manager", variable=self.nav_var, value="wallet", command=self.switch_tab)
        btn_wallet.pack(pady=10, padx=30, anchor="w")
        
        btn_miner = ctk.CTkRadioButton(self.sidebar, text="Mining Control", variable=self.nav_var, value="miner", command=self.switch_tab)
        btn_miner.pack(pady=10, padx=30, anchor="w")

        # Stats at Bottom
        self.lbl_blocks = ctk.CTkLabel(self.sidebar, text=f"Height: {len(self.chain.chain)}")
        self.lbl_blocks.pack(side="bottom", pady=5)
        self.lbl_ver = ctk.CTkLabel(self.sidebar, text="v3.0.1-stable", text_color="gray")
        self.lbl_ver.pack(side="bottom", pady=10)

    def setup_main_view(self):
        self.main_area = ctk.CTkFrame(self, fg_color="transparent")
        self.main_area.grid(row=0, column=1, sticky="nsew", padx=20, pady=20)
        
        # Tabs container
        self.tabview = ctk.CTkTabview(self.main_area, width=900, height=700)
        self.tabview.pack(fill="both", expand=True)
        
        self.tab_dash = self.tabview.add("Dashboard")
        self.tab_wallet = self.tabview.add("Wallet")
        self.tab_miner = self.tabview.add("Miner")
        
        self.build_dashboard()
        self.build_wallet()
        self.build_miner()

    # --- TAB 1: DASHBOARD ---
    def build_dashboard(self):
        # KPI Row
        kpi_frame = ctk.CTkFrame(self.tab_dash, fg_color="transparent")
        kpi_frame.pack(fill="x", pady=10)
        
        self.create_kpi(kpi_frame, "Total Supply", f"{self.chain.total_supply:,.2f} GXC", 0)
        self.create_kpi(kpi_frame, "Difficulty", f"{self.chain.difficulty:.0f}", 1)
        self.create_kpi(kpi_frame, "Mempool", "0 Txs", 2)
        
        # Chart Area
        self.chart_frame = ctk.CTkFrame(self.tab_dash, fg_color="#2b2b2b")
        self.chart_frame.pack(fill="both", expand=True, pady=10)
        
        self.fig = Figure(figsize=(6, 4), dpi=100)
        self.fig.patch.set_facecolor('#2b2b2b')
        self.ax = self.fig.add_subplot(111)
        self.ax.set_facecolor('#2b2b2b')
        
        self.canvas = FigureCanvasTkAgg(self.fig, master=self.chart_frame)
        self.canvas.draw()
        self.canvas.get_tk_widget().pack(fill="both", expand=True)
        
        self.update_dashboard_chart()

    def create_kpi(self, parent, title, value, col):
        f = ctk.CTkFrame(parent)
        f.grid(row=0, column=col, padx=10, sticky="ew")
        parent.grid_columnconfigure(col, weight=1)
        ctk.CTkLabel(f, text=title, text_color="gray").pack(pady=(10,0))
        lbl = ctk.CTkLabel(f, text=value, font=("Arial", 20, "bold"))
        lbl.pack(pady=(0,10))
        return lbl

    def update_dashboard_chart(self):
        self.ax.clear()
        # Data
        indices = [b.index for b in self.chain.chain]
        rewards = [b.reward for b in self.chain.chain]
        times = [b.mining_time for b in self.chain.chain]
        
        # Plot Logic
        self.ax.plot(indices, times, color="cyan", label="Block Time (s)")
        self.ax.bar(indices, rewards, color="#333", alpha=0.5, label="Reward")
        self.ax.legend()
        self.ax.tick_params(colors='white')
        self.canvas.draw()

    # --- TAB 2: WALLET ---
    def build_wallet(self):
        ctk.CTkLabel(self.tab_wallet, text="GXC WALLET MANAGER", font=("Arial", 20)).pack(pady=20)
        
        # Address Gen
        self.my_address = f"0x{hashlib.sha256(str(random.random()).encode()).hexdigest()[:30]}"
        ctk.CTkLabel(self.tab_wallet, text="Your Address:").pack()
        self.entry_addr = ctk.CTkEntry(self.tab_wallet, width=400)
        self.entry_addr.insert(0, self.my_address)
        self.entry_addr.configure(state="readonly")
        self.entry_addr.pack(pady=5)
        
        # Send Tx
        ctk.CTkLabel(self.tab_wallet, text="Send Transaction").pack(pady=(30,5))
        self.entry_to = ctk.CTkEntry(self.tab_wallet, placeholder_text="Receiver Address", width=300)
        self.entry_to.pack(pady=5)
        
        self.entry_amt = ctk.CTkEntry(self.tab_wallet, placeholder_text="Amount (GXC)", width=300)
        self.entry_amt.pack(pady=5)
        
        btn_send = ctk.CTkButton(self.tab_wallet, text="Sign & Broadcast", command=self.send_tx)
        btn_send.pack(pady=10)

    def send_tx(self):
        rcv = self.entry_to.get()
        amt = self.entry_amt.get()
        if rcv and amt:
            self.chain.add_transaction(self.my_address, rcv, float(amt))
            print(f"Transaction sent to {rcv}")
            # Visual feedback
            self.entry_to.delete(0, 'end')
            self.entry_amt.delete(0, 'end')

    # --- TAB 3: MINER ---
    def build_miner(self):
        ctk.CTkLabel(self.tab_miner, text="MINING CONTROLS", font=("Arial", 20)).pack(pady=20)
        
        self.console = ctk.CTkTextbox(self.tab_miner, height=300, fg_color="black", text_color="#00FF00", font=("Consolas", 12))
        self.console.pack(fill="x", padx=20, pady=10)
        
        btn_frame = ctk.CTkFrame(self.tab_miner, fg_color="transparent")
        btn_frame.pack(pady=10)
        
        self.btn_start = ctk.CTkButton(btn_frame, text="START MINING", fg_color="green", command=self.start_mining)
        self.btn_start.pack(side="left", padx=10)
        
        self.btn_stop = ctk.CTkButton(btn_frame, text="STOP MINING", fg_color="red", state="disabled", command=self.stop_mining_process)
        self.btn_stop.pack(side="left", padx=10)

    def log(self, msg):
        self.console.insert("end", f"> {msg}\n")
        self.console.see("end")

    # --- MINING LOGIC (THREADED) ---
    def start_mining(self):
        if self.auto_mine_active: return
        self.auto_mine_active = True
        self.stop_mining.clear()
        self.btn_start.configure(state="disabled")
        self.btn_stop.configure(state="normal")
        self.status_box.configure(text="● MINING", border_color="#FFFF00", text_color="yellow")
        
        self.mining_thread = threading.Thread(target=self.mining_loop)
        self.mining_thread.start()

    def stop_mining_process(self):
        self.stop_mining.set()
        self.auto_mine_active = False
        self.btn_start.configure(state="normal")
        self.btn_stop.configure(state="disabled")
        self.status_box.configure(text="● IDLE", border_color="#00FF00", text_color="white")
        self.log("Mining Stopping...")

    def mining_loop(self):
        self.log("Initializing Miner...")
        
        while not self.stop_mining.is_set():
            last_block = self.chain.get_last_block()
            
            # 1. Prepare Block
            new_diff = self.chain.adjust_difficulty(last_block)
            txs = self.chain.mempool[:10] # Take 10 txs
            
            new_block = Block(last_block.index + 1, last_block.hash, txs, new_diff)
            
            self.log(f"Mining Block {new_block.index} [Diff: {new_diff:.0f}]...")
            
            # 2. Mine (Blocking, but in thread)
            success = new_block.mine_block(self.stop_mining)
            
            if success:
                # 3. Finalize
                reward = new_block.calculate_reward()
                self.chain.chain.append(new_block)
                self.chain.total_supply += reward
                self.chain.mempool = self.chain.mempool[10:] # Remove txs
                
                self.log(f"SUCCESS! Hash: {new_block.hash[:10]}...")
                self.log(f"Reward: {reward:.2f} GXC (Time: {new_block.mining_time:.2f}s)")
                
                if reward < 40.0:
                    self.log("WARNING: Emission Guard triggered (Block too fast)")

                # Save to disk
                self.chain.save_chain()
                
                # Update UI
                self.after(0, self.update_dashboard_chart)
                self.after(0, lambda: self.lbl_blocks.configure(text=f"Height: {len(self.chain.chain)}"))

    def switch_tab(self):
        # Helper to handle radio buttons switching tabs
        pass

if __name__ == "__main__":
    app = GXCEnterpriseApp()
    app.mainloop()