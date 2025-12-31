#!/usr/bin/env python3
"""
Taint Propagation Algorithm Test

Tests the mathematical correctness of taint propagation:
τ(T_j) = Σ w_i · τ(T_i)
where w_i = value_from_T_i / total_input_value
"""

import json
from typing import Dict, List

class TaintPropagationTest:
    def __init__(self):
        self.transactions = {}
        self.taint_scores = {}
        
    def log(self, message: str):
        print(f"[TAINT TEST] {message}")
        
    def create_transaction(self, tx_hash: str, inputs: List[Dict], outputs: List[Dict]):
        """Create a transaction with inputs and outputs"""
        self.transactions[tx_hash] = {
            "hash": tx_hash,
            "inputs": inputs,
            "outputs": outputs,
            "total_input": sum(inp["amount"] for inp in inputs),
            "total_output": sum(out["amount"] for out in outputs)
        }
        
    def mark_as_stolen(self, tx_hash: str):
        """Mark transaction as stolen (τ = 1.0)"""
        self.taint_scores[tx_hash] = 1.0
        self.log(f"Marked {tx_hash} as stolen (τ = 1.0)")
        
    def calculate_taint(self, tx_hash: str) -> float:
        """
        Calculate taint for a transaction based on its inputs
        Formula: τ(T_j) = Σ w_i · τ(T_i)
        """
        if tx_hash in self.taint_scores:
            return self.taint_scores[tx_hash]
            
        tx = self.transactions.get(tx_hash)
        if not tx or not tx["inputs"]:
            return 0.0
            
        total_input_value = tx["total_input"]
        weighted_taint_sum = 0.0
        
        for inp in tx["inputs"]:
            input_tx_hash = inp["from_tx"]
            input_taint = self.taint_scores.get(input_tx_hash, 0.0)
            weight = inp["amount"] / total_input_value
            weighted_taint_sum += weight * input_taint
            
        self.taint_scores[tx_hash] = weighted_taint_sum
        return weighted_taint_sum
        
    def test_simple_propagation(self):
        """Test: Stolen funds → Single recipient"""
        self.log("\nTest 1: Simple Propagation")
        
        # TX1: Stolen transaction
        self.create_transaction("TX1", [], [{"address": "A", "amount": 1000}])
        self.mark_as_stolen("TX1")
        
        # TX2: Spends TX1 output
        self.create_transaction("TX2", 
            [{"from_tx": "TX1", "amount": 1000}],
            [{"address": "B", "amount": 1000}]
        )
        
        taint_tx2 = self.calculate_taint("TX2")
        
        assert abs(taint_tx2 - 1.0) < 0.001, f"Expected 1.0, got {taint_tx2}"
        print(f"✅ TX2 taint: {taint_tx2:.3f} (expected 1.0)")
        
    def test_value_conservation(self):
        """Test: Taint follows value proportions"""
        self.log("\nTest 2: Value Conservation")
        
        # TX1: Stolen 1000 GXC
        self.create_transaction("TX1", [], [{"address": "A", "amount": 1000}])
        self.mark_as_stolen("TX1")
        
        # TX2: Clean 9000 GXC
        self.create_transaction("TX2", [], [{"address": "B", "amount": 9000}])
        self.taint_scores["TX2"] = 0.0
        
        # TX3: Mixes TX1 (1000 tainted) + TX2 (9000 clean)
        self.create_transaction("TX3",
            [
                {"from_tx": "TX1", "amount": 1000},
                {"from_tx": "TX2", "amount": 9000}
            ],
            [{"address": "C", "amount": 10000}]
        )
        
        taint_tx3 = self.calculate_taint("TX3")
        expected = 0.1  # 10% tainted
        
        assert abs(taint_tx3 - expected) < 0.001, f"Expected {expected}, got {taint_tx3}"
        print(f"✅ TX3 taint: {taint_tx3:.3f} (expected {expected:.3f})")
        
    def test_multi_hop_propagation(self):
        """Test: Taint propagates through multiple hops"""
        self.log("\nTest 3: Multi-Hop Propagation")
        
        # TX1: Stolen
        self.create_transaction("TX1_hop", [], [{"address": "A", "amount": 1000}])
        self.mark_as_stolen("TX1_hop")
        
        # TX2: First hop (100% tainted)
        self.create_transaction("TX2_hop",
            [{"from_tx": "TX1_hop", "amount": 1000}],
            [{"address": "B", "amount": 1000}]
        )
        taint_tx2 = self.calculate_taint("TX2_hop")
        
        # TX3: Second hop (mix with clean funds)
        self.create_transaction("TX3_clean_hop", [], [{"address": "X", "amount": 1000}])
        self.taint_scores["TX3_clean_hop"] = 0.0
        
        self.create_transaction("TX3_hop",
            [
                {"from_tx": "TX2_hop", "amount": 1000},
                {"from_tx": "TX3_clean_hop", "amount": 1000}
            ],
            [{"address": "C", "amount": 2000}]
        )
        taint_tx3 = self.calculate_taint("TX3_hop")
        
        # TX4: Third hop
        self.create_transaction("TX4_hop",
            [{"from_tx": "TX3_hop", "amount": 2000}],
            [{"address": "D", "amount": 2000}]
        )
        taint_tx4 = self.calculate_taint("TX4_hop")
        
        print(f"✅ Hop 1 (TX2): {taint_tx2:.3f}")
        print(f"✅ Hop 2 (TX3): {taint_tx3:.3f}")
        print(f"✅ Hop 3 (TX4): {taint_tx4:.3f}")
        
        assert abs(taint_tx2 - 1.0) < 0.001, f"First hop should be 100% tainted, got {taint_tx2}"
        assert 0.4 < taint_tx3 < 0.6, f"Second hop should be ~50% tainted, got {taint_tx3}"
        assert abs(taint_tx4 - taint_tx3) < 0.001, "Third hop should preserve taint"
        
    def test_fan_out_taint(self):
        """Test: Taint distributes across multiple outputs"""
        self.log("\nTest 4: Fan-Out Taint Distribution")
        
        # TX1: Stolen 1000 GXC
        self.create_transaction("TX1_fanout", [], [{"address": "A", "amount": 1000}])
        self.mark_as_stolen("TX1_fanout")
        
        # TX2: Split into 10 outputs
        outputs = [{"address": f"OUT_{i}", "amount": 100} for i in range(10)]
        self.create_transaction("TX2_fanout",
            [{"from_tx": "TX1_fanout", "amount": 1000}],
            outputs
        )
        
        taint_tx2 = self.calculate_taint("TX2_fanout")
        
        # Each output inherits the same taint
        print(f"✅ Fan-out taint: {taint_tx2:.3f} (each output)")
        assert abs(taint_tx2 - 1.0) < 0.001, f"Fan-out should preserve taint, got {taint_tx2}"
        
    def test_re_aggregation_taint(self):
        """Test: Re-aggregating tainted funds"""
        self.log("\nTest 5: Re-Aggregation")
        
        # Create 5 stolen transactions
        for i in range(5):
            tx_hash = f"STOLEN_{i}"
            self.create_transaction(tx_hash, [], [{"address": f"A_{i}", "amount": 200}])
            self.mark_as_stolen(tx_hash)
            
        # Re-aggregate all 5 into one transaction
        inputs = [{"from_tx": f"STOLEN_{i}", "amount": 200} for i in range(5)]
        self.create_transaction("REAGG",
            inputs,
            [{"address": "AGGREGATED", "amount": 1000}]
        )
        
        taint_reagg = self.calculate_taint("REAGG")
        
        print(f"✅ Re-aggregation taint: {taint_reagg:.3f}")
        assert abs(taint_reagg - 1.0) < 0.001, "Re-aggregation should be 100% tainted"
        
    def test_taint_dilution_chain(self):
        """Test: Progressive taint dilution through mixing"""
        self.log("\nTest 6: Taint Dilution Chain")
        
        # Start with stolen funds
        self.create_transaction("STOLEN", [], [{"address": "A", "amount": 1000}])
        self.mark_as_stolen("STOLEN")
        
        current_tx = "STOLEN"
        current_taint = 1.0
        
        # Mix with clean funds 5 times
        for i in range(5):
            clean_tx = f"CLEAN_{i}"
            self.create_transaction(clean_tx, [], [{"address": f"X_{i}", "amount": 1000}])
            self.taint_scores[clean_tx] = 0.0
            
            mix_tx = f"MIX_{i}"
            self.create_transaction(mix_tx,
                [
                    {"from_tx": current_tx, "amount": 1000},
                    {"from_tx": clean_tx, "amount": 1000}
                ],
                [{"address": f"M_{i}", "amount": 2000}]
            )
            
            current_taint = self.calculate_taint(mix_tx)
            current_tx = mix_tx
            
            print(f"  Mix {i+1}: τ = {current_taint:.4f}")
            
        # After 5 mixes, taint should be significantly diluted
        assert current_taint < 0.1, "Taint should be diluted below threshold"
        print(f"✅ Final taint after 5 mixes: {current_taint:.4f}")
        
    def test_mathematical_properties(self):
        """Test: Mathematical properties of taint function"""
        self.log("\nTest 7: Mathematical Properties")
        
        # Property 1: τ ∈ [0, 1]
        self.create_transaction("TX1_prop", [], [{"address": "A", "amount": 1000}])
        self.mark_as_stolen("TX1_prop")
        
        for tx_hash, taint in self.taint_scores.items():
            assert 0.0 <= taint <= 1.0, f"Taint {taint} out of bounds for {tx_hash}"
            
        print("✅ Property 1: All taint scores in [0, 1]")
        
        # Property 2: Conservation of value
        self.create_transaction("TX2_prop",
            [{"from_tx": "TX1_prop", "amount": 1000}],
            [{"address": "B", "amount": 1000}]
        )
        taint_tx2 = self.calculate_taint("TX2_prop")
        
        assert abs(taint_tx2 - 1.0) < 0.001, f"Value conservation violated: {taint_tx2}"
        print("✅ Property 2: Value conservation holds")
        
        # Property 3: Monotonicity (taint doesn't increase)
        self.create_transaction("CLEAN_prop", [], [{"address": "C", "amount": 1000}])
        self.taint_scores["CLEAN_prop"] = 0.0
        
        self.create_transaction("TX3_prop",
            [
                {"from_tx": "TX2_prop", "amount": 1000},
                {"from_tx": "CLEAN_prop", "amount": 1000}
            ],
            [{"address": "D", "amount": 2000}]
        )
        taint_tx3 = self.calculate_taint("TX3_prop")
        
        assert taint_tx3 <= taint_tx2, f"Taint increased (monotonicity violated): {taint_tx3} > {taint_tx2}"
        print("✅ Property 3: Monotonicity holds (taint doesn't increase)")
        
    def run_all_tests(self):
        self.log("=== Starting Taint Propagation Tests ===")
        
        self.test_simple_propagation()
        self.test_value_conservation()
        self.test_multi_hop_propagation()
        self.test_fan_out_taint()
        self.test_re_aggregation_taint()
        self.test_taint_dilution_chain()
        self.test_mathematical_properties()
        
        self.log("\n=== All Taint Propagation Tests Passed ===")
        print("\n✅ Taint propagation algorithm is mathematically correct!\n")

if __name__ == "__main__":
    test = TaintPropagationTest()
    test.run_all_tests()
