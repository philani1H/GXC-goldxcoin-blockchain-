#include "../include/Stratum.h"
#include "../include/Logger.h"
#include "../include/Utils.h"
#include "../include/HashUtils.h"
#include <sstream>
#include <iomanip>
#include <random>

StratumServer::StratumServer(Blockchain* blockchain, uint16_t port)
    : blockchain(blockchain), serverPort(port), isRunning(false), 
      difficulty(10000.0), nextJobId(1) {
    LOG_MINING(LogLevel::INFO, "Stratum server initialized on port " + std::to_string(port));
}

StratumServer::~StratumServer() {
    stop();
}

bool StratumServer::start() {
    if (isRunning) {
        LOG_MINING(LogLevel::WARNING, "Stratum server already running");
        return true;
    }
    
    LOG_MINING(LogLevel::INFO, "Starting Stratum server");
    
    try {
        isRunning = true;
        
        // Start server thread
        serverThread = std::thread(&StratumServer::serverLoop, this);
        
        // Start job generation thread
        jobThread = std::thread(&StratumServer::jobGenerationLoop, this);
        
        LOG_MINING(LogLevel::INFO, "Stratum server started successfully on port " + std::to_string(serverPort));
        return true;
        
    } catch (const std::exception& e) {
        LOG_MINING(LogLevel::ERROR, "Failed to start Stratum server: " + std::string(e.what()));
        isRunning = false;
        return false;
    }
}

void StratumServer::stop() {
    if (!isRunning) {
        return;
    }
    
    LOG_MINING(LogLevel::INFO, "Stopping Stratum server");
    
    isRunning = false;
    
    // Disconnect all miners
    {
        std::lock_guard<std::mutex> lock(minersMutex);
        for (auto& miner : miners) {
            miner.second.connected = false;
        }
        miners.clear();
    }
    
    // Stop threads
    if (serverThread.joinable()) {
        serverThread.join();
    }
    
    if (jobThread.joinable()) {
        jobThread.join();
    }
    
    LOG_MINING(LogLevel::INFO, "Stratum server stopped");
}

void StratumServer::addMiner(const std::string& minerId, const std::string& address) {
    std::lock_guard<std::mutex> lock(minersMutex);
    
    if (miners.find(minerId) != miners.end()) {
        LOG_MINING(LogLevel::WARNING, "Miner already connected: " + minerId);
        return;
    }
    
    StratumMiner miner;
    miner.id = minerId;
    miner.address = address;
    miner.connected = true;
    miner.authorized = false;
    miner.connectTime = Utils::getCurrentTimestamp();
    miner.lastActivity = Utils::getCurrentTimestamp();
    miner.difficulty = difficulty;
    miner.hashRate = 0.0;
    miner.sharesAccepted = 0;
    miner.sharesRejected = 0;
    
    miners[minerId] = miner;
    
    LOG_MINING(LogLevel::INFO, "Added miner: " + minerId + " from " + address);
}

void StratumServer::removeMiner(const std::string& minerId) {
    std::lock_guard<std::mutex> lock(minersMutex);
    
    auto it = miners.find(minerId);
    if (it != miners.end()) {
        miners.erase(it);
        LOG_MINING(LogLevel::INFO, "Removed miner: " + minerId);
    }
}

bool StratumServer::authorizeMiner(const std::string& minerId, const std::string& username, const std::string& password) {
    std::lock_guard<std::mutex> lock(minersMutex);
    
    auto it = miners.find(minerId);
    if (it == miners.end()) {
        return false;
    }
    
    // Simple authorization - in real implementation would check credentials
    if (!username.empty()) {
        it->second.authorized = true;
        it->second.username = username;
        it->second.lastActivity = Utils::getCurrentTimestamp();
        
        LOG_MINING(LogLevel::INFO, "Authorized miner: " + minerId + " (username: " + username + ")");
        
        // Send initial mining job
        sendMiningJob(minerId);
        
        return true;
    }
    
    return false;
}

void StratumServer::submitShare(const std::string& minerId, const StratumShare& share) {
    std::lock_guard<std::mutex> lock(minersMutex);
    
    auto it = miners.find(minerId);
    if (it == miners.end() || !it->second.authorized) {
        LOG_MINING(LogLevel::WARNING, "Share submitted by unauthorized miner: " + minerId);
        return;
    }
    
    // Update miner activity
    it->second.lastActivity = Utils::getCurrentTimestamp();
    
    // Validate share
    if (validateShare(share)) {
        it->second.sharesAccepted++;
        
        // Check if share meets blockchain difficulty
        if (shareIsBlockSolution(share)) {
            // Create and submit block
            submitBlockSolution(share);
        }
        
        // Send success response
        sendShareResponse(minerId, true, "Share accepted");
        
        LOG_MINING(LogLevel::INFO, "Accepted share from miner: " + minerId);
    } else {
        it->second.sharesRejected++;
        
        // Send failure response
        sendShareResponse(minerId, false, "Invalid share");
        
        LOG_MINING(LogLevel::WARNING, "Rejected share from miner: " + minerId);
    }
    
    // Update miner hash rate
    updateMinerHashRate(minerId);
}

