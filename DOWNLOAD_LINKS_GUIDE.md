# 📥 GXC Miner Download Links Guide

## ✅ All Download Links Updated!

All HTML files now point to the correct local download API endpoints.

---

## 🔗 Download URLs

### Main Download Page
**URL:** http://localhost:3000/downloads

This is the main page where users can:
- See all available miners
- Read installation instructions
- Get pool configuration info
- View system requirements

### Direct Download API Endpoints

#### Windows Miners
```
SHA256 Miner:  http://localhost:3000/api/downloads/windows/sha256
Ethash Miner:  http://localhost:3000/api/downloads/windows/ethash
GXHash Miner:  http://localhost:3000/api/downloads/windows/gxhash
```

#### Linux Miners
```
SHA256 Miner:  http://localhost:3000/api/downloads/linux/sha256
Ethash Miner:  http://localhost:3000/api/downloads/linux/ethash
GXHash Miner:  http://localhost:3000/api/downloads/linux/gxhash
```

---

## 📍 Where to Find Download Links

### 1. Main Navigation (Explorer)
**Location:** Top navigation bar on http://localhost:3000

Click: **"Download Miners"** → Goes to `/downloads`

### 2. Mining Guide Page
**Location:** http://localhost:3000/mining-guide

Features:
- Step-by-step mining instructions
- Direct download buttons for each miner
- Platform-specific guides
- Pool configuration examples

### 3. Mining Page
**Location:** http://localhost:3000/mining

Features:
- Mining overview
- Download links for all miners
- Getting started guide

### 4. Pool Dashboard
**Location:** http://localhost:6000 (General Pool)

Features:
- "Download Miners" button in Quick Start section
- Links to `/downloads` page
- Pool connection instructions

### 5. Downloads Page
**Location:** http://localhost:3000/downloads

Features:
- Professional download interface
- All miners listed with descriptions
- Installation instructions
- System requirements
- Pool configuration guide

---

## 🎯 How Users Download Miners

### Method 1: Via Download Page (Recommended)

1. Visit http://localhost:3000
2. Click "Download Miners" in navigation
3. Choose platform (Windows/Linux)
4. Choose algorithm (SHA256/Ethash/GXHash)
5. Click download button
6. Follow installation instructions

### Method 2: Direct API Link

```bash
# Windows SHA256 Miner
curl -O http://localhost:3000/api/downloads/windows/sha256

# Linux GXHash Miner
curl -O http://localhost:3000/api/downloads/linux/gxhash
```

### Method 3: From Mining Guide

1. Visit http://localhost:3000/mining-guide
2. Scroll to "Download Mining Software"
3. Click platform-specific download button
4. Miner downloads automatically

---

## 🔄 After Deployment

When you deploy to a domain (e.g., `gxc.yourdomain.com`), update these:

### 1. Update Script
```bash
# Run this script to update all links to your domain
./update_download_links.sh
```

### 2. Manual Updates (if needed)

Edit these files to replace `localhost:3000` with your domain:

```
web/templates/downloads.html
web/templates/mining_guide.html
web/templates/mining.html
web/templates/explorer_index.html
mining_pool/templates/pool_dashboard.html
```

**Find and replace:**
```
localhost:3000 → gxc.yourdomain.com
```

### 3. Update Miner Configs

In the installer packages, update `config.json`:

```json
{
    "pool_url": "pool.yourdomain.com:3333",
    "api_url": "https://api.yourdomain.com",
    "explorer_url": "https://explorer.yourdomain.com"
}
```

---

## 📦 What Gets Downloaded

### Windows Miners (.zip files)

**Contents:**
```
sha256_miner_gui-windows-x64.zip
├── sha256_miner_gui.exe
├── install.bat
├── uninstall.bat
├── config.json
└── README.txt
```

