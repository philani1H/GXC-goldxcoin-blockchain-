#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <fstream>
#include <sstream>
#include "BlockExplorer.h"
#include "Logger.h"
#include "Database.h"
#include "blockchain.h"
#include "Config.h"
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <nlohmann/json.hpp>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;
using json = nlohmann::json;

class GXCExplorer {
private:
    std::unique_ptr<BlockExplorer> explorer_;
    std::unique_ptr<Database> database_;
    std::unique_ptr<Blockchain> blockchain_;
    std::unique_ptr<Config> config_;
    net::io_context ioContext_;
    tcp::acceptor acceptor_;
    bool isRunning_;
    int port_;
    std::thread serverThread_;

public:
    GXCExplorer(int port = 8080) : acceptor_(ioContext_), isRunning_(false), port_(port) {}
    
    ~GXCExplorer() {
        stop();
    }
    
    bool initialize(const std::string& configPath = "config/gxc.conf") {
        try {
            Logger::info("Initializing GXC Explorer...");
            
            // Load configuration
            config_ = std::make_unique<Config>(configPath);
            
            // Initialize database
            std::string dbPath = config_->getDatabasePath();
            database_ = std::make_unique<Database>(dbPath);
            Logger::info("Database connected: " + dbPath);
            
            // Initialize blockchain
            blockchain_ = std::make_unique<Blockchain>(database_.get());
            
            // Initialize explorer
            explorer_ = std::make_unique<BlockExplorer>(database_.get(), blockchain_.get());
            
            Logger::info("GXC Explorer initialized successfully");
            return true;
            
        } catch (const std::exception& e) {
            Logger::error("Failed to initialize explorer: " + std::string(e.what()));
            return false;
        }
    }
    
    bool start() {
        if (isRunning_) {
            Logger::warning("Explorer is already running");
            return false;
        }
        
        try {
            // Set up HTTP server
            tcp::endpoint endpoint{net::ip::make_address("0.0.0.0"), static_cast<unsigned short>(port_)};
            acceptor_.open(endpoint.protocol());
            acceptor_.set_option(net::socket_base::reuse_address(true));
            acceptor_.bind(endpoint);
            acceptor_.listen(net::socket_base::max_listen_connections);
            
            isRunning_ = true;
            
            // Start accepting connections
            startAccept();
            
            // Start server thread
            serverThread_ = std::thread([this]() {
                try {
                    ioContext_.run();
                } catch (const std::exception& e) {
                    Logger::error("Server thread error: " + std::string(e.what()));
                }
            });
            
            Logger::info("GXC Explorer started on http://localhost:" + std::to_string(port_));
            Logger::info("Available routes:");
            printAvailableRoutes();
            
            return true;
            
        } catch (const std::exception& e) {
            Logger::error("Failed to start explorer: " + std::string(e.what()));
            isRunning_ = false;
            return false;
        }
    }
    
    void stop() {
        if (!isRunning_) {
            return;
        }
        
        Logger::info("Stopping GXC Explorer...");
        isRunning_ = false;
        
        acceptor_.close();
        ioContext_.stop();
        
        if (serverThread_.joinable()) {
            serverThread_.join();
        }
        
        Logger::info("GXC Explorer stopped");
    }
    
private:
    void startAccept() {
        auto socket = std::make_shared<tcp::socket>(ioContext_);
        
        acceptor_.async_accept(*socket,
            [this, socket](beast::error_code ec) {
                if (!ec && isRunning_) {
                    std::thread([this, socket]() {
                        handleRequest(socket);
                    }).detach();
                    
                    startAccept();
                } else if (ec != net::error::operation_aborted) {
                    Logger::error("Accept error: " + ec.message());
                    if (isRunning_) {
                        startAccept();
                    }
                }
            });
    }
    
    void handleRequest(std::shared_ptr<tcp::socket> socket) {
        try {
            beast::flat_buffer buffer;
            http::request<http::string_body> request;
            
            // Read the request
            http::read(*socket, buffer, request);
            
            // Process the request and generate response
            auto response = processRequest(request);
            
            // Send the response
            http::write(*socket, response);
            
            // Close the connection
            socket->shutdown(tcp::socket::shutdown_send);
            
        } catch (const std::exception& e) {
            Logger::error("Request handling error: " + std::string(e.what()));
        }
    }
    
