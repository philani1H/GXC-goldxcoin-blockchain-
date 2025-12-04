"""
GXC Miner Suite
SHA256 • Ethash • GxHash - Real Blockchain Mining
"""

__version__ = "2.0.0"
__author__ = "GXC Blockchain"

# Main unified miner
from .gxc_miner import (
    GXCMinerApp,
    GXCBlockchainClient,
    StratumClient,
    MiningEngine,
    DeviceDetector,
    SHA256Algorithm,
    EthashAlgorithm,
    GxHashAlgorithm,
    main as unified_miner
)

# Individual miners
from .sha256_miner import SHA256MinerGUI, SHA256Miner, main as sha256_main
from .ethash_miner import EthashMinerGUI, EthashMiner, main as ethash_main

__all__ = [
    'GXCMinerApp',
    'GXCBlockchainClient',
    'StratumClient', 
    'MiningEngine',
    'DeviceDetector',
    'SHA256Algorithm',
    'EthashAlgorithm',
    'GxHashAlgorithm',
    'SHA256MinerGUI',
    'SHA256Miner',
    'EthashMinerGUI',
    'EthashMiner',
    'unified_miner',
    'sha256_main',
    'ethash_main'
]
