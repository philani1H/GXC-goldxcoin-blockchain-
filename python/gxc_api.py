#!/usr/bin/env python3
"""
GXC Blockchain Python API
Production-ready Python interface for the GXC blockchain.
Supports wallet management, mining, staking, and full blockchain interaction.
"""

import json
import requests
import hashlib
import hmac
import base64
import time
import threading
import websocket
import queue
from typing import Dict, List, Optional, Any, Union, Callable
from dataclasses import dataclass, asdict
from decimal import Decimal
import logging
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.hazmat.primitives.kdf.pbkdf2 import PBKDF2HMAC
from cryptography.fernet import Fernet
import mnemonic
import bip32utils
import socket
import struct
import uuid

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

@dataclass
class NetworkConfig:
    """Network configuration for mainnet/testnet"""
    is_testnet: bool = False
    rpc_host: str = "127.0.0.1"
    rpc_port: int = 8332
    rpc_user: str = ""
    rpc_password: str = ""
    rpc_ssl: bool = False
    p2p_port: int = 8333
    websocket_port: int = 8081
    
    def __post_init__(self):
        if self.is_testnet:
            self.rpc_port = 18332
            self.p2p_port = 18333

@dataclass
class WalletInfo:
    """Wallet information structure"""
    address: str
    balance: Decimal
    unconfirmed_balance: Decimal
    staking_balance: Decimal
    tx_count: int
    hd_path: str
    is_encrypted: bool
    last_activity: int

@dataclass
class TransactionInfo:
    """Transaction information structure"""
    txid: str
    hash: str
    version: int
    size: int
    fee: Decimal
    confirmations: int
    time: int
    inputs: List[Dict]
    outputs: List[Dict]
    block_hash: Optional[str] = None
    block_time: Optional[int] = None

@dataclass
class BlockInfo:
    """Block information structure"""
    hash: str
    height: int
    version: int
    merkle_root: str
    time: int
    nonce: int
    difficulty: float
    size: int
    tx_count: int
    previous_hash: str
    next_hash: Optional[str]
    miner: str
    block_type: str
    confirmations: int

@dataclass
class MiningInfo:
    """Mining information structure"""
    blocks: int
    difficulty: float
    network_hashrate: float
    pooled_tx: int
    chain: str
    warnings: str
    current_block_size: int
    current_block_tx: int

@dataclass
class StakingInfo:
    """Staking information structure"""
    enabled: bool
    staking: bool
    errors: str
    weight: float
    net_stake_weight: float
    expected_time: int
    search_interval: int

@dataclass
class PeerInfo:
    """Peer information structure"""
    id: int
    addr: str
    services: str
    last_send: int
    last_recv: int
    bytes_sent: int
    bytes_recv: int
    conn_time: int
    ping_time: float
    version: int
    subver: str
    inbound: bool
    start_height: int

class GXCRPCError(Exception):
    """Custom exception for GXC RPC errors"""
    def __init__(self, error_code: int, message: str):
        self.error_code = error_code
        super().__init__(f"RPC Error {error_code}: {message}")

