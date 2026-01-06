# API Field Names Migration Guide

## What Changed

We removed duplicate field names from JSON-RPC responses to fix compatibility issues with the Rust peer node and improve API consistency.

## Field Name Changes

### Block Fields

| Old Field Names (REMOVED) | New Field Name | Example |
|---------------------------|----------------|---------|
| `previous_hash`, `prev_hash`, `parent_hash` | `previousblockhash` | `"previousblockhash": "abc123..."` |
| `merkle_root` | `merkleroot` | `"merkleroot": "def456..."` |
| `timestamp` | `time` | `"time": 1767733570` |
| `transactions` | `tx` | `"tx": [...]` |
| `block_hash` | `hash` | `"hash": "789ghi..."` |
| `block_number`, `number` | `height` | `"height": 42` |
| `miner_address` | `miner` | `"miner": "tGXC..."` |
| `block_reward` | `reward` | `"reward": 50.0` |
| `transaction_count`, `tx_count` | `nTx` | `"nTx": 5` |
| `consensus_type`, `block_type` | `consensustype` | `"consensustype": "pow_sha256"` |
| `total_difficulty`, `totalDifficulty` | `chainwork` | `"chainwork": 42.0` |

### Transaction Fields

| Old Field Names (REMOVED) | New Field Name | Example |
|---------------------------|----------------|---------|
| `tx_hash`, `transactionHash` | `txid` or `hash` | `"txid": "abc..."` |
| `block_number`, `blockNumber`, `blockHeight` | `blockheight` | `"blockheight": 42` |
| `block_hash` | `blockhash` | `"blockhash": "def..."` |
| `is_coinbase`, `isCoinbase` | `coinbase` | `"coinbase": true` |
| `from_address`, `fromAddress` | `from` | `"from": "tGXC..."` |
| `to_address`, `toAddress` | `to` | `"to": "tGXC..."` |
| `value` | `amount` | `"amount": 10.5` |
| `timestamp` | `time` | `"time": 1767733570` |

## Migration Examples

### Before (Old Code)

```javascript
// ❌ This will NOT work anymore
fetch('/api/getblock', {
  method: 'POST',
  body: JSON.stringify({
    jsonrpc: '2.0',
    method: 'getblock',
    params: [1]
  })
})
.then(r => r.json())
.then(data => {
  const block = data.result;
  console.log(block.previous_hash);  // ❌ undefined
  console.log(block.merkle_root);    // ❌ undefined
  console.log(block.timestamp);      // ❌ undefined
  console.log(block.transactions);   // ❌ undefined
});
```

### After (New Code)

```javascript
// ✅ Use these field names
fetch('/api/getblock', {
  method: 'POST',
  body: JSON.stringify({
    jsonrpc: '2.0',
    method: 'getblock',
    params: [1]
  })
})
.then(r => r.json())
.then(data => {
  const block = data.result;
  console.log(block.previousblockhash);  // ✅ works
  console.log(block.merkleroot);         // ✅ works
  console.log(block.time);               // ✅ works
  console.log(block.tx);                 // ✅ works
});
```

### Python Example

```python
# ❌ Old code
block = get_block(1)
prev_hash = block['previous_hash']  # ❌ KeyError
merkle = block['merkle_root']       # ❌ KeyError

# ✅ New code
block = get_block(1)
prev_hash = block['previousblockhash']  # ✅ works
merkle = block['merkleroot']            # ✅ works
```

## Compatibility Helper

If you have existing code, you can add a compatibility layer:

```javascript
function normalizeBlock(block) {
  // Add old field names for backward compatibility
  block.previous_hash = block.previousblockhash;
  block.merkle_root = block.merkleroot;
  block.timestamp = block.time;
  block.transactions = block.tx;
  return block;
}

// Use it
const block = normalizeBlock(data.result);
console.log(block.previous_hash);  // Now works
```

```python
def normalize_block(block):
    """Add old field names for backward compatibility"""
    block['previous_hash'] = block.get('previousblockhash', '')
    block['merkle_root'] = block.get('merkleroot', '')
    block['timestamp'] = block.get('time', 0)
    block['transactions'] = block.get('tx', [])
    return block

# Use it
block = normalize_block(get_block(1))
print(block['previous_hash'])  # Now works
```

## Why This Change?

1. **Eliminated Duplicates**: The API was returning the same data under multiple field names, wasting bandwidth
2. **Fixed Rust Peer Node**: Duplicate fields caused JSON parsing errors
3. **Bitcoin Compatibility**: New names follow Bitcoin Core RPC conventions
4. **Consistency**: One field name per piece of data

## What Still Works

✅ **All endpoints work** - No endpoints were removed  
✅ **Mining works** - Python miner, C++ miners all functional  
✅ **Mining pool works** - Already uses correct field names  
✅ **Balances work** - getbalance, sendtoaddress, etc.  
✅ **Block sync works** - Rust peer node can now sync  

## Testing Your Code

Run this test to check if your code needs updates:

```bash
# Get a block and check field names
curl -X POST https://your-node.com/api/getblock \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getblock","params":[1]}' \
  | jq '.result | keys'
```

You should see:
- `previousblockhash` (not previous_hash)
- `merkleroot` (not merkle_root)
- `time` (not timestamp)
- `tx` (not transactions)

## Need Help?

If your application breaks after this update:

1. Check the field name mapping table above
2. Update your code to use new field names
3. Or add a compatibility helper function
4. Test with the latest node version

## Rollback (Not Recommended)

If you absolutely need the old field names temporarily, you can:

1. Checkout commit before this change: `git checkout <commit_before_fix>`
2. Rebuild and deploy
3. But note: Rust peer node won't work with old field names

## Summary

✅ **Breaking Change**: Field names changed  
✅ **Easy Fix**: Update field names in your code  
✅ **Better API**: More consistent, less duplication  
✅ **Rust Compatible**: Peer node now works  

Update your code to use the new field names listed in the table above.
