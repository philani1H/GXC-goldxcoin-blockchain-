# -*- mode: python ; coding: utf-8 -*-
# PyInstaller spec file for GXC Miner Suite

block_cipher = None

# Main unified miner
gxc_miner = Analysis(
    ['src/gxc_miner.py'],
    pathex=[],
    binaries=[],
    datas=[('LICENSE', '.'), ('README.md', '.')],
    hiddenimports=['customtkinter', 'psutil', 'requests', 'matplotlib'],
    hookspath=[],
    hooksconfig={},
    runtime_hooks=[],
    excludes=[],
    win_no_prefer_redirects=False,
    win_private_assemblies=False,
    cipher=block_cipher,
    noarchive=False,
)

gxc_pyz = PYZ(gxc_miner.pure, gxc_miner.zipped_data, cipher=block_cipher)

gxc_exe = EXE(
    gxc_pyz,
    gxc_miner.scripts,
    gxc_miner.binaries,
    gxc_miner.zipfiles,
    gxc_miner.datas,
    [],
    name='GXC-Miner-Suite',
    debug=False,
    bootloader_ignore_signals=False,
    strip=False,
    upx=True,
    upx_exclude=[],
    runtime_tmpdir=None,
    console=False,
    disable_windowed_traceback=False,
    argv_emulation=False,
    target_arch=None,
    codesign_identity=None,
    entitlements_file=None,
)

# SHA256 miner
sha256_miner = Analysis(
    ['src/sha256_miner.py'],
    pathex=[],
    binaries=[],
    datas=[],
    hiddenimports=['customtkinter', 'psutil', 'requests'],
    hookspath=[],
    runtime_hooks=[],
    excludes=[],
    cipher=block_cipher,
)

sha256_pyz = PYZ(sha256_miner.pure, sha256_miner.zipped_data, cipher=block_cipher)

sha256_exe = EXE(
    sha256_pyz,
    sha256_miner.scripts,
    sha256_miner.binaries,
    sha256_miner.zipfiles,
    sha256_miner.datas,
    [],
    name='GXC-SHA256-Miner',
    debug=False,
    strip=False,
    upx=True,
    console=False,
)

# Ethash miner
ethash_miner = Analysis(
    ['src/ethash_miner.py'],
    pathex=[],
    binaries=[],
    datas=[],
    hiddenimports=['customtkinter', 'psutil', 'requests'],
    hookspath=[],
    runtime_hooks=[],
    excludes=[],
    cipher=block_cipher,
)

ethash_pyz = PYZ(ethash_miner.pure, ethash_miner.zipped_data, cipher=block_cipher)

ethash_exe = EXE(
    ethash_pyz,
    ethash_miner.scripts,
    ethash_miner.binaries,
    ethash_miner.zipfiles,
    ethash_miner.datas,
    [],
    name='GXC-Ethash-Miner',
    debug=False,
    strip=False,
    upx=True,
    console=False,
)
