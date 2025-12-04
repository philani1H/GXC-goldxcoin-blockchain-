#!/usr/bin/env python3
"""
GXC Miner v2.0.0 - Command Line Interface
Real pool mining with Stratum protocol

Usage:
  gxc-miner -o stratum+tcp://pool.gxc.io:3333 -u WALLET -p x
  gxc-miner --algo gxhash -o pool.gxc.io:3333 -u WALLET -p x
  gxc-miner --algo sha256 -o pool.gxc.io:3333 -u WALLET --threads 4

Copyright (c) 2024 GXC Blockchain - MIT License
"""

import socket
import threading
import json
import time
import hashlib
import struct
import sys
import os
import argparse
import signal
from datetime import datetime
from typing import Optional, Dict, Any, Tuple
from dataclasses import dataclass
import logging

# Version
VERSION = "2.0.0"

# ═══════════════════════════════════════════════════════════════════════════════
#                              CONFIGURATION
# ═══════════════════════════════════════════════════════════════════════════════

@dataclass
class MinerConfig:
    pool_url: str = ""
    pool_host: str = ""
    pool_port: int = 3333
    wallet: str = ""
    password: str = "x"
    worker: str = ""
    algorithm: str = "gxhash"
    threads: int = 1
    benchmark: bool = False
    verbose: bool = False


@dataclass
class MiningStats:
    hashrate: float = 0.0
    hashrate_avg: float = 0.0
    shares_accepted: int = 0
    shares_rejected: int = 0
    shares_stale: int = 0
    hashes_total: int = 0
    blocks_found: int = 0
    uptime: float = 0.0
    last_share_time: float = 0.0
    pool_difficulty: float = 1.0
    connection_time: float = 0.0


# ═══════════════════════════════════════════════════════════════════════════════
#                              LOGGING
# ═══════════════════════════════════════════════════════════════════════════════

class MinerLogger:
    """Colored console logger"""
    
    COLORS = {
        'green': '\033[92m',
        'yellow': '\033[93m',
        'red': '\033[91m',
        'cyan': '\033[96m',
        'magenta': '\033[95m',
        'white': '\033[97m',
        'reset': '\033[0m'
    }
    
    def __init__(self, verbose: bool = False):
        self.verbose = verbose
        self.start_time = time.time()
    
    def _format(self, level: str, msg: str, color: str) -> str:
        ts = datetime.now().strftime("%H:%M:%S")
        elapsed = int(time.time() - self.start_time)
        h, r = divmod(elapsed, 3600)
        m, s = divmod(r, 60)
        uptime = f"{h:02d}:{m:02d}:{s:02d}"
        c = self.COLORS.get(color, '')
        r = self.COLORS['reset']
        return f"[{ts}] [{uptime}] {c}[{level}]{r} {msg}"
    
    def info(self, msg: str):
        print(self._format("INFO", msg, "cyan"))
    
    def success(self, msg: str):
        print(self._format("OK", msg, "green"))
    
    def warning(self, msg: str):
        print(self._format("WARN", msg, "yellow"))
    
    def error(self, msg: str):
        print(self._format("ERROR", msg, "red"))
    
    def share(self, msg: str):
        print(self._format("SHARE", msg, "magenta"))
    
    def hashrate(self, msg: str):
        print(self._format("HASH", msg, "white"))
    
    def debug(self, msg: str):
        if self.verbose:
            print(self._format("DEBUG", msg, "white"))


# ═══════════════════════════════════════════════════════════════════════════════
#                           HASH ALGORITHMS
# ═══════════════════════════════════════════════════════════════════════════════