void StratumServer::broadcastJob(const StratumJob& job) {
    std::lock_guard<std::mutex> lock(minersMutex);
    
    for (auto& miner : miners) {
        if (miner.second.connected && miner.second.authorized) {
            sendJobToMiner(miner.first, job);
        }
    }
    
    LOG_MINING(LogLevel::INFO, "Broadcasted job " + job.jobId + " to " + std::to_string(miners.size()) + " miners");
}

std::vector<StratumMiner> StratumServer::getConnectedMiners() const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(minersMutex));
    
    std::vector<StratumMiner> connectedMiners;
    for (const auto& miner : miners) {
        if (miner.second.connected) {
            connectedMiners.push_back(miner.second);
        }
    }
    
    return connectedMiners;
}

StratumStats StratumServer::getStats() const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(minersMutex));
    
    StratumStats stats;
    stats.connectedMiners = 0;
    stats.totalHashRate = 0.0;
    stats.totalShares = 0;
    stats.acceptedShares = 0;
    stats.rejectedShares = 0;
    stats.blocksFound = blocksFound;
    
    for (const auto& miner : miners) {
        if (miner.second.connected) {
            stats.connectedMiners++;
            stats.totalHashRate += miner.second.hashRate;
            stats.totalShares += miner.second.sharesAccepted + miner.second.sharesRejected;
            stats.acceptedShares += miner.second.sharesAccepted;
            stats.rejectedShares += miner.second.sharesRejected;
        }
    }
    
    return stats;
}

