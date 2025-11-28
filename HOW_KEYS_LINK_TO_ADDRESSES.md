# How Private Keys and Mnemonics Link to Addresses

## The Mathematical Chain: Mnemonic → Private Key → Public Key → Address

### Overview

The relationship between mnemonics, private keys, and addresses is **mathematical and deterministic**. This means:
- **Same input = Same output (always)**
- **Any app can verify the link** by performing the same calculations
- **No blockchain needed** to verify the relationship

---

## The Derivation Chain

```
Mnemonic (12 words)
    ↓
    [BIP39 Derivation]
    ↓
Private Key (ED25519)
    ↓
    [Cryptographic Derivation]
    ↓
Public Key (ED25519)
    ↓
    [Hash Function]
    ↓
Address (GXC...)
```

---

## Step-by-Step: How It Works

### 1. Mnemonic → Private Key

**Process: BIP39 Seed Generation**

```python
# Simplified explanation
mnemonic = "word1 word2 word3 ... word12"
seed = PBKDF2(mnemonic, passphrase="", iterations=2048)
private_key = derive_from_seed(seed)
```

**Key Points:**
- ✅ **Deterministic**: Same mnemonic → Same private key (always)
- ✅ **Standard**: BIP39 is an industry standard
- ✅ **Any app can do this**: Standard algorithm, publicly documented

**Example:**
```
Mnemonic: "abandon abandon abandon ... abandon"
    ↓
Private Key: "-----BEGIN PRIVATE KEY-----\n..."
```

---

### 2. Private Key → Public Key

**Process: ED25519 Key Derivation**

```python
# Cryptographic derivation (one-way function)
private_key = ED25519PrivateKey.generate()  # or from mnemonic
public_key = private_key.public_key()
```

**Key Points:**
- ✅ **Mathematical**: Public key is mathematically derived from private key
- ✅ **One-way**: Can't reverse (get private key from public key)
- ✅ **Deterministic**: Same private key → Same public key (always)
- ✅ **Standard**: ED25519 is a standard cryptographic algorithm

**Example:**
```
Private Key: "-----BEGIN PRIVATE KEY-----\n..."
    ↓
Public Key: "-----BEGIN PUBLIC KEY-----\n..."
```

**How Third-Party Apps Verify:**
1. You provide: Private Key
2. App derives: Public Key
3. App compares: Derived public key matches the one associated with the address

---

### 3. Public Key → Address

**Process: Hash-Based Address Generation**

```python
# Our implementation (from wallet_service.py)
def generate_address(self, public_key_pem):
    # Hash the public key
    public_key_bytes = public_key_pem.encode()
    hash1 = hashlib.sha256(public_key_bytes).digest()
    hash2 = hashlib.sha256(hash1).digest()
    
    # Take first 20 bytes and add GXC prefix
    address_bytes = hash2[:20]
    address = 'GXC' + address_bytes.hex()
    
    return address
```

**Key Points:**
- ✅ **Deterministic**: Same public key → Same address (always)
- ✅ **Hash function**: SHA256 (standard, one-way)
- ✅ **Any app can verify**: Standard hash function, publicly documented

**Example:**
```
Public Key: "-----BEGIN PUBLIC KEY-----\n..."
    ↓
Hash1 = SHA256(public_key)
Hash2 = SHA256(Hash1)
Address = "GXC" + Hash2[0:20].hex()
    ↓
Address: "GXC1a2b3c4d5e6f7g8h9i0j..."
```

---

## How Third-Party Apps Verify Ownership

### Scenario: You Want to Import Your Wallet

**What You Provide:**
- Option 1: Private Key
- Option 2: Mnemonic (12 words)

**What the App Does:**

#### If You Provide Private Key:

```python
# 1. Derive public key from private key
public_key = derive_public_key(private_key)

# 2. Derive address from public key
address = hash_public_key(public_key)

# 3. Verify: Does this address match?
if address == claimed_address:
    # ✅ Private key is valid for this address
    # User can access the wallet
else:
    # ❌ Private key doesn't match address
    # Reject access
```

#### If You Provide Mnemonic:

```python
# 1. Derive private key from mnemonic (BIP39)
private_key = derive_from_mnemonic(mnemonic)

# 2. Derive public key from private key
public_key = derive_public_key(private_key)

# 3. Derive address from public key
address = hash_public_key(public_key)

# 4. Verify: Does this address match?
if address == claimed_address:
    # ✅ Mnemonic is valid for this address
    # User can access the wallet
else:
    # ❌ Mnemonic doesn't match address
    # Reject access
```

---

## Real Example: Complete Chain

### Starting Point: Mnemonic

```
Mnemonic: "abandon ability able about above absent absorb abstract absurd abuse access accident"
```

### Step 1: Mnemonic → Private Key

