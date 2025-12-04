import customtkinter as ctk
import tkinter as tk
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure
import numpy as np
import time

# --- 1. GXC Core Logic (The Backend) ---

# Constants
TARGET_BLOCK_TIME = 10.0
PREDICTIVE_SENSITIVITY = 0.5
STAKE_INFLUENCE_FACTOR = 0.2
BASE_BLOCK_REWARD = 50.0
MIN_REWARD_RATIO = 0.1

class GXCNetwork:
    def __init__(self):
        self.blockchain = []
        # Genesis Block
        self.blockchain.append({
            'id': 0, 'diff': 1000, 'reward': 50, 'hashrate': 50000, 
            'time': 10.0, 'supply': 0
        })
        self.current_hashrate = 50000.0
        self.previous_hashrate = 50000.0
        self.total_staked = 4000000.0
        self.total_supply = 10000000.0

    def compute_predictive_difficulty(self, current_diff, curr_hash, prev_hash):
        if prev_hash == 0: return current_diff
        hash_delta = (curr_hash - prev_hash) / prev_hash
        if hash_delta > 0.10: # 10% spike trigger
            penalty = 1.0 + (hash_delta * PREDICTIVE_SENSITIVITY)
            return current_diff * penalty
        return current_diff

    def apply_staker_influence(self, difficulty, total_staked, total_supply):
        if total_supply == 0: return difficulty
        stake_ratio = total_staked / total_supply
        modifier = 1.0 + (stake_ratio * STAKE_INFLUENCE_FACTOR)
        return difficulty * modifier

    def calculate_emission(self, base_reward, time_taken, target_time):
        if time_taken >= target_time:
            return base_reward
        ratio = time_taken / target_time
        if ratio < MIN_REWARD_RATIO: ratio = MIN_REWARD_RATIO
        return base_reward * ratio

    def mine_block(self, simulated_time_taken, hashrate_override=None):
        last_block = self.blockchain[-1]
        
        prev_hash = self.current_hashrate
        curr_hash = hashrate_override if hashrate_override else prev_hash
        self.previous_hashrate = prev_hash
        self.current_hashrate = curr_hash

        # 1. Difficulty Calc
        base_diff = last_block['diff']
        predictive_diff = self.compute_predictive_difficulty(base_diff, curr_hash, prev_hash)
        final_diff = self.apply_staker_influence(predictive_diff, self.total_staked, self.total_supply)

        # 2. Reward Calc
        reward = self.calculate_emission(BASE_BLOCK_REWARD, simulated_time_taken, TARGET_BLOCK_TIME)

        # 3. Update State
        self.total_supply += reward
        new_block = {
            'id': last_block['id'] + 1,
            'diff': final_diff,
            'reward': reward,
            'hashrate': curr_hash,
            'time': simulated_time_taken,
            'supply': self.total_supply
        }
        self.blockchain.append(new_block)
        return new_block

# --- 2. The GUI (The Frontend) ---

ctk.set_appearance_mode("Dark") 
ctk.set_default_color_theme("dark-blue")