void StratumServer::serverLoop() {
    LOG_MINING(LogLevel::INFO, "Stratum server loop started");
    
    while (isRunning) {
        try {
            // Handle incoming connections
            handleIncomingConnections();
            
            // Process miner messages
            processMinerMessages();
            
            // Clean up inactive miners
            cleanupInactiveMiners();
            
            // Update statistics
            updateStatistics();
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
        } catch (const std::exception& e) {
            LOG_MINING(LogLevel::ERROR, "Error in Stratum server loop: " + std::string(e.what()));
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    
    LOG_MINING(LogLevel::INFO, "Stratum server loop stopped");
}

void StratumServer::jobGenerationLoop() {
    LOG_MINING(LogLevel::INFO, "Stratum job generation loop started");
    
    while (isRunning) {
        try {
            // Generate new mining job
            StratumJob job = generateMiningJob();
            
            // Broadcast to all miners
            broadcastJob(job);
            
            // Store current job
            {
                std::lock_guard<std::mutex> lock(jobsMutex);
                currentJobs[job.jobId] = job;
                
                // Clean old jobs (keep last 10)
                if (currentJobs.size() > 10) {
                    auto oldest = currentJobs.begin();
                    currentJobs.erase(oldest);
                }
            }
            
            // Wait before next job (30 seconds)
            std::this_thread::sleep_for(std::chrono::seconds(30));
            
        } catch (const std::exception& e) {
            LOG_MINING(LogLevel::ERROR, "Error in job generation loop: " + std::string(e.what()));
            std::this_thread::sleep_for(std::chrono::seconds(10));
        }
    }
    
    LOG_MINING(LogLevel::INFO, "Stratum job generation loop stopped");
}

void StratumServer::handleIncomingConnections() {
    // Simplified connection handling - would handle actual TCP connections
    static uint32_t connectionCounter = 0;
    static auto lastConnectionTime = Utils::getCurrentTimestamp();
    
    auto currentTime = Utils::getCurrentTimestamp();
    
    // Simulate new miner connections every 60 seconds
    if (currentTime - lastConnectionTime > 60 && miners.size() < 10) {
        std::string minerId = "miner_" + std::to_string(++connectionCounter);
        std::string address = "192.168.1." + std::to_string(100 + (connectionCounter % 50));
        
        addMiner(minerId, address);
        lastConnectionTime = currentTime;
    }
}

void StratumServer::processMinerMessages() {
    // Simplified message processing - would handle actual JSON-RPC messages
    std::lock_guard<std::mutex> lock(minersMutex);
    
    for (auto& miner : miners) {
        if (miner.second.connected && !miner.second.authorized) {
            // Auto-authorize for demo purposes
            miner.second.authorized = true;
            miner.second.username = "demo_user_" + miner.first;
            sendMiningJob(miner.first);
        }
    }
}

void StratumServer::cleanupInactiveMiners() {
    std::lock_guard<std::mutex> lock(minersMutex);
    
    auto currentTime = Utils::getCurrentTimestamp();
    
    for (auto it = miners.begin(); it != miners.end();) {
        // Remove miners inactive for more than 5 minutes
        if (currentTime - it->second.lastActivity > 300) {
            LOG_MINING(LogLevel::INFO, "Removing inactive miner: " + it->first);
            it = miners.erase(it);
        } else {
            ++it;
        }
    }
}

void StratumServer::updateStatistics() {
    // Update global statistics
    auto stats = getStats();
    LOG_MINING(LogLevel::DEBUG, "Pool stats - Miners: " + std::to_string(stats.connectedMiners) + 
                                ", Hash rate: " + std::to_string(static_cast<uint64_t>(stats.totalHashRate)) + " H/s");
}

StratumJob StratumServer::generateMiningJob() {
    StratumJob job;
    job.jobId = std::to_string(nextJobId++);
    job.prevBlockHash = blockchain->getLatestBlock().getHash();
    job.timestamp = Utils::getCurrentTimestamp();
    job.difficulty = difficulty;
    job.height = blockchain->getHeight();
    
    // Get pending transactions for the block
    auto pendingTx = blockchain->getPendingTransactions(100);
    for (const auto& tx : pendingTx) {
        if (tx.isTraceabilityValid()) {
            job.transactions.push_back(tx);
        }
    }
    
    // Limit transactions per job
    if (job.transactions.size() > 100) {
        job.transactions.resize(100);
    }
    
    // Create coinbase transaction
    Transaction coinbase;
    coinbase.setReceiverAddress("pool_address");
    // Note: blockReward would need to be set via transaction outputs
    job.coinbaseTransaction = coinbase;
    
    // Calculate merkle root
    job.merkleRoot = calculateMerkleRoot(job.transactions);
    
    // Set mining parameters
    job.nonceMask = 0xFFFFFFFF;
    job.extraNonce1 = generateExtraNonce();
    job.extraNonce2Size = 4;
    
    return job;
}

void StratumServer::sendMiningJob(const std::string& minerId) {
    StratumJob job = generateMiningJob();
    sendJobToMiner(minerId, job);
    
    // Store job for this miner
    std::lock_guard<std::mutex> lock(jobsMutex);
    currentJobs[job.jobId] = job;
}

void StratumServer::sendJobToMiner(const std::string& minerId, const StratumJob& job) {
    // Create JSON-RPC job notification
    std::string message = createJobMessage(job);
    
    // In real implementation, would send over TCP connection
    LOG_MINING(LogLevel::DEBUG, "Sent job " + job.jobId + " to miner " + minerId);
}

void StratumServer::sendShareResponse(const std::string& minerId, bool accepted, const std::string& reason) {
    // Create JSON-RPC response
    std::string message = createShareResponse(accepted, reason);
    
    // In real implementation, would send over TCP connection
    LOG_MINING(LogLevel::DEBUG, "Sent share response to miner " + minerId + ": " + 
                                (accepted ? "accepted" : "rejected"));
}

bool StratumServer::validateShare(const StratumShare& share) {
    // Find the job
    std::lock_guard<std::mutex> lock(jobsMutex);
    
    auto it = currentJobs.find(share.jobId);
    if (it == currentJobs.end()) {
        LOG_MINING(LogLevel::WARNING, "Unknown job ID: " + share.jobId);
        return false;
    }
    
    const StratumJob& job = it->second;
    
    // Basic validation
    if (share.nonce.empty()) {
        return false;
    }
    
    // Check if share meets the minimum difficulty
    std::string blockHeader = constructBlockHeader(job, share);
    std::string hash = sha256(blockHeader);
    
    return checkDifficulty(hash, share.difficulty);
}

bool StratumServer::shareIsBlockSolution(const StratumShare& share) {
    // Check if share meets blockchain difficulty (higher than pool difficulty)
    std::lock_guard<std::mutex> lock(jobsMutex);
    
    auto it = currentJobs.find(share.jobId);
    if (it == currentJobs.end()) {
        return false;
    }
    
    const StratumJob& job = it->second;
    std::string blockHeader = constructBlockHeader(job, share);
    std::string hash = sha256(blockHeader);
    
    double blockchainDifficulty = blockchain->getDifficulty();
    return checkDifficulty(hash, blockchainDifficulty);
}

void StratumServer::submitBlockSolution(const StratumShare& share) {
    std::lock_guard<std::mutex> lock(jobsMutex);
    
    auto it = currentJobs.find(share.jobId);
    if (it == currentJobs.end()) {
        return;
    }
    
    const StratumJob& job = it->second;
    
    try {
        // Create block from job and share
        Block newBlock = createBlockFromShare(job, share);
        
        // Submit to blockchain
        if (blockchain->addBlock(newBlock)) {
            blocksFound++;
            LOG_MINING(LogLevel::INFO, "Successfully mined block " + std::to_string(newBlock.getIndex()) + 
                                      " via pool");
        } else {
            LOG_MINING(LogLevel::ERROR, "Failed to add mined block to blockchain");
        }
        
    } catch (const std::exception& e) {
        LOG_MINING(LogLevel::ERROR, "Error submitting block solution: " + std::string(e.what()));
    }
}

Block StratumServer::createBlockFromShare(const StratumJob& job, const StratumShare& share) {
    // Create block with job transactions
    std::vector<Transaction> blockTxs = job.transactions;
    blockTxs.insert(blockTxs.begin(), job.coinbaseTransaction);
    
    Block block(job.height, job.prevBlockHash, BlockType::POW_SHA256);
    for (const auto& tx : blockTxs) {
        block.addTransaction(tx);
    }
    block.setTimestamp(static_cast<std::time_t>(job.timestamp));
    // Note: nonce is uint64_t, share.nonce is string - would need conversion
    // For now, set a default nonce
    block.setNonce(0);
    block.setDifficulty(job.difficulty);
    block.setMinerAddress("pool_miner");
    
    return block;
}

void StratumServer::updateMinerHashRate(const std::string& minerId) {
    std::lock_guard<std::mutex> lock(minersMutex);
    
    auto it = miners.find(minerId);
    if (it == miners.end()) {
        return;
    }
    
    auto& miner = it->second;
    auto currentTime = Utils::getCurrentTimestamp();
    auto elapsed = currentTime - miner.connectTime;
    
    if (elapsed > 0) {
        uint64_t totalShares = miner.sharesAccepted + miner.sharesRejected;
        miner.hashRate = static_cast<double>(totalShares * miner.difficulty) / elapsed;
    }
}

std::string StratumServer::calculateMerkleRoot(const std::vector<Transaction>& transactions) {
    if (transactions.empty()) {
        return "0000000000000000000000000000000000000000000000000000000000000000";
    }
    
    // Simplified merkle root calculation
    std::string combined;
    for (const auto& tx : transactions) {
        combined += tx.getHash();
    }
    
    return sha256(combined);
}

std::string StratumServer::generateExtraNonce() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dis;
    
    uint32_t nonce = dis(gen);
    
    std::ostringstream oss;
    oss << std::hex << std::setfill('0') << std::setw(8) << nonce;
    
    return oss.str();
}

std::string StratumServer::constructBlockHeader(const StratumJob& job, const StratumShare& share) {
    std::ostringstream oss;
    oss << job.prevBlockHash
        << job.merkleRoot
        << job.timestamp
        << job.difficulty
        << share.nonce
        << share.extraNonce2;
    
    return oss.str();
}

bool StratumServer::checkDifficulty(const std::string& hash, double difficulty) {
    // Simplified difficulty check - count leading zeros
    int leadingZeros = 0;
    for (char c : hash) {
        if (c == '0') {
            leadingZeros++;
        } else {
            break;
        }
    }
    
    int requiredZeros = static_cast<int>(difficulty / 1000.0) + 4;
    return leadingZeros >= requiredZeros;
}

std::string StratumServer::createJobMessage(const StratumJob& job) {
    std::ostringstream oss;
    oss << R"({
        "id": null,
        "method": "mining.notify",
        "params": [
            ")" << job.jobId << R"(",
            ")" << job.prevBlockHash << R"(",
            ")" << job.merkleRoot << R"(",
            ")" << job.timestamp << R"(",
            ")" << std::hex << static_cast<uint32_t>(job.difficulty) << R"(",
            true
        ]
    })";
    
    return oss.str();
}

std::string StratumServer::createShareResponse(bool accepted, const std::string& reason) {
    std::ostringstream oss;
    oss << R"({
        "id": 1,
        "result": )" << (accepted ? "true" : "false") << R"(,
        "error": )" << (accepted ? "null" : "\"" + reason + "\"") << R"(
    })";
    
    return oss.str();
}