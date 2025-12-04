# -*- mode: python ; coding: utf-8 -*-
"""
GXC Miner Pro - PyInstaller Specification
Creates standalone executable for all platforms
"""

import sys
import os
from PyInstaller.utils.hooks import collect_data_files, collect_submodules

block_cipher = None

# Collect all required data files
datas = []

# Try to collect customtkinter data files
try:
    datas += collect_data_files('customtkinter')
except Exception:
    pass

# Try to collect matplotlib data files
try:
    datas += collect_data_files('matplotlib')
except Exception:
    pass

# Hidden imports for all dependencies
hiddenimports = [
    'customtkinter',
    'tkinter',
    'tkinter.ttk',
    'matplotlib',
    'matplotlib.pyplot',
    'matplotlib.backends.backend_tkagg',
    'psutil',
    'requests',
    'json',
    'hashlib',
    'socket',
    'threading',
    'queue',
    'struct',
    'argparse',
    'dataclasses',
    'typing',
    'enum',
    'abc',
    'platform',
    'subprocess',
    'time',
    'datetime',
    'random',
    'os',
    'sys',
]

# Try to add pycryptodome
try:
    hiddenimports.extend(collect_submodules('Crypto'))
except Exception:
    pass

a = Analysis(
    ['src/gxc_miner_pro.py'],
    pathex=[],
    binaries=[],
    datas=datas,
    hiddenimports=hiddenimports,
    hookspath=[],
    hooksconfig={},
    runtime_hooks=[],
    excludes=[
        'pytest',
        'sphinx',
        'notebook',
        'jupyter',
    ],
    win_no_prefer_redirects=False,
    win_private_assemblies=False,
    cipher=block_cipher,
    noarchive=False,
)

pyz = PYZ(
    a.pure,
    a.zipped_data,
    cipher=block_cipher,
)

exe = EXE(
    pyz,
    a.scripts,
    a.binaries,
    a.zipfiles,
    a.datas,
    [],
    name='GXC-Miner-Pro',
    debug=False,
    bootloader_ignore_signals=False,
    strip=False,
    upx=True,
    upx_exclude=[],
    runtime_tmpdir=None,
    console=True,  # Keep console for CLI mode
    disable_windowed_traceback=False,
    argv_emulation=False,
    target_arch=None,
    codesign_identity=None,
    entitlements_file=None,
    icon=None,  # Add icon path here if available
)

# For macOS, create an app bundle
if sys.platform == 'darwin':
    app = BUNDLE(
        exe,
        name='GXC Miner Pro.app',
        icon=None,
        bundle_identifier='network.gxc.miner-pro',
        info_plist={
            'CFBundleName': 'GXC Miner Pro',
            'CFBundleDisplayName': 'GXC Miner Pro',
            'CFBundleVersion': '2.0.0',
            'CFBundleShortVersionString': '2.0.0',
            'NSHighResolutionCapable': True,
        },
    )