**Installation:**
1. Extract ZIP
2. Double-click `install.bat`
3. Installs to `%LOCALAPPDATA%\GXC\Miners\`
4. Creates desktop shortcut
5. Creates start menu entry

### Linux Miners (.tar.gz files)

**Contents:**
```
sha256_miner_gui-linux-x64.tar.gz
├── sha256_miner_gui.AppDir/
│   ├── AppRun
│   ├── usr/bin/sha256_miner_gui
│   └── usr/share/applications/
└── README.txt
```

**Installation:**
```bash
tar -xzf sha256_miner_gui-linux-x64.tar.gz
chmod +x sha256_miner_gui.AppDir/AppRun
./sha256_miner_gui.AppDir/AppRun
```

---

## 🧪 Testing Download Links

### Test All Links

```bash
# Test download page
curl -I http://localhost:3000/downloads

# Test Windows SHA256 download
curl -I http://localhost:3000/api/downloads/windows/sha256

# Test Linux Ethash download
curl -I http://localhost:3000/api/downloads/linux/ethash

# Test GXHash download
curl -I http://localhost:3000/api/downloads/linux/gxhash
```

**Expected Response:**
- Status: `200 OK` (if miners built)
- Status: `404 Not Found` with helpful message (if not built yet)

### Build Miners First

If you get 404 errors, build the miners:

```bash
./build_miners_installers.sh
```

Then test again.

---

## 🔍 Troubleshooting

### "404 Not Found" Error

**Cause:** Miners haven't been built yet

**Solution:**
```bash
./build_miners_installers.sh
```

### "File Not Found" Error

**Cause:** Miner files not in expected location

**Solution:**
```bash
# Check if files exist
ls -la miners/installers/windows/
ls -la miners/installers/linux/

# Rebuild if missing
./build_miners_installers.sh
```

### Download Link Not Working

**Cause:** Explorer not running or wrong port

**Solution:**
```bash
# Check if explorer is running
curl http://localhost:3000

# Restart if needed
./stop_complete_ecosystem.sh
./start_complete_ecosystem.sh
```

### Old GitHub Links Still Showing

**Cause:** HTML files not updated

**Solution:**
```bash
# Run update script
./update_download_links.sh

# Restart explorer
pkill -f blockchain_explorer.py
nohup python3 web/blockchain_explorer.py > logs/explorer.log 2>&1 &
```

---

## 📊 Download Statistics

To track downloads, check the logs:

```bash
# View download requests
grep "api/downloads" logs/explorer.log

# Count downloads by platform
grep "api/downloads/windows" logs/explorer.log | wc -l
grep "api/downloads/linux" logs/explorer.log | wc -l

# Count downloads by algorithm
grep "api/downloads.*sha256" logs/explorer.log | wc -l
grep "api/downloads.*ethash" logs/explorer.log | wc -l
grep "api/downloads.*gxhash" logs/explorer.log | wc -l
```

---

## 🎨 Customizing Download Page

### Add Your Branding

Edit `web/templates/downloads.html`:

```html
<!-- Update title -->
<title>Download Miners | Your Brand</title>

<!-- Update header -->
<h1 class="display-4 mb-3">
    <i class="fas fa-download me-3"></i>Download Your Brand Miners
</h1>

<!-- Update footer -->
<p class="mb-0">&copy; 2024 Your Brand. All rights reserved.</p>
```

### Add Custom Instructions

Add your pool URL, wallet setup guide, or support links.

---

## 🚀 Production Checklist

Before going live:

- [ ] Build all miners: `./build_miners_installers.sh`
- [ ] Test all download links
- [ ] Update domain in HTML files
- [ ] Update miner configs with production pool URL
- [ ] Test installation on Windows
- [ ] Test installation on Linux
- [ ] Add SSL certificates
- [ ] Setup CDN (optional)
- [ ] Monitor download logs
- [ ] Create GitHub releases (optional backup)

---

## 📝 Summary

**All download links now:**
- ✅ Point to local API endpoints
- ✅ Work without GitHub
- ✅ Serve files directly
- ✅ Include installation instructions
- ✅ Are easy to find
- ✅ Are production ready

**Users can download from:**
- Main navigation bar
- Download page
- Mining guide
- Mining page
- Pool dashboard

**Everything is ready to use!** 🎉

---

**For more information:**
- Main Guide: `COMPLETE_SETUP_GUIDE.md`
- Deployment: `DEPLOYMENT_GUIDE.md`
- Final Status: `FINAL_STATUS.md`