class GXCWallet:
    """GXC Wallet management class"""
    
    def __init__(self, api_client: 'GXCAPI'):
        self.api = api_client
        self.mnemonic_generator = mnemonic.Mnemonic("english")
        
    def create_wallet(self, name: str, password: str, mnemonic_phrase: Optional[str] = None) -> Dict[str, Any]:
        """Create a new HD wallet with mnemonic seed"""
        if not mnemonic_phrase:
            mnemonic_phrase = self.mnemonic_generator.generate(strength=256)
        
        # Validate mnemonic
        if not self.mnemonic_generator.check(mnemonic_phrase):
            raise ValueError("Invalid mnemonic phrase")
        
        # Create wallet via RPC
        result = self.api.rpc_call("createwallet", [name, password, mnemonic_phrase])
        return {
            "wallet_name": name,
            "mnemonic": mnemonic_phrase,
            "address": result.get("address"),
            "success": True
        }
    
    def load_wallet(self, name: str, password: str) -> Dict[str, Any]:
        """Load an existing wallet"""
        return self.api.rpc_call("loadwallet", [name, password])
    
    def import_mnemonic(self, mnemonic_phrase: str, password: str, name: str = None) -> Dict[str, Any]:
        """Import wallet from mnemonic phrase"""
        if not self.mnemonic_generator.check(mnemonic_phrase):
            raise ValueError("Invalid mnemonic phrase")
        
        if not name:
            name = f"imported_wallet_{int(time.time())}"
        
        return self.api.rpc_call("importmnemonic", [mnemonic_phrase, password, name])
    
    def get_balance(self, address: str = None) -> Decimal:
        """Get wallet balance"""
        result = self.api.rpc_call("getbalance", [address] if address else [])
        return Decimal(str(result))
    
    def get_unconfirmed_balance(self) -> Decimal:
        """Get unconfirmed balance"""
        result = self.api.rpc_call("getunconfirmedbalance")
        return Decimal(str(result))
    
    def get_new_address(self, label: str = "") -> str:
        """Generate a new address"""
        return self.api.rpc_call("getnewaddress", [label])
    
    def send_to_address(self, address: str, amount: Union[Decimal, float, str], 
                       comment: str = "", fee: Optional[Union[Decimal, float]] = None) -> str:
        """Send GXC to an address"""
        params = [address, float(amount)]
        if comment:
            params.append(comment)
        if fee is not None:
            params.append(float(fee))
        
        return self.api.rpc_call("sendtoaddress", params)
    
    def send_many(self, recipients: Dict[str, Union[Decimal, float]], 
                  comment: str = "", fee: Optional[Union[Decimal, float]] = None) -> str:
        """Send to multiple addresses"""
        # Convert amounts to float for JSON serialization
        recipients_float = {addr: float(amount) for addr, amount in recipients.items()}
        
        params = [recipients_float]
        if comment:
            params.append(comment)
        if fee is not None:
            params.append(float(fee))
        
        return self.api.rpc_call("sendmany", params)
    
    def list_transactions(self, count: int = 10, from_: int = 0) -> List[TransactionInfo]:
        """List recent transactions"""
        result = self.api.rpc_call("listtransactions", ["*", count, from_])
        transactions = []
        
        for tx_data in result:
            tx = TransactionInfo(
                txid=tx_data["txid"],
                hash=tx_data.get("hash", tx_data["txid"]),
                version=tx_data.get("version", 1),
                size=tx_data.get("size", 0),
                fee=Decimal(str(tx_data.get("fee", 0))),
                confirmations=tx_data.get("confirmations", 0),
                time=tx_data.get("time", 0),
                inputs=tx_data.get("inputs", []),
                outputs=tx_data.get("outputs", []),
                block_hash=tx_data.get("blockhash"),
                block_time=tx_data.get("blocktime")
            )
            transactions.append(tx)
        
        return transactions
    
    def get_transaction(self, txid: str) -> TransactionInfo:
        """Get detailed transaction information"""
        result = self.api.rpc_call("gettransaction", [txid])
        
        return TransactionInfo(
            txid=result["txid"],
            hash=result.get("hash", result["txid"]),
            version=result.get("version", 1),
            size=result.get("size", 0),
            fee=Decimal(str(result.get("fee", 0))),
            confirmations=result.get("confirmations", 0),
            time=result.get("time", 0),
            inputs=result.get("vin", []),
            outputs=result.get("vout", []),
            block_hash=result.get("blockhash"),
            block_time=result.get("blocktime")
        )
    
    def encrypt_wallet(self, passphrase: str) -> bool:
        """Encrypt the wallet"""
        try:
            self.api.rpc_call("encryptwallet", [passphrase])
            return True
        except GXCRPCError:
            return False
    
    def unlock_wallet(self, passphrase: str, timeout: int = 300) -> bool:
        """Unlock the wallet for transactions"""
        try:
            self.api.rpc_call("walletpassphrase", [passphrase, timeout])
            return True
        except GXCRPCError:
            return False
    
    def lock_wallet(self) -> bool:
        """Lock the wallet"""
        try:
            self.api.rpc_call("walletlock")
            return True
        except GXCRPCError:
            return False
    
    def backup_wallet(self, destination: str) -> bool:
        """Backup wallet to file"""
        try:
            self.api.rpc_call("backupwallet", [destination])
            return True
        except GXCRPCError:
            return False

