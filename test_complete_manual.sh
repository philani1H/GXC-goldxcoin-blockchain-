#!/bin/bash
# Complete manual test of all blockchain functions

set -e

echo "============================================================"
echo "  GXC Complete Manual Test"
echo "  Testing: Mining → Sending → Staking → Unstaking"
echo "============================================================"
echo ""

# Colors
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Step 1: Mine blocks using Python
echo -e "${BLUE}Step 1: Mining blocks...${NC}"
python3 << 'EOF'
import sys
sys.path.insert(0, './python')
from gxc_api import GXCAPI

api = GXCAPI(testnet=True)
wallet_address = api.create_wallet()
print(f"Wallet created: {wallet_address}")

print("Mining 10 blocks...")
for i in range(10):
    block = api.mine_block(wallet_address)
    print(f"  Block {i+1} mined")

balance = api.get_balance(wallet_address)
print(f"Balance: {balance} GXC")

# Save wallet address for next steps
with open('/tmp/gxc_test_wallet.txt', 'w') as f:
    f.write(wallet_address)
EOF

echo -e "${GREEN}✅ Mining complete${NC}"
echo ""

# Step 2: Send transaction
echo -e "${BLUE}Step 2: Sending transaction...${NC}"
python3 << 'EOF'
import sys
sys.path.insert(0, './python')
from gxc_api import GXCAPI

api = GXCAPI(testnet=True)

# Load wallet
with open('/tmp/gxc_test_wallet.txt', 'r') as f:
    wallet_address = f.read().strip()

# Create recipient
recipient = api.create_wallet()
print(f"Recipient: {recipient}")

# Send transaction
print("Sending 10 GXC...")
tx_hash = api.send_transaction(wallet_address, recipient, 10.0)
print(f"Transaction: {tx_hash}")

# Mine to confirm
api.mine_block(wallet_address)

# Check balances
sender_balance = api.get_balance(wallet_address)
recipient_balance = api.get_balance(recipient)
print(f"Sender balance: {sender_balance} GXC")
print(f"Recipient balance: {recipient_balance} GXC")
EOF

echo -e "${GREEN}✅ Transaction complete${NC}"
echo ""

# Step 3: Register validator and stake
echo -e "${BLUE}Step 3: Registering validator and staking...${NC}"
python3 << 'EOF'
import sys
sys.path.insert(0, './python')
from gxc_api import GXCAPI

api = GXCAPI(testnet=True)

# Load wallet
with open('/tmp/gxc_test_wallet.txt', 'r') as f:
    wallet_address = f.read().strip()

# Register validator
print("Registering validator with 50 GXC stake...")
result = api.register_validator(wallet_address, 50.0)
print(f"Validator registered: {result}")

# Mine to confirm
api.mine_block(wallet_address)

# Check validator info
validator_info = api.get_validator_info(wallet_address)
print(f"Validator stake: {validator_info.get('stake', 0)} GXC")
EOF

echo -e "${GREEN}✅ Validator registration complete${NC}"
echo ""

# Step 4: Additional staking
echo -e "${BLUE}Step 4: Additional staking...${NC}"
python3 << 'EOF'
import sys
sys.path.insert(0, './python')
from gxc_api import GXCAPI

api = GXCAPI(testnet=True)

# Load wallet
with open('/tmp/gxc_test_wallet.txt', 'r') as f:
    wallet_address = f.read().strip()

# Stake more
print("Staking additional 25 GXC...")
tx_hash = api.stake(wallet_address, 25.0)
print(f"Stake transaction: {tx_hash}")

# Mine to confirm
api.mine_block(wallet_address)

# Check validator info
validator_info = api.get_validator_info(wallet_address)
print(f"Total staked: {validator_info.get('stake', 0)} GXC")
EOF

echo -e "${GREEN}✅ Additional staking complete${NC}"
echo ""

# Step 5: Unstake
echo -e "${BLUE}Step 5: Unstaking...${NC}"
python3 << 'EOF'
import sys
sys.path.insert(0, './python')
from gxc_api import GXCAPI

api = GXCAPI(testnet=True)

# Load wallet
with open('/tmp/gxc_test_wallet.txt', 'r') as f:
    wallet_address = f.read().strip()

# Unstake
print("Unstaking 20 GXC...")
tx_hash = api.unstake(wallet_address, 20.0)
print(f"Unstake transaction: {tx_hash}")

# Mine to confirm
api.mine_block(wallet_address)

# Check validator info
validator_info = api.get_validator_info(wallet_address)
print(f"Remaining staked: {validator_info.get('stake', 0)} GXC")

# Final balance
balance = api.get_balance(wallet_address)
print(f"Final balance: {balance} GXC")
EOF

echo -e "${GREEN}✅ Unstaking complete${NC}"
echo ""

# Cleanup
rm -f /tmp/gxc_test_wallet.txt

echo "============================================================"
echo -e "${GREEN}✅ All tests completed successfully!${NC}"
echo "============================================================"
