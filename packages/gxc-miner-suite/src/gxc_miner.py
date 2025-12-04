#!/usr/bin/env python3
"""
╔═══════════════════════════════════════════════════════════════════════════════╗
║                                                                               ║
║     ██████╗ ██╗  ██╗ ██████╗    ███╗   ███╗██╗███╗   ██╗███████╗██████╗      ║
║    ██╔════╝ ╚██╗██╔╝██╔════╝    ████╗ ████║██║████╗  ██║██╔════╝██╔══██╗     ║
║    ██║  ███╗ ╚███╔╝ ██║         ██╔████╔██║██║██╔██╗ ██║█████╗  ██████╔╝     ║
║    ██║   ██║ ██╔██╗ ██║         ██║╚██╔╝██║██║██║╚██╗██║██╔══╝  ██╔══██╗     ║
║    ╚██████╔╝██╔╝ ██╗╚██████╗    ██║ ╚═╝ ██║██║██║ ╚████║███████╗██║  ██║     ║
║     ╚═════╝ ╚═╝  ╚═╝ ╚═════╝    ╚═╝     ╚═╝╚═╝╚═╝  ╚═══╝╚══════╝╚═╝  ╚═╝     ║
║                                                                               ║
║                    GXC MINER SUITE v2.0.0                                    ║
║          SHA256 • Ethash • GxHash - Real Blockchain Mining                   ║
║                                                                               ║
╚═══════════════════════════════════════════════════════════════════════════════╝

Copyright (c) 2024 GXC Blockchain - MIT License
Real blockchain connectivity - No simulations
"""

import socket
import threading
import json
import time
import hashlib
import struct
import os
import sys
import platform
import subprocess
import requests
from datetime import datetime
from typing import Dict, List, Optional, Callable, Any, Tuple
from dataclasses import dataclass, field
from queue import Queue
from abc import ABC, abstractmethod
import logging

# GUI Framework
import customtkinter as ctk
from tkinter import messagebox, filedialog
import tkinter as tk

# Plotting
import matplotlib
matplotlib.use("TkAgg")
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure

# System monitoring
try:
    import psutil
    HAS_PSUTIL = True
except ImportError:
    HAS_PSUTIL = False
    print("Warning: psutil not installed. Some features will be limited.")

# ═══════════════════════════════════════════════════════════════════════════════
#                              CONFIGURATION
# ═══════════════════════════════════════════════════════════════════════════════

VERSION = "2.0.0"
APP_NAME = "GXC Miner Suite"

# Network Configuration
NETWORKS = {
    "mainnet": {
        "name": "Mainnet",
        "rpc_url": "http://node.gxc.network:8332",
        "pool_host": "pool.gxc.network",
        "pool_port": 3333,
        "explorer": "https://explorer.gxc.io",
        "prefix": "GXC"
    },
    "testnet": {
        "name": "Testnet",
        "rpc_url": "http://localhost:8332",
        "pool_host": "localhost", 
        "pool_port": 3333,
        "explorer": "http://localhost:3000",
        "prefix": "tGXC"
    }
}

# Default network
ACTIVE_NETWORK = NETWORKS["testnet"]

# Algorithm configurations
ALGORITHMS = {
    "SHA256": {
        "name": "SHA256",
        "device": "ASIC",
        "description": "Double SHA256 - ASIC optimized",
        "color": "#ff6b6b"
    },
    "Ethash": {
        "name": "Ethash", 
        "device": "GPU",
        "description": "Ethash/ProgPoW - GPU optimized",
        "color": "#4ecdc4"
    },
    "GxHash": {
        "name": "GxHash",
        "device": "CPU",
        "description": "GxHash - Traceability optimized",
        "color": "#00ff9d"
    }
}

# Theme Colors
class Colors:
    BG = "#0a0e1a"
    PANEL = "#111827"
    SIDEBAR = "#1a1f2e"
    CARD = "#1f2937"
    ACCENT = "#00ff9d"
    ACCENT_DIM = "#008f58"
    ACCENT_GLOW = "#00ff9d33"
    TEXT = "#e0e6ed"
    TEXT_DIM = "#6b7280"
    TEXT_MUTED = "#4b5563"
    DANGER = "#ef4444"
    WARNING = "#f59e0b"
    SUCCESS = "#10b981"
    INFO = "#3b82f6"
    SHA256 = "#ff6b6b"
    ETHASH = "#4ecdc4"
    GXHASH = "#00ff9d"

# Logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s [%(levelname)s] %(message)s',
    datefmt='%H:%M:%S'
)
logger = logging.getLogger("GXC-Miner")


# ═══════════════════════════════════════════════════════════════════════════════
#                              DATA MODELS
# ═══════════════════════════════════════════════════════════════════════════════

@dataclass
class MiningDevice:
    """Mining device representation"""
    id: str
    name: str
    device_type: str  # CPU, GPU, ASIC
    algorithm: str
    vendor: str = "Unknown"
    model: str = ""
    memory: str = ""
    cores: int = 0
    frequency: str = ""
    ip_address: str = ""
    hashrate: float = 0.0
    temperature: float = 0.0
    power_usage: float = 0.0
    fan_speed: int = 0
    shares_accepted: int = 0
    shares_rejected: int = 0
    is_active: bool = False
    status: str = "Idle"
    efficiency: float = 0.0


@dataclass
class MiningJob:
    """Mining job from pool/node"""
    job_id: str
    height: int
    prev_hash: str
    merkle_root: str = ""
    timestamp: int = 0
    difficulty: float = 1.0
    target: str = ""
    nonce_range: Tuple[int, int] = (0, 0xFFFFFFFF)
    clean: bool = True


@dataclass
class MiningStats:
    """Mining statistics"""
    hashrate: float = 0.0
    hashrate_avg: float = 0.0
    hashrate_peak: float = 0.0
    shares_accepted: int = 0
    shares_rejected: int = 0
    shares_stale: int = 0
    blocks_found: int = 0
    uptime: int = 0
    efficiency: float = 0.0
    power_total: float = 0.0
    revenue_estimated: float = 0.0


@dataclass
class BlockchainInfo:
    """Blockchain state"""
    height: int = 0
    difficulty: float = 0.0
    network_hashrate: float = 0.0
    block_reward: float = 50.0
    last_block_time: str = ""
    connected: bool = False
    synced: bool = False
    peers: int = 0


# ═══════════════════════════════════════════════════════════════════════════════
#                           BLOCKCHAIN CLIENT
# ═══════════════════════════════════════════════════════════════════════════════

class GXCBlockchainClient:
    """
    GXC Blockchain JSON-RPC Client
    Real connection to blockchain node - NO SIMULATION
    """
    
    def __init__(self, rpc_url: str):
        self.rpc_url = rpc_url
        self.session = requests.Session()
        self.session.headers.update({'Content-Type': 'application/json'})
        self.request_id = 0
        self.connected = False
        self.last_error = None
        
    def _call(self, method: str, params: List = None) -> Any:
        """Execute RPC call"""
        if params is None:
            params = []
            
        self.request_id += 1
        payload = {
            "jsonrpc": "2.0",
            "method": method,
            "params": params,
            "id": self.request_id
        }
        
        try:
            response = self.session.post(self.rpc_url, json=payload, timeout=15)
            response.raise_for_status()
            data = response.json()
            
            if "error" in data and data["error"]:
                self.last_error = data["error"]
                raise Exception(f"RPC Error: {data['error']}")
            
            self.connected = True
            self.last_error = None
            return data.get("result")
            
        except requests.exceptions.RequestException as e:
            self.connected = False
            self.last_error = str(e)
            raise
    
    def test_connection(self) -> bool:
        """Test connection to node"""
        try:
            self._call("getblockchaininfo")
            return True
        except:
            return False
    
    def get_blockchain_info(self) -> dict:
        """Get blockchain state"""
        return self._call("getblockchaininfo")
    
    def get_block_template(self) -> dict:
        """Get block template for mining"""
        return self._call("getblocktemplate")
    
    def submit_block(self, block_data: dict) -> bool:
        """Submit mined block"""
        result = self._call("submitblock", [block_data])
        return result is None  # null = success
    
    def get_mining_info(self) -> dict:
        """Get mining information"""
        return self._call("getmininginfo")
    
    def get_difficulty(self) -> float:
        """Get current difficulty"""
        return self._call("getdifficulty")
    
    def get_network_hashrate(self) -> float:
        """Get network hashrate"""
        return self._call("getnetworkhashps")
    
    def get_balance(self, address: str) -> float:
        """Get wallet balance"""
        return self._call("getbalance", [address])
    
    def estimate_fee(self) -> dict:
        """Get fee estimation"""
        return self._call("estimatefee")
    
    def get_staking_info(self, address: str) -> dict:
        """Get staking info for address"""
        return self._call("getstakinginfo", [address])