    http::response<http::string_body> processRequest(const http::request<http::string_body>& request) {
        http::response<http::string_body> response;
        response.version(request.version());
        response.keep_alive(false);
        
        std::string target = std::string(request.target());
        
        try {
            // Parse URL and query parameters
            auto [path, queryParams] = parseUrl(target);
            
            // Route handling
            if (path == "/" || path == "/index") {
                response = handleHomePage();
            } else if (path == "/api/stats") {
                response = handleApiStats();
            } else if (path == "/api/blocks") {
                response = handleApiBlocks(queryParams);
            } else if (path.substr(0, 11) == "/api/block/") {
                std::string blockHash = path.substr(11);
                response = handleApiBlock(blockHash);
            } else if (path == "/api/transactions") {
                response = handleApiTransactions(queryParams);
            } else if (path.substr(0, 16) == "/api/transaction/") {
                std::string txHash = path.substr(16);
                response = handleApiTransaction(txHash);
            } else if (path == "/api/addresses") {
                response = handleApiAddresses(queryParams);
            } else if (path.substr(0, 13) == "/api/address/") {
                std::string address = path.substr(13);
                response = handleApiAddress(address);
            } else if (path == "/api/miners") {
                response = handleApiMiners(queryParams);
            } else if (path == "/api/mining/rewards") {
                response = handleApiMiningRewards(queryParams);
            } else if (path == "/api/network") {
                response = handleApiNetwork();
            } else if (path == "/api/peers") {
                response = handleApiPeers();
            } else if (path == "/api/validators") {
                response = handleApiValidators();
            } else if (path == "/api/governance") {
                response = handleApiGovernance();
            } else if (path == "/api/tokens") {
                response = handleApiTokens();
            } else if (path == "/api/analytics/transactions") {
                response = handleApiAnalyticsTransactions(queryParams);
            } else if (path == "/api/analytics/mining") {
                response = handleApiAnalyticsMining(queryParams);
            } else if (path == "/api/analytics/network") {
                response = handleApiAnalyticsNetwork(queryParams);
            } else if (path == "/api/analytics/performance") {
                response = handleApiAnalyticsPerformance(queryParams);
            } else if (path == "/api/search") {
                response = handleApiSearch(queryParams);
            } else if (path == "/blocks") {
                response = handleBlocksPage();
            } else if (path == "/transactions") {
                response = handleTransactionsPage();
            } else if (path == "/mining") {
                response = handleMiningPage();
            } else if (path == "/network") {
                response = handleNetworkPage();
            } else if (path == "/analytics") {
                response = handleAnalyticsPage();
            } else {
                response = handle404();
            }
            
        } catch (const std::exception& e) {
            Logger::error("Request processing error: " + std::string(e.what()));
            response = handleError("Internal Server Error: " + std::string(e.what()));
        }
        
        // Add CORS headers
        response.set(http::field::access_control_allow_origin, "*");
        response.set(http::field::access_control_allow_methods, "GET, POST, OPTIONS");
        response.set(http::field::access_control_allow_headers, "Content-Type");
        
        return response;
    }
    
    std::pair<std::string, std::map<std::string, std::string>> parseUrl(const std::string& target) {
        size_t queryPos = target.find('?');
        std::string path = target.substr(0, queryPos);
        std::map<std::string, std::string> params;
        
        if (queryPos != std::string::npos) {
            std::string query = target.substr(queryPos + 1);
            std::stringstream ss(query);
            std::string param;
            
            while (std::getline(ss, param, '&')) {
                size_t equalPos = param.find('=');
                if (equalPos != std::string::npos) {
                    std::string key = param.substr(0, equalPos);
                    std::string value = param.substr(equalPos + 1);
                    params[key] = value;
                }
            }
        }
        
        return {path, params};
    }
    
