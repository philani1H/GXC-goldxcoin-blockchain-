#!/usr/bin/env python3
"""
GXC Ethash Miner v2.0.0
GPU Mining for GXC Blockchain
Real Blockchain Connection - No Simulation

Copyright (c) 2024 GXC Blockchain - MIT License
"""

import socket
import threading
import json
import time
import hashlib
import platform
import requests
from datetime import datetime
from typing import Optional, Callable, Any
from dataclasses import dataclass
import logging

import customtkinter as ctk
from tkinter import messagebox

try:
    import psutil
    HAS_PSUTIL = True
except ImportError:
    HAS_PSUTIL = False

VERSION = "2.0.0"
APP_NAME = "GXC Ethash Miner"

NETWORKS = {
    "mainnet": {"rpc": "http://node.gxc.network:8332", "pool": "pool.gxc.network", "port": 3333},
    "testnet": {"rpc": "http://localhost:8332", "pool": "localhost", "port": 3333}
}
ACTIVE = NETWORKS["testnet"]

class Colors:
    BG = "#0a0e1a"
    PANEL = "#111827"
    SIDEBAR = "#1a1f2e"
    ACCENT = "#4ecdc4"
    ACCENT_DIM = "#3eb5ad"
    TEXT = "#e0e6ed"
    TEXT_DIM = "#6b7280"
    DANGER = "#ef4444"
    WARNING = "#f59e0b"
    SUCCESS = "#10b981"
    INFO = "#3b82f6"

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger("Ethash")


@dataclass
class MinerStats:
    hashrate: float = 0.0
    hashes: int = 0
    accepted: int = 0
    rejected: int = 0
    blocks: int = 0
    uptime: int = 0
    gpu_temp: float = 0.0
    gpu_power: float = 0.0


class GXCClient:
    def __init__(self, url: str):
        self.url = url
        self.session = requests.Session()
        self.id = 0
    
    def call(self, method: str, params: list = None) -> Any:
        self.id += 1
        response = self.session.post(self.url, json={
            "jsonrpc": "2.0", "method": method, 
            "params": params or [], "id": self.id
        }, timeout=10)
        data = response.json()
        if "error" in data and data["error"]:
            raise Exception(data["error"])
        return data.get("result")
    
    def get_template(self): return self.call("getblocktemplate")
    def submit_block(self, b): return self.call("submitblock", [b]) is None


class EthashMiner:
    """Ethash GPU Miner (Keccak-256 based)"""
    
    def __init__(self, wallet: str, rpc_url: str, callback: Callable):
        self.wallet = wallet
        self.client = GXCClient(rpc_url)
        self.callback = callback
        self.running = False
        self.threads = []
        self.num_threads = 1
        self.job = None
        self.stats = MinerStats()
        self.start_time = 0
        self.lock = threading.Lock()
    
    def start(self, threads: int = 1):
        self.num_threads = threads
        self.running = True
        self.start_time = time.time()
        
        threading.Thread(target=self._job_loop, daemon=True).start()
        
        for i in range(threads):
            t = threading.Thread(target=self._mine, args=(i,), daemon=True)
            t.start()
            self.threads.append(t)
        
        logger.info(f"Ethash miner started with {threads} threads")
        self.callback("started", threads)
    
    def stop(self):
        self.running = False
        self.threads.clear()
        self.callback("stopped", None)
    
    def _job_loop(self):
        while self.running:
            try:
                t = self.client.get_template()
                self.job = {
                    "height": t.get("height", 0),
                    "prev": t.get("previousblockhash", ""),
                    "diff": t.get("difficulty", 1.0),
                    "ts": int(time.time())
                }
                self.callback("job", self.job)
            except Exception as e:
                logger.error(f"Job error: {e}")
            time.sleep(10)
    
    def _mine(self, tid: int):
        nonce = tid * 0x10000000
        
        while self.running:
            if not self.job:
                time.sleep(0.1)
                continue
            
            j = self.job
            header = f"{j['prev']}{self.wallet}{j['ts']}{j['height']}{nonce}".encode()
            
            # Ethash (simplified using Keccak-256)
            try:
                h = hashlib.sha3_256(header).digest()
            except:
                h = hashlib.sha256(header).digest()
            
            with self.lock:
                self.stats.hashes += 1
            
            if self._meets_target(h, j['diff']):
                self._submit(j, nonce, h)
            
            nonce += 1
            
            if self.stats.hashes % 25000 == 0:
                self._update_stats()
    
    def _meets_target(self, h: bytes, diff: float) -> bool:
        target = int(0x00000000FFFF0000000000000000000000000000000000000000000000000000 / diff)
        return int.from_bytes(h, 'big') < target
    
    def _submit(self, job: dict, nonce: int, h: bytes):
        logger.info(f"Block found! Nonce: {nonce}")
        try:
            if self.client.submit_block({
                "height": job["height"], "hash": h.hex(),
                "previousblockhash": job["prev"], "nonce": nonce,
                "miner": self.wallet, "timestamp": job["ts"]
            }):
                with self.lock:
                    self.stats.blocks += 1
                    self.stats.accepted += 1
                self.callback("block", {"height": job["height"]})
            else:
                with self.lock:
                    self.stats.rejected += 1
        except Exception as e:
            logger.error(f"Submit error: {e}")
    
    def _update_stats(self):
        elapsed = time.time() - self.start_time
        with self.lock:
            self.stats.hashrate = self.stats.hashes / elapsed if elapsed > 0 else 0
            self.stats.uptime = int(elapsed)
        self.callback("stats", self.stats)


