# What Happens When You Send Coins to Someone

## Complete Transaction Flow Explained

### Step 1: Create Transaction (Wallet)
**Location**: `src/Wallet.cpp` - `createTransaction()`

When you want to send coins:

1. **Find Your UTXOs**:
   - Wallet searches the UTXO set for unspent outputs belonging to your address
   - Collects enough UTXOs to cover the amount you want to send

2. **Create Transaction Inputs**:
   - Each UTXO becomes a transaction input
   - Input references: `{previous_tx_hash, output_index}`

3. **Create Transaction Outputs**:
   - **Recipient Output**: Amount sent to recipient address
   - **Change Output** (if needed): Remaining amount back to your address
   - Example: You have 100 GXC, send 30 GXC
     - Input: 100 GXC UTXO
     - Output 1: 30 GXC to recipient
     - Output 2: 70 GXC back to you (change)

4. **Sign Transaction**:
   - Transaction inputs are signed with your private key
   - Signature proves you own the UTXOs being spent
   - `tx.signInputs(privateKey)` creates cryptographic signatures

5. **Calculate Transaction Hash**:
   - Transaction hash is calculated from all transaction data
   - This becomes the unique transaction ID

### Step 2: Submit Transaction (RPC API)
**Location**: `src/RPCAPI.cpp` - `sendToAddress()` or `sendRawTransaction()`

When transaction is submitted via RPC:

1. **Transaction Received**:
   - RPC endpoint receives transaction data
   - Transaction can be sent as:
     - `sendtoaddress` RPC call (address + amount)
     - `sendrawtransaction` RPC call (raw transaction data)

2. **Add to Transaction Pool**:
   - Transaction is added to `pendingTransactions` pool
   - `Blockchain::addTransaction(tx)` validates and adds it

### Step 3: Transaction Validation
**Location**: `src/Blockchain.cpp` - `validateTransaction()`

Before adding to pool, transaction is validated:

1. **Signature Validation**:
   - Verifies signatures on all inputs
   - Confirms you own the UTXOs being spent

2. **UTXO Validation**:
   - Checks that referenced UTXOs exist and haven't been spent
   - Verifies you have sufficient balance

3. **Amount Validation**:
   - Ensures outputs don't exceed inputs (no money creation)
   - Validates amounts are positive

4. **Address Validation**:
   - Verifies recipient address is valid format
   - Checks addresses match network (testnet/mainnet)

5. **Traceability Validation**:
   - Validates transaction traceability rules
   - Ensures `prevTxHash` matches input references

### Step 4: Wait in Transaction Pool
**Location**: `src/Blockchain.cpp` - `pendingTransactions` vector

Transaction sits in memory pool waiting to be included in a block:

- **Status**: Pending/Unconfirmed
- **Visibility**: Can be queried via `listtransactions` RPC
- **Expiration**: Transactions expire after 1 hour if not confirmed
- **Broadcasting**: Transaction is broadcast to network peers

### Step 5: Miner Includes Transaction in Block
**Location**: `src/RPCAPI.cpp` - `getBlockTemplate()`

When a miner requests a block template:

1. **Get Pending Transactions**:
   - Miner calls `getblocktemplate` RPC
   - Node returns pending transactions from pool
   - Miner selects transactions to include (up to block size limit)

2. **Miner Creates Block**:
   - Miner includes your transaction in the block
   - Miner adds coinbase transaction (mining reward)
   - Miner solves proof-of-work puzzle

3. **Miner Submits Block**:
   - Miner submits completed block via `gxc_submitBlock` RPC
   - Block includes your transaction

### Step 6: Block Validation & Addition
**Location**: `src/Blockchain.cpp` - `addBlock()`

When block is received:

1. **Block Validation**:
   - Validates proof-of-work
   - Validates block structure
   - Validates all transactions in block

2. **Transaction Validation**:
   - Re-validates all transactions (including yours)
   - Ensures UTXOs haven't been double-spent
   - Verifies signatures again

3. **Block Added to Chain**:
   - Block is added to blockchain
   - Block height increases
   - Block hash is recorded

### Step 7: UTXO Set Update
**Location**: `src/Blockchain.cpp` - `updateUtxoSet()`

When block is added, UTXO set is updated:

1. **Remove Spent UTXOs** (Inputs):
   - Your UTXOs that were spent are removed from UTXO set
   - Example: Your 100 GXC UTXO is removed (spent)

2. **Add New UTXOs** (Outputs):
   - Recipient's new UTXO is added: 30 GXC to recipient address
   - Your change UTXO is added: 70 GXC back to your address
   - Coinbase UTXO is added: Mining reward to miner

3. **Database Update**:
   - UTXO set is saved to database
   - `Database::updateUtxoSet()` updates `utxo` table
   - Spent UTXOs deleted, new UTXOs inserted

### Step 8: Balance Updates
**Location**: `src/Blockchain.cpp` - `getBalance()`

Balances are calculated from UTXO set:

1. **Your Balance**:
   - Before: 100 GXC (1 UTXO)
   - After: 70 GXC (1 UTXO - the change)
   - Your 30 GXC UTXO was spent, 70 GXC change UTXO added

