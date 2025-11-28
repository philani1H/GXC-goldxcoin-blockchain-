#!/usr/bin/env python3
"""
Start all GXC services (Windows/Linux/Mac compatible)
No building required!
"""

import subprocess
import time
import os
import sys
import platform

def start_service(name, command, cwd=None):
    """Start a service in a new window/terminal"""
    print(f"Starting {name}...")
    
    try:
        if platform.system() == 'Windows':
            # Windows: Start in new cmd window
            subprocess.Popen(
                f'start cmd /k "title {name} && {command}"',
                shell=True,
                cwd=cwd
            )
        elif platform.system() == 'Darwin':  # macOS
            # macOS: Start in new Terminal window
            script = f'tell application "Terminal" to do script "cd {os.getcwd() if not cwd else os.path.join(os.getcwd(), cwd)} && {command}"'
            subprocess.Popen(['osascript', '-e', script])
        else:  # Linux
            # Linux: Start in background
            subprocess.Popen(
                command,
                shell=True,
                cwd=cwd,
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL
            )
        
        time.sleep(2)
        print(f"  ✅ {name} started")
        
    except Exception as e:
        print(f"  ⚠️  Could not start {name}: {e}")

def check_python():
    """Check Python version"""
    version = sys.version_info
    if version.major < 3 or (version.major == 3 and version.minor < 8):
        print("❌ Python 3.8+ required")
        print(f"   Current version: {version.major}.{version.minor}.{version.micro}")
        sys.exit(1)
    print(f"✅ Python {version.major}.{version.minor}.{version.micro}")

def check_dependencies():
    """Check if dependencies are installed"""
    try:
        import flask
        print("✅ Dependencies installed")
        return True
    except ImportError:
        print("⚠️  Dependencies not installed")
        print("\nInstalling dependencies...")
        subprocess.run([sys.executable, '-m', 'pip', 'install', '-r', 'requirements.txt'])
        return True

if __name__ == '__main__':
    print("=" * 60)
    print("🚀 GXC Blockchain - Starting All Services")
    print("=" * 60)
    print()
    
    # Check Python version
    print("Checking Python version...")
    check_python()
    print()
    
    # Check dependencies
    print("Checking dependencies...")
    check_dependencies()
    print()
    
    print("Starting services...")
    print()
    
    # Start blockchain node
    start_service(
        "Blockchain Node",
        f"{sys.executable} testnet_blockchain_node.py"
    )
    
    # Start explorer
    start_service(
        "Explorer",
        f"{sys.executable} blockchain_explorer.py",
        cwd="web"
    )
    
    # Start wallet API
    start_service(
        "Wallet API",
        f"{sys.executable} wallet_service.py",
        cwd="api"
    )
    
    # Start mining pool
    start_service(
        "Mining Pool",
        f"{sys.executable} general_pool.py",
        cwd="mining_pool"
    )
    
    # Start forum
    start_service(
        "Forum",
        f"{sys.executable} forum.py",
        cwd="web"
    )
    
    print()
    print("=" * 60)
    print("✅ All services started!")
    print("=" * 60)
    print()
    print("Access your blockchain:")
    print("  🌐 Explorer:     http://localhost:3000")
    print("  💰 Wallet API:   http://localhost:5000")
    print("  ⛏️  Mining Pool:  http://localhost:6000")
    print("  💬 Forum:        http://localhost:3001")
    print()
    print("To mine:")
    print(f"  cd mining")
    print(f"  {sys.executable} gxhash_miner.py --pool localhost:3333 --address YOUR_ADDRESS")
    print()
    print("To stop all services:")
    print(f"  {sys.executable} stop_all_services.py")
    print()
    
    if platform.system() == 'Windows':
        print("Services are running in separate windows.")
        print("Close the windows to stop services.")
    else:
        print("Services are running in background.")
        print(f"Run: {sys.executable} stop_all_services.py to stop")
    
    print()
    print("Press Ctrl+C to exit this script (services will continue running)")
    print()
    
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("\n\n👋 Exiting... (services still running)")
        print(f"To stop services: {sys.executable} stop_all_services.py")
