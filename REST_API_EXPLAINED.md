# REST API URL Explained - Complete Guide

## What is a REST API?

**REST** stands for **Representational State Transfer**. It's a way for applications to communicate over the internet using standard HTTP methods.

### Simple Explanation

Think of REST API like a **menu at a restaurant**:
- **URL** = What dish you want (the endpoint)
- **HTTP Method** = How you order it (GET, POST, etc.)
- **Response** = The food you get back (data in JSON format)

---

## What is a REST API URL?

A **REST API URL** is a web address (like a website URL) that you use to request data or perform actions on a server.

### URL Structure

```
http://localhost:8080/api/v1/blockchain/info
│    │           │    │   │  │         │
│    │           │    │   │  │         └─ Endpoint (what you want)
│    │           │    │   │  └─ Version
│    │           │    │   └─ API path
│    │           │    └─ Port number
│    │           └─ Domain/hostname
│    └─ Protocol (HTTP/HTTPS)
```

### Real Example (GXC Blockchain)

```
http://localhost:8080/api/v1/address/GXC123.../balance
```

**Breaking it down:**
- `http://` - Protocol (how to communicate)
- `localhost` - Server address (your computer)
- `8080` - Port number (which service)
- `/api/v1/` - API path and version
- `address/GXC123.../balance` - What you want (balance for that address)

---

## REST vs RPC - What's the Difference?

### REST API (RESTful)
- Uses **HTTP methods** (GET, POST, PUT, DELETE)
- **URL-based** - different URLs for different actions
- **Stateless** - each request is independent
- **Human-readable** URLs
- Returns **JSON** data

**Example:**
```bash
GET http://localhost:8080/api/v1/blockchain/info
```

### RPC (Remote Procedure Call)
- Uses **POST** method only
- **Method-based** - same URL, different methods
- Sends **JSON-RPC** format
- More like function calls

**Example:**
```bash
POST http://localhost:8545
Body: {
  "jsonrpc": "2.0",
  "method": "getblockchaininfo",
  "params": [],
  "id": 1
}
```

---

## GXC Blockchain REST API Endpoints

### Base URL

```
http://localhost:8080/api/v1
```

### Common Endpoints

#### 1. **Get Blockchain Info**
```
GET http://localhost:8080/api/v1/blockchain/info
```

**Response:**
```json
{
  "chain": "main",
  "blocks": 12345,
  "difficulty": 4.5,
  "bestblockhash": "0xabc123..."
}
```

#### 2. **Get Address Balance**
```
GET http://localhost:8080/api/v1/address/GXC123.../balance
```

**Response:**
```json
{
  "address": "GXC123...",
  "balance": 100.5,
  "confirmed": 100.5,
  "unconfirmed": 0.0
}
```

#### 3. **Get Address Transactions**
```
GET http://localhost:8080/api/v1/address/GXC123.../transactions?limit=50
```

**Response:**
```json
[
  {
    "hash": "0x...",
    "from": "GXC...",
    "to": "GXC123...",
    "amount": 12.5,
    "timestamp": 1234567890
  }
]
```

#### 4. **Get Block**
```
GET http://localhost:8080/api/v1/block/12345
```

#### 5. **Get Transaction**
```
GET http://localhost:8080/api/v1/transaction/0xabc123...
```

#### 6. **Submit Transaction**
```
POST http://localhost:8080/api/v1/transactions
Body: {
  "from": "GXC...",
  "to": "GXC...",
  "amount": 10.0,
  "fee": 0.001
}
```

---

## How to Use REST API URLs

### Method 1: Browser

Simply paste the URL in your browser:

```
http://localhost:8080/api/v1/blockchain/info
```

The browser will make a GET request and show the JSON response.

### Method 2: cURL (Command Line)

```bash
# GET request
curl http://localhost:8080/api/v1/blockchain/info

# With formatting
curl http://localhost:8080/api/v1/blockchain/info | python -m json.tool

# POST request
curl -X POST http://localhost:8080/api/v1/transactions \
  -H "Content-Type: application/json" \
  -d '{"from":"GXC...","to":"GXC...","amount":10.0}'
```