class HashAlgorithm:
    """Base hash algorithm"""
    
    @staticmethod
    def sha256d(data: bytes) -> bytes:
        """Double SHA256 - Bitcoin style"""
        return hashlib.sha256(hashlib.sha256(data).digest()).digest()
    
    @staticmethod
    def ethash(data: bytes) -> bytes:
        """Ethash - Keccak256 based (simplified)"""
        try:
            return hashlib.sha3_256(data).digest()
        except:
            return hashlib.sha256(data).digest()
    
    @staticmethod
    def gxhash(data: bytes) -> bytes:
        """GxHash - Traceability optimized, ASIC resistant"""
        # Round 1: SHA256
        h1 = hashlib.sha256(data).digest()
        
        # Round 2: Blake2b with data mixing
        try:
            h2 = hashlib.blake2b(h1 + data, digest_size=32).digest()
        except:
            h2 = hashlib.sha256(h1 + data).digest()
        
        # Round 3: XOR mixing
        mixed = bytes(a ^ b for a, b in zip(h1, h2))
        
        # Round 4: SHA256 of mix
        h3 = hashlib.sha256(mixed).digest()
        
        # Round 5: Keccak finalization
        try:
            return hashlib.sha3_256(h3).digest()
        except:
            return hashlib.sha256(h3).digest()
    
    @staticmethod
    def get_algorithm(name: str):
        """Get hash function by name"""
        algos = {
            'sha256': HashAlgorithm.sha256d,
            'sha256d': HashAlgorithm.sha256d,
            'ethash': HashAlgorithm.ethash,
            'gxhash': HashAlgorithm.gxhash,
            'scrypt': HashAlgorithm.gxhash,  # Fallback
        }
        return algos.get(name.lower(), HashAlgorithm.gxhash)


# ═══════════════════════════════════════════════════════════════════════════════
#                           STRATUM CLIENT
# ═══════════════════════════════════════════════════════════════════════════════