class GXCMining:
    """GXC Mining management class"""
    
    def __init__(self, api_client: 'GXCAPI'):
        self.api = api_client
    
    def get_mining_info(self) -> MiningInfo:
        """Get current mining information"""
        result = self.api.rpc_call("getmininginfo")
        
        return MiningInfo(
            blocks=result["blocks"],
            difficulty=result["difficulty"],
            network_hashrate=result["networkhashps"],
            pooled_tx=result["pooledtx"],
            chain=result["chain"],
            warnings=result.get("warnings", ""),
            current_block_size=result.get("currentblocksize", 0),
            current_block_tx=result.get("currentblocktx", 0)
        )
    
    def get_block_template(self, address: str = None) -> Dict[str, Any]:
        """Get block template for mining"""
        params = []
        if address:
            params.append({"mode": "template", "address": address})
        
        return self.api.rpc_call("getblocktemplate", params)
    
    def submit_block(self, block_hex: str) -> bool:
        """Submit a mined block"""
        try:
            self.api.rpc_call("submitblock", [block_hex])
            return True
        except GXCRPCError:
            return False
    
    def get_work(self) -> Dict[str, Any]:
        """Get work for mining (deprecated, use getblocktemplate)"""
        return self.api.rpc_call("getwork")
    
    def submit_work(self, data: str, target: str, nonce: str) -> bool:
        """Submit work (deprecated, use submitblock)"""
        try:
            result = self.api.rpc_call("submitwork", [data, target, nonce])
            return result
        except GXCRPCError:
            return False
    
    def get_network_hashrate(self, blocks: int = 120) -> float:
        """Get network hash rate"""
        return self.api.rpc_call("getnetworkhashps", [blocks])
    
    def prioritise_transaction(self, txid: str, priority_delta: float, fee_delta: int) -> bool:
        """Prioritise a transaction for mining"""
        try:
            self.api.rpc_call("prioritisetransaction", [txid, priority_delta, fee_delta])
            return True
        except GXCRPCError:
            return False

class GXCStaking:
    """GXC Staking management class"""
    
    def __init__(self, api_client: 'GXCAPI'):
        self.api = api_client
    
    def get_staking_info(self) -> StakingInfo:
        """Get current staking information"""
        result = self.api.rpc_call("getstakinginfo")
        
        return StakingInfo(
            enabled=result.get("enabled", False),
            staking=result.get("staking", False),
            errors=result.get("errors", ""),
            weight=result.get("weight", 0),
            net_stake_weight=result.get("netstakeweight", 0),
            expected_time=result.get("expectedtime", 0),
            search_interval=result.get("search-interval", 0)
        )
    
    def set_staking(self, enabled: bool) -> bool:
        """Enable or disable staking"""
        try:
            self.api.rpc_call("setstaking", [enabled])
            return True
        except GXCRPCError:
            return False
    
    def create_stake(self, amount: Union[Decimal, float], days: int, address: str = None) -> str:
        """Create a new stake"""
        params = [float(amount), days]
        if address:
            params.append(address)
        
        return self.api.rpc_call("createstake", params)
    
    def withdraw_stake(self, stake_id: str) -> str:
        """Withdraw a stake"""
        return self.api.rpc_call("withdrawstake", [stake_id])
    
    def list_staking(self) -> List[Dict[str, Any]]:
        """List all stakes"""
        return self.api.rpc_call("liststaking")
    
    def get_validators(self) -> List[Dict[str, Any]]:
        """Get list of validators"""
        return self.api.rpc_call("getvalidators")
    
    def get_validator_info(self, address: str) -> Dict[str, Any]:
        """Get validator information"""
        return self.api.rpc_call("getvalidatorinfo", [address])

class GXCGovernance:
    """GXC Governance management class"""
    
    def __init__(self, api_client: 'GXCAPI'):
        self.api = api_client
    
    def submit_proposal(self, title: str, description: str, 
                       proposal_type: str, value: float = None) -> str:
        """Submit a governance proposal"""
        params = [title, description, proposal_type]
        if value is not None:
            params.append(value)
        
        return self.api.rpc_call("submitproposal", params)
    
    def vote(self, proposal_id: str, vote: str, weight: float = None) -> str:
        """Vote on a proposal"""
        params = [proposal_id, vote]
        if weight is not None:
            params.append(weight)
        
        return self.api.rpc_call("vote", params)
    
    def get_proposal(self, proposal_id: str) -> Dict[str, Any]:
        """Get proposal details"""
        return self.api.rpc_call("getproposal", [proposal_id])
    
    def list_proposals(self, status: str = "active") -> List[Dict[str, Any]]:
        """List proposals by status"""
        return self.api.rpc_call("listproposals", [status])
    
    def list_votes(self, proposal_id: str = None) -> List[Dict[str, Any]]:
        """List votes for proposals"""
        params = [proposal_id] if proposal_id else []
        return self.api.rpc_call("listvotes", params)

