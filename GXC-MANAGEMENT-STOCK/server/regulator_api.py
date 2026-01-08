#!/usr/bin/env python3
"""
GXC Regulator API - Compliance Tools & Communication Hub
For SEC, FCA, MAS and other regulatory bodies to monitor stock contracts

This API provides regulators with:
- Full transaction traceability for stock tokens
- SEC/FCA formatted compliance reports
- Wash trading detection
- Beneficial ownership tracking
- Official communication channels with market makers
"""
import os
import json
import sqlite3
import hashlib
import time
import uuid
from datetime import datetime, timedelta
from flask import Flask, request, jsonify, Response
from flask_cors import CORS
import csv
import io

app = Flask(__name__)
CORS(app)

DB_PATH = 'gxc_regulator.db'

def get_db():
    conn = sqlite3.connect(DB_PATH)
    conn.row_factory = sqlite3.Row
    return conn

def init_db():
    conn = get_db()
    c = conn.cursor()
    
    # Regulatory Organizations table
    c.execute('''CREATE TABLE IF NOT EXISTS organizations (
        org_id TEXT PRIMARY KEY,
        org_name TEXT NOT NULL,
        org_type TEXT NOT NULL,
        country TEXT NOT NULL,
        api_key_hash TEXT NOT NULL,
        permissions TEXT NOT NULL,
        is_active INTEGER DEFAULT 1,
        created_at INTEGER,
        created_by TEXT,
        last_access INTEGER
    )''')
    
    # Organization Users table
    c.execute('''CREATE TABLE IF NOT EXISTS org_users (
        user_id TEXT PRIMARY KEY,
        org_id TEXT NOT NULL,
        username TEXT NOT NULL,
        password_hash TEXT NOT NULL,
        role TEXT NOT NULL,
        email TEXT,
        is_active INTEGER DEFAULT 1,
        created_at INTEGER,
        last_login INTEGER,
        FOREIGN KEY (org_id) REFERENCES organizations(org_id)
    )''')
    
    # Compliance Reports table
    c.execute('''CREATE TABLE IF NOT EXISTS compliance_reports (
        report_id TEXT PRIMARY KEY,
        org_id TEXT NOT NULL,
        report_type TEXT NOT NULL,
        format TEXT NOT NULL,
        parameters TEXT,
        generated_at INTEGER,
        generated_by TEXT,
        file_hash TEXT,
        status TEXT DEFAULT 'GENERATED',
        FOREIGN KEY (org_id) REFERENCES organizations(org_id)
    )''')
    
    # Transaction Audit Trail table
    c.execute('''CREATE TABLE IF NOT EXISTS transaction_audit (
        audit_id TEXT PRIMARY KEY,
        tx_hash TEXT NOT NULL,
        stock_symbol TEXT,
        from_address TEXT,
        to_address TEXT,
        amount REAL,
        price REAL,
        timestamp INTEGER,
        tx_type TEXT,
        market_maker_id TEXT,
        traced_origin TEXT,
        flags TEXT
    )''')
    
    # Beneficial Ownership table
    c.execute('''CREATE TABLE IF NOT EXISTS beneficial_ownership (
        ownership_id TEXT PRIMARY KEY,
        stock_symbol TEXT NOT NULL,
        holder_address TEXT NOT NULL,
        holder_name TEXT,
        holder_type TEXT,
        shares_held REAL,
        percentage REAL,
        last_change INTEGER,
        change_type TEXT,
        reported_at INTEGER,
        verified INTEGER DEFAULT 0
    )''')
    
    # Wash Trading Alerts table
    c.execute('''CREATE TABLE IF NOT EXISTS wash_trading_alerts (
        alert_id TEXT PRIMARY KEY,
        detected_at INTEGER,
        stock_symbol TEXT,
        addresses_involved TEXT,
        pattern_type TEXT,
        confidence_score REAL,
        total_volume REAL,
        time_window INTEGER,
        status TEXT DEFAULT 'PENDING',
        reviewed_by TEXT,
        reviewed_at INTEGER,
        notes TEXT
    )''')
    
    # Communications table
    c.execute('''CREATE TABLE IF NOT EXISTS communications (
        comm_id TEXT PRIMARY KEY,
        from_org_id TEXT,
        to_market_maker_id TEXT,
        comm_type TEXT NOT NULL,
        subject TEXT NOT NULL,
        message TEXT NOT NULL,
        priority TEXT DEFAULT 'NORMAL',
        requires_response INTEGER DEFAULT 0,
        response_deadline INTEGER,
        status TEXT DEFAULT 'SENT',
        sent_at INTEGER,
        read_at INTEGER,
        responded_at INTEGER,
        response TEXT,
        attachments TEXT
    )''')
    
    # Communication Templates table
    c.execute('''CREATE TABLE IF NOT EXISTS comm_templates (
        template_id TEXT PRIMARY KEY,
        template_name TEXT NOT NULL,
        comm_type TEXT NOT NULL,
        subject_template TEXT,
        message_template TEXT,
        default_priority TEXT,
        default_deadline_days INTEGER,
        created_by TEXT,
        created_at INTEGER
    )''')
    
    # Regulatory Broadcasts table
    c.execute('''CREATE TABLE IF NOT EXISTS broadcasts (
        broadcast_id TEXT PRIMARY KEY,
        org_id TEXT NOT NULL,
        title TEXT NOT NULL,
        message TEXT NOT NULL,
        broadcast_type TEXT NOT NULL,
        target_audience TEXT,
        effective_date INTEGER,
        expiry_date INTEGER,
        sent_at INTEGER,
        sent_by TEXT,
        acknowledgments TEXT DEFAULT '[]'
    )''')
    
    # Access Log table
    c.execute('''CREATE TABLE IF NOT EXISTS access_log (
        log_id TEXT PRIMARY KEY,
        org_id TEXT,
        user_id TEXT,
        action TEXT NOT NULL,
        resource TEXT,
        details TEXT,
        ip_address TEXT,
        timestamp INTEGER
    )''')
    
    conn.commit()
    conn.close()

init_db()

def log_access(org_id, user_id, action, resource=None, details=None):
    conn = get_db()
    c = conn.cursor()
    log_id = f"ACCESS_{int(time.time())}_{uuid.uuid4().hex[:8]}"
    ip = request.remote_addr if request else None
    c.execute('''INSERT INTO access_log (log_id, org_id, user_id, action, resource, details, ip_address, timestamp)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?)''', 
        (log_id, org_id, user_id, action, resource, details, ip, int(time.time())))
    conn.commit()
    conn.close()