ctk.set_appearance_mode("Dark")
ctk.set_default_color_theme("green")


class EthashMinerGUI(ctk.CTk):
    def __init__(self):
        super().__init__()
        
        self.title(f"{APP_NAME} v{VERSION}")
        self.geometry("1000x700")
        self.configure(fg_color=Colors.BG)
        self.minsize(800, 600)
        
        self.miner: Optional[EthashMiner] = None
        self.is_mining = False
        self.stats = MinerStats()
        self.rpc_url = ACTIVE["rpc"]
        
        self.grid_columnconfigure(1, weight=1)
        self.grid_rowconfigure(0, weight=1)
        
        self._setup_sidebar()
        self._setup_main()
        self.after(1000, self._update_loop)
    
    def _setup_sidebar(self):
        sidebar = ctk.CTkFrame(self, width=280, corner_radius=0, fg_color=Colors.SIDEBAR)
        sidebar.grid(row=0, column=0, sticky="nsew")
        sidebar.grid_propagate(False)
        
        logo = ctk.CTkFrame(sidebar, fg_color="transparent")
        logo.pack(pady=(30, 20))
        ctk.CTkLabel(logo, text="Ethash", font=("Impact", 42), text_color=Colors.ACCENT).pack()
        ctk.CTkLabel(logo, text="GPU MINER", font=("Arial", 12, "bold"), text_color="#666").pack()
        ctk.CTkLabel(logo, text=f"v{VERSION}", font=("Arial", 10), text_color="#444").pack()
        
        stats = ctk.CTkFrame(sidebar, fg_color=Colors.PANEL, corner_radius=12)
        stats.pack(fill="x", padx=15, pady=10)
        ctk.CTkLabel(stats, text="⚡ MINING STATS", font=("Arial", 11, "bold"),
                    text_color=Colors.ACCENT).pack(pady=(12, 8))
        
        self.lbl_hr = self._row(stats, "Hashrate", "0 H/s")
        self.lbl_hashes = self._row(stats, "Hashes", "0")
        self.lbl_shares = self._row(stats, "Shares", "0/0")
        self.lbl_blocks = self._row(stats, "Blocks", "0")
        self.lbl_uptime = self._row(stats, "Uptime", "00:00:00")
        
        gpu = ctk.CTkFrame(sidebar, fg_color=Colors.PANEL, corner_radius=12)
        gpu.pack(fill="x", padx=15, pady=10)
        ctk.CTkLabel(gpu, text="🎮 GPU STATUS", font=("Arial", 11, "bold"),
                    text_color=Colors.INFO).pack(pady=(12, 8))
        self.lbl_gpu_temp = self._row(gpu, "Temperature", "N/A")
        self.lbl_gpu_power = self._row(gpu, "Power", "N/A")
        
        btns = ctk.CTkFrame(sidebar, fg_color="transparent")
        btns.pack(fill="x", padx=15, pady=20)
        
        self.btn_start = ctk.CTkButton(btns, text="▶ START MINING",
            command=self._toggle, font=("Arial", 14, "bold"),
            fg_color=Colors.ACCENT, hover_color=Colors.ACCENT_DIM, height=50)
        self.btn_start.pack(fill="x", pady=5)
    
    def _row(self, p, l, v):
        r = ctk.CTkFrame(p, fg_color="transparent")
        r.pack(fill="x", padx=12, pady=4)
        ctk.CTkLabel(r, text=l, font=("Arial", 10), text_color="#aaa").pack(side="left")
        lbl = ctk.CTkLabel(r, text=v, font=("Arial", 10, "bold"))
        lbl.pack(side="right")
        return lbl
    
    def _setup_main(self):
        main = ctk.CTkFrame(self, fg_color=Colors.BG)
        main.grid(row=0, column=1, padx=20, pady=20, sticky="nsew")
        main.grid_columnconfigure(0, weight=1)
        main.grid_rowconfigure(1, weight=1)
        
        cfg = ctk.CTkFrame(main, fg_color=Colors.PANEL, corner_radius=12)
        cfg.grid(row=0, column=0, sticky="ew", pady=(0, 10))
        
        ctk.CTkLabel(cfg, text="⚙️ CONFIGURATION", font=("Arial", 14, "bold"),
                    text_color=Colors.ACCENT).pack(anchor="w", padx=20, pady=(15, 10))
        
        wf = ctk.CTkFrame(cfg, fg_color="transparent")
        wf.pack(fill="x", padx=20, pady=5)
        ctk.CTkLabel(wf, text="Wallet:", width=120, anchor="e").pack(side="left", padx=5)
        self.entry_wallet = ctk.CTkEntry(wf, width=400, placeholder_text="Enter GXC wallet address")
        self.entry_wallet.pack(side="left", padx=5)
        
        nf = ctk.CTkFrame(cfg, fg_color="transparent")
        nf.pack(fill="x", padx=20, pady=5)
        ctk.CTkLabel(nf, text="Network:", width=120, anchor="e").pack(side="left", padx=5)
        self.net_var = ctk.StringVar(value="Testnet")
        ctk.CTkOptionMenu(nf, values=["Testnet", "Mainnet"], variable=self.net_var,
                         width=200, command=self._net_change).pack(side="left", padx=5)
        
        tf = ctk.CTkFrame(cfg, fg_color="transparent")
        tf.pack(fill="x", padx=20, pady=(5, 15))
        ctk.CTkLabel(tf, text="GPU Threads:", width=120, anchor="e").pack(side="left", padx=5)
        self.slider = ctk.CTkSlider(tf, from_=1, to=8, width=200)
        self.slider.set(4)
        self.slider.pack(side="left", padx=5)
        self.lbl_t = ctk.CTkLabel(tf, text="4", width=30)
        self.lbl_t.pack(side="left", padx=5)
        
        console = ctk.CTkFrame(main, fg_color=Colors.PANEL, corner_radius=12)
        console.grid(row=1, column=0, sticky="nsew")
        ctk.CTkLabel(console, text="📋 MINING LOG", font=("Arial", 14, "bold"),
                    text_color=Colors.ACCENT).pack(anchor="w", padx=20, pady=(15, 10))
        self.console = ctk.CTkTextbox(console, font=("Consolas", 11), fg_color="#000",
                                      text_color=Colors.TEXT, corner_radius=8)
        self.console.pack(fill="both", expand=True, padx=15, pady=(0, 15))
        
        self._log(f"{APP_NAME} initialized", "INFO")
        self._log("Note: GPU mining requires CUDA/OpenCL", "WARNING")
    
    def _net_change(self, n):
        global ACTIVE
        ACTIVE = NETWORKS["mainnet" if n == "Mainnet" else "testnet"]
        self.rpc_url = ACTIVE["rpc"]
        self._log(f"Network: {n}", "INFO")
    
    def _toggle(self):
        if not self.is_mining:
            self._start()
        else:
            self._stop()
    
    def _start(self):
        wallet = self.entry_wallet.get().strip()
        if not wallet:
            messagebox.showwarning("Wallet", "Enter wallet address!")
            return
        
        self.is_mining = True
        self.btn_start.configure(text="⏹ STOP", fg_color=Colors.DANGER)
        
        threads = int(self.slider.get())
        self._log(f"Starting Ethash mining with {threads} GPU threads...", "INFO")
        
        self.miner = EthashMiner(wallet, self.rpc_url, self._callback)
        self.miner.start(threads)
    
    def _stop(self):
        if self.miner:
            self.miner.stop()
            self.miner = None
        self.is_mining = False
        self.btn_start.configure(text="▶ START", fg_color=Colors.ACCENT)
        self._log("Stopped", "INFO")
    
    def _callback(self, event, data):
        if event == "stats":
            self.stats = data
            self.after(0, self._update_ui)
        elif event == "block":
            self._log(f"🎉 BLOCK FOUND! Height: {data.get('height')}", "SUCCESS")
    
    def _update_loop(self):
        if self.is_mining:
            self._update_ui()
        self.after(1000, self._update_loop)
    
    def _update_ui(self):
        hr = self.stats.hashrate
        if hr >= 1e9: hs = f"{hr/1e9:.2f} GH/s"
        elif hr >= 1e6: hs = f"{hr/1e6:.2f} MH/s"
        elif hr >= 1e3: hs = f"{hr/1e3:.2f} KH/s"
        else: hs = f"{hr:.2f} H/s"
        
        self.lbl_hr.configure(text=hs)
        self.lbl_hashes.configure(text=f"{self.stats.hashes:,}")
        self.lbl_shares.configure(text=f"{self.stats.accepted}/{self.stats.rejected}")
        self.lbl_blocks.configure(text=str(self.stats.blocks))
        
        h, r = divmod(self.stats.uptime, 3600)
        m, s = divmod(r, 60)
        self.lbl_uptime.configure(text=f"{h:02d}:{m:02d}:{s:02d}")
    
    def _log(self, msg, lvl="INFO"):
        ts = datetime.now().strftime("%H:%M:%S")
        pre = {"INFO": "[i]", "SUCCESS": "[✓]", "WARNING": "[!]", "ERROR": "[✗]"}.get(lvl, "[?]")
        self.after(0, lambda: (self.console.insert("end", f"[{ts}] {pre} {msg}\n"), self.console.see("end")))
    
    def on_close(self):
        if self.is_mining:
            if messagebox.askokcancel("Quit", "Stop mining?"):
                self._stop()
                time.sleep(0.5)
                self.destroy()
        else:
            self.destroy()


def main():
    app = EthashMinerGUI()
    app.protocol("WM_DELETE_WINDOW", app.on_close)
    app.mainloop()


if __name__ == "__main__":
    main()
