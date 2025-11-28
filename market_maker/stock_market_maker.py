#!/usr/bin/env python3
"""
GXC Stock Market Maker Software
Automated market making for tokenized stock contracts
"""

import os
import json
import sqlite3
import threading
import time
import logging
from datetime import datetime, timedelta
from flask import Flask, request, jsonify, render_template
from dataclasses import dataclass
from typing import Dict, List, Optional
import yfinance as yf
import numpy as np

app = Flask(__name__)

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

@dataclass
class MarketOrder:
    order_id: str
    symbol: str
    side: str  # 'buy' or 'sell'
    quantity: float
    price: float
    timestamp: datetime
    status: str = 'active'

@dataclass
class StockPosition:
    symbol: str
    quantity: float
    avg_price: float
    current_price: float
    unrealized_pnl: float
    last_updated: datetime

class StockMarketMaker:
    def __init__(self):
        self.init_database()
        self.active_orders: Dict[str, MarketOrder] = {}
        self.positions: Dict[str, StockPosition] = {}
        self.market_data_cache: Dict[str, Dict] = {}
        self.spread_percentage = 0.005  # 0.5% default spread
        self.max_position_size = 1000  # Max shares per symbol
        self.rebalance_threshold = 0.1  # 10% price movement triggers rebalance
        
        # Start background services
        self.start_market_data_feed()
        self.start_order_management()
        self.start_position_monitor()
    
    def init_database(self):
        """Initialize market maker database"""
        # Use /tmp for Vercel (writable), otherwise use local path
        db_path = os.environ.get('DATABASE_PATH', '/tmp/market_maker.db' if os.path.exists('/tmp') else 'market_maker.db')
        conn = sqlite3.connect(db_path)
        cursor = conn.cursor()
        
        # Tracked stocks
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS tracked_stocks (
                symbol TEXT PRIMARY KEY,
                company_name TEXT NOT NULL,
                exchange TEXT NOT NULL,
                sector TEXT,
                market_cap REAL,
                is_active BOOLEAN DEFAULT TRUE,
                added_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                min_spread REAL DEFAULT 0.005,
                max_position REAL DEFAULT 1000,
                tick_size REAL DEFAULT 0.01
            )
        ''')
        
        # Market orders
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS market_orders (
                order_id TEXT PRIMARY KEY,
                symbol TEXT NOT NULL,
                side TEXT NOT NULL,
                quantity REAL NOT NULL,
                price REAL NOT NULL,
                timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                status TEXT DEFAULT 'active',
                filled_quantity REAL DEFAULT 0.0,
                filled_price REAL DEFAULT 0.0,
                commission REAL DEFAULT 0.0,
                FOREIGN KEY (symbol) REFERENCES tracked_stocks (symbol)
            )
        ''')
        
        # Positions
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS positions (
                symbol TEXT PRIMARY KEY,
                quantity REAL NOT NULL,
                avg_price REAL NOT NULL,
                current_price REAL NOT NULL,
                unrealized_pnl REAL NOT NULL,
                realized_pnl REAL DEFAULT 0.0,
                last_updated TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (symbol) REFERENCES tracked_stocks (symbol)
            )
        ''')
        
        # Price history
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS price_history (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                symbol TEXT NOT NULL,
                price REAL NOT NULL,
                volume INTEGER DEFAULT 0,
                timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                source TEXT DEFAULT 'yahoo',
                FOREIGN KEY (symbol) REFERENCES tracked_stocks (symbol)
            )
        ''')
        
        # Market making statistics
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS mm_stats (
                stat_id INTEGER PRIMARY KEY AUTOINCREMENT,
                timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                total_volume REAL NOT NULL,
                total_pnl REAL NOT NULL,
                active_orders INTEGER NOT NULL,
                active_positions INTEGER NOT NULL,
                uptime_seconds INTEGER NOT NULL
            )
        ''')
        
        # Insert default tracked stocks
        default_stocks = [
            ('AAPL', 'Apple Inc.', 'NASDAQ', 'Technology'),
            ('MSFT', 'Microsoft Corporation', 'NASDAQ', 'Technology'),
            ('GOOGL', 'Alphabet Inc.', 'NASDAQ', 'Technology'),
            ('AMZN', 'Amazon.com Inc.', 'NASDAQ', 'Consumer Discretionary'),
            ('TSLA', 'Tesla Inc.', 'NASDAQ', 'Consumer Discretionary'),
            ('NVDA', 'NVIDIA Corporation', 'NASDAQ', 'Technology'),
            ('META', 'Meta Platforms Inc.', 'NASDAQ', 'Technology'),
            ('BRK-B', 'Berkshire Hathaway Inc.', 'NYSE', 'Financial Services'),
            ('JPM', 'JPMorgan Chase & Co.', 'NYSE', 'Financial Services'),
            ('JNJ', 'Johnson & Johnson', 'NYSE', 'Healthcare')
        ]
        
        for symbol, name, exchange, sector in default_stocks:
            cursor.execute('''
                INSERT OR IGNORE INTO tracked_stocks (symbol, company_name, exchange, sector)
                VALUES (?, ?, ?, ?)
            ''', (symbol, name, exchange, sector))
        
        conn.commit()
        conn.close()
    
    def get_tracked_stocks(self) -> List[str]:
        """Get list of actively tracked stock symbols"""
        # Use /tmp for Vercel (writable), otherwise use local path
        db_path = os.environ.get('DATABASE_PATH', '/tmp/market_maker.db' if os.path.exists('/tmp') else 'market_maker.db')
        conn = sqlite3.connect(db_path)
        cursor = conn.cursor()
        
        cursor.execute('SELECT symbol FROM tracked_stocks WHERE is_active = TRUE')
        symbols = [row[0] for row in cursor.fetchall()]
        
        conn.close()
        return symbols
    
    def fetch_market_data(self, symbol: str) -> Optional[Dict]:
        """Fetch real-time market data for a symbol"""
        try:
            ticker = yf.Ticker(symbol)
            
            # Get current price and volume
            info = ticker.info
            hist = ticker.history(period="1d", interval="1m")
            
            if hist.empty:
                logger.warning(f"No market data available for {symbol}")
                return None
            
            latest = hist.iloc[-1]
            
            market_data = {
                'symbol': symbol,
                'price': float(latest['Close']),
                'volume': int(latest['Volume']),
                'bid': float(latest['Close']) * 0.999,  # Approximate bid
                'ask': float(latest['Close']) * 1.001,  # Approximate ask
                'timestamp': datetime.now(),
                'day_change': float(info.get('regularMarketChangePercent', 0)),
                'market_cap': info.get('marketCap', 0)
            }
            
            # Cache the data
            self.market_data_cache[symbol] = market_data
            
            # Store in database
            self.store_price_data(symbol, market_data['price'], market_data['volume'])
            
            return market_data
            
        except Exception as e:
            logger.error(f"Error fetching market data for {symbol}: {e}")
            return None
    
    def store_price_data(self, symbol: str, price: float, volume: int):
        """Store price data in database"""
        # Use /tmp for Vercel (writable), otherwise use local path
        db_path = os.environ.get('DATABASE_PATH', '/tmp/market_maker.db' if os.path.exists('/tmp') else 'market_maker.db')
        conn = sqlite3.connect(db_path)
        cursor = conn.cursor()
        
        try:
            cursor.execute('''
                INSERT INTO price_history (symbol, price, volume)
                VALUES (?, ?, ?)
            ''', (symbol, price, volume))
            
            conn.commit()
            
        except Exception as e:
            logger.error(f"Error storing price data: {e}")
        finally:
            conn.close()
    
    def calculate_fair_price(self, symbol: str) -> Optional[float]:
        """Calculate fair price based on recent market data"""
        if symbol not in self.market_data_cache:
            return None
        
        market_data = self.market_data_cache[symbol]
        
        # Simple fair price calculation (mid-point of bid/ask)
        fair_price = (market_data['bid'] + market_data['ask']) / 2
        
        return fair_price
    
    def generate_quotes(self, symbol: str) -> Optional[tuple]:
        """Generate bid/ask quotes for a symbol"""
        fair_price = self.calculate_fair_price(symbol)
        if not fair_price:
            return None
        
        # Get stock-specific parameters
        # Use /tmp for Vercel (writable), otherwise use local path
        db_path = os.environ.get('DATABASE_PATH', '/tmp/market_maker.db' if os.path.exists('/tmp') else 'market_maker.db')
        conn = sqlite3.connect(db_path)
        cursor = conn.cursor()
        
        cursor.execute('''
            SELECT min_spread, tick_size FROM tracked_stocks WHERE symbol = ?
        ''', (symbol,))
        
        result = cursor.fetchone()
        conn.close()
        
        if not result:
            return None
        
        min_spread, tick_size = result
        
        # Calculate spread based on volatility and position
        base_spread = max(min_spread, self.spread_percentage)
        
        # Adjust spread based on position size
        position = self.positions.get(symbol)
        if position:
            position_ratio = abs(position.quantity) / self.max_position_size
            spread_adjustment = position_ratio * 0.002  # Additional 0.2% per position ratio
            base_spread += spread_adjustment
        
        half_spread = fair_price * base_spread / 2
        
        # Round to tick size
        bid_price = round((fair_price - half_spread) / tick_size) * tick_size
        ask_price = round((fair_price + half_spread) / tick_size) * tick_size
        
        return bid_price, ask_price
    
    def place_market_orders(self, symbol: str):
        """Place bid and ask orders for a symbol"""
        quotes = self.generate_quotes(symbol)
        if not quotes:
            return
        
        bid_price, ask_price = quotes
        
        # Calculate order size based on position and risk management
        current_position = self.positions.get(symbol, StockPosition(symbol, 0, 0, 0, 0, datetime.now()))
        
        # Determine order size (smaller if we have large position)
        max_order_size = min(100, self.max_position_size - abs(current_position.quantity))
        
        if max_order_size <= 0:
            logger.info(f"Maximum position reached for {symbol}, skipping orders")
            return
        
        order_size = min(max_order_size, 50)  # Default 50 shares
        
        # Cancel existing orders for this symbol
        self.cancel_orders_for_symbol(symbol)
        
        # Place new bid order
        bid_order = MarketOrder(
            order_id=f"{symbol}_bid_{int(time.time())}",
            symbol=symbol,
            side='buy',
            quantity=order_size,
            price=bid_price,
            timestamp=datetime.now()
        )
        
        # Place new ask order
        ask_order = MarketOrder(
            order_id=f"{symbol}_ask_{int(time.time())}",
            symbol=symbol,
            side='sell',
            quantity=order_size,
            price=ask_price,
            timestamp=datetime.now()
        )
        
        # Store orders
        self.active_orders[bid_order.order_id] = bid_order
        self.active_orders[ask_order.order_id] = ask_order
        
        # Save to database
        self.store_order(bid_order)
        self.store_order(ask_order)
        
        logger.info(f"Placed orders for {symbol}: BID {bid_price:.2f} x {order_size}, ASK {ask_price:.2f} x {order_size}")
    
    def store_order(self, order: MarketOrder):
        """Store order in database"""
        # Use /tmp for Vercel (writable), otherwise use local path
        db_path = os.environ.get('DATABASE_PATH', '/tmp/market_maker.db' if os.path.exists('/tmp') else 'market_maker.db')
        conn = sqlite3.connect(db_path)
        cursor = conn.cursor()
        
        try:
            cursor.execute('''
                INSERT INTO market_orders (
                    order_id, symbol, side, quantity, price, timestamp, status
                ) VALUES (?, ?, ?, ?, ?, ?, ?)
            ''', (
                order.order_id, order.symbol, order.side, order.quantity,
                order.price, order.timestamp, order.status
            ))
            
            conn.commit()
            
        except Exception as e:
            logger.error(f"Error storing order: {e}")
        finally:
            conn.close()
    
    def cancel_orders_for_symbol(self, symbol: str):
        """Cancel all active orders for a symbol"""
        orders_to_cancel = [
            order_id for order_id, order in self.active_orders.items()
            if order.symbol == symbol and order.status == 'active'
        ]
        
        for order_id in orders_to_cancel:
            self.cancel_order(order_id)
    
    def cancel_order(self, order_id: str):
        """Cancel a specific order"""
        if order_id in self.active_orders:
            order = self.active_orders[order_id]
            order.status = 'cancelled'
            
            # Update in database
            # Use /tmp for Vercel (writable), otherwise use local path
            db_path = os.environ.get('DATABASE_PATH', '/tmp/market_maker.db' if os.path.exists('/tmp') else 'market_maker.db')
            conn = sqlite3.connect(db_path)
            cursor = conn.cursor()
            
            cursor.execute('''
                UPDATE market_orders SET status = 'cancelled' WHERE order_id = ?
            ''', (order_id,))
            
            conn.commit()
            conn.close()
            
            del self.active_orders[order_id]
            logger.info(f"Cancelled order {order_id}")
    
    def update_position(self, symbol: str, quantity_change: float, price: float):
        """Update position after a trade"""
        if symbol not in self.positions:
            self.positions[symbol] = StockPosition(
                symbol=symbol,
                quantity=0,
                avg_price=0,
                current_price=price,
                unrealized_pnl=0,
                last_updated=datetime.now()
            )
        
        position = self.positions[symbol]
        
        # Update position quantity and average price
        if quantity_change > 0:  # Buy
            if position.quantity >= 0:
                # Adding to long position or creating long position
                total_cost = (position.quantity * position.avg_price) + (quantity_change * price)
                position.quantity += quantity_change
                position.avg_price = total_cost / position.quantity if position.quantity > 0 else price
            else:
                # Reducing short position
                position.quantity += quantity_change
                if position.quantity >= 0:
                    position.avg_price = price
        else:  # Sell
            if position.quantity <= 0:
                # Adding to short position or creating short position
                total_cost = (abs(position.quantity) * position.avg_price) + (abs(quantity_change) * price)
                position.quantity += quantity_change
                position.avg_price = total_cost / abs(position.quantity) if position.quantity < 0 else price
            else:
                # Reducing long position
                position.quantity += quantity_change
                if position.quantity <= 0:
                    position.avg_price = price
        
        position.current_price = price
        position.last_updated = datetime.now()
        
        # Calculate unrealized PnL
        if position.quantity > 0:
            position.unrealized_pnl = position.quantity * (price - position.avg_price)
        elif position.quantity < 0:
            position.unrealized_pnl = abs(position.quantity) * (position.avg_price - price)
        else:
            position.unrealized_pnl = 0
        
        # Store updated position
        self.store_position(position)
        
        logger.info(f"Updated position for {symbol}: {position.quantity} shares @ {position.avg_price:.2f}")
    
    def store_position(self, position: StockPosition):
        """Store position in database"""
        # Use /tmp for Vercel (writable), otherwise use local path
        db_path = os.environ.get('DATABASE_PATH', '/tmp/market_maker.db' if os.path.exists('/tmp') else 'market_maker.db')
        conn = sqlite3.connect(db_path)
        cursor = conn.cursor()
        
        try:
            cursor.execute('''
                INSERT OR REPLACE INTO positions (
                    symbol, quantity, avg_price, current_price, unrealized_pnl, last_updated
                ) VALUES (?, ?, ?, ?, ?, ?)
            ''', (
                position.symbol, position.quantity, position.avg_price,
                position.current_price, position.unrealized_pnl, position.last_updated
            ))
            
            conn.commit()
            
        except Exception as e:
            logger.error(f"Error storing position: {e}")
        finally:
            conn.close()
    
    def start_market_data_feed(self):
        """Start background market data feed"""
        def market_data_worker():
            while True:
                try:
                    symbols = self.get_tracked_stocks()
                    
                    for symbol in symbols:
                        self.fetch_market_data(symbol)
                        time.sleep(1)  # Rate limiting
                    
                    time.sleep(30)  # Update every 30 seconds
                    
                except Exception as e:
                    logger.error(f"Market data feed error: {e}")
                    time.sleep(60)
        
        thread = threading.Thread(target=market_data_worker)
        thread.daemon = True
        thread.start()
        logger.info("Started market data feed")
    
    def start_order_management(self):
        """Start background order management"""
        def order_management_worker():
            while True:
                try:
                    symbols = self.get_tracked_stocks()
                    
                    for symbol in symbols:
                        if symbol in self.market_data_cache:
                            self.place_market_orders(symbol)
                    
                    time.sleep(60)  # Update orders every minute
                    
                except Exception as e:
                    logger.error(f"Order management error: {e}")
                    time.sleep(60)
        
        thread = threading.Thread(target=order_management_worker)
        thread.daemon = True
        thread.start()
        logger.info("Started order management")
    
    def start_position_monitor(self):
        """Start background position monitoring"""
        def position_monitor_worker():
            while True:
                try:
                    for symbol, position in self.positions.items():
                        if symbol in self.market_data_cache:
                            current_price = self.market_data_cache[symbol]['price']
                            
                            # Update position with current price
                            position.current_price = current_price
                            
                            # Recalculate unrealized PnL
                            if position.quantity > 0:
                                position.unrealized_pnl = position.quantity * (current_price - position.avg_price)
                            elif position.quantity < 0:
                                position.unrealized_pnl = abs(position.quantity) * (position.avg_price - current_price)
                            
                            position.last_updated = datetime.now()
                            self.store_position(position)
                    
                    time.sleep(10)  # Update every 10 seconds
                    
                except Exception as e:
                    logger.error(f"Position monitor error: {e}")
                    time.sleep(30)
        
        thread = threading.Thread(target=position_monitor_worker)
        thread.daemon = True
        thread.start()
        logger.info("Started position monitor")
    
    def get_portfolio_summary(self) -> Dict:
        """Get portfolio summary"""
        total_pnl = sum(pos.unrealized_pnl for pos in self.positions.values())
        total_positions = len([pos for pos in self.positions.values() if pos.quantity != 0])
        total_orders = len([order for order in self.active_orders.values() if order.status == 'active'])
        
        return {
            'total_unrealized_pnl': total_pnl,
            'active_positions': total_positions,
            'active_orders': total_orders,
            'tracked_symbols': len(self.get_tracked_stocks()),
            'last_updated': datetime.now().isoformat()
        }

# Initialize market maker
market_maker = StockMarketMaker()

# Flask routes
@app.route('/')
def dashboard():
    """Market maker dashboard"""
    portfolio = market_maker.get_portfolio_summary()
    positions = list(market_maker.positions.values())
    recent_orders = list(market_maker.active_orders.values())[-10:]
    
    return render_template('market_maker_dashboard.html',
                         portfolio=portfolio,
                         positions=positions,
                         orders=recent_orders)

@app.route('/api/portfolio')
def api_portfolio():
    """Get portfolio summary"""
    return jsonify(market_maker.get_portfolio_summary())

@app.route('/api/positions')
def api_positions():
    """Get all positions"""
    positions = {symbol: {
        'symbol': pos.symbol,
        'quantity': pos.quantity,
        'avg_price': pos.avg_price,
        'current_price': pos.current_price,
        'unrealized_pnl': pos.unrealized_pnl,
        'last_updated': pos.last_updated.isoformat()
    } for symbol, pos in market_maker.positions.items()}
    
    return jsonify(positions)

@app.route('/api/orders')
def api_orders():
    """Get active orders"""
    orders = {order_id: {
        'order_id': order.order_id,
        'symbol': order.symbol,
        'side': order.side,
        'quantity': order.quantity,
        'price': order.price,
        'status': order.status,
        'timestamp': order.timestamp.isoformat()
    } for order_id, order in market_maker.active_orders.items()}
    
    return jsonify(orders)

@app.route('/api/market_data/<symbol>')
def api_market_data(symbol):
    """Get market data for symbol"""
    if symbol in market_maker.market_data_cache:
        data = market_maker.market_data_cache[symbol].copy()
        data['timestamp'] = data['timestamp'].isoformat()
        return jsonify(data)
    else:
        return jsonify({'error': 'Symbol not found'}), 404

@app.route('/api/add_stock', methods=['POST'])
def api_add_stock():
    """Add a new stock to track"""
    data = request.get_json()
    
    required_fields = ['symbol', 'company_name', 'exchange']
    if not all(field in data for field in required_fields):
        return jsonify({'success': False, 'error': 'Missing required fields'}), 400
    
    # Use /tmp for Vercel (writable), otherwise use local path
    db_path = os.environ.get('DATABASE_PATH', '/tmp/market_maker.db' if os.path.exists('/tmp') else 'market_maker.db')
    conn = sqlite3.connect(db_path)
    cursor = conn.cursor()
    
    try:
        cursor.execute('''
            INSERT INTO tracked_stocks (symbol, company_name, exchange, sector)
            VALUES (?, ?, ?, ?)
        ''', (data['symbol'], data['company_name'], data['exchange'], data.get('sector', '')))
        
        conn.commit()
        
        return jsonify({'success': True, 'message': f'Added {data["symbol"]} to tracking'})
        
    except sqlite3.IntegrityError:
        return jsonify({'success': False, 'error': 'Symbol already exists'}), 400
    except Exception as e:
        return jsonify({'success': False, 'error': str(e)}), 500
    finally:
        conn.close()

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=4000, debug=False)