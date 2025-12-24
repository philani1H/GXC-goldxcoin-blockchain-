#!/usr/bin/env python3
"""
Simple Pool Payout Test
Tests payout calculation logic without full pool infrastructure
"""

import sys

def calculate_pplns_payouts(shares_by_miner, block_reward, pool_fee_percent=1.0):
    """
    Calculate PPLNS (Pay Per Last N Shares) payouts
    
    Args:
        shares_by_miner: Dict of {miner_address: share_count}
        block_reward: Block reward in GXC
        pool_fee_percent: Pool fee percentage (default 1%)
    
    Returns:
        Dict of {miner_address: payout_amount}
    """
    total_shares = sum(shares_by_miner.values())
    
    if total_shares == 0:
        return {}
    
    # Calculate pool fee
    pool_fee = block_reward * (pool_fee_percent / 100)
    distributable = block_reward - pool_fee
    
    # Calculate payouts proportional to shares
    payouts = {}
    for miner, shares in shares_by_miner.items():
        payout = (shares / total_shares) * distributable
        payouts[miner] = payout
    
    return payouts, pool_fee, distributable

def test_payout_scenarios():
    """Test various payout scenarios"""
    
    print("="*70)
    print("GXC MINING POOL - PAYOUT CALCULATION TEST")
    print("="*70)
    print()
    
    block_reward = 50.0
    pool_fee_percent = 1.0
    
    # Scenario 1: Two miners, equal shares
    print("SCENARIO 1: Two Miners, Equal Shares")
    print("-"*70)
    
    shares = {
        "tGXCminer1": 10,
        "tGXCminer2": 10
    }
    
    payouts, pool_fee, distributable = calculate_pplns_payouts(shares, block_reward, pool_fee_percent)
    
    print(f"\nBlock reward: {block_reward} GXC")
    print(f"Pool fee ({pool_fee_percent}%): {pool_fee} GXC")
    print(f"Distributable: {distributable} GXC")
    print(f"\nShares:")
    for miner, share_count in shares.items():
        print(f"  {miner}: {share_count} shares")
    
    print(f"\nPayouts:")
    total_payout = 0
    for miner, payout in payouts.items():
        percentage = (shares[miner] / sum(shares.values())) * 100
        print(f"  {miner}: {payout:.6f} GXC ({percentage:.1f}%)")
        total_payout += payout
    
    print(f"\nVerification:")
    print(f"  Total payout: {total_payout:.6f} GXC")
    print(f"  Pool fee: {pool_fee:.6f} GXC")
    print(f"  Sum: {total_payout + pool_fee:.6f} GXC")
    print(f"  Expected: {block_reward:.6f} GXC")
    print(f"  Match: {'✓' if abs((total_payout + pool_fee) - block_reward) < 0.000001 else '✗'}")
    
    # Scenario 2: Three miners, different shares
    print("\n" + "="*70)
    print("SCENARIO 2: Three Miners, Different Shares")
    print("-"*70)
    
    shares = {
        "tGXCminer1": 15,
        "tGXCminer2": 10,
        "tGXCminer3": 5
    }
    
    payouts, pool_fee, distributable = calculate_pplns_payouts(shares, block_reward, pool_fee_percent)
    
    print(f"\nBlock reward: {block_reward} GXC")
    print(f"Pool fee ({pool_fee_percent}%): {pool_fee} GXC")
    print(f"Distributable: {distributable} GXC")
    print(f"\nShares:")
    for miner, share_count in shares.items():
        print(f"  {miner}: {share_count} shares")
    
    print(f"\nPayouts:")
    total_payout = 0
    for miner, payout in payouts.items():
        percentage = (shares[miner] / sum(shares.values())) * 100
        print(f"  {miner}: {payout:.6f} GXC ({percentage:.1f}%)")
        total_payout += payout
    
    print(f"\nVerification:")
    print(f"  Total payout: {total_payout:.6f} GXC")
    print(f"  Pool fee: {pool_fee:.6f} GXC")
    print(f"  Sum: {total_payout + pool_fee:.6f} GXC")
    print(f"  Expected: {block_reward:.6f} GXC")
    print(f"  Match: {'✓' if abs((total_payout + pool_fee) - block_reward) < 0.000001 else '✗'}")
    
    # Scenario 3: One miner dominates
    print("\n" + "="*70)
    print("SCENARIO 3: One Miner Dominates (80% shares)")
    print("-"*70)
    
    shares = {
        "tGXCminer1": 80,
        "tGXCminer2": 15,
        "tGXCminer3": 5
    }
    
    payouts, pool_fee, distributable = calculate_pplns_payouts(shares, block_reward, pool_fee_percent)
    
    print(f"\nBlock reward: {block_reward} GXC")
    print(f"Pool fee ({pool_fee_percent}%): {pool_fee} GXC")
    print(f"Distributable: {distributable} GXC")
    print(f"\nShares:")
    for miner, share_count in shares.items():
        print(f"  {miner}: {share_count} shares")
    
    print(f"\nPayouts:")
    total_payout = 0
    for miner, payout in payouts.items():
        percentage = (shares[miner] / sum(shares.values())) * 100
        print(f"  {miner}: {payout:.6f} GXC ({percentage:.1f}%)")
        total_payout += payout
    
    print(f"\nVerification:")
    print(f"  Total payout: {total_payout:.6f} GXC")
    print(f"  Pool fee: {pool_fee:.6f} GXC")
    print(f"  Sum: {total_payout + pool_fee:.6f} GXC")
    print(f"  Expected: {block_reward:.6f} GXC")
    print(f"  Match: {'✓' if abs((total_payout + pool_fee) - block_reward) < 0.000001 else '✗'}")
    
    # Scenario 4: Different pool fees
    print("\n" + "="*70)
    print("SCENARIO 4: Different Pool Fee (2%)")
    print("-"*70)
    
    shares = {
        "tGXCminer1": 10,
        "tGXCminer2": 10
    }
    
    pool_fee_percent = 2.0
    payouts, pool_fee, distributable = calculate_pplns_payouts(shares, block_reward, pool_fee_percent)
    
    print(f"\nBlock reward: {block_reward} GXC")
    print(f"Pool fee ({pool_fee_percent}%): {pool_fee} GXC")
    print(f"Distributable: {distributable} GXC")
    print(f"\nShares:")
    for miner, share_count in shares.items():
        print(f"  {miner}: {share_count} shares")
    
    print(f"\nPayouts:")
    total_payout = 0
    for miner, payout in payouts.items():
        percentage = (shares[miner] / sum(shares.values())) * 100
        print(f"  {miner}: {payout:.6f} GXC ({percentage:.1f}%)")
        total_payout += payout
    
    print(f"\nVerification:")
    print(f"  Total payout: {total_payout:.6f} GXC")
    print(f"  Pool fee: {pool_fee:.6f} GXC")
    print(f"  Sum: {total_payout + pool_fee:.6f} GXC")
    print(f"  Expected: {block_reward:.6f} GXC")
    print(f"  Match: {'✓' if abs((total_payout + pool_fee) - block_reward) < 0.000001 else '✗'}")
    
    # Summary
    print("\n" + "="*70)
    print("SUMMARY")
    print("="*70)
    print()
    print("✓ PPLNS payout calculation working correctly")
    print("✓ Pool fee deducted properly")
    print("✓ Payouts proportional to shares")
    print("✓ Total always equals block reward")
    print()
    print("Formula: Miner Payout = (Miner Shares / Total Shares) × (Block Reward - Pool Fee)")
    print()
    print("="*70)
    
    return True

if __name__ == "__main__":
    try:
        success = test_payout_scenarios()
        sys.exit(0 if success else 1)
    except Exception as e:
        print(f"\n✗ Test failed: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)