def verify_org_session(session_token):
    """Verify organization session and return org details"""
    conn = get_db()
    c = conn.cursor()
    # Simple session verification - in production use proper JWT
    if session_token and session_token.startswith('ORG_SESSION_'):
        parts = session_token.split('_')
        if len(parts) >= 4:
            org_id = '_'.join(parts[2:-1])
            c.execute('SELECT * FROM organizations WHERE org_id = ? AND is_active = 1', (org_id,))
            org = c.fetchone()
            conn.close()
            return dict(org) if org else None
    conn.close()
    return None

# ========== ORGANIZATION MANAGEMENT ==========

@app.route('/api/v1/regulator/organization/register', methods=['POST'])
def register_organization():
    """GXC Admin registers a regulatory organization"""
    data = request.json
    conn = get_db()
    c = conn.cursor()
    
    org_id = f"ORG_{data['orgType']}_{data['country']}_{uuid.uuid4().hex[:8]}"
    api_key = f"GXC_REG_{uuid.uuid4().hex}"
    api_key_hash = hashlib.sha256(api_key.encode()).hexdigest()
    
    default_permissions = [
        'view_transactions', 'view_ownership', 'generate_reports',
        'view_alerts', 'send_communications', 'view_market_makers'
    ]
    
    c.execute('''INSERT INTO organizations 
        (org_id, org_name, org_type, country, api_key_hash, permissions, created_at, created_by)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?)''',
        (org_id, data['orgName'], data['orgType'], data['country'], 
         api_key_hash, json.dumps(default_permissions), int(time.time()), data.get('createdBy', 'GXC_ADMIN')))
    
    conn.commit()
    conn.close()
    
    return jsonify({
        'success': True,
        'orgId': org_id,
        'apiKey': api_key,  # Only shown once!
        'message': f"Organization {data['orgName']} registered successfully",
        'permissions': default_permissions
    })

@app.route('/api/v1/regulator/organization/login', methods=['POST'])
def org_login():
    """Organization user login"""
    data = request.json
    conn = get_db()
    c = conn.cursor()
    
    pw_hash = hashlib.sha256(data['password'].encode()).hexdigest()
    c.execute('''SELECT u.*, o.org_name, o.permissions as org_permissions 
        FROM org_users u JOIN organizations o ON u.org_id = o.org_id
        WHERE u.username = ? AND u.password_hash = ? AND u.is_active = 1 AND o.is_active = 1''',
        (data['username'], pw_hash))
    user = c.fetchone()
    
    if not user:
        return jsonify({'success': False, 'error': 'Invalid credentials'}), 401
    
    session_token = f"ORG_SESSION_{user['org_id']}_{int(time.time())}"
    c.execute('UPDATE org_users SET last_login = ? WHERE user_id = ?', (int(time.time()), user['user_id']))
    c.execute('UPDATE organizations SET last_access = ? WHERE org_id = ?', (int(time.time()), user['org_id']))
    conn.commit()
    conn.close()
    
    log_access(user['org_id'], user['user_id'], 'login')
    
    return jsonify({
        'success': True,
        'sessionToken': session_token,
        'userId': user['user_id'],
        'orgId': user['org_id'],
        'orgName': user['org_name'],
        'role': user['role'],
        'permissions': json.loads(user['org_permissions'])
    })

# ========== COMPLIANCE REPORTS ==========

@app.route('/api/v1/regulator/reports/generate', methods=['POST'])
def generate_compliance_report():
    """Generate SEC/FCA formatted compliance reports"""
    data = request.json
    session = verify_org_session(request.headers.get('Authorization', '').replace('Bearer ', ''))
    if not session:
        return jsonify({'success': False, 'error': 'Unauthorized'}), 401
    
    report_id = f"RPT_{data['reportType']}_{int(time.time())}_{uuid.uuid4().hex[:8]}"
    
    # Report types: SEC_13F, SEC_13D, FCA_TRANSACTION, MAS_QUARTERLY, CUSTOM_AUDIT
    report_type = data['reportType']
    format_type = data.get('format', 'JSON')  # JSON, CSV, XML, PDF
    
    conn = get_db()
    c = conn.cursor()
    
    # Generate report based on type
    report_data = {}
    
    if report_type == 'SEC_13F':
        # SEC Form 13F - Institutional Holdings Report
        report_data = generate_sec_13f_report(c, data.get('parameters', {}))
    elif report_type == 'SEC_13D':
        # SEC Form 13D - Beneficial Ownership Report
        report_data = generate_sec_13d_report(c, data.get('parameters', {}))
    elif report_type == 'FCA_TRANSACTION':
        # FCA Transaction Report
        report_data = generate_fca_transaction_report(c, data.get('parameters', {}))
    elif report_type == 'TRANSACTION_HISTORY':
        # Full transaction history for audit
        report_data = generate_transaction_history(c, data.get('parameters', {}))
    elif report_type == 'WASH_TRADING':
        # Wash trading analysis report
        report_data = generate_wash_trading_report(c, data.get('parameters', {}))
    elif report_type == 'OWNERSHIP_CHANGES':
        # Beneficial ownership changes report
        report_data = generate_ownership_changes_report(c, data.get('parameters', {}))
    
    # Store report metadata
    c.execute('''INSERT INTO compliance_reports 
        (report_id, org_id, report_type, format, parameters, generated_at, generated_by, status)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?)''',
        (report_id, session['org_id'], report_type, format_type, 
         json.dumps(data.get('parameters', {})), int(time.time()), data.get('userId'), 'GENERATED'))
    
    conn.commit()
    conn.close()
    
    log_access(session['org_id'], data.get('userId'), 'generate_report', report_type)
    
    return jsonify({
        'success': True,
        'reportId': report_id,
        'reportType': report_type,
        'format': format_type,
        'generatedAt': int(time.time()),
        'data': report_data
    })

def generate_sec_13f_report(cursor, params):
    """Generate SEC Form 13F format report"""
    cursor.execute('''SELECT stock_symbol, holder_address, holder_name, holder_type, 
        shares_held, percentage FROM beneficial_ownership 
        WHERE shares_held > 0 ORDER BY stock_symbol, shares_held DESC''')
    holdings = cursor.fetchall()
    
    return {
        'formType': '13F-HR',
        'reportingPeriod': params.get('period', 'Q4 2025'),
        'filingDate': datetime.now().strftime('%Y-%m-%d'),
        'holdings': [{
            'nameOfIssuer': h['stock_symbol'],
            'titleOfClass': 'Common Stock',
            'cusip': f"GXC{h['stock_symbol'][:6]}",
            'value': int(h['shares_held'] * 100),  # In thousands
            'sharesOrPrincipal': h['shares_held'],
            'investmentDiscretion': 'SOLE',
            'votingAuthority': {'sole': h['shares_held'], 'shared': 0, 'none': 0},
            'manager': h['holder_name'] or h['holder_address'][:20]
        } for h in holdings]
    }

