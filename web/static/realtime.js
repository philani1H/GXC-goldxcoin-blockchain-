/**
 * GXC Explorer Real-Time Updates
 * Provides WebSocket and polling fallback for real-time data updates
 */

class GXCRealtime {
    constructor(options = {}) {
        this.socket = null;
        this.pollingInterval = null;
        this.useWebSocket = options.useWebSocket !== false;
        this.usePolling = options.usePolling !== false;
        this.pollInterval = options.pollInterval || 10000; // 10 seconds
        this.onBlock = options.onBlock || null;
        this.onTransaction = options.onTransaction || null;
        this.onAddressUpdate = options.onAddressUpdate || null;
        this.onStatsUpdate = options.onStatsUpdate || null;
        this.connected = false;
        this.reconnectAttempts = 0;
        this.maxReconnectAttempts = 10;
        
        // Initialize
        if (this.useWebSocket && typeof io !== 'undefined') {
            this.initWebSocket();
        } else if (this.usePolling) {
            this.initPolling();
        }
    }
    
    initWebSocket() {
        try {
            this.socket = io();
            
            this.socket.on('connect', () => {
                console.log('[Realtime] Connected to GXC Explorer');
                this.connected = true;
                this.reconnectAttempts = 0;
                this.onConnected();
            });
            
            this.socket.on('disconnect', () => {
                console.log('[Realtime] Disconnected from GXC Explorer');
                this.connected = false;
                this.onDisconnected();
                this.attemptReconnect();
            });
            
            this.socket.on('new_block', (block) => {
                console.log('[Realtime] New block received:', block);
                if (this.onBlock) {
                    this.onBlock(block);
                }
            });
            
            this.socket.on('new_transaction', (tx) => {
                console.log('[Realtime] New transaction received:', tx);
                if (this.onTransaction) {
                    this.onTransaction(tx);
                }
            });
            
            this.socket.on('address_update', (address) => {
                console.log('[Realtime] Address update received:', address);
                if (this.onAddressUpdate) {
                    this.onAddressUpdate(address);
                }
            });
            
            this.socket.on('stats_update', (stats) => {
                console.log('[Realtime] Stats update received:', stats);
                if (this.onStatsUpdate) {
                    this.onStatsUpdate(stats);
                }
            });
            
            this.socket.on('connect_error', (error) => {
                console.warn('[Realtime] WebSocket connection error:', error);
                this.connected = false;
                if (this.usePolling) {
                    this.initPolling();
                }
            });
            
        } catch (error) {
            console.warn('[Realtime] WebSocket initialization failed:', error);
            if (this.usePolling) {
                this.initPolling();
            }
        }
    }
    
    initPolling() {
        console.log('[Realtime] Initializing polling fallback');
        if (this.pollingInterval) {
            clearInterval(this.pollingInterval);
        }
        
        // Initial fetch
        this.poll();
        
        // Set up polling interval
        this.pollingInterval = setInterval(() => {
            this.poll();
        }, this.pollInterval);
    }
    
    poll() {
        // Poll for latest data - this will be customized per page
        if (this.onStatsUpdate) {
            fetch('/api/v1/stats')
                .then(response => response.json())
                .then(data => {
                    if (this.onStatsUpdate) {
                        this.onStatsUpdate(data);
                    }
                })
                .catch(error => console.warn('[Realtime] Polling error:', error));
        }
    }
    
    attemptReconnect() {
        if (this.reconnectAttempts < this.maxReconnectAttempts) {
            this.reconnectAttempts++;
            const delay = Math.min(1000 * Math.pow(2, this.reconnectAttempts), 30000);
            console.log(`[Realtime] Attempting reconnect in ${delay}ms (attempt ${this.reconnectAttempts})`);
            setTimeout(() => {
                if (!this.connected) {
                    this.initWebSocket();
                }
            }, delay);
        } else {
            console.warn('[Realtime] Max reconnect attempts reached, falling back to polling');
            if (this.usePolling) {
                this.initPolling();
            }
        }
    }
    
    onConnected() {
        // Override in options
    }
    
    onDisconnected() {
        // Override in options
    }
    
    disconnect() {
        if (this.socket) {
            this.socket.disconnect();
            this.socket = null;
        }
        if (this.pollingInterval) {
            clearInterval(this.pollingInterval);
            this.pollingInterval = null;
        }
        this.connected = false;
    }
}

