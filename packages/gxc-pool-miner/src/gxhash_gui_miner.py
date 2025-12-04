#!/usr/bin/env python3
"""
GXC GxHash GUI Miner
CPU-optimized miner for GxHash algorithm
Real blockchain connection - No simulation

Copyright (c) 2024 GXC Blockchain
"""

import socket
import threading
import json
import time
import hashlib
import os
import platform
import requests
from datetime import datetime
from typing import Optional, Callable, Any
from dataclasses import dataclass
from queue import Queue
import logging

# GUI
import customtkinter as ctk
from tkinter import messagebox

# System
try:
    import psutil
    HAS_PSUTIL = True
except ImportError:
    HAS_PSUTIL = False

# ============================================================================
#                              CONFIGURATION
# ============================================================================

VERSION = "2.0.0"
APP_NAME = "GXC GxHash Miner"

# Network
TESTNET_RPC = "http://localhost:8332"
MAINNET_RPC = "http://node.gxc.network:8332"

# Theme
COLORS = {
    "bg": "#0a0e1a",
    "panel": "#111827",
    "sidebar": "#1a1f2e",
    "accent": "#00ff9d",
    "accent_dim": "#008f58",
    "text": "#e0e6ed",
    "danger": "#ff3333",
    "warning": "#ffaa00",
    "success": "#00ff9d",
    "info": "#3b82f6"
}

logging.basicConfig(level=logging.INFO, format='%(asctime)s [%(levelname)s] %(message)s')
logger = logging.getLogger("GxHash")


# ============================================================================
#                           DATA CLASSES
# ============================================================================

@dataclass
class MinerStats:
    hashrate: float = 0.0
    hashes_done: int = 0
    shares_accepted: int = 0
    shares_rejected: int = 0
    blocks_found: int = 0
    uptime: int = 0
    cpu_usage: float = 0.0
    temperature: float = 0.0


# ============================================================================
#                        BLOCKCHAIN CLIENT
# ============================================================================

class GXCClient:
    """GXC Blockchain RPC Client"""
    
    def __init__(self, rpc_url: str):
        self.rpc_url = rpc_url
        self.session = requests.Session()
        self.request_id = 0
        
    def call(self, method: str, params: list = None) -> Any:
        if params is None:
            params = []
            
        self.request_id += 1
        payload = {
            "jsonrpc": "2.0",
            "method": method,
            "params": params,
            "id": self.request_id
        }
        
        response = self.session.post(self.rpc_url, json=payload, timeout=10)
        data = response.json()
        
        if "error" in data and data["error"]:
            raise Exception(f"RPC Error: {data['error']}")
            
        return data.get("result")
    
    def get_block_template(self) -> dict:
        return self.call("getblocktemplate")
    
    def submit_block(self, block_data: dict) -> bool:
        result = self.call("submitblock", [block_data])
        return result is None
    
    def get_blockchain_info(self) -> dict:
        return self.call("getblockchaininfo")
    
    def get_balance(self, address: str) -> float:
        return self.call("getbalance", [address])


# ============================================================================
#                          GXHASH MINER
# ============================================================================

