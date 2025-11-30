# Transaction Traceability Implementation

## Overview

Yes, **transactions are fully trackable** in the GXC blockchain. The system implements comprehensive transaction traceability that allows you to:

1. **Trace transaction lineage** - Follow the chain of transactions backwards from any transaction
2. **Verify transaction integrity** - Validate that transaction chains are unbroken
3. **Get transaction chains by address** - Find all transactions related to a specific address
4. **Validate input references** - Ensure all transaction inputs reference valid previous transactions

## Traceability Formula

The GXC blockchain uses a traceability formula to ensure transaction integrity:

```
Ti.Inputs[0].txHash == Ti.PrevTxHash
Ti.Inputs[0].amount == Ti.ReferencedAmount
```

This means:
- Each transaction's first input must reference the transaction's `prevTxHash`
- The amount in the first input must match the transaction's `referencedAmount`

## Implementation Details

### Core Traceability Methods

All traceability methods are now fully implemented in `src/Blockchain.cpp`:

1. **`traceTransactionLineage(txHash)`** - Returns a vector of transaction hashes tracing backwards from the given transaction
2. **`verifyTransactionLineage(txHash)`** - Verifies that the entire transaction chain is valid
3. **`getTransactionByHash(hash)`** - Retrieves a transaction by its hash
4. **`isLineageValid(txHash)`** - Checks if a transaction's lineage is valid
5. **`getTransactionChain(address, depth)`** - Gets all transactions for an address, tracing backwards up to the specified depth
6. **`verifyInputReferences(tx)`** - Validates that all transaction inputs reference valid previous transactions

### Traceability Index

The blockchain maintains a `traceabilityIndex` that stores:
- Transaction hash
- Previous transaction hash (`prevTxHash`)
- Referenced amount
- From/to addresses
- Timestamp

This index is:
- **Built on initialization** - All existing transactions are indexed
- **Updated on new blocks** - New transactions are automatically added to the index

### RPC API Endpoints

Three new RPC endpoints are available for traceability:

1. **`tracetransaction <txHash>`**
   - Returns an array of transaction hashes tracing backwards from the given transaction
   - Example: `["tx1", "tx2", "tx3"]` where tx1 references tx2, which references tx3

2. **`verifytransactionlineage <txHash>`**
   - Returns `true` if the transaction lineage is valid, `false` otherwise
   - Validates the entire chain of transactions

3. **`gettransactionchain <address> [depth]`**
   - Returns an array of transaction hashes for the given address
   - Optional `depth` parameter (default: 100) limits how far back to trace
   - Finds the most recent transaction for the address and traces backwards

## Transaction Creation

When transactions are created (via `createTransactionFromJson` in `src/RPCAPI.cpp`):

1. **`prevTxHash` is automatically set** from:
   - Explicit `prevTxHash` field in JSON
   - First input's `txHash` if `prevTxHash` is not provided

2. **`referencedAmount` is automatically set** from:
   - Explicit `referencedAmount` field in JSON
   - First input's `amount` if `referencedAmount` is not provided

3. **Sender/receiver addresses** are parsed from:
   - `from` / `from_address` / `fromAddress` fields
   - `to` / `to_address` / `toAddress` fields

## Validation

Every non-coinbase transaction is validated for traceability:

1. **On transaction creation** - `Transaction::isTraceabilityValid()` checks:
   - Hash match: `inputs[0].txHash == prevTxHash`
   - Amount match: `inputs[0].amount == referencedAmount`
   - Input reference validity
   - Amount consistency

2. **On block validation** - `Blockchain::validateTransactionTraceability()` ensures:
   - The traceability formula is satisfied
   - All input references are valid

3. **On block addition** - New transactions are automatically added to the traceability index

## Usage Examples

### Trace a Transaction Lineage

```bash
# Via RPC call
curl -X POST http://localhost:8080/rpc \
  -H "Content-Type: application/json" \
  -d '{
    "method": "tracetransaction",
    "params": ["abc123..."]
  }'
```

### Verify Transaction Lineage

```bash
curl -X POST http://localhost:8080/rpc \
  -H "Content-Type: application/json" \
  -d '{
    "method": "verifytransactionlineage",
    "params": ["abc123..."]
  }'
```

### Get Transaction Chain for Address

```bash
curl -X POST http://localhost:8080/rpc \
  -H "Content-Type: application/json" \
  -d '{
    "method": "gettransactionchain",
    "params": ["tGXC9fab7317231b966a...", 50]
  }'
```

## Benefits

1. **Audit Trail** - Complete history of how funds moved through the blockchain
2. **Fraud Prevention** - Invalid transaction chains are rejected
3. **Transparency** - Anyone can verify transaction integrity
4. **Compliance** - Meets regulatory requirements for transaction tracking
5. **Debugging** - Easy to trace issues in transaction flows

## Status

✅ **Fully Implemented and Operational**

All traceability features are:
- Implemented in the C++ blockchain node
- Exposed via RPC API
- Automatically validated on every transaction
- Indexed for fast lookups
- Thread-safe with proper mutex locking