// Utility functions for updating UI
const RealtimeUI = {
    updateBlockRow: function(block, tbody) {
        if (!tbody || !block) return;
        
        const blockNumber = block.number || block.block_number || block.height || 0;
        const miner = block.miner || block.miner_address || '';
        const txCount = block.tx_count || block.transaction_count || (block.transactions ? block.transactions.length : 0) || 0;
        const consensusType = block.consensus_type || block.consensusType || 'pow';
        
        // Check if row already exists
        const existingRow = tbody.querySelector(`tr[data-block="${blockNumber}"]`);
        if (existingRow) {
            return; // Block already in table
        }
        
        const row = document.createElement('tr');
        row.setAttribute('data-block', blockNumber);
        
        const minerCell = miner && miner.length > 0 ? 
            `<a href="/address/${miner}" class="address-link text-decoration-none">${miner.substring(0, 10)}...</a>` :
            `<span class="text-muted">Unknown</span>`;
        
        row.innerHTML = `
            <td><a href="/block/${blockNumber}" class="text-decoration-none">${blockNumber}</a></td>
            <td class="text-muted small">just now</td>
            <td><span class="badge badge-modern bg-secondary">${txCount}</span></td>
            <td>${minerCell}</td>
            <td>
                ${consensusType === 'pos' ? 
                    '<span class="badge-modern badge-pos"><span class="block-type-indicator pos-indicator"></span>PoS</span>' : 
                    '<span class="badge-modern badge-pow"><span class="block-type-indicator pow-indicator"></span>PoW</span>'}
            </td>
        `;
        
        tbody.insertBefore(row, tbody.firstChild);
        
        // Remove last row if we have more than 10
        while (tbody.rows.length > 10) {
            tbody.removeChild(tbody.lastChild);
        }
    },
    
    updateTransactionRow: function(tx, tbody) {
        if (!tbody || !tx) return;
        
        const txHash = tx.hash || tx.tx_hash || '';
        if (!txHash) return;
        
        // Check if row already exists
        const existingRow = tbody.querySelector(`tr[data-tx="${txHash}"]`);
        if (existingRow) {
            return; // Transaction already in table
        }
        
        const row = document.createElement('tr');
        row.setAttribute('data-tx', txHash);
        
        const fromAddr = tx.from_address || '';
        const toAddr = tx.to_address || '';
        const value = tx.value || 0;
        const isCoinbase = tx.is_coinbase || false;
        
        row.innerHTML = `
            <td>
                <a href="/tx/${txHash}" class="text-decoration-none font-monospace">
                    ${txHash.substring(0, 16)}...
                </a>
            </td>
            <td>
                ${fromAddr ? 
                    `<a href="/address/${fromAddr}" class="text-decoration-none">${fromAddr.substring(0, 10)}...</a>` :
                    '<span class="text-muted">Coinbase</span>'}
            </td>
            <td>
                ${toAddr ? 
                    `<a href="/address/${toAddr}" class="text-decoration-none">${toAddr.substring(0, 10)}...</a>` :
                    '<span class="text-muted">-</span>'}
            </td>
            <td>${value.toFixed(4)} GXC</td>
            <td>
                ${isCoinbase ? 
                    '<span class="badge bg-info">Coinbase</span>' :
                    '<span class="badge bg-primary">Transfer</span>'}
            </td>
            <td>
                <span class="badge bg-warning text-dark" title="Unconfirmed">
                    <i class="fas fa-clock me-1"></i>0/6
                </span>
            </td>
            <td class="text-muted small">just now</td>
        `;
        
        tbody.insertBefore(row, tbody.firstChild);
        
        // Remove last row if we have more than 20
        while (tbody.rows.length > 20) {
            tbody.removeChild(tbody.lastChild);
        }
    },
    
    updateStats: function(stats) {
        // Update statistics display
        if (stats.total_blocks !== undefined) {
            const el = document.getElementById('total-blocks');
            if (el) el.textContent = stats.total_blocks.toLocaleString();
        }
        if (stats.total_transactions !== undefined) {
            const el = document.getElementById('total-transactions');
            if (el) el.textContent = stats.total_transactions.toLocaleString();
        }
        if (stats.total_addresses !== undefined) {
            const el = document.getElementById('total-addresses');
            if (el) el.textContent = stats.total_addresses.toLocaleString();
        }
        if (stats.total_supply !== undefined) {
            const el = document.getElementById('total-supply');
            if (el) el.textContent = stats.total_supply.toLocaleString();
        }
    },
    
    showNotification: function(message, type = 'info') {
        // Show toast notification
        const toastContainer = document.getElementById('toast-container') || document.body;
        const toast = document.createElement('div');
        toast.className = `toast align-items-center text-white bg-${type} border-0`;
        toast.setAttribute('role', 'alert');
        toast.innerHTML = `
            <div class="d-flex">
                <div class="toast-body">${message}</div>
                <button type="button" class="btn-close btn-close-white me-2 m-auto" data-bs-dismiss="toast"></button>
            </div>
        `;
        toastContainer.appendChild(toast);
        const bsToast = new bootstrap.Toast(toast);
        bsToast.show();
        toast.addEventListener('hidden.bs.toast', () => toast.remove());
    }
};
