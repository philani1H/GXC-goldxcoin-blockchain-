# Explorer Real-Time Updates Implementation

## Overview
All explorer templates now display data in real-time using WebSocket connections with automatic polling fallback.

## Changes Made

### 1. Created Shared Real-Time JavaScript Library
**File:** `web/static/realtime.js`

A reusable JavaScript class `GXCRealtime` that provides:
- WebSocket connection via Socket.IO
- Automatic polling fallback if WebSocket fails
- Event handlers for blocks, transactions, address updates, and stats
- Automatic reconnection with exponential backoff
- Utility functions for updating UI elements

### 2. Added WebSocket Events in Backend
**File:** `web/blockchain_explorer.py`

**New Events:**
- `new_transaction` - Emitted when a new transaction is stored
- `new_block` - Already existed, now enhanced
- `address_update` - Emitted when address balance/activity changes
- `stats_update` - Emitted when network statistics change

**Event Emission:**
- `store_transaction()` now emits `new_transaction` events
- `store_block()` already emits `new_block` events
- Address updates emit `address_update` events

### 3. Updated All Explorer Templates

#### blocks.html
- Added Socket.IO and realtime.js
- Real-time block table updates
- New block notifications
- Statistics auto-refresh

#### transactions.html
- Added Socket.IO and realtime.js
- Real-time transaction table updates
- New transaction notifications
- Statistics auto-refresh

#### address_detail.html
- Added Socket.IO and realtime.js
- Real-time balance updates
- Transaction list updates for this address
- Address-specific transaction notifications

#### block_detail.html
- Added Socket.IO and realtime.js
- Real-time confirmation count updates
- Block status updates

#### transaction_detail.html
- Added Socket.IO and realtime.js
- Real-time confirmation count updates
- Transaction status badge updates

#### explorer_index.html
- Already had WebSocket support
- Enhanced with new events

### 4. API Endpoints

**New/Updated:**
- `/api/v1/stats` - Returns current network statistics (for polling fallback)
- `/api/stats` - Existing endpoint (also works)

**Response Format:**
```json
{
    "total_blocks": 1234,
    "total_transactions": 5678,
    "total_addresses": 9012,
    "total_supply": 1000000,
    "current_height": 1234,
    "height": 1234
}
```

## How It Works

### WebSocket Flow
1. Client connects via Socket.IO
2. Server emits events when data changes:
   - New block mined → `new_block` event
   - New transaction stored → `new_transaction` event
   - Address updated → `address_update` event
3. Client receives events and updates UI
4. If connection fails, automatically falls back to polling

### Polling Fallback
1. If WebSocket connection fails
2. Client automatically switches to polling
3. Polls `/api/v1/stats` every 10 seconds
4. Updates statistics and refreshes data

### Real-Time Updates Per Page

**Blocks Page:**
- New blocks appear at top of table
- Block count updates
- Toast notification for new blocks

**Transactions Page:**
- New transactions appear at top of table
- Transaction count updates
- Toast notification for new transactions

**Address Detail Page:**
- Balance updates when transactions occur
- New transactions for this address appear
- Transaction count updates

**Block Detail Page:**
- Confirmation count updates as new blocks are mined
- Block status updates

**Transaction Detail Page:**
- Confirmation count updates as new blocks are mined
- Status badge updates (Unconfirmed → Confirming → Confirmed)

## Features

### Automatic Reconnection
- Exponential backoff (1s, 2s, 4s, 8s, ... up to 30s)
- Max 10 reconnection attempts
- Falls back to polling if reconnection fails

### Duplicate Prevention
- Checks if block/transaction already exists before adding
- Uses `data-block` and `data-tx` attributes for tracking

### Performance
- Only updates relevant UI elements
- Limits table rows (10 blocks, 20 transactions)
- Efficient DOM updates

### User Experience
- Toast notifications for important events
- Smooth animations
- No page refresh needed
- Works offline (shows last known data)

## Testing

### Test WebSocket Connection
1. Open browser console
2. Should see: `[Realtime] Connected to GXC Explorer`
3. Watch for events: `[Realtime] New block received: ...`

### Test Polling Fallback
1. Disable WebSocket in browser (block Socket.IO)
2. Should see: `[Realtime] Initializing polling fallback`
3. Data should still update every 10 seconds

### Test Real-Time Updates
1. Open blocks page
2. Mine a new block
3. Should see:
   - New block appears in table
   - Toast notification
   - Statistics update

## Configuration

### Customize Poll Interval
```javascript
const realtime = new GXCRealtime({
    pollInterval: 5000, // 5 seconds instead of 10
    useWebSocket: true,
    usePolling: true
});
```

### Disable Polling
```javascript
const realtime = new GXCRealtime({
    usePolling: false // Only use WebSocket
});
```

### Custom Event Handlers
```javascript
const realtime = new GXCRealtime({
    onBlock: function(block) {
        // Custom block handler
        console.log('Custom block handler:', block);
    },
    onTransaction: function(tx) {
        // Custom transaction handler
        console.log('Custom transaction handler:', tx);
    }
});
```

## Browser Support

- Modern browsers with WebSocket support
- Falls back to polling for older browsers
- Works with Socket.IO 4.5.4+

## Notes

- WebSocket connection is established on page load
- Events are only emitted when data actually changes
- Address updates only trigger for relevant addresses
- Confirmation counts update automatically as blocks are mined
- All updates are non-blocking and don't require page refresh