def generate_sec_13d_report(cursor, params):
    """Generate SEC Form 13D format report - Beneficial Ownership"""
    stock = params.get('stockSymbol')
    cursor.execute('''SELECT * FROM beneficial_ownership 
        WHERE stock_symbol = ? AND percentage >= 5.0 ORDER BY percentage DESC''', (stock,))
    owners = cursor.fetchall()
    
    return {
        'formType': '13D',
        'issuerName': stock,
        'filingDate': datetime.now().strftime('%Y-%m-%d'),
        'beneficialOwners': [{
            'name': o['holder_name'] or o['holder_address'],
            'address': o['holder_address'],
            'citizenship': 'N/A',
            'sharesOwned': o['shares_held'],
            'percentageOwned': o['percentage'],
            'typeOfOwnership': o['holder_type'],
            'lastChange': datetime.fromtimestamp(o['last_change']).strftime('%Y-%m-%d') if o['last_change'] else None,
            'changeType': o['change_type']
        } for o in owners]
    }

def generate_fca_transaction_report(cursor, params):
    """Generate FCA Transaction Report format"""
    start_time = params.get('startTime', int(time.time()) - 86400 * 30)
    end_time = params.get('endTime', int(time.time()))
    
    cursor.execute('''SELECT * FROM transaction_audit 
        WHERE timestamp BETWEEN ? AND ? ORDER BY timestamp DESC''', (start_time, end_time))
    txs = cursor.fetchall()
    
    return {
        'reportType': 'FCA_TRANSACTION_REPORT',
        'reportingPeriod': {
            'from': datetime.fromtimestamp(start_time).strftime('%Y-%m-%d'),
            'to': datetime.fromtimestamp(end_time).strftime('%Y-%m-%d')
        },
        'totalTransactions': len(txs),
        'transactions': [{
            'transactionReferenceNumber': t['tx_hash'],
            'tradingDateTime': datetime.fromtimestamp(t['timestamp']).isoformat(),
            'instrumentIdentification': t['stock_symbol'],
            'price': t['price'],
            'quantity': t['amount'],
            'buyerIdentification': t['to_address'],
            'sellerIdentification': t['from_address'],
            'transactionType': t['tx_type'],
            'marketMaker': t['market_maker_id'],
            'traceability': t['traced_origin']
        } for t in txs]
    }

def generate_transaction_history(cursor, params):
    """Generate full transaction history for audits"""
    stock = params.get('stockSymbol')
    address = params.get('address')
    start_time = params.get('startTime', 0)
    end_time = params.get('endTime', int(time.time()))
    
    query = 'SELECT * FROM transaction_audit WHERE timestamp BETWEEN ? AND ?'
    query_params = [start_time, end_time]
    
    if stock:
        query += ' AND stock_symbol = ?'
        query_params.append(stock)
    if address:
        query += ' AND (from_address = ? OR to_address = ?)'
        query_params.extend([address, address])
    
    query += ' ORDER BY timestamp DESC LIMIT 10000'
    cursor.execute(query, query_params)
    txs = cursor.fetchall()
    
    return {
        'exportDate': datetime.now().isoformat(),
        'totalRecords': len(txs),
        'filters': params,
        'transactions': [dict(t) for t in txs]
    }

def generate_wash_trading_report(cursor, params):
    """Generate wash trading analysis report"""
    cursor.execute('''SELECT * FROM wash_trading_alerts 
        WHERE detected_at > ? ORDER BY confidence_score DESC''',
        (params.get('since', int(time.time()) - 86400 * 30),))
    alerts = cursor.fetchall()
    
    return {
        'reportType': 'WASH_TRADING_ANALYSIS',
        'generatedAt': datetime.now().isoformat(),
        'totalAlerts': len(alerts),
        'alerts': [{
            'alertId': a['alert_id'],
            'detectedAt': datetime.fromtimestamp(a['detected_at']).isoformat(),
            'stockSymbol': a['stock_symbol'],
            'patternType': a['pattern_type'],
            'confidenceScore': a['confidence_score'],
            'addressesInvolved': json.loads(a['addresses_involved']) if a['addresses_involved'] else [],
            'totalVolume': a['total_volume'],
            'timeWindowMinutes': a['time_window'],
            'status': a['status'],
            'reviewNotes': a['notes']
        } for a in alerts]
    }

def generate_ownership_changes_report(cursor, params):
    """Generate beneficial ownership changes report"""
    cursor.execute('''SELECT * FROM beneficial_ownership 
        WHERE last_change > ? ORDER BY last_change DESC''',
        (params.get('since', int(time.time()) - 86400 * 30),))
    changes = cursor.fetchall()
    
    return {
        'reportType': 'OWNERSHIP_CHANGES',
        'generatedAt': datetime.now().isoformat(),
        'totalChanges': len(changes),
        'changes': [{
            'stockSymbol': c['stock_symbol'],
            'holderAddress': c['holder_address'],
            'holderName': c['holder_name'],
            'sharesHeld': c['shares_held'],
            'percentageOwned': c['percentage'],
            'changeType': c['change_type'],
            'changeDate': datetime.fromtimestamp(c['last_change']).isoformat() if c['last_change'] else None
        } for c in changes]
    }

@app.route('/api/v1/regulator/reports/export/<report_id>')
def export_report(report_id):
    """Export report in requested format (CSV, XML)"""
    format_type = request.args.get('format', 'CSV')
    session = verify_org_session(request.headers.get('Authorization', '').replace('Bearer ', ''))
    if not session:
        return jsonify({'success': False, 'error': 'Unauthorized'}), 401
    
    conn = get_db()
    c = conn.cursor()
    c.execute('SELECT * FROM compliance_reports WHERE report_id = ?', (report_id,))
    report = c.fetchone()
    conn.close()
    
    if not report:
        return jsonify({'success': False, 'error': 'Report not found'}), 404
    
    if format_type == 'CSV':
        # Generate CSV
        output = io.StringIO()
        writer = csv.writer(output)
        writer.writerow(['Report ID', 'Type', 'Generated At', 'Organization'])
        writer.writerow([report['report_id'], report['report_type'], 
                        datetime.fromtimestamp(report['generated_at']).isoformat(), 
                        report['org_id']])
        
        return Response(
            output.getvalue(),
            mimetype='text/csv',
            headers={'Content-Disposition': f'attachment; filename={report_id}.csv'}
        )
    
    return jsonify({'success': False, 'error': 'Format not supported'}), 400


