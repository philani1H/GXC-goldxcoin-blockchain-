# 🪟 GXC Blockchain - Windows Quick Start

## ✅ No Building Required on Windows!

Everything runs on Python - no C++ compilation needed.

---

## 🚀 Quick Start (3 Steps)

### Step 1: Install Python

Download and install Python 3.8+ from [python.org](https://www.python.org/downloads/)

Or use winget:
```powershell
winget install Python.Python.3.11
```

### Step 2: Clone and Setup

```powershell
# Clone repository
git clone https://github.com/YOUR_USERNAME/GXC-goldxcoin-blockchain-.git
cd GXC-goldxcoin-blockchain-

# Install dependencies
pip install -r requirements.txt
```

### Step 3: Start Everything

```powershell
python start_all_services.py
```

**Done!** 🎉

---

## 🌐 Access Your Blockchain

After starting services:

- **Explorer:** http://localhost:3000
- **Wallet API:** http://localhost:5000
- **Mining Pool:** http://localhost:6000
- **Forum:** http://localhost:3001

---

## ⛏️ Start Mining

```powershell
cd mining
python gxhash_miner.py --pool localhost:3333 --address YOUR_GXC_ADDRESS
```

---

## 🛑 Stop Services

```powershell
python stop_all_services.py
```

---

## ❌ Don't Build C++ Code!

If you see CMake errors, **ignore them**. You don't need to build anything.

### Fix CMake Error

```powershell
# Delete build directory
Remove-Item -Recurse -Force build/
```

**Then just use Python:**
```powershell
python start_all_services.py
```

---

## 📋 What You Need

### Required ✅
- Python 3.8+
- pip
- Git

### NOT Required ❌
- Visual Studio
- CMake
- C++ Build Tools
- Any compilers

---

## 🎯 Common Commands

```powershell
# Start everything
python start_all_services.py

# Stop everything
python stop_all_services.py

# Start mining
cd mining
python gxhash_miner.py --pool localhost:3333 --address YOUR_ADDRESS

# Check if services are running
netstat -ano | findstr :3000
netstat -ano | findstr :5000
netstat -ano | findstr :6000
```

---

## 🐛 Troubleshooting

### "Python not found"
```powershell
# Install Python
winget install Python.Python.3.11

# Or download from python.org
```

### "Module not found"
```powershell
pip install -r requirements.txt
```

### "Port already in use"
```powershell
# Stop services
python stop_all_services.py

# Or kill specific port
netstat -ano | findstr :3000
taskkill /PID <process_id> /F
```

### "CMake error"
**Solution:** Ignore it! You don't need to build.
```powershell
# Just run Python
python start_all_services.py
```

---

## 📚 Documentation

- **Windows Setup:** `WINDOWS_SETUP.md`
- **Complete Guide:** `COMPLETE_SETUP_GUIDE.md`
- **Deployment:** `DEPLOYMENT_GUIDE.md`

---

## 🎊 Summary

**On Windows:**
1. Install Python
2. Clone repository
3. Run `python start_all_services.py`

**No building, no errors, just works!** ✅

---

## 📞 Need Help?

See `WINDOWS_SETUP.md` for detailed instructions.

**Remember: You don't need to build anything on Windows!**
