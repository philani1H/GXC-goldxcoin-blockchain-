#!/usr/bin/env python3
"""
GXC Regulator Server - Real Blockchain Connected
Powerful algorithms for market surveillance, wash trading detection,
beneficial ownership tracking, and regulatory compliance.

Connects directly to GXC blockchain node for real-time data.

Admin Login:
  Username: Philani-GXC.Foundation
  Password: GXC_SuperAdmin_2024!
"""
import os
import sys
import json
import sqlite3
import hashlib
import time
import uuid
import requests
import threading
from datetime import datetime, timedelta
from flask import Flask, request, jsonify, Response
from flask_cors import CORS
from collections import defaultdict
import math

app = Flask(__name__)
CORS(app)

# Configuration
RAILWAY_NODE = 'https://gxc-chain112-blockchain-node-production.up.railway.app'
LOCAL_NODE = 'http://localhost:8545'
DB_PATH = 'gxc_regulator.db'

print("=" * 70)
print("GXC REGULATOR SERVER - Blockchain Connected")
print("=" * 70)

# ========== BLOCKCHAIN CONNECTION ==========
class BlockchainNode:
    def __init__(self):
        self.url = None
        self.connected = False
        self.height = 0
    
    def connect(self):
        for url in [LOCAL_NODE, RAILWAY_NODE]:
            try:
                r = requests.post(url, json={"jsonrpc":"2.0","method":"getblockcount","params":[],"id":1}, timeout=5)
                if r.status_code == 200 and 'result' in r.json():
                    self.url = url
                    self.connected = True
                    self.height = r.json()['result']
                    print(f"✅ Connected to: {url} (height: {self.height})")
                    return True
            except: pass
        print("❌ No blockchain node available")
        return False
    
    def rpc(self, method, params=None):
        if not self.connected: self.connect()
        if not self.url: return None
        try:
            r = requests.post(self.url, json={"jsonrpc":"2.0","method":method,"params":params or [],"id":1}, timeout=30)
            return r.json().get('result') if r.status_code == 200 else None
        except: return None

node = BlockchainNode()
node.connect()

# ========== DATABASE ==========
def get_db():
    conn = sqlite3.connect(DB_PATH)
    conn.row_factory = sqlite3.Row
    return conn

def init_db():
    conn = get_db()
    c = conn.cursor()
    c.execute('''CREATE TABLE IF NOT EXISTS organizations (
        org_id TEXT PRIMARY KEY, org_name TEXT, org_type TEXT, country TEXT,
        api_key_hash TEXT, permissions TEXT, is_active INTEGER DEFAULT 1,
        created_at INTEGER, last_access INTEGER)''')
    c.execute('''CREATE TABLE IF NOT EXISTS org_users (
        user_id TEXT PRIMARY KEY, org_id TEXT, username TEXT UNIQUE,
        password_hash TEXT, role TEXT, is_active INTEGER DEFAULT 1,
        created_at INTEGER, last_login INTEGER)''')
    c.execute('''CREATE TABLE IF NOT EXISTS transactions (
        tx_hash TEXT PRIMARY KEY, block_height INTEGER, timestamp INTEGER,
        stock_symbol TEXT, from_address TEXT, to_address TEXT, amount REAL,
        price REAL, tx_type TEXT, market_maker_id TEXT, flags TEXT)''')
    c.execute('''CREATE TABLE IF NOT EXISTS beneficial_ownership (
        id INTEGER PRIMARY KEY, stock_symbol TEXT, holder_address TEXT,
        holder_name TEXT, shares_held REAL, percentage REAL, last_change INTEGER)''')
    c.execute('''CREATE TABLE IF NOT EXISTS wash_alerts (
        alert_id TEXT PRIMARY KEY, detected_at INTEGER, stock_symbol TEXT,
        pattern_type TEXT, confidence REAL, addresses TEXT, volume REAL,
        status TEXT DEFAULT 'PENDING', notes TEXT)''')
    c.execute('''CREATE TABLE IF NOT EXISTS communications (
        comm_id TEXT PRIMARY KEY, from_org TEXT, to_mm TEXT, comm_type TEXT,
        subject TEXT, message TEXT, priority TEXT, deadline INTEGER,
        status TEXT DEFAULT 'SENT', sent_at INTEGER, response TEXT)''')
    c.execute('''CREATE TABLE IF NOT EXISTS price_history (
        id INTEGER PRIMARY KEY, symbol TEXT, price REAL, volume REAL, ts INTEGER)''')
    # Demo SEC org
    c.execute('SELECT COUNT(*) FROM organizations')
    if c.fetchone()[0] == 0:
        key = hashlib.sha256('SEC_KEY_2025'.encode()).hexdigest()
        c.execute('INSERT INTO organizations VALUES (?,?,?,?,?,?,?,?,?)',
            ('ORG_SEC', 'SEC', 'REGULATOR', 'USA', key, '["all"]', 1, int(time.time()), None))
        pw = hashlib.sha256('SEC_Pass_2025!'.encode()).hexdigest()
        c.execute('INSERT INTO org_users VALUES (?,?,?,?,?,?,?,?)',
            ('USER_SEC_1', 'ORG_SEC', 'sec_analyst', pw, 'analyst', 1, int(time.time()), None))
    conn.commit()
    conn.close()