# ═══════════════════════════════════════════════════════════════════════════════
#                            STRATUM CLIENT
# ═══════════════════════════════════════════════════════════════════════════════

class StratumClient:
    """
    Stratum Protocol Client for Pool Mining
    Real TCP connection - NO SIMULATION
    """
    
    def __init__(self, host: str, port: int, wallet: str, worker: str, callback: Callable):
        self.host = host
        self.port = port
        self.wallet = wallet
        self.worker = worker
        self.callback = callback
        
        self.socket: Optional[socket.socket] = None
        self.connected = False
        self.subscribed = False
        self.authorized = False
        self.running = False
        
        self.extranonce1 = ""
        self.extranonce2_size = 4
        self.difficulty = 1.0
        self.current_job: Optional[MiningJob] = None
        
        self.request_id = 0
        self.recv_buffer = ""
        self.recv_thread = None
        
    def connect(self) -> bool:
        """Connect to mining pool"""
        try:
            logger.info(f"Connecting to pool {self.host}:{self.port}...")
            
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.settimeout(30)
            self.socket.connect((self.host, self.port))
            
            self.connected = True
            self.running = True
            
            # Start receive thread
            self.recv_thread = threading.Thread(target=self._recv_loop, daemon=True)
            self.recv_thread.start()
            
            # Subscribe
            self._subscribe()
            time.sleep(0.5)
            
            # Authorize
            self._authorize()
            time.sleep(0.5)
            
            logger.info("Connected to pool successfully")
            self.callback("pool_connected", {"host": self.host, "port": self.port})
            return True
            
        except Exception as e:
            logger.error(f"Pool connection failed: {e}")
            self.connected = False
            self.callback("pool_error", str(e))
            return False
    
    def disconnect(self):
        """Disconnect from pool"""
        self.running = False
        self.connected = False
        if self.socket:
            try:
                self.socket.close()
            except:
                pass
        logger.info("Disconnected from pool")
        self.callback("pool_disconnected", None)
    
    def _send(self, data: dict):
        """Send JSON message"""
        if not self.socket or not self.connected:
            return
        try:
            msg = json.dumps(data) + "\n"
            self.socket.sendall(msg.encode('utf-8'))
        except Exception as e:
            logger.error(f"Send error: {e}")
            self.connected = False
    
    def _recv_loop(self):
        """Receive loop"""
        while self.running and self.connected:
            try:
                data = self.socket.recv(4096)
                if not data:
                    break
                
                self.recv_buffer += data.decode('utf-8')
                
                while '\n' in self.recv_buffer:
                    line, self.recv_buffer = self.recv_buffer.split('\n', 1)
                    if line.strip():
                        self._handle_message(json.loads(line))
                        
            except socket.timeout:
                continue
            except Exception as e:
                if self.running:
                    logger.error(f"Receive error: {e}")
                break
        
        self.connected = False
    
    def _handle_message(self, msg: dict):
        """Handle incoming message"""
        if "method" in msg:
            method = msg["method"]
            params = msg.get("params", [])
            
            if method == "mining.notify":
                self._handle_notify(params)
            elif method == "mining.set_difficulty":
                self._handle_difficulty(params)
            elif method == "mining.set_extranonce":
                self._handle_extranonce(params)
                
        elif "id" in msg:
            req_id = msg["id"]
            result = msg.get("result")
            error = msg.get("error")
            
            if error:
                logger.error(f"Pool error: {error}")
                self.callback("pool_error", error)
            elif req_id == 1:  # Subscribe
                self._handle_subscribe_result(result)
            elif req_id == 2:  # Authorize
                self._handle_auth_result(result)
            else:  # Share result
                self._handle_share_result(result, req_id)
    
    def _subscribe(self):
        """Send mining.subscribe"""
        self.request_id = 1
        self._send({
            "id": self.request_id,
            "method": "mining.subscribe",
            "params": [f"{APP_NAME}/{VERSION}"]
        })
    
    def _authorize(self):
        """Send mining.authorize"""
        self.request_id = 2
        worker_name = f"{self.wallet}.{self.worker}"
        self._send({
            "id": self.request_id,
            "method": "mining.authorize",
            "params": [worker_name, "x"]
        })
    
    def _handle_subscribe_result(self, result):
        if result:
            self.subscribed = True
            if len(result) >= 2:
                self.extranonce1 = result[1]
                self.extranonce2_size = result[2] if len(result) > 2 else 4
            logger.info(f"Subscribed: extranonce1={self.extranonce1[:16]}...")
            self.callback("subscribed", result)
    
    def _handle_auth_result(self, result):
        self.authorized = bool(result)
        if self.authorized:
            logger.info("Worker authorized")
            self.callback("authorized", True)
        else:
            logger.error("Authorization failed")
            self.callback("auth_failed", False)
    
    def _handle_notify(self, params):
        """Handle new job notification"""
        if len(params) >= 8:
            self.current_job = MiningJob(
                job_id=params[0],
                height=0,  # Not always provided
                prev_hash=params[1],
                merkle_root="",
                timestamp=int(params[7], 16) if params[7] else int(time.time()),
                difficulty=self.difficulty,
                target=params[6],
                clean=params[8] if len(params) > 8 else True
            )
            self.callback("new_job", self.current_job)
    
    def _handle_difficulty(self, params):
        """Handle difficulty change"""
        if params:
            self.difficulty = float(params[0])
            logger.info(f"Difficulty set: {self.difficulty}")
            self.callback("difficulty", self.difficulty)
    
    def _handle_extranonce(self, params):
        """Handle extranonce change"""
        if len(params) >= 2:
            self.extranonce1 = params[0]
            self.extranonce2_size = params[1]
    
    def _handle_share_result(self, result, req_id):
        """Handle share submission result"""
        if result:
            self.callback("share_accepted", req_id)
        else:
            self.callback("share_rejected", req_id)
    
    def submit_share(self, job_id: str, extranonce2: str, ntime: str, nonce: str) -> int:
        """Submit share to pool"""
        self.request_id += 1
        worker_name = f"{self.wallet}.{self.worker}"
        
        self._send({
            "id": self.request_id,
            "method": "mining.submit",
            "params": [worker_name, job_id, extranonce2, ntime, nonce]
        })
        
        return self.request_id


# ═══════════════════════════════════════════════════════════════════════════════
#                           MINING ALGORITHMS
# ═══════════════════════════════════════════════════════════════════════════════

class HashAlgorithm(ABC):
    """Base class for mining algorithms"""
    
    @abstractmethod
    def compute(self, data: bytes) -> bytes:
        pass
    
    @abstractmethod
    def get_name(self) -> str:
        pass


class SHA256Algorithm(HashAlgorithm):
    """Double SHA256 - ASIC optimized"""
    
    def compute(self, data: bytes) -> bytes:
        return hashlib.sha256(hashlib.sha256(data).digest()).digest()
    
    def get_name(self) -> str:
        return "SHA256"