class GXCWebSocket:
    """WebSocket client for real-time updates"""
    
    def __init__(self, host: str = "127.0.0.1", port: int = 8081):
        self.host = host
        self.port = port
        self.ws = None
        self.callbacks = {
            'new_block': [],
            'new_transaction': [],
            'price_update': [],
            'mining_update': []
        }
        self.running = False
        self.thread = None
    
    def connect(self):
        """Connect to WebSocket server"""
        url = f"ws://{self.host}:{self.port}"
        self.ws = websocket.WebSocketApp(
            url,
            on_message=self._on_message,
            on_error=self._on_error,
            on_close=self._on_close,
            on_open=self._on_open
        )
        
        self.running = True
        self.thread = threading.Thread(target=self.ws.run_forever)
        self.thread.daemon = True
        self.thread.start()
    
    def disconnect(self):
        """Disconnect from WebSocket server"""
        self.running = False
        if self.ws:
            self.ws.close()
        if self.thread:
            self.thread.join()
    
    def subscribe(self, event_type: str, callback: Callable):
        """Subscribe to events"""
        if event_type in self.callbacks:
            self.callbacks[event_type].append(callback)
    
    def unsubscribe(self, event_type: str, callback: Callable):
        """Unsubscribe from events"""
        if event_type in self.callbacks and callback in self.callbacks[event_type]:
            self.callbacks[event_type].remove(callback)
    
    def _on_message(self, ws, message):
        """Handle incoming WebSocket messages"""
        try:
            data = json.loads(message)
            event_type = data.get('type')
            
            if event_type in self.callbacks:
                for callback in self.callbacks[event_type]:
                    try:
                        callback(data)
                    except Exception as e:
                        logger.error(f"Error in callback: {e}")
        except json.JSONDecodeError:
            logger.error(f"Invalid JSON message: {message}")
    
    def _on_error(self, ws, error):
        """Handle WebSocket errors"""
        logger.error(f"WebSocket error: {error}")
    
    def _on_close(self, ws, close_status_code, close_msg):
        """Handle WebSocket close"""
        logger.info("WebSocket connection closed")
    
    def _on_open(self, ws):
        """Handle WebSocket open"""
        logger.info("WebSocket connection opened")