    // API Route Handlers
    http::response<http::string_body> handleApiStats() {
        json stats;
        
        try {
            stats["network"] = {
                {"blockHeight", blockchain_->getHeight()},
                {"totalTransactions", blockchain_->getTotalTransactionCount()},
                {"networkHashrate", blockchain_->getNetworkHashrate()},
                {"difficulty", blockchain_->getCurrentDifficulty()},
                {"avgBlockTime", blockchain_->getAverageBlockTime()},
                {"totalSupply", explorer_->getTotalSupply()},
                {"circulatingSupply", explorer_->getCirculatingSupply()}
            };
            
            stats["mining"] = {
                {"totalMiners", explorer_->getActiveMinerCount()},
                {"totalRewards", explorer_->getTotalMiningRewards()},
                {"earlyMinerBonuses", explorer_->getEarlyMinerBonuses()},
                {"avgRewardPerBlock", explorer_->getAverageRewardPerBlock()}
            };
            
            stats["validators"] = {
                {"activeValidators", explorer_->getActiveValidatorCount()},
                {"totalStaked", explorer_->getTotalStakedAmount()},
                {"avgValidatorUptime", explorer_->getAverageValidatorUptime()}
            };
            
        } catch (const std::exception& e) {
            Logger::error("Error getting stats: " + std::string(e.what()));
        }
        
        return createJsonResponse(stats);
    }
    
    http::response<http::string_body> handleApiBlocks(const std::map<std::string, std::string>& params) {
        int limit = getParam(params, "limit", 20);
        int offset = getParam(params, "offset", 0);
        
        auto blocks = database_->getBlocks(limit, offset);
        json blocksJson = json::array();
        
        for (const auto& block : blocks) {
            json blockJson;
            blockJson["hash"] = block.hash;
            blockJson["height"] = block.height;
            blockJson["timestamp"] = block.timestamp;
            blockJson["txCount"] = block.transactions.size();
            blockJson["validator"] = block.validator;
            blockJson["difficulty"] = block.difficulty;
            blockJson["size"] = block.size;
            blocksJson.push_back(blockJson);
        }
        
        json response;
        response["blocks"] = blocksJson;
        response["total"] = blockchain_->getHeight();
        response["limit"] = limit;
        response["offset"] = offset;
        
        return createJsonResponse(response);
    }
    
    http::response<http::string_body> handleApiBlock(const std::string& blockHash) {
        try {
            auto blockData = explorer_->getBlockDetails(blockHash);
            return createJsonResponse(blockData);
        } catch (const std::exception& e) {
            return handleError("Block not found: " + blockHash);
        }
    }
    
    http::response<http::string_body> handleApiTransactions(const std::map<std::string, std::string>& params) {
        int limit = getParam(params, "limit", 50);
        int offset = getParam(params, "offset", 0);
        std::string address = getParam(params, "address", "");
        
        auto transactions = explorer_->getTransactions(limit, offset, address);
        json txJson = json::array();
        
        for (const auto& tx : transactions) {
            json transaction;
            transaction["hash"] = tx.hash;
            transaction["from"] = tx.fromAddress;
            transaction["to"] = tx.toAddress;
            transaction["amount"] = tx.amount;
            transaction["fee"] = tx.fee;
            transaction["timestamp"] = tx.timestamp;
            txJson.push_back(transaction);
        }
        
        json response;
        response["transactions"] = txJson;
        response["limit"] = limit;
        response["offset"] = offset;
        
        return createJsonResponse(response);
    }
    
    http::response<http::string_body> handleApiTransaction(const std::string& txHash) {
        try {
            auto txData = explorer_->getTransactionDetails(txHash);
            return createJsonResponse(txData);
        } catch (const std::exception& e) {
            return handleError("Transaction not found: " + txHash);
        }
    }
    
    http::response<http::string_body> handleApiAddress(const std::string& address) {
        try {
            auto addressData = explorer_->getAddressDetails(address);
            return createJsonResponse(addressData);
        } catch (const std::exception& e) {
            return handleError("Address not found: " + address);
        }
    }
    
