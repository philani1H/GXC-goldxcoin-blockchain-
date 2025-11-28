# Transaction Detail Page - What Information is Shown

## URL Format
When you access a transaction by ID/hash:
```
https://gxc-blockchain.vercel.app/tx/<transaction_hash>
```

Example:
```
https://gxc-blockchain.vercel.app/tx/0x1a2b3c4d5e6f7g8h9i0j1k2l3m4n5o6p7q8r9s0t1u2v3w4x5y6z
```

---

## Information Displayed on Transaction Detail Page

### 1. **Hero Section** (Top)
- ✅ **Transaction Hash** - Full transaction ID
- ✅ **Status Badge** - Success or Failed (large, prominent)

### 2. **Transaction Flow** (Visual Diagram)
- ✅ **From Address** - Clickable link to sender's address page
- ✅ **To Address** - Clickable link to recipient's address page
- ✅ **Amount** - Transaction value in GXC
- ✅ **Visual Arrow** - Shows direction of transaction

### 3. **Transaction Information** (Main Details)
- ✅ **Transaction Hash** - Full hash (clickable/copyable)
- ✅ **Block Number** - Link to block detail page
- ✅ **Timestamp** - When transaction was included in block
- ✅ **From Address** - Sender address (clickable)
- ✅ **To Address** - Recipient address (clickable)
- ✅ **Value** - Amount transferred in GXC
- ✅ **Transaction Fee** - Fee paid for transaction
- ✅ **Gas Price** - Gas price (if applicable)
- ✅ **Gas Used** - Gas consumed (if applicable)
- ✅ **Nonce** - Transaction nonce
- ✅ **Transaction Type** - Type badge (transfer, contract, etc.)
- ✅ **Input Data** - Raw transaction data (if present)

### 4. **Transaction Type Flags**
- ✅ **Coinbase Badge** - If it's a mining reward transaction
- ✅ **Gold-Backed Badge** - If transaction is backed by gold
- ✅ **Traceability Valid/Invalid Badge** - Traceability validation status

### 5. **Traceability Information** (GXC Unique Feature)
- ✅ **Previous Tx Hash** - Link to previous transaction in chain
- ✅ **Referenced Amount** - Amount from previous transaction
- ✅ **Traceability Chain** - Full chain of linked transactions (up to 20 levels)
  - Shows each link in the chain
  - Clickable links to navigate through chain
  - Validation status for each link
  - Block height for each transaction

### 6. **Transaction Inputs** (UTXO Model)
- ✅ **Input Index** - Input position
- ✅ **Previous Transaction Hash** - Source transaction
- ✅ **Previous Output Index** - Which output was spent
- ✅ **Amount** - Input amount
- ✅ **Signature** - Transaction signature
- ✅ **Public Key** - Sender's public key

### 7. **Transaction Outputs** (UTXO Model)
- ✅ **Output Index** - Output position
- ✅ **Address** - Recipient address (clickable)
- ✅ **Amount** - Output amount
- ✅ **Script** - Script data (if present)
- ✅ **Spent Status** - Whether output has been spent

### 8. **Additional Information** (If Available)
- ✅ **Memo** - Transaction memo/message
- ✅ **Lock Time** - Time lock information
- ✅ **PoP Reference** - Proof of Price reference
- ✅ **Contract Address** - If it's a contract transaction

---

## Key Features

### Clickable Links
- ✅ **Transaction Hash** → Copy/share
- ✅ **Block Number** → View block details
- ✅ **From/To Addresses** → View address details
- ✅ **Previous Tx Hash** → Navigate traceability chain
- ✅ **All addresses** → Click to view wallet/address page

### Visual Elements
- ✅ **Status Badge** - Large, color-coded (green=success, red=failed)
- ✅ **Flow Diagram** - Visual representation of transaction flow
- ✅ **Type Badges** - Color-coded badges for transaction types
- ✅ **Traceability Chain** - Visual chain with arrows showing links

