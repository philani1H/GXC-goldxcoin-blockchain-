# 📜 GXC Command-Line Cheat Sheet

> Quick reference for **every day-to-day command** you’re likely to use while operating a GXC node, wallet, miner, or developer stack.  
> (Works on Linux/macOS; replace forward-slashes with back-slashes and `.sh` with `.bat` on Windows.)

---
## 1  Node daemon (`gxc-node`)
| Action | Command |
|--------|---------|
| Start in background | `gxc-node -daemon` |
| Start with explicit datadir | `gxc-node -datadir=/srv/gxc -conf=/srv/gxc/gxc.conf -daemon` |
| Stop gracefully | `gxc-cli stop` |
| View live log | `tail -F ~/.gxc/debug.log` |
| Check peer info | `gxc-cli getpeerinfo` |
| Network hash rate | `gxc-cli getnetworkhashps` |
| Block height / difficulty | `gxc-cli getblockchaininfo` |
| Rescan chainstate | `gxc-cli rescanblockchain` |
| Reindex from genesis | `gxc-node -reindex -daemon` |

---
## 2  Wallet commands (`gxc-cli`)
| Task | Command |
|------|---------|
| Create new wallet | `gxc-cli createwallet MyWallet` |
| List wallets | `gxc-cli listwallets` |
| Load wallet | `gxc-cli loadwallet MyWallet` |
| Display seed (first run only) | Shown interactively when `createwallet` finishes |
| Generate new receive address | `gxc-cli getnewaddress` |
| Show balance | `gxc-cli getbalance` |
| Send coins | `gxc-cli sendtoaddress <GXCaddr> 100 "memo"` |
| Unlock wallet 1 h | `gxc-cli walletpassphrase <password> 3600` |
| Dump raw key | `gxc-cli dumpprivkey <GXCaddr>` |
| Import raw key | `gxc-cli importprivkey <L1aW4…>` |
| Stake 500 GXC 90 days | `gxc-cli stake 500 90` |

---
## 3  Mining
### 3.1 Solo (SHA-256 ASIC)
```bash
# On the miner
miner.exe -o stratum+tcp://NODE_IP:4334 \
          -u GXCADDRESS.worker  -p x
```
### 3.2 Solo (Ethash GPU)
```bash
phoenixminer -pool stratum+tcp://NODE_IP:4335 \
             -wal GXCADDRESS.worker  -proto 4
```
### 3.3 Pool
```bash
miner.exe -o stratum+tcp://pool.gxc.network:4334 \
          -u GXCADDRESS.worker  -p x
```

---
## 4  Miner-Manager service
| Action | Command |
|--------|---------|
| Start service | `python miner_manager/manage_miner.py` |
| Query config  | `curl http://127.0.0.1:5055/config` |
| Change payout addr | `curl -X POST http://127.0.0.1:5055/config -d '{"address":"GXC…"}'` |
| Check running? | `curl http://127.0.0.1:5055/status` |

---
## 5  Wallet REST API (Flask)
| Endpoint | Example |
|----------|---------|
| Register | `POST /api/v1/register {username,email,password}` |
| Login    | `POST /api/v1/login` → returns JWT |
| Create wallet | `POST /api/v1/wallets/create` (Auth: `Bearer <token>`) |
| List wallets  | `GET  /api/v1/wallets` |

---
## 6  Explorer / Indexer
| Service | How to run |
|---------|-----------|
| Indexer | `python web/indexer.py` |
| Explorer GUI | `python web/blockchain_explorer.py` → browse `http://localhost:3000` |

---
## 7  Docker Compose one-liner (full stack)
```bash
git clone https://github.com/philchade/gxc-blockchain.git
cd gxc-blockchain
docker compose up -d      # node + explorer + wallet-api + market-maker
```

Ports:
* 9735 – P2P   • 8545 – RPC   • 4334/4335 – Solo-stratum   • 3000 – Explorer GUI  
* 4000 – Market-Maker   • 5000 – Wallet API   • 5055 – Miner-Manager

---
## 8  Node maintenance & troubleshooting
| Problem | Quick fix |
|---------|-----------|
| Node stuck on old block | `gxc-node -reindex-chainstate` |
| Wallet “insufficient funds” | `loadwallet` correct wallet; check `getbalance` |
| Can’t connect peers | Open port 9735 on firewall, verify `port=` in conf |
| RPC 403 error | Check `rpcallowip` in `gxc.conf` or use SSH tunnel |
| Miner shares rejected | Verify algo/port and payout address string |

---
## 9  Windows PowerShell equivalents
| Linux / macOS | Windows PowerShell |
|---------------|--------------------|
| `tail -F debug.log` | `Get-Content debug.log -Wait` |
| `chmod 600 gxc.conf` | `icacls gxc.conf /inheritance:r /grant:r "$env:UserName`:R`"` |
| `./gxc-node -daemon` | `Start-Process gxc-node.exe -ArgumentList "-daemon"` |

---
## 10  Version / build info
```bash
gxc-cli getinfo        # node version & stats
gxc-cli verifymessage  # sign/verify helper
./configure && make    # C++ build (Unix)
cmake .. && msbuild    # C++ build (Windows MSVC)
```

Store this file in `docs/command_cheat_sheet.md` and you’ll always have the essential syntax at hand.