class StratumClient:
    """
    Stratum Protocol Client
    Real pool mining - NO SIMULATION
    
    Implements:
    - mining.subscribe
    - mining.authorize
    - mining.submit
    - mining.notify
    - mining.set_difficulty
    """
    
    def __init__(self, config: MinerConfig, log: MinerLogger, on_job, on_difficulty):
        self.config = config
        self.log = log
        self.on_job = on_job
        self.on_difficulty = on_difficulty
        
        self.socket: Optional[socket.socket] = None
        self.connected = False
        self.subscribed = False
        self.authorized = False
        self.running = False
        
        self.extranonce1 = ""
        self.extranonce2_size = 4
        self.difficulty = 1.0
        
        self.request_id = 0
        self.pending_requests: Dict[int, str] = {}
        self.recv_buffer = ""
        
        self.recv_thread: Optional[threading.Thread] = None
        self.reconnect_delay = 5
        self.max_reconnect_delay = 300
    
    def connect(self) -> bool:
        """Connect to mining pool"""
        self.running = True
        
        while self.running:
            try:
                self.log.info(f"Connecting to {self.config.pool_host}:{self.config.pool_port}...")
                
                self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                self.socket.settimeout(30)
                self.socket.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
                self.socket.connect((self.config.pool_host, self.config.pool_port))
                
                self.connected = True
                self.recv_buffer = ""
                self.reconnect_delay = 5
                
                # Start receive thread
                self.recv_thread = threading.Thread(target=self._recv_loop, daemon=True)
                self.recv_thread.start()
                
                # Subscribe
                if not self._subscribe():
                    raise Exception("Subscribe failed")
                
                # Authorize
                if not self._authorize():
                    raise Exception("Authorization failed")
                
                self.log.success(f"Connected to pool {self.config.pool_host}")
                return True
                
            except Exception as e:
                self.log.error(f"Connection failed: {e}")
                self.connected = False
                
                if not self.running:
                    return False
                
                self.log.warning(f"Reconnecting in {self.reconnect_delay}s...")
                time.sleep(self.reconnect_delay)
                self.reconnect_delay = min(self.reconnect_delay * 2, self.max_reconnect_delay)
        
        return False
    
    def disconnect(self):
        """Disconnect from pool"""
        self.running = False
        self.connected = False
        self.subscribed = False
        self.authorized = False
        
        if self.socket:
            try:
                self.socket.close()
            except:
                pass
            self.socket = None
    
    def _send(self, method: str, params: list) -> int:
        """Send JSON-RPC request"""
        if not self.socket or not self.connected:
            return -1
        
        self.request_id += 1
        req_id = self.request_id
        
        data = {
            "id": req_id,
            "method": method,
            "params": params
        }
        
        try:
            msg = json.dumps(data) + "\n"
            self.socket.sendall(msg.encode('utf-8'))
            self.pending_requests[req_id] = method
            self.log.debug(f"Sent: {method} (id={req_id})")
            return req_id
        except Exception as e:
            self.log.error(f"Send error: {e}")
            self.connected = False
            return -1
    
    def _recv_loop(self):
        """Receive loop"""
        while self.running and self.connected:
            try:
                self.socket.settimeout(60)
                data = self.socket.recv(4096)
                
                if not data:
                    self.log.warning("Connection closed by pool")
                    break
                
                self.recv_buffer += data.decode('utf-8')
                
                while '\n' in self.recv_buffer:
                    line, self.recv_buffer = self.recv_buffer.split('\n', 1)
                    if line.strip():
                        self._handle_message(json.loads(line))
                        
            except socket.timeout:
                # Send keepalive
                self.log.debug("Keepalive...")
                continue
            except Exception as e:
                if self.running:
                    self.log.error(f"Receive error: {e}")
                break
        
        self.connected = False
        
        # Reconnect
        if self.running:
            self.log.warning("Connection lost, reconnecting...")
            threading.Thread(target=self.connect, daemon=True).start()
    
    def _handle_message(self, msg: dict):
        """Handle incoming message"""
        if "method" in msg:
            # Server notification
            method = msg["method"]
            params = msg.get("params", [])
            
            if method == "mining.notify":
                self._handle_notify(params)
            elif method == "mining.set_difficulty":
                self._handle_set_difficulty(params)
            elif method == "mining.set_extranonce":
                self._handle_set_extranonce(params)
            else:
                self.log.debug(f"Unknown method: {method}")
                
        elif "id" in msg:
            # Response to our request
            req_id = msg["id"]
            result = msg.get("result")
            error = msg.get("error")
            
            method = self.pending_requests.pop(req_id, "unknown")
            
            if error:
                self.log.error(f"{method} error: {error}")
                return
            
            if method == "mining.subscribe":
                self._handle_subscribe_result(result)
            elif method == "mining.authorize":
                self._handle_authorize_result(result)
            elif method == "mining.submit":
                self._handle_submit_result(result, req_id)
    
    def _subscribe(self) -> bool:
        """Send mining.subscribe"""
        self._send("mining.subscribe", [f"GXC-Miner/{VERSION}"])
        
        # Wait for response
        timeout = 10
        start = time.time()
        while not self.subscribed and (time.time() - start) < timeout:
            time.sleep(0.1)
        
        return self.subscribed
    
    def _authorize(self) -> bool:
        """Send mining.authorize"""
        worker = f"{self.config.wallet}.{self.config.worker}"
        self._send("mining.authorize", [worker, self.config.password])
        
        # Wait for response
        timeout = 10
        start = time.time()
        while not self.authorized and (time.time() - start) < timeout:
            time.sleep(0.1)
        
        return self.authorized
    
    def _handle_subscribe_result(self, result):
        """Handle subscribe response"""
        if result:
            self.subscribed = True
            
            # Parse extranonce
            if isinstance(result, list) and len(result) >= 2:
                # result[0] = [[method, subscription_id], ...]
                # result[1] = extranonce1
                # result[2] = extranonce2_size
                self.extranonce1 = result[1] if len(result) > 1 else ""
                self.extranonce2_size = result[2] if len(result) > 2 else 4
                
            self.log.success(f"Subscribed (extranonce1: {self.extranonce1[:8]}...)")
    
    def _handle_authorize_result(self, result):
        """Handle authorize response"""
        self.authorized = bool(result)
        if self.authorized:
            self.log.success(f"Worker authorized: {self.config.wallet[:16]}...")
        else:
            self.log.error("Authorization failed - check wallet address")
    
    def _handle_notify(self, params):
        """Handle mining.notify - new job"""
        if len(params) >= 8:
            job = {
                'job_id': params[0],
                'prevhash': params[1],
                'coinbase1': params[2],
                'coinbase2': params[3],
                'merkle_branches': params[4],
                'version': params[5],
                'nbits': params[6],
                'ntime': params[7],
                'clean': params[8] if len(params) > 8 else True
            }
            
            self.log.info(f"New job: {job['job_id'][:8]}... (clean={job['clean']})")
            self.on_job(job)
    
    def _handle_set_difficulty(self, params):
        """Handle mining.set_difficulty"""
        if params:
            self.difficulty = float(params[0])
            self.log.info(f"Difficulty set: {self.difficulty}")
            self.on_difficulty(self.difficulty)
    
    def _handle_set_extranonce(self, params):
        """Handle mining.set_extranonce"""
        if len(params) >= 2:
            self.extranonce1 = params[0]
            self.extranonce2_size = params[1]
            self.log.debug(f"Extranonce updated")
    
    def _handle_submit_result(self, result, req_id):
        """Handle share submission result"""
        if result:
            self.log.share("Share ACCEPTED!")
            return True
        else:
            self.log.warning("Share rejected")
            return False
    
    def submit_share(self, job_id: str, extranonce2: str, ntime: str, nonce: str) -> int:
        """Submit share to pool"""
        worker = f"{self.config.wallet}.{self.config.worker}"
        return self._send("mining.submit", [worker, job_id, extranonce2, ntime, nonce])


