#!/usr/bin/env python3
"""
Advanced Market Maker API Endpoints
REST API for advanced trading features
"""

from flask import Flask, request, jsonify
from advanced_features import AdvancedMarketMaker, OrderType
import json

app = Flask(__name__)
amm = AdvancedMarketMaker()

# ========================================
# ICEBERG ORDERS
# ========================================

@app.route('/api/v1/orders/iceberg', methods=['POST'])
def place_iceberg_order():
    """
    Place iceberg order
    
    POST /api/v1/orders/iceberg
    {
        "symbol": "AAPL",
        "side": "buy",
        "total_quantity": 10000,
        "visible_quantity": 100,
        "price": 150.50
    }
    """
    data = request.json
    
    try:
        order_id = amm.place_iceberg_order(
            symbol=data['symbol'],
            side=data['side'],
            total_quantity=data['total_quantity'],
            visible_quantity=data['visible_quantity'],
            price=data['price']
        )
        
        return jsonify({
            'success': True,
            'order_id': order_id,
            'order_type': 'iceberg',
            'message': f'Iceberg order placed: {data["total_quantity"]} shares, showing {data["visible_quantity"]} at a time'
        })
    except Exception as e:
        return jsonify({'success': False, 'error': str(e)}), 400

# ========================================
# TWAP ORDERS
# ========================================

@app.route('/api/v1/orders/twap', methods=['POST'])
def place_twap_order():
    """
    Place TWAP order
    
    POST /api/v1/orders/twap
    {
        "symbol": "AAPL",
        "side": "buy",
        "total_quantity": 10000,
        "duration_minutes": 60
    }
    """
    data = request.json
    
    try:
        order_id = amm.place_twap_order(
            symbol=data['symbol'],
            side=data['side'],
            total_quantity=data['total_quantity'],
            duration_minutes=data['duration_minutes']
        )
        
        return jsonify({
            'success': True,
            'order_id': order_id,
            'order_type': 'twap',
            'message': f'TWAP order placed: {data["total_quantity"]} shares over {data["duration_minutes"]} minutes'
        })
    except Exception as e:
        return jsonify({'success': False, 'error': str(e)}), 400

# ========================================
# VWAP ORDERS
# ========================================

@app.route('/api/v1/orders/vwap', methods=['POST'])
def place_vwap_order():
    """
    Place VWAP order
    
    POST /api/v1/orders/vwap
    {
        "symbol": "AAPL",
        "side": "buy",
        "total_quantity": 10000,
        "target_participation": 0.1
    }
    """
    data = request.json
    
    try:
        order_id = amm.place_vwap_order(
            symbol=data['symbol'],
            side=data['side'],
            total_quantity=data['total_quantity'],
            target_participation=data.get('target_participation', 0.1)
        )
        
        return jsonify({
            'success': True,
            'order_id': order_id,
            'order_type': 'vwap',
            'message': f'VWAP order placed: {data["total_quantity"]} shares at {data.get("target_participation", 0.1)*100}% participation'
        })
    except Exception as e:
        return jsonify({'success': False, 'error': str(e)}), 400

# ========================================
# BLOCK TRADING
# ========================================

@app.route('/api/v1/trades/block', methods=['POST'])
def execute_block_trade():
    """
    Execute block trade
    
    POST /api/v1/trades/block
    {
        "symbol": "AAPL",
        "side": "buy",
        "quantity": 100000,
        "price": 150.50,
        "counterparty": "Goldman Sachs"
    }
    """
    data = request.json
    
    try:
        trade_id = amm.execute_block_trade(
            symbol=data['symbol'],
            side=data['side'],
            quantity=data['quantity'],
            price=data['price'],
            counterparty=data['counterparty']
        )
        
        return jsonify({
            'success': True,
            'trade_id': trade_id,
            'trade_type': 'block',
            'message': f'Block trade executed: {data["quantity"]} shares @ ${data["price"]}',
            'settlement': 'T+2'
        })
    except Exception as e:
        return jsonify({'success': False, 'error': str(e)}), 400

# ========================================
# RISK MANAGEMENT
# ========================================

@app.route('/api/v1/risk/metrics/<symbol>', methods=['GET'])
def get_risk_metrics(symbol):
    """
    Get risk metrics for a symbol
    
    GET /api/v1/risk/metrics/AAPL
    """
    try:
        # Get current position and price (mock data for now)
        position_size = 5000
        current_price = 150.50
        
        metrics = amm.calculate_risk_metrics(symbol, position_size, current_price)
        
        return jsonify({
            'success': True,
            'symbol': metrics.symbol,
            'position_size': metrics.position_size,
            'position_limit': metrics.position_limit,
            'exposure_usd': metrics.exposure_usd,
            'exposure_limit_usd': metrics.exposure_limit_usd,
            'var_95': metrics.var_95,
            'volatility': metrics.volatility,
            'beta': metrics.beta,
            'risk_score': metrics.risk_score,
            'risk_level': 'LOW' if metrics.risk_score < 30 else 'MEDIUM' if metrics.risk_score < 60 else 'HIGH'
        })
    except Exception as e:
        return jsonify({'success': False, 'error': str(e)}), 400

