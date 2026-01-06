# Node Wallet Persistence

## Problem

Railway (and other cloud platforms) use ephemeral filesystems. When the container restarts, the wallet file is lost and a new wallet is created with a different address.

## Solution

The node wallet can now be persisted using an environment variable.

## How It Works

The node checks for wallet persistence in this order:

1. **Environment Variable** (Recommended for cloud deployments)
   - Checks `GXC_WALLET_PRIVATE_KEY` environment variable
   - If set, imports the private key
   - Wallet address stays consistent across restarts

2. **File System** (For local/persistent storage)
   - Checks `./wallet/wallet.dat` file
   - If exists, loads the wallet
   - Works for local deployments with persistent storage

3. **Create New** (Fallback)
   - If neither exists, creates a new wallet
   - Saves to file (but won't persist on ephemeral filesystem)
   - Logs warning about persistence

## Setup for Railway

### Step 1: Get Your Wallet Private Key

When the node starts for the first time, check the logs:

```
Creating new node wallet: tGXC3faf86b6e70fe24ddab8cbbde4e8b93ef9
⚠️  IMPORTANT: To persist this wallet across restarts:
   1. Check wallet file: ./wallet/wallet.dat
   2. Set GXC_WALLET_PRIVATE_KEY environment variable
   3. Or configure persistent volume
```

### Step 2: Extract Private Key from Wallet File

If you have access to the wallet file:

```bash
# The wallet.dat file contains:
# Line 1: Private Key
# Line 2: Public Key
# Line 3: Address
# Line 4: Last TX Hash

# Get the private key (first line)
head -n 1 ./wallet/wallet.dat
```

### Step 3: Set Environment Variable in Railway

1. Go to your Railway project
2. Click on your service
3. Go to **Variables** tab
4. Add new variable:
   - **Name**: `GXC_WALLET_PRIVATE_KEY`
   - **Value**: `your_private_key_here`
5. Click **Add**
6. Redeploy the service

### Step 4: Verify

After redeployment, check the logs:

```
✅ Loaded node wallet from environment: tGXC3faf86b6e70fe24ddab8cbbde4e8b93ef9
```

The address should now stay consistent across restarts!

## Setup for Other Platforms

### Docker

```bash
docker run -e GXC_WALLET_PRIVATE_KEY="your_key" gxc-node
```

### Docker Compose

```yaml
services:
  gxc-node:
    image: gxc-node
    environment:
      - GXC_WALLET_PRIVATE_KEY=your_key
```

### Kubernetes

```yaml
apiVersion: v1
kind: Secret
metadata:
  name: gxc-wallet
type: Opaque
stringData:
  private-key: your_key
---
apiVersion: v1
kind: Pod
metadata:
  name: gxc-node
spec:
  containers:
  - name: gxc-node
    image: gxc-node
    env:
    - name: GXC_WALLET_PRIVATE_KEY
      valueFrom:
        secretKeyRef:
          name: gxc-wallet
          key: private-key
```

### Local Development

For local development with persistent storage, no environment variable is needed. The wallet file will persist automatically.

## Security Considerations

### ⚠️ Important

- **Never commit private keys to git**
- **Use secrets management** for production
- **Rotate keys** if compromised
- **Backup wallet file** before setting environment variable

### Best Practices

1. **Use Railway Secrets**: Environment variables in Railway are encrypted
2. **Limit Access**: Only authorized team members should see the private key
3. **Monitor Usage**: Check wallet transactions regularly
4. **Separate Wallets**: Use different wallets for testnet and mainnet

## Troubleshooting

### Wallet Address Keeps Changing

**Cause**: Environment variable not set or incorrect

**Solution**:
1. Verify `GXC_WALLET_PRIVATE_KEY` is set in Railway
2. Check the private key format (should be hex string)
3. Redeploy after setting the variable

### "Failed to import wallet from environment variable"

**Cause**: Invalid private key format

**Solution**:
1. Verify the private key is a valid hex string
2. Check for extra spaces or newlines
3. Ensure you copied the entire key

### Wallet File Not Found

**Cause**: Ephemeral filesystem (normal on Railway)

**Solution**:
- This is expected on cloud platforms
- Use environment variable instead
- Or configure persistent volume (advanced)

## Migration Guide

### From File-Based to Environment Variable

1. **Before Migration**: Note your current wallet address
2. **Extract Key**: Get private key from wallet.dat (line 1)
3. **Set Variable**: Add `GXC_WALLET_PRIVATE_KEY` in Railway
4. **Redeploy**: Trigger a new deployment
5. **Verify**: Check logs show same wallet address

### From Old Wallet to New Wallet

If you want to use a different wallet:

1. **Generate New**: Let node create a new wallet
2. **Extract Key**: Get private key from logs or file
3. **Transfer Funds**: Send coins from old wallet to new
4. **Set Variable**: Use new private key
5. **Redeploy**: New wallet will be used

## Code Reference

**Location**: `src/RPCAPI.cpp:23-52`

```cpp
// Helper function to initialize wallet with persistence
static void initializeWallet(std::unique_ptr<Wallet>& wallet) {
    wallet = std::make_unique<Wallet>();
    
    // Check for wallet private key in environment variable
    const char* envPrivateKey = std::getenv("GXC_WALLET_PRIVATE_KEY");
    if (envPrivateKey && strlen(envPrivateKey) > 0) {
        if (wallet->importPrivateKey(envPrivateKey)) {
            LOG_API(LogLevel::INFO, "✅ Loaded node wallet from environment");
            return;
        }
    }
    
    // Fall back to file-based wallet
    // ...
}
```

## FAQ

**Q: Is it safe to store private key in environment variable?**  
A: Yes, Railway encrypts environment variables. It's safer than storing in code or public repos.

**Q: Can I use the same wallet for multiple nodes?**  
A: Technically yes, but not recommended. Each node should have its own wallet.

**Q: What happens if I lose the private key?**  
A: You lose access to the wallet and its funds. Always backup the private key securely.

**Q: Can I change the wallet address?**  
A: Yes, just set a different private key in the environment variable and redeploy.

**Q: Do I need to set this for local development?**  
A: No, local development uses file-based persistence automatically.

## Summary

✅ **Problem Solved**: Node wallet now persists across Railway restarts  
✅ **Method**: Environment variable `GXC_WALLET_PRIVATE_KEY`  
✅ **Benefit**: Consistent wallet address, no fund loss on restart  
✅ **Security**: Encrypted in Railway, never in code  

Set the environment variable once, and your wallet will persist forever!