class GxHashMiner:
    """GxHash CPU Miner - Traceability-optimized algorithm"""
    
    def __init__(self, wallet_address: str, rpc_url: str, callback: Callable):
        self.wallet_address = wallet_address
        self.client = GXCClient(rpc_url)
        self.callback = callback
        self.running = False
        self.paused = False
        self.threads = []
        self.num_threads = 1
        self.current_job = None
        self.nonce = 0
        self.stats = MinerStats()
        self.start_time = 0
        self.lock = threading.Lock()
        
    def start(self, num_threads: int = None):
        """Start mining with specified threads"""
        if num_threads is None:
            num_threads = max(1, (psutil.cpu_count(logical=False) or 1) - 1)
        
        self.num_threads = num_threads
        self.running = True
        self.paused = False
        self.start_time = time.time()
        self.stats = MinerStats()
        
        # Start job fetcher
        job_thread = threading.Thread(target=self._job_loop, daemon=True)
        job_thread.start()
        
        # Start mining threads
        for i in range(num_threads):
            t = threading.Thread(target=self._mine_loop, args=(i,), daemon=True)
            t.start()
            self.threads.append(t)
        
        logger.info(f"Started mining with {num_threads} threads")
        self.callback("started", num_threads)
    
    def stop(self):
        """Stop mining"""
        self.running = False
        self.threads.clear()
        logger.info("Mining stopped")
        self.callback("stopped", None)
    
    def pause(self):
        """Pause mining"""
        self.paused = True
        self.callback("paused", None)
    
    def resume(self):
        """Resume mining"""
        self.paused = False
        self.callback("resumed", None)
    
    def _job_loop(self):
        """Fetch new jobs from blockchain"""
        while self.running:
            try:
                template = self.client.get_block_template()
                self.current_job = {
                    "height": template.get("height", 0),
                    "previousblockhash": template.get("previousblockhash", ""),
                    "difficulty": template.get("difficulty", 1.0),
                    "target": template.get("target", "0" * 64),
                    "coinbase_value": template.get("coinbase_value", 50.0),
                    "timestamp": int(time.time())
                }
                self.nonce = 0
                self.callback("new_job", self.current_job)
                
            except Exception as e:
                logger.error(f"Failed to get job: {e}")
            
            time.sleep(10)  # Poll every 10 seconds
    
    def _mine_loop(self, thread_id: int):
        """Mining loop for a single thread"""
        local_nonce = thread_id * 0x10000000
        
        while self.running:
            if self.paused or not self.current_job:
                time.sleep(0.1)
                continue
            
            job = self.current_job
            
            # Build header
            header = self._build_header(job, local_nonce)
            
            # Compute GxHash
            hash_result = self._gxhash(header)
            
            with self.lock:
                self.stats.hashes_done += 1
            
            # Check if meets target
            if self._meets_target(hash_result, job.get("difficulty", 1.0)):
                self._submit_block(job, local_nonce, hash_result)
            
            local_nonce += 1
            
            # Update stats periodically
            if self.stats.hashes_done % 10000 == 0:
                self._update_stats()
    
    def _build_header(self, job: dict, nonce: int) -> bytes:
        """Build block header for hashing"""
        data = (
            job.get("previousblockhash", "0" * 64) +
            self.wallet_address +
            str(job.get("timestamp", 0)) +
            str(job.get("height", 0)) +
            str(nonce)
        )
        return data.encode('utf-8')
    
    def _gxhash(self, data: bytes) -> bytes:
        """
        GxHash Algorithm
        Multi-round hashing with traceability optimization
        """
        # Round 1: SHA256
        h1 = hashlib.sha256(data).digest()
        
        # Round 2: Blake2b mixing
        try:
            h2 = hashlib.blake2b(h1 + data, digest_size=32).digest()
        except:
            h2 = hashlib.sha256(h1 + data).digest()
        
        # Round 3: XOR mixing for traceability
        mixed = bytes(a ^ b for a, b in zip(h1, h2))
        
        # Round 4: Final SHA256
        h3 = hashlib.sha256(mixed).digest()
        
        # Round 5: SHA3 finalization (if available)
        try:
            result = hashlib.sha3_256(h3).digest()
        except:
            result = hashlib.sha256(h3).digest()
        
        return result
    
    def _meets_target(self, hash_result: bytes, difficulty: float) -> bool:
        """Check if hash meets difficulty target"""
        hash_int = int.from_bytes(hash_result, 'big')
        target = int(0x00000000FFFF0000000000000000000000000000000000000000000000000000 / difficulty)
        return hash_int < target
    
    def _submit_block(self, job: dict, nonce: int, hash_result: bytes):
        """Submit found block to blockchain"""
        logger.info(f"Found valid block! Nonce: {nonce}")
        
        block_data = {
            "height": job.get("height", 0),
            "hash": hash_result.hex(),
            "previousblockhash": job.get("previousblockhash", ""),
            "nonce": nonce,
            "miner": self.wallet_address,
            "timestamp": job.get("timestamp", int(time.time())),
            "difficulty": job.get("difficulty", 1.0),
            "consensus_type": "pow",
            "block_type": "pow"
        }
        
        try:
            if self.client.submit_block(block_data):
                with self.lock:
                    self.stats.blocks_found += 1
                    self.stats.shares_accepted += 1
                logger.info(f"Block accepted! Height: {job.get('height')}")
                self.callback("block_found", block_data)
            else:
                with self.lock:
                    self.stats.shares_rejected += 1
                logger.warning("Block rejected")
                self.callback("block_rejected", block_data)
        except Exception as e:
            logger.error(f"Block submission failed: {e}")
            with self.lock:
                self.stats.shares_rejected += 1
    
    def _update_stats(self):
        """Update mining statistics"""
        elapsed = time.time() - self.start_time
        
        with self.lock:
            self.stats.hashrate = self.stats.hashes_done / elapsed if elapsed > 0 else 0
            self.stats.uptime = int(elapsed)
            
            if HAS_PSUTIL:
                self.stats.cpu_usage = psutil.cpu_percent()
                try:
                    temps = psutil.sensors_temperatures()
                    if temps:
                        for name, entries in temps.items():
                            if entries:
                                self.stats.temperature = entries[0].current
                                break
                except:
                    pass
        
        self.callback("stats", self.stats)


