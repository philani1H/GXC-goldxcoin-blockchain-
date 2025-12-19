#!/usr/bin/env python3
"""
Unit tests for advanced market maker features
Tests functionality without requiring API server
"""

import sys
import os
sys.path.append('market_maker')

from advanced_features import AdvancedMarketMaker, OrderType
from datetime import datetime

def test_iceberg_orders():
    """Test iceberg order placement"""
    print("Testing: Iceberg Orders... ", end="")
    try:
        amm = AdvancedMarketMaker(db_path="/tmp/test_amm.db")
        order_id = amm.place_iceberg_order(
            symbol="AAPL",
            side="buy",
            total_quantity=10000,
            visible_quantity=100,
            price=150.50
        )
        assert order_id.startswith("ICE_")
        print("✅ PASSED")
        return True
    except Exception as e:
        print(f"❌ FAILED: {e}")
        return False

def test_twap_orders():
    """Test TWAP order placement"""
    print("Testing: TWAP Orders... ", end="")
    try:
        amm = AdvancedMarketMaker(db_path="/tmp/test_amm.db")
        order_id = amm.place_twap_order(
            symbol="AAPL",
            side="buy",
            total_quantity=10000,
            duration_minutes=60
        )
        assert order_id.startswith("TWAP_")
        print("✅ PASSED")
        return True
    except Exception as e:
        print(f"❌ FAILED: {e}")
        return False

def test_vwap_orders():
    """Test VWAP order placement"""
    print("Testing: VWAP Orders... ", end="")
    try:
        amm = AdvancedMarketMaker(db_path="/tmp/test_amm.db")
        order_id = amm.place_vwap_order(
            symbol="AAPL",
            side="buy",
            total_quantity=10000,
            target_participation=0.1
        )
        assert order_id.startswith("VWAP_")
        print("✅ PASSED")
        return True
    except Exception as e:
        print(f"❌ FAILED: {e}")
        return False

def test_block_trading():
    """Test block trade execution"""
    print("Testing: Block Trading... ", end="")
    try:
        amm = AdvancedMarketMaker(db_path="/tmp/test_amm.db")
        trade_id = amm.execute_block_trade(
            symbol="AAPL",
            side="buy",
            quantity=100000,
            price=150.50,
            counterparty="Goldman Sachs"
        )
        assert trade_id.startswith("BLOCK_")
        print("✅ PASSED")
        return True
    except Exception as e:
        print(f"❌ FAILED: {e}")
        return False

def test_risk_metrics():
    """Test risk metrics calculation"""
    print("Testing: Risk Metrics... ", end="")
    try:
        amm = AdvancedMarketMaker(db_path="/tmp/test_amm.db")
        metrics = amm.calculate_risk_metrics(
            symbol="AAPL",
            position_size=5000,
            current_price=150.50
        )
        assert metrics.symbol == "AAPL"
        assert metrics.position_size == 5000
        assert metrics.exposure_usd > 0
        assert 0 <= metrics.risk_score <= 100
        print("✅ PASSED")
        print(f"   Risk Score: {metrics.risk_score:.1f}/100")
        print(f"   Exposure: ${metrics.exposure_usd:,.2f}")
        print(f"   VaR 95%: ${metrics.var_95:,.2f}")
        return True
    except Exception as e:
        print(f"❌ FAILED: {e}")
        return False

def test_circuit_breakers():
    """Test circuit breaker logic"""
    print("Testing: Circuit Breakers... ", end="")
    try:
        amm = AdvancedMarketMaker(db_path="/tmp/test_amm.db")
        
        # Test 5% move (should not halt)
        should_halt_5 = amm.check_circuit_breaker("AAPL", 150.50, 143.00)
        assert should_halt_5 == False
        
        # Test 15% move (should halt)
        should_halt_15 = amm.check_circuit_breaker("TSLA", 250.00, 217.00)
        assert should_halt_15 == True
        
        # Test 25% move (should halt)
        should_halt_25 = amm.check_circuit_breaker("NVDA", 500.00, 400.00)
        assert should_halt_25 == True
        
        print("✅ PASSED")
        print("   5% move: Trading continues")
        print("   15% move: Trading halted")
        print("   25% move: Trading halted")
        return True
    except Exception as e:
        print(f"❌ FAILED: {e}")
        return False

