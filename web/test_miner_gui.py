import os
import re
import json
import socket
import threading
import time
import argparse

def parse_stratum_url(url):
    u = url.strip()
    if u.startswith('stratum+tcp://'):
        u = u[len('stratum+tcp://'):]
    m = re.match(r'^([^:/]+):(\d+)', u)
    if m:
        return m.group(1), int(m.group(2))
    return u, 3333

class StratumClient:
    def __init__(self, host, port, user, password='x'):
        self.host = host
        self.port = port
        self.user = user
        self.password = password
        self.sock = None
        self.running = False
        self.recv_thread = None
        self.id_counter = 0

    def _send(self, payload):
        data = json.dumps(payload) + '\n'
        self.sock.sendall(data.encode())

    def connect(self):
        self.sock = socket.create_connection((self.host, self.port), timeout=10)
        self.running = True
        self._send({"id": 1, "method": "mining.subscribe", "params": ["gxc-gui-miner"]})
        self._send({"id": 2, "method": "mining.authorize", "params": [self.user, self.password]})
        self.recv_thread = threading.Thread(target=self._recv_loop, daemon=True)
        self.recv_thread.start()

    def _recv_loop(self):
        buf = b''
        while self.running:
            try:
                chunk = self.sock.recv(4096)
                if not chunk:
                    break
                buf += chunk
                while b"\n" in buf:
                    line, buf = buf.split(b"\n", 1)
                    s = line.decode(errors='ignore').strip()
                    if s:
                        print(s)
            except Exception:
                break

    def disconnect(self):
        self.running = False
        try:
            if self.sock:
                self.sock.close()
        except Exception:
            pass

def run_probe(url, address):
    host, port = parse_stratum_url(url)
    user = f"{address}.worker1" if address else "test.worker1"
    c = StratumClient(host, port, user)
    try:
        print(f"Connecting to {host}:{port} as {user}...")
        c.connect()
        t0 = time.time()
        while time.time() - t0 < 5:
            time.sleep(0.25)
        print("Disconnecting...")
    finally:
        c.disconnect()

def run_gui(url_default, addr_default):
    import tkinter as tk
    from tkinter import ttk
    from tkinter import messagebox
    host, port = parse_stratum_url(url_default)
    user = f"{addr_default}.worker1" if addr_default else "test.worker1"
    client = None

    root = tk.Tk()
    root.title("GXC Test Miner GUI")
    frm = ttk.Frame(root, padding=10)
    frm.grid(column=0, row=0, sticky="nsew")
    root.rowconfigure(0, weight=1)
    root.columnconfigure(0, weight=1)

    tk.Label(frm, text="Stratum URL").grid(row=0, column=0, sticky="w")
    url_var = tk.StringVar(value=url_default)
    tk.Entry(frm, textvariable=url_var, width=40).grid(row=0, column=1, sticky="ew")

    tk.Label(frm, text="Address").grid(row=1, column=0, sticky="w")
    addr_var = tk.StringVar(value=addr_default)
    tk.Entry(frm, textvariable=addr_var, width=40).grid(row=1, column=1, sticky="ew")

    log = tk.Text(frm, height=16, width=60)
    log.grid(row=2, column=0, columnspan=2, sticky="nsew", pady=(6,6))
    frm.rowconfigure(2, weight=1)
    frm.columnconfigure(1, weight=1)

    def append(text):
        log.insert("end", text + "\n")
        log.see("end")

    def on_connect():
        nonlocal client
        u = url_var.get().strip()
        a = addr_var.get().strip()
        h, p = parse_stratum_url(u)
        user_s = f"{a}.worker1" if a else "test.worker1"
        try:
            client = StratumClient(h, p, user_s)
            client.connect()
            append(f"Connected to {h}:{p} as {user_s}")
        except Exception as e:
            messagebox.showerror("Connection error", str(e))

    def on_disconnect():
        nonlocal client
        if client:
            client.disconnect()
            client = None
            append("Disconnected")

    btnf = ttk.Frame(frm)
    btnf.grid(row=3, column=0, columnspan=2, sticky="ew")
    ttk.Button(btnf, text="Connect", command=on_connect).grid(row=0, column=0, padx=4)
    ttk.Button(btnf, text="Disconnect", command=on_disconnect).grid(row=0, column=1, padx=4)

    root.mainloop()

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--probe', action='store_true')
    parser.add_argument('--stratum', default=os.environ.get('GENERAL_POOL_STRATUM_URL', 'stratum+tcp://pool.gxc.network:3333'))
    parser.add_argument('--address', default=os.environ.get('MINER_ADDRESS', ''))
    args = parser.parse_args()
    if args.probe:
        run_probe(args.stratum, args.address)
    else:
        run_gui(args.stratum, args.address)

if __name__ == '__main__':
    main()