@app.route('/api/v1/risk/dashboard', methods=['GET'])
def get_risk_dashboard():
    """
    Get comprehensive risk dashboard
    
    GET /api/v1/risk/dashboard
    """
    try:
        dashboard = amm.get_risk_dashboard()
        return jsonify({
            'success': True,
            'dashboard': dashboard
        })
    except Exception as e:
        return jsonify({'success': False, 'error': str(e)}), 400

# ========================================
# CIRCUIT BREAKERS
# ========================================

@app.route('/api/v1/circuit-breaker/check', methods=['POST'])
def check_circuit_breaker():
    """
    Check if circuit breaker should trigger
    
    POST /api/v1/circuit-breaker/check
    {
        "symbol": "AAPL",
        "current_price": 150.50,
        "previous_price": 140.00
    }
    """
    data = request.json
    
    try:
        should_halt = amm.check_circuit_breaker(
            symbol=data['symbol'],
            current_price=data['current_price'],
            previous_price=data['previous_price']
        )
        
        price_change = abs((data['current_price'] - data['previous_price']) / data['previous_price'])
        
        return jsonify({
            'success': True,
            'symbol': data['symbol'],
            'price_change': f"{price_change:.2%}",
            'should_halt': should_halt,
            'status': 'HALTED' if should_halt else 'TRADING'
        })
    except Exception as e:
        return jsonify({'success': False, 'error': str(e)}), 400

# ========================================
# MARKET SURVEILLANCE
# ========================================

@app.route('/api/v1/surveillance/wash-trading/<symbol>', methods=['GET'])
def detect_wash_trading(symbol):
    """
    Detect wash trading
    
    GET /api/v1/surveillance/wash-trading/AAPL
    """
    try:
        is_wash_trading = amm.detect_wash_trading(symbol)
        
        return jsonify({
            'success': True,
            'symbol': symbol,
            'wash_trading_detected': is_wash_trading,
            'status': 'FLAGGED' if is_wash_trading else 'NORMAL'
        })
    except Exception as e:
        return jsonify({'success': False, 'error': str(e)}), 400

@app.route('/api/v1/surveillance/unusual-activity', methods=['POST'])
def detect_unusual_activity():
    """
    Detect unusual activity
    
    POST /api/v1/surveillance/unusual-activity
    {
        "symbol": "AAPL",
        "current_volume": 1000000,
        "avg_volume": 300000
    }
    """
    data = request.json
    
    try:
        is_unusual = amm.detect_unusual_activity(
            symbol=data['symbol'],
            current_volume=data['current_volume'],
            avg_volume=data['avg_volume']
        )
        
        volume_ratio = data['current_volume'] / data['avg_volume'] if data['avg_volume'] > 0 else 0
        
        return jsonify({
            'success': True,
            'symbol': data['symbol'],
            'volume_ratio': f"{volume_ratio:.1f}x",
            'unusual_activity_detected': is_unusual,
            'status': 'FLAGGED' if is_unusual else 'NORMAL'
        })
    except Exception as e:
        return jsonify({'success': False, 'error': str(e)}), 400

# ========================================
# DYNAMIC SPREADS
# ========================================

@app.route('/api/v1/spreads/calculate', methods=['POST'])
def calculate_dynamic_spread():
    """
    Calculate dynamic spread
    
    POST /api/v1/spreads/calculate
    {
        "symbol": "AAPL",
        "base_spread": 0.005,
        "position_size": 5000,
        "volatility": 0.02
    }
    """
    data = request.json
    
    try:
        dynamic_spread = amm.calculate_dynamic_spread(
            symbol=data['symbol'],
            base_spread=data['base_spread'],
            position_size=data['position_size'],
            volatility=data['volatility']
        )
        
        return jsonify({
            'success': True,
            'symbol': data['symbol'],
            'base_spread': f"{data['base_spread']:.2%}",
            'dynamic_spread': f"{dynamic_spread:.2%}",
            'adjustment': f"{(dynamic_spread / data['base_spread'] - 1) * 100:.1f}%"
        })
    except Exception as e:
        return jsonify({'success': False, 'error': str(e)}), 400

# ========================================
# ORDER EXECUTION REPORTS
# ========================================

@app.route('/api/v1/orders/report/<order_id>', methods=['GET'])
def get_order_execution_report(order_id):
    """
    Get order execution report
    
    GET /api/v1/orders/report/TWAP_1234567890
    """
    try:
        report = amm.get_order_execution_report(order_id)
        
        return jsonify({
            'success': True,
            'report': report
        })
    except Exception as e:
        return jsonify({'success': False, 'error': str(e)}), 400

# ========================================
# HEALTH CHECK
# ========================================

@app.route('/api/v1/health', methods=['GET'])
def health_check():
    """Health check endpoint"""
    return jsonify({
        'success': True,
        'status': 'healthy',
        'features': [
            'Iceberg Orders',
            'TWAP Orders',
            'VWAP Orders',
            'Block Trading',
            'Risk Management',
            'Circuit Breakers',
            'Market Surveillance',
            'Dynamic Spreads'
        ]
    })

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5002, debug=True)
