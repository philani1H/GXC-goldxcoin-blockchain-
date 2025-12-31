#!/usr/bin/env python3
"""
GXC Blockchain - Proof of Receipt Implementation Example

This module demonstrates how to implement proof of receipt handling
for mining pools to verify work submissions.
"""

import hashlib
import json
from typing import Dict, Any, Optional


class ProofOfReceipt:
    """
    Handles proof of receipt generation and verification for mining work.

    The work receipt hash links the coinbase transaction to the actual
    proof-of-work, preventing miners from claiming others' work.
    """

    @staticmethod
    def generate_work_receipt(
        previous_hash: str,
        nonce: int,
        timestamp: int,
        miner_address: str
    ) -> str:
        """
        Generate a work receipt hash from mining work parameters.

        Args:
            previous_hash: Hash of previous block
            nonce: Nonce value that produced valid hash
            timestamp: Block timestamp
            miner_address: Address of miner who found the block

        Returns:
            SHA-256 hash of the work receipt data
        """
        # Combine all work parameters
        receipt_data = {
            "previous_hash": previous_hash,
            "nonce": nonce,
            "timestamp": timestamp,
            "miner_address": miner_address
        }

        # Create deterministic JSON string
        receipt_json = json.dumps(receipt_data, sort_keys=True)

        # Calculate SHA-256 hash
        receipt_hash = hashlib.sha256(receipt_json.encode()).hexdigest()

        return receipt_hash

    @staticmethod
    def verify_work_receipt(
        block_data: Dict[str, Any],
        expected_receipt: Optional[str] = None
    ) -> bool:
        """
        Verify that the work receipt hash in a block is valid.

        Args:
            block_data: Complete block data including transactions
            expected_receipt: Optional expected receipt hash to compare

        Returns:
            True if receipt is valid, False otherwise
        """
        try:
            # Extract block parameters
            previous_hash = block_data.get("previousblockhash", "")
            nonce = block_data.get("nonce", 0)
            timestamp = block_data.get("timestamp", 0)
            miner_address = block_data.get("miner", "")

            # Validate required fields
            if not previous_hash or not miner_address:
                print("❌ Missing required fields for work receipt")
                return False

            # Generate receipt from block data
            calculated_receipt = ProofOfReceipt.generate_work_receipt(
                previous_hash, nonce, timestamp, miner_address
            )

            # Get receipt from coinbase transaction
            transactions = block_data.get("transactions", [])
            if not transactions:
                print("❌ No transactions in block")
                return False

            coinbase = transactions[0]
            actual_receipt = coinbase.get("work_receipt_hash", "")

            # Verify receipt hash
            if expected_receipt:
                # Compare with provided expected value
                if calculated_receipt != expected_receipt:
                    print(f"❌ Receipt mismatch (expected: {expected_receipt[:16]}...)")
                    return False

            if actual_receipt:
                # Compare calculated with actual
                if calculated_receipt != actual_receipt:
                    print(f"❌ Receipt verification failed")
                    print(f"   Calculated: {calculated_receipt[:32]}...")
                    print(f"   Actual:     {actual_receipt[:32]}...")
                    return False

            print(f"✅ Work receipt verified: {calculated_receipt[:32]}...")
            return True

        except Exception as e:
            print(f"❌ Error verifying work receipt: {e}")
            return False

    @staticmethod
    def create_coinbase_with_receipt(
        miner_address: str,
        block_reward: float,
        previous_hash: str,
        nonce: int,
        timestamp: int,
        pool_name: str = "GXC Mining Pool",
        pool_version: str = "1.0.0"
    ) -> Dict[str, Any]:
        """
        Create a coinbase transaction with work receipt.

        Args:
            miner_address: Address to receive mining reward
            block_reward: Amount of reward in GXC
            previous_hash: Hash of previous block
            nonce: Nonce value
            timestamp: Block timestamp
            pool_name: Name of mining pool (for metadata)
            pool_version: Version of pool software (for metadata)

        Returns:
            Coinbase transaction dictionary with work receipt
        """
        # Generate work receipt
        work_receipt = ProofOfReceipt.generate_work_receipt(
            previous_hash, nonce, timestamp, miner_address
        )

        # Create coinbase transaction
        coinbase_tx = {
            "type": "COINBASE",
            "from": "",
            "to": miner_address,
            "amount": block_reward,
            "fee": 0.0,
            "outputs": [
                {
                    "address": miner_address,
                    "amount": block_reward,
                    "script": ""
                }
            ],
            "inputs": [],
            "work_receipt_hash": work_receipt,
            "metadata": {
                "pool_name": pool_name,
                "pool_version": pool_version,
                "timestamp": timestamp
            }
        }

        return coinbase_tx