    http::response<http::string_body> handleApiMiningRewards(const std::map<std::string, std::string>& params) {
        int limit = getParam(params, "limit", 100);
        int offset = getParam(params, "offset", 0);
        std::string minerAddress = getParam(params, "miner", "");
        
        auto rewards = explorer_->getMiningRewards(limit, offset, minerAddress);
        
        json rewardsJson = json::array();
        for (const auto& reward : rewards) {
            json rewardJson;
            rewardJson["blockHash"] = reward.blockHash;
            rewardJson["minerAddress"] = reward.minerAddress;
            rewardJson["amount"] = reward.amount;
            rewardJson["earlyMinerBonus"] = reward.earlyMinerBonus;
            rewardJson["algorithm"] = reward.algorithm;
            rewardJson["timestamp"] = reward.timestamp;
            rewardsJson.push_back(rewardJson);
        }
        
        json response;
        response["rewards"] = rewardsJson;
        response["limit"] = limit;
        response["offset"] = offset;
        
        return createJsonResponse(response);
    }
    
    http::response<http::string_body> handleApiAnalyticsTransactions(const std::map<std::string, std::string>& params) {
        std::string period = getParam(params, "period", "24h");
        
        auto analytics = explorer_->getTransactionAnalytics(period);
        
        json response;
        response["period"] = period;
        response["totalTransactions"] = analytics.totalTransactions;
        response["totalVolume"] = analytics.totalVolume;
        response["avgTransactionSize"] = analytics.avgTransactionSize;
        response["peakTps"] = analytics.peakTps;
        response["hourlyData"] = analytics.hourlyData;
        
        return createJsonResponse(response);
    }
    
    http::response<http::string_body> handleApiAnalyticsMining(const std::map<std::string, std::string>& params) {
        std::string period = getParam(params, "period", "24h");
        
        auto analytics = explorer_->getMiningAnalytics(period);
        
        json response;
        response["period"] = period;
        response["totalBlocks"] = analytics.totalBlocks;
        response["totalRewards"] = analytics.totalRewards;
        response["avgBlockTime"] = analytics.avgBlockTime;
        response["difficultyChanges"] = analytics.difficultyChanges;
        response["topMiners"] = analytics.topMiners;
        response["algorithmDistribution"] = analytics.algorithmDistribution;
        
        return createJsonResponse(response);
    }
    
    http::response<http::string_body> handleApiSearch(const std::map<std::string, std::string>& params) {
        std::string query = getParam(params, "q", "");
        
        if (query.empty()) {
            return handleError("Search query is required");
        }
        
        auto results = explorer_->search(query);
        return createJsonResponse(results);
    }
    
    // HTML Page Handlers
    http::response<http::string_body> handleHomePage() {
        std::string html = generateHomePage();
        return createHtmlResponse(html);
    }
    
    http::response<http::string_body> handleBlocksPage() {
        std::string html = generateBlocksPage();
        return createHtmlResponse(html);
    }
    
    http::response<http::string_body> handleTransactionsPage() {
        std::string html = generateTransactionsPage();
        return createHtmlResponse(html);
    }
    
    http::response<http::string_body> handleMiningPage() {
        std::string html = generateMiningPage();
        return createHtmlResponse(html);
    }
    
    http::response<http::string_body> handleAnalyticsPage() {
        std::string html = generateAnalyticsPage();
        return createHtmlResponse(html);
    }
    
    // Response Helpers
    http::response<http::string_body> createJsonResponse(const json& data) {
        http::response<http::string_body> response{http::status::ok, 11};
        response.set(http::field::content_type, "application/json");
        response.body() = data.dump(2);
        response.prepare_payload();
        return response;
    }
    
    http::response<http::string_body> createHtmlResponse(const std::string& html) {
        http::response<http::string_body> response{http::status::ok, 11};
        response.set(http::field::content_type, "text/html");
        response.body() = html;
        response.prepare_payload();
        return response;
    }
    