class GXCAPI:
    """Main GXC API client"""
    
    def __init__(self, config: NetworkConfig = None):
        self.config = config or NetworkConfig()
        self.session = requests.Session()
        
        # Setup authentication if provided
        if self.config.rpc_user and self.config.rpc_password:
            self.session.auth = (self.config.rpc_user, self.config.rpc_password)
        
        # Initialize component classes
        self.wallet = GXCWallet(self)
        self.mining = GXCMining(self)
        self.staking = GXCStaking(self)
        self.governance = GXCGovernance(self)
        self.websocket = GXCWebSocket(self.config.rpc_host, self.config.websocket_port)
        
        self._request_id = 0
    
    def _get_rpc_url(self) -> str:
        """Get RPC URL"""
        protocol = "https" if self.config.rpc_ssl else "http"
        return f"{protocol}://{self.config.rpc_host}:{self.config.rpc_port}"
    
    def rpc_call(self, method: str, params: List[Any] = None) -> Any:
        """Make RPC call to GXC node"""
        self._request_id += 1
        
        payload = {
            "jsonrpc": "2.0",
            "method": method,
            "params": params or [],
            "id": self._request_id
        }
        
        try:
            response = self.session.post(
                self._get_rpc_url(),
                json=payload,
                timeout=30
            )
            response.raise_for_status()
            
            result = response.json()
            
            if "error" in result and result["error"] is not None:
                error = result["error"]
                raise GXCRPCError(error["code"], error["message"])
            
            return result.get("result")
            
        except requests.RequestException as e:
            raise GXCRPCError(-1, f"Network error: {str(e)}")
        except json.JSONDecodeError as e:
            raise GXCRPCError(-2, f"JSON decode error: {str(e)}")
    
    # Blockchain information methods
    def get_blockchain_info(self) -> Dict[str, Any]:
        """Get blockchain information"""
        return self.rpc_call("getblockchaininfo")
    
    def get_best_block_hash(self) -> str:
        """Get best block hash"""
        return self.rpc_call("getbestblockhash")
    
    def get_block_count(self) -> int:
        """Get current block count"""
        return self.rpc_call("getblockcount")
    
    def get_block(self, block_hash: str, verbosity: int = 1) -> Union[str, Dict[str, Any]]:
        """Get block by hash"""
        return self.rpc_call("getblock", [block_hash, verbosity])
    
    def get_block_hash(self, height: int) -> str:
        """Get block hash by height"""
        return self.rpc_call("getblockhash", [height])
    
    def get_block_header(self, block_hash: str, verbose: bool = True) -> Union[str, Dict[str, Any]]:
        """Get block header"""
        return self.rpc_call("getblockheader", [block_hash, verbose])
    
    def get_difficulty(self) -> float:
        """Get current difficulty"""
        return self.rpc_call("getdifficulty")
    
    def get_mempool_info(self) -> Dict[str, Any]:
        """Get mempool information"""
        return self.rpc_call("getmempoolinfo")
    
    def get_raw_mempool(self, verbose: bool = False) -> Union[List[str], Dict[str, Any]]:
        """Get raw mempool"""
        return self.rpc_call("getrawmempool", [verbose])
    
    # Transaction methods
    def get_raw_transaction(self, txid: str, verbose: bool = True) -> Union[str, Dict[str, Any]]:
        """Get raw transaction"""
        return self.rpc_call("getrawtransaction", [txid, verbose])
    
    def send_raw_transaction(self, hex_string: str) -> str:
        """Send raw transaction"""
        return self.rpc_call("sendrawtransaction", [hex_string])
    
    def create_raw_transaction(self, inputs: List[Dict], outputs: Dict[str, float]) -> str:
        """Create raw transaction"""
        return self.rpc_call("createrawtransaction", [inputs, outputs])
    
    def sign_raw_transaction(self, hex_string: str, prevtxs: List[Dict] = None,
                           privkeys: List[str] = None, sighashtype: str = "ALL") -> Dict[str, Any]:
        """Sign raw transaction"""
        params = [hex_string]
        if prevtxs is not None:
            params.append(prevtxs)
        if privkeys is not None:
            params.append(privkeys)
        if sighashtype != "ALL":
            params.append(sighashtype)
        
        return self.rpc_call("signrawtransaction", params)
    
    # Network methods
    def get_network_info(self) -> Dict[str, Any]:
        """Get network information"""
        return self.rpc_call("getnetworkinfo")
    
    def get_peer_info(self) -> List[PeerInfo]:
        """Get peer information"""
        result = self.rpc_call("getpeerinfo")
        peers = []
        
        for peer_data in result:
            peer = PeerInfo(
                id=peer_data["id"],
                addr=peer_data["addr"],
                services=peer_data["services"],
                last_send=peer_data["lastsend"],
                last_recv=peer_data["lastrecv"],
                bytes_sent=peer_data["bytessent"],
                bytes_recv=peer_data["bytesrecv"],
                conn_time=peer_data["conntime"],
                ping_time=peer_data.get("pingtime", 0),
                version=peer_data["version"],
                subver=peer_data["subver"],
                inbound=peer_data["inbound"],
                start_height=peer_data["startingheight"]
            )
            peers.append(peer)
        
        return peers
    
    def get_connection_count(self) -> int:
        """Get connection count"""
        return self.rpc_call("getconnectioncount")
    
    def add_node(self, node: str, command: str = "add") -> None:
        """Add/remove/onetry node"""
        self.rpc_call("addnode", [node, command])
    
    # Oracle methods
    def get_price(self, symbol: str = "GXC") -> Dict[str, Any]:
        """Get current price from oracle"""
        return self.rpc_call("getprice", [symbol])
    
    def get_price_history(self, symbol: str = "GXC", days: int = 30) -> List[Dict[str, Any]]:
        """Get price history"""
        return self.rpc_call("getpricehistory", [symbol, days])
    
    def submit_price(self, symbol: str, price: float, signature: str) -> str:
        """Submit price to oracle (for authorized oracles)"""
        return self.rpc_call("submitprice", [symbol, price, signature])
    
    # Gold token methods
    def get_gold_balance(self, address: str = None) -> Decimal:
        """Get GXC-G (gold token) balance"""
        result = self.rpc_call("getgoldbalance", [address] if address else [])
        return Decimal(str(result))
    
    def transfer_gold_tokens(self, to_address: str, amount: Union[Decimal, float]) -> str:
        """Transfer gold tokens"""
        return self.rpc_call("transfergoldtokens", [to_address, float(amount)])
    
    def get_gold_reserves(self) -> Dict[str, Any]:
        """Get gold reserves information"""
        return self.rpc_call("getgoldreserves")
    
    # Utility methods
    def validate_address(self, address: str) -> Dict[str, Any]:
        """Validate an address"""
        return self.rpc_call("validateaddress", [address])
    
    def estimate_fee(self, nblocks: int = 6) -> float:
        """Estimate transaction fee"""
        return self.rpc_call("estimatefee", [nblocks])
    
    def get_info(self) -> Dict[str, Any]:
        """Get general information (deprecated, use getblockchaininfo/getnetworkinfo)"""
        return self.rpc_call("getinfo")
    
    def ping(self) -> None:
        """Ping the node"""
        self.rpc_call("ping")
    
    def uptime(self) -> int:
        """Get node uptime in seconds"""
        return self.rpc_call("uptime")
    
    # Convenience methods
    def wait_for_confirmations(self, txid: str, confirmations: int = 1, 
                              timeout: int = 300, poll_interval: int = 5) -> bool:
        """Wait for transaction confirmations"""
        start_time = time.time()
        
        while time.time() - start_time < timeout:
            try:
                tx_info = self.wallet.get_transaction(txid)
                if tx_info.confirmations >= confirmations:
                    return True
            except GXCRPCError:
                pass
            
            time.sleep(poll_interval)
        
        return False
    
    def wait_for_block(self, height: int, timeout: int = 600, poll_interval: int = 10) -> bool:
        """Wait for specific block height"""
        start_time = time.time()
        
        while time.time() - start_time < timeout:
            current_height = self.get_block_count()
            if current_height >= height:
                return True
            
            time.sleep(poll_interval)
        
        return False