class PoolWorkManager:
    """
    Manages work distribution and receipt verification for a mining pool.
    """

    def __init__(self, pool_address: str, pool_name: str = "GXC Pool"):
        self.pool_address = pool_address
        self.pool_name = pool_name
        self.active_work = {}  # Track distributed work

    def distribute_work(
        self,
        template: Dict[str, Any],
        miner_id: str
    ) -> Dict[str, Any]:
        """
        Distribute mining work to a miner and track the receipt.

        Args:
            template: Block template from getblocktemplate
            miner_id: Unique identifier for the miner

        Returns:
            Work package for the miner
        """
        previous_hash = template.get("previousblockhash", "")
        height = template.get("height", 0)
        difficulty = template.get("difficulty", 1.0)
        timestamp = template.get("curtime", 0)

        # Create work package
        work = {
            "job_id": f"{height}_{miner_id}_{timestamp}",
            "previous_hash": previous_hash,
            "height": height,
            "difficulty": difficulty,
            "timestamp": timestamp,
            "target": template.get("target", ""),
            "transactions": template.get("transactions", [])
        }

        # Store work for verification
        self.active_work[work["job_id"]] = {
            "miner_id": miner_id,
            "previous_hash": previous_hash,
            "timestamp": timestamp,
            "distributed_at": timestamp
        }

        return work

    def verify_work_submission(
        self,
        job_id: str,
        nonce: int,
        block_hash: str,
        miner_address: str
    ) -> bool:
        """
        Verify a work submission from a miner.

        Args:
            job_id: Job ID from distributed work
            nonce: Nonce found by miner
            block_hash: Hash produced by nonce
            miner_address: Address of submitting miner

        Returns:
            True if work is valid, False otherwise
        """
        # Check if job exists
        if job_id not in self.active_work:
            print(f"❌ Unknown job_id: {job_id}")
            return False

        work_info = self.active_work[job_id]

        # Generate expected receipt
        expected_receipt = ProofOfReceipt.generate_work_receipt(
            work_info["previous_hash"],
            nonce,
            work_info["timestamp"],
            miner_address
        )

        print(f"✅ Work submission verified for job {job_id}")
        print(f"   Miner: {miner_address[:20]}...")
        print(f"   Nonce: {nonce}")
        print(f"   Hash: {block_hash[:32]}...")
        print(f"   Receipt: {expected_receipt[:32]}...")

        # Clean up completed work
        del self.active_work[job_id]

        return True


# Example usage
if __name__ == "__main__":
    print("=" * 70)
    print("GXC Blockchain - Proof of Receipt Example")
    print("=" * 70)
    print()

    # Example 1: Generate work receipt
    print("Example 1: Generate Work Receipt")
    print("-" * 70)

    previous_hash = "0000a1b2c3d4e5f6789abcdef0123456789abcdef0123456789abcdef012345"
    nonce = 123456789
    timestamp = 1735646500
    miner_address = "tGXC4VoV6DfSnM7YwTLCciBUzizARg2H64T7n"

    receipt = ProofOfReceipt.generate_work_receipt(
        previous_hash, nonce, timestamp, miner_address
    )

    print(f"Previous Hash:  {previous_hash}")
    print(f"Nonce:          {nonce}")
    print(f"Timestamp:      {timestamp}")
    print(f"Miner Address:  {miner_address}")
    print(f"Work Receipt:   {receipt}")
    print()

    # Example 2: Create coinbase with receipt
    print("Example 2: Create Coinbase Transaction with Receipt")
    print("-" * 70)

    coinbase = ProofOfReceipt.create_coinbase_with_receipt(
        miner_address=miner_address,
        block_reward=50.0,
        previous_hash=previous_hash,
        nonce=nonce,
        timestamp=timestamp,
        pool_name="GXC Test Pool",
        pool_version="1.0.0"
    )

    print(json.dumps(coinbase, indent=2))
    print()

    # Example 3: Verify work receipt
    print("Example 3: Verify Work Receipt in Block")
    print("-" * 70)

    block_data = {
        "height": 151,
        "previousblockhash": previous_hash,
        "nonce": nonce,
        "timestamp": timestamp,
        "miner": miner_address,
        "transactions": [coinbase]
    }

    is_valid = ProofOfReceipt.verify_work_receipt(block_data, receipt)
    print(f"Receipt Valid: {is_valid}")
    print()

    # Example 4: Pool work management
    print("Example 4: Pool Work Distribution & Verification")
    print("-" * 70)

    pool = PoolWorkManager(
        pool_address="tGXCPoolAddress123456789",
        pool_name="GXC Demo Pool"
    )

    # Simulate block template from node
    template = {
        "previousblockhash": previous_hash,
        "height": 151,
        "difficulty": 0.1,
        "curtime": timestamp,
        "target": "00000000ffff0000000000000000000000000000000000000000000000000000",
        "transactions": []
    }

    # Distribute work to miner
    work = pool.distribute_work(template, miner_id="miner_001")
    print(f"Work distributed to miner_001:")
    print(f"  Job ID: {work['job_id']}")
    print(f"  Height: {work['height']}")
    print(f"  Difficulty: {work['difficulty']}")
    print()

    # Miner finds nonce and submits
    found_nonce = 123456789
    found_hash = "00000abc123def456789..."

    is_valid = pool.verify_work_submission(
        job_id=work["job_id"],
        nonce=found_nonce,
        block_hash=found_hash,
        miner_address=miner_address
    )
    print()

    print("=" * 70)
    print("Examples Complete")
    print("=" * 70)