# ========== TRANSACTION TRACEABILITY ==========

@app.route('/api/v1/regulator/transactions/trace/<tx_hash>')
def trace_transaction(tx_hash):
    """Trace a transaction back to its origin using GXC traceability formula"""
    session = verify_org_session(request.headers.get('Authorization', '').replace('Bearer ', ''))
    if not session:
        return jsonify({'success': False, 'error': 'Unauthorized'}), 401
    
    conn = get_db()
    c = conn.cursor()
    
    # Get the transaction
    c.execute('SELECT * FROM transaction_audit WHERE tx_hash = ?', (tx_hash,))
    tx = c.fetchone()
    
    if not tx:
        return jsonify({'success': False, 'error': 'Transaction not found'}), 404
    
    # Build trace chain back to origin
    trace_chain = []
    current_tx = dict(tx)
    trace_chain.append(current_tx)
    
    # Follow the traced_origin chain
    while current_tx.get('traced_origin'):
        c.execute('SELECT * FROM transaction_audit WHERE tx_hash = ?', (current_tx['traced_origin'],))
        prev_tx = c.fetchone()
        if prev_tx:
            current_tx = dict(prev_tx)
            trace_chain.append(current_tx)
        else:
            break
    
    conn.close()
    
    log_access(session['org_id'], None, 'trace_transaction', tx_hash)
    
    return jsonify({
        'success': True,
        'txHash': tx_hash,
        'traceDepth': len(trace_chain),
        'originTransaction': trace_chain[-1] if trace_chain else None,
        'traceChain': trace_chain
    })

@app.route('/api/v1/regulator/transactions/search', methods=['POST'])
def search_transactions():
    """Search transactions with filters"""
    data = request.json
    session = verify_org_session(request.headers.get('Authorization', '').replace('Bearer ', ''))
    if not session:
        return jsonify({'success': False, 'error': 'Unauthorized'}), 401
    
    conn = get_db()
    c = conn.cursor()
    
    query = 'SELECT * FROM transaction_audit WHERE 1=1'
    params = []
    
    if data.get('stockSymbol'):
        query += ' AND stock_symbol = ?'
        params.append(data['stockSymbol'])
    if data.get('fromAddress'):
        query += ' AND from_address = ?'
        params.append(data['fromAddress'])
    if data.get('toAddress'):
        query += ' AND to_address = ?'
        params.append(data['toAddress'])
    if data.get('minAmount'):
        query += ' AND amount >= ?'
        params.append(data['minAmount'])
    if data.get('maxAmount'):
        query += ' AND amount <= ?'
        params.append(data['maxAmount'])
    if data.get('startTime'):
        query += ' AND timestamp >= ?'
        params.append(data['startTime'])
    if data.get('endTime'):
        query += ' AND timestamp <= ?'
        params.append(data['endTime'])
    if data.get('txType'):
        query += ' AND tx_type = ?'
        params.append(data['txType'])
    if data.get('marketMakerId'):
        query += ' AND market_maker_id = ?'
        params.append(data['marketMakerId'])
    if data.get('flagged'):
        query += ' AND flags IS NOT NULL AND flags != ""'
    
    query += ' ORDER BY timestamp DESC LIMIT ?'
    params.append(data.get('limit', 1000))
    
    c.execute(query, params)
    txs = [dict(row) for row in c.fetchall()]
    conn.close()
    
    log_access(session['org_id'], None, 'search_transactions', json.dumps(data))
    
    return jsonify({
        'success': True,
        'totalResults': len(txs),
        'transactions': txs
    })

# ========== BENEFICIAL OWNERSHIP TRACKING ==========

@app.route('/api/v1/regulator/ownership/<stock_symbol>')
def get_ownership(stock_symbol):
    """Get beneficial ownership for a stock"""
    session = verify_org_session(request.headers.get('Authorization', '').replace('Bearer ', ''))
    if not session:
        return jsonify({'success': False, 'error': 'Unauthorized'}), 401
    
    conn = get_db()
    c = conn.cursor()
    
    c.execute('''SELECT * FROM beneficial_ownership 
        WHERE stock_symbol = ? ORDER BY percentage DESC''', (stock_symbol,))
    owners = [dict(row) for row in c.fetchall()]
    
    # Calculate totals
    total_shares = sum(o['shares_held'] for o in owners)
    major_holders = [o for o in owners if o['percentage'] >= 5.0]
    
    conn.close()
    
    log_access(session['org_id'], None, 'view_ownership', stock_symbol)
    
    return jsonify({
        'success': True,
        'stockSymbol': stock_symbol,
        'totalSharesTracked': total_shares,
        'totalHolders': len(owners),
        'majorHolders': len(major_holders),
        'ownership': owners
    })

@app.route('/api/v1/regulator/ownership/changes')
def get_ownership_changes():
    """Get recent beneficial ownership changes"""
    session = verify_org_session(request.headers.get('Authorization', '').replace('Bearer ', ''))
    if not session:
        return jsonify({'success': False, 'error': 'Unauthorized'}), 401
    
    days = int(request.args.get('days', 30))
    threshold = float(request.args.get('threshold', 1.0))  # Minimum % change to report
    
    since = int(time.time()) - (days * 86400)
    
    conn = get_db()
    c = conn.cursor()
    
    c.execute('''SELECT * FROM beneficial_ownership 
        WHERE last_change > ? ORDER BY last_change DESC''', (since,))
    changes = [dict(row) for row in c.fetchall()]
    
    conn.close()
    
    # Filter by threshold
    significant_changes = [c for c in changes if abs(c.get('percentage', 0)) >= threshold]
    
    return jsonify({
        'success': True,
        'periodDays': days,
        'thresholdPercent': threshold,
        'totalChanges': len(significant_changes),
        'changes': significant_changes
    })

@app.route('/api/v1/regulator/ownership/report', methods=['POST'])
def report_ownership_change():
    """Market maker reports ownership change"""
    data = request.json
    
    conn = get_db()
    c = conn.cursor()
    
    ownership_id = f"OWN_{data['stockSymbol']}_{uuid.uuid4().hex[:8]}"
    
    c.execute('''INSERT OR REPLACE INTO beneficial_ownership 
        (ownership_id, stock_symbol, holder_address, holder_name, holder_type, 
         shares_held, percentage, last_change, change_type, reported_at)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)''',
        (ownership_id, data['stockSymbol'], data['holderAddress'], 
         data.get('holderName'), data.get('holderType', 'INDIVIDUAL'),
         data['sharesHeld'], data['percentage'], int(time.time()),
         data.get('changeType', 'UPDATE'), int(time.time())))
    
    conn.commit()
    conn.close()
    
    return jsonify({
        'success': True,
        'ownershipId': ownership_id,
        'message': 'Ownership change reported successfully'
    })