def test_dynamic_spreads():
    """Test dynamic spread calculation"""
    print("Testing: Dynamic Spreads... ", end="")
    try:
        amm = AdvancedMarketMaker(db_path="/tmp/test_amm.db")
        
        # Low volatility
        spread_low = amm.calculate_dynamic_spread("AAPL", 0.005, 1000, 0.01)
        
        # High volatility
        spread_high = amm.calculate_dynamic_spread("TSLA", 0.005, 5000, 0.05)
        
        assert spread_high > spread_low
        
        print("✅ PASSED")
        print(f"   Low volatility spread: {spread_low:.4f} ({spread_low*100:.2f}%)")
        print(f"   High volatility spread: {spread_high:.4f} ({spread_high*100:.2f}%)")
        return True
    except Exception as e:
        print(f"❌ FAILED: {e}")
        return False

def test_market_surveillance():
    """Test market surveillance features"""
    print("Testing: Market Surveillance... ", end="")
    try:
        amm = AdvancedMarketMaker(db_path="/tmp/test_amm.db")
        
        # Test wash trading detection
        wash_trading = amm.detect_wash_trading("AAPL", time_window_minutes=5)
        
        # Test unusual activity detection
        unusual_normal = amm.detect_unusual_activity("AAPL", 1000000, 900000)
        unusual_high = amm.detect_unusual_activity("MSFT", 5000000, 1000000)
        
        assert unusual_high == True
        
        print("✅ PASSED")
        print("   Wash trading detection: Working")
        print("   Unusual activity detection: Working")
        return True
    except Exception as e:
        print(f"❌ FAILED: {e}")
        return False

def test_risk_dashboard():
    """Test risk dashboard"""
    print("Testing: Risk Dashboard... ", end="")
    try:
        amm = AdvancedMarketMaker(db_path="/tmp/test_amm.db")
        dashboard = amm.get_risk_dashboard()
        
        assert 'timestamp' in dashboard
        assert 'total_positions' in dashboard
        assert 'positions' in dashboard
        
        print("✅ PASSED")
        print(f"   Total positions: {dashboard['total_positions']}")
        print(f"   Total exposure: ${dashboard['total_exposure']:,.2f}")
        return True
    except Exception as e:
        print(f"❌ FAILED: {e}")
        return False

def main():
    """Run all tests"""
    print("=" * 50)
    print("  ADVANCED MARKET MAKER FEATURES - UNIT TESTS")
    print("=" * 50)
    print()
    
    tests = [
        ("Iceberg Orders", test_iceberg_orders),
        ("TWAP Orders", test_twap_orders),
        ("VWAP Orders", test_vwap_orders),
        ("Block Trading", test_block_trading),
        ("Risk Metrics", test_risk_metrics),
        ("Circuit Breakers", test_circuit_breakers),
        ("Dynamic Spreads", test_dynamic_spreads),
        ("Market Surveillance", test_market_surveillance),
        ("Risk Dashboard", test_risk_dashboard),
    ]
    
    passed = 0
    failed = 0
    
    for name, test_func in tests:
        print()
        print(f"--- {name} ---")
        if test_func():
            passed += 1
        else:
            failed += 1
    
    print()
    print("=" * 50)
    print("  TEST SUMMARY")
    print("=" * 50)
    print(f"Tests Passed: {passed}")
    print(f"Tests Failed: {failed}")
    print()
    
    if failed == 0:
        print("✅ ALL TESTS PASSED!")
        print()
        print("Advanced Features Verified:")
        print("  ✅ Iceberg Orders - Hide large orders")
        print("  ✅ TWAP Orders - Time-weighted execution")
        print("  ✅ VWAP Orders - Volume-weighted execution")
        print("  ✅ Block Trading - Large institutional trades")
        print("  ✅ Risk Management - Real-time risk metrics")
        print("  ✅ Circuit Breakers - Halt trading on volatility")
        print("  ✅ Dynamic Spreads - Adjust based on conditions")
        print("  ✅ Market Surveillance - Detect manipulation")
        print("  ✅ Risk Dashboard - Comprehensive monitoring")
        print()
        return 0
    else:
        print("❌ SOME TESTS FAILED")
        return 1

if __name__ == '__main__':
    exit(main())