2. **Recipient Balance**:
   - Before: 0 GXC
   - After: 30 GXC (1 new UTXO)
   - Recipient now has 30 GXC UTXO

3. **Balance Queries**:
   - `getbalance` RPC reads from UTXO set
   - Sums all UTXOs for an address
   - Returns current balance

### Step 9: Database Persistence
**Location**: `src/Database.cpp` - `saveBlock()`, `saveTransaction()`

Everything is saved to database:

1. **Block Saved**:
   - Block data → `blocks` table
   - Height, hash, miner, difficulty, etc.

2. **Transactions Saved**:
   - Your transaction → `transactions` table
   - Transaction hash, sender, recipient, amount, etc.
   - Transaction inputs → `transaction_inputs` table
   - Transaction outputs → `transaction_outputs` table

3. **UTXO Set Saved**:
   - UTXO updates → `utxo` table
   - Spent UTXOs removed
   - New UTXOs added with block height

4. **Traceability Saved**:
   - Traceability records → `traceability_index` table
   - Links transactions to previous transactions

### Step 10: Transaction Confirmation
**Location**: `src/RPCAPI.cpp` - `getTransaction()`

Transaction becomes confirmed:

1. **Confirmation Count**:
   - Confirmation = Current block height - Transaction block height + 1
   - Example: Transaction in block 100, current height 105 = 6 confirmations

2. **Finality**:
   - After 6+ confirmations, transaction is considered final
   - Very unlikely to be reversed (would require chain reorganization)

3. **Transaction Status**:
   - Status changes from "pending" to "confirmed"
   - Can be queried via `gettransaction` RPC
   - Shows confirmations, block height, block hash

## Visual Flow Diagram

```
[Your Wallet]
    │
    ├─> Create Transaction (find UTXOs, create inputs/outputs, sign)
    │
    ├─> Submit via RPC (sendtoaddress or sendrawtransaction)
    │
    ├─> Transaction Pool (pendingTransactions)
    │   └─> Wait for miner to include
    │
    ├─> Miner Gets Block Template (includes your transaction)
    │
    ├─> Miner Creates Block (includes your transaction + coinbase)
    │
    ├─> Miner Submits Block (gxc_submitBlock)
    │
    ├─> Block Validation (proof-of-work, transactions, signatures)
    │
    ├─> Block Added to Chain
    │
    ├─> UTXO Set Updated
    │   ├─> Remove: Your spent UTXO (100 GXC)
    │   ├─> Add: Recipient UTXO (30 GXC)
    │   └─> Add: Your change UTXO (70 GXC)
    │
    ├─> Database Saved
    │   ├─> Block → blocks table
    │   ├─> Transaction → transactions table
    │   └─> UTXOs → utxo table
    │
    └─> Balances Updated
        ├─> Your balance: 100 → 70 GXC
        └─> Recipient balance: 0 → 30 GXC
```

## Key Points

### ✅ What Gets Updated:
1. **UTXO Set**: Spent UTXOs removed, new UTXOs added
2. **Balances**: Calculated from UTXO set (not stored directly)
3. **Database**: All blocks, transactions, UTXOs saved
4. **Transaction Pool**: Confirmed transactions removed from pool

### ✅ Security Features:
1. **Signatures**: Prove ownership of UTXOs
2. **Validation**: Multiple validation checks prevent double-spending
3. **Consensus**: Network agrees on which transactions are valid
4. **Immutability**: Once confirmed, transaction is permanent

### ✅ Transaction States:
1. **Created**: Transaction created and signed
2. **Pending**: In transaction pool, waiting for confirmation
3. **Confirmed**: Included in a block (1 confirmation)
4. **Final**: 6+ confirmations (very unlikely to reverse)

## Example: Sending 30 GXC

**Before Transaction**:
- Your UTXOs: [100 GXC]
- Your Balance: 100 GXC
- Recipient Balance: 0 GXC

**Transaction Created**:
- Input: 100 GXC UTXO (from previous transaction)
- Output 1: 30 GXC to recipient
- Output 2: 70 GXC change back to you
- Signed with your private key

**After Block Confirmation**:
- Your UTXOs: [70 GXC] (change)
- Your Balance: 70 GXC
- Recipient UTXOs: [30 GXC]
- Recipient Balance: 30 GXC
- Transaction: Confirmed in block #X

## Database Tables Updated

1. **blocks**: New block record
2. **transactions**: Your transaction record
3. **transaction_inputs**: Your transaction inputs (spent UTXOs)
4. **transaction_outputs**: Transaction outputs (recipient + change)
5. **utxo**: UTXO set updated (removed spent, added new)
6. **traceability_index**: Traceability record created

## Summary

When you send coins:
1. ✅ Transaction is created and signed
2. ✅ Transaction is validated and added to pool
3. ✅ Miner includes transaction in a block
4. ✅ Block is validated and added to chain
5. ✅ UTXO set is updated (spent UTXOs removed, new UTXOs added)
6. ✅ Balances are updated (calculated from UTXO set)
7. ✅ Everything is saved to database
8. ✅ Transaction becomes confirmed (1+ confirmations)

**The entire process is secure, validated, and permanently recorded on the blockchain!**
