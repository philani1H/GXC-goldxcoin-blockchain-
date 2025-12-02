# GXC Wallet Web Developer Guide

Complete guide for building a modern web wallet application for the GXC blockchain.

---

## 📋 Table of Contents

1. [Overview](#overview)
2. [Architecture](#architecture)
3. [API Reference](#api-reference)
4. [Getting Started](#getting-started)
5. [Frontend Implementation](#frontend-implementation)
6. [Backend Integration](#backend-integration)
7. [Real-time Updates](#real-time-updates)
8. [Security](#security)
9. [UI/UX Best Practices](#uiux-best-practices)
10. [Testing](#testing)
11. [Deployment](#deployment)
12. [Complete Examples](#complete-examples)

---

## 🎯 Overview

This guide provides everything you need to build a production-ready web wallet application for the GXC blockchain, including:

- ✅ **RPC API Integration** - Connect to blockchain nodes
- ✅ **REST API** - Use REST endpoints for simplified access
- ✅ **WebSocket Support** - Real-time transaction updates
- ✅ **Modern Frameworks** - React, Vue, Angular examples
- ✅ **Mobile Responsive** - Works on all devices
- ✅ **Security Best Practices** - Production-ready security
- ✅ **Complete Code Examples** - Copy-paste ready code

---

## 🏗️ Architecture

### Recommended Stack

**Frontend:**
- React/Vue/Angular (or vanilla JavaScript)
- WebSocket client for real-time updates
- State management (Redux/Vuex/NgRx)
- UI framework (Material-UI, Tailwind CSS, Bootstrap)

**Backend (Optional):**
- Node.js/Python/Go backend proxy
- WebSocket server for real-time updates
- Caching layer (Redis)
- Rate limiting

**Blockchain Connection:**
- Direct RPC connection to GXC node
- Or use REST API proxy
- WebSocket for real-time events

### Architecture Diagram

```
┌─────────────────┐
│  Web Wallet UI  │
│  (React/Vue)    │
└────────┬────────┘
         │
    ┌────┴────┐
    │         │
┌───▼───┐ ┌──▼──────┐
│ REST  │ │ WebSocket│
│ Proxy │ │  Server  │
└───┬───┘ └──┬───────┘
    │        │
    └───┬────┘
        │
┌───────▼────────┐
│  GXC Blockchain │
│      Node       │
│  (RPC: 18332)   │
└─────────────────┘
```

---

## 📡 API Reference

### RPC API Endpoints

All RPC calls use JSON-RPC 2.0 format:

**Base URL:** `http://localhost:18332` (testnet) or `http://localhost:8332` (mainnet)

**Request Format:**
```json
{
  "jsonrpc": "2.0",
  "method": "method_name",
  "params": [...],
  "id": 1
}
```

### Wallet Methods

| Method | Description | Parameters | Example |
|--------|-------------|------------|---------|
| `getnewaddress` | Generate new address | `[]` | `["getnewaddress", []]` |
| `getbalance` | Get balance | `[address]` | `["getbalance", ["tGXC...123"]]` |
| `listtransactions` | Get transactions | `[address, limit]` | `["listtransactions", ["tGXC...123", 100]]` |
| `sendtoaddress` | Send coins | `[from, to, amount]` | `["sendtoaddress", ["from", "to", 100.0]]` |
| `validateaddress` | Validate address | `[address]` | `["validateaddress", ["tGXC...123"]]` |
| `gettransaction` | Get transaction details | `[txid]` | `["gettransaction", ["abc123..."]]` |

### Staking Methods

| Method | Description | Parameters | Example |
|--------|-------------|------------|---------|
| `registervalidator` | Register as validator | `[address, stakeAmount, stakingDays]` | `["registervalidator", ["tGXC...123", 1000.0, 90]]` |
| `stake` | Stake coins (alias) | `[address, stakeAmount, stakingDays]` | Same as above |
| `addstake` | Add more stake | `[address, additionalAmount]` | `["addstake", ["tGXC...123", 500.0]]` |
| `unstake` | Withdraw stake | `[address]` | `["unstake", ["tGXC...123"]]` |
| `getvalidators` | List all validators | `[]` | `["getvalidators", []]` |
| `getvalidatorinfo` | Get validator info | `[address]` | `["getvalidatorinfo", ["tGXC...123"]]` |

### Blockchain Methods

| Method | Description | Parameters |
|--------|-------------|------------|
| `getblockchaininfo` | Get chain info | `[]` |
| `getblockcount` | Get block count | `[]` |
| `getblock` | Get block | `[height/hash, verbose]` |
| `getlatestblock` | Get latest block | `[]` |
| `getblocktemplate` | Get mining template | `[]` |

### REST API Endpoints (If Available)

**Base URL:** `http://localhost:5000/api/v1` (explorer API)

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/address/{address}/balance` | GET | Get address balance |
| `/address/{address}/transactions` | GET | Get address transactions |
| `/block/{height}` | GET | Get block by height |
| `/transaction/{txid}` | GET | Get transaction details |

---

## 🚀 Getting Started

### Prerequisites

- Node.js 16+ (for frontend build tools)
- Running GXC blockchain node
- Modern web browser
- Code editor (VS Code recommended)

### Project Setup

#### Option 1: React + TypeScript

```bash
# Create React app
npx create-react-app gxc-wallet --template typescript
cd gxc-wallet

# Install dependencies
npm install axios socket.io-client
npm install @mui/material @emotion/react @emotion/styled
npm install @reduxjs/toolkit react-redux

# Start development server
npm start
```

#### Option 2: Vue 3 + TypeScript

```bash
# Create Vue app
npm create vue@latest gxc-wallet
cd gxc-wallet

# Install dependencies
npm install axios socket.io-client
npm install vuex@next
npm install @vueuse/core

# Start development server
npm run dev
```

#### Option 3: Vanilla JavaScript

```bash
# Create project directory
mkdir gxc-wallet
cd gxc-wallet

# Create HTML file
touch index.html
touch app.js
touch styles.css

# Use a simple HTTP server
python3 -m http.server 8080
```

---

## 💻 Frontend Implementation

### 1. RPC Client Service

#### TypeScript/JavaScript

```typescript
// services/rpcClient.ts
import axios, { AxiosInstance } from 'axios';

export interface RPCRequest {
  jsonrpc: string;
  method: string;
  params: any[];
  id: number;
}

export interface RPCResponse<T = any> {
  jsonrpc: string;
  result?: T;
  error?: {
    code: number;
    message: string;
  };
  id: number;
}

export class GXCRPCClient {
  private client: AxiosInstance;
  private requestId: number = 1;

  constructor(rpcUrl: string = 'http://localhost:18332') {
    this.client = axios.create({
      baseURL: rpcUrl,
      headers: {
        'Content-Type': 'application/json',
      },
      timeout: 10000,
    });
  }

  async call<T = any>(method: string, params: any[] = []): Promise<T> {
    const request: RPCRequest = {
      jsonrpc: '2.0',
      method,
      params,
      id: this.requestId++,
    };

    try {
      const response = await this.client.post<RPCResponse<T>>('', request);
      
      if (response.data.error) {
        throw new Error(`RPC Error: ${response.data.error.message}`);
      }

      return response.data.result as T;
    } catch (error: any) {
      if (error.response) {
        throw new Error(`RPC Error: ${error.response.data?.error?.message || error.message}`);
      }
      throw new Error(`Connection error: ${error.message}`);
    }
  }

  // Wallet methods
  async getNewAddress(): Promise<string> {
    return this.call<string>('getnewaddress');
  }

  async getBalance(address: string): Promise<number> {
    return this.call<number>('getbalance', [address]);
  }

  async getTransactions(address: string, limit: number = 100): Promise<any[]> {
    return this.call<any[]>('listtransactions', [address, limit]);
  }

  async sendCoins(fromAddress: string, toAddress: string, amount: number): Promise<any> {
    return this.call('sendtoaddress', [fromAddress, toAddress, amount]);
  }

  async validateAddress(address: string): Promise<boolean> {
    const result = await this.call<any>('validateaddress', [address]);
    return result.isvalid || false;
  }

  async getTransaction(txid: string): Promise<any> {
    return this.call('gettransaction', [txid]);
  }

  // Staking methods
  async registerValidator(
    address: string,
    stakeAmount: number,
    stakingDays: number
  ): Promise<any> {
    if (stakeAmount < 100) {
      throw new Error('Minimum stake is 100 GXC');
    }
    if (stakingDays < 14 || stakingDays > 365) {
      throw new Error('Staking days must be between 14 and 365');
    }
    return this.call('registervalidator', [address, stakeAmount, stakingDays]);
  }

  async addStake(address: string, additionalAmount: number): Promise<any> {
    return this.call('addstake', [address, additionalAmount]);
  }

  async unstake(address: string): Promise<any> {
    return this.call('unstake', [address]);
  }

  async getValidators(): Promise<any[]> {
    return this.call<any[]>('getvalidators');
  }

  async getValidatorInfo(address: string): Promise<any> {
    return this.call('getvalidatorinfo', [address]);
  }

  // Blockchain methods
  async getBlockchainInfo(): Promise<any> {
    return this.call('getblockchaininfo');
  }

  async getBlockCount(): Promise<number> {
    return this.call<number>('getblockcount');
  }

  async getLatestBlock(): Promise<any> {
    return this.call('getlatestblock');
  }
}

// Export singleton instance
export const rpcClient = new GXCRPCClient(
  process.env.REACT_APP_RPC_URL || 'http://localhost:18332'
);
```

### 2. Wallet Store (State Management)

#### Redux (React)

```typescript
// store/walletSlice.ts
import { createSlice, createAsyncThunk, PayloadAction } from '@reduxjs/toolkit';
import { rpcClient } from '../services/rpcClient';

export interface WalletState {
  address: string | null;
  balance: number;
  transactions: any[];
  validators: any[];
  loading: boolean;
  error: string | null;
}

const initialState: WalletState = {
  address: null,
  balance: 0,
  transactions: [],
  validators: [],
  loading: false,
  error: null,
};

// Async thunks
export const generateAddress = createAsyncThunk(
  'wallet/generateAddress',
  async () => {
    return await rpcClient.getNewAddress();
  }
);

export const fetchBalance = createAsyncThunk(
  'wallet/fetchBalance',
  async (address: string) => {
    return await rpcClient.getBalance(address);
  }
);

export const fetchTransactions = createAsyncThunk(
  'wallet/fetchTransactions',
  async (address: string) => {
    return await rpcClient.getTransactions(address);
  }
);

export const sendCoins = createAsyncThunk(
  'wallet/sendCoins',
  async ({ from, to, amount }: { from: string; to: string; amount: number }) => {
    return await rpcClient.sendCoins(from, to, amount);
  }
);

export const fetchValidators = createAsyncThunk(
  'wallet/fetchValidators',
  async () => {
    return await rpcClient.getValidators();
  }
);

const walletSlice = createSlice({
  name: 'wallet',
  initialState,
  reducers: {
    clearError: (state) => {
      state.error = null;
    },
    setAddress: (state, action: PayloadAction<string>) => {
      state.address = action.payload;
    },
  },
  extraReducers: (builder) => {
    builder
      // Generate address
      .addCase(generateAddress.pending, (state) => {
        state.loading = true;
        state.error = null;
      })
      .addCase(generateAddress.fulfilled, (state, action) => {
        state.address = action.payload;
        state.loading = false;
      })
      .addCase(generateAddress.rejected, (state, action) => {
        state.loading = false;
        state.error = action.error.message || 'Failed to generate address';
      })
      // Fetch balance
      .addCase(fetchBalance.fulfilled, (state, action) => {
        state.balance = action.payload;
      })
      // Fetch transactions
      .addCase(fetchTransactions.fulfilled, (state, action) => {
        state.transactions = action.payload;
      })
      // Send coins
      .addCase(sendCoins.pending, (state) => {
        state.loading = true;
        state.error = null;
      })
      .addCase(sendCoins.fulfilled, (state) => {
        state.loading = false;
        // Refresh balance and transactions
      })
      .addCase(sendCoins.rejected, (state, action) => {
        state.loading = false;
        state.error = action.error.message || 'Failed to send coins';
      })
      // Fetch validators
      .addCase(fetchValidators.fulfilled, (state, action) => {
        state.validators = action.payload;
      });
  },
});

export const { clearError, setAddress } = walletSlice.actions;
export default walletSlice.reducer;
```

### 3. React Components

#### Wallet Dashboard Component

```tsx
// components/WalletDashboard.tsx
import React, { useEffect } from 'react';
import { useDispatch, useSelector } from 'react-redux';
import {
  generateAddress,
  fetchBalance,
  fetchTransactions,
  fetchValidators,
} from '../store/walletSlice';
import { AppDispatch, RootState } from '../store';
import {
  Box,
  Card,
  CardContent,
  Typography,
  Button,
  CircularProgress,
  Alert,
  Grid,
} from '@mui/material';

export const WalletDashboard: React.FC = () => {
  const dispatch = useDispatch<AppDispatch>();
  const { address, balance, transactions, validators, loading, error } = useSelector(
    (state: RootState) => state.wallet
  );

  useEffect(() => {
    if (!address) {
      dispatch(generateAddress());
    } else {
      dispatch(fetchBalance(address));
      dispatch(fetchTransactions(address));
      dispatch(fetchValidators());
    }
  }, [dispatch, address]);

  const handleRefresh = () => {
    if (address) {
      dispatch(fetchBalance(address));
      dispatch(fetchTransactions(address));
    }
  };

  return (
    <Box sx={{ p: 3 }}>
      <Typography variant="h4" gutterBottom>
        GXC Wallet
      </Typography>

      {error && (
        <Alert severity="error" sx={{ mb: 2 }}>
          {error}
        </Alert>
      )}

      <Grid container spacing={3}>
        {/* Balance Card */}
        <Grid item xs={12} md={4}>
          <Card>
            <CardContent>
              <Typography variant="h6" color="textSecondary">
                Balance
              </Typography>
              {loading ? (
                <CircularProgress />
              ) : (
                <Typography variant="h3" color="primary">
                  {balance.toFixed(8)} GXC
                </Typography>
              )}
              <Button onClick={handleRefresh} sx={{ mt: 2 }}>
                Refresh
              </Button>
            </CardContent>
          </Card>
        </Grid>

        {/* Address Card */}
        <Grid item xs={12} md={8}>
          <Card>
            <CardContent>
              <Typography variant="h6" color="textSecondary">
                Address
              </Typography>
              <Typography variant="body1" sx={{ wordBreak: 'break-all' }}>
                {address || 'Generating...'}
              </Typography>
            </CardContent>
          </Card>
        </Grid>

        {/* Transactions */}
        <Grid item xs={12}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                Recent Transactions
              </Typography>
              {transactions.length === 0 ? (
                <Typography>No transactions yet</Typography>
              ) : (
                <Box>
                  {transactions.slice(0, 10).map((tx: any) => (
                    <Box key={tx.hash} sx={{ mb: 2, p: 2, border: '1px solid #ddd' }}>
                      <Typography variant="body2">
                        <strong>Hash:</strong> {tx.hash}
                      </Typography>
                      <Typography variant="body2">
                        <strong>Amount:</strong> {tx.amount} GXC
                      </Typography>
                      <Typography variant="body2">
                        <strong>Confirmations:</strong> {tx.confirmations}
                      </Typography>
                    </Box>
                  ))}
                </Box>
              )}
            </CardContent>
          </Card>
        </Grid>

        {/* Validators */}
        <Grid item xs={12}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                Active Validators ({validators.length})
              </Typography>
              {validators.length === 0 ? (
                <Typography>No validators found</Typography>
              ) : (
                <Box>
                  {validators.map((validator: any) => (
                    <Box key={validator.address} sx={{ mb: 2, p: 2, border: '1px solid #ddd' }}>
                      <Typography variant="body2">
                        <strong>Address:</strong> {validator.address}
                      </Typography>
                      <Typography variant="body2">
                        <strong>Stake:</strong> {validator.stake_amount} GXC
                      </Typography>
                      <Typography variant="body2">
                        <strong>Weighted Stake:</strong> {validator.weighted_stake} GXC
                      </Typography>
                    </Box>
                  ))}
                </Box>
              )}
            </CardContent>
          </Card>
        </Grid>
      </Grid>
    </Box>
  );
};
```

#### Send Coins Component

```tsx
// components/SendCoins.tsx
import React, { useState } from 'react';
import { useDispatch, useSelector } from 'react-redux';
import { sendCoins } from '../store/walletSlice';
import { AppDispatch, RootState } from '../store';
import {
  Box,
  Card,
  CardContent,
  TextField,
  Button,
  Typography,
  Alert,
} from '@mui/material';

export const SendCoins: React.FC = () => {
  const dispatch = useDispatch<AppDispatch>();
  const { address, balance } = useSelector((state: RootState) => state.wallet);
  
  const [toAddress, setToAddress] = useState('');
  const [amount, setAmount] = useState('');
  const [error, setError] = useState('');
  const [success, setSuccess] = useState('');

  const handleSend = async () => {
    setError('');
    setSuccess('');

    // Validation
    if (!toAddress) {
      setError('Recipient address is required');
      return;
    }

    const amountNum = parseFloat(amount);
    if (isNaN(amountNum) || amountNum <= 0) {
      setError('Invalid amount');
      return;
    }

    if (amountNum > balance) {
      setError('Insufficient balance');
      return;
    }

    if (!address) {
      setError('No wallet address');
      return;
    }

    try {
      const result = await dispatch(
        sendCoins({ from: address, to: toAddress, amount: amountNum })
      ).unwrap();
      
      setSuccess(`Transaction sent! TXID: ${result.txid}`);
      setToAddress('');
      setAmount('');
    } catch (err: any) {
      setError(err.message || 'Failed to send coins');
    }
  };

  return (
    <Card sx={{ mt: 3 }}>
      <CardContent>
        <Typography variant="h6" gutterBottom>
          Send Coins
        </Typography>

        {error && (
          <Alert severity="error" sx={{ mb: 2 }}>
            {error}
          </Alert>
        )}

        {success && (
          <Alert severity="success" sx={{ mb: 2 }}>
            {success}
          </Alert>
        )}

        <Box sx={{ display: 'flex', flexDirection: 'column', gap: 2 }}>
          <TextField
            label="Recipient Address"
            value={toAddress}
            onChange={(e) => setToAddress(e.target.value)}
            fullWidth
            placeholder="tGXC..."
          />

          <TextField
            label="Amount (GXC)"
            type="number"
            value={amount}
            onChange={(e) => setAmount(e.target.value)}
            fullWidth
            inputProps={{ min: 0, step: 0.00000001 }}
            helperText={`Available: ${balance.toFixed(8)} GXC`}
          />

          <Button
            variant="contained"
            onClick={handleSend}
            disabled={!toAddress || !amount}
            fullWidth
          >
            Send Coins
          </Button>
        </Box>
      </CardContent>
    </Card>
  );
};
```

#### Staking Component

```tsx
// components/Staking.tsx
import React, { useState, useEffect } from 'react';
import { useDispatch, useSelector } from 'react-redux';
import { rpcClient } from '../services/rpcClient';
import { fetchValidators, fetchBalance } from '../store/walletSlice';
import { AppDispatch, RootState } from '../store';
import {
  Box,
  Card,
  CardContent,
  TextField,
  Button,
  Typography,
  Alert,
  Tabs,
  Tab,
} from '@mui/material';

export const Staking: React.FC = () => {
  const dispatch = useDispatch<AppDispatch>();
  const { address, balance, validators } = useSelector((state: RootState) => state.wallet);
  
  const [tab, setTab] = useState(0);
  const [stakeAmount, setStakeAmount] = useState('');
  const [stakingDays, setStakingDays] = useState('90');
  const [additionalStake, setAdditionalStake] = useState('');
  const [validatorInfo, setValidatorInfo] = useState<any>(null);
  const [error, setError] = useState('');
  const [success, setSuccess] = useState('');

  useEffect(() => {
    if (address) {
      loadValidatorInfo();
    }
  }, [address]);

  const loadValidatorInfo = async () => {
    if (!address) return;
    try {
      const info = await rpcClient.getValidatorInfo(address);
      setValidatorInfo(info);
    } catch (err) {
      // Not a validator yet
      setValidatorInfo(null);
    }
  };

  const handleStake = async () => {
    setError('');
    setSuccess('');

    if (!address) {
      setError('No wallet address');
      return;
    }

    const amount = parseFloat(stakeAmount);
    const days = parseInt(stakingDays);

    if (isNaN(amount) || amount < 100) {
      setError('Minimum stake is 100 GXC');
      return;
    }

    if (isNaN(days) || days < 14 || days > 365) {
      setError('Staking days must be between 14 and 365');
      return;
    }

    if (amount > balance) {
      setError('Insufficient balance');
      return;
    }

    try {
      await rpcClient.registerValidator(address, amount, days);
      setSuccess('Successfully staked! You are now a validator.');
      dispatch(fetchValidators());
      dispatch(fetchBalance(address));
      loadValidatorInfo();
      setStakeAmount('');
    } catch (err: any) {
      setError(err.message || 'Failed to stake');
    }
  };

  const handleAddStake = async () => {
    setError('');
    setSuccess('');

    const amount = parseFloat(additionalStake);
    if (isNaN(amount) || amount <= 0) {
      setError('Invalid amount');
      return;
    }

    try {
      await rpcClient.addStake(address!, amount);
      setSuccess('Additional stake added successfully!');
      dispatch(fetchBalance(address!));
      loadValidatorInfo();
      setAdditionalStake('');
    } catch (err: any) {
      setError(err.message || 'Failed to add stake');
    }
  };

  const handleUnstake = async () => {
    if (!window.confirm('Are you sure you want to unstake? This will deactivate your validator.')) {
      return;
    }

    try {
      await rpcClient.unstake(address!);
      setSuccess('Successfully unstaked!');
      dispatch(fetchValidators());
      dispatch(fetchBalance(address!));
      setValidatorInfo(null);
    } catch (err: any) {
      setError(err.message || 'Failed to unstake');
    }
  };

  return (
    <Box sx={{ mt: 3 }}>
      <Card>
        <CardContent>
          <Typography variant="h6" gutterBottom>
            Staking
          </Typography>

          {error && (
            <Alert severity="error" sx={{ mb: 2 }}>
              {error}
            </Alert>
          )}

          {success && (
            <Alert severity="success" sx={{ mb: 2 }}>
              {success}
            </Alert>
          )}

          <Tabs value={tab} onChange={(_, v) => setTab(v)} sx={{ mb: 2 }}>
            <Tab label="Stake" />
            <Tab label="Manage Stake" />
            <Tab label="Validators" />
          </Tabs>

          {/* Stake Tab */}
          {tab === 0 && (
            <Box>
              {validatorInfo ? (
                <Alert severity="info">
                  You are already a validator. Use "Manage Stake" tab to add more stake or unstake.
                </Alert>
              ) : (
                <Box sx={{ display: 'flex', flexDirection: 'column', gap: 2 }}>
                  <TextField
                    label="Stake Amount (GXC)"
                    type="number"
                    value={stakeAmount}
                    onChange={(e) => setStakeAmount(e.target.value)}
                    fullWidth
                    inputProps={{ min: 100 }}
                    helperText="Minimum: 100 GXC"
                  />

                  <TextField
                    label="Staking Days"
                    type="number"
                    value={stakingDays}
                    onChange={(e) => setStakingDays(e.target.value)}
                    fullWidth
                    inputProps={{ min: 14, max: 365 }}
                    helperText="14-365 days"
                  />

                  <Button variant="contained" onClick={handleStake} fullWidth>
                    Stake Coins
                  </Button>
                </Box>
              )}
            </Box>
          )}

          {/* Manage Stake Tab */}
          {tab === 1 && (
            <Box>
              {validatorInfo ? (
                <Box>
                  <Typography variant="body1" gutterBottom>
                    <strong>Current Stake:</strong> {validatorInfo.stake_amount} GXC
                  </Typography>
                  <Typography variant="body1" gutterBottom>
                    <strong>Weighted Stake:</strong> {validatorInfo.weighted_stake} GXC
                  </Typography>
                  <Typography variant="body1" gutterBottom>
                    <strong>Staking Days:</strong> {validatorInfo.staking_days}
                  </Typography>

                  <Box sx={{ mt: 3, display: 'flex', flexDirection: 'column', gap: 2 }}>
                    <TextField
                      label="Additional Stake (GXC)"
                      type="number"
                      value={additionalStake}
                      onChange={(e) => setAdditionalStake(e.target.value)}
                      fullWidth
                    />

                    <Button variant="contained" onClick={handleAddStake} fullWidth>
                      Add Stake
                    </Button>

                    <Button variant="outlined" color="error" onClick={handleUnstake} fullWidth>
                      Unstake All
                    </Button>
                  </Box>
                </Box>
              ) : (
                <Alert severity="info">
                  You are not a validator yet. Use the "Stake" tab to become a validator.
                </Alert>
              )}
            </Box>
          )}

          {/* Validators Tab */}
          {tab === 2 && (
            <Box>
              <Typography variant="h6" gutterBottom>
                All Validators ({validators.length})
              </Typography>
              {validators.map((v: any) => (
                <Card key={v.address} sx={{ mb: 2 }}>
                  <CardContent>
                    <Typography variant="body2">
                      <strong>Address:</strong> {v.address}
                    </Typography>
                    <Typography variant="body2">
                      <strong>Stake:</strong> {v.stake_amount} GXC
                    </Typography>
                    <Typography variant="body2">
                      <strong>Weighted Stake:</strong> {v.weighted_stake} GXC
                    </Typography>
                  </CardContent>
                </Card>
              ))}
            </Box>
          )}
        </CardContent>
      </Card>
    </Box>
  );
};
```

---

## 🔄 Real-time Updates

### WebSocket Integration

```typescript
// services/websocketClient.ts
import { io, Socket } from 'socket.io-client';

export class WebSocketClient {
  private socket: Socket | null = null;

  connect(url: string = 'http://localhost:5000') {
    this.socket = io(url, {
      transports: ['websocket'],
      reconnection: true,
      reconnectionDelay: 1000,
    });

    this.socket.on('connect', () => {
      console.log('WebSocket connected');
    });

    this.socket.on('disconnect', () => {
      console.log('WebSocket disconnected');
    });

    return this.socket;
  }

  subscribeToAddress(address: string, callback: (data: any) => void) {
    if (!this.socket) return;

    this.socket.emit('subscribe_address', address);
    this.socket.on('address_update', callback);
  }

  subscribeToBlockchain(callback: (data: any) => void) {
    if (!this.socket) return;

    this.socket.on('new_block', callback);
    this.socket.on('new_transaction', callback);
  }

  disconnect() {
    if (this.socket) {
      this.socket.disconnect();
      this.socket = null;
    }
  }
}

export const wsClient = new WebSocketClient();
```

### Using WebSocket in React

```tsx
// hooks/useWebSocket.ts
import { useEffect } from 'react';
import { useDispatch } from 'react-redux';
import { wsClient } from '../services/websocketClient';
import { fetchBalance, fetchTransactions } from '../store/walletSlice';

export const useWebSocket = (address: string | null) => {
  const dispatch = useDispatch();

  useEffect(() => {
    if (!address) return;

    wsClient.connect();
    
    // Subscribe to address updates
    wsClient.subscribeToAddress(address, (data) => {
      if (data.type === 'balance_update') {
        dispatch(fetchBalance(address));
      }
      if (data.type === 'new_transaction') {
        dispatch(fetchTransactions(address));
      }
    });

    // Subscribe to blockchain updates
    wsClient.subscribeToBlockchain((data) => {
      console.log('Blockchain update:', data);
    });

    return () => {
      wsClient.disconnect();
    };
  }, [address, dispatch]);
};
```

---

## 🔒 Security

### 1. Environment Variables

```bash
# .env
REACT_APP_RPC_URL=http://localhost:18332
REACT_APP_WS_URL=http://localhost:5000
REACT_APP_NETWORK=testnet
```

### 2. Input Validation

```typescript
// utils/validation.ts
export const validateAddress = (address: string): boolean => {
  // GXC addresses start with GXC or tGXC
  return /^[t]?GXC[a-zA-Z0-9]{30,}$/.test(address);
};

export const validateAmount = (amount: number): boolean => {
  return amount > 0 && amount <= Number.MAX_SAFE_INTEGER;
};
```

### 3. Error Handling

```typescript
// utils/errorHandler.ts
export const handleRPCError = (error: any): string => {
  if (error.response?.data?.error) {
    return error.response.data.error.message;
  }
  if (error.message) {
    return error.message;
  }
  return 'An unexpected error occurred';
};
```

### 4. Rate Limiting

```typescript
// utils/rateLimiter.ts
class RateLimiter {
  private requests: Map<string, number[]> = new Map();
  private maxRequests: number;
  private windowMs: number;

  constructor(maxRequests: number = 10, windowMs: number = 1000) {
    this.maxRequests = maxRequests;
    this.windowMs = windowMs;
  }

  canMakeRequest(key: string): boolean {
    const now = Date.now();
    const requests = this.requests.get(key) || [];
    
    // Remove old requests
    const recentRequests = requests.filter(
      (time) => now - time < this.windowMs
    );

    if (recentRequests.length >= this.maxRequests) {
      return false;
    }

    recentRequests.push(now);
    this.requests.set(key, recentRequests);
    return true;
  }
}

export const rateLimiter = new RateLimiter(10, 1000);
```

---

## 🎨 UI/UX Best Practices

### 1. Loading States

```tsx
{loading ? (
  <Box sx={{ display: 'flex', justifyContent: 'center', p: 3 }}>
    <CircularProgress />
  </Box>
) : (
  <Content />
)}
```

### 2. Error Boundaries

```tsx
// components/ErrorBoundary.tsx
import React, { Component, ErrorInfo, ReactNode } from 'react';
import { Alert } from '@mui/material';

interface Props {
  children: ReactNode;
}

interface State {
  hasError: boolean;
  error: Error | null;
}

export class ErrorBoundary extends Component<Props, State> {
  public state: State = {
    hasError: false,
    error: null,
  };

  public static getDerivedStateFromError(error: Error): State {
    return { hasError: true, error };
  }

  public componentDidCatch(error: Error, errorInfo: ErrorInfo) {
    console.error('Uncaught error:', error, errorInfo);
  }

  public render() {
    if (this.state.hasError) {
      return (
        <Alert severity="error">
          Something went wrong: {this.state.error?.message}
        </Alert>
      );
    }

    return this.props.children;
  }
}
```

### 3. Responsive Design

```tsx
// Use Material-UI Grid for responsive layout
<Grid container spacing={3}>
  <Grid item xs={12} sm={6} md={4}>
    <Card>...</Card>
  </Grid>
</Grid>
```

### 4. Dark Mode Support

```tsx
// theme.ts
import { createTheme } from '@mui/material/styles';

export const lightTheme = createTheme({
  palette: {
    mode: 'light',
  },
});

export const darkTheme = createTheme({
  palette: {
    mode: 'dark',
  },
});
```

---

## 🧪 Testing

### Unit Tests

```typescript
// __tests__/rpcClient.test.ts
import { GXCRPCClient } from '../services/rpcClient';

describe('GXCRPCClient', () => {
  let client: GXCRPCClient;

  beforeEach(() => {
    client = new GXCRPCClient('http://localhost:18332');
  });

  test('should generate address', async () => {
    const address = await client.getNewAddress();
    expect(address).toMatch(/^[t]?GXC/);
  });

  test('should get balance', async () => {
    const balance = await client.getBalance('tGXC123...');
    expect(typeof balance).toBe('number');
  });
});
```

---

## 📦 Deployment

### Build for Production

```bash
# React
npm run build

# Vue
npm run build

# The build folder contains optimized production files
```

### Environment Configuration

```bash
# Production .env
REACT_APP_RPC_URL=https://your-node.com:18332
REACT_APP_WS_URL=https://your-websocket-server.com
REACT_APP_NETWORK=mainnet
```

### Docker Deployment

```dockerfile
# Dockerfile
FROM node:18-alpine AS builder
WORKDIR /app
COPY package*.json ./
RUN npm ci
COPY . .
RUN npm run build

FROM nginx:alpine
COPY --from=builder /app/build /usr/share/nginx/html
COPY nginx.conf /etc/nginx/nginx.conf
EXPOSE 80
CMD ["nginx", "-g", "daemon off;"]
```

---

## 📚 Complete Example: Full React App

See the complete example implementation in the `examples/` directory:

- `react-wallet/` - Complete React + TypeScript wallet
- `vue-wallet/` - Complete Vue 3 wallet
- `vanilla-js/` - Vanilla JavaScript implementation

---

## 🆘 Troubleshooting

### Connection Issues
- Check node is running
- Verify RPC port (18332 for testnet)
- Check CORS settings
- Verify firewall rules

### Balance Not Updating
- Wait for confirmations (6+ blocks)
- Refresh manually
- Check WebSocket connection
- Verify address is correct

### Staking Errors
- Ensure balance >= 100 GXC
- Check staking days (14-365)
- Verify address format
- Check if already validator

---

## 📝 Summary

This guide provides:

✅ **Complete API Reference** - All RPC and REST endpoints  
✅ **Modern Code Examples** - React, Vue, TypeScript  
✅ **Real-time Updates** - WebSocket integration  
✅ **Security Best Practices** - Production-ready security  
✅ **UI/UX Guidelines** - Best practices for wallet apps  
✅ **Testing & Deployment** - Complete workflow  

Start building your GXC wallet app today! 🚀