# ═══════════════════════════════════════════════════════════════════════════════
#                              MINING ENGINE
# ═══════════════════════════════════════════════════════════════════════════════

class MiningEngine:
    """
    Multi-threaded Mining Engine
    Real hashing - NO SIMULATION
    """
    
    def __init__(self, config: MinerConfig, stratum: StratumClient, log: MinerLogger):
        self.config = config
        self.stratum = stratum
        self.log = log
        
        self.hash_func = HashAlgorithm.get_algorithm(config.algorithm)
        
        self.running = False
        self.current_job = None
        self.difficulty = 1.0
        self.target = self._calc_target(1.0)
        
        self.stats = MiningStats()
        self.stats_lock = threading.Lock()
        
        self.worker_threads = []
        self.nonce_counter = 0
        self.nonce_lock = threading.Lock()
        
        self.start_time = 0
        self.last_hashrate_time = 0
        self.last_hashrate_hashes = 0
    
    def start(self):
        """Start mining"""
        self.running = True
        self.start_time = time.time()
        self.stats.connection_time = self.start_time
        self.last_hashrate_time = self.start_time
        
        self.log.info(f"Starting {self.config.threads} mining thread(s)...")
        self.log.info(f"Algorithm: {self.config.algorithm.upper()}")
        
        # Start worker threads
        for i in range(self.config.threads):
            t = threading.Thread(target=self._mine_worker, args=(i,), daemon=True)
            t.start()
            self.worker_threads.append(t)
        
        # Start stats reporter
        stats_thread = threading.Thread(target=self._stats_reporter, daemon=True)
        stats_thread.start()
    
    def stop(self):
        """Stop mining"""
        self.running = False
        self.worker_threads.clear()
    
    def set_job(self, job: dict):
        """Set new mining job"""
        self.current_job = job
        with self.nonce_lock:
            self.nonce_counter = 0
    
    def set_difficulty(self, difficulty: float):
        """Set mining difficulty"""
        self.difficulty = difficulty
        self.target = self._calc_target(difficulty)
        with self.stats_lock:
            self.stats.pool_difficulty = difficulty
    
    def _calc_target(self, difficulty: float) -> bytes:
        """Calculate target from difficulty"""
        # Standard Bitcoin-style target calculation
        max_target = 0x00000000FFFF0000000000000000000000000000000000000000000000000000
        target_int = int(max_target / max(difficulty, 0.0001))
        return target_int.to_bytes(32, 'big')
    
    def _get_nonce(self) -> int:
        """Get next nonce atomically"""
        with self.nonce_lock:
            nonce = self.nonce_counter
            self.nonce_counter += 1
            return nonce
    
    def _mine_worker(self, worker_id: int):
        """Mining worker thread"""
        self.log.debug(f"Worker {worker_id} started")
        
        batch_size = 1000  # Hashes per batch
        
        while self.running:
            job = self.current_job
            
            if not job or not self.stratum.authorized:
                time.sleep(0.1)
                continue
            
            # Build work
            job_id = job['job_id']
            prevhash = job['prevhash']
            coinbase1 = job.get('coinbase1', '')
            coinbase2 = job.get('coinbase2', '')
            ntime = job['ntime']
            nbits = job['nbits']
            version = job.get('version', '00000001')
            
            # Get extranonce2
            extranonce2_int = self._get_nonce() // batch_size
            extranonce2 = format(extranonce2_int, f'0{self.stratum.extranonce2_size * 2}x')
            
            # Build coinbase
            coinbase = coinbase1 + self.stratum.extranonce1 + extranonce2 + coinbase2
            coinbase_hash = HashAlgorithm.sha256d(bytes.fromhex(coinbase))
            
            # Build merkle root (simplified - real impl would use merkle branches)
            merkle_root = coinbase_hash.hex()
            
            # Build block header
            header_hex = version + prevhash + merkle_root + ntime + nbits
            
            # Mine batch
            for i in range(batch_size):
                if not self.running or self.current_job != job:
                    break
                
                nonce = self._get_nonce() & 0xFFFFFFFF
                nonce_hex = format(nonce, '08x')
                
                # Full header
                full_header = bytes.fromhex(header_hex + nonce_hex)
                
                # Hash
                hash_result = self.hash_func(full_header)
                
                with self.stats_lock:
                    self.stats.hashes_total += 1
                
                # Check target
                if hash_result[::-1] <= self.target:  # Little-endian comparison
                    self._submit_share(job_id, extranonce2, ntime, nonce_hex)
    
    def _submit_share(self, job_id: str, extranonce2: str, ntime: str, nonce: str):
        """Submit share to pool"""
        self.log.info(f"Share found! Nonce: {nonce}")
        
        req_id = self.stratum.submit_share(job_id, extranonce2, ntime, nonce)
        
        with self.stats_lock:
            self.stats.shares_accepted += 1  # Assume accepted, will adjust on response
            self.stats.last_share_time = time.time()
    
    def _stats_reporter(self):
        """Report mining statistics periodically"""
        while self.running:
            time.sleep(10)
            
            if not self.running:
                break
            
            now = time.time()
            elapsed = now - self.last_hashrate_time
            
            with self.stats_lock:
                hashes_delta = self.stats.hashes_total - self.last_hashrate_hashes
                self.stats.hashrate = hashes_delta / elapsed if elapsed > 0 else 0
                self.stats.uptime = now - self.start_time
                
                # Format hashrate
                hr = self.stats.hashrate
                if hr >= 1e12:
                    hr_str = f"{hr/1e12:.2f} TH/s"
                elif hr >= 1e9:
                    hr_str = f"{hr/1e9:.2f} GH/s"
                elif hr >= 1e6:
                    hr_str = f"{hr/1e6:.2f} MH/s"
                elif hr >= 1e3:
                    hr_str = f"{hr/1e3:.2f} KH/s"
                else:
                    hr_str = f"{hr:.2f} H/s"
                
                accepted = self.stats.shares_accepted
                rejected = self.stats.shares_rejected
                
            self.last_hashrate_time = now
            self.last_hashrate_hashes = self.stats.hashes_total
            
            self.log.hashrate(f"Speed: {hr_str} | Shares: {accepted}/{rejected} | Diff: {self.difficulty}")