### Method 3: Python (requests library)

```python
import requests

# GET request
response = requests.get("http://localhost:8080/api/v1/blockchain/info")
data = response.json()
print(data)

# POST request
response = requests.post(
    "http://localhost:8080/api/v1/transactions",
    json={
        "from": "GXC...",
        "to": "GXC...",
        "amount": 10.0
    }
)
result = response.json()
```

### Method 4: JavaScript (fetch)

```javascript
// GET request
fetch('http://localhost:8080/api/v1/blockchain/info')
  .then(response => response.json())
  .then(data => console.log(data));

// POST request
fetch('http://localhost:8080/api/v1/transactions', {
  method: 'POST',
  headers: {
    'Content-Type': 'application/json',
  },
  body: JSON.stringify({
    from: 'GXC...',
    to: 'GXC...',
    amount: 10.0
  })
})
.then(response => response.json())
.then(data => console.log(data));
```

---

## HTTP Methods Explained

### GET
- **Read data** - doesn't change anything
- **No body** - all info in URL
- **Safe** - can call multiple times

**Example:**
```
GET http://localhost:8080/api/v1/blockchain/info
```

### POST
- **Create/Submit data** - changes something
- **Has body** - data in request body
- **Not idempotent** - calling multiple times may have different results

**Example:**
```
POST http://localhost:8080/api/v1/transactions
Body: { "from": "...", "to": "...", "amount": 10.0 }
```

### PUT
- **Update data** - replaces existing data
- **Has body** - new data in body

### DELETE
- **Remove data** - deletes something
- **No body** usually

---

## URL Parameters

### Query Parameters

Add `?` and `&` to pass parameters:

```
GET http://localhost:8080/api/v1/address/GXC.../transactions?limit=50&offset=0
```

**Parameters:**
- `limit=50` - Get 50 transactions
- `offset=0` - Start from first transaction

### Path Parameters

Parameters in the URL path:

```
GET http://localhost:8080/api/v1/address/GXC123.../balance
                                    └─ Address is a path parameter
```

---

## Response Formats

### JSON (JavaScript Object Notation)

Most REST APIs return JSON:

```json
{
  "address": "GXC123...",
  "balance": 100.5,
  "transactions": [
    {
      "hash": "0x...",
      "amount": 12.5
    }
  ]
}
```

### Status Codes

HTTP status codes tell you if the request succeeded:

- **200 OK** - Success ✅
- **201 Created** - Resource created ✅
- **400 Bad Request** - Invalid request ❌
- **404 Not Found** - Resource doesn't exist ❌
- **500 Server Error** - Server problem ❌

---

## GXC Blockchain REST API Examples

### Example 1: Check Blockchain Status

```bash
curl http://localhost:8080/api/v1/blockchain/info
```

**Response:**
```json
{
  "chain": "main",
  "blocks": 12345,
  "difficulty": 4.5,
  "bestblockhash": "0xabc123...",
  "networkhashps": 1000000
}
```

### Example 2: Get Your Wallet Balance

```bash
curl http://localhost:8080/api/v1/address/GXC123.../balance
```

**Response:**
```json
{
  "address": "GXC123...",
  "balance": 100.5,
  "confirmed": 100.5,
  "unconfirmed": 0.0
}
```

### Example 3: Get Your Transactions

```bash
curl http://localhost:8080/api/v1/address/GXC123.../transactions?limit=10
```

**Response:**
```json
[
  {
    "hash": "0xabc123...",
    "type": "coinbase",
    "from": null,
    "to": "GXC123...",
    "amount": 12.5,
    "timestamp": 1234567890,
    "confirmations": 6
  },
  {
    "hash": "0xdef456...",
    "type": "transfer",
    "from": "GXC789...",
    "to": "GXC123...",
    "amount": 5.0,
    "timestamp": 1234567880,
    "confirmations": 6
  }
]
```

### Example 4: Get Specific Block

```bash
curl http://localhost:8080/api/v1/block/12345
```

### Example 5: Get Specific Transaction

```bash
curl http://localhost:8080/api/v1/transaction/0xabc123...
```

---

## REST API URL in GXC Context

### Where You See It

1. **Wallet Service:**
   ```python
   REST_URL = "http://localhost:8080"
   # Used to get balance, transactions, etc.
   ```

2. **Miner:**
   ```python
   rest_url = "http://localhost:8080"
   # Used to get blockchain info, transactions
   ```

3. **Explorer:**
   ```
   http://localhost:8080/api/v1/...
   # Used to display blockchain data
   ```

### Default Ports

- **REST API:** Port `8080`
- **RPC API:** Port `8545`
- **Network (P2P):** Port `9333`

### Production URLs

When deployed to a server:

```
http://your-server.com:8080/api/v1/blockchain/info
http://your-server.com:8080/api/v1/address/GXC.../balance
```

Or with domain:

```
https://api.gxc-blockchain.com/api/v1/blockchain/info
```

---

## Testing REST API

### Quick Test Script

```python
#!/usr/bin/env python3
import requests

# Test REST API
base_url = "http://localhost:8080/api/v1"

# Test 1: Blockchain info
print("Testing blockchain info...")
response = requests.get(f"{base_url}/blockchain/info")
if response.status_code == 200:
    print("✅ REST API is working!")
    print(response.json())
else:
    print(f"❌ Error: {response.status_code}")

# Test 2: Get balance (replace with your address)
address = "GXC..."
print(f"\nTesting balance for {address}...")
response = requests.get(f"{base_url}/address/{address}/balance")
if response.status_code == 200:
    data = response.json()
    print(f"✅ Balance: {data.get('balance', 0)} GXC")
else:
    print(f"❌ Error: {response.status_code}")
```

---

## Common Questions

### Q: What's the difference between REST and RPC?

**A:**
- **REST:** URL-based, uses HTTP methods, human-readable
- **RPC:** Method-based, same URL, function-like calls

### Q: Why use REST instead of RPC?

**A:**
- Easier to understand
- Can test in browser
- Better for web applications
- Standard HTTP methods

### Q: Can I use both?

**A:** Yes! GXC blockchain provides both:
- **REST:** `http://localhost:8080` - For web apps
- **RPC:** `http://localhost:8545` - For programmatic access

### Q: What if the port is different?

**A:** Change the port in the URL:
```
http://localhost:8080  →  http://localhost:9000
```

### Q: How do I know what endpoints are available?

**A:**
- Check API documentation
- Look at the code (RESTServer.cpp)
- Try common patterns:
  - `/api/v1/blockchain/info`
  - `/api/v1/address/{address}/balance`
  - `/api/v1/block/{height}`

---

## Summary

### REST API URL = Web Address for Data

**Structure:**
```
http://hostname:port/path/to/resource
```

**Example:**
```
http://localhost:8080/api/v1/address/GXC123.../balance
```

**What it does:**
- Connects to blockchain node
- Requests specific data
- Returns JSON response
- No authentication needed (for public data)

**Use cases:**
- ✅ Get blockchain info
- ✅ Check wallet balance
- ✅ View transactions
- ✅ Get block data
- ✅ Submit transactions

**It's like:**
- A **phone number** to call the blockchain
- A **menu item** to order specific data
- A **web page** that returns JSON instead of HTML

---

## Quick Reference

### GXC REST API Base URL
```
http://localhost:8080/api/v1
```

### Common Endpoints
```
GET  /blockchain/info
GET  /address/{address}/balance
GET  /address/{address}/transactions
GET  /block/{height}
GET  /transaction/{hash}
POST /transactions
```

### Test in Browser
Just paste the URL:
```
http://localhost:8080/api/v1/blockchain/info
```

### Test with cURL
```bash
curl http://localhost:8080/api/v1/blockchain/info
```

---

**Need more help?** Check the API documentation or explore endpoints by trying different URLs!

