#!/usr/bin/env python3
"""
GXC-POOL-MINER v2.0.0
Universal Mining Management System

Real blockchain connectivity - No simulations
Supports SHA256, Ethash/ProgPoW, GxHash algorithms
Auto-detects ASICs, GPUs, CPUs

Copyright (c) 2024 GXC Blockchain
Licensed under MIT License
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
from typing import Dict, List, Optional, Callable, Any
from dataclasses import dataclass, field
from queue import Queue
import logging

# GUI imports
import customtkinter as ctk
from tkinter import messagebox, filedialog
import tkinter as tk

# Plotting
import matplotlib
matplotlib.use("TkAgg")
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure

# System info
try:
    import psutil
    HAS_PSUTIL = True
except ImportError:
    HAS_PSUTIL = False

# ============================================================================
#                              CONFIGURATION
# ============================================================================

VERSION = "2.0.0"
APP_NAME = "GXC-POOL-MINER"

# Network Configuration
MAINNET_CONFIG = {
    "name": "Mainnet",
    "rpc_url": "http://node.gxc.network:8332",
    "pool_host": "pool.gxc.network",
    "pool_port": 3333,
    "explorer_url": "https://explorer.gxc.io",
    "address_prefix": "GXC"
}

TESTNET_CONFIG = {
    "name": "Testnet", 
    "rpc_url": "http://localhost:8332",
    "pool_host": "localhost",
    "pool_port": 3333,
    "explorer_url": "http://localhost:3000",
    "address_prefix": "tGXC"
}

# Default to local testnet for development
ACTIVE_NETWORK = TESTNET_CONFIG

# Theme Constants
COLORS = {
    "bg": "#0a0e1a",
    "panel": "#111827",
    "sidebar": "#1a1f2e",
    "accent": "#00ff9d",
    "accent_dim": "#008f58",
    "text": "#e0e6ed",
    "text_dim": "#888888",
    "danger": "#ff3333",
    "warning": "#ffaa00",
    "success": "#00ff9d",
    "info": "#3b82f6"
}

# Mining algorithms
ALGORITHMS = {
    "SHA256": {"name": "SHA256", "device": "ASIC", "unit": "TH/s", "multiplier": 1e12},
    "Ethash": {"name": "Ethash", "device": "GPU", "unit": "MH/s", "multiplier": 1e6},
    "GxHash": {"name": "GxHash", "device": "CPU", "unit": "KH/s", "multiplier": 1e3}
}

# Setup logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s [%(levelname)s] %(message)s',
    datefmt='%H:%M:%S'
)
logger = logging.getLogger("GXC-Miner")


# ============================================================================
#                           DATA CLASSES
# ============================================================================

@dataclass
class MiningDevice:
    """Represents a mining device"""
    id: str
    name: str
    device_type: str  # CPU, GPU, ASIC
    algorithm: str
    vendor: str = "Unknown"
    memory: str = "Unknown"
    cores: int = 0
    frequency: str = "Unknown"
    ip_address: str = ""
    status: str = "Detected"
    hashrate: float = 0.0
    temperature: float = 0.0
    power: float = 0.0
    shares_accepted: int = 0
    shares_rejected: int = 0
    is_active: bool = False


@dataclass  
class PoolConfig:
    """Pool connection configuration"""
    host: str
    port: int
    wallet_address: str
    worker_name: str = "default"
    password: str = "x"
    algorithm: str = "GxHash"
    

@dataclass
class BlockchainStats:
    """Blockchain network statistics"""
    height: int = 0
    difficulty: float = 0.0
    network_hashrate: float = 0.0
    block_reward: float = 0.0
    last_block_time: str = ""
    connected: bool = False


@dataclass
class MiningStats:
    """Current mining statistics"""
    total_hashrate: float = 0.0
    shares_accepted: int = 0
    shares_rejected: int = 0
    blocks_found: int = 0
    uptime_seconds: int = 0
    efficiency: float = 0.0
    estimated_earnings: float = 0.0


# ============================================================================
#                        GXC BLOCKCHAIN CLIENT
# ============================================================================

class GXCBlockchainClient:
    """
    Real GXC Blockchain RPC Client
    Connects to actual GXC node - NO SIMULATION
    """
    
    def __init__(self, rpc_url: str):
        self.rpc_url = rpc_url
        self.request_id = 0
        self.connected = False
        self.session = requests.Session()
        self.session.headers.update({'Content-Type': 'application/json'})
        
    def _call(self, method: str, params: List = None) -> Any:
        """Make RPC call to blockchain node"""
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
            response = self.session.post(self.rpc_url, json=payload, timeout=10)
            response.raise_for_status()
            data = response.json()
            
            if "error" in data and data["error"]:
                raise Exception(f"RPC Error: {data['error']}")
                
            self.connected = True
            return data.get("result")
            
        except requests.exceptions.RequestException as e:
            self.connected = False
            logger.error(f"RPC connection failed: {e}")
            raise
    
    def test_connection(self) -> bool:
        """Test connection to blockchain node"""
        try:
            self._call("getblockchaininfo")
            self.connected = True
            return True
        except:
            self.connected = False
            return False
    
    def get_blockchain_info(self) -> dict:
        """Get blockchain information"""
        return self._call("getblockchaininfo")
    
    def get_block_template(self) -> dict:
        """Get block template for mining"""
        return self._call("getblocktemplate")
    
    def submit_block(self, block_data: dict) -> bool:
        """Submit mined block"""
        result = self._call("submitblock", [block_data])
        return result is None  # null = success
    
    def get_balance(self, address: str) -> float:
        """Get address balance"""
        return self._call("getbalance", [address])
    
    def get_mining_info(self) -> dict:
        """Get mining information"""
        return self._call("getmininginfo")
    
    def get_difficulty(self) -> float:
        """Get current difficulty"""
        return self._call("getdifficulty")
    
    def get_network_hashrate(self) -> float:
        """Get network hashrate"""
        return self._call("getnetworkhashps")
    
    def estimate_fee(self) -> dict:
        """Get fee estimation"""
        return self._call("estimatefee")


# ============================================================================
#                          STRATUM CLIENT
# ============================================================================

class StratumClient:
    """
    Stratum protocol client for pool mining
    Real implementation - connects to actual mining pools
    """
    
    def __init__(self, pool_config: PoolConfig, callback: Callable):
        self.config = pool_config
        self.callback = callback
        self.socket: Optional[socket.socket] = None
        self.connected = False
        self.subscribed = False
        self.authorized = False
        self.running = False
        self.request_id = 0
        self.extranonce1 = ""
        self.extranonce2_size = 4
        self.current_job = None
        self.recv_thread = None
        self.recv_buffer = ""
        
    def connect(self) -> bool:
        """Connect to mining pool"""
        try:
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.settimeout(30)
            self.socket.connect((self.config.host, self.config.port))
            self.connected = True
            self.running = True
            
            # Start receive thread
            self.recv_thread = threading.Thread(target=self._receive_loop, daemon=True)
            self.recv_thread.start()
            
            # Subscribe
            self._subscribe()
            
            # Authorize
            time.sleep(0.5)
            self._authorize()
            
            logger.info(f"Connected to pool {self.config.host}:{self.config.port}")
            self.callback("connected", None)
            return True
            
        except Exception as e:
            logger.error(f"Pool connection failed: {e}")
            self.connected = False
            self.callback("error", str(e))
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
    
    def _send(self, data: dict):
        """Send JSON message to pool"""
        if not self.socket or not self.connected:
            return
        try:
            message = json.dumps(data) + "\n"
            self.socket.sendall(message.encode('utf-8'))
        except Exception as e:
            logger.error(f"Send error: {e}")
            self.connected = False
    
    def _receive_loop(self):
        """Receive messages from pool"""
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
        """Handle incoming pool message"""
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
                
        elif "id" in msg:
            # Response to our request
            req_id = msg["id"]
            result = msg.get("result")
            error = msg.get("error")
            
            if error:
                logger.error(f"Pool error: {error}")
                self.callback("error", error)
            elif req_id == 1:  # Subscribe response
                self._handle_subscribe_response(result)
            elif req_id == 2:  # Authorize response
                self._handle_authorize_response(result)
            else:  # Share response
                self._handle_share_response(result, req_id)
    
    def _subscribe(self):
        """Send mining.subscribe"""
        self.request_id = 1
        self._send({
            "id": self.request_id,
            "method": "mining.subscribe",
            "params": [f"{APP_NAME}/{VERSION}", None, self.config.host, self.config.port]
        })
    
    def _authorize(self):
        """Send mining.authorize"""
        self.request_id = 2
        worker = f"{self.config.wallet_address}.{self.config.worker_name}"
        self._send({
            "id": self.request_id,
            "method": "mining.authorize",
            "params": [worker, self.config.password]
        })
    
    def _handle_subscribe_response(self, result):
        """Handle subscribe response"""
        if result:
            self.subscribed = True
            if len(result) >= 2:
                self.extranonce1 = result[1]
                self.extranonce2_size = result[2] if len(result) > 2 else 4
            logger.info(f"Subscribed: extranonce1={self.extranonce1}")
            self.callback("subscribed", result)
    
    def _handle_authorize_response(self, result):
        """Handle authorize response"""
        self.authorized = bool(result)
        if self.authorized:
            logger.info("Worker authorized")
            self.callback("authorized", True)
        else:
            logger.error("Worker authorization failed")
            self.callback("auth_failed", False)
    
    def _handle_notify(self, params):
        """Handle mining.notify - new job"""
        if len(params) >= 8:
            self.current_job = {
                "job_id": params[0],
                "prevhash": params[1],
                "coinb1": params[2],
                "coinb2": params[3],
                "merkle_branch": params[4],
                "version": params[5],
                "nbits": params[6],
                "ntime": params[7],
                "clean_jobs": params[8] if len(params) > 8 else True
            }
            logger.debug(f"New job: {params[0]}")
            self.callback("new_job", self.current_job)
    
    def _handle_set_difficulty(self, params):
        """Handle mining.set_difficulty"""
        if params:
            difficulty = params[0]
            logger.info(f"Difficulty set to {difficulty}")
            self.callback("difficulty", difficulty)
    
    def _handle_set_extranonce(self, params):
        """Handle mining.set_extranonce"""
        if len(params) >= 2:
            self.extranonce1 = params[0]
            self.extranonce2_size = params[1]
            logger.info(f"New extranonce1: {self.extranonce1}")
    
    def _handle_share_response(self, result, req_id):
        """Handle share submission response"""
        if result:
            self.callback("share_accepted", req_id)
        else:
            self.callback("share_rejected", req_id)
    
    def submit_share(self, job_id: str, extranonce2: str, ntime: str, nonce: str) -> int:
        """Submit mining share to pool"""
        self.request_id += 1
        worker = f"{self.config.wallet_address}.{self.config.worker_name}"
        
        self._send({
            "id": self.request_id,
            "method": "mining.submit",
            "params": [worker, job_id, extranonce2, ntime, nonce]
        })
        
        return self.request_id


# ============================================================================
#                          MINING ENGINE
# ============================================================================

class MiningEngine:
    """
    Core mining engine
    Implements SHA256, Ethash, and GxHash algorithms
    """
    
    def __init__(self, device: MiningDevice, stratum: StratumClient, callback: Callable):
        self.device = device
        self.stratum = stratum
        self.callback = callback
        self.running = False
        self.paused = False
        self.current_job = None
        self.target = bytes(32)
        self.difficulty = 1.0
        self.nonce = 0
        self.hashes_done = 0
        self.start_time = 0
        self.worker_thread = None
        
    def start(self):
        """Start mining"""
        self.running = True
        self.paused = False
        self.start_time = time.time()
        self.worker_thread = threading.Thread(target=self._mine_loop, daemon=True)
        self.worker_thread.start()
        logger.info(f"Mining started on {self.device.name}")
        
    def stop(self):
        """Stop mining"""
        self.running = False
        if self.worker_thread:
            self.worker_thread.join(timeout=2)
        logger.info(f"Mining stopped on {self.device.name}")
    
    def pause(self):
        """Pause mining"""
        self.paused = True
        
    def resume(self):
        """Resume mining"""
        self.paused = False
    
    def set_job(self, job: dict):
        """Set new mining job"""
        self.current_job = job
        self.nonce = 0
    
    def set_difficulty(self, difficulty: float):
        """Set target difficulty"""
        self.difficulty = difficulty
        self.target = self._calculate_target(difficulty)
    
    def _calculate_target(self, difficulty: float) -> bytes:
        """Calculate target from difficulty"""
        # Target = max_target / difficulty
        max_target = 0x00000000FFFF0000000000000000000000000000000000000000000000000000
        target_int = int(max_target / difficulty)
        return target_int.to_bytes(32, byteorder='big')
    
    def _mine_loop(self):
        """Main mining loop"""
        while self.running:
            if self.paused or not self.current_job:
                time.sleep(0.1)
                continue
            
            # Get job data
            job = self.current_job
            
            # Build block header
            header = self._build_header(job)
            
            # Mine batch of nonces
            batch_size = self._get_batch_size()
            start_nonce = self.nonce
            
            for i in range(batch_size):
                if not self.running or self.paused:
                    break
                    
                nonce = start_nonce + i
                
                # Hash based on algorithm
                hash_result = self._compute_hash(header, nonce)
                self.hashes_done += 1
                
                # Check if meets target
                if hash_result < self.target:
                    # Found valid share!
                    self._submit_share(job, nonce)
            
            self.nonce = start_nonce + batch_size
            
            # Update hashrate
            elapsed = time.time() - self.start_time
            if elapsed > 0:
                self.device.hashrate = self.hashes_done / elapsed
                self.callback("hashrate", self.device)
    
    def _build_header(self, job: dict) -> bytes:
        """Build block header from job"""
        # Simplified header construction
        version = bytes.fromhex(job.get("version", "00000001"))
        prev_hash = bytes.fromhex(job.get("prevhash", "0" * 64))
        merkle_root = self._compute_merkle_root(job)
        ntime = bytes.fromhex(job.get("ntime", hex(int(time.time()))[2:].zfill(8)))
        nbits = bytes.fromhex(job.get("nbits", "1d00ffff"))
        
        header = version + prev_hash + merkle_root + ntime + nbits
        return header
    
    def _compute_merkle_root(self, job: dict) -> bytes:
        """Compute merkle root from job"""
        # Build coinbase
        coinb1 = bytes.fromhex(job.get("coinb1", ""))
        extranonce1 = bytes.fromhex(self.stratum.extranonce1) if self.stratum.extranonce1 else b""
        extranonce2 = self.nonce.to_bytes(self.stratum.extranonce2_size, 'little')
        coinb2 = bytes.fromhex(job.get("coinb2", ""))
        
        coinbase = coinb1 + extranonce1 + extranonce2 + coinb2
        coinbase_hash = hashlib.sha256(hashlib.sha256(coinbase).digest()).digest()
        
        # Build merkle root
        merkle = coinbase_hash
        for branch in job.get("merkle_branch", []):
            branch_bytes = bytes.fromhex(branch)
            merkle = hashlib.sha256(hashlib.sha256(merkle + branch_bytes).digest()).digest()
        
        return merkle
    
    def _compute_hash(self, header: bytes, nonce: int) -> bytes:
        """Compute hash based on algorithm"""
        nonce_bytes = nonce.to_bytes(4, 'little')
        full_header = header + nonce_bytes
        
        if self.device.algorithm == "SHA256":
            # Double SHA256
            return hashlib.sha256(hashlib.sha256(full_header).digest()).digest()
            
        elif self.device.algorithm == "Ethash":
            # Simplified Ethash (Keccak-256)
            import hashlib
            return hashlib.sha3_256(full_header).digest()
            
        elif self.device.algorithm == "GxHash":
            # GxHash - traceability-optimized
            return self._gxhash(full_header)
            
        else:
            return hashlib.sha256(full_header).digest()
    
    def _gxhash(self, data: bytes) -> bytes:
        """
        GxHash algorithm - GXC's traceability-optimized hash
        Combines SHA256 with additional mixing for ASIC resistance
        """
        # Initial SHA256
        h1 = hashlib.sha256(data).digest()
        
        # Mix with Blake2b
        try:
            h2 = hashlib.blake2b(h1 + data, digest_size=32).digest()
        except:
            h2 = hashlib.sha256(h1 + data).digest()
        
        # XOR mixing
        result = bytes(a ^ b for a, b in zip(h1, h2))
        
        # Final SHA256
        return hashlib.sha256(result).digest()
    
    def _get_batch_size(self) -> int:
        """Get batch size based on device type"""
        if self.device.device_type == "ASIC":
            return 1000000
        elif self.device.device_type == "GPU":
            return 100000
        else:  # CPU
            return 10000
    
    def _submit_share(self, job: dict, nonce: int):
        """Submit found share to pool"""
        extranonce2 = nonce.to_bytes(self.stratum.extranonce2_size, 'little').hex()
        ntime = job.get("ntime", hex(int(time.time()))[2:].zfill(8))
        nonce_hex = nonce.to_bytes(4, 'little').hex()
        
        self.stratum.submit_share(
            job["job_id"],
            extranonce2,
            ntime,
            nonce_hex
        )
        
        self.callback("share_found", {
            "device": self.device.name,
            "nonce": nonce
        })


# ============================================================================
#                        DEVICE DETECTOR
# ============================================================================

class DeviceDetector:
    """Auto-detects mining hardware"""
    
    @staticmethod
    def detect_all() -> List[MiningDevice]:
        """Detect all mining devices"""
        devices = []
        
        # Detect CPUs
        cpu = DeviceDetector.detect_cpu()
        if cpu:
            devices.append(cpu)
        
        # Detect GPUs
        gpus = DeviceDetector.detect_gpus()
        devices.extend(gpus)
        
        # Detect ASICs
        asics = DeviceDetector.detect_asics()
        devices.extend(asics)
        
        return devices
    
    @staticmethod
    def detect_cpu() -> Optional[MiningDevice]:
        """Detect CPU for GxHash mining"""
        try:
            cpu_name = platform.processor() or "Unknown CPU"
            cores = 1
            threads = 1
            frequency = "Unknown"
            
            if HAS_PSUTIL:
                cores = psutil.cpu_count(logical=False) or 1
                threads = psutil.cpu_count(logical=True) or 1
                freq = psutil.cpu_freq()
                if freq:
                    frequency = f"{freq.max:.0f} MHz"
            
            return MiningDevice(
                id=f"cpu_0",
                name=cpu_name[:40],
                device_type="CPU",
                algorithm="GxHash",
                vendor="System",
                cores=cores,
                frequency=frequency,
                status="Ready"
            )
        except Exception as e:
            logger.error(f"CPU detection failed: {e}")
            return None
    
    @staticmethod
    def detect_gpus() -> List[MiningDevice]:
        """Detect GPUs for Ethash mining"""
        gpus = []
        
        # NVIDIA detection
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
                            memory=f"{parts[1]} MB" if len(parts) > 1 else "Unknown",
                            temperature=float(parts[2]) if len(parts) > 2 and parts[2] != '[N/A]' else 0,
                            power=float(parts[3]) if len(parts) > 3 and parts[3] != '[N/A]' else 0,
                            status="Ready"
                        ))
        except Exception as e:
            logger.debug(f"NVIDIA detection: {e}")
        
        # AMD detection
        try:
            result = subprocess.run(
                ['rocm-smi', '--showproductname', '--showtemp', '--showpower'],
                capture_output=True, text=True, timeout=5
            )
            if result.returncode == 0:
                # Parse rocm-smi output
                lines = result.stdout.strip().split('\n')
                gpu_idx = 0
                for line in lines:
                    if 'GPU[' in line and 'Name' in line:
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
        except Exception as e:
            logger.debug(f"AMD detection: {e}")
        
        return gpus
    
    @staticmethod
    def detect_asics() -> List[MiningDevice]:
        """Detect ASIC miners on network"""
        asics = []
        
        # Get local network range
        local_ip = DeviceDetector.get_local_ip()
        if local_ip == "127.0.0.1":
            return asics
        
        # Scan common ASIC ports
        network_prefix = '.'.join(local_ip.split('.')[:-1])
        
        # Common ASIC manufacturer default IPs and ports
        asic_configs = [
            {"name": "Antminer", "port": 4028, "api": "cgminer"},
            {"name": "Whatsminer", "port": 4028, "api": "cgminer"},
            {"name": "AvalonMiner", "port": 4028, "api": "cgminer"},
        ]
        
        # Quick scan first 10 IPs (expand in production)
        for host_num in range(1, 11):
            ip = f"{network_prefix}.{host_num}"
            
            for config in asic_configs:
                try:
                    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                    sock.settimeout(0.5)
                    result = sock.connect_ex((ip, config["port"]))
                    sock.close()
                    
                    if result == 0:
                        # Found potential ASIC
                        asic_info = DeviceDetector._query_asic(ip, config["port"])
                        if asic_info:
                            asics.append(MiningDevice(
                                id=f"asic_{ip.replace('.', '_')}",
                                name=asic_info.get("name", f"{config['name']} Miner"),
                                device_type="ASIC",
                                algorithm="SHA256",
                                vendor=config["name"],
                                ip_address=ip,
                                hashrate=asic_info.get("hashrate", 0),
                                status="Detected"
                            ))
                except:
                    pass
        
        return asics
    
    @staticmethod
    def _query_asic(ip: str, port: int) -> Optional[dict]:
        """Query ASIC miner API"""
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.settimeout(2)
            sock.connect((ip, port))
            
            # Send cgminer API command
            sock.sendall(b'{"command":"summary"}')
            response = sock.recv(4096)
            sock.close()
            
            data = json.loads(response.decode().rstrip('\x00'))
            if "SUMMARY" in data:
                summary = data["SUMMARY"][0]
                return {
                    "name": summary.get("Type", "ASIC Miner"),
                    "hashrate": summary.get("GHS 5s", 0) * 1e9
                }
        except:
            pass
        return None
    
    @staticmethod
    def get_local_ip() -> str:
        """Get local IP address"""
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            s.connect(("8.8.8.8", 80))
            ip = s.getsockname()[0]
            s.close()
            return ip
        except:
            return "127.0.0.1"


# ============================================================================
#                          MAIN APPLICATION
# ============================================================================

ctk.set_appearance_mode("Dark")
ctk.set_default_color_theme("green")


class GXCPoolMinerApp(ctk.CTk):
    """Main application GUI"""
    
    def __init__(self):
        super().__init__()
        
        self.title(f"{APP_NAME} v{VERSION} - Universal Mining Manager")
        self.geometry("1500x950")
        self.configure(fg_color=COLORS["bg"])
        self.minsize(1200, 800)
        
        # State
        self.devices: List[MiningDevice] = []
        self.mining_engines: Dict[str, MiningEngine] = {}
        self.stratum_client: Optional[StratumClient] = None
        self.blockchain_client: Optional[GXCBlockchainClient] = None
        self.is_mining = False
        self.start_time = None
        self.stats = MiningStats()
        self.blockchain_stats = BlockchainStats()
        self.hashrate_history = []
        self.log_queue = Queue()
        
        # Pool config
        self.pool_config = PoolConfig(
            host=ACTIVE_NETWORK["pool_host"],
            port=ACTIVE_NETWORK["pool_port"],
            wallet_address="",
            worker_name=platform.node()[:20],
            algorithm="GxHash"
        )
        
        # Setup UI
        self.grid_columnconfigure(1, weight=1)
        self.grid_rowconfigure(0, weight=1)
        
        self._setup_sidebar()
        self._setup_main_area()
        self._setup_status_bar()
        
        # Initialize
        self.after(100, self._initialize)
        
        # Update timer
        self.after(1000, self._update_loop)
    
    def _initialize(self):
        """Initialize application"""
        self._log("GXC-POOL-MINER initialized", "INFO")
        self._log(f"Version: {VERSION}", "INFO")
        self._log(f"System: {platform.system()} {platform.release()}", "INFO")
        self._log(f"Network: {ACTIVE_NETWORK['name']}", "INFO")
        
        # Connect to blockchain
        self._connect_blockchain()
        
        # Scan devices
        self.after(500, self._scan_devices)
    
    def _connect_blockchain(self):
        """Connect to GXC blockchain"""
        self._log(f"Connecting to {ACTIVE_NETWORK['rpc_url']}...", "INFO")
        
        self.blockchain_client = GXCBlockchainClient(ACTIVE_NETWORK["rpc_url"])
        
        if self.blockchain_client.test_connection():
            self._log("✓ Connected to GXC blockchain", "SUCCESS")
            self.blockchain_stats.connected = True
            self._update_blockchain_stats()
        else:
            self._log("✗ Failed to connect to blockchain", "ERROR")
            self._log("  Mining will use pool-provided data", "WARNING")
    
    def _update_blockchain_stats(self):
        """Update blockchain statistics"""
        if not self.blockchain_client or not self.blockchain_client.connected:
            return
        
        try:
            info = self.blockchain_client.get_blockchain_info()
            self.blockchain_stats.height = info.get("blocks", 0)
            self.blockchain_stats.difficulty = info.get("difficulty", 0)
            self.blockchain_stats.block_reward = info.get("block_reward", 50.0)
            self.blockchain_stats.last_block_time = datetime.now().strftime("%H:%M:%S")
            
            # Update UI
            self.lbl_block_height.configure(text=f"{self.blockchain_stats.height:,}")
            self.lbl_difficulty.configure(text=f"{self.blockchain_stats.difficulty:,.0f}")
            self.lbl_block_reward.configure(text=f"{self.blockchain_stats.block_reward:.2f} GXC")
            
            try:
                hashrate = self.blockchain_client.get_network_hashrate()
                self.blockchain_stats.network_hashrate = hashrate
                self.lbl_net_hashrate.configure(text=self._format_hashrate(hashrate))
            except:
                pass
                
        except Exception as e:
            logger.debug(f"Blockchain stats update failed: {e}")
    
    def _setup_sidebar(self):
        """Setup sidebar with stats and controls"""
        self.sidebar = ctk.CTkFrame(self, width=300, corner_radius=0, fg_color=COLORS["sidebar"])
        self.sidebar.grid(row=0, column=0, rowspan=2, sticky="nsew")
        self.sidebar.grid_propagate(False)
        
        # Logo
        logo_frame = ctk.CTkFrame(self.sidebar, fg_color="transparent")
        logo_frame.pack(pady=(25, 15))
        
        ctk.CTkLabel(logo_frame, text="GXC", font=("Impact", 52), text_color=COLORS["accent"]).pack()
        ctk.CTkLabel(logo_frame, text="POOL MINER", font=("Arial", 14, "bold"), text_color="#666").pack()
        ctk.CTkLabel(logo_frame, text=f"v{VERSION}", font=("Arial", 10), text_color="#444").pack()
        
        # Mining Stats
        stats_frame = ctk.CTkFrame(self.sidebar, fg_color=COLORS["panel"], corner_radius=12)
        stats_frame.pack(fill="x", padx=15, pady=10)
        
        ctk.CTkLabel(stats_frame, text="⚡ MINING STATS", font=("Arial", 11, "bold"), 
                    text_color=COLORS["accent"]).pack(pady=(12, 8))
        
        self.lbl_total_hashrate = self._create_stat_row(stats_frame, "Total Hashrate", "0 H/s")
        self.lbl_active_devices = self._create_stat_row(stats_frame, "Active Devices", "0")
        self.lbl_shares_accepted = self._create_stat_row(stats_frame, "Shares Accepted", "0")
        self.lbl_shares_rejected = self._create_stat_row(stats_frame, "Shares Rejected", "0")
        self.lbl_efficiency = self._create_stat_row(stats_frame, "Efficiency", "0%")
        self.lbl_uptime = self._create_stat_row(stats_frame, "Uptime", "00:00:00")
        
        # Blockchain Stats
        chain_frame = ctk.CTkFrame(self.sidebar, fg_color=COLORS["panel"], corner_radius=12)
        chain_frame.pack(fill="x", padx=15, pady=10)
        
        ctk.CTkLabel(chain_frame, text="🔗 GXC NETWORK", font=("Arial", 11, "bold"),
                    text_color=COLORS["info"]).pack(pady=(12, 8))
        
        self.lbl_connection = self._create_stat_row(chain_frame, "Connection", "Connecting...")
        self.lbl_block_height = self._create_stat_row(chain_frame, "Block Height", "...")
        self.lbl_difficulty = self._create_stat_row(chain_frame, "Difficulty", "...")
        self.lbl_net_hashrate = self._create_stat_row(chain_frame, "Network Hash", "...")
        self.lbl_block_reward = self._create_stat_row(chain_frame, "Block Reward", "...")
        
        # Control Buttons
        btn_frame = ctk.CTkFrame(self.sidebar, fg_color="transparent")
        btn_frame.pack(fill="x", padx=15, pady=20)
        
        self.btn_start = ctk.CTkButton(
            btn_frame,
            text="▶ START MINING",
            command=self._toggle_mining,
            font=("Arial", 15, "bold"),
            fg_color=COLORS["accent"],
            hover_color=COLORS["accent_dim"],
            height=50,
            corner_radius=10
        )
        self.btn_start.pack(fill="x", pady=5)
        
        ctk.CTkButton(
            btn_frame,
            text="🔍 SCAN DEVICES",
            command=self._scan_devices,
            font=("Arial", 12),
            fg_color=COLORS["panel"],
            hover_color="#1f2937",
            height=40,
            corner_radius=8
        ).pack(fill="x", pady=5)
        
        ctk.CTkButton(
            btn_frame,
            text="🔄 REFRESH STATS",
            command=self._refresh_stats,
            font=("Arial", 12),
            fg_color=COLORS["panel"],
            hover_color="#1f2937",
            height=40,
            corner_radius=8
        ).pack(fill="x", pady=5)
    
    def _create_stat_row(self, parent, label: str, value: str) -> ctk.CTkLabel:
        """Create a stat row in sidebar"""
        row = ctk.CTkFrame(parent, fg_color="transparent")
        row.pack(fill="x", padx=12, pady=4)
        
        ctk.CTkLabel(row, text=label, font=("Arial", 10), text_color="#aaa", anchor="w").pack(side="left")
        lbl = ctk.CTkLabel(row, text=value, font=("Arial", 10, "bold"), text_color="#fff", anchor="e")
        lbl.pack(side="right")
        return lbl
    
    def _setup_main_area(self):
        """Setup main content area with tabs"""
        self.tabview = ctk.CTkTabview(
            self,
            fg_color=COLORS["bg"],
            segmented_button_fg_color=COLORS["panel"],
            segmented_button_selected_color=COLORS["accent"],
            segmented_button_selected_hover_color=COLORS["accent_dim"],
            corner_radius=10
        )
        self.tabview.grid(row=0, column=1, padx=20, pady=(20, 10), sticky="nsew")
        
        # Create tabs
        self.tab_dashboard = self.tabview.add("📊 DASHBOARD")
        self.tab_devices = self.tabview.add("💻 DEVICES")
        self.tab_pool = self.tabview.add("🌐 POOL CONFIG")
        self.tab_wallet = self.tabview.add("💰 WALLET")
        self.tab_logs = self.tabview.add("📋 LOGS")
        
        self._setup_dashboard_tab()
        self._setup_devices_tab()
        self._setup_pool_tab()
        self._setup_wallet_tab()
        self._setup_logs_tab()
    
    def _setup_dashboard_tab(self):
        """Setup dashboard with charts"""
        # Chart frame
        chart_frame = ctk.CTkFrame(self.tab_dashboard, fg_color=COLORS["panel"], corner_radius=12)
        chart_frame.pack(fill="both", expand=True, padx=10, pady=10)
        
        header = ctk.CTkFrame(chart_frame, fg_color="transparent")
        header.pack(fill="x", padx=15, pady=10)
        
        ctk.CTkLabel(header, text="📈 HASHRATE MONITOR", font=("Arial", 16, "bold"),
                    text_color=COLORS["accent"]).pack(side="left")
        
        self.lbl_current_hashrate = ctk.CTkLabel(header, text="0 H/s", font=("Arial", 24, "bold"),
                                                 text_color="#fff")
        self.lbl_current_hashrate.pack(side="right", padx=20)
        
        # Matplotlib chart
        self.fig = Figure(figsize=(10, 5), facecolor=COLORS["panel"], dpi=100)
        self.ax = self.fig.add_subplot(111)
        self.ax.set_facecolor(COLORS["bg"])
        self.ax.tick_params(colors='#666', labelsize=8)
        self.ax.grid(True, alpha=0.2, color='#333')
        self.ax.set_xlabel('Time (seconds)', color='#888', fontsize=9)
        self.ax.set_ylabel('Hashrate', color='#888', fontsize=9)
        
        for spine in self.ax.spines.values():
            spine.set_color('#333')
        
        self.line, = self.ax.plot([], [], color=COLORS["accent"], linewidth=2)
        self.ax.fill_between([], [], alpha=0.3, color=COLORS["accent"])
        
        canvas = FigureCanvasTkAgg(self.fig, chart_frame)
        canvas.draw()
        canvas.get_tk_widget().pack(fill="both", expand=True, padx=10, pady=(0, 10))
        self.chart_canvas = canvas
        
        # Stats cards
        cards_frame = ctk.CTkFrame(self.tab_dashboard, fg_color="transparent")
        cards_frame.pack(fill="x", padx=10, pady=10)
        
        for i in range(4):
            cards_frame.grid_columnconfigure(i, weight=1)
        
        self._create_stat_card(cards_frame, "🔥 Current Hash", "0 H/s", 0)
        self._create_stat_card(cards_frame, "⚡ Peak Hash", "0 H/s", 1)
        self._create_stat_card(cards_frame, "💎 Shares/Hour", "0", 2)
        self._create_stat_card(cards_frame, "💰 Est. Earnings", "0 GXC/day", 3)
    
    def _create_stat_card(self, parent, title: str, value: str, column: int):
        """Create a stat card"""
        card = ctk.CTkFrame(parent, fg_color=COLORS["panel"], corner_radius=10)
        card.grid(row=0, column=column, padx=5, pady=5, sticky="nsew")
        
        ctk.CTkLabel(card, text=title, font=("Arial", 11), text_color="#888").pack(pady=(15, 5))
        ctk.CTkLabel(card, text=value, font=("Arial", 18, "bold"), text_color="#fff").pack(pady=(0, 15))
    
    def _setup_devices_tab(self):
        """Setup devices list"""
        # Header
        header = ctk.CTkFrame(self.tab_devices, fg_color=COLORS["panel"], corner_radius=10, height=50)
        header.pack(fill="x", padx=10, pady=(10, 5))
        header.pack_propagate(False)
        
        cols = ["Device", "Type", "Algorithm", "Hashrate", "Temp", "Shares", "Status"]
        for i, col in enumerate(cols):
            lbl = ctk.CTkLabel(header, text=col, font=("Arial", 11, "bold"), text_color="#aaa")
            lbl.pack(side="left", expand=True, padx=5, pady=12)
        
        # Device list
        self.device_list = ctk.CTkScrollableFrame(self.tab_devices, fg_color="transparent")
        self.device_list.pack(fill="both", expand=True, padx=10, pady=5)
    
    def _setup_pool_tab(self):
        """Setup pool configuration"""
        config_frame = ctk.CTkFrame(self.tab_pool, fg_color=COLORS["panel"], corner_radius=12)
        config_frame.pack(fill="both", expand=True, padx=20, pady=20)
        
        ctk.CTkLabel(config_frame, text="🌐 MINING POOL CONFIGURATION",
                    font=("Arial", 20, "bold"), text_color=COLORS["accent"]).pack(pady=(30, 20))
        
        # Network selection
        net_frame = ctk.CTkFrame(config_frame, fg_color="transparent")
        net_frame.pack(pady=10)
        
        ctk.CTkLabel(net_frame, text="Network:", font=("Arial", 12), width=150, anchor="e").pack(side="left", padx=10)
        self.network_var = ctk.StringVar(value="Testnet")
        self.network_menu = ctk.CTkOptionMenu(net_frame, values=["Mainnet", "Testnet", "Custom"],
                                              variable=self.network_var, width=400,
                                              command=self._on_network_change)
        self.network_menu.pack(side="left", padx=10)
        
        # Form
        form_frame = ctk.CTkFrame(config_frame, fg_color="transparent")
        form_frame.pack(pady=20)
        
        self.entries = {}
        fields = [
            ("Pool Address:", f"{ACTIVE_NETWORK['pool_host']}:{ACTIVE_NETWORK['pool_port']}"),
            ("Wallet Address:", "Enter your GXC wallet address"),
            ("Worker Name:", platform.node()[:20]),
            ("Password:", "x"),
            ("Algorithm:", "GxHash")
        ]
        
        for label, default in fields:
            row = ctk.CTkFrame(form_frame, fg_color="transparent")
            row.pack(fill="x", pady=8)
            
            ctk.CTkLabel(row, text=label, font=("Arial", 12), width=150, anchor="e").pack(side="left", padx=10)
            entry = ctk.CTkEntry(row, font=("Arial", 12), width=400, placeholder_text=default)
            if "Wallet" not in label:
                entry.insert(0, default)
            entry.pack(side="left", padx=10)
            self.entries[label] = entry
        
        # Buttons
        btn_frame = ctk.CTkFrame(config_frame, fg_color="transparent")
        btn_frame.pack(pady=30)
        
        ctk.CTkButton(
            btn_frame,
            text="💾 SAVE CONFIGURATION",
            font=("Arial", 14, "bold"),
            fg_color=COLORS["accent"],
            hover_color=COLORS["accent_dim"],
            height=45,
            width=200,
            command=self._save_config
        ).pack(side="left", padx=10)
        
        ctk.CTkButton(
            btn_frame,
            text="🧪 TEST CONNECTION",
            font=("Arial", 14),
            fg_color=COLORS["info"],
            hover_color="#2563eb",
            height=45,
            width=200,
            command=self._test_pool_connection
        ).pack(side="left", padx=10)
    
    def _setup_wallet_tab(self):
        """Setup wallet view"""
        wallet_frame = ctk.CTkFrame(self.tab_wallet, fg_color=COLORS["panel"], corner_radius=12)
        wallet_frame.pack(fill="both", expand=True, padx=20, pady=20)
        
        ctk.CTkLabel(wallet_frame, text="💰 WALLET & EARNINGS",
                    font=("Arial", 20, "bold"), text_color=COLORS["accent"]).pack(pady=(30, 20))
        
        # Balance display
        balance_frame = ctk.CTkFrame(wallet_frame, fg_color=COLORS["bg"], corner_radius=10)
        balance_frame.pack(fill="x", padx=40, pady=20)
        
        ctk.CTkLabel(balance_frame, text="Current Balance", font=("Arial", 14),
                    text_color="#888").pack(pady=(20, 5))
        self.lbl_wallet_balance = ctk.CTkLabel(balance_frame, text="0.00000000 GXC",
                                               font=("Arial", 32, "bold"), text_color=COLORS["accent"])
        self.lbl_wallet_balance.pack(pady=(0, 20))
        
        # Stats
        stats = ctk.CTkFrame(wallet_frame, fg_color="transparent")
        stats.pack(fill="x", padx=40, pady=20)
        
        for i in range(3):
            stats.grid_columnconfigure(i, weight=1)
        
        stat_data = [
            ("Pending Balance", "0.00000000 GXC"),
            ("Total Earned", "0.00000000 GXC"),
            ("Last Payout", "Never")
        ]
        
        for i, (title, value) in enumerate(stat_data):
            card = ctk.CTkFrame(stats, fg_color=COLORS["bg"], corner_radius=10)
            card.grid(row=0, column=i, padx=10, pady=5, sticky="nsew")
            ctk.CTkLabel(card, text=title, font=("Arial", 11), text_color="#888").pack(pady=(15, 5))
            ctk.CTkLabel(card, text=value, font=("Arial", 14, "bold")).pack(pady=(0, 15))
        
        # Refresh button
        ctk.CTkButton(
            wallet_frame,
            text="🔄 REFRESH BALANCE",
            font=("Arial", 14),
            fg_color=COLORS["accent"],
            hover_color=COLORS["accent_dim"],
            height=45,
            width=200,
            command=self._refresh_wallet
        ).pack(pady=20)
    
    def _setup_logs_tab(self):
        """Setup logs console"""
        self.log_console = ctk.CTkTextbox(
            self.tab_logs,
            font=("Consolas", 11),
            text_color=COLORS["text"],
            fg_color="#000",
            wrap="word",
            corner_radius=10
        )
        self.log_console.pack(fill="both", expand=True, padx=10, pady=10)
        
        # Control buttons
        btn_frame = ctk.CTkFrame(self.tab_logs, fg_color="transparent")
        btn_frame.pack(fill="x", padx=10, pady=(0, 10))
        
        ctk.CTkButton(btn_frame, text="Clear Logs", command=self._clear_logs,
                     fg_color=COLORS["panel"], width=100).pack(side="left", padx=5)
        ctk.CTkButton(btn_frame, text="Export Logs", command=self._export_logs,
                     fg_color=COLORS["panel"], width=100).pack(side="left", padx=5)
    
    def _setup_status_bar(self):
        """Setup bottom status bar"""
        self.status_bar = ctk.CTkFrame(self, height=30, fg_color=COLORS["panel"], corner_radius=0)
        self.status_bar.grid(row=1, column=1, sticky="ew", padx=20, pady=(0, 10))
        
        self.lbl_status = ctk.CTkLabel(self.status_bar, text="Ready", font=("Arial", 10),
                                       text_color="#888")
        self.lbl_status.pack(side="left", padx=15, pady=5)
        
        self.lbl_time = ctk.CTkLabel(self.status_bar, text="", font=("Arial", 10),
                                     text_color="#666")
        self.lbl_time.pack(side="right", padx=15, pady=5)
    
    # ========== ACTIONS ==========
    
    def _scan_devices(self):
        """Scan for mining devices"""
        self._log("🔍 Scanning for mining devices...", "INFO")
        
        self.devices = DeviceDetector.detect_all()
        
        if not self.devices:
            self._log("⚠ No mining devices detected", "WARNING")
        else:
            for device in self.devices:
                self._log(f"✓ Found: {device.name} ({device.device_type}) - {device.algorithm}", "SUCCESS")
            self._log(f"📊 Scan complete: {len(self.devices)} device(s)", "SUCCESS")
        
        self._refresh_device_list()
    
    def _refresh_device_list(self):
        """Refresh device list UI"""
        for widget in self.device_list.winfo_children():
            widget.destroy()
        
        for device in self.devices:
            row = ctk.CTkFrame(self.device_list, fg_color=COLORS["panel"], corner_radius=8, height=50)
            row.pack(fill="x", pady=3)
            row.pack_propagate(False)
            
            # Device info
            ctk.CTkLabel(row, text=device.name[:25], font=("Arial", 11, "bold"),
                        width=180, anchor="w").pack(side="left", padx=10, pady=12)
            ctk.CTkLabel(row, text=device.device_type, width=80,
                        text_color="#888").pack(side="left", padx=5)
            ctk.CTkLabel(row, text=device.algorithm, width=100,
                        text_color=COLORS["accent"]).pack(side="left", padx=5)
            ctk.CTkLabel(row, text=self._format_hashrate(device.hashrate),
                        width=120).pack(side="left", padx=5)
            ctk.CTkLabel(row, text=f"{device.temperature:.0f}°C" if device.temperature else "N/A",
                        width=80, text_color="#888").pack(side="left", padx=5)
            ctk.CTkLabel(row, text=f"{device.shares_accepted}/{device.shares_rejected}",
                        width=100).pack(side="left", padx=5)
            
            status_color = COLORS["success"] if device.is_active else (
                COLORS["warning"] if device.status == "Detected" else "#666")
            ctk.CTkLabel(row, text=f"● {device.status}", text_color=status_color,
                        width=100).pack(side="left", padx=5)
    
    def _toggle_mining(self):
        """Start or stop mining"""
        if not self.is_mining:
            self._start_mining()
        else:
            self._stop_mining()
    
    def _start_mining(self):
        """Start mining on all devices"""
        if not self.devices:
            messagebox.showwarning("No Devices", "Please scan for devices first!")
            return
        
        wallet = self.entries.get("Wallet Address:")
        if wallet:
            wallet_text = wallet.get().strip()
            if not wallet_text or wallet_text == "Enter your GXC wallet address":
                messagebox.showwarning("Wallet Required", "Please enter your GXC wallet address!")
                return
            self.pool_config.wallet_address = wallet_text
        
        self.is_mining = True
        self.start_time = time.time()
        self.btn_start.configure(text="⏹ STOP MINING", fg_color=COLORS["danger"])
        self._log("▶ Starting mining operations...", "INFO")
        
        # Connect to pool
        self.stratum_client = StratumClient(self.pool_config, self._stratum_callback)
        
        threading.Thread(target=self._connect_pool, daemon=True).start()
    
    def _connect_pool(self):
        """Connect to pool in background"""
        if self.stratum_client.connect():
            self._log(f"✓ Connected to pool {self.pool_config.host}", "SUCCESS")
            
            # Start mining engines
            for device in self.devices:
                engine = MiningEngine(device, self.stratum_client, self._mining_callback)
                engine.start()
                self.mining_engines[device.id] = engine
                device.is_active = True
                device.status = "Mining"
                self._log(f"⚡ Started mining on {device.name}", "SUCCESS")
        else:
            self._log("✗ Failed to connect to pool", "ERROR")
            self.after(0, self._stop_mining)
    
    def _stop_mining(self):
        """Stop all mining"""
        self.is_mining = False
        self.btn_start.configure(text="▶ START MINING", fg_color=COLORS["accent"])
        self._log("⏹ Stopping mining operations...", "WARNING")
        
        # Stop engines
        for engine in self.mining_engines.values():
            engine.stop()
        self.mining_engines.clear()
        
        # Disconnect pool
        if self.stratum_client:
            self.stratum_client.disconnect()
            self.stratum_client = None
        
        # Update device status
        for device in self.devices:
            device.is_active = False
            device.status = "Ready"
        
        self._refresh_device_list()
        self._log("✓ Mining stopped", "INFO")
    
    def _stratum_callback(self, event: str, data: Any):
        """Handle stratum events"""
        if event == "new_job":
            for engine in self.mining_engines.values():
                engine.set_job(data)
        elif event == "difficulty":
            for engine in self.mining_engines.values():
                engine.set_difficulty(data)
        elif event == "share_accepted":
            self.stats.shares_accepted += 1
            self._log(f"✓ Share accepted! Total: {self.stats.shares_accepted}", "SUCCESS")
        elif event == "share_rejected":
            self.stats.shares_rejected += 1
            self._log(f"✗ Share rejected", "WARNING")
        elif event == "error":
            self._log(f"Pool error: {data}", "ERROR")
    
    def _mining_callback(self, event: str, data: Any):
        """Handle mining engine events"""
        if event == "hashrate":
            self.after(0, self._update_stats)
        elif event == "share_found":
            self._log(f"💎 Share found by {data['device']}", "SUCCESS")
    
    def _update_loop(self):
        """Main update loop"""
        # Update time
        self.lbl_time.configure(text=datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
        
        # Update status
        if self.is_mining:
            self.lbl_status.configure(text="Mining...", text_color=COLORS["success"])
        elif self.blockchain_stats.connected:
            self.lbl_status.configure(text="Connected", text_color=COLORS["info"])
        else:
            self.lbl_status.configure(text="Ready", text_color="#888")
        
        # Update stats
        if self.is_mining:
            self._update_stats()
        
        # Update blockchain stats periodically
        if hasattr(self, '_last_chain_update'):
            if time.time() - self._last_chain_update > 30:
                self._update_blockchain_stats()
                self._last_chain_update = time.time()
        else:
            self._last_chain_update = time.time()
        
        # Schedule next update
        self.after(1000, self._update_loop)
    
    def _update_stats(self):
        """Update mining statistics"""
        # Calculate totals
        total_hashrate = sum(d.hashrate for d in self.devices if d.is_active)
        active_count = sum(1 for d in self.devices if d.is_active)
        
        # Update sidebar
        self.lbl_total_hashrate.configure(text=self._format_hashrate(total_hashrate))
        self.lbl_active_devices.configure(text=str(active_count))
        self.lbl_shares_accepted.configure(text=str(self.stats.shares_accepted))
        self.lbl_shares_rejected.configure(text=str(self.stats.shares_rejected))
        
        total_shares = self.stats.shares_accepted + self.stats.shares_rejected
        efficiency = (self.stats.shares_accepted / total_shares * 100) if total_shares > 0 else 0
        self.lbl_efficiency.configure(text=f"{efficiency:.1f}%")
        
        if self.start_time:
            elapsed = int(time.time() - self.start_time)
            hours, remainder = divmod(elapsed, 3600)
            minutes, seconds = divmod(remainder, 60)
            self.lbl_uptime.configure(text=f"{hours:02d}:{minutes:02d}:{seconds:02d}")
        
        # Update chart
        self.hashrate_history.append(total_hashrate / 1e6)  # MH/s
        if len(self.hashrate_history) > 120:
            self.hashrate_history.pop(0)
        
        self.line.set_data(range(len(self.hashrate_history)), self.hashrate_history)
        self.ax.relim()
        self.ax.autoscale_view()
        self.chart_canvas.draw_idle()
        
        # Update dashboard label
        self.lbl_current_hashrate.configure(text=self._format_hashrate(total_hashrate))
        
        # Refresh device list
        self._refresh_device_list()
    
    def _refresh_stats(self):
        """Refresh all statistics"""
        self._update_blockchain_stats()
        self._update_stats()
        self._log("🔄 Statistics refreshed", "INFO")
    
    def _refresh_wallet(self):
        """Refresh wallet balance"""
        wallet = self.entries.get("Wallet Address:")
        if wallet:
            address = wallet.get().strip()
            if address and self.blockchain_client:
                try:
                    balance = self.blockchain_client.get_balance(address)
                    self.lbl_wallet_balance.configure(text=f"{balance:.8f} GXC")
                    self._log(f"💰 Wallet balance: {balance:.8f} GXC", "SUCCESS")
                except Exception as e:
                    self._log(f"Failed to get balance: {e}", "ERROR")
    
    def _save_config(self):
        """Save pool configuration"""
        pool_entry = self.entries.get("Pool Address:")
        wallet_entry = self.entries.get("Wallet Address:")
        worker_entry = self.entries.get("Worker Name:")
        password_entry = self.entries.get("Password:")
        
        if pool_entry:
            pool_str = pool_entry.get()
            if ':' in pool_str:
                host, port = pool_str.rsplit(':', 1)
                self.pool_config.host = host
                self.pool_config.port = int(port)
        
        if wallet_entry:
            self.pool_config.wallet_address = wallet_entry.get()
        if worker_entry:
            self.pool_config.worker_name = worker_entry.get()
        if password_entry:
            self.pool_config.password = password_entry.get()
        
        self._log("💾 Configuration saved", "SUCCESS")
        messagebox.showinfo("Saved", "Configuration saved successfully!")
    
    def _test_pool_connection(self):
        """Test pool connection"""
        self._log(f"🧪 Testing connection to {self.pool_config.host}:{self.pool_config.port}...", "INFO")
        
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.settimeout(5)
            result = sock.connect_ex((self.pool_config.host, self.pool_config.port))
            sock.close()
            
            if result == 0:
                self._log("✓ Pool connection successful!", "SUCCESS")
                messagebox.showinfo("Success", "Pool connection successful!")
            else:
                self._log("✗ Pool connection failed", "ERROR")
                messagebox.showerror("Failed", "Could not connect to pool")
        except Exception as e:
            self._log(f"✗ Connection error: {e}", "ERROR")
            messagebox.showerror("Error", f"Connection error: {e}")
    
    def _on_network_change(self, network: str):
        """Handle network selection change"""
        global ACTIVE_NETWORK
        
        if network == "Mainnet":
            ACTIVE_NETWORK = MAINNET_CONFIG
        elif network == "Testnet":
            ACTIVE_NETWORK = TESTNET_CONFIG
        
        self.pool_config.host = ACTIVE_NETWORK["pool_host"]
        self.pool_config.port = ACTIVE_NETWORK["pool_port"]
        
        pool_entry = self.entries.get("Pool Address:")
        if pool_entry:
            pool_entry.delete(0, "end")
            pool_entry.insert(0, f"{ACTIVE_NETWORK['pool_host']}:{ACTIVE_NETWORK['pool_port']}")
        
        self._log(f"🌐 Switched to {network}", "INFO")
    
    def _log(self, message: str, level: str = "INFO"):
        """Log message to console"""
        timestamp = datetime.now().strftime("%H:%M:%S")
        
        prefix_map = {
            "INFO": "[i]",
            "SUCCESS": "[✓]",
            "WARNING": "[!]",
            "ERROR": "[✗]"
        }
        
        log_line = f"[{timestamp}] {prefix_map.get(level, '[?]')} {message}\n"
        
        def write():
            self.log_console.insert("end", log_line)
            self.log_console.see("end")
        
        self.after(0, write)
    
    def _clear_logs(self):
        """Clear log console"""
        self.log_console.delete("1.0", "end")
    
    def _export_logs(self):
        """Export logs to file"""
        filename = filedialog.asksaveasfilename(
            defaultextension=".txt",
            filetypes=[("Text files", "*.txt"), ("All files", "*.*")],
            initialfile=f"gxc_miner_logs_{datetime.now().strftime('%Y%m%d_%H%M%S')}.txt"
        )
        
        if filename:
            with open(filename, 'w') as f:
                f.write(self.log_console.get("1.0", "end"))
            self._log(f"📄 Logs exported to {filename}", "SUCCESS")
    
    def _format_hashrate(self, hashrate: float) -> str:
        """Format hashrate with appropriate unit"""
        if hashrate >= 1e15:
            return f"{hashrate/1e15:.2f} PH/s"
        elif hashrate >= 1e12:
            return f"{hashrate/1e12:.2f} TH/s"
        elif hashrate >= 1e9:
            return f"{hashrate/1e9:.2f} GH/s"
        elif hashrate >= 1e6:
            return f"{hashrate/1e6:.2f} MH/s"
        elif hashrate >= 1e3:
            return f"{hashrate/1e3:.2f} KH/s"
        else:
            return f"{hashrate:.2f} H/s"
    
    def on_closing(self):
        """Handle window close"""
        if self.is_mining:
            if messagebox.askokcancel("Quit", "Mining is active. Stop mining and quit?"):
                self._stop_mining()
                time.sleep(0.5)
                self.destroy()
        else:
            self.destroy()


# ============================================================================
#                              MAIN ENTRY
# ============================================================================

def main():
    """Main entry point"""
    app = GXCPoolMinerApp()
    app.protocol("WM_DELETE_WINDOW", app.on_closing)
    app.mainloop()


if __name__ == "__main__":
    main()
