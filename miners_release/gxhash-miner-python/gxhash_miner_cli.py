#!/usr/bin/env python3
"""
GXC GxHash Miner v2.0.0 - CLI Version
CPU Miner for GxHash Algorithm (No GUI Dependencies)
Real Blockchain Connection - No Simulation

Copyright (c) 2024 GXC Blockchain - MIT License
"""

import threading
import json
import time
import hashlib
import platform
import argparse
import signal
import sys
import requests
from datetime import datetime
from typing import Optional, Any
from dataclasses import dataclass

try:
    import psutil
    HAS_PSUTIL = True
except ImportError:
    HAS_PSUTIL = False

# ═══════════════════════════════════════════════════════════════════════════════
#                              CONFIGURATION
# ═══════════════════════════════════════════════════════════════════════════════

VERSION = "2.0.0"
APP_NAME = "GXC GxHash Miner CLI"

NETWORKS = {
    "mainnet": {"rpc": "http://node.gxc.network:8332", "pool": "pool.gxc.network", "port": 3333},
    "testnet": {"rpc": "http://localhost:18332", "pool": "localhost", "port": 3333}
}


@dataclass
class MinerStats:
    hashrate: float = 0.0
    hashes: int = 0
    accepted: int = 0
    rejected: int = 0
    blocks: int = 0
    uptime: int = 0
    cpu: float = 0.0


class GXCClient:
    """Real GXC Blockchain RPC Client"""
    
    def __init__(self, url: str):
        self.url = url
        self.session = requests.Session()
        self.id = 0
    
    def call(self, method: str, params: list = None) -> Any:
        self.id += 1
        try:
            response = self.session.post(self.url, json={
                "jsonrpc": "2.0", "method": method, 
                "params": params or [], "id": self.id
            }, timeout=10)
            data = response.json()
            if "error" in data and data["error"]:
                raise Exception(data["error"])
            return data.get("result")
        except requests.exceptions.RequestException as e:
            raise Exception(f"Connection error: {e}")
    
    def get_template(self): return self.call("getblocktemplate")
    def submit_block(self, b): return self.call("submitblock", [b]) is None
    def get_info(self): return self.call("getblockchaininfo")
    def get_balance(self, a): return self.call("getbalance", [a])


class GxHashMinerCLI:
    """GxHash CPU Miner - CLI Version"""
    
    def __init__(self, wallet: str, rpc_url: str):
        self.wallet = wallet
        self.client = GXCClient(rpc_url)
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
        
        # Job fetcher
        threading.Thread(target=self._job_loop, daemon=True).start()
        
        # Mining threads
        for i in range(threads):
            t = threading.Thread(target=self._mine, args=(i,), daemon=True)
            t.start()
            self.threads.append(t)
        
        print(f"[{self._ts()}] Started with {threads} threads")
    
    def stop(self):
        self.running = False
        self.threads.clear()
        print(f"[{self._ts()}] Miner stopped")
    
    def _ts(self):
        return datetime.now().strftime("%H:%M:%S")
    
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
                print(f"[{self._ts()}] New job: height={self.job['height']}, diff={self.job['diff']}")
            except Exception as e:
                print(f"[{self._ts()}] Job error: {e}")
            time.sleep(10)
    
    def _mine(self, tid: int):
        nonce = tid * 0x10000000
        
        while self.running:
            if not self.job:
                time.sleep(0.1)
                continue
            
            j = self.job
            header = f"{j['prev']}{self.wallet}{j['ts']}{j['height']}{nonce}".encode()
            h = self._gxhash(header)
            
            with self.lock:
                self.stats.hashes += 1
            
            if self._meets_target(h, j['diff']):
                self._submit(j, nonce, h)
            
            nonce += 1
            
            if self.stats.hashes % 50000 == 0:
                self._print_stats()
    
    def _gxhash(self, data: bytes) -> bytes:
        h1 = hashlib.sha256(data).digest()
        try:
            h2 = hashlib.blake2b(h1 + data, digest_size=32).digest()
        except:
            h2 = hashlib.sha256(h1 + data).digest()
        mixed = bytes(a ^ b for a, b in zip(h1, h2))
        h3 = hashlib.sha256(mixed).digest()
        try:
            return hashlib.sha3_256(h3).digest()
        except:
            return hashlib.sha256(h3).digest()
    
    def _meets_target(self, h: bytes, diff: float) -> bool:
        target = int(0x00000000FFFF0000000000000000000000000000000000000000000000000000 / diff)
        return int.from_bytes(h, 'big') < target
    
    def _submit(self, job: dict, nonce: int, h: bytes):
        print(f"[{self._ts()}] 🎉 Block found! Nonce: {nonce}")
        try:
            if self.client.submit_block({
                "height": job["height"], "hash": h.hex(),
                "previousblockhash": job["prev"], "nonce": nonce,
                "miner": self.wallet, "timestamp": job["ts"],
                "difficulty": job["diff"]
            }):
                with self.lock:
                    self.stats.blocks += 1
                    self.stats.accepted += 1
                print(f"[{self._ts()}] ✓ Block accepted!")
            else:
                with self.lock:
                    self.stats.rejected += 1
                print(f"[{self._ts()}] ✗ Block rejected")
        except Exception as e:
            print(f"[{self._ts()}] Submit error: {e}")
    
    def _print_stats(self):
        elapsed = time.time() - self.start_time
        with self.lock:
            self.stats.hashrate = self.stats.hashes / elapsed if elapsed > 0 else 0
            self.stats.uptime = int(elapsed)
            if HAS_PSUTIL:
                self.stats.cpu = psutil.cpu_percent()
        
        hr = self.stats.hashrate
        if hr >= 1e6: hs = f"{hr/1e6:.2f} MH/s"
        elif hr >= 1e3: hs = f"{hr/1e3:.2f} KH/s"
        else: hs = f"{hr:.2f} H/s"
        
        h, r = divmod(self.stats.uptime, 3600)
        m, s = divmod(r, 60)
        
        print(f"[{self._ts()}] Rate: {hs} | Hashes: {self.stats.hashes:,} | "
              f"Blocks: {self.stats.blocks} | Uptime: {h:02d}:{m:02d}:{s:02d}")


