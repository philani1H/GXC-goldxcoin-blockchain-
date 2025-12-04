"""
GXC Pool Miner
Universal Mining Management System for GXC Blockchain
"""

__version__ = "2.0.0"
__author__ = "GXC Blockchain"
__license__ = "MIT"

from .gxc_pool_miner import (
    GXCPoolMinerApp,
    GXCBlockchainClient,
    StratumClient,
    MiningEngine,
    DeviceDetector,
    MiningDevice,
    PoolConfig,
    BlockchainStats,
    MiningStats,
    main,
)

__all__ = [
    "GXCPoolMinerApp",
    "GXCBlockchainClient",
    "StratumClient",
    "MiningEngine",
    "DeviceDetector",
    "MiningDevice",
    "PoolConfig",
    "BlockchainStats",
    "MiningStats",
    "main",
]