class EthashAlgorithm(HashAlgorithm):
    """Ethash - GPU optimized (simplified)"""
    
    def compute(self, data: bytes) -> bytes:
        # Simplified Ethash using Keccak-256
        try:
            return hashlib.sha3_256(data).digest()
        except:
            return hashlib.sha256(data).digest()
    
    def get_name(self) -> str:
        return "Ethash"


class GxHashAlgorithm(HashAlgorithm):
    """GxHash - Traceability optimized, ASIC resistant"""
    
    def compute(self, data: bytes) -> bytes:
        # Round 1: SHA256
        h1 = hashlib.sha256(data).digest()
        
        # Round 2: Blake2b
        try:
            h2 = hashlib.blake2b(h1 + data, digest_size=32).digest()
        except:
            h2 = hashlib.sha256(h1 + data).digest()
        
        # Round 3: XOR mixing
        mixed = bytes(a ^ b for a, b in zip(h1, h2))
        
        # Round 4: Final hash
        h3 = hashlib.sha256(mixed).digest()
        
        # Round 5: SHA3 finalization
        try:
            return hashlib.sha3_256(h3).digest()
        except:
            return hashlib.sha256(h3).digest()
    
    def get_name(self) -> str:
        return "GxHash"


# ═══════════════════════════════════════════════════════════════════════════════
#                            MINING ENGINE
# ═══════════════════════════════════════════════════════════════════════════════

class MiningEngine:
    """
    Core Mining Engine
    Supports multiple algorithms and devices
    """
    
    def __init__(self, device: MiningDevice, algorithm: HashAlgorithm, 
                 stratum: Optional[StratumClient], callback: Callable):
        self.device = device
        self.algorithm = algorithm
        self.stratum = stratum
        self.callback = callback
        
        self.running = False
        self.paused = False
        self.current_job: Optional[MiningJob] = None
        self.target = bytes(32)
        self.difficulty = 1.0
        
        self.nonce = 0
        self.hashes_done = 0
        self.start_time = 0
        self.last_hash_count = 0
        self.last_hash_time = 0
        
        self.worker_threads: List[threading.Thread] = []
        self.num_threads = 1
        self.lock = threading.Lock()
        
    def start(self, num_threads: int = 1):
        """Start mining"""
        self.num_threads = num_threads
        self.running = True
        self.paused = False
        self.start_time = time.time()
        self.last_hash_time = time.time()
        self.hashes_done = 0
        
        for i in range(num_threads):
            t = threading.Thread(target=self._mine_loop, args=(i,), daemon=True)
            t.start()
            self.worker_threads.append(t)
        
        logger.info(f"Started {self.algorithm.get_name()} mining on {self.device.name} with {num_threads} threads")
        self.device.is_active = True
        self.device.status = "Mining"
        
    def stop(self):
        """Stop mining"""
        self.running = False
        self.worker_threads.clear()
        self.device.is_active = False
        self.device.status = "Stopped"
        logger.info(f"Stopped mining on {self.device.name}")
    
    def pause(self):
        self.paused = True
        self.device.status = "Paused"
        
    def resume(self):
        self.paused = False
        self.device.status = "Mining"
    
    def set_job(self, job: MiningJob):
        """Set new mining job"""
        with self.lock:
            self.current_job = job
            self.nonce = 0
        
    def set_difficulty(self, difficulty: float):
        """Set target difficulty"""
        self.difficulty = difficulty
        self.target = self._calc_target(difficulty)
    
    def _calc_target(self, difficulty: float) -> bytes:
        """Calculate target from difficulty"""
        max_target = 0x00000000FFFF0000000000000000000000000000000000000000000000000000
        target_int = int(max_target / max(difficulty, 0.001))
        return target_int.to_bytes(32, 'big')
    
    def _mine_loop(self, thread_id: int):
        """Mining loop for single thread"""
        local_nonce = thread_id * 0x10000000
        batch_size = self._get_batch_size()
        
        while self.running:
            if self.paused or not self.current_job:
                time.sleep(0.1)
                continue
            
            job = self.current_job
            
            # Build header
            header = self._build_header(job)
            
            # Mine batch
            for i in range(batch_size):
                if not self.running or self.paused:
                    break
                
                nonce = local_nonce + i
                nonce_bytes = struct.pack('<I', nonce & 0xFFFFFFFF)
                
                # Compute hash
                hash_result = self.algorithm.compute(header + nonce_bytes)
                
                with self.lock:
                    self.hashes_done += 1
                
                # Check target
                if hash_result < self.target:
                    self._submit_share(job, nonce, hash_result)
            
            local_nonce += batch_size
            
            # Update stats periodically
            if self.hashes_done % 50000 == 0:
                self._update_hashrate()
    
    def _build_header(self, job: MiningJob) -> bytes:
        """Build block header"""
        if self.stratum and self.stratum.extranonce1:
            # Pool mining header
            header = (
                job.prev_hash.encode() +
                self.stratum.extranonce1.encode() +
                struct.pack('<I', job.timestamp)
            )
        else:
            # Solo mining header  
            header = (
                job.prev_hash.encode() +
                struct.pack('<I', job.height) +
                struct.pack('<I', job.timestamp)
            )
        return header
    
    def _get_batch_size(self) -> int:
        """Get batch size for device type"""
        if self.device.device_type == "ASIC":
            return 1000000
        elif self.device.device_type == "GPU":
            return 100000
        else:
            return 10000
    
    def _submit_share(self, job: MiningJob, nonce: int, hash_result: bytes):
        """Submit found share"""
        logger.info(f"Share found! Nonce: {nonce}, Hash: {hash_result.hex()[:16]}...")
        
        if self.stratum:
            extranonce2 = struct.pack('<I', nonce).hex()
            ntime = hex(job.timestamp)[2:].zfill(8)
            nonce_hex = struct.pack('<I', nonce & 0xFFFFFFFF).hex()
            
            self.stratum.submit_share(job.job_id, extranonce2, ntime, nonce_hex)
        
        self.callback("share_found", {
            "device": self.device.name,
            "nonce": nonce,
            "hash": hash_result.hex()
        })
    
    def _update_hashrate(self):
        """Update hashrate calculation"""
        now = time.time()
        elapsed = now - self.last_hash_time
        
        if elapsed > 0:
            hashes_this_period = self.hashes_done - self.last_hash_count
            self.device.hashrate = hashes_this_period / elapsed
            
        self.last_hash_time = now
        self.last_hash_count = self.hashes_done
        
        self.callback("hashrate_update", self.device)


# ═══════════════════════════════════════════════════════════════════════════════
#                          DEVICE DETECTOR
# ═══════════════════════════════════════════════════════════════════════════════

