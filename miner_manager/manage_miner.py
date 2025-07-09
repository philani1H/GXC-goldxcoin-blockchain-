#!/usr/bin/env python3
"""
Minimal Miner-Manager service for GXC
------------------------------------
• Supervises an external SHA-256 / Ethash miner (ASIC or GPU)
• Provides REST endpoints so the browser GUI can change config without hand-editing files
    GET  /config   – returns current JSON config
    POST /config   – partial update, then auto-restart miner
    GET  /status   – { running: true/false }

Config is stored in ~/.gxc/miner.conf (Windows: %APPDATA%\GXC\miner.conf)
Log file is ~/.gxc/miner.log

Works cross-platform – replace the `build_cmd()` logic with the actual command
line syntax of the miner binary you use (BraiinsOS, cgminer, PhoenixMiner, etc.).
"""
import json
import os
import signal
import subprocess
import sys
import time
from pathlib import Path

from flask import Flask, request, jsonify
from flask_cors import CORS

HOME = Path.home()
DATA_DIR = HOME / ("AppData/Roaming/GXC" if os.name == "nt" else ".gxc")
CONF_FILE = DATA_DIR / "miner.conf"
LOG_FILE = DATA_DIR / "miner.log"

DEFAULT_CONF = {
    "algo": "sha256",          # sha256 or ethash
    "mode": "solo",           # solo or pool
    "pool": "stratum+tcp://127.0.0.1:4334",
    "address": "GXC1ReplaceMe",  # must be valid address
    "worker": "myRig",
    "threads": 0               # 0 = auto-detect
}

app = Flask(__name__)
CORS(app)
miner_proc: subprocess.Popen | None = None

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def load_conf() -> dict:
    if CONF_FILE.exists():
        with CONF_FILE.open() as f:
            return json.load(f)
    # first run – create default conf
    DATA_DIR.mkdir(parents=True, exist_ok=True)
    save_conf(DEFAULT_CONF)
    return DEFAULT_CONF.copy()

def save_conf(cfg: dict):
    CONF_FILE.write_text(json.dumps(cfg, indent=2))


def build_cmd(cfg: dict) -> list[str]:
    """Return list usable by subprocess.Popen."""
    # Choose your preferred miner binary here
    if cfg["algo"] == "sha256":
        exe = "cpuminer"
        port = 4334
    else:
        exe = "ethminer"
        port = 4335

    url = cfg["pool"]
    if cfg["mode"] == "solo":
        # enforce localhost stratum if user forgets
        url = f"stratum+tcp://127.0.0.1:{port}"

    user = f"{cfg['address']}.{cfg['worker']}"
    threads_arg = [] if cfg["threads"] in (0, None) else ["-t", str(cfg["threads"])]

    return [exe, "-a", cfg["algo"], "-o", url, "-u", user, "-p", "x", *threads_arg]


# ---------------------------------------------------------------------------
# Miner process control
# ---------------------------------------------------------------------------

def start_miner():
    global miner_proc
    if miner_proc and miner_proc.poll() is None:
        return  # already running

    cmd = build_cmd(load_conf())
    LOG_FILE.parent.mkdir(parents=True, exist_ok=True)
    log_handle = LOG_FILE.open("ab", buffering=0)
    miner_proc = subprocess.Popen(cmd, stdout=log_handle, stderr=log_handle)
    print(f"[Miner-Manager] Started miner: {' '.join(cmd)}")


def stop_miner():
    global miner_proc
    if miner_proc and miner_proc.poll() is None:
        print("[Miner-Manager] Stopping miner …")
        miner_proc.terminate()
        try:
            miner_proc.wait(10)
        except subprocess.TimeoutExpired:
            miner_proc.kill()
    miner_proc = None


def restart_miner():
    stop_miner()
    start_miner()

# ---------------------------------------------------------------------------
# REST API
# ---------------------------------------------------------------------------

@app.route("/config", methods=["GET"])
def api_get_config():
    return jsonify(load_conf())


@app.route("/config", methods=["POST"])
def api_set_config():
    data = request.get_json(force=True)
    cfg = load_conf()
    # merge only known keys
    for key in DEFAULT_CONF:
        if key in data:
            cfg[key] = data[key]
    save_conf(cfg)
    restart_miner()
    return jsonify({"success": True, "config": cfg})


@app.route("/status", methods=["GET"])
def api_status():
    running = miner_proc is not None and miner_proc.poll() is None
    return jsonify({"running": running})

# ---------------------------------------------------------------------------
if __name__ == "__main__":
    start_miner()
    port = 5055
    print(f"[Miner-Manager] REST listening on 127.0.0.1:{port}")
    app.run(host="127.0.0.1", port=port)