# ============================================================================
#                           GUI APPLICATION
# ============================================================================

ctk.set_appearance_mode("Dark")
ctk.set_default_color_theme("green")


class GxHashMinerGUI(ctk.CTk):
    """GxHash Miner GUI Application"""
    
    def __init__(self):
        super().__init__()
        
        self.title(f"{APP_NAME} v{VERSION}")
        self.geometry("1000x700")
        self.configure(fg_color=COLORS["bg"])
        self.minsize(800, 600)
        
        # State
        self.miner: Optional[GxHashMiner] = None
        self.is_mining = False
        self.stats = MinerStats()
        self.hashrate_history = []
        
        # RPC URL
        self.rpc_url = TESTNET_RPC
        
        # Layout
        self.grid_columnconfigure(1, weight=1)
        self.grid_rowconfigure(0, weight=1)
        
        self._setup_sidebar()
        self._setup_main()
        
        # Update loop
        self.after(1000, self._update_loop)
    
    def _setup_sidebar(self):
        """Setup sidebar"""
        sidebar = ctk.CTkFrame(self, width=280, corner_radius=0, fg_color=COLORS["sidebar"])
        sidebar.grid(row=0, column=0, sticky="nsew")
        sidebar.grid_propagate(False)
        
        # Logo
        logo = ctk.CTkFrame(sidebar, fg_color="transparent")
        logo.pack(pady=(30, 20))
        
        ctk.CTkLabel(logo, text="GxHash", font=("Impact", 42), text_color=COLORS["accent"]).pack()
        ctk.CTkLabel(logo, text="CPU MINER", font=("Arial", 12, "bold"), text_color="#666").pack()
        ctk.CTkLabel(logo, text=f"v{VERSION}", font=("Arial", 9), text_color="#444").pack()
        
        # Stats
        stats = ctk.CTkFrame(sidebar, fg_color=COLORS["panel"], corner_radius=10)
        stats.pack(fill="x", padx=15, pady=10)
        
        ctk.CTkLabel(stats, text="⚡ MINING STATS", font=("Arial", 11, "bold"),
                    text_color=COLORS["accent"]).pack(pady=(12, 8))
        
        self.lbl_hashrate = self._stat_row(stats, "Hashrate", "0 H/s")
        self.lbl_hashes = self._stat_row(stats, "Total Hashes", "0")
        self.lbl_shares = self._stat_row(stats, "Shares", "0 / 0")
        self.lbl_blocks = self._stat_row(stats, "Blocks Found", "0")
        self.lbl_uptime = self._stat_row(stats, "Uptime", "00:00:00")
        
        # System
        sys_frame = ctk.CTkFrame(sidebar, fg_color=COLORS["panel"], corner_radius=10)
        sys_frame.pack(fill="x", padx=15, pady=10)
        
        ctk.CTkLabel(sys_frame, text="💻 SYSTEM", font=("Arial", 11, "bold"),
                    text_color=COLORS["info"]).pack(pady=(12, 8))
        
        self.lbl_cpu = self._stat_row(sys_frame, "CPU Usage", "0%")
        self.lbl_temp = self._stat_row(sys_frame, "Temperature", "N/A")
        self.lbl_threads = self._stat_row(sys_frame, "Threads", "0")
        
        # Buttons
        btn_frame = ctk.CTkFrame(sidebar, fg_color="transparent")
        btn_frame.pack(fill="x", padx=15, pady=20)
        
        self.btn_start = ctk.CTkButton(
            btn_frame,
            text="▶ START MINING",
            command=self._toggle_mining,
            font=("Arial", 14, "bold"),
            fg_color=COLORS["accent"],
            hover_color=COLORS["accent_dim"],
            height=50,
            corner_radius=10
        )
        self.btn_start.pack(fill="x", pady=5)
    
    def _stat_row(self, parent, label: str, value: str) -> ctk.CTkLabel:
        row = ctk.CTkFrame(parent, fg_color="transparent")
        row.pack(fill="x", padx=12, pady=4)
        
        ctk.CTkLabel(row, text=label, font=("Arial", 10), text_color="#aaa").pack(side="left")
        lbl = ctk.CTkLabel(row, text=value, font=("Arial", 10, "bold"), text_color="#fff")
        lbl.pack(side="right")
        return lbl
    
    def _setup_main(self):
        """Setup main content"""
        main = ctk.CTkFrame(self, fg_color=COLORS["bg"])
        main.grid(row=0, column=1, padx=20, pady=20, sticky="nsew")
        main.grid_columnconfigure(0, weight=1)
        main.grid_rowconfigure(1, weight=1)
        
        # Config section
        config = ctk.CTkFrame(main, fg_color=COLORS["panel"], corner_radius=12)
        config.grid(row=0, column=0, sticky="ew", pady=(0, 10))
        
        ctk.CTkLabel(config, text="⚙️ CONFIGURATION", font=("Arial", 14, "bold"),
                    text_color=COLORS["accent"]).pack(anchor="w", padx=20, pady=(15, 10))
        
        # Wallet
        wallet_frame = ctk.CTkFrame(config, fg_color="transparent")
        wallet_frame.pack(fill="x", padx=20, pady=5)
        
        ctk.CTkLabel(wallet_frame, text="Wallet Address:", width=120, anchor="e").pack(side="left", padx=5)
        self.entry_wallet = ctk.CTkEntry(wallet_frame, width=400, placeholder_text="Enter your GXC wallet address")
        self.entry_wallet.pack(side="left", padx=5)
        
        # Network
        net_frame = ctk.CTkFrame(config, fg_color="transparent")
        net_frame.pack(fill="x", padx=20, pady=5)
        
        ctk.CTkLabel(net_frame, text="Network:", width=120, anchor="e").pack(side="left", padx=5)
        self.network_var = ctk.StringVar(value="Testnet")
        ctk.CTkOptionMenu(net_frame, values=["Testnet", "Mainnet"], variable=self.network_var,
                         width=200, command=self._on_network_change).pack(side="left", padx=5)
        
        # Threads
        thread_frame = ctk.CTkFrame(config, fg_color="transparent")
        thread_frame.pack(fill="x", padx=20, pady=(5, 15))
        
        max_threads = psutil.cpu_count(logical=True) if HAS_PSUTIL else 4
        ctk.CTkLabel(thread_frame, text="Mining Threads:", width=120, anchor="e").pack(side="left", padx=5)
        self.slider_threads = ctk.CTkSlider(thread_frame, from_=1, to=max_threads, number_of_steps=max_threads-1,
                                           width=200, command=self._on_threads_change)
        self.slider_threads.set(max(1, max_threads - 1))
        self.slider_threads.pack(side="left", padx=5)
        self.lbl_thread_count = ctk.CTkLabel(thread_frame, text=str(max(1, max_threads - 1)), width=30)
        self.lbl_thread_count.pack(side="left", padx=5)
        
        # Console
        console_frame = ctk.CTkFrame(main, fg_color=COLORS["panel"], corner_radius=12)
        console_frame.grid(row=1, column=0, sticky="nsew")
        
        ctk.CTkLabel(console_frame, text="📋 MINING LOG", font=("Arial", 14, "bold"),
                    text_color=COLORS["accent"]).pack(anchor="w", padx=20, pady=(15, 10))
        
        self.console = ctk.CTkTextbox(console_frame, font=("Consolas", 11), fg_color="#000",
                                      text_color=COLORS["text"], corner_radius=8)
        self.console.pack(fill="both", expand=True, padx=15, pady=(0, 15))
        
        self._log("GxHash Miner initialized", "INFO")
        self._log(f"System: {platform.system()} {platform.release()}", "INFO")
        if HAS_PSUTIL:
            cpu_count = psutil.cpu_count(logical=True)
            self._log(f"CPU: {cpu_count} threads available", "INFO")
    
    def _on_network_change(self, network: str):
        if network == "Mainnet":
            self.rpc_url = MAINNET_RPC
        else:
            self.rpc_url = TESTNET_RPC
        self._log(f"Network changed to {network}", "INFO")
    
    def _on_threads_change(self, value):
        self.lbl_thread_count.configure(text=str(int(value)))
    
    def _toggle_mining(self):
        if not self.is_mining:
            self._start_mining()
        else:
            self._stop_mining()
    
    def _start_mining(self):
        wallet = self.entry_wallet.get().strip()
        if not wallet:
            messagebox.showwarning("Wallet Required", "Please enter your GXC wallet address!")
            return
        
        self.is_mining = True
        self.btn_start.configure(text="⏹ STOP MINING", fg_color=COLORS["danger"])
        
        threads = int(self.slider_threads.get())
        self.lbl_threads.configure(text=str(threads))
        
        self._log(f"Starting mining with {threads} threads...", "INFO")
        self._log(f"Wallet: {wallet[:20]}...", "INFO")
        self._log(f"Node: {self.rpc_url}", "INFO")
        
        self.miner = GxHashMiner(wallet, self.rpc_url, self._miner_callback)
        self.miner.start(threads)
    
    def _stop_mining(self):
        if self.miner:
            self.miner.stop()
            self.miner = None
        
        self.is_mining = False
        self.btn_start.configure(text="▶ START MINING", fg_color=COLORS["accent"])
        self._log("Mining stopped", "INFO")
    
    def _miner_callback(self, event: str, data: Any):
        if event == "stats":
            self.stats = data
            self.after(0, self._update_ui)
        elif event == "new_job":
            self._log(f"New job: height={data.get('height')}, diff={data.get('difficulty'):.2f}", "INFO")
        elif event == "block_found":
            self._log(f"🎉 BLOCK FOUND! Height: {data.get('height')}", "SUCCESS")
        elif event == "block_rejected":
            self._log(f"Block rejected", "WARNING")
        elif event == "started":
            self._log(f"Mining started with {data} threads", "SUCCESS")
        elif event == "stopped":
            self._log("Mining stopped", "INFO")
    
    def _update_loop(self):
        """Main update loop"""
        if self.is_mining:
            self._update_ui()
        self.after(1000, self._update_loop)
    
    def _update_ui(self):
        """Update UI with current stats"""
        # Hashrate
        hr = self.stats.hashrate
        if hr >= 1e6:
            hr_str = f"{hr/1e6:.2f} MH/s"
        elif hr >= 1e3:
            hr_str = f"{hr/1e3:.2f} KH/s"
        else:
            hr_str = f"{hr:.2f} H/s"
        self.lbl_hashrate.configure(text=hr_str)
        
        # Other stats
        self.lbl_hashes.configure(text=f"{self.stats.hashes_done:,}")
        self.lbl_shares.configure(text=f"{self.stats.shares_accepted} / {self.stats.shares_rejected}")
        self.lbl_blocks.configure(text=str(self.stats.blocks_found))
        
        # Uptime
        h, r = divmod(self.stats.uptime, 3600)
        m, s = divmod(r, 60)
        self.lbl_uptime.configure(text=f"{h:02d}:{m:02d}:{s:02d}")
        
        # System
        self.lbl_cpu.configure(text=f"{self.stats.cpu_usage:.1f}%")
        if self.stats.temperature > 0:
            self.lbl_temp.configure(text=f"{self.stats.temperature:.0f}°C")
    
    def _log(self, message: str, level: str = "INFO"):
        timestamp = datetime.now().strftime("%H:%M:%S")
        prefix = {"INFO": "[i]", "SUCCESS": "[✓]", "WARNING": "[!]", "ERROR": "[✗]"}.get(level, "[?]")
        
        def write():
            self.console.insert("end", f"[{timestamp}] {prefix} {message}\n")
            self.console.see("end")
        
        self.after(0, write)
    
    def on_closing(self):
        if self.is_mining:
            if messagebox.askokcancel("Quit", "Mining is active. Stop and quit?"):
                self._stop_mining()
                time.sleep(0.5)
                self.destroy()
        else:
            self.destroy()


def main():
    app = GxHashMinerGUI()
    app.protocol("WM_DELETE_WINDOW", app.on_closing)
    app.mainloop()


if __name__ == "__main__":
    main()