class DeviceDetector:
    """Auto-detect mining hardware"""
    
    @staticmethod
    def detect_all() -> List[MiningDevice]:
        """Detect all mining devices"""
        devices = []
        
        # CPU
        cpu = DeviceDetector.detect_cpu()
        if cpu:
            devices.append(cpu)
        
        # GPUs
        gpus = DeviceDetector.detect_gpus()
        devices.extend(gpus)
        
        # ASICs
        asics = DeviceDetector.detect_asics()
        devices.extend(asics)
        
        return devices
    
    @staticmethod
    def detect_cpu() -> Optional[MiningDevice]:
        """Detect CPU"""
        try:
            name = platform.processor() or "Unknown CPU"
            cores = 1
            threads = 1
            freq = ""
            
            if HAS_PSUTIL:
                cores = psutil.cpu_count(logical=False) or 1
                threads = psutil.cpu_count(logical=True) or 1
                cpu_freq = psutil.cpu_freq()
                if cpu_freq:
                    freq = f"{cpu_freq.max:.0f} MHz"
            
            return MiningDevice(
                id="cpu_0",
                name=name[:35],
                device_type="CPU",
                algorithm="GxHash",
                vendor="System",
                cores=cores,
                frequency=freq,
                status="Ready"
            )
        except Exception as e:
            logger.error(f"CPU detection error: {e}")
            return None
    
    @staticmethod
    def detect_gpus() -> List[MiningDevice]:
        """Detect GPUs"""
        gpus = []
        
        # NVIDIA
        try:
            result = subprocess.run(
                ['nvidia-smi', '--query-gpu=name,memory.total,temperature.gpu,power.draw',
                 '--format=csv,noheader,nounits'],
                capture_output=True, text=True, timeout=5
            )
            if result.returncode == 0:
                for i, line in enumerate(result.stdout.strip().split('\n')):
                    if line:
                        parts = [p.strip() for p in line.split(',')]
                        gpus.append(MiningDevice(
                            id=f"nvidia_{i}",
                            name=parts[0] if parts else f"NVIDIA GPU {i}",
                            device_type="GPU",
                            algorithm="Ethash",
                            vendor="NVIDIA",
                            memory=f"{parts[1]} MB" if len(parts) > 1 else "",
                            temperature=float(parts[2]) if len(parts) > 2 and parts[2] not in ['[N/A]', 'N/A', ''] else 0,
                            power_usage=float(parts[3]) if len(parts) > 3 and parts[3] not in ['[N/A]', 'N/A', ''] else 0,
                            status="Ready"
                        ))
        except:
            pass
        
        # AMD
        try:
            result = subprocess.run(
                ['rocm-smi', '--showproductname'],
                capture_output=True, text=True, timeout=5
            )
            if result.returncode == 0:
                gpu_idx = 0
                for line in result.stdout.strip().split('\n'):
                    if 'GPU' in line:
                        name = line.split(':')[-1].strip() if ':' in line else f"AMD GPU {gpu_idx}"
                        gpus.append(MiningDevice(
                            id=f"amd_{gpu_idx}",
                            name=name,
                            device_type="GPU",
                            algorithm="Ethash",
                            vendor="AMD",
                            status="Ready"
                        ))
                        gpu_idx += 1
        except:
            pass
        
        return gpus
    
    @staticmethod
    def detect_asics() -> List[MiningDevice]:
        """Detect ASIC miners on network"""
        asics = []
        
        # Get local network
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            s.connect(("8.8.8.8", 80))
            local_ip = s.getsockname()[0]
            s.close()
        except:
            return asics
        
        if local_ip == "127.0.0.1":
            return asics
        
        # Scan network for ASIC miners
        network_prefix = '.'.join(local_ip.split('.')[:-1])
        
        for host_num in range(1, 20):  # Scan first 20 IPs
            ip = f"{network_prefix}.{host_num}"
            
            # Try cgminer API port
            try:
                sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                sock.settimeout(0.3)
                if sock.connect_ex((ip, 4028)) == 0:
                    # Query ASIC
                    sock.sendall(b'{"command":"summary"}')
                    response = sock.recv(4096)
                    sock.close()
                    
                    data = json.loads(response.decode().rstrip('\x00'))
                    if "SUMMARY" in data:
                        summary = data["SUMMARY"][0]
                        asics.append(MiningDevice(
                            id=f"asic_{ip.replace('.', '_')}",
                            name=summary.get("Type", "ASIC Miner"),
                            device_type="ASIC",
                            algorithm="SHA256",
                            vendor="Unknown",
                            ip_address=ip,
                            hashrate=summary.get("GHS 5s", 0) * 1e9,
                            status="Ready"
                        ))
                else:
                    sock.close()
            except:
                pass
        
        return asics


# ═══════════════════════════════════════════════════════════════════════════════
#                              GUI APPLICATION
# ═══════════════════════════════════════════════════════════════════════════════

ctk.set_appearance_mode("Dark")
ctk.set_default_color_theme("green")