# ========== WASH TRADING DETECTION ==========

@app.route('/api/v1/regulator/wash-trading/alerts')
def get_wash_trading_alerts():
    """Get wash trading alerts"""
    session = verify_org_session(request.headers.get('Authorization', '').replace('Bearer ', ''))
    if not session:
        return jsonify({'success': False, 'error': 'Unauthorized'}), 401
    
    status = request.args.get('status', 'PENDING')
    
    conn = get_db()
    c = conn.cursor()
    
    if status == 'ALL':
        c.execute('SELECT * FROM wash_trading_alerts ORDER BY detected_at DESC LIMIT 100')
    else:
        c.execute('SELECT * FROM wash_trading_alerts WHERE status = ? ORDER BY detected_at DESC LIMIT 100', (status,))
    
    alerts = []
    for row in c.fetchall():
        alert = dict(row)
        alert['addresses_involved'] = json.loads(alert['addresses_involved']) if alert['addresses_involved'] else []
        alerts.append(alert)
    
    conn.close()
    
    return jsonify({
        'success': True,
        'totalAlerts': len(alerts),
        'alerts': alerts
    })

@app.route('/api/v1/regulator/wash-trading/analyze', methods=['POST'])
def analyze_wash_trading():
    """Analyze transactions for wash trading patterns"""
    data = request.json
    session = verify_org_session(request.headers.get('Authorization', '').replace('Bearer ', ''))
    if not session:
        return jsonify({'success': False, 'error': 'Unauthorized'}), 401
    
    conn = get_db()
    c = conn.cursor()
    
    stock = data.get('stockSymbol')
    time_window = data.get('timeWindowMinutes', 60)
    start_time = int(time.time()) - (time_window * 60)
    
    # Get recent transactions
    c.execute('''SELECT * FROM transaction_audit 
        WHERE stock_symbol = ? AND timestamp > ? ORDER BY timestamp''', (stock, start_time))
    txs = [dict(row) for row in c.fetchall()]
    
    # Detect patterns
    patterns_found = []
    
    # Pattern 1: Same address buying and selling
    address_activity = {}
    for tx in txs:
        from_addr = tx['from_address']
        to_addr = tx['to_address']
        
        if from_addr not in address_activity:
            address_activity[from_addr] = {'buys': 0, 'sells': 0, 'volume': 0}
        if to_addr not in address_activity:
            address_activity[to_addr] = {'buys': 0, 'sells': 0, 'volume': 0}
        
        address_activity[from_addr]['sells'] += 1
        address_activity[from_addr]['volume'] += tx['amount'] or 0
        address_activity[to_addr]['buys'] += 1
        address_activity[to_addr]['volume'] += tx['amount'] or 0
    
    # Find addresses with both buys and sells (potential wash trading)
    suspicious_addresses = []
    for addr, activity in address_activity.items():
        if activity['buys'] > 0 and activity['sells'] > 0:
            suspicious_addresses.append({
                'address': addr,
                'buys': activity['buys'],
                'sells': activity['sells'],
                'totalVolume': activity['volume']
            })
    
    # Pattern 2: Circular trading (A->B->C->A)
    # Simplified detection
    
    if suspicious_addresses:
        alert_id = f"WASH_{stock}_{int(time.time())}_{uuid.uuid4().hex[:6]}"
        confidence = min(0.9, len(suspicious_addresses) * 0.15 + 0.3)
        total_volume = sum(a['totalVolume'] for a in suspicious_addresses)
        
        c.execute('''INSERT INTO wash_trading_alerts 
            (alert_id, detected_at, stock_symbol, addresses_involved, pattern_type, 
             confidence_score, total_volume, time_window, status)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)''',
            (alert_id, int(time.time()), stock, 
             json.dumps([a['address'] for a in suspicious_addresses]),
             'SELF_TRADING', confidence, total_volume, time_window, 'PENDING'))
        
        patterns_found.append({
            'alertId': alert_id,
            'patternType': 'SELF_TRADING',
            'confidence': confidence,
            'suspiciousAddresses': suspicious_addresses
        })
        
        conn.commit()
    
    conn.close()
    
    log_access(session['org_id'], None, 'analyze_wash_trading', stock)
    
    return jsonify({
        'success': True,
        'stockSymbol': stock,
        'timeWindowMinutes': time_window,
        'transactionsAnalyzed': len(txs),
        'patternsFound': len(patterns_found),
        'patterns': patterns_found
    })

@app.route('/api/v1/regulator/wash-trading/review', methods=['POST'])
def review_wash_trading_alert():
    """Review and update wash trading alert status"""
    data = request.json
    session = verify_org_session(request.headers.get('Authorization', '').replace('Bearer ', ''))
    if not session:
        return jsonify({'success': False, 'error': 'Unauthorized'}), 401
    
    conn = get_db()
    c = conn.cursor()
    
    c.execute('''UPDATE wash_trading_alerts 
        SET status = ?, reviewed_by = ?, reviewed_at = ?, notes = ?
        WHERE alert_id = ?''',
        (data['status'], data.get('reviewedBy'), int(time.time()), 
         data.get('notes'), data['alertId']))
    
    conn.commit()
    conn.close()
    
    log_access(session['org_id'], data.get('reviewedBy'), 'review_wash_trading', data['alertId'])
    
    return jsonify({
        'success': True,
        'alertId': data['alertId'],
        'newStatus': data['status'],
        'message': 'Alert reviewed successfully'
    })


# ========== COMMUNICATION HUB ==========