init_db()

# ========== WASH TRADING DETECTION ALGORITHM ==========
class WashTradingDetector:
    """Advanced wash trading detection - self-trading, circular, coordinated"""
    
    def analyze(self, symbol, minutes=60):
        conn = get_db()
        c = conn.cursor()
        cutoff = int(time.time()) - (minutes * 60)
        c.execute('SELECT * FROM transactions WHERE stock_symbol=? AND timestamp>?', (symbol, cutoff))
        txs = [dict(r) for r in c.fetchall()]
        conn.close()
        
        if len(txs) < 2:
            return {'patterns': [], 'risk': 0, 'level': 'NONE'}
        
        patterns = []
        
        # 1. Self-Trading: same address buys and sells
        activity = defaultdict(lambda: {'buys': 0, 'sells': 0, 'vol': 0})
        for tx in txs:
            f, t, amt = tx.get('from_address',''), tx.get('to_address',''), tx.get('amount',0) or 0
            if f: activity[f]['sells'] += 1; activity[f]['vol'] += amt
            if t: activity[t]['buys'] += 1; activity[t]['vol'] += amt
        
        self_traders = [{'addr': a, **d} for a, d in activity.items() if d['buys'] > 0 and d['sells'] > 0]
        if self_traders:
            conf = min(0.95, len(self_traders) * 0.2 + 0.3)
            patterns.append({'type': 'SELF_TRADING', 'confidence': conf, 'addresses': self_traders})
        
        # 2. Circular Trading: A->B->C->A
        graph = defaultdict(set)
        for tx in txs:
            f, t = tx.get('from_address',''), tx.get('to_address','')
            if f and t: graph[f].add(t)
        
        cycles = []
        for start in graph:
            visited = {start}
            stack = [(start, [start])]
            while stack:
                curr, path = stack.pop()
                for nxt in graph.get(curr, []):
                    if nxt == start and len(path) >= 2:
                        cycles.append(path)
                    elif nxt not in visited and len(path) < 5:
                        visited.add(nxt)
                        stack.append((nxt, path + [nxt]))
        
        if cycles:
            conf = min(0.9, len(cycles) * 0.25)
            patterns.append({'type': 'CIRCULAR', 'confidence': conf, 'cycles': cycles[:5]})
        
        # 3. Coordinated: multiple txs in same minute
        buckets = defaultdict(list)
        for tx in txs:
            buckets[tx.get('timestamp', 0) // 60].append(tx)
        
        coordinated = [{'ts': b*60, 'count': len(t)} for b, t in buckets.items() if len(t) >= 3]
        if coordinated:
            conf = min(0.85, len(coordinated) * 0.15)
            patterns.append({'type': 'COORDINATED', 'confidence': conf, 'events': coordinated})
        
        # Risk score
        risk = sum(p['confidence'] * {'SELF_TRADING': 0.4, 'CIRCULAR': 0.35, 'COORDINATED': 0.25}.get(p['type'], 0.1) for p in patterns)
        level = 'CRITICAL' if risk >= 0.7 else 'HIGH' if risk >= 0.5 else 'MEDIUM' if risk >= 0.3 else 'LOW'
        
        return {'symbol': symbol, 'txs_analyzed': len(txs), 'patterns': patterns, 'risk': round(risk, 3), 'level': level}

detector = WashTradingDetector()

# ========== MARKET MANIPULATION DETECTOR ==========
class ManipulationDetector:
    """Detects pump-and-dump, spoofing, front-running"""
    
    def analyze(self, symbol, hours=24):
        conn = get_db()
        c = conn.cursor()
        cutoff = int(time.time()) - (hours * 3600)
        c.execute('SELECT * FROM price_history WHERE symbol=? AND ts>? ORDER BY ts', (symbol, cutoff))
        prices = [dict(r) for r in c.fetchall()]
        conn.close()
        
        if len(prices) < 10:
            return {'alerts': [], 'risk': 0}
        
        alerts = []
        
        # 1. Pump and Dump: rapid price increase followed by crash
        for i in range(len(prices) - 5):
            window = prices[i:i+5]
            start_price = window[0]['price']
            max_price = max(p['price'] for p in window)
            end_price = window[-1]['price']
            
            if start_price > 0:
                pump = (max_price - start_price) / start_price
                dump = (max_price - end_price) / max_price if max_price > 0 else 0
                
                if pump > 0.2 and dump > 0.15:  # 20% pump, 15% dump
                    alerts.append({
                        'type': 'PUMP_AND_DUMP',
                        'confidence': min(0.9, pump + dump),
                        'pump_pct': round(pump * 100, 2),
                        'dump_pct': round(dump * 100, 2),
                        'timestamp': window[0]['ts']
                    })
        
        # 2. Abnormal Volume Spike
        volumes = [p.get('volume', 0) or 0 for p in prices]
        if volumes:
            avg_vol = sum(volumes) / len(volumes)
            for i, p in enumerate(prices):
                vol = p.get('volume', 0) or 0
                if avg_vol > 0 and vol > avg_vol * 5:  # 5x average
                    alerts.append({
                        'type': 'VOLUME_SPIKE',
                        'confidence': min(0.8, vol / avg_vol / 10),
                        'volume': vol,
                        'avg_volume': round(avg_vol, 2),
                        'multiplier': round(vol / avg_vol, 2),
                        'timestamp': p['ts']
                    })
        
        # 3. Price Manipulation (sudden jumps)
        for i in range(1, len(prices)):
            prev, curr = prices[i-1]['price'], prices[i]['price']
            if prev > 0:
                change = abs(curr - prev) / prev
                if change > 0.1:  # 10% sudden change
                    alerts.append({
                        'type': 'PRICE_MANIPULATION',
                        'confidence': min(0.85, change),
                        'change_pct': round(change * 100, 2),
                        'from_price': prev,
                        'to_price': curr,
                        'timestamp': prices[i]['ts']
                    })
        
        risk = min(1.0, sum(a['confidence'] for a in alerts) / 3) if alerts else 0
        return {'symbol': symbol, 'alerts': alerts[:20], 'risk': round(risk, 3)}

manipulation_detector = ManipulationDetector()

# ========== OWNERSHIP TRACKER ==========
class OwnershipTracker:
    """Track beneficial ownership from blockchain transactions"""
    
    def calculate_ownership(self, symbol):
        """Calculate ownership from all transactions"""
        conn = get_db()
        c = conn.cursor()
        c.execute('SELECT * FROM transactions WHERE stock_symbol=? ORDER BY timestamp', (symbol,))
        txs = [dict(r) for r in c.fetchall()]
        conn.close()
        
        # Calculate balances
        balances = defaultdict(float)
        for tx in txs:
            f = tx.get('from_address', '')
            t = tx.get('to_address', '')
            amt = tx.get('amount', 0) or 0
            if f: balances[f] -= amt
            if t: balances[t] += amt
        
        # Filter positive balances
        holders = {a: b for a, b in balances.items() if b > 0 and a}
        total = sum(holders.values())
        
        ownership = []
        for addr, shares in sorted(holders.items(), key=lambda x: -x[1]):
            pct = (shares / total * 100) if total > 0 else 0
            ownership.append({
                'address': addr,
                'shares': round(shares, 6),
                'percentage': round(pct, 4),
                'is_major': pct >= 5.0  # SEC 13D threshold
            })
        
        major = [o for o in ownership if o['is_major']]
        return {
            'symbol': symbol,
            'total_supply': round(total, 6),
            'holder_count': len(ownership),
            'major_holders': len(major),
            'ownership': ownership[:100],
            'concentration': round(sum(o['percentage'] for o in ownership[:10]), 2)  # Top 10 concentration
        }
    
    def detect_changes(self, symbol, days=30):
        """Detect significant ownership changes"""
        conn = get_db()
        c = conn.cursor()
        cutoff = int(time.time()) - (days * 86400)
        c.execute('SELECT * FROM transactions WHERE stock_symbol=? AND timestamp>? ORDER BY timestamp', (symbol, cutoff))
        txs = [dict(r) for r in c.fetchall()]
        conn.close()
        
        # Track daily changes
        daily = defaultdict(lambda: defaultdict(float))
        for tx in txs:
            day = tx.get('timestamp', 0) // 86400
            t = tx.get('to_address', '')
            amt = tx.get('amount', 0) or 0
            if t: daily[day][t] += amt
        
        # Find large acquisitions
        changes = []
        for day, addrs in daily.items():
            for addr, amt in addrs.items():
                if amt > 1000:  # Significant threshold
                    changes.append({
                        'date': datetime.fromtimestamp(day * 86400).strftime('%Y-%m-%d'),
                        'address': addr,
                        'acquired': round(amt, 6),
                        'type': 'ACQUISITION'
                    })
        
        return {'symbol': symbol, 'period_days': days, 'changes': changes[:50]}

ownership_tracker = OwnershipTracker()

# ========== TRACEABILITY ENGINE ==========
class TraceabilityEngine:
    """Full transaction traceability using GXC formula"""
    
    def trace(self, tx_hash):
        """Trace transaction back to origin"""
        # Try blockchain first
        chain = []
        result = node.rpc('tracetransaction', [tx_hash])
        if result:
            return {'tx_hash': tx_hash, 'chain': result, 'source': 'blockchain'}
        
        # Fallback to local DB
        conn = get_db()
        c = conn.cursor()
        current = tx_hash
        visited = set()
        
        while current and current not in visited:
            visited.add(current)
            c.execute('SELECT * FROM transactions WHERE tx_hash=?', (current,))
            tx = c.fetchone()
            if tx:
                tx_dict = dict(tx)
                chain.append(tx_dict)
                # Find previous tx from same address
                c.execute('SELECT tx_hash FROM transactions WHERE to_address=? AND timestamp<? ORDER BY timestamp DESC LIMIT 1',
                    (tx_dict.get('from_address'), tx_dict.get('timestamp', 0)))
                prev = c.fetchone()
                current = prev[0] if prev else None
            else:
                break
        
        conn.close()
        return {
            'tx_hash': tx_hash,
            'trace_depth': len(chain),
            'origin': chain[-1] if chain else None,
            'chain': chain,
            'source': 'local_db'
        }
    
    def trace_address(self, address):
        """Get full transaction history for address"""
        # Try blockchain
        result = node.rpc('getaddresstransactions', [address])
        if result:
            return {'address': address, 'transactions': result, 'source': 'blockchain'}
        
        # Fallback
        conn = get_db()
        c = conn.cursor()
        c.execute('SELECT * FROM transactions WHERE from_address=? OR to_address=? ORDER BY timestamp DESC',
            (address, address))
        txs = [dict(r) for r in c.fetchall()]
        conn.close()
        
        return {'address': address, 'transaction_count': len(txs), 'transactions': txs[:100], 'source': 'local_db'}

tracer = TraceabilityEngine()

# ========== REPORT GENERATOR ==========
class ReportGenerator:
    """Generate SEC/FCA formatted compliance reports"""
    
    def sec_13f(self, params=None):
        """SEC Form 13F - Institutional Holdings"""
        conn = get_db()
        c = conn.cursor()
        c.execute('SELECT DISTINCT stock_symbol FROM transactions')
        symbols = [r[0] for r in c.fetchall()]
        
        holdings = []
        for sym in symbols:
            own = ownership_tracker.calculate_ownership(sym)
            for h in own['ownership'][:20]:
                holdings.append({
                    'issuer': sym,
                    'cusip': f'GXC{sym[:6]}',
                    'shares': h['shares'],
                    'value': h['shares'] * 100,  # Placeholder price
                    'manager': h['address'][:20]
                })
        
        conn.close()
        return {
            'form_type': '13F-HR',
            'period': params.get('period', 'Q4 2025') if params else 'Q4 2025',
            'filing_date': datetime.now().strftime('%Y-%m-%d'),
            'holdings': holdings
        }
    
    def sec_13d(self, symbol):
        """SEC Form 13D - Beneficial Ownership >5%"""
        own = ownership_tracker.calculate_ownership(symbol)
        major = [h for h in own['ownership'] if h['is_major']]
        
        return {
            'form_type': '13D',
            'issuer': symbol,
            'filing_date': datetime.now().strftime('%Y-%m-%d'),
            'beneficial_owners': [{
                'name': h['address'],
                'shares': h['shares'],
                'percentage': h['percentage']
            } for h in major]
        }
    
    def fca_transaction(self, symbol, days=30):
        """FCA Transaction Report"""
        conn = get_db()
        c = conn.cursor()
        cutoff = int(time.time()) - (days * 86400)
        c.execute('SELECT * FROM transactions WHERE stock_symbol=? AND timestamp>? ORDER BY timestamp DESC', (symbol, cutoff))
        txs = [dict(r) for r in c.fetchall()]
        conn.close()
        
        return {
            'report_type': 'FCA_TRANSACTION',
            'instrument': symbol,
            'period_days': days,
            'total_transactions': len(txs),
            'transactions': [{
                'reference': tx['tx_hash'],
                'datetime': datetime.fromtimestamp(tx['timestamp']).isoformat() if tx.get('timestamp') else None,
                'price': tx.get('price'),
                'quantity': tx.get('amount'),
                'buyer': tx.get('to_address'),
                'seller': tx.get('from_address')
            } for tx in txs[:1000]]
        }
    
    def wash_trading_report(self, symbol):
        """Wash Trading Analysis Report"""
        analysis = detector.analyze(symbol, minutes=1440)  # 24 hours
        return {
            'report_type': 'WASH_TRADING_ANALYSIS',
            'symbol': symbol,
            'generated_at': datetime.now().isoformat(),
            'analysis': analysis
        }

reporter = ReportGenerator()

# ========== BLOCKCHAIN SYNC ==========
def sync_from_blockchain():
    """Sync transactions from blockchain to local DB"""
    if not node.connected:
        return {'synced': 0, 'error': 'Not connected'}
    
    # Get recent blocks
    blocks = node.rpc('getrecentblocks', [50])
    if not blocks:
        return {'synced': 0, 'error': 'Could not fetch blocks'}
    
    conn = get_db()
    c = conn.cursor()
    synced = 0
    
    for block in blocks if isinstance(blocks, list) else []:
        txs = block.get('transactions', [])
        for tx in txs:
            tx_hash = tx.get('hash') or tx.get('txid')
            if not tx_hash:
                continue
            
            # Check if exists
            c.execute('SELECT 1 FROM transactions WHERE tx_hash=?', (tx_hash,))
            if c.fetchone():
                continue
            
            # Insert
            c.execute('''INSERT OR IGNORE INTO transactions 
                (tx_hash, block_height, timestamp, from_address, to_address, amount, tx_type)
                VALUES (?, ?, ?, ?, ?, ?, ?)''',
                (tx_hash, block.get('height'), tx.get('timestamp') or block.get('timestamp'),
                 tx.get('from') or tx.get('sender'), tx.get('to') or tx.get('receiver'),
                 tx.get('amount') or tx.get('value'), tx.get('type', 'TRANSFER')))
            synced += 1
    
    conn.commit()
    conn.close()
    return {'synced': synced, 'blocks_checked': len(blocks) if isinstance(blocks, list) else 0}

# ========== AUTH ==========
def verify_session(token):
    if not token or not token.startswith('SESSION_'):
        return None
    conn = get_db()
    c = conn.cursor()
    parts = token.split('_')
    if len(parts) >= 3:
        org_id = parts[1]
        c.execute('SELECT * FROM organizations WHERE org_id=? AND is_active=1', (org_id,))
        org = c.fetchone()
        conn.close()
        return dict(org) if org else None
    conn.close()
    return None

# ========== API ENDPOINTS ===lt)
ify(resujson
    return ss(address)addre.trace_tracer  result = ess):
  addrs(resf trace_add
de)/<address>'essddrtrace/a/regulator/v1te('/api/app.rou)

