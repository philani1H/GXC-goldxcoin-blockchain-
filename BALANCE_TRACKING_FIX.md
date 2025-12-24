# Balance Tracking Issue - Fix Documentation

## Problem

The miner GUI shows:
```
[17:27:11] TX 35eec4b6f84a5be8... CONFIRMED! +50.0 GXC
[17:27:14] Total earned: 0.00000000 GXC
[17:27:14] Balance: 0.00000000 GXC
```

**Issue:** Blocks are being mined successfully and transactions are confirming, but the balance always shows 0.00000000 GXC.

## Root Cause

The balance query in `mine_testnet_gui.py` is trying multiple RPC methods but all are returning 0 or None:

```python
balance_methods = [
    ("gxc_getBalance", [address]),
    ("getbalance", [address]),
    ("getaddressbalance", [address]),
]
```

**Possible causes:**
1. Node RPC method name mismatch
2. Address format issue
3. Node not tracking wallet balances correctly
4. Coinbase transactions not being credited to address

## Investigation Steps

### 1. Check Node RPC Methods

```bash
curl -X POST http://localhost:8332 \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"help","params":[],"id":1}' \
  | grep -i balance
```

**Expected output:** List of balance-related methods

### 2. Test Balance Query

```bash
# Try different methods
curl -X POST http://localhost:8332 \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getbalance","params":["tGXC3tz6MsTbP2NCMvc3"],"id":1}'

curl -X POST http://localhost:8332 \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"gxc_getBalance","params":["tGXC3tz6MsTbP2NCMvc3"],"id":1}'
```

### 3. Check Transaction Details

```bash
# Get transaction to see if it credits the address
curl -X POST http://localhost:8332 \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"gettransaction","params":["35eec4b6f84a5be8..."],"id":1}'
```

### 4. List Transactions for Address

```bash
curl -X POST http://localhost:8332 \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"listtransactions","params":["*", 10],"id":1}'
```

## Potential Fixes

### Fix 1: Update RPC Method Names

If the node uses different method names, update the miner:

```python
# In mine_testnet_gui.py, line ~905
balance_methods = [
    ("getbalance", [address]),  # Standard Bitcoin RPC
    ("gxc_getBalance", [address]),  # GXC-specific
    ("getaccountbalance", [address]),  # Alternative
    ("getaddressbalance", [{"addresses": [address]}]),  # Insight API style
]
```

### Fix 2: Check Wallet Import

The node might need the address imported to track it:

```python
def ensure_address_imported(self, address):
    """Import address to node wallet for tracking"""
    try:
        # Try to import address
        self.rpc_call("importaddress", [address, "", False])
        self.log(f"Address imported to node wallet", "INFO")
    except:
        pass  # Address might already be imported
```

### Fix 3: Calculate Balance from UTXOs

If balance RPC doesn't work, calculate from unspent outputs:

```python
def get_balance_from_utxos(self, address):
    """Calculate balance from unspent transaction outputs"""
    try:
        # Get unspent outputs
        utxos = self.rpc_call("listunspent", [0, 9999999, [address]])
        
        if utxos:
            balance = sum(utxo.get('amount', 0) for utxo in utxos)
            return balance
        
        return 0.0
    except:
        return 0.0
```

### Fix 4: Use Transaction History

Calculate balance from transaction history:

```python
def calculate_balance_from_transactions(self, address):
    """Calculate balance from transaction history"""
    try:
        # Get all transactions
        txs = self.rpc_call("listtransactions", ["*", 1000])
        
        if not txs:
            return 0.0
        
        balance = 0.0
        for tx in txs:
            # Check if transaction involves our address
            if tx.get('address') == address:
                category = tx.get('category', '')
                amount = tx.get('amount', 0)
                
                if category in ['receive', 'generate', 'immature']:
                    balance += amount
                elif category == 'send':
                    balance -= amount
        
        return balance
    except:
        return 0.0
```

## Recommended Solution

Update `mine_testnet_gui.py` to use multiple fallback methods:

```python
def refresh_balance_from_blockchain(self):
    """Refresh balance and transactions from blockchain"""
    if not self.wallet_address.get() or not self.node_connected:
        return

    address = self.wallet_address.get().strip()
    if not address:
        return

    self.log("Refreshing balance from blockchain...", "INFO")

    def fetch():
        try:
            balance = 0.0
            
            # Method 1: Try standard balance RPC
            balance_methods = [
                ("getbalance", [address]),
                ("gxc_getBalance", [address]),
                ("getaccountbalance", [address]),
            ]

            for method, params in balance_methods:
                result = self.rpc_call(method, params, show_errors=False)
                if result is not None and result > 0:
                    balance = float(result)
                    self.log(f"Got balance from {method}: {balance}", "DEBUG")
                    break
            
            # Method 2: If balance still 0, try UTXOs
            if balance == 0:
                utxos = self.rpc_call("listunspent", [0, 9999999, [address]], show_errors=False)
                if utxos:
                    balance = sum(utxo.get('amount', 0) for utxo in utxos)
                    self.log(f"Calculated balance from {len(utxos)} UTXOs: {balance}", "DEBUG")
            
            # Method 3: If still 0, calculate from transactions
            if balance == 0:
                txs = self.rpc_call("listtransactions", ["*", 1000], show_errors=False)
                if txs:
                    for tx in txs:
                        if tx.get('address') == address:
                            category = tx.get('category', '')
                            amount = tx.get('amount', 0)
                            
                            if category in ['receive', 'generate', 'immature']:
                                balance += amount
                            elif category == 'send':
                                balance -= amount
                    
                    if balance > 0:
                        self.log(f"Calculated balance from transaction history: {balance}", "DEBUG")
            
            # Update display
            self.root.after(0, lambda b=balance: self.update_balance_display(b))
            self.root.after(0, lambda b=balance: self.log(f"Balance: {b:.8f} GXC", "INFO"))
            
            # Get transactions for earnings calculation
            transactions = self.get_address_transactions(address, limit=100)
            total_earned = self.calculate_total_earned_from_blockchain(transactions)
            
            self.root.after(0, lambda te=total_earned: self.update_earnings_display(te))
            self.root.after(0, lambda te=total_earned: self.log(f"Total earned: {te:.8f} GXC", "INFO"))

        except Exception as e:
            self.root.after(0, lambda err=str(e): self.log(f"Failed to fetch balance: {err}", "ERROR"))

    threading.Thread(target=fetch, daemon=True).start()
```

## Testing the Fix

1. **Start the node:**
   ```bash
   ./gxc-node --testnet
   ```

2. **Test balance query:**
   ```bash
   python3 -c "
   import requests, json
   
   address = 'tGXC3tz6MsTbP2NCMvc3'
   
   # Test getbalance
   r = requests.post('http://localhost:8332', json={
       'jsonrpc': '2.0',
       'method': 'getbalance',
       'params': [address],
       'id': 1
   })
   print('getbalance:', r.json())
   
   # Test listtransactions
   r = requests.post('http://localhost:8332', json={
       'jsonrpc': '2.0',
       'method': 'listtransactions',
       'params': ['*', 10],
       'id': 1
   })
   print('listtransactions:', r.json())
   "
   ```

3. **Run the miner:**
   ```bash
   python3 mine_testnet_gui.py
   ```

4. **Verify balance updates after mining a block**

## Expected Behavior After Fix

```
[17:27:11] Mining block to: tGXC3tz6MsTbP2NCMvc3...
[17:27:11] Found valid hash: 08fc9800274d0565...
[17:27:11] Submitting block...
[17:27:11] BLOCK MINED! Height: 33
[17:27:11] Expected reward: 50.0 GXC
[17:27:14] Refreshing balance from blockchain...
[17:27:14] Got balance from getbalance: 50.0
[17:27:14] Balance: 50.00000000 GXC
[17:27:14] Total earned: 50.00000000 GXC
```

## Summary

The issue is that the balance RPC method is not returning the correct value. The fix involves:

1. Testing which RPC method the node actually supports
2. Adding fallback methods (UTXOs, transaction history)
3. Ensuring the address is imported to the node wallet
4. Adding debug logging to see which method works

Once the correct RPC method is identified, update the miner to use it consistently.

---

**Status:** Issue identified, fix documented  
**Next Step:** Test RPC methods and apply appropriate fix