@app.route('/api/v1/regulator/communications/send', methods=['POST'])
def send_communication():
    """Send official notice to market maker"""
    data = request.json
    session = verify_org_session(request.headers.get('Authorization', '').replace('Bearer ', ''))
    if not session:
        return jsonify({'success': False, 'error': 'Unauthorized'}), 401
    
    conn = get_db()
    c = conn.cursor()
    
    comm_id = f"COMM_{data['commType']}_{int(time.time())}_{uuid.uuid4().hex[:8]}"
    
    # Calculate response deadline if required
    response_deadline = None
    if data.get('requiresResponse'):
        deadline_days = data.get('responseDeadlineDays', 14)
        response_deadline = int(time.time()) + (deadline_days * 86400)
    
    c.execute('''INSERT INTO communications 
        (comm_id, from_org_id, to_market_maker_id, comm_type, subject, message, 
         priority, requires_response, response_deadline, status, sent_at, attachments)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)''',
        (comm_id, session['org_id'], data['toMarketMakerId'], data['commType'],
         data['subject'], data['message'], data.get('priority', 'NORMAL'),
         1 if data.get('requiresResponse') else 0, response_deadline,
         'SENT', int(time.time()), json.dumps(data.get('attachments', []))))
    
    conn.commit()
    conn.close()
    
    log_access(session['org_id'], None, 'send_communication', comm_id)
    
    return jsonify({
        'success': True,
        'communicationId': comm_id,
        'sentAt': int(time.time()),
        'responseDeadline': response_deadline,
        'message': f"Communication sent to {data['toMarketMakerId']}"
    })

@app.route('/api/v1/regulator/communications/templates', methods=['GET', 'POST'])
def manage_templates():
    """Get or create communication templates"""
    session = verify_org_session(request.headers.get('Authorization', '').replace('Bearer ', ''))
    if not session:
        return jsonify({'success': False, 'error': 'Unauthorized'}), 401
    
    conn = get_db()
    c = conn.cursor()
    
    if request.method == 'GET':
        c.execute('SELECT * FROM comm_templates ORDER BY template_name')
        templates = [dict(row) for row in c.fetchall()]
        conn.close()
        return jsonify({'success': True, 'templates': templates})
    
    # POST - Create template
    data = request.json
    template_id = f"TPL_{data['commType']}_{uuid.uuid4().hex[:8]}"
    
    c.execute('''INSERT INTO comm_templates 
        (template_id, template_name, comm_type, subject_template, message_template, 
         default_priority, default_deadline_days, created_by, created_at)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)''',
        (template_id, data['templateName'], data['commType'], data['subjectTemplate'],
         data['messageTemplate'], data.get('defaultPriority', 'NORMAL'),
         data.get('defaultDeadlineDays', 14), data.get('createdBy'), int(time.time())))
    
    conn.commit()
    conn.close()
    
    return jsonify({
        'success': True,
        'templateId': template_id,
        'message': 'Template created successfully'
    })

@app.route('/api/v1/regulator/communications/inbox')
def get_communications_inbox():
    """Get communications sent to market makers (for regulator view)"""
    session = verify_org_session(request.headers.get('Authorization', '').replace('Bearer ', ''))
    if not session:
        return jsonify({'success': False, 'error': 'Unauthorized'}), 401
    
    status = request.args.get('status')
    market_maker = request.args.get('marketMakerId')
    
    conn = get_db()
    c = conn.cursor()
    
    query = 'SELECT * FROM communications WHERE from_org_id = ?'
    params = [session['org_id']]
    
    if status:
        query += ' AND status = ?'
        params.append(status)
    if market_maker:
        query += ' AND to_market_maker_id = ?'
        params.append(market_maker)
    
    query += ' ORDER BY sent_at DESC LIMIT 100'
    
    c.execute(query, params)
    comms = []
    for row in c.fetchall():
        comm = dict(row)
        comm['attachments'] = json.loads(comm['attachments']) if comm['attachments'] else []
        comms.append(comm)
    
    conn.close()
    
    return jsonify({
        'success': True,
        'totalCommunications': len(comms),
        'communications': comms
    })

@app.route('/api/v1/regulator/communications/pending-responses')
def get_pending_responses():
    """Get communications awaiting response"""
    session = verify_org_session(request.headers.get('Authorization', '').replace('Bearer ', ''))
    if not session:
        return jsonify({'success': False, 'error': 'Unauthorized'}), 401
    
    conn = get_db()
    c = conn.cursor()
    
    c.execute('''SELECT * FROM communications 
        WHERE from_org_id = ? AND requires_response = 1 AND responded_at IS NULL
        ORDER BY response_deadline ASC''', (session['org_id'],))
    
    pending = []
    now = int(time.time())
    for row in c.fetchall():
        comm = dict(row)
        comm['attachments'] = json.loads(comm['attachments']) if comm['attachments'] else []
        comm['isOverdue'] = comm['response_deadline'] and comm['response_deadline'] < now
        comm['daysUntilDeadline'] = (comm['response_deadline'] - now) // 86400 if comm['response_deadline'] else None
        pending.append(comm)
    
    conn.close()
    
    overdue = [p for p in pending if p['isOverdue']]
    
    return jsonify({
        'success': True,
        'totalPending': len(pending),
        'overdueCount': len(overdue),
        'pendingResponses': pending
    })

@app.route('/api/v1/regulator/communications/<comm_id>/response', methods=['POST'])
def record_response():
    """Record market maker's response to communication"""
    data = request.json
    
    conn = get_db()
    c = conn.cursor()
    
    c.execute('''UPDATE communications 
        SET responded_at = ?, response = ?, status = ?
        WHERE comm_id = ?''',
        (int(time.time()), data['response'], 'RESPONDED', data.get('commId') or comm_id))
    
    conn.commit()
    conn.close()
    
    return jsonify({
        'success': True,
        'message': 'Response recorded successfully'
    })

# ========== REGULATORY BROADCASTS ==========

@app.route('/api/v1/regulator/broadcasts/send', methods=['POST'])
def send_broadcast():
    """Send regulatory update to all market makers"""
    data = request.json
    session = verify_org_session(request.headers.get('Authorization', '').replace('Bearer ', ''))
    if not session:
        return jsonify({'success': False, 'error': 'Unauthorized'}), 401
    
    conn = get_db()
    c = conn.cursor()
    
    broadcast_id = f"BROADCAST_{int(time.time())}_{uuid.uuid4().hex[:8]}"
    
    c.execute('''INSERT INTO broadcasts 
        (broadcast_id, org_id, title, message, broadcast_type, target_audience, 
         effective_date, expiry_date, sent_at, sent_by)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)''',
        (broadcast_id, session['org_id'], data['title'], data['message'],
         data['broadcastType'], data.get('targetAudience', 'ALL_MARKET_MAKERS'),
         data.get('effectiveDate'), data.get('expiryDate'),
         int(time.time()), data.get('sentBy')))
    
    conn.commit()
    conn.close()
    
    log_access(session['org_id'], data.get('sentBy'), 'send_broadcast', broadcast_id)
    
    return jsonify({
        'success': True,
        'broadcastId': broadcast_id,
        'sentAt': int(time.time()),
        'message': 'Broadcast sent to all market makers'
    })

