# GXC Regulator API Documentation

## For SEC, FCA, MAS and Other Regulatory Bodies

**Version**: 1.0  
**Purpose**: Enable regulators to monitor stock contracts with full traceability

---

## Overview

The GXC Admin system provides regulatory organizations (SEC, FCA, MAS, etc.) with complete visibility into stock token contracts. The GXC team grants access to regulators, who can then:

- **View all transactions** with full traceability back to origin
- **Generate compliance reports** in SEC/FCA formats
- **Track beneficial ownership** changes
- **Detect wash trading** patterns
- **Communicate officially** with market makers
- **Issue compliance warnings** with response deadlines

The blockchain manages itself - regulators only oversee the stock contract layer.

---

## Quick Start

### Demo Credentials
```
Username: sec_analyst
Password: SEC_User_2025!
```

### Login
```bash
curl -X POST http://localhost:5002/api/v1/regulator/organization/login \
  -H "Content-Type: application/json" \
  -d '{"username": "sec_analyst", "password": "SEC_User_2025!"}'
```

---

## API Endpoints

### 1. Compliance Reports

#### Generate Report
```
POST /api/v1/regulator/reports/generate
```

**Report Types:**
- `SEC_13F` - Institutional Holdings Report
- `SEC_13D` - Beneficial Ownership Report  
- `FCA_TRANSACTION` - FCA Transaction Report
- `TRANSACTION_HISTORY` - Full audit trail
- `WASH_TRADING` - Wash trading analysis
- `OWNERSHIP_CHANGES` - Ownership change report

**Request:**
```json
{
  "reportType": "SEC_13F",
  "format": "JSON",
  "parameters": {
    "period": "Q4 2025"
  }
}
```

**Response:**
```json
{
  "success": true,
  "reportId": "RPT_SEC_13F_1703001234_abc123",
  "data": {
    "formType": "13F-HR",
    "reportingPeriod": "Q4 2025",
    "holdings": [...]
  }
}
```

---

### 2. Transaction Traceability

#### Trace Transaction
```
GET /api/v1/regulator/transactions/trace/{tx_hash}
```

Uses GXC's traceability formula to trace any transaction back to its origin.

**Response:**
```json
{
  "success": true,
  "txHash": "TX_001",
  "traceDepth": 3,
  "originTransaction": {...},
  "traceChain": [
    {"tx_hash": "TX_001", "from": "user1", "to": "user2", "amount": 100},
    {"tx_hash": "TX_000", "from": "coinbase", "to": "user1", "amount": 100}
  ]
}
```

#### Search Transactions
```
POST /api/v1/regulator/transactions/search
```

**Request:**
```json
{
  "stockSymbol": "AAPL",
  "fromAddress": "tGXC_user1",
  "minAmount": 100,
  "startTime": 1703000000,
  "endTime": 1703100000,
  "flagged": true
}
```

---

### 3. Beneficial Ownership

#### Get Ownership
```
GET /api/v1/regulator/ownership/{stock_symbol}
```

**Response:**
```json
{
  "success": true,
  "stockSymbol": "AAPL",
  "totalSharesTracked": 18200,
  "majorHolders": 2,
  "ownership": [
    {
      "holder_address": "tGXC_fund1",
      "holder_name": "Vanguard Fund",
      "shares_held": 5000,
      "percentage": 27.5
    }
  ]
}
```

#### Get Ownership Changes
```
GET /api/v1/regulator/ownership/changes?days=30&threshold=1.0
```

---

### 4. Wash Trading Detection

#### Get Alerts
```
GET /api/v1/regulator/wash-trading/alerts?status=PENDING
```

#### Analyze for Patterns
```
POST /api/v1/regulator/wash-trading/analyze
```

**Request:**
```json
{
  "stockSymbol": "AAPL",
  "timeWindowMinutes": 60
}
```

**Response:**
```json
{
  "success": true,
  "transactionsAnalyzed": 150,
  "patternsFound": 1,
  "patterns": [
    {
      "alertId": "WASH_AAPL_1703001234_abc123",
      "patternType": "SELF_TRADING",
      "confidence": 0.75,
      "suspiciousAddresses": [...]
    }
  ]
}
```

#### Review Alert
```
POST /api/v1/regulator/wash-trading/review
```

---

### 5. Communication Hub

#### Send Official Notice
```
POST /api/v1/regulator/communications/send
```