class GXCNodeApp(ctk.CTk):
    def __init__(self):
        super().__init__()
        self.network = GXCNetwork()
        
        # Window Setup
        self.title("GXC Core - Node Simulator v1.0")
        self.geometry("1100x700")
        
        # Layout Config
        self.grid_columnconfigure(1, weight=1)
        self.grid_rowconfigure(0, weight=1)

        # --- Sidebar (Controls) ---
        self.sidebar = ctk.CTkFrame(self, width=200, corner_radius=0)
        self.sidebar.grid(row=0, column=0, sticky="nsew")
        
        self.logo_label = ctk.CTkLabel(self.sidebar, text="GXC CORE", font=ctk.CTkFont(size=20, weight="bold"))
        self.logo_label.grid(row=0, column=0, padx=20, pady=(20, 10))
        
        self.status_label = ctk.CTkLabel(self.sidebar, text="STATUS: IDLE", text_color="gray")
        self.status_label.grid(row=1, column=0, padx=20, pady=10)

        # Simulation Controls
        self.sim_mode = ctk.IntVar(value=0)
        self.radio_normal = ctk.CTkRadioButton(self.sidebar, text="Normal Mining", variable=self.sim_mode, value=0)
        self.radio_normal.grid(row=2, column=0, pady=10, padx=20, sticky="w")
        
        self.radio_attack = ctk.CTkRadioButton(self.sidebar, text="100x Attack Spike", variable=self.sim_mode, value=1, text_color="#FF5555")
        self.radio_attack.grid(row=3, column=0, pady=10, padx=20, sticky="w")
        
        self.radio_recover = ctk.CTkRadioButton(self.sidebar, text="Recovery Phase", variable=self.sim_mode, value=2, text_color="#55FF55")
        self.radio_recover.grid(row=4, column=0, pady=10, padx=20, sticky="w")

        self.btn_mine = ctk.CTkButton(self.sidebar, text="MINE NEXT BLOCK", command=self.manual_mine)
        self.btn_mine.grid(row=5, column=0, padx=20, pady=20)

        # Metrics Panel in Sidebar
        self.metric_frame = ctk.CTkFrame(self.sidebar, fg_color="transparent")
        self.metric_frame.grid(row=6, column=0, padx=20, pady=10, sticky="ew")
        
        self.lbl_height = ctk.CTkLabel(self.metric_frame, text="Height: 0")
        self.lbl_height.pack(anchor="w")
        self.lbl_diff = ctk.CTkLabel(self.metric_frame, text="Diff: 1000")
        self.lbl_diff.pack(anchor="w")
        self.lbl_reward = ctk.CTkLabel(self.metric_frame, text="Reward: 50 GXC")
        self.lbl_reward.pack(anchor="w")

        # --- Main Area (Charts) ---
        self.main_frame = ctk.CTkFrame(self)
        self.main_frame.grid(row=0, column=1, padx=20, pady=20, sticky="nsew")
        
        # Setup Matplotlib Figures
        self.fig = Figure(figsize=(8, 6), dpi=100)
        self.fig.patch.set_facecolor('#242424') # Match Dark Theme
        
        # Plot 1: Hashrate vs Difficulty
        self.ax1 = self.fig.add_subplot(211)
        self.ax1.set_title("Network Security (Hashrate vs Difficulty)", color="white")
        self.ax1.set_facecolor('#2b2b2b')
        self.ax1.tick_params(axis='x', colors='white')
        self.ax1.tick_params(axis='y', colors='white')
        
        # Plot 2: Rewards (The Emission Guard)
        self.ax2 = self.fig.add_subplot(212)
        self.ax2.set_title("Block Rewards (Inflation Control)", color="white")
        self.ax2.set_facecolor('#2b2b2b')
        self.ax2.tick_params(axis='x', colors='white')
        self.ax2.tick_params(axis='y', colors='white')

        self.canvas = FigureCanvasTkAgg(self.fig, master=self.main_frame)
        self.canvas.draw()
        self.canvas.get_tk_widget().pack(fill="both", expand=True)

    def update_charts(self):
        # Extract Data
        data = self.network.blockchain
        ids = [b['id'] for b in data]
        hashrates = [b['hashrate'] for b in data]
        diffs = [b['diff'] for b in data]
        rewards = [b['reward'] for b in data]

        # Clear and Redraw
        self.ax1.clear()
        self.ax2.clear()

        # Config Aesthetics
        self.ax1.set_title("Network Security (Hashrate vs Difficulty)", color="white", fontsize=10)
        self.ax1.set_facecolor('#2b2b2b')
        self.ax2.set_title("Block Rewards (Inflation Control)", color="white", fontsize=10)
        self.ax2.set_facecolor('#2b2b2b')

        # Draw Lines
        self.ax1.plot(ids, hashrates, color='#00ffcc', label='Hashrate')
        self.ax1.plot(ids, diffs, color='#ffcc00', label='Difficulty')
        self.ax1.legend()
        
        # Draw Bar
        colors = ['#ff5555' if r < 40 else '#55ff55' for r in rewards]
        self.ax2.bar(ids, rewards, color=colors)
        self.ax2.set_ylabel("GXC Reward")

        self.canvas.draw()

        # Update Sidebar Metrics
        last = data[-1]
        self.lbl_height.configure(text=f"Height: {last['id']}")
        self.lbl_diff.configure(text=f"Diff: {int(last['diff'])}")
        self.lbl_reward.configure(text=f"Reward: {last['reward']:.2f} GXC", 
                                  text_color="#ff5555" if last['reward'] < 40 else "#55ff55")

    def manual_mine(self):
        mode = self.sim_mode.get()
        
        if mode == 0: # Normal
            self.status_label.configure(text="STATUS: MINING (Normal)")
            self.network.mine_block(simulated_time_taken=10.0, hashrate_override=50000)
        
        elif mode == 1: # Attack
            self.status_label.configure(text="STATUS: UNDER ATTACK!", text_color="red")
            # 100x hashrate -> 0.1s block time
            self.network.mine_block(simulated_time_taken=0.1, hashrate_override=5000000)
            
        elif mode == 2: # Recover
            self.status_label.configure(text="STATUS: RECOVERING", text_color="yellow")
            # High hashrate persists, but diff is high now, so time returns to normal
            self.network.mine_block(simulated_time_taken=10.0, hashrate_override=5000000)
            
        self.update_charts()

if __name__ == "__main__":
    app = GXCNodeApp()
    app.mainloop()