@app.route('/api/v1/regulator/broadcasts')
def get_broadcasts():
    """Get all broadcasts"""
    session = verify_org_session(request.headers.get('Authorization', '').replace('Bearer ', ''))
    if not session:
        return jsonify({'success': False, 'error': 'Unauthorized'}), 401
    
    conn = get_db()
    c = conn.cursor()
    
    c.execute('''SELECT * FROM broadcasts 
        WHERE org_id = ? ORDER BY sent_at DESC LIMIT 50''', (session['org_id'],))
    
    broadcasts = []
    for row in c.fetchall():
        b = dict(row)
        b['acknowledgments'] = json.loads(b['acknowledgments']) if b['acknowledgments'] else []
        broadcasts.append(b)
    
    conn.close()
    
    return jsonify({
        'success': True,
        'totalBroadcasts': len(broadcasts),
        'broadcasts': broadcasts
    })

@app.route('/api/v1/regulator/broadcasts/<broadcast_id>/acknowledge', methods=['POST'])
def acknowledge_broadcast(broadcast_id):
    """Market maker acknowledges receipt of broadcast"""
    data = request.json
    
    conn = get_db()
    c = conn.cursor()
    
    c.execute('SELECT acknowledgments FROM broadcasts WHERE broadcast_id = ?', (broadcast_id,))
    row = c.fetchone()
    
    if row:
        acks = json.loads(row['acknowledgments']) if row['acknowledgments'] else []
        acks.append({
            'marketMakerId': data['marketMakerId'],
            'acknowledgedAt': int(time.time())
        })
        c.execute('UPDATE broadcasts SET acknowledgments = ? WHERE broadcast_id = ?',
                  (json.dumps(acks), broadcast_id))
        conn.commit()
    
    conn.close()
    
    return jsonify({
        'success': True,
        'message': 'Broadcast acknowledged'
    })

# ========== COMPLIANCE WARNINGS ==========

@app.route('/api/v1/regulator/warnings/issue', methods=['POST'])
def issue_warning():
    """Issue compliance warning to market maker"""
    data = request.json
    session = verify_org_session(request.headers.get('Authorization', '').replace('Bearer ', ''))
    if not session:
        return jsonify({'success': False, 'error': 'Unauthorized'}), 401
    
    # Create warning as high-priority communication
    warning_data = {
        'toMarketMakerId': data['marketMakerId'],
        'commType': 'COMPLIANCE_WARNING',
        'subject': f"Compliance Warning: {data['warningType']}",
        'message': data['message'],
        'priority': 'HIGH',
        'requiresResponse': True,
        'responseDeadlineDays': data.get('responseDeadlineDays', 7),
        'attachments': data.get('attachments', [])
    }
    
    conn = get_db()
    c = conn.cursor()
    
    comm_id = f"WARNING_{data['warningType']}_{int(time.time())}_{uuid.uuid4().hex[:6]}"
    response_deadline = int(time.time()) + (warning_data['responseDeadlineDays'] * 86400)
    
    c.execute('''INSERT INTO communications 
        (comm_id, from_org_id, to_market_maker_id, comm_type, subject, message, 
         priority, requires_response, response_deadline, status, sent_at, attachments)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)''',
        (comm_id, session['org_id'], data['marketMakerId'], 'COMPLIANCE_WARNING',
         warning_data['subject'], warning_data['message'], 'HIGH',
         1, response_deadline, 'SENT', int(time.time()), 
         json.dumps(warning_data['attachments'])))
    
    conn.commit()
    conn.close()
    
    log_access(session['org_id'], None, 'issue_warning', comm_id)
    
    return jsonify({
        'success': True,
        'warningId': comm_id,
        'warningType': data['warningType'],
        'issuedAt': int(time.time()),
        'responseDeadline': response_deadline,
        'message': f"Compliance warning issued to {data['marketMakerId']}"
    })

@app.route('/api/v1/regulator/warnings/history/<market_maker_id>')
def get_warning_history(market_maker_id):
    """Get warning history for a market maker"""
    session = verify_org_session(request.headers.get('Authorization', '').replace('Bearer ', ''))
    if not session:
        return jsonify({'success': False, 'error': 'Unauthorized'}), 401
    
    conn = get_db()
    c = conn.cursor()
    
    c.execute('''SELECT * FROM communications 
        WHERE to_market_maker_id = ? AND comm_type = 'COMPLIANCE_WARNING'
        ORDER BY sent_at DESC''', (market_maker_id,))
    
    warnings = [dict(row) for row in c.fetchall()]
    conn.close()
    
    return jsonify({
        'success': True,
        'marketMakerId': market_maker_id,
        'totalWarnings': len(warnings),
        'warnings': warnings
    })

# ========== DOCUMENT REQUESTS ==========

@app.route('/api/v1/regulator/documents/request', methods=['POST'])
def request_documents():
    """Request additional documentation from market maker"""
    data = request.json
    session = verify_org_session(request.headers.get('Authorization', '').replace('Bearer ', ''))
    if not session:
        return jsonify({'success': False, 'error': 'Unauthorized'}), 401
    
    doc_request = {
        'toMarketMakerId': data['marketMakerId'],
        'commType': 'DOCUMENT_REQUEST',
        'subject': f"Document Request: {data['documentType']}",
        'message': f"""
Dear {data['marketMakerId']},

We are requesting the following documentation:

Document Type: {data['documentType']}
Purpose: {data['purpose']}
Details: {data.get('details', 'N/A')}

Please submit the requested documents by the deadline specified.

Required Documents:
{chr(10).join('- ' + doc for doc in data.get('requiredDocuments', []))}

Regards,
{session['org_name']}
        """,
        'priority': data.get('priority', 'NORMAL'),
        'requiresResponse': True,
        'responseDeadlineDays': data.get('deadlineDays', 14)
    }
    
    conn = get_db()
    c = conn.cursor()
    
    comm_id = f"DOCREQ_{int(time.time())}_{uuid.uuid4().hex[:6]}"
    response_deadline = int(time.time()) + (doc_request['responseDeadlineDays'] * 86400)
    
    c.execute('''INSERT INTO communications 
        (comm_id, from_org_id, to_market_maker_id, comm_type, subject, message, 
         priority, requires_response, response_deadline, status, sent_at)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)''',
        (comm_id, session['org_id'], data['marketMakerId'], 'DOCUMENT_REQUEST',
         doc_request['subject'], doc_request['message'], doc_request['priority'],
         1, response_deadline, 'SENT', int(time.time())))
    
    conn.commit()
    conn.close()
    
    log_access(session['org_id'], None, 'request_documents', comm_id)
    
    return jsonify({
        'success': True,
        'requestId': comm_id,
        'documentType': data['documentType'],
        'deadline': response_deadline,
        'message': f"Document request sent to {data['marketMakerId']}"
    })