### Data Formatting
- ✅ **Amounts** - Formatted to 8 decimal places
- ✅ **Timestamps** - Human-readable format
- ✅ **Hashes** - Monospace font, truncated with "..." for display
- ✅ **Numbers** - Comma-separated for readability

---

## Example: What You See

When you visit `/tx/0x1a2b3c4d...`:

```
┌─────────────────────────────────────────┐
│ Transaction Details                     │
│ 0x1a2b3c4d5e6f7g8h9i0j1k2l3m4n5o6p7q8r9 │
│ [✅ Success]                             │
└─────────────────────────────────────────┘

┌─────────────────────────────────────────┐
│ Transaction Flow                        │
│                                         │
│  From: GXC1a2b3c4...  →  To: GXC5e6f7g8... │
│        100.0 GXC                        │
└─────────────────────────────────────────┘

┌─────────────────────────────────────────┐
│ Transaction Information                 │
│ • Hash: 0x1a2b3c4d...                   │
│ • Block: #12345 (clickable)             │
│ • Timestamp: 2024-01-15 10:30:45        │
│ • From: GXC1a2b3c4... (clickable)       │
│ • To: GXC5e6f7g8... (clickable)         │
│ • Value: 100.00000000 GXC               │
│ • Fee: 0.00100000 GXC                   │
│ • Status: Success                       │
│ • Type: Transfer                        │
│ • [Coinbase] [Gold-Backed] [Valid]      │
└─────────────────────────────────────────┘

┌─────────────────────────────────────────┐
│ Traceability Chain                      │
│                                         │
│ Level 1: 0x1a2b3c4d... → 0x9z8y7x6w... │
│   Amount: 100.0 GXC | Valid ✅          │
│                                         │
│ Level 2: 0x9z8y7x6w... → 0x5v4u3t2s... │
│   Amount: 50.0 GXC | Valid ✅           │
│                                         │
│ ... (up to 20 levels)                   │
└─────────────────────────────────────────┘

┌─────────────────────────────────────────┐
│ Transaction Inputs                      │
│ • Input 0: 0xprev1... Output 0         │
│   Amount: 100.0 GXC                     │
└─────────────────────────────────────────┘

┌─────────────────────────────────────────┐
│ Transaction Outputs                      │
│ • Output 0: GXC5e6f7g8...               │
│   Amount: 100.0 GXC                     │
│   Status: Spent/Unspent                 │
└─────────────────────────────────────────┘
```

---

## What Makes This Special

### 1. **Complete Transaction Data**
- Shows ALL transaction information from blockchain
- Nothing is hidden or simplified
- Full transparency

### 2. **Traceability Chain** (GXC Unique)
- Shows complete history of coins
- Links back to origin transaction
- Validates traceability
- Up to 20 levels deep

### 3. **UTXO Details**
- Shows inputs (where coins came from)
- Shows outputs (where coins went)
- Tracks spent/unspent status

### 4. **Interactive Navigation**
- Click addresses → View wallet details
- Click block → View block details
- Click previous tx → Navigate traceability chain
- All links are functional

### 5. **Visual Clarity**
- Color-coded status badges
- Flow diagrams
- Organized sections
- Easy to read formatting

---

## Summary

When you access a transaction by ID (`/tx/<hash>`), you see:

✅ **Complete transaction details** - All blockchain data
✅ **Visual flow diagram** - From → To with amount
✅ **Traceability chain** - Full history of linked transactions
✅ **UTXO details** - Inputs and outputs
✅ **Clickable links** - Navigate to addresses, blocks, previous transactions
✅ **Status information** - Success/failed, validation status
✅ **Type information** - Coinbase, gold-backed, traceability status
✅ **All metadata** - Timestamp, nonce, gas, fees, etc.

**This is the most comprehensive transaction detail view possible** - showing everything the blockchain knows about that transaction!