@onify(result  return js
  ash)ce(tx_h tracer.trault =  reshash):
  tx_tx(
def trace_h>')_hastrace/<txor//regulati/v1oute('/ap

@app.rfy(result)n jsoni    returl, days)
mbochanges(syect_cker.detp_tra ownershiesult =0))
    r', 3s.get('daysst.argint(reque  days = 
  ol):s(symbangenership_chget_owf 
denges')<symbol>/chap/ershigulator/own('/api/v1/repp.route

@afy(result)ni  return jsombol)
  nership(syulate_owlcer.cack_tra = ownershipsult):
    reip(symbolt_ownershef ge>')
dmbolwnership/<syator/o/reguli/v1e('/ap

@app.routsult)n jsonify(retur))
    reurs', 24t('hoa.gedat'), PL 'AA'symbol',t(e(data.gelyzr.anaetectoion_dulat manipult =  reson
   request.jsa =   dat():
 ationipulalyze_manT'])
def an'POShods=[alyze', meton/antimanipulagulator//api/v1/reapp.route('esult)

@y(rn jsonif retur
       
lert_id_id'] = a'alert result[      )
 ose(   conn.cl()
     nn.commit co
       ', None))NDINGlyzed'], 'PEnasult['txs_a         re']]),
    t['patternsesul rr p in'type'] fop[n.dumps([sk'], jso  result['ri
           '],evel result['lymbol'],esult['s()), rme.timeint(tirt_id, ale   (
         )',?,??,?,?,?,?,?,?,ALUES (rts VNTO wash_aleINSERT Ite('     c.execu}"
   time())}_{int(time.'symbol']result[ASH_{ = f"W  alert_id)
      cursor(  c = conn.    get_db()
   = conn:
        sk'] >= 0.5riresult['  if 
  f high risk i Save alert
    #
    tes', 60))ta.get('minu 'AAPL'), da('symbol',ta.getlyze(dadetector.ana   result = on
 equest.jsdata = r    ng():
tradie_wash_ef analyzPOST'])
dods=[', methyze'/analingtradulator/wash-/api/v1/regapp.route('result)

@urn jsonify(()
    retainlockchsync_from_bt =    resulchain():
 _block
def sync])ds=['POST'sync', methon/lockchairegulator/b('/api/v1/route
@app.)
fo
    }': in       'infoght,
 ei.hht': node       'heigurl,
 ode.l': n   'node_ur   ed,
  ectonnde.c nocted':   'conney({
     jsonif return 
   ')aininfoockchblpc('getode.r    info = n_status():
ckchain)
def bloatus'ain/stator/blockchul/api/v1/regpp.route('e']})

@auser['rol': olename'], 'r user['org_org':': token, 'True, 'token: ({'success'n jsonify returlose()
   nn.ct()
    commi  conn.coid']))
  er_user['us.time()),  (int(timer_id=?',HERE usest_login=? Wlas SET E org_userDATe('UPecut c.ex))}"
   ime(_{int(time.t['org_id']}SSION_{usern = f"SE01
    toke'}), 4ialscredentalid : 'Inve, 'error'Falsess': ({'succfyjsoni     return 
   ser:if not u    
e()tchonr = c.fe   useash))
 '], pw_husername, (data['ve=1'tiac? AND u.is__hash=sswordAND u.pame=? sernaWHERE u.u_id id=o.orgorg_u.s o ON tionOIN organizarg_users u J_name FROM o u.*, o.org'SELECTxecute( c.eigest()
   hexdde()).encossword'].6(data['pa25haib.shlhash = has)
    pw_cursor( = conn.()
    c= get_dbconn on
    js= request. data    
():
def loginPOST'])ds=['gin', methoor/logulat/re'/api/v1oute(
@app.r
=======