class GXCMinerApp(ctk.CTk):
    """Main GXC Miner Application"""
    
    def __init__(self):
        super().__init__()
        
        self.title(f"{APP_NAME} v{VERSION}")
        self.geometry("1600x1000")
        self.configure(fg_color=Colors.BG)
        self.minsize(1200, 800)
        
        # State
        self.devices: List[MiningDevice] = []
        self.engines: Dict[str, MiningEngine] = {}
        self.stratum: Optional[StratumClient] = None
        self.blockchain: Optional[GXCBlockchainClient] = None
        
        self.is_mining = False
        self.selected_algorithm = "GxHash"
        self.start_time = None
        
        self.stats = MiningStats()
        self.chain_info = BlockchainInfo()
        self.hashrate_history: List[float] = []
        
        # Config
        self.wallet_address = ""
        self.worker_name = platform.node()[:20]
        self.num_threads = max(1, (psutil.cpu_count(logical=False) or 1) - 1) if HAS_PSUTIL else 1
        
        # Layout
        self.grid_columnconfigure(1, weight=1)
        self.grid_rowconfigure(0, weight=1)
        
        self._setup_sidebar()
        self._setup_main()
        self._setup_statusbar()
        
        # Initialize
        self.after(100, self._initialize)
        self.after(1000, self._update_loop)
    
    def _initialize(self):
        """Initialize application"""
        self._log("GXC Miner Suite initialized", "INFO")
        self._log(f"Version: {VERSION}", "INFO")
        self._log(f"System: {platform.system()} {platform.release()}", "INFO")
        
        # Connect blockchain
        self._connect_blockchain()
        
        # Detect devices
        self.after(500, self._scan_devices)
    
    def _connect_blockchain(self):
        """Connect to blockchain"""
        self._log(f"Connecting to {ACTIVE_NETWORK['rpc_url']}...", "INFO")
        
        self.blockchain = GXCBlockchainClient(ACTIVE_NETWORK["rpc_url"])
        
        if self.blockchain.test_connection():
            self._log("✓ Connected to GXC blockchain", "SUCCESS")
            self.chain_info.connected = True
            self._update_chain_info()
        else:
            self._log("✗ Blockchain connection failed", "ERROR")
            self._log("  Will use pool data instead", "WARNING")
    
    def _update_chain_info(self):
        """Update blockchain info"""
        if not self.blockchain or not self.blockchain.connected:
            return
        
        try:
            info = self.blockchain.get_blockchain_info()
            self.chain_info.height = info.get("blocks", 0)
            self.chain_info.difficulty = info.get("difficulty", 0)
            self.chain_info.block_reward = info.get("block_reward", 50.0)
            self.chain_info.last_block_time = datetime.now().strftime("%H:%M:%S")
            
            # Update UI
            self.lbl_height.configure(text=f"{self.chain_info.height:,}")
            self.lbl_difficulty.configure(text=f"{self.chain_info.difficulty:,.0f}")
            self.lbl_reward.configure(text=f"{self.chain_info.block_reward:.2f} GXC")
            
            try:
                hr = self.blockchain.get_network_hashrate()
                self.chain_info.network_hashrate = hr
                self.lbl_nethash.configure(text=self._format_hashrate(hr))
            except:
                pass
                
        except Exception as e:
            logger.debug(f"Chain info update error: {e}")
    
    def _setup_sidebar(self):
        """Setup sidebar with stats"""
        sidebar = ctk.CTkFrame(self, width=320, corner_radius=0, fg_color=Colors.SIDEBAR)
        sidebar.grid(row=0, column=0, rowspan=2, sticky="nsew")
        sidebar.grid_propagate(False)
        
        # Logo
        logo_frame = ctk.CTkFrame(sidebar, fg_color="transparent")
        logo_frame.pack(pady=(25, 15))
        
        ctk.CTkLabel(logo_frame, text="GXC", font=("Impact", 56), 
                    text_color=Colors.ACCENT).pack()
        ctk.CTkLabel(logo_frame, text="MINER SUITE", font=("Arial", 14, "bold"),
                    text_color=Colors.TEXT_DIM).pack()
        ctk.CTkLabel(logo_frame, text=f"v{VERSION}", font=("Arial", 10),
                    text_color=Colors.TEXT_MUTED).pack()
        
        # Algorithm selector
        algo_frame = ctk.CTkFrame(sidebar, fg_color=Colors.PANEL, corner_radius=12)
        algo_frame.pack(fill="x", padx=15, pady=10)
        
        ctk.CTkLabel(algo_frame, text="⚡ ALGORITHM", font=("Arial", 11, "bold"),
                    text_color=Colors.ACCENT).pack(pady=(12, 8))
        
        self.algo_var = ctk.StringVar(value="GxHash")
        for algo, info in ALGORITHMS.items():
            rb = ctk.CTkRadioButton(
                algo_frame, text=f"{algo} ({info['device']})",
                variable=self.algo_var, value=algo,
                font=("Arial", 11),
                fg_color=info["color"],
                hover_color=info["color"],
                command=self._on_algo_change
            )
            rb.pack(anchor="w", padx=20, pady=3)
        
        # Mining stats
        stats_frame = ctk.CTkFrame(sidebar, fg_color=Colors.PANEL, corner_radius=12)
        stats_frame.pack(fill="x", padx=15, pady=10)
        
        ctk.CTkLabel(stats_frame, text="📊 MINING STATS", font=("Arial", 11, "bold"),
                    text_color=Colors.ACCENT).pack(pady=(12, 8))
        
        self.lbl_hashrate = self._stat_row(stats_frame, "Hashrate", "0 H/s")
        self.lbl_avg_hash = self._stat_row(stats_frame, "Average", "0 H/s")
        self.lbl_shares_a = self._stat_row(stats_frame, "Accepted", "0")
        self.lbl_shares_r = self._stat_row(stats_frame, "Rejected", "0")
        self.lbl_efficiency = self._stat_row(stats_frame, "Efficiency", "0%")
        self.lbl_uptime = self._stat_row(stats_frame, "Uptime", "00:00:00")
        
        # Network stats
        net_frame = ctk.CTkFrame(sidebar, fg_color=Colors.PANEL, corner_radius=12)
        net_frame.pack(fill="x", padx=15, pady=10)
        
        ctk.CTkLabel(net_frame, text="🔗 GXC NETWORK", font=("Arial", 11, "bold"),
                    text_color=Colors.INFO).pack(pady=(12, 8))
        
        self.lbl_connection = self._stat_row(net_frame, "Status", "Connecting...")
        self.lbl_height = self._stat_row(net_frame, "Block Height", "...")
        self.lbl_difficulty = self._stat_row(net_frame, "Difficulty", "...")
        self.lbl_nethash = self._stat_row(net_frame, "Network Hash", "...")
        self.lbl_reward = self._stat_row(net_frame, "Block Reward", "...")
        
        # Control buttons
        btn_frame = ctk.CTkFrame(sidebar, fg_color="transparent")
        btn_frame.pack(fill="x", padx=15, pady=20)
        
        self.btn_start = ctk.CTkButton(
            btn_frame,
            text="▶ START MINING",
            command=self._toggle_mining,
            font=("Arial", 15, "bold"),
            fg_color=Colors.ACCENT,
            hover_color=Colors.ACCENT_DIM,
            height=55,
            corner_radius=12
        )
        self.btn_start.pack(fill="x", pady=5)
        
        ctk.CTkButton(
            btn_frame,
            text="🔍 SCAN DEVICES",
            command=self._scan_devices,
            font=("Arial", 12),
            fg_color=Colors.PANEL,
            hover_color=Colors.CARD,
            height=42,
            corner_radius=10
        ).pack(fill="x", pady=5)
        
        ctk.CTkButton(
            btn_frame,
            text="🔄 REFRESH",
            command=self._refresh_all,
            font=("Arial", 12),
            fg_color=Colors.PANEL,
            hover_color=Colors.CARD,
            height=42,
            corner_radius=10
        ).pack(fill="x", pady=5)
    
    def _stat_row(self, parent, label: str, value: str) -> ctk.CTkLabel:
        """Create stat row"""
        row = ctk.CTkFrame(parent, fg_color="transparent")
        row.pack(fill="x", padx=12, pady=4)
        
        ctk.CTkLabel(row, text=label, font=("Arial", 10), 
                    text_color=Colors.TEXT_DIM).pack(side="left")
        lbl = ctk.CTkLabel(row, text=value, font=("Arial", 10, "bold"),
                          text_color=Colors.TEXT)
        lbl.pack(side="right")
        return lbl
    
    def _setup_main(self):
        """Setup main content area"""
        self.tabview = ctk.CTkTabview(
            self,
            fg_color=Colors.BG,
            segmented_button_fg_color=Colors.PANEL,
            segmented_button_selected_color=Colors.ACCENT,
            segmented_button_selected_hover_color=Colors.ACCENT_DIM,
            corner_radius=12
        )
        self.tabview.grid(row=0, column=1, padx=20, pady=(20, 10), sticky="nsew")
        
        self.tab_dashboard = self.tabview.add("📊 Dashboard")
        self.tab_devices = self.tabview.add("💻 Devices")
        self.tab_config = self.tabview.add("⚙️ Config")
        self.tab_wallet = self.tabview.add("💰 Wallet")
        self.tab_logs = self.tabview.add("📋 Logs")
        
        self._setup_dashboard()
        self._setup_devices()
        self._setup_config()
        self._setup_wallet()
        self._setup_logs()
    
    def _setup_dashboard(self):
        """Setup dashboard with chart"""
        # Chart
        chart_frame = ctk.CTkFrame(self.tab_dashboard, fg_color=Colors.PANEL, corner_radius=12)
        chart_frame.pack(fill="both", expand=True, padx=10, pady=10)
        
        header = ctk.CTkFrame(chart_frame, fg_color="transparent")
        header.pack(fill="x", padx=15, pady=10)
        
        ctk.CTkLabel(header, text="📈 HASHRATE MONITOR", font=("Arial", 16, "bold"),
                    text_color=Colors.ACCENT).pack(side="left")
        
        self.lbl_current_hash = ctk.CTkLabel(header, text="0 H/s", 
                                             font=("Arial", 28, "bold"))
        self.lbl_current_hash.pack(side="right", padx=20)
        
        # Matplotlib chart
        self.fig = Figure(figsize=(12, 5), facecolor=Colors.PANEL, dpi=100)
        self.ax = self.fig.add_subplot(111)
        self.ax.set_facecolor(Colors.BG)
        self.ax.tick_params(colors=Colors.TEXT_DIM, labelsize=9)
        self.ax.grid(True, alpha=0.2, color='#333')
        self.ax.set_xlabel('Time (s)', color=Colors.TEXT_DIM)
        self.ax.set_ylabel('Hashrate', color=Colors.TEXT_DIM)
        
        for spine in self.ax.spines.values():
            spine.set_color('#333')
        
        self.line, = self.ax.plot([], [], color=Colors.ACCENT, linewidth=2)
        
        canvas = FigureCanvasTkAgg(self.fig, chart_frame)
        canvas.draw()
        canvas.get_tk_widget().pack(fill="both", expand=True, padx=10, pady=(0, 10))
        self.chart_canvas = canvas
        
        # Stats cards
        cards = ctk.CTkFrame(self.tab_dashboard, fg_color="transparent")
        cards.pack(fill="x", padx=10, pady=10)
        
        for i in range(4):
            cards.grid_columnconfigure(i, weight=1)
        
        card_data = [
            ("⚡ Current", "0 H/s", Colors.ACCENT),
            ("📈 Peak", "0 H/s", Colors.INFO),
            ("💎 Shares/hr", "0", Colors.SUCCESS),
            ("💰 Est. Earnings", "0 GXC/day", Colors.WARNING)
        ]
        
        for i, (title, value, color) in enumerate(card_data):
            card = ctk.CTkFrame(cards, fg_color=Colors.PANEL, corner_radius=10)
            card.grid(row=0, column=i, padx=5, pady=5, sticky="nsew")
            
            ctk.CTkLabel(card, text=title, font=("Arial", 11), 
                        text_color=color).pack(pady=(15, 5))
            ctk.CTkLabel(card, text=value, font=("Arial", 18, "bold")).pack(pady=(0, 15))
    
    def _setup_devices(self):
        """Setup devices list"""
        # Header
        header = ctk.CTkFrame(self.tab_devices, fg_color=Colors.PANEL, corner_radius=10, height=50)
        header.pack(fill="x", padx=10, pady=(10, 5))
        header.pack_propagate(False)
        
        cols = ["Device", "Type", "Algorithm", "Hashrate", "Temp", "Power", "Shares", "Status"]
        for col in cols:
            ctk.CTkLabel(header, text=col, font=("Arial", 11, "bold"),
                        text_color=Colors.TEXT_DIM).pack(side="left", expand=True, pady=12)
        
        # Device list
        self.device_list = ctk.CTkScrollableFrame(self.tab_devices, fg_color="transparent")
        self.device_list.pack(fill="both", expand=True, padx=10, pady=5)
    
    def _setup_config(self):
        """Setup configuration"""
        config_frame = ctk.CTkFrame(self.tab_config, fg_color=Colors.PANEL, corner_radius=12)
        config_frame.pack(fill="both", expand=True, padx=20, pady=20)
        
        ctk.CTkLabel(config_frame, text="⚙️ MINING CONFIGURATION",
                    font=("Arial", 20, "bold"), text_color=Colors.ACCENT).pack(pady=(30, 20))
        
        # Network
        net_frame = ctk.CTkFrame(config_frame, fg_color="transparent")
        net_frame.pack(pady=10)
        
        ctk.CTkLabel(net_frame, text="Network:", width=150, anchor="e",
                    font=("Arial", 12)).pack(side="left", padx=10)
        self.network_var = ctk.StringVar(value="Testnet")
        ctk.CTkOptionMenu(net_frame, values=["Testnet", "Mainnet"],
                         variable=self.network_var, width=400,
                         command=self._on_network_change).pack(side="left", padx=10)
        
        # Pool
        pool_frame = ctk.CTkFrame(config_frame, fg_color="transparent")
        pool_frame.pack(pady=10)
        
        ctk.CTkLabel(pool_frame, text="Pool Address:", width=150, anchor="e",
                    font=("Arial", 12)).pack(side="left", padx=10)
        self.entry_pool = ctk.CTkEntry(pool_frame, width=400,
                                       placeholder_text=f"{ACTIVE_NETWORK['pool_host']}:{ACTIVE_NETWORK['pool_port']}")
        self.entry_pool.insert(0, f"{ACTIVE_NETWORK['pool_host']}:{ACTIVE_NETWORK['pool_port']}")
        self.entry_pool.pack(side="left", padx=10)
        
        # Wallet
        wallet_frame = ctk.CTkFrame(config_frame, fg_color="transparent")
        wallet_frame.pack(pady=10)
        
        ctk.CTkLabel(wallet_frame, text="Wallet Address:", width=150, anchor="e",
                    font=("Arial", 12)).pack(side="left", padx=10)
        self.entry_wallet = ctk.CTkEntry(wallet_frame, width=400,
                                        placeholder_text="Enter your GXC wallet address")
        self.entry_wallet.pack(side="left", padx=10)
        
        # Worker
        worker_frame = ctk.CTkFrame(config_frame, fg_color="transparent")
        worker_frame.pack(pady=10)
        
        ctk.CTkLabel(worker_frame, text="Worker Name:", width=150, anchor="e",
                    font=("Arial", 12)).pack(side="left", padx=10)
        self.entry_worker = ctk.CTkEntry(worker_frame, width=400)
        self.entry_worker.insert(0, self.worker_name)
        self.entry_worker.pack(side="left", padx=10)
        
        # Threads
        thread_frame = ctk.CTkFrame(config_frame, fg_color="transparent")
        thread_frame.pack(pady=10)
        
        max_threads = psutil.cpu_count(logical=True) if HAS_PSUTIL else 8
        ctk.CTkLabel(thread_frame, text="Mining Threads:", width=150, anchor="e",
                    font=("Arial", 12)).pack(side="left", padx=10)
        self.slider_threads = ctk.CTkSlider(thread_frame, from_=1, to=max_threads,
                                           number_of_steps=max_threads-1, width=300,
                                           command=self._on_threads_change)
        self.slider_threads.set(self.num_threads)
        self.slider_threads.pack(side="left", padx=10)
        self.lbl_threads = ctk.CTkLabel(thread_frame, text=str(self.num_threads), width=50)
        self.lbl_threads.pack(side="left")
        
        # Buttons
        btn_frame = ctk.CTkFrame(config_frame, fg_color="transparent")
        btn_frame.pack(pady=30)
        
        ctk.CTkButton(btn_frame, text="💾 SAVE CONFIG", font=("Arial", 14, "bold"),
                     fg_color=Colors.ACCENT, hover_color=Colors.ACCENT_DIM,
                     height=45, width=200, command=self._save_config).pack(side="left", padx=10)
        
        ctk.CTkButton(btn_frame, text="🧪 TEST CONNECTION", font=("Arial", 14),
                     fg_color=Colors.INFO, hover_color="#2563eb",
                     height=45, width=200, command=self._test_connection).pack(side="left", padx=10)
    
    def _setup_wallet(self):
        """Setup wallet view"""
        wallet_frame = ctk.CTkFrame(self.tab_wallet, fg_color=Colors.PANEL, corner_radius=12)
        wallet_frame.pack(fill="both", expand=True, padx=20, pady=20)
        
        ctk.CTkLabel(wallet_frame, text="💰 WALLET & EARNINGS",
                    font=("Arial", 20, "bold"), text_color=Colors.ACCENT).pack(pady=(30, 20))
        
        # Balance card
        balance_card = ctk.CTkFrame(wallet_frame, fg_color=Colors.BG, corner_radius=12)
        balance_card.pack(fill="x", padx=40, pady=20)
        
        ctk.CTkLabel(balance_card, text="Current Balance", font=("Arial", 14),
                    text_color=Colors.TEXT_DIM).pack(pady=(25, 5))
        self.lbl_balance = ctk.CTkLabel(balance_card, text="0.00000000 GXC",
                                        font=("Arial", 36, "bold"), text_color=Colors.ACCENT)
        self.lbl_balance.pack(pady=(0, 25))
        
        # Stats grid
        stats_grid = ctk.CTkFrame(wallet_frame, fg_color="transparent")
        stats_grid.pack(fill="x", padx=40, pady=20)
        
        for i in range(3):
            stats_grid.grid_columnconfigure(i, weight=1)
        
        stat_items = [
            ("Pending", "0.00000000 GXC"),
            ("Total Earned", "0.00000000 GXC"),
            ("Last Payout", "Never")
        ]
        
        for i, (title, value) in enumerate(stat_items):
            card = ctk.CTkFrame(stats_grid, fg_color=Colors.BG, corner_radius=10)
            card.grid(row=0, column=i, padx=10, pady=5, sticky="nsew")
            ctk.CTkLabel(card, text=title, font=("Arial", 11),
                        text_color=Colors.TEXT_DIM).pack(pady=(15, 5))
            ctk.CTkLabel(card, text=value, font=("Arial", 14, "bold")).pack(pady=(0, 15))
        
        # Refresh button
        ctk.CTkButton(wallet_frame, text="🔄 REFRESH BALANCE", font=("Arial", 14),
                     fg_color=Colors.ACCENT, hover_color=Colors.ACCENT_DIM,
                     height=45, width=200, command=self._refresh_wallet).pack(pady=20)
    
    def _setup_logs(self):
        """Setup logs console"""
        self.console = ctk.CTkTextbox(self.tab_logs, font=("Consolas", 11),
                                      fg_color="#000", text_color=Colors.TEXT,
                                      corner_radius=12)
        self.console.pack(fill="both", expand=True, padx=10, pady=10)
        
        btn_frame = ctk.CTkFrame(self.tab_logs, fg_color="transparent")
        btn_frame.pack(fill="x", padx=10, pady=(0, 10))
        
        ctk.CTkButton(btn_frame, text="Clear", fg_color=Colors.PANEL, width=100,
                     command=lambda: self.console.delete("1.0", "end")).pack(side="left", padx=5)
        ctk.CTkButton(btn_frame, text="Export", fg_color=Colors.PANEL, width=100,
                     command=self._export_logs).pack(side="left", padx=5)
    
    def _setup_statusbar(self):
        """Setup status bar"""
        self.statusbar = ctk.CTkFrame(self, height=32, fg_color=Colors.PANEL, corner_radius=0)
        self.statusbar.grid(row=1, column=1, sticky="ew", padx=20, pady=(0, 10))
        
        self.lbl_status = ctk.CTkLabel(self.statusbar, text="Ready", font=("Arial", 10),
                                       text_color=Colors.TEXT_DIM)
        self.lbl_status.pack(side="left", padx=15, pady=6)
        
        self.lbl_time = ctk.CTkLabel(self.statusbar, text="", font=("Arial", 10),
                                     text_color=Colors.TEXT_MUTED)
        self.lbl_time.pack(side="right", padx=15, pady=6)
    
    # ═══════════════════════════════════════════════════════════════════════════
    #                              ACTIONS
    # ═══════════════════════════════════════════════════════════════════════════
    
    def _scan_devices(self):
        """Scan for mining devices"""
        self._log("🔍 Scanning for mining devices...", "INFO")
        
        self.devices = DeviceDetector.detect_all()
        
        if not self.devices:
            self._log("⚠ No mining devices detected", "WARNING")
        else:
            for d in self.devices:
                self._log(f"✓ Found: {d.name} ({d.device_type}) - {d.algorithm}", "SUCCESS")
            self._log(f"📊 Total: {len(self.devices)} device(s)", "SUCCESS")
        
        self._refresh_device_list()
    
    def _refresh_device_list(self):
        """Refresh device list UI"""
        for w in self.device_list.winfo_children():
            w.destroy()
        
        for device in self.devices:
            row = ctk.CTkFrame(self.device_list, fg_color=Colors.PANEL, corner_radius=8, height=55)
            row.pack(fill="x", pady=3)
            row.pack_propagate(False)
            
            # Device info
            ctk.CTkLabel(row, text=device.name[:28], font=("Arial", 11, "bold"),
                        width=200, anchor="w").pack(side="left", padx=10, pady=15)
            ctk.CTkLabel(row, text=device.device_type, width=80,
                        text_color=Colors.TEXT_DIM).pack(side="left", padx=5)
            
            algo_color = ALGORITHMS.get(device.algorithm, {}).get("color", Colors.TEXT)
            ctk.CTkLabel(row, text=device.algorithm, width=100,
                        text_color=algo_color).pack(side="left", padx=5)
            
            ctk.CTkLabel(row, text=self._format_hashrate(device.hashrate),
                        width=120).pack(side="left", padx=5)
            ctk.CTkLabel(row, text=f"{device.temperature:.0f}°C" if device.temperature else "N/A",
                        width=80, text_color=Colors.TEXT_DIM).pack(side="left", padx=5)
            ctk.CTkLabel(row, text=f"{device.power_usage:.0f}W" if device.power_usage else "N/A",
                        width=80, text_color=Colors.TEXT_DIM).pack(side="left", padx=5)
            ctk.CTkLabel(row, text=f"{device.shares_accepted}/{device.shares_rejected}",
                        width=100).pack(side="left", padx=5)
            
            status_color = Colors.SUCCESS if device.is_active else (
                Colors.WARNING if device.status == "Ready" else Colors.TEXT_DIM)
            ctk.CTkLabel(row, text=f"● {device.status}", text_color=status_color,
                        width=100).pack(side="left", padx=5)
    
    def _toggle_mining(self):
        """Start/stop mining"""
        if not self.is_mining:
            self._start_mining()
        else:
            self._stop_mining()
    
    def _start_mining(self):
        """Start mining"""
        # Validate
        wallet = self.entry_wallet.get().strip()
        if not wallet:
            messagebox.showwarning("Wallet Required", "Please enter your GXC wallet address!")
            return
        
        if not self.devices:
            messagebox.showwarning("No Devices", "Please scan for devices first!")
            return
        
        self.wallet_address = wallet
        self.worker_name = self.entry_worker.get().strip() or platform.node()[:20]
        self.num_threads = int(self.slider_threads.get())
        self.selected_algorithm = self.algo_var.get()
        
        self.is_mining = True
        self.start_time = time.time()
        self.btn_start.configure(text="⏹ STOP MINING", fg_color=Colors.DANGER)
        
        self._log(f"▶ Starting {self.selected_algorithm} mining...", "INFO")
        self._log(f"  Wallet: {wallet[:20]}...", "INFO")
        self._log(f"  Threads: {self.num_threads}", "INFO")
        
        # Connect pool
        pool_str = self.entry_pool.get().strip()
        if ':' in pool_str:
            host, port = pool_str.rsplit(':', 1)
            port = int(port)
        else:
            host = ACTIVE_NETWORK["pool_host"]
            port = ACTIVE_NETWORK["pool_port"]
        
        self.stratum = StratumClient(host, port, wallet, self.worker_name, self._stratum_callback)
        
        threading.Thread(target=self._connect_and_mine, daemon=True).start()
    
    def _connect_and_mine(self):
        """Connect to pool and start mining"""
        if self.stratum.connect():
            self._log(f"✓ Connected to pool {self.stratum.host}:{self.stratum.port}", "SUCCESS")
            
            # Get algorithm
            algo_map = {
                "SHA256": SHA256Algorithm(),
                "Ethash": EthashAlgorithm(),
                "GxHash": GxHashAlgorithm()
            }
            algorithm = algo_map.get(self.selected_algorithm, GxHashAlgorithm())
            
            # Start engines for matching devices
            for device in self.devices:
                if device.algorithm == self.selected_algorithm or self.selected_algorithm == "GxHash":
                    engine = MiningEngine(device, algorithm, self.stratum, self._mining_callback)
                    threads = self.num_threads if device.device_type == "CPU" else 1
                    engine.start(threads)
                    self.engines[device.id] = engine
                    self._log(f"⚡ Started on {device.name}", "SUCCESS")
        else:
            self._log("✗ Failed to connect to pool", "ERROR")
            self.after(0, self._stop_mining)
    
    def _stop_mining(self):
        """Stop mining"""
        self.is_mining = False
        self.btn_start.configure(text="▶ START MINING", fg_color=Colors.ACCENT)
        
        self._log("⏹ Stopping mining...", "WARNING")
        
        for engine in self.engines.values():
            engine.stop()
        self.engines.clear()
        
        if self.stratum:
            self.stratum.disconnect()
            self.stratum = None
        
        for d in self.devices:
            d.is_active = False
            d.status = "Idle"
        
        self._refresh_device_list()
        self._log("✓ Mining stopped", "INFO")
    
    def _stratum_callback(self, event: str, data: Any):
        """Handle stratum events"""
        if event == "new_job":
            for engine in self.engines.values():
                engine.set_job(data)
        elif event == "difficulty":
            for engine in self.engines.values():
                engine.set_difficulty(data)
        elif event == "share_accepted":
            self.stats.shares_accepted += 1
            self._log(f"✓ Share accepted! Total: {self.stats.shares_accepted}", "SUCCESS")
        elif event == "share_rejected":
            self.stats.shares_rejected += 1
            self._log("✗ Share rejected", "WARNING")
        elif event == "pool_error":
            self._log(f"Pool error: {data}", "ERROR")
    
    def _mining_callback(self, event: str, data: Any):
        """Handle mining events"""
        if event == "hashrate_update":
            self.after(0, self._update_stats)
        elif event == "share_found":
            self._log(f"💎 Share found on {data['device']}", "SUCCESS")
    
    def _update_loop(self):
        """Main update loop"""
        self.lbl_time.configure(text=datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
        
        if self.is_mining:
            self.lbl_status.configure(text="Mining...", text_color=Colors.SUCCESS)
            self._update_stats()
        elif self.chain_info.connected:
            self.lbl_status.configure(text="Connected", text_color=Colors.INFO)
            self.lbl_connection.configure(text="Connected", text_color=Colors.SUCCESS)
        else:
            self.lbl_status.configure(text="Ready", text_color=Colors.TEXT_DIM)
        
        # Update chain info periodically
        if hasattr(self, '_last_chain_update'):
            if time.time() - self._last_chain_update > 30:
                self._update_chain_info()
                self._last_chain_update = time.time()
        else:
            self._last_chain_update = time.time()
        
        self.after(1000, self._update_loop)
    
    def _update_stats(self):
        """Update mining stats"""
        total_hr = sum(d.hashrate for d in self.devices if d.is_active)
        
        self.lbl_hashrate.configure(text=self._format_hashrate(total_hr))
        self.lbl_current_hash.configure(text=self._format_hashrate(total_hr))
        self.lbl_shares_a.configure(text=str(self.stats.shares_accepted))
        self.lbl_shares_r.configure(text=str(self.stats.shares_rejected))
        
        total = self.stats.shares_accepted + self.stats.shares_rejected
        eff = (self.stats.shares_accepted / total * 100) if total > 0 else 0
        self.lbl_efficiency.configure(text=f"{eff:.1f}%")
        
        if self.start_time:
            elapsed = int(time.time() - self.start_time)
            h, r = divmod(elapsed, 3600)
            m, s = divmod(r, 60)
            self.lbl_uptime.configure(text=f"{h:02d}:{m:02d}:{s:02d}")
        
        # Update chart
        self.hashrate_history.append(total_hr / 1e3)  # KH/s
        if len(self.hashrate_history) > 120:
            self.hashrate_history.pop(0)
        
        self.line.set_data(range(len(self.hashrate_history)), self.hashrate_history)
        self.ax.relim()
        self.ax.autoscale_view()
        self.chart_canvas.draw_idle()
        
        self._refresh_device_list()
    
    def _on_algo_change(self):
        """Handle algorithm change"""
        algo = self.algo_var.get()
        self.selected_algorithm = algo
        self._log(f"Algorithm changed to {algo}", "INFO")
    
    def _on_network_change(self, network: str):
        """Handle network change"""
        global ACTIVE_NETWORK
        ACTIVE_NETWORK = NETWORKS.get(network.lower(), NETWORKS["testnet"])
        
        self.entry_pool.delete(0, "end")
        self.entry_pool.insert(0, f"{ACTIVE_NETWORK['pool_host']}:{ACTIVE_NETWORK['pool_port']}")
        
        self._log(f"Network: {network}", "INFO")
        self._connect_blockchain()
    
    def _on_threads_change(self, value):
        """Handle thread slider change"""
        self.num_threads = int(value)
        self.lbl_threads.configure(text=str(self.num_threads))
    
    def _save_config(self):
        """Save configuration"""
        self.wallet_address = self.entry_wallet.get().strip()
        self.worker_name = self.entry_worker.get().strip()
        self.num_threads = int(self.slider_threads.get())
        
        self._log("💾 Configuration saved", "SUCCESS")
        messagebox.showinfo("Saved", "Configuration saved successfully!")
    
    def _test_connection(self):
        """Test pool connection"""
        pool_str = self.entry_pool.get().strip()
        if ':' in pool_str:
            host, port = pool_str.rsplit(':', 1)
            port = int(port)
        else:
            host = ACTIVE_NETWORK["pool_host"]
            port = ACTIVE_NETWORK["pool_port"]
        
        self._log(f"🧪 Testing {host}:{port}...", "INFO")
        
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.settimeout(5)
            result = sock.connect_ex((host, port))
            sock.close()
            
            if result == 0:
                self._log("✓ Connection successful!", "SUCCESS")
                messagebox.showinfo("Success", "Pool connection successful!")
            else:
                self._log("✗ Connection failed", "ERROR")
                messagebox.showerror("Failed", "Could not connect to pool")
        except Exception as e:
            self._log(f"✗ Error: {e}", "ERROR")
            messagebox.showerror("Error", str(e))
    
    def _refresh_all(self):
        """Refresh all data"""
        self._update_chain_info()
        self._update_stats()
        self._log("🔄 Refreshed", "INFO")
    
    def _refresh_wallet(self):
        """Refresh wallet balance"""
        wallet = self.entry_wallet.get().strip()
        if wallet and self.blockchain and self.blockchain.connected:
            try:
                balance = self.blockchain.get_balance(wallet)
                self.lbl_balance.configure(text=f"{balance:.8f} GXC")
                self._log(f"💰 Balance: {balance:.8f} GXC", "SUCCESS")
            except Exception as e:
                self._log(f"Balance error: {e}", "ERROR")
    
    def _log(self, message: str, level: str = "INFO"):
        """Log message"""
        ts = datetime.now().strftime("%H:%M:%S")
        prefix = {"INFO": "[i]", "SUCCESS": "[✓]", "WARNING": "[!]", "ERROR": "[✗]"}.get(level, "[?]")
        
        def write():
            self.console.insert("end", f"[{ts}] {prefix} {message}\n")
            self.console.see("end")
        
        self.after(0, write)
    
    def _export_logs(self):
        """Export logs to file"""
        filename = filedialog.asksaveasfilename(
            defaultextension=".txt",
            filetypes=[("Text", "*.txt"), ("All", "*.*")],
            initialfile=f"gxc_miner_{datetime.now().strftime('%Y%m%d_%H%M%S')}.txt"
        )
        if filename:
            with open(filename, 'w') as f:
                f.write(self.console.get("1.0", "end"))
            self._log(f"📄 Exported to {filename}", "SUCCESS")
    
    def _format_hashrate(self, hr: float) -> str:
        """Format hashrate"""
        if hr >= 1e15:
            return f"{hr/1e15:.2f} PH/s"
        elif hr >= 1e12:
            return f"{hr/1e12:.2f} TH/s"
        elif hr >= 1e9:
            return f"{hr/1e9:.2f} GH/s"
        elif hr >= 1e6:
            return f"{hr/1e6:.2f} MH/s"
        elif hr >= 1e3:
            return f"{hr/1e3:.2f} KH/s"
        else:
            return f"{hr:.2f} H/s"
    
    def on_closing(self):
        """Handle window close"""
        if self.is_mining:
            if messagebox.askokcancel("Quit", "Mining is active. Stop and quit?"):
                self._stop_mining()
                time.sleep(0.5)
                self.destroy()
        else:
            self.destroy()


# ═══════════════════════════════════════════════════════════════════════════════
#                                  MAIN
# ═══════════════════════════════════════════════════════════════════════════════

def main():
    """Main entry point"""
    app = GXCMinerApp()
    app.protocol("WM_DELETE_WINDOW", app.on_closing)
    app.mainloop()


if __name__ == "__main__":
    main()