    http::response<http::string_body> handle404() {
        http::response<http::string_body> response{http::status::not_found, 11};
        response.set(http::field::content_type, "application/json");
        response.body() = R"({"error": "Not Found"})";
        response.prepare_payload();
        return response;
    }
    
    http::response<http::string_body> handleError(const std::string& message) {
        http::response<http::string_body> response{http::status::internal_server_error, 11};
        response.set(http::field::content_type, "application/json");
        json errorJson;
        errorJson["error"] = message;
        response.body() = errorJson.dump();
        response.prepare_payload();
        return response;
    }
    
    // HTML Generation
    std::string generateHomePage() {
        return R"(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>GXC Blockchain Explorer</title>
    <style>
        body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; margin: 0; background: #f5f7fa; }
        .header { background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); color: white; padding: 2rem; text-align: center; }
        .container { max-width: 1200px; margin: 0 auto; padding: 2rem; }
        .stats-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(250px, 1fr)); gap: 1.5rem; margin: 2rem 0; }
        .stat-card { background: white; padding: 1.5rem; border-radius: 8px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }
        .stat-title { color: #666; font-size: 0.9rem; margin-bottom: 0.5rem; }
        .stat-value { font-size: 2rem; font-weight: bold; color: #333; }
        .nav { background: white; padding: 1rem; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
        .nav a { margin: 0 1rem; text-decoration: none; color: #667eea; font-weight: 500; }
        .search-box { padding: 0.75rem; border: 2px solid #ddd; border-radius: 4px; width: 300px; margin: 1rem 0; }
    </style>
</head>
<body>
    <div class="header">
        <h1>GXC Blockchain Explorer</h1>
        <p>Advanced Cryptocurrency with Hybrid Consensus & Enhanced Early Miner Rewards</p>
    </div>
    
    <div class="nav">
        <a href="/">Home</a>
        <a href="/blocks">Blocks</a>
        <a href="/transactions">Transactions</a>
        <a href="/mining">Mining</a>
        <a href="/network">Network</a>
        <a href="/analytics">Analytics</a>
    </div>
    
    <div class="container">
        <input type="text" class="search-box" placeholder="Search by block hash, transaction hash, or address..." id="searchBox">
        
        <div class="stats-grid" id="statsGrid">
            <div class="stat-card">
                <div class="stat-title">Block Height</div>
                <div class="stat-value" id="blockHeight">Loading...</div>
            </div>
            <div class="stat-card">
                <div class="stat-title">Network Hashrate</div>
                <div class="stat-value" id="hashrate">Loading...</div>
            </div>
            <div class="stat-card">
                <div class="stat-title">Total Transactions</div>
                <div class="stat-value" id="totalTx">Loading...</div>
            </div>
            <div class="stat-card">
                <div class="stat-title">Active Miners</div>
                <div class="stat-value" id="activeMiners">Loading...</div>
            </div>
            <div class="stat-card">
                <div class="stat-title">Early Miner Bonuses</div>
                <div class="stat-value" id="earlyBonus">Loading...</div>
            </div>
            <div class="stat-card">
                <div class="stat-title">Total Supply</div>
                <div class="stat-value" id="totalSupply">Loading...</div>
            </div>
        </div>
        
        <h2>Recent Blocks</h2>
        <div id="recentBlocks">Loading...</div>
        
        <h2>Recent Transactions</h2>
        <div id="recentTransactions">Loading...</div>
    </div>
    
    <script>
        // Load stats
        fetch('/api/stats')
            .then(response => response.json())
            .then(data => {
                document.getElementById('blockHeight').textContent = data.network.blockHeight.toLocaleString();
                document.getElementById('hashrate').textContent = (data.network.networkHashrate / 1e6).toFixed(2) + ' MH/s';
                document.getElementById('totalTx').textContent = data.network.totalTransactions.toLocaleString();
                document.getElementById('activeMiners').textContent = data.mining.totalMiners.toLocaleString();
                document.getElementById('earlyBonus').textContent = data.mining.earlyMinerBonuses.toFixed(2) + ' GXC';
                document.getElementById('totalSupply').textContent = data.network.totalSupply.toFixed(2) + ' GXC';
            });
        
        // Search functionality
        document.getElementById('searchBox').addEventListener('keypress', function(e) {
            if (e.key === 'Enter') {
                const query = this.value;
                if (query) {
                    window.location.href = '/search?q=' + encodeURIComponent(query);
                }
            }
        });
        
        // Auto-refresh every 30 seconds
        setInterval(function() {
            location.reload();
        }, 30000);
    </script>
</body>
</html>
        )";
    }
    
    std::string generateBlocksPage() {
        return R"(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Blocks - GXC Explorer</title>
    <style>
        body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; margin: 0; background: #f5f7fa; }
        .header { background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); color: white; padding: 1rem; text-align: center; }
        .container { max-width: 1200px; margin: 0 auto; padding: 2rem; }
        .block-table { width: 100%; background: white; border-radius: 8px; overflow: hidden; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }
        .block-table th, .block-table td { padding: 1rem; text-align: left; border-bottom: 1px solid #eee; }
        .block-table th { background: #f8f9fa; font-weight: 600; }
        .hash { font-family: monospace; color: #667eea; }
        .timestamp { color: #666; }
    </style>
</head>
<body>
    <div class="header">
        <h1>Recent Blocks</h1>
    </div>
    
    <div class="container">
        <table class="block-table">
            <thead>
                <tr>
                    <th>Height</th>
                    <th>Hash</th>
                    <th>Timestamp</th>
                    <th>Transactions</th>
                    <th>Validator</th>
                    <th>Difficulty</th>
                </tr>
            </thead>
            <tbody id="blocksTable">
                <tr><td colspan="6">Loading...</td></tr>
            </tbody>
        </table>
    </div>
    
    <script>
        fetch('/api/blocks?limit=50')
            .then(response => response.json())
            .then(data => {
                const tbody = document.getElementById('blocksTable');
                tbody.innerHTML = '';
                
                data.blocks.forEach(block => {
                    const row = document.createElement('tr');
                    row.innerHTML = `
                        <td>${block.height}</td>
                        <td class="hash">${block.hash.substring(0, 20)}...</td>
                        <td class="timestamp">${new Date(block.timestamp * 1000).toLocaleString()}</td>
                        <td>${block.txCount}</td>
                        <td class="hash">${block.validator.substring(0, 12)}...</td>
                        <td>${block.difficulty.toFixed(4)}</td>
                    `;
                    tbody.appendChild(row);
                });
            });
    </script>
</body>
</html>
        )";
    }
    
    std::string generateAnalyticsPage() {
        return R"(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Analytics - GXC Explorer</title>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <style>
        body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; margin: 0; background: #f5f7fa; }
        .header { background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); color: white; padding: 1rem; text-align: center; }
        .container { max-width: 1200px; margin: 0 auto; padding: 2rem; }
        .chart-container { background: white; padding: 2rem; border-radius: 8px; margin: 1rem 0; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }
        .chart-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(400px, 1fr)); gap: 2rem; }
    </style>
</head>
<body>
    <div class="header">
        <h1>Blockchain Analytics</h1>
        <p>Advanced tracking and performance metrics</p>
    </div>
    
    <div class="container">
        <div class="chart-grid">
            <div class="chart-container">
                <h3>Transaction Volume (24h)</h3>
                <canvas id="transactionChart"></canvas>
            </div>
            <div class="chart-container">
                <h3>Mining Rewards Distribution</h3>
                <canvas id="miningChart"></canvas>
            </div>
            <div class="chart-container">
                <h3>Network Hashrate</h3>
                <canvas id="hashrateChart"></canvas>
            </div>
            <div class="chart-container">
                <h3>Early Miner Bonus Tracking</h3>
                <canvas id="bonusChart"></canvas>
            </div>
        </div>
    </div>
    
    <script>
        // Initialize charts with sample data
        // In a real implementation, this would fetch data from the analytics APIs
        
        // Transaction Volume Chart
        const txCtx = document.getElementById('transactionChart').getContext('2d');
        new Chart(txCtx, {
            type: 'line',
            data: {
                labels: ['00:00', '04:00', '08:00', '12:00', '16:00', '20:00'],
                datasets: [{
                    label: 'Transactions',
                    data: [45, 78, 123, 156, 134, 98],
                    borderColor: '#667eea',
                    tension: 0.1
                }]
            }
        });
        
        // Mining Distribution Chart
        const miningCtx = document.getElementById('miningChart').getContext('2d');
        new Chart(miningCtx, {
            type: 'doughnut',
            data: {
                labels: ['SHA256', 'Ethash', 'GXHash'],
                datasets: [{
                    data: [30, 25, 45],
                    backgroundColor: ['#ff6384', '#36a2eb', '#cc65fe']
                }]
            }
        });
    </script>
</body>
</html>
        )";
    }
    
    // Utility functions
    int getParam(const std::map<std::string, std::string>& params, const std::string& key, int defaultValue) {
        auto it = params.find(key);
        if (it != params.end()) {
            try {
                return std::stoi(it->second);
            } catch (...) {
                return defaultValue;
            }
        }
        return defaultValue;
    }
    
    std::string getParam(const std::map<std::string, std::string>& params, const std::string& key, const std::string& defaultValue) {
        auto it = params.find(key);
        return (it != params.end()) ? it->second : defaultValue;
    }
    
    void printAvailableRoutes() {
        Logger::info("  / - Home page with network overview");
        Logger::info("  /blocks - Recent blocks listing");
        Logger::info("  /transactions - Recent transactions");
        Logger::info("  /mining - Mining statistics and rewards");
        Logger::info("  /analytics - Advanced blockchain analytics");
        Logger::info("  /api/stats - Network statistics JSON");
        Logger::info("  /api/blocks - Blocks data with pagination");
        Logger::info("  /api/block/{hash} - Specific block details");
        Logger::info("  /api/transactions - Transactions with filtering");
        Logger::info("  /api/transaction/{hash} - Transaction details");
        Logger::info("  /api/address/{address} - Address information");
        Logger::info("  /api/mining/rewards - Mining rewards data");
        Logger::info("  /api/analytics/* - Various analytics endpoints");
        Logger::info("  /api/search?q={query} - Search functionality");
    }
};

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [OPTIONS]\n"
              << "Options:\n"
              << "  -p, --port <port>      Port to listen on (default: 8080)\n"
              << "  -c, --config <path>    Configuration file path\n"
              << "  -h, --help             Show this help message\n"
              << "  -v, --version          Show version information\n";
}

