# -*- mode: python ; coding: utf-8 -*-
"""
PyInstaller Spec File for GXC Pool Miner
Creates standalone executables for Windows, macOS, and Linux
"""

import sys
import os
from PyInstaller.utils.hooks import collect_data_files, collect_submodules

block_cipher = None

# Application info
APP_NAME = 'GXC-Pool-Miner'
APP_VERSION = '2.0.0'

# Paths
src_path = os.path.abspath('src')
assets_path = os.path.abspath('assets')

# Collect data files
datas = [
    ('LICENSE', '.'),
    ('README.md', '.'),
]

if os.path.exists(assets_path):
    datas.append((assets_path, 'assets'))

# Hidden imports needed for the application
hiddenimports = [
    'customtkinter',
    'tkinter',
    'tkinter.ttk',
    'tkinter.messagebox',
    'tkinter.filedialog',
    'matplotlib',
    'matplotlib.backends.backend_tkagg',
    'matplotlib.figure',
    'psutil',
    'requests',
    'json',
    'socket',
    'threading',
    'hashlib',
    'struct',
    'queue',
    'logging',
    'dataclasses',
]

# Collect customtkinter data files
try:
    import customtkinter
    ctk_path = os.path.dirname(customtkinter.__file__)
    datas.append((ctk_path, 'customtkinter'))
except ImportError:
    pass

# Main analysis
a = Analysis(
    ['src/gxc_pool_miner.py'],
    pathex=[src_path],
    binaries=[],
    datas=datas,
    hiddenimports=hiddenimports,
    hookspath=[],
    hooksconfig={},
    runtime_hooks=[],
    excludes=[
        'pytest',
        'test',
        'tests',
        'unittest',
        'doctest',
    ],
    win_no_prefer_redirects=False,
    win_private_assemblies=False,
    cipher=block_cipher,
    noarchive=False,
)

# Remove unnecessary files
excluded_binaries = [
    'tcl85.dll',
    'tk85.dll',
    '_tkinter',
]

a.binaries = [x for x in a.binaries if x[0] not in excluded_binaries]

pyz = PYZ(
    a.pure,
    a.zipped_data,
    cipher=block_cipher
)

# Executable configuration
exe = EXE(
    pyz,
    a.scripts,
    a.binaries,
    a.zipfiles,
    a.datas,
    [],
    name=APP_NAME,
    debug=False,
    bootloader_ignore_signals=False,
    strip=False,
    upx=True,
    upx_exclude=[],
    runtime_tmpdir=None,
    console=False,  # GUI application, no console
    disable_windowed_traceback=False,
    target_arch=None,
    codesign_identity=None,
    entitlements_file=None,
    icon='assets/icon.ico' if os.path.exists('assets/icon.ico') else None,
)

# macOS App Bundle
if sys.platform == 'darwin':
    app = BUNDLE(
        exe,
        name=f'{APP_NAME}.app',
        icon='assets/icon.icns' if os.path.exists('assets/icon.icns') else None,
        bundle_identifier='io.gxc.poolminer',
        info_plist={
            'NSPrincipalClass': 'NSApplication',
            'NSAppleScriptEnabled': False,
            'CFBundleDocumentTypes': [],
            'CFBundleName': APP_NAME,
            'CFBundleDisplayName': 'GXC Pool Miner',
            'CFBundleShortVersionString': APP_VERSION,
            'CFBundleVersion': APP_VERSION,
            'NSHumanReadableCopyright': 'Copyright © 2024 GXC Blockchain',
            'NSHighResolutionCapable': True,
        },
    )

# One-folder distribution (alternative)
coll = COLLECT(
    exe,
    a.binaries,
    a.zipfiles,
    a.datas,
    strip=False,
    upx=True,
    upx_exclude=[],
    name=APP_NAME,
)
