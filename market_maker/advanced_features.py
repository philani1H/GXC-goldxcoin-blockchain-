#!/usr/bin/env python3
"""
Advanced Market Maker Features for GXC Stock Market
Includes AMM strategies, advanced orders, risk management, and institutional features
"""

import time
import json
import sqlite3
from datetime import datetime, timedelta
from typing import Dict, List, Optional, Tuple
from dataclasses import dataclass
from enum import Enum
import numpy as np

class OrderType(Enum):
    MARKET = "market"
    LIMIT = "limit"
    ICEBERG = "iceberg"
    TWAP = "twap"
    VWAP = "vwap"
    PEGGED = "pegged"
    STOP_LOSS = "stop_loss"
    BLOCK = "block"

@dataclass
class AdvancedOrder:
    order_id: str
    order_type: OrderType
    symbol: str
    side: str
    total_quantity: float
    visible_quantity: float
    price: Optional[float]
    time_start: datetime
    time_end: datetime
    status: str = "active"
    filled_quantity: float = 0.0
    
@dataclass
class RiskMetrics:
    symbol: str
    position_size: float
    position_limit: float
    exposure_usd: float
    exposure_limit_usd: float
    var_95: float  # Value at Risk 95%
    volatility: float
    beta: float
    risk_score: float

class AdvancedMarketMaker:
    """Advanced market making features"""
    
    def __init__(self, db_path: str = "market_maker.db"):
        self.db_path = db_path
        self.init_advanced_tables()
        
    def init_advanced_tables(self):
        """Initialize advanced feature tables"""
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        
        # Price history (needed for risk calculations)
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS price_history (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                symbol TEXT NOT NULL,
                price REAL NOT NULL,
                volume INTEGER DEFAULT 0,
                timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            )
        ''')
        
        # Market orders (needed for surveillance)
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS market_orders (
                order_id TEXT PRIMARY KEY,
                symbol TEXT NOT NULL,
                side TEXT NOT NULL,
                quantity REAL NOT NULL,
                price REAL NOT NULL,
                timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                status TEXT DEFAULT 'active'
            )
        ''')
        
        # Advanced orders
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS advanced_orders (
                order_id TEXT PRIMARY KEY,
                order_type TEXT NOT NULL,
                symbol TEXT NOT NULL,
                side TEXT NOT NULL,
                total_quantity REAL NOT NULL,
                visible_quantity REAL NOT NULL,
                price REAL,
                time_start TIMESTAMP NOT NULL,
                time_end TIMESTAMP NOT NULL,
                status TEXT DEFAULT 'active',
                filled_quantity REAL DEFAULT 0.0,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            )
        ''')
        
        # Risk metrics
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS risk_metrics (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                symbol TEXT NOT NULL,
                position_size REAL NOT NULL,
                position_limit REAL NOT NULL,
                exposure_usd REAL NOT NULL,
                exposure_limit_usd REAL NOT NULL,
                var_95 REAL NOT NULL,
                volatility REAL NOT NULL,
                beta REAL NOT NULL,
                risk_score REAL NOT NULL,
                timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            )
        ''')
        
        # Circuit breakers
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS circuit_breakers (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                symbol TEXT NOT NULL,
                trigger_type TEXT NOT NULL,
                trigger_value REAL NOT NULL,
                triggered_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                resolved_at TIMESTAMP,
                status TEXT DEFAULT 'active'
            )
        ''')
        
        # Block trades
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS block_trades (
                trade_id TEXT PRIMARY KEY,
                symbol TEXT NOT NULL,
                side TEXT NOT NULL,
                quantity REAL NOT NULL,
                price REAL NOT NULL,
                counterparty TEXT NOT NULL,
                executed_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                settlement_date TIMESTAMP,
                status TEXT DEFAULT 'pending'
            )
        ''')
        
        conn.commit()
        conn.close()
    
    # ========================================
    # 1. ICEBERG ORDERS
    # ========================================
    
    def place_iceberg_order(self, symbol: str, side: str, total_quantity: float, 
                           visible_quantity: float, price: float) -> str:
        """
        Place iceberg order - hide large order, show small portions
        """
        order_id = f"ICE_{int(time.time() * 1000)}"
        
        order = AdvancedOrder(
            order_id=order_id,
            order_type=OrderType.ICEBERG,
            symbol=symbol,
            side=side,
            total_quantity=total_quantity,
            visible_quantity=visible_quantity,
            price=price,
            time_start=datetime.now(),
            time_end=datetime.now() + timedelta(days=1)
        )
        
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        
        cursor.execute('''
            INSERT INTO advanced_orders 
            (order_id, order_type, symbol, side, total_quantity, visible_quantity, 
             price, time_start, time_end)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
        ''', (order.order_id, order.order_type.value, order.symbol, order.side,
              order.total_quantity, order.visible_quantity, order.price,
              order.time_start, order.time_end))
        
        conn.commit()
        conn.close()
        
        return order_id
    
    # ========================================
    # 2. TWAP (Time-Weighted Average Price)
    # ========================================
    
    def place_twap_order(self, symbol: str, side: str, total_quantity: float,
                        duration_minutes: int) -> str:
        """
        Execute order over time to minimize market impact
        """
        order_id = f"TWAP_{int(time.time() * 1000)}"
        
        time_start = datetime.now()
        time_end = time_start + timedelta(minutes=duration_minutes)
        
        # Calculate slice size (execute every minute)
        num_slices = duration_minutes
        slice_size = total_quantity / num_slices
        
        order = AdvancedOrder(
            order_id=order_id,
            order_type=OrderType.TWAP,
            symbol=symbol,
            side=side,
            total_quantity=total_quantity,
            visible_quantity=slice_size,
            price=None,  # Market price
            time_start=time_start,
            time_end=time_end
        )
        
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        
        cursor.execute('''
            INSERT INTO advanced_orders 
            (order_id, order_type, symbol, side, total_quantity, visible_quantity, 
             time_start, time_end)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?)
        ''', (order.order_id, order.order_type.value, order.symbol, order.side,
              order.total_quantity, order.visible_quantity, order.time_start, order.time_end))
        
        conn.commit()
        conn.close()
        
        return order_id
    
    # ========================================
    # 3. VWAP (Volume-Weighted Average Price)
    # ========================================
    
    def place_vwap_order(self, symbol: str, side: str, total_quantity: float,
                        target_participation: float = 0.1) -> str:
        """
        Execute based on market volume (e.g., 10% of market volume)
        """
        order_id = f"VWAP_{int(time.time() * 1000)}"
        
        order = AdvancedOrder(
            order_id=order_id,
            order_type=OrderType.VWAP,
            symbol=symbol,
            side=side,
            total_quantity=total_quantity,
            visible_quantity=total_quantity * target_participation,
            price=None,
            time_start=datetime.now(),
            time_end=datetime.now() + timedelta(hours=1)
        )
        
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        
        cursor.execute('''
            INSERT INTO advanced_orders 
            (order_id, order_type, symbol, side, total_quantity, visible_quantity, 
             time_start, time_end)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?)
        ''', (order.order_id, order.order_type.value, order.symbol, order.side,
              order.total_quantity, order.visible_quantity, order.time_start, order.time_end))
        
        conn.commit()
        conn.close()
        
        return order_id
    
    # ========================================
    # 4. RISK MANAGEMENT
    # ========================================
    
    def calculate_risk_metrics(self, symbol: str, position_size: float, 
                              current_price: float) -> RiskMetrics:
        """
        Calculate comprehensive risk metrics
        """
        # Position limits
        position_limit = 10000  # Max 10,000 shares per stock
        exposure_usd = position_size * current_price
        exposure_limit_usd = 1000000  # Max $1M exposure per stock
        
        # Calculate volatility (simplified - use historical data in production)
        volatility = self.calculate_volatility(symbol)
        
        # Calculate VaR (Value at Risk) - 95% confidence
        var_95 = exposure_usd * volatility * 1.645  # 1.645 = z-score for 95%
        
        # Calculate beta (market correlation)
        beta = self.calculate_beta(symbol)
        
        # Risk score (0-100, higher = riskier)
        position_risk = (abs(position_size) / position_limit) * 30
        exposure_risk = (exposure_usd / exposure_limit_usd) * 30
        volatility_risk = min(volatility * 100, 40)
        risk_score = position_risk + exposure_risk + volatility_risk
        
        metrics = RiskMetrics(
            symbol=symbol,
            position_size=position_size,
            position_limit=position_limit,
            exposure_usd=exposure_usd,
            exposure_limit_usd=exposure_limit_usd,
            var_95=var_95,
            volatility=volatility,
            beta=beta,
            risk_score=risk_score
        )
        
        # Store metrics
        self.store_risk_metrics(metrics)
        
        return metrics
    
    def calculate_volatility(self, symbol: str) -> float:
        """Calculate historical volatility"""
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        
        cursor.execute('''
            SELECT price FROM price_history 
            WHERE symbol = ? 
            ORDER BY timestamp DESC 
            LIMIT 100
        ''', (symbol,))
        
        prices = [row[0] for row in cursor.fetchall()]
        conn.close()
        
        if len(prices) < 2:
            return 0.02  # Default 2% volatility
        
        returns = np.diff(prices) / prices[:-1]
        volatility = np.std(returns)
        
        return volatility
    
    def calculate_beta(self, symbol: str) -> float:
        """Calculate beta (market correlation)"""
        # Simplified - return 1.0 (market neutral)
        # In production, calculate correlation with market index
        return 1.0
    
    def store_risk_metrics(self, metrics: RiskMetrics):
        """Store risk metrics in database"""
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        
        cursor.execute('''
            INSERT INTO risk_metrics 
            (symbol, position_size, position_limit, exposure_usd, exposure_limit_usd,
             var_95, volatility, beta, risk_score)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
        ''', (metrics.symbol, metrics.position_size, metrics.position_limit,
              metrics.exposure_usd, metrics.exposure_limit_usd, metrics.var_95,
              metrics.volatility, metrics.beta, metrics.risk_score))
        
        conn.commit()
        conn.close()
    
    # ========================================
    # 5. CIRCUIT BREAKERS
    # ========================================
    
    def check_circuit_breaker(self, symbol: str, current_price: float, 
                             previous_price: float) -> bool:
        """
        Check if circuit breaker should trigger
        Returns True if trading should halt
        """
        price_change = abs((current_price - previous_price) / previous_price)
        
        # Trigger thresholds
        LEVEL_1 = 0.07  # 7% - warning
        LEVEL_2 = 0.13  # 13% - halt 15 minutes
        LEVEL_3 = 0.20  # 20% - halt rest of day
        
        if price_change >= LEVEL_3:
            self.trigger_circuit_breaker(symbol, "LEVEL_3", price_change)
            return True
        elif price_change >= LEVEL_2:
            self.trigger_circuit_breaker(symbol, "LEVEL_2", price_change)
            return True
        elif price_change >= LEVEL_1:
            self.trigger_circuit_breaker(symbol, "LEVEL_1", price_change)
            # Level 1 is just a warning, don't halt
            return False
        
        return False
    
    def trigger_circuit_breaker(self, symbol: str, trigger_type: str, 
                               trigger_value: float):
        """Trigger circuit breaker"""
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        
        cursor.execute('''
            INSERT INTO circuit_breakers (symbol, trigger_type, trigger_value)
            VALUES (?, ?, ?)
        ''', (symbol, trigger_type, trigger_value))
        
        conn.commit()
        conn.close()
        
        print(f"⚠️ CIRCUIT BREAKER TRIGGERED: {symbol} - {trigger_type} - {trigger_value:.2%}")
    
    # ========================================
    # 6. BLOCK TRADING
    # ========================================
    
    def execute_block_trade(self, symbol: str, side: str, quantity: float,
                           price: float, counterparty: str) -> str:
        """
        Execute large block trade with minimal market impact
        """
        trade_id = f"BLOCK_{int(time.time() * 1000)}"
        
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        
        cursor.execute('''
            INSERT INTO block_trades 
            (trade_id, symbol, side, quantity, price, counterparty, settlement_date)
            VALUES (?, ?, ?, ?, ?, ?, ?)
        ''', (trade_id, symbol, side, quantity, price, counterparty,
              datetime.now() + timedelta(days=2)))  # T+2 settlement
        
        conn.commit()
        conn.close()
        
        return trade_id
    
    # ========================================
    # 7. DYNAMIC SPREAD ADJUSTMENT
    # ========================================
    
    def calculate_dynamic_spread(self, symbol: str, base_spread: float,
                                position_size: float, volatility: float) -> float:
        """
        Adjust spread based on market conditions
        """
        # Volatility adjustment
        volatility_multiplier = 1 + (volatility * 10)
        
        # Position adjustment (widen spread if large position)
        position_multiplier = 1 + (abs(position_size) / 10000) * 0.5
        
        # Time of day adjustment (wider spreads during low liquidity)
        hour = datetime.now().hour
        if hour < 10 or hour > 15:  # Before 10am or after 3pm
            time_multiplier = 1.2
        else:
            time_multiplier = 1.0
        
        dynamic_spread = base_spread * volatility_multiplier * position_multiplier * time_multiplier
        
        return dynamic_spread
    
    # ========================================
    # 8. MARKET SURVEILLANCE
    # ========================================
    
    def detect_wash_trading(self, symbol: str, time_window_minutes: int = 5) -> bool:
        """
        Detect potential wash trading (same user buying and selling)
        """
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        
        time_threshold = datetime.now() - timedelta(minutes=time_window_minutes)
        
        cursor.execute('''
            SELECT COUNT(*) FROM market_orders
            WHERE symbol = ? 
            AND timestamp > ?
            AND status = 'filled'
        ''', (symbol, time_threshold))
        
        trade_count = cursor.fetchone()[0]
        conn.close()
        
        # If more than 10 trades in 5 minutes, flag for review
        if trade_count > 10:
            print(f"⚠️ WASH TRADING ALERT: {symbol} - {trade_count} trades in {time_window_minutes} minutes")
            return True
        
        return False
    
    def detect_unusual_activity(self, symbol: str, current_volume: int,
                               avg_volume: int) -> bool:
        """
        Detect unusual trading activity
        """
        volume_ratio = current_volume / avg_volume if avg_volume > 0 else 0
        
        # Flag if volume is 3x normal
        if volume_ratio > 3.0:
            print(f"⚠️ UNUSUAL ACTIVITY: {symbol} - Volume {volume_ratio:.1f}x normal")
            return True
        
        return False
    
    # ========================================
    # 9. ANALYTICS & REPORTING
    # ========================================
    
    def get_risk_dashboard(self) -> Dict:
        """Get comprehensive risk dashboard"""
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        
        # Get latest risk metrics for all symbols
        cursor.execute('''
            SELECT symbol, position_size, exposure_usd, var_95, volatility, risk_score
            FROM risk_metrics
            WHERE id IN (
                SELECT MAX(id) FROM risk_metrics GROUP BY symbol
            )
            ORDER BY risk_score DESC
        ''')
        
        metrics = []
        for row in cursor.fetchall():
            metrics.append({
                'symbol': row[0],
                'position_size': row[1],
                'exposure_usd': row[2],
                'var_95': row[3],
                'volatility': row[4],
                'risk_score': row[5]
            })
        
        conn.close()
        
        return {
            'timestamp': datetime.now().isoformat(),
            'total_positions': len(metrics),
            'total_exposure': sum(m['exposure_usd'] for m in metrics),
            'total_var': sum(m['var_95'] for m in metrics),
            'avg_risk_score': np.mean([m['risk_score'] for m in metrics]) if metrics else 0,
            'positions': metrics
        }
    
    def get_order_execution_report(self, order_id: str) -> Dict:
        """Get detailed order execution report"""
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        
        cursor.execute('''
            SELECT * FROM advanced_orders WHERE order_id = ?
        ''', (order_id,))
        
        row = cursor.fetchone()
        conn.close()
        
        if not row:
            return {'error': 'Order not found'}
        
        return {
            'order_id': row[0],
            'order_type': row[1],
            'symbol': row[2],
            'side': row[3],
            'total_quantity': row[4],
            'visible_quantity': row[5],
            'price': row[6],
            'time_start': row[7],
            'time_end': row[8],
            'status': row[9],
            'filled_quantity': row[10],
            'fill_rate': (row[10] / row[4] * 100) if row[4] > 0 else 0
        }

# Export class
__all__ = ['AdvancedMarketMaker', 'OrderType', 'AdvancedOrder', 'RiskMetrics']