def main():
    parser = argparse.ArgumentParser(
        description=f"{APP_NAME} v{VERSION}",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  %(prog)s --address GXC123... --rpc-url http://localhost:18332
  %(prog)s --address GXC123... --threads 4 --network testnet
        """
    )
    
    parser.add_argument("--address", "-a", required=True, help="GXC wallet address")
    parser.add_argument("--rpc-url", "-r", default=None, help="RPC URL (overrides network)")
    parser.add_argument("--network", "-n", default="testnet", choices=["mainnet", "testnet"], 
                       help="Network to use (default: testnet)")
    parser.add_argument("--threads", "-t", type=int, default=0, 
                       help="Mining threads (0 = auto-detect)")
    parser.add_argument("--version", "-v", action="version", version=f"{APP_NAME} v{VERSION}")
    
    args = parser.parse_args()
    
    # Determine RPC URL
    rpc_url = args.rpc_url or NETWORKS[args.network]["rpc"]
    
    # Determine thread count
    if args.threads <= 0:
        if HAS_PSUTIL:
            threads = max(1, psutil.cpu_count(logical=True) - 1)
        else:
            threads = 4
    else:
        threads = args.threads
    
    print(f"""
╔════════════════════════════════════════════════════════════════╗
║             GXC GxHash Miner v{VERSION} (CLI)                    ║
║                                                                ║
║  Wallet:  {args.address[:30]}...
║  RPC:     {rpc_url}
║  Threads: {threads}
║  Network: {args.network}
╚════════════════════════════════════════════════════════════════╝
""")
    
    # Test connection
    print(f"Connecting to {rpc_url}...")
    try:
        client = GXCClient(rpc_url)
        info = client.get_info()
        print(f"✓ Connected! Chain: {info.get('chain', 'unknown')}, Height: {info.get('height', 0)}")
    except Exception as e:
        print(f"✗ Connection failed: {e}")
        print("Make sure the GXC node is running with RPC enabled.")
        sys.exit(1)
    
    # Start miner
    miner = GxHashMinerCLI(args.address, rpc_url)
    
    def signal_handler(sig, frame):
        print("\nShutting down...")
        miner.stop()
        sys.exit(0)
    
    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)
    
    miner.start(threads)
    
    print("\n⚡ Mining started! Press Ctrl+C to stop.\n")
    
    # Keep main thread alive
    while miner.running:
        time.sleep(1)


if __name__ == "__main__":
    main()
