#!/bin/bash
# GXC GxHash Miner Installer for Linux

set -e
APP="GXC GxHash Miner"
VER="2.0.0"
DIR="$HOME/.gxc-gxhash-miner"
BIN="$HOME/.local/bin"

G='\033[0;32m'; C='\033[0;36m'; N='\033[0m'

echo -e "${C}╔═══════════════════════════════════════════════════╗${N}"
echo -e "${C}║    GXC GxHash CPU Miner Installer v${VER}          ║${N}"
echo -e "${C}╚═══════════════════════════════════════════════════╝${N}"
echo

# Check Python
if command -v python3 &>/dev/null; then PY=python3
elif command -v python &>/dev/null; then PY=python
else echo "Error: Python 3 required"; exit 1; fi
echo -e "${G}[✓]${N} Python: $($PY --version)"

# License
echo
echo "MIT License - Mining involves financial risk."
read -p "Accept? [y/N] " -n 1 -r; echo
[[ ! $REPLY =~ ^[Yy]$ ]] && echo "Cancelled." && exit 1

# Install
mkdir -p "$DIR" "$BIN"
cp gxhash_miner.py requirements.txt "$DIR/"
$PY -m pip install -r requirements.txt

cat > "$BIN/gxhash-miner" << EOF
#!/bin/bash
$PY "$DIR/gxhash_miner.py" "\$@"
EOF
chmod +x "$BIN/gxhash-miner"

echo -e "${G}[✓]${N} Installed! Run: gxhash-miner"
