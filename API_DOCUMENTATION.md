# GXC Blockchain API Documentation

This documentation details the JSON-RPC API for the GXC Blockchain. Developers can use these methods to build wallets, explorers, and other applications that interact with the GXC node.

## Connection

*   **Default Mainnet RPC Port**: `8332`
*   **Default Testnet RPC Port**: `18332`
*   **Protocol**: JSON-RPC 2.0 over HTTP

### Common Request Format
```json
{
  "jsonrpc": "2.0",
  "method": "method_name",
  "params": ["param1", "param2"],
  "id": 1
}
```

### Common Response Format
```json
{
  "jsonrpc": "2.0",
  "result": { ... },
  "error": null,
  "id": 1
}
```

---

## Wallet & Transaction Methods

These methods are essential for building wallet applications.

### `getbalance`
Returns the balance of a specific address.
*   **Params**: `[address]` (string)
*   **Result**: `amount` (number) - The balance in GXC.
*   **Example**:
    ```json
    {"method": "getbalance", "params": ["tGXC..."], "id": 1}
    ```

### `getnewaddress`
Generates a new address. If the node wallet is active, returns the node's wallet address.
*   **Params**: `[account]` (string, optional) - Account name (ignored in current version).
*   **Result**: `address` (string) - A new GXC address (starts with `GXC` on Mainnet, `tGXC` on Testnet).

### `sendtoaddress`
Sends GXC to a specific address.
*   **Params**:
    1.  `address` (string) - Recipient address.
    2.  `amount` (number or string) - Amount to send.
*   **Result**: `txid` (string) - The transaction hash.
*   **Example**:
    ```json
    {"method": "sendtoaddress", "params": ["tGXC...", 10.5], "id": 1}
    ```

### `validateaddress`
Checks if an address is valid.
*   **Params**: `[address]` (string)
*   **Result**:
    ```json
    {
      "isvalid": true,
      "address": "..."
    }
    ```

### `listtransactions`
Lists transactions for an address.
*   **Params**:
    1.  `address` (string)
    2.  `count` (int, default: 10)
    3.  `skip` (int, default: 0)
*   **Result**: Array of transaction objects.

### `gettransaction`
Get detailed information about a specific transaction.
*   **Params**: `[txid]` (string)
*   **Result**: Transaction object including inputs, outputs, confirmations, and traceability info.

### `estimatefee`
Estimates the transaction fee required for timely confirmation.
*   **Params**: None
*   **Result**:
    ```json
    {
      "fee": 0.0015,
      "recommended_fee": 0.0015,
      "pending_transactions": 5
    }
    ```

---

## Staking & Validator Methods

Methods for staking GXC and running validators.

### `registervalidator`
Registers a new validator. Requires the node wallet to have sufficient funds.
*   **Params**:
    1.  `address` (string) - The validator address (must match node wallet).
    2.  `stakeAmount` (number) - Amount to stake (min 100 GXC).
    3.  `stakingDays` (int) - Lockup period in days (14-365).
*   **Result**:
    ```json
    {
      "success": true,
      "stake_tx": "txid..."
    }
    ```

### `stake`
Alias for `registervalidator`.

### `unstake`
Unstakes funds from a validator.
*   **Params**:
    1.  `address` (string)
    2.  `amount` (number, optional) - Amount to unstake. If 0, unstakes all.
*   **Result**: JSON object confirming the unstake transaction.

### `getstakinginfo`
Returns detailed staking information for an address.
*   **Params**: `[address]` (string)
*   **Result**:
    ```json
    {
      "spendable_balance": 100.0,
      "staked_balance": 500.0,
      "is_validator": true,
      "validator_total_rewards": 12.5,
      "apy": 5.2
    }
    ```

### `getvalidators`
Returns a list of all active validators.
*   **Params**: None
*   **Result**: Array of validator objects.

---

## Blockchain Information Methods

Methods to query blockchain state.

### `getblockchaininfo`
Returns general information about the blockchain state.
*   **Params**: None
*   **Result**:
    ```json
    {
      "chain": "test",
      "blocks": 12345,
      "difficulty": 0.1,
      "network": "testnet"
    }
    ```

### `getblock`
Returns details of a block.
*   **Params**: `[hash_or_height]` (string or number)
*   **Result**: Block object including transactions.

### `getblockcount`
Returns the current block height.
*   **Params**: None
*   **Result**: `height` (number)

---

## Utility Methods

### `/health` (HTTP GET)
Health check endpoint for monitoring.
*   **Response**:
    ```json
    {
      "status": "ok",
      "network": "testnet",
      "height": 12345
    }
    ```

## Network Configuration

*   **Testnet Addresses**: Start with `tGXC`.
*   **Mainnet Addresses**: Start with `GXC`.
*   **Traceability**: All transactions enforce the formula `Ti.Inputs[0].txHash == Ti.PrevTxHash`. Wallet developers must ensure `prevTxHash` is correctly set to the transaction hash of the first input.