```python
# BIP39 derivation
seed = PBKDF2(mnemonic, salt="mnemonic", iterations=2048)
private_key = ED25519PrivateKey.from_seed(seed)
```

**Result:**
```
Private Key (PEM):
-----BEGIN PRIVATE KEY-----
MC4CAQAwBQYDK2VwBCIEI...
-----END PRIVATE KEY-----
```

### Step 2: Private Key → Public Key

```python
# ED25519 derivation
public_key = private_key.public_key()
```

**Result:**
```
Public Key (PEM):
-----BEGIN PUBLIC KEY-----
MCowBQYDK2VwAyEA...
-----END PUBLIC KEY-----
```

### Step 3: Public Key → Address

```python
# Hash-based derivation
hash1 = SHA256(public_key_bytes)
hash2 = SHA256(hash1)
address = "GXC" + hash2[0:20].hex()
```

**Result:**
```
Address: GXC1a2b3c4d5e6f7g8h9i0j1k2l3m4n5o6p7q8r9s0t
```

---

## Why This Works Across All Apps

### Standard Algorithms

All apps use the **same standard algorithms**:

1. **BIP39**: Mnemonic → Seed → Private Key
   - Industry standard
   - Same algorithm everywhere
   - Publicly documented

2. **ED25519**: Private Key → Public Key
   - Standard cryptographic curve
   - Same algorithm everywhere
   - Publicly documented

3. **SHA256**: Public Key → Address
   - Standard hash function
   - Same algorithm everywhere
   - Publicly documented

### Deterministic Process

Because the process is **deterministic**:
- ✅ Same mnemonic → Same private key (always)
- ✅ Same private key → Same public key (always)
- ✅ Same public key → Same address (always)

**Result:** Any app can verify the link by performing the same calculations.

---

## How Apps Know the Link is Valid

### Verification Process

When you import a wallet in a third-party app:

1. **You provide**: Mnemonic OR Private Key
2. **App derives**: Address using standard algorithms
3. **App checks**: Does derived address match your claimed address?
4. **If match**: ✅ Valid - grant access
5. **If no match**: ❌ Invalid - reject access

### No Blockchain Needed

**Important:** This verification happens **offline**:
- ✅ No blockchain connection needed
- ✅ Pure mathematics
- ✅ Instant verification
- ✅ Works in any app

The blockchain is only needed to:
- Check balance
- Send/receive transactions
- Query transaction history

---

## Security: Why This is Safe

### One-Way Functions

1. **Private Key → Public Key**: One-way (can't reverse)
2. **Public Key → Address**: One-way (can't reverse)
3. **Mnemonic → Private Key**: One-way (can't reverse)

**Result:**
- ✅ Can derive address from private key
- ❌ Can't derive private key from address
- ✅ Safe to share address publicly
- ❌ Never share private key or mnemonic

### Mathematical Proof

The link is **mathematically provable**:
- Anyone can verify: `address == hash(public_key)`
- Anyone can verify: `public_key == derive(private_key)`
- Anyone can verify: `private_key == derive(mnemonic)`

**But:**
- ❌ Can't reverse: `private_key != reverse_hash(address)`
- ❌ Can't reverse: `mnemonic != reverse_derive(private_key)`

---

## Summary

### The Chain

```
Mnemonic (12 words)
    ↓ [BIP39 - Standard]
Private Key (ED25519)
    ↓ [Cryptography - Standard]
Public Key (ED25519)
    ↓ [SHA256 - Standard]
Address (GXC...)
```

### How Third-Party Apps Verify

1. **You provide**: Mnemonic or Private Key
2. **App derives**: Address using standard algorithms
3. **App verifies**: Derived address matches claimed address
4. **Result**: ✅ Valid → Access granted

### Key Points

- ✅ **Deterministic**: Same input → Same output (always)
- ✅ **Standard**: All apps use same algorithms
- ✅ **Offline**: No blockchain needed to verify
- ✅ **Mathematical**: Pure cryptography
- ✅ **Secure**: One-way functions prevent reversal

---

## Example: Importing to Third-Party App

### What Happens:

1. **You**: Enter mnemonic "word1 word2 ... word12"
2. **App**: Derives private key using BIP39
3. **App**: Derives public key using ED25519
4. **App**: Derives address using SHA256
5. **App**: Checks if address matches your wallet
6. **Result**: ✅ Match → Wallet imported successfully

### Why It Works:

- All apps use the **same algorithms**
- The process is **deterministic**
- The link is **mathematically provable**
- No blockchain needed to verify

---

**Bottom Line**: The link between mnemonic/private key and address is **mathematical and deterministic**. Any app can verify it by performing the same standard calculations. This is how all blockchain wallets work - it's standard cryptography, not blockchain magic!