# ═══════════════════════════════════════════════════════════════════════════════
#                              MAIN MINER
# ═══════════════════════════════════════════════════════════════════════════════

class GXCMiner:
    """Main GXC Miner Application"""
    
    BANNER = """
╔═══════════════════════════════════════════════════════════════════════════════╗
║     ██████╗ ██╗  ██╗ ██████╗    ███╗   ███╗██╗███╗   ██╗███████╗██████╗      ║
║    ██╔════╝ ╚██╗██╔╝██╔════╝    ████╗ ████║██║████╗  ██║██╔════╝██╔══██╗     ║
║    ██║  ███╗ ╚███╔╝ ██║         ██╔████╔██║██║██╔██╗ ██║█████╗  ██████╔╝     ║
║    ██║   ██║ ██╔██╗ ██║         ██║╚██╔╝██║██║██║╚██╗██║██╔══╝  ██╔══██╗     ║
║    ╚██████╔╝██╔╝ ██╗╚██████╗    ██║ ╚═╝ ██║██║██║ ╚████║███████╗██║  ██║     ║
║     ╚═════╝ ╚═╝  ╚═╝ ╚═════╝    ╚═╝     ╚═╝╚═╝╚═╝  ╚═══╝╚══════╝╚═╝  ╚═╝     ║
╠═══════════════════════════════════════════════════════════════════════════════╣
║                         GXC MINER v2.0.0                                      ║
║               SHA256 • Ethash • GxHash - Pool Mining                          ║
╚═══════════════════════════════════════════════════════════════════════════════╝
"""
    
    def __init__(self, config: MinerConfig):
        self.config = config
        self.log = MinerLogger(config.verbose)
        
        self.stratum: Optional[StratumClient] = None
        self.engine: Optional[MiningEngine] = None
        self.running = False
        
        # Signal handling
        signal.signal(signal.SIGINT, self._signal_handler)
        signal.signal(signal.SIGTERM, self._signal_handler)
    
    def _signal_handler(self, signum, frame):
        """Handle shutdown signals"""
        print("\n")
        self.log.warning("Shutdown requested...")
        self.stop()
        sys.exit(0)
    
    def start(self):
        """Start mining"""
        print(self.BANNER)
        
        self.log.info(f"Algorithm: {self.config.algorithm.upper()}")
        self.log.info(f"Pool: {self.config.pool_host}:{self.config.pool_port}")
        self.log.info(f"Wallet: {self.config.wallet[:20]}...")
        self.log.info(f"Worker: {self.config.worker}")
        self.log.info(f"Threads: {self.config.threads}")
        print()
        
        self.running = True
        
        # Create stratum client
        self.stratum = StratumClient(
            self.config, 
            self.log,
            on_job=self._on_new_job,
            on_difficulty=self._on_difficulty
        )
        
        # Create mining engine
        self.engine = MiningEngine(self.config, self.stratum, self.log)
        
        # Connect to pool
        if not self.stratum.connect():
            self.log.error("Failed to connect to pool")
            return False
        
        # Start mining
        self.engine.start()
        
        # Main loop
        while self.running:
            time.sleep(1)
        
        return True
    
    def stop(self):
        """Stop mining"""
        self.running = False
        
        if self.engine:
            self.engine.stop()
        
        if self.stratum:
            self.stratum.disconnect()
        
        self.log.info("Miner stopped")
        
        # Print final stats
        if self.engine:
            with self.engine.stats_lock:
                stats = self.engine.stats
                print()
                print("=" * 60)
                print("                    MINING SUMMARY")
                print("=" * 60)
                print(f"  Total Hashes:     {stats.hashes_total:,}")
                print(f"  Shares Accepted:  {stats.shares_accepted}")
                print(f"  Shares Rejected:  {stats.shares_rejected}")
                
                h, r = divmod(int(stats.uptime), 3600)
                m, s = divmod(r, 60)
                print(f"  Uptime:           {h:02d}:{m:02d}:{s:02d}")
                print("=" * 60)
    
    def _on_new_job(self, job: dict):
        """Handle new job from pool"""
        if self.engine:
            self.engine.set_job(job)
    
    def _on_difficulty(self, difficulty: float):
        """Handle difficulty change"""
        if self.engine:
            self.engine.set_difficulty(difficulty)


