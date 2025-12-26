# Third-Party Wallet Transaction Fix

## Problem

Third-party wallets were unable to send transactions because the blockchain's traceability validation was too strict. The validation required:

1. `prevTxHash` field to be explicitly set
2. `referencedAmount` field to be explicitly set

When third-party wallets created transactions without these fields, the validation would fail with:

```
Transaction traceability validation failed
```

## Root Cause

The traceability validation in `Transaction::isTraceabilityValid()` was checking:
- `hasValidPrevReference()` - required `prevTxHash` to be non-empty
- `verifyTraceabilityFormula()` - required exact match between `prevTxHash` and `inputs[0].txHash`

Third-party wallets typically only provide:
- Transaction inputs (with txHash and amount)
- Transaction outputs
- Signatures

They don't necessarily track or provide the `prevTxHash` and `referencedAmount` fields, which are internal blockchain traceability metadata.

## Solution

The fix makes the blockchain **automatically derive** these fields from the transaction inputs when they're not provided:

### 1. Transaction Validation (`src/Transaction.cpp`)

**`verifyTraceabilityFormula()`:**
- If `prevTxHash` is empty or "0", automatically use `inputs[0].txHash`
- If `referencedAmount` is 0.0, automatically use `inputs[0].amount`

**`hasValidPrevReference()`:**
- Allow transactions with empty `prevTxHash` if they have valid inputs
- The blockchain will derive the reference from the first input

### 2. API Layer (`src/RPCAPI.cpp`)

**`sendRawTransaction()`:**
- Auto-derive `prevTxHash` from `inputs[0].txHash` if not provided
- Auto-derive `referencedAmount` from `inputs[0].amount` if not provided
- Log when auto-derivation occurs for debugging

## Benefits

1. **Backward Compatible**: Existing wallets that set these fields continue to work
2. **Third-Party Friendly**: External wallets can now send transactions without internal metadata
3. **Maintains Traceability**: The blockchain still enforces traceability using the input references
4. **Automatic**: No changes required to third-party wallet implementations

## Testing

Run the test script to verify the fix:

```bash
python3 test_third_party_wallet_fix.py
```

This test simulates a third-party wallet sending a transaction without `prevTxHash` and `referencedAmount` fields.

## For Third-Party Wallet Developers

When creating transactions, you only need to provide:

```json
{
  "inputs": [{
    "txHash": "previous_transaction_hash",
    "outputIndex": 0,
    "amount": 50.0,
    "signature": "your_signature",
    "publicKey": "your_public_key"
  }],
  "outputs": [{
    "address": "recipient_address",
    "amount": 49.9,
    "script": ""
  }],
  "fee": 0.0001,
  "senderAddress": "your_address",
  "receiverAddress": "recipient_address",
  "type": "NORMAL"
}
```

The blockchain will automatically:
- Set `prevTxHash = inputs[0].txHash`
- Set `referencedAmount = inputs[0].amount`

## Technical Details

### Traceability Formula

The blockchain enforces this formula for transaction traceability:

```
Ti.Inputs[0].txHash == Ti.PrevTxHash
Ti.Inputs[0].amount == Ti.ReferencedAmount
```

With this fix:
- If `Ti.PrevTxHash` is not set, use `Ti.Inputs[0].txHash`
- If `Ti.ReferencedAmount` is not set, use `Ti.Inputs[0].amount`

This ensures the formula always holds while being flexible for external wallets.

### Files Changed

1. `src/Transaction.cpp`:
   - `verifyTraceabilityFormula()` - Auto-derive fields
   - `hasValidPrevReference()` - Allow empty prevTxHash with valid inputs

2. `src/RPCAPI.cpp`:
   - `sendRawTransaction()` - Auto-derive fields at API layer

3. `test_third_party_wallet_fix.py`:
   - Test script to verify the fix

## Impact

This fix enables:
- Mobile wallet apps to send transactions
- Web wallet interfaces to work correctly
- Hardware wallets to integrate easily
- Exchange integrations to function properly

All without requiring them to understand or track internal blockchain traceability metadata.
