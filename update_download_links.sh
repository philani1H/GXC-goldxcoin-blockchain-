#!/bin/bash
# Update all miner download links in HTML files to point to local API

echo "🔗 Updating miner download links in HTML files..."

# Function to update links in a file
update_file() {
    local file=$1
    echo "Updating: $file"
    
    # Backup original
    cp "$file" "$file.bak"
    
    # Update GXHash miner links
    sed -i 's|https://github.com/gxc-blockchain/gxc-blockchain/releases/latest/download/gxc-gxhash-miner-linux-x64.tar.gz|/api/downloads/linux/gxhash|g' "$file"
    sed -i 's|https://github.com/gxc-blockchain/gxc-blockchain/releases/latest/download/gxc-gxhash-miner-windows-x64.zip|/api/downloads/windows/gxhash|g' "$file"
    
    # Update SHA256 miner links
    sed -i 's|https://github.com/gxc-blockchain/gxc-blockchain/releases/latest/download/gxc-sha256-miner-linux-x64.tar.gz|/api/downloads/linux/sha256|g' "$file"
    sed -i 's|https://github.com/gxc-blockchain/gxc-blockchain/releases/latest/download/gxc-sha256-miner-windows-x64.zip|/api/downloads/windows/sha256|g' "$file"
    
    # Update Ethash miner links (NVIDIA)
    sed -i 's|https://github.com/gxc-blockchain/gxc-blockchain/releases/latest/download/gxc-ethash-miner-linux-nvidia.tar.gz|/api/downloads/linux/ethash|g' "$file"
    sed -i 's|https://github.com/gxc-blockchain/gxc-blockchain/releases/latest/download/gxc-ethash-miner-windows-nvidia.zip|/api/downloads/windows/ethash|g' "$file"
    
    # Update Ethash miner links (AMD)
    sed -i 's|https://github.com/gxc-blockchain/gxc-blockchain/releases/latest/download/gxc-ethash-miner-linux-amd.tar.gz|/api/downloads/linux/ethash|g' "$file"
    sed -i 's|https://github.com/gxc-blockchain/gxc-blockchain/releases/latest/download/gxc-ethash-miner-windows-amd.zip|/api/downloads/windows/ethash|g' "$file"
    
    # Update generic miner links
    sed -i 's|https://github.com/gxc-blockchain/gxc-blockchain/releases/latest/download/gxc-miner-linux-x64.tar.gz|/api/downloads/linux/gxhash|g' "$file"
    sed -i 's|https://github.com/gxc-blockchain/gxc-blockchain/releases/latest/download/gxc-miner-windows-x64.zip|/api/downloads/windows/gxhash|g' "$file"
    
    # Update macOS links to point to downloads page (we don't have macOS builds yet)
    sed -i 's|https://github.com/gxc-blockchain/gxc-blockchain/releases/latest/download/gxc-gxhash-miner-macos-x64.tar.gz|/downloads|g' "$file"
    sed -i 's|https://github.com/gxc-blockchain/gxc-blockchain/releases/latest/download/gxc-miner-macos-x64.tar.gz|/downloads|g' "$file"
    
    # Update release page links to downloads page
    sed -i 's|https://github.com/gxc-blockchain/gxc-blockchain/releases/latest|/downloads|g' "$file"
    
    echo "  ✅ Updated"
}

# Update mining guide
if [ -f "web/templates/mining_guide.html" ]; then
    update_file "web/templates/mining_guide.html"
fi

# Update mining page
if [ -f "web/templates/mining.html" ]; then
    update_file "web/templates/mining.html"
fi

# Update pool templates
for file in mining_pool/templates/*.html; do
    if [ -f "$file" ]; then
        if grep -q "github.com/gxc-blockchain" "$file" 2>/dev/null; then
            update_file "$file"
        fi
    fi
done

# Update explorer index
if [ -f "web/templates/explorer_index.html" ]; then
    if grep -q "github.com/gxc-blockchain" "web/templates/explorer_index.html" 2>/dev/null; then
        update_file "web/templates/explorer_index.html"
    fi
fi

# Update any other HTML files with download links
for file in web/templates/*.html; do
    if [ -f "$file" ]; then
        if grep -q "github.com/gxc-blockchain.*download.*miner" "$file" 2>/dev/null; then
            update_file "$file"
        fi
    fi
done

echo ""
echo "======================================"
echo "✅ Download links updated!"
echo "======================================"
echo ""
echo "Updated links now point to:"
echo "  • /api/downloads/linux/sha256"
echo "  • /api/downloads/linux/ethash"
echo "  • /api/downloads/linux/gxhash"
echo "  • /api/downloads/windows/sha256"
echo "  • /api/downloads/windows/ethash"
echo "  • /api/downloads/windows/gxhash"
echo "  • /downloads (for download page)"
echo ""
echo "Backup files created with .bak extension"
echo ""
