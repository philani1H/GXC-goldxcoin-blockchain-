#!/bin/bash

# Test Fraud Detection APIs
# This script tests all public fraud detection endpoints

BASE_URL="http://localhost:8080"

echo "========================================="
echo "Testing Fraud Detection APIs"
echo "========================================="
echo ""

# Test 1: Check Transaction Taint
echo "Test 1: Check Transaction Taint"
echo "GET /api/fraud/check-transaction/test_tx_hash"
curl -s "$BASE_URL/api/fraud/check-transaction/test_tx_hash" | jq '.'
echo ""
echo ""

# Test 2: Check Address Fraud
echo "Test 2: Check Address Fraud"
echo "GET /api/fraud/check-address/test_address"
curl -s "$BASE_URL/api/fraud/check-address/test_address" | jq '.'
echo ""
echo ""

# Test 3: Report Stolen Funds
echo "Test 3: Report Stolen Funds"
echo "POST /api/fraud/report-stolen"
curl -s -X POST "$BASE_URL/api/fraud/report-stolen" \
  -H "Content-Type: application/json" \
  -d '{
    "txHash": "test_stolen_tx_12345",
    "reporterAddress": "test_victim_address",
    "amount": 100.0,
    "description": "Test fraud report - funds were stolen"
  }' | jq '.'
echo ""
echo ""

# Test 4: Get Blockchain Info
echo "Test 4: Get Blockchain Info"
echo "GET /api/blockchain/info"
curl -s "$BASE_URL/api/blockchain/info" | jq '.'
echo ""
echo ""

echo "========================================="
echo "All API tests completed"
echo "========================================="
