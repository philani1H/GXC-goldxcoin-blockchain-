#!/usr/bin/env python3
"""
Simple test node for running stress tests
Implements minimal RPC server for testing
"""

from http.server import HTTPServer, BaseHTTPRequestHandler
import json
import threading
import time

class SimpleBlockchain:
    def __init__(self):
        self.height = 0
        self.difficulty = 1.0
        self.validators = []
        self.mempool = []
        
    def get_info(self):
        return {
            "blocks": self.height,
            "difficulty": self.difficulty,
            "validators": len(self.validators)
        }

blockchain = SimpleBlockchain()

class RPCHandler(BaseHTTPRequestHandler):
    def log_message(self, format, *args):
        pass  # Suppress logs
        
    def do_POST(self):
        content_length = int(self.headers['Content-Length'])
        body = self.rfile.read(content_length)
        
        try:
            request = json.loads(body)
            method = request.get('method')
            params = request.get('params', [])
            
            result = None
            
            if method == 'getblockchaininfo':
                result = blockchain.get_info()
            elif method == 'getblocktemplate':
                miner_addr = params[0].get('minerAddress', 'test') if params else 'test'
                result = {
                    'height': blockchain.height,
                    'previousblockhash': '0' * 64,
                    'difficulty': blockchain.difficulty,
                    'transactions': [],
                    'coinbasevalue': 50.0
                }
            elif method == 'submitblock':
                blockchain.height += 1
                result = None
            elif method == 'listvalidators':
                result = blockchain.validators
            elif method == 'registervalidator':
                blockchain.validators.append({'address': params[0] if params else 'test'})
                result = {'success': True}
            elif method == 'stake':
                result = {'success': True, 'txHash': 'test_tx'}
            elif method == 'unstake':
                result = {'success': True}
            elif method == 'sendtoaddress':
                result = 'test_tx_hash'
            else:
                result = {'error': f'Unknown method: {method}'}
            
            response = {
                'jsonrpc': '2.0',
                'result': result,
                'id': request.get('id', 1)
            }
            
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            self.wfile.write(json.dumps(response).encode())
            
        except Exception as e:
            error_response = {
                'jsonrpc': '2.0',
                'error': {'code': -32603, 'message': str(e)},
                'id': 1
            }
            self.send_response(500)
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            self.wfile.write(json.dumps(error_response).encode())

def run_server(port=8332):
    server = HTTPServer(('localhost', port), RPCHandler)
    print(f"Simple test node running on port {port}")
    print("Press Ctrl+C to stop")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nShutting down...")
        server.shutdown()

if __name__ == '__main__':
    run_server()