# ========== ACCESS LOG & AUDIT ==========

@app.route('/api/v1/regulator/access-log')
def get_access_log():
    """Get access log for audit purposes"""
    session = verify_org_session(request.headers.get('Authorization', '').replace('Bearer ', ''))
    if not session:
        return jsonify({'success': False, 'error': 'Unauthorized'}), 401
    
    days = int(request.args.get('days', 30))
    since = int(time.time()) - (days * 86400)
    
    conn = get_db()
    c = conn.cursor()
    
    c.execute('''SELECT * FROM access_log 
        WHERE org_id = ? AND timestamp > ? ORDER BY timestamp DESC LIMIT 1000''',
        (session['org_id'], since))
    
    logs = [dict(row) for row in c.fetchall()]
    conn.close()
    
    return jsonify({
        'success': True,
        'periodDays': days,
        'totalLogs': len(logs),
        'accessLog': logs
    })

# ========== DEMO DATA ==========

def add_demo_data():
    """Add demo data for testing"""
    conn = get_db()
    c = conn.cursor()
    
    # Check if demo data exists
    c.execute('SELECT COUNT(*) as cnt FROM organizations')
    if c.fetchone()['cnt'] == 0:
        # Add SEC as demo organization
        sec_api_key = hashlib.sha256('SEC_API_KEY_2025'.encode()).hexdigest()
        c.execute('''INSERT INTO organizations 
            (org_id, org_name, org_type, country, api_key_hash, permissions, created_at, created_by)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?)''',
            ('ORG_SEC_USA_demo', 'U.S. Securities and Exchange Commission', 'REGULATOR', 'USA',
             sec_api_key, json.dumps(['all']), int(time.time()), 'GXC_ADMIN'))
        
        # Add SEC user
        pw_hash = hashlib.sha256('SEC_User_2025!'.encode()).hexdigest()
        c.execute('''INSERT INTO org_users 
            (user_id, org_id, username, password_hash, role, email, created_at)
            VALUES (?, ?, ?, ?, ?, ?, ?)''',
            ('USER_SEC_001', 'ORG_SEC_USA_demo', 'sec_analyst', pw_hash, 
             'analyst', 'analyst@sec.gov', int(time.time())))
        
        # Add demo transactions
        demo_txs = [
            ('TX_001', 'AAPL', 'tGXC_user1', 'tGXC_user2', 100, 150.50, 'BUY', 'MM_Goldman'),
            ('TX_002', 'AAPL', 'tGXC_user2', 'tGXC_user3', 50, 151.00, 'BUY', 'MM_Goldman'),
            ('TX_003', 'TSLA', 'tGXC_user1', 'tGXC_user4', 25, 250.00, 'BUY', 'MM_Citadel'),
            ('TX_004', 'AAPL', 'tGXC_user3', 'tGXC_user1', 30, 149.50, 'SELL', 'MM_Goldman'),
        ]
        
        for tx_hash, symbol, from_addr, to_addr, amount, price, tx_type, mm in demo_txs:
            c.execute('''INSERT INTO transaction_audit 
                (audit_id, tx_hash, stock_symbol, from_address, to_address, amount, price, 
                 timestamp, tx_type, market_maker_id)
                VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)''',
                (f"AUDIT_{tx_hash}", tx_hash, symbol, from_addr, to_addr, amount, price,
                 int(time.time()) - 3600, tx_type, mm))
        
        # Add demo ownership
        demo_ownership = [
            ('AAPL', 'tGXC_user1', 'John Smith', 'INDIVIDUAL', 1000, 5.5),
            ('AAPL', 'tGXC_user2', 'Jane Doe', 'INDIVIDUAL', 500, 2.75),
            ('AAPL', 'tGXC_fund1', 'Vanguard Fund', 'INSTITUTION', 5000, 27.5),
            ('TSLA', 'tGXC_user1', 'John Smith', 'INDIVIDUAL', 200, 3.0),
        ]
        
        for symbol, addr, name, holder_type, shares, pct in demo_ownership:
            c.execute('''INSERT INTO beneficial_ownership 
                (ownership_id, stock_symbol, holder_address, holder_name, holder_type, 
                 shares_held, percentage, last_change, reported_at)
                VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)''',
                (f"OWN_{symbol}_{addr}", symbol, addr, name, holder_type, shares, pct,
                 int(time.time()) - 86400, int(time.time())))
        
        # Add demo communication templates
        templates = [
            ('COMPLIANCE_WARNING', 'Standard Compliance Warning', 
             'Compliance Warning: {warning_type}',
             'Dear {market_maker},\n\nThis is to notify you of a compliance issue...\n\nRegards,\n{regulator}'),
            ('DOCUMENT_REQUEST', 'Document Request',
             'Document Request: {document_type}',
             'Dear {market_maker},\n\nWe are requesting the following documentation...\n\nRegards,\n{regulator}'),
            ('REGULATORY_UPDATE', 'Regulatory Update Notice',
             'Regulatory Update: {update_title}',
             'Dear Market Makers,\n\nPlease be advised of the following regulatory update...\n\nRegards,\n{regulator}'),
        ]
        
        for comm_type, name, subject, message in templates:
            c.execute('''INSERT INTO comm_templates 
                (template_id, template_name, comm_type, subject_template, message_template, created_at)
                VALUES (?, ?, ?, ?, ?, ?)''',
                (f"TPL_{comm_type}_default", name, comm_type, subject, message, int(time.time())))
        
        conn.commit()
    
    conn.close()

add_demo_data()

if __name__ == '__main__':
    print("=" * 60)
    print("GXC Regulator API - Compliance Tools & Communication Hub")
    print("=" * 60)
    print("\nDemo Credentials:")
    print("  Username: sec_analyst")
    print("  Password: SEC_User_2025!")
    print("\nEndpoints:")
    print("  POST /api/v1/regulator/organization/login")
    print("  POST /api/v1/regulator/reports/generate")
    print("  GET  /api/v1/regulator/transactions/trace/<tx_hash>")
    print("  GET  /api/v1/regulator/ownership/<stock_symbol>")
    print("  GET  /api/v1/regulator/wash-trading/alerts")
    print("  POST /api/v1/regulator/communications/send")
    print("  POST /api/v1/regulator/broadcasts/send")
    print("  POST /api/v1/regulator/warnings/issue")
    print("=" * 60)
    app.run(host='0.0.0.0', port=5002, debug=True)