int main(int argc, char* argv[]) {
    int port = 8080;
    std::string configPath = "config/gxc.conf";
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "-p" || arg == "--port") {
            if (i + 1 < argc) {
                port = std::atoi(argv[++i]);
            } else {
                std::cerr << "Error: --port requires a port number\n";
                return 1;
            }
        } else if (arg == "-c" || arg == "--config") {
            if (i + 1 < argc) {
                configPath = argv[++i];
            } else {
                std::cerr << "Error: --config requires a path\n";
                return 1;
            }
        } else if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "-v" || arg == "--version") {
            std::cout << "GXC Blockchain Explorer v2.0.0\n";
            return 0;
        } else {
            std::cerr << "Error: Unknown option " << arg << "\n";
            printUsage(argv[0]);
            return 1;
        }
    }
    
    try {
        // Initialize logging
        Logger::initialize("logs/gxc-explorer.log");
        Logger::info("Starting GXC Blockchain Explorer...");
        
        // Create and initialize explorer
        GXCExplorer explorer(port);
        
        if (!explorer.initialize(configPath)) {
            Logger::error("Failed to initialize explorer");
            return 1;
        }
        
        if (!explorer.start()) {
            Logger::error("Failed to start explorer");
            return 1;
        }
        
        // Keep the server running
        std::cout << "GXC Explorer is running on http://localhost:" << port << std::endl;
        std::cout << "Press Ctrl+C to stop the server" << std::endl;
        
        // Wait for interrupt signal
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        
    } catch (const std::exception& e) {
        Logger::error("Fatal error: " + std::string(e.what()));
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}