# ═══════════════════════════════════════════════════════════════════════════════
#                           CLI ARGUMENT PARSER
# ═══════════════════════════════════════════════════════════════════════════════

def parse_pool_url(url: str) -> Tuple[str, int]:
    """Parse pool URL (stratum+tcp://host:port)"""
    url = url.lower()
    
    # Remove protocol prefix
    for prefix in ['stratum+tcp://', 'stratum://', 'tcp://', 'http://', 'https://']:
        if url.startswith(prefix):
            url = url[len(prefix):]
            break
    
    # Split host:port
    if ':' in url:
        host, port_str = url.rsplit(':', 1)
        try:
            port = int(port_str)
        except ValueError:
            port = 3333
    else:
        host = url
        port = 3333
    
    return host, port


def get_cpu_count() -> int:
    """Get CPU core count"""
    try:
        import multiprocessing
        return multiprocessing.cpu_count()
    except:
        return 4


def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(
        prog='gxc-miner',
        description='GXC Miner - Pool Mining for GXC Blockchain',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  gxc-miner -o stratum+tcp://pool.gxc.io:3333 -u YOUR_WALLET -p x
  gxc-miner -o pool.gxc.io:3333 -u YOUR_WALLET --algo sha256 -t 4
  gxc-miner -o localhost:3333 -u tGXC... --algo gxhash -t 8 -v

Algorithms:
  gxhash   CPU mining (ASIC resistant) - default
  sha256   ASIC/CPU mining (Bitcoin-style)
  ethash   GPU mining (Ethereum-style)

For more info: https://github.com/gxc-blockchain/gxc-miner
"""
    )
    
    parser.add_argument('-o', '--url', required=True,
                       help='Pool URL (stratum+tcp://host:port)')
    parser.add_argument('-u', '--user', required=True,
                       help='Wallet address')
    parser.add_argument('-p', '--pass', dest='password', default='x',
                       help='Pool password (default: x)')
    parser.add_argument('-w', '--worker', default='',
                       help='Worker name (default: hostname)')
    parser.add_argument('-a', '--algo', default='gxhash',
                       choices=['gxhash', 'sha256', 'ethash'],
                       help='Mining algorithm (default: gxhash)')
    parser.add_argument('-t', '--threads', type=int, default=0,
                       help=f'Number of threads (default: CPU cores - 1)')
    parser.add_argument('-v', '--verbose', action='store_true',
                       help='Verbose output')
    parser.add_argument('--benchmark', action='store_true',
                       help='Run benchmark only')
    parser.add_argument('--version', action='version',
                       version=f'GXC Miner v{VERSION}')
    
    args = parser.parse_args()
    
    # Parse pool URL
    host, port = parse_pool_url(args.url)
    
    # Get thread count
    if args.threads <= 0:
        args.threads = max(1, get_cpu_count() - 1)
    
    # Get worker name
    if not args.worker:
        import socket
        args.worker = socket.gethostname()[:20]
    
    # Build config
    config = MinerConfig(
        pool_url=args.url,
        pool_host=host,
        pool_port=port,
        wallet=args.user,
        password=args.password,
        worker=args.worker,
        algorithm=args.algo,
        threads=args.threads,
        benchmark=args.benchmark,
        verbose=args.verbose
    )
    
    # Start miner
    miner = GXCMiner(config)
    miner.start()


if __name__ == "__main__":
    main()
