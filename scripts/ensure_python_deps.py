#!/usr/bin/env python3
"""ensure_python_deps.py – lightweight bootstrapper

Run this once before any Flask / wallet / explorer service to guarantee
all required third-party packages are present. The script:
1. Reads ../requirements.txt (relative to repo root)
2. Tries `importlib.import_module()` for each package’s canonical import name
3. If import fails it runs `pip install <package_spec>` in a subprocess.

This avoids the classic “ModuleNotFoundError” when a new dev clones the project
and forgets to run `pip install -r requirements.txt`.
"""
from __future__ import annotations

import importlib
import subprocess
import sys
from pathlib import Path

REQ = Path(__file__).resolve().parent.parent / "requirements.txt"

# Map pip name → import name (when they differ)
ALIASES = {
    "Flask-Cors": "flask_cors",
    "Flask-SocketIO": "flask_socketio",
    "PyJWT": "jwt",
    "yfinance": "yfinance",  # same
}

def ensure(pkg_spec: str):
    """Import; if fail, pip-install the requirement string."""
    pkg_name = pkg_spec.split("==")[0].split(">=")[0]
    mod_name = ALIASES.get(pkg_name, pkg_name.lower())
    try:
        importlib.import_module(mod_name)
    except ImportError:
        print(f"[deps] Missing {pkg_spec} → installing…", flush=True)
        subprocess.check_call([sys.executable, "-m", "pip", "install", pkg_spec])


def main():
    if not REQ.exists():
        print("requirements.txt not found – nothing to check")
        return
    for line in REQ.read_text().splitlines():
        line = line.strip()
        if line and not line.startswith("#"):
            ensure(line)

if __name__ == "__main__":
    main()