# Convenience factory functions
def create_mainnet_client(rpc_user: str = "", rpc_password: str = "", 
                         host: str = "127.0.0.1") -> GXCAPI:
    """Create mainnet API client"""
    config = NetworkConfig(
        is_testnet=False,
        rpc_host=host,
        rpc_user=rpc_user,
        rpc_password=rpc_password
    )
    return GXCAPI(config)

def create_testnet_client(rpc_user: str = "", rpc_password: str = "", 
                         host: str = "127.0.0.1") -> GXCAPI:
    """Create testnet API client"""
    config = NetworkConfig(
        is_testnet=True,
        rpc_host=host,
        rpc_user=rpc_user,
        rpc_password=rpc_password
    )
    return GXCAPI(config)

# Example usage
if __name__ == "__main__":
    # Create API client
    api = create_mainnet_client("rpcuser", "rpcpassword")
    
    try:
        # Get blockchain info
        info = api.get_blockchain_info()
        print(f"Chain: {info['chain']}")
        print(f"Blocks: {info['blocks']}")
        print(f"Difficulty: {info['difficulty']}")
        
        # Get wallet balance
        balance = api.wallet.get_balance()
        print(f"Balance: {balance} GXC")
        
        # Get mining info
        mining_info = api.mining.get_mining_info()
        print(f"Network Hash Rate: {mining_info.network_hashrate} H/s")
        
        # Get staking info
        staking_info = api.staking.get_staking_info()
        print(f"Staking Enabled: {staking_info.enabled}")
        
    except GXCRPCError as e:
        print(f"RPC Error: {e}")
    except Exception as e:
        print(f"Error: {e}")