**Request:**
```json
{
  "toMarketMakerId": "MM_Goldman_Sachs",
  "commType": "INQUIRY",
  "subject": "Transaction Inquiry - AAPL",
  "message": "Please provide details...",
  "priority": "HIGH",
  "requiresResponse": true,
  "responseDeadlineDays": 14
}
```

#### Get Pending Responses
```
GET /api/v1/regulator/communications/pending-responses
```

Shows all communications awaiting market maker response, with overdue flags.

---

### 6. Regulatory Broadcasts

#### Send Broadcast
```
POST /api/v1/regulator/broadcasts/send
```

**Request:**
```json
{
  "title": "New Reporting Requirements",
  "message": "Effective January 1, 2026...",
  "broadcastType": "REGULATORY_UPDATE",
  "targetAudience": "ALL_MARKET_MAKERS",
  "effectiveDate": 1735689600
}
```

---

### 7. Compliance Warnings

#### Issue Warning
```
POST /api/v1/regulator/warnings/issue
```

**Request:**
```json
{
  "marketMakerId": "MM_Example",
  "warningType": "LATE_REPORTING",
  "message": "Your Q3 report was submitted 5 days late...",
  "responseDeadlineDays": 7
}
```

#### Get Warning History
```
GET /api/v1/regulator/warnings/history/{market_maker_id}
```

---

### 8. Document Requests

#### Request Documents
```
POST /api/v1/regulator/documents/request
```

**Request:**
```json
{
  "marketMakerId": "MM_Example",
  "documentType": "FINANCIAL_STATEMENTS",
  "purpose": "Annual Audit",
  "requiredDocuments": [
    "Audited Balance Sheet",
    "Income Statement",
    "Cash Flow Statement"
  ],
  "deadlineDays": 14
}
```

---

## Communication Types

| Type | Description | Default Priority |
|------|-------------|------------------|
| `INQUIRY` | General inquiry | NORMAL |
| `COMPLIANCE_WARNING` | Compliance issue warning | HIGH |
| `DOCUMENT_REQUEST` | Request for documentation | NORMAL |
| `REGULATORY_UPDATE` | Regulatory change notice | NORMAL |
| `ENFORCEMENT_NOTICE` | Enforcement action notice | URGENT |

---

## Report Formats

### SEC Form 13F (Institutional Holdings)
```json
{
  "formType": "13F-HR",
  "reportingPeriod": "Q4 2025",
  "holdings": [
    {
      "nameOfIssuer": "AAPL",
      "cusip": "GXCAAPL00",
      "value": 150000,
      "sharesOrPrincipal": 1000,
      "votingAuthority": {"sole": 1000, "shared": 0, "none": 0}
    }
  ]
}
```

### SEC Form 13D (Beneficial Ownership)
```json
{
  "formType": "13D",
  "issuerName": "AAPL",
  "beneficialOwners": [
    {
      "name": "Vanguard Fund",
      "sharesOwned": 5000,
      "percentageOwned": 27.5,
      "typeOfOwnership": "INSTITUTION"
    }
  ]
}
```

### FCA Transaction Report
```json
{
  "reportType": "FCA_TRANSACTION_REPORT",
  "transactions": [
    {
      "transactionReferenceNumber": "TX_001",
      "tradingDateTime": "2025-12-23T10:30:00",
      "instrumentIdentification": "AAPL",
      "price": 150.50,
      "quantity": 100,
      "traceability": "TX_000"
    }
  ]
}
```

---

## Access Control

### Organization Registration (GXC Admin Only)
```
POST /api/v1/regulator/organization/register
```

GXC team registers regulatory organizations and provides API keys.

### Permissions
- `view_transactions` - View transaction data
- `view_ownership` - View beneficial ownership
- `generate_reports` - Generate compliance reports
- `view_alerts` - View wash trading alerts
- `send_communications` - Send official communications
- `view_market_makers` - View market maker details

---

## Why This Matters

1. **Full Traceability** - Every stock token can be traced back to its origin
2. **Real-time Monitoring** - Regulators see transactions as they happen
3. **Automated Detection** - Wash trading patterns flagged automatically
4. **Official Channel** - Documented communications with audit trail
5. **Standard Formats** - Reports in SEC/FCA formats for easy integration

The blockchain is self-governing. Regulators oversee the securities layer to ensure market integrity.

---

## Running the Server

```bash
cd GXC-MANAGEMENT-STOCK/server
python regulator_api.py
```

Server runs on port 5002.
