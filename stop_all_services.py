#!/usr/bin/env python3
"""
Stop all GXC services (Windows/Linux/Mac compatible)
"""

import subprocess
import os
import platform
import sys

def stop_services_windows():
    """Stop services on Windows"""
    print("🛑 Stopping GXC services on Windows...")
    
    # Kill Python processes running our services
    services = [
        "testnet_blockchain_node",
        "blockchain_explorer",
        "wallet_service",
        "general_pool",
        "forum"
    ]
    
    for service in services:
        try:
            # Find and kill processes
            result = subprocess.run(
                f'tasklist /FI "WINDOWTITLE eq {service}*" /FO CSV',
                shell=True,
                capture_output=True,
                text=True
            )
            
            if "python" in result.stdout.lower():
                subprocess.run(
                    f'taskkill /FI "WINDOWTITLE eq {service}*" /F',
                    shell=True,
                    stdout=subprocess.DEVNULL,
                    stderr=subprocess.DEVNULL
                )
                print(f"  ✅ Stopped {service}")
        except Exception as e:
            pass
    
    # Also try to kill by process name
    try:
        subprocess.run(
            'taskkill /F /IM python.exe /FI "WINDOWTITLE eq Blockchain*"',
            shell=True,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL
        )
        subprocess.run(
            'taskkill /F /IM python.exe /FI "WINDOWTITLE eq Explorer*"',
            shell=True,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL
        )
        subprocess.run(
            'taskkill /F /IM python.exe /FI "WINDOWTITLE eq Wallet*"',
            shell=True,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL
        )
        subprocess.run(
            'taskkill /F /IM python.exe /FI "WINDOWTITLE eq Mining*"',
            shell=True,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL
        )
        subprocess.run(
            'taskkill /F /IM python.exe /FI "WINDOWTITLE eq Forum*"',
            shell=True,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL
        )
    except:
        pass

def stop_services_unix():
    """Stop services on Linux/Mac"""
    print("🛑 Stopping GXC services on Unix...")
    
    services = [
        "testnet_blockchain_node",
        "blockchain_explorer",
        "wallet_service",
        "general_pool",
        "forum"
    ]
    
    for service in services:
        try:
            subprocess.run(
                f'pkill -f "python.*{service}"',
                shell=True,
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL
            )
            print(f"  ✅ Stopped {service}")
        except Exception as e:
            pass

def stop_services():
    """Stop all services based on platform"""
    print("=" * 60)
    print("🛑 Stopping GXC Blockchain Services")
    print("=" * 60)
    print()
    
    if platform.system() == 'Windows':
        stop_services_windows()
    else:
        stop_services_unix()
    
    print()
    print("=" * 60)
    print("✅ All services stopped")
    print("=" * 60)
    print()
    print("To start services again:")
    print(f"  {sys.executable} start_all_services.py")
    print()

if __name__ == '__main__':
    stop_services()
