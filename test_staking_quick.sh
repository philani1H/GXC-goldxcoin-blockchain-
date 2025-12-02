#!/bin/bash
# Quick staking test script

RPC_URL="${1:-http://localhost:18332}"

echo "=========================================="
echo "Quick Staking Test"
echo "=========================================="
echo "RPC URL: $RPC_URL"
echo ""

# Test 1: Connection
echo "Test 1: Testing connection..."
RESPONSE=$(curl -s -X POST "$RPC_URL" \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getblockchaininfo","params":[],"id":1}')

if echo "$RESPONSE" | grep -q '"result"'; then
    HEIGHT=$(echo "$RESPONSE" | grep -o '"blocks":[0-9]*' | grep -o '[0-9]*')
    echo "✅ Connected! Chain height: $HEIGHT"
else
    echo "❌ Connection failed!"
    echo "$RESPONSE"
    exit 1
fi

# Test 2: Get validators
echo ""
echo "Test 2: Getting validators..."
RESPONSE=$(curl -s -X POST "$RPC_URL" \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getvalidators","params":[],"id":1}')

if echo "$RESPONSE" | grep -q '"result"'; then
    VALIDATOR_COUNT=$(echo "$RESPONSE" | grep -o '"address"' | wc -l)
    echo "✅ Found $VALIDATOR_COUNT validators"
    if [ "$VALIDATOR_COUNT" -gt 0 ]; then
        echo "$RESPONSE" | grep -o '"address":"[^"]*"' | head -3 | sed 's/"address":"/   - /' | sed 's/"$//'
    fi
else
    echo "⚠️  No validators found or error occurred"
fi

# Test 3: Get block template
echo ""
echo "Test 3: Getting block template..."
RESPONSE=$(curl -s -X POST "$RPC_URL" \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"getblocktemplate","params":[],"id":1}')

if echo "$RESPONSE" | grep -q '"result"'; then
    CONSENSUS=$(echo "$RESPONSE" | grep -o '"consensus_type":"[^"]*"' | grep -o ':"[^"]*"' | sed 's/:"//' | sed 's/"//')
    HEIGHT=$(echo "$RESPONSE" | grep -o '"height":[0-9]*' | grep -o '[0-9]*')
    echo "✅ Block template retrieved"
    echo "   Height: $HEIGHT"
    echo "   Consensus type: $CONSENSUS"
    
    if [ "$CONSENSUS" = "pos" ]; then
        echo "   ✅ Next block is PoS!"
        VALIDATOR=$(echo "$RESPONSE" | grep -o '"validator_address":"[^"]*"' | grep -o ':"[^"]*"' | sed 's/:"//' | sed 's/"//')
        if [ -n "$VALIDATOR" ]; then
            echo "   Validator: $VALIDATOR"
        fi
    else
        echo "   ℹ️  Next block is PoW (PoS blocks are at odd heights)"
    fi
else
    echo "❌ Failed to get block template"
fi

echo ""
echo "=========================================="
echo "Quick test complete!"
echo "=========================================="
echo ""
echo "For comprehensive testing, run:"
echo "  python3 test_staking.py --rpc-url $RPC_URL"
