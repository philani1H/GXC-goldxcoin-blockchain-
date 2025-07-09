#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/chrono.h>
#include <pybind11/functional.h>

#include "../../include/blockchain.h"
#include "../../include/transaction.h"
#include "../../include/Wallet.h"
#include "../../include/Block.h"
#include "../../include/Network.h"
#include "../../include/Logger.h"
#include "../../include/Config.h"
#include "../../include/HashUtils.h"
#include "../../include/Utils.h"

namespace py = pybind11;

// Python-friendly wrapper classes
class PyBlockchain {
private:
    Blockchain* blockchain;

public:
    PyBlockchain() {
        blockchain = new Blockchain();
    }
    
    ~PyBlockchain() {
        delete blockchain;
    }
    
    bool addBlock(const Block& block) {
        return blockchain->addBlock(block);
    }
    
    Block getLatestBlock() {
        return blockchain->getLatestBlock();
    }
    
    Block getBlock(const std::string& hash) {
        return blockchain->getBlock(hash);
    }
    
    std::vector<Block> getBlocks(int count) {
        return blockchain->getBlocks(count);
    }
    
    bool isValid() {
        return blockchain->isValid();
    }
    
    double getDifficulty() {
        return blockchain->getDifficulty();
    }
    
    std::string getStats() {
        return blockchain->getStats();
    }
    
    int getHeight() {
        return blockchain->getHeight();
    }
    
    bool validateTraceability() {
        return blockchain->validateTraceability();
    }
};

class PyWallet {
private:
    Wallet* wallet;

public:
    PyWallet() {
        wallet = new Wallet();
    }
    
    ~PyWallet() {
        delete wallet;
    }
    
    std::string generateAddress() {
        return wallet->generateAddress();
    }
    
    double getBalance(const std::string& address) {
        return wallet->getBalance(address);
    }
    
    std::string sendTransaction(const std::string& from, const std::string& to, double amount) {
        return wallet->sendTransaction(from, to, amount);
    }
    
    std::vector<Transaction> getTransactions(const std::string& address) {
        return wallet->getTransactions(address);
    }
    
    bool importPrivateKey(const std::string& privateKey) {
        return wallet->importPrivateKey(privateKey);
    }
    
    std::string exportPrivateKey(const std::string& address) {
        return wallet->exportPrivateKey(address);
    }
    
    std::vector<std::string> listAddresses() {
        return wallet->listAddresses();
    }
    
    bool backup(const std::string& filename) {
        return wallet->backup(filename);
    }
    
    bool restore(const std::string& filename) {
        return wallet->restore(filename);
    }
};

class PyMiner {
private:
    MiningManager* manager;

public:
    PyMiner() {
        manager = new MiningManager();
    }
    
    ~PyMiner() {
        delete manager;
    }
    
    bool startMining(const std::string& algorithm, int threads) {
        MiningAlgorithm algo = MiningAlgorithm::SHA256;
        if (algorithm == "ethash") algo = MiningAlgorithm::ETHASH;
        else if (algorithm == "gxhash") algo = MiningAlgorithm::GXHASH;
        
        return manager->startMining(algo, threads);
    }
    
    void stopMining() {
        manager->stopMining();
    }
    
    py::dict getStats() {
        auto stats = manager->getStats();
        py::dict result;
        result["hash_rate"] = stats.hashRate;
        result["total_hashes"] = stats.totalHashes;
        result["threads_active"] = stats.threadsActive;
        result["uptime"] = stats.uptime;
        result["algorithm"] = static_cast<int>(stats.algorithm);
        return result;
    }
    
    bool isMining() {
        return manager->isMining();
    }
    
    void setPoolInfo(const std::string& url, const std::string& username, const std::string& password) {
        manager->setPoolInfo(url, username, password);
    }
};

class PyNetwork {
private:
    Network* network;

public:
    PyNetwork() {
        network = new Network();
    }
    
    ~PyNetwork() {
        delete network;
    }
    
    bool start(int port) {
        return network->start(port);
    }
    
    void stop() {
        network->stop();
    }
    
    bool connectToPeer(const std::string& host, int port) {
        return network->connectToPeer(host, port);
    }
    
    std::vector<std::string> getPeers() {
        return network->getPeers();
    }
    
    py::dict getNetworkStats() {
        auto stats = network->getNetworkStats();
        py::dict result;
        result["peers_connected"] = stats.peersConnected;
        result["bytes_sent"] = stats.bytesSent;
        result["bytes_received"] = stats.bytesReceived;
        result["messages_sent"] = stats.messagesSent;
        result["messages_received"] = stats.messagesReceived;
        return result;
    }
    
    bool broadcastTransaction(const Transaction& tx) {
        return network->broadcastTransaction(tx);
    }
    
    bool broadcastBlock(const Block& block) {
        return network->broadcastBlock(block);
    }
};

// Utility functions for Python
py::dict getSystemInfo() {
    py::dict info;
    info["version"] = "1.0.0";
    info["build_date"] = __DATE__;
    info["build_time"] = __TIME__;
    info["platform"] = 
        #ifdef _WIN32
            "Windows";
        #elif __linux__
            "Linux";
        #elif __APPLE__
            "macOS";
        #else
            "Unknown";
        #endif
    
    info["features"] = py::list();
    py::list features = info["features"];
    features.append("Traceability");
    features.append("Multi-Algorithm Mining");
    features.append("P2P Network");
    features.append("Smart Contracts");
    features.append("Governance");
    
    return info;
}

std::string hashData(const std::string& data, const std::string& algorithm) {
    if (algorithm == "sha256") {
        return HashUtils::sha256(data);
    } else if (algorithm == "ripemd160") {
        return HashUtils::ripemd160(data);
    } else {
        throw std::runtime_error("Unsupported hash algorithm: " + algorithm);
    }
}

py::dict validateTraceabilityPython(const Transaction& tx) {
    py::dict result;
    result["valid"] = tx.isTraceabilityValid();
    result["formula"] = "Ti.Inputs[0].txHash == Ti.PrevTxHash && Ti.Inputs[0].amount == Ti.ReferencedAmount";
    result["prev_tx_hash"] = tx.getPrevTxHash();
    result["referenced_amount"] = tx.getReferencedAmount();
    
    if (!tx.getInputs().empty()) {
        result["input_tx_hash"] = tx.getInputs()[0].txHash;
        result["input_amount"] = tx.getInputs()[0].amount;
    }
    
    return result;
}

// Initialize Python logging
void initializePythonLogging(const std::string& level) {
    Logger::initialize();
    
    LogLevel logLevel = LogLevel::INFO;
    if (level == "DEBUG") logLevel = LogLevel::DEBUG;
    else if (level == "WARNING") logLevel = LogLevel::WARNING;
    else if (level == "ERROR") logLevel = LogLevel::ERROR;
    
    Logger::setLogLevel(logLevel);
}

PYBIND11_MODULE(pygxc, m) {
    m.doc() = "GXC Blockchain Python Bindings - Advanced Cryptocurrency with Traceability";
    
    // Version info
    m.attr("__version__") = "1.0.0";
    
    // Utility functions
    m.def("get_system_info", &getSystemInfo, "Get system and build information");
    m.def("hash_data", &hashData, "Hash data using specified algorithm", 
          py::arg("data"), py::arg("algorithm") = "sha256");
    m.def("validate_traceability", &validateTraceabilityPython, 
          "Validate transaction traceability using GXC formula");
    m.def("initialize_logging", &initializePythonLogging, 
          "Initialize logging system", py::arg("level") = "INFO");
    m.def("get_current_timestamp", &Utils::getCurrentTimestamp, "Get current Unix timestamp");
    m.def("format_amount", &Utils::formatAmount, "Format amount with precision", 
          py::arg("amount"), py::arg("precision") = 8);
    
    // Transaction class
    py::class_<Transaction>(m, "Transaction")
        .def(py::init<>())
        .def("get_hash", &Transaction::getHash)
        .def("get_prev_tx_hash", &Transaction::getPrevTxHash)
        .def("get_referenced_amount", &Transaction::getReferencedAmount)
        .def("get_timestamp", &Transaction::getTimestamp)
        .def("is_traceability_valid", &Transaction::isTraceabilityValid)
        .def("is_coinbase_transaction", &Transaction::isCoinbaseTransaction)
        .def("to_json", &Transaction::toJson)
        .def("from_json", &Transaction::fromJson)
        .def("get_fee", &Transaction::getFee)
        .def("get_total_input_amount", &Transaction::getTotalInputAmount)
        .def("get_total_output_amount", &Transaction::getTotalOutputAmount);
    
    // Block class
    py::class_<Block>(m, "Block")
        .def(py::init<>())
        .def("get_hash", &Block::getHash)
        .def("get_previous_hash", &Block::getPreviousHash)
        .def("get_merkle_root", &Block::getMerkleRoot)
        .def("get_timestamp", &Block::getTimestamp)
        .def("get_nonce", &Block::getNonce)
        .def("get_difficulty", &Block::getDifficulty)
        .def("get_height", &Block::getHeight)
        .def("is_valid", &Block::isValid)
        .def("get_transaction_count", &Block::getTransactionCount)
        .def("calculate_merkle_root", &Block::calculateMerkleRoot)
        .def("to_json", &Block::toJson)
        .def("from_json", &Block::fromJson);
    
    // Blockchain wrapper
    py::class_<PyBlockchain>(m, "Blockchain")
        .def(py::init<>())
        .def("add_block", &PyBlockchain::addBlock, "Add block to blockchain")
        .def("get_latest_block", &PyBlockchain::getLatestBlock, "Get latest block")
        .def("get_block", &PyBlockchain::getBlock, "Get block by hash")
        .def("get_blocks", &PyBlockchain::getBlocks, "Get recent blocks")
        .def("is_valid", &PyBlockchain::isValid, "Validate entire blockchain")
        .def("get_difficulty", &PyBlockchain::getDifficulty, "Get current difficulty")
        .def("get_stats", &PyBlockchain::getStats, "Get blockchain statistics")
        .def("get_height", &PyBlockchain::getHeight, "Get blockchain height")
        .def("validate_traceability", &PyBlockchain::validateTraceability, 
             "Validate traceability for all transactions");
    
    // Wallet wrapper
    py::class_<PyWallet>(m, "Wallet")
        .def(py::init<>())
        .def("generate_address", &PyWallet::generateAddress, "Generate new address")
        .def("get_balance", &PyWallet::getBalance, "Get address balance")
        .def("send_transaction", &PyWallet::sendTransaction, "Send transaction")
        .def("get_transactions", &PyWallet::getTransactions, "Get address transactions")
        .def("import_private_key", &PyWallet::importPrivateKey, "Import private key")
        .def("export_private_key", &PyWallet::exportPrivateKey, "Export private key")
        .def("list_addresses", &PyWallet::listAddresses, "List all addresses")
        .def("backup", &PyWallet::backup, "Backup wallet to file")
        .def("restore", &PyWallet::restore, "Restore wallet from file");
    
    // Miner wrapper
    py::class_<PyMiner>(m, "Miner")
        .def(py::init<>())
        .def("start_mining", &PyMiner::startMining, "Start mining", 
             py::arg("algorithm") = "gxhash", py::arg("threads") = 0)
        .def("stop_mining", &PyMiner::stopMining, "Stop mining")
        .def("get_stats", &PyMiner::getStats, "Get mining statistics")
        .def("is_mining", &PyMiner::isMining, "Check if mining is active")
        .def("set_pool_info", &PyMiner::setPoolInfo, "Set mining pool information");
    
    // Network wrapper
    py::class_<PyNetwork>(m, "Network")
        .def(py::init<>())
        .def("start", &PyNetwork::start, "Start network service")
        .def("stop", &PyNetwork::stop, "Stop network service")
        .def("connect_to_peer", &PyNetwork::connectToPeer, "Connect to peer")
        .def("get_peers", &PyNetwork::getPeers, "Get connected peers")
        .def("get_network_stats", &PyNetwork::getNetworkStats, "Get network statistics")
        .def("broadcast_transaction", &PyNetwork::broadcastTransaction, "Broadcast transaction")
        .def("broadcast_block", &PyNetwork::broadcastBlock, "Broadcast block");
    
    // Constants
    m.attr("TRACEABILITY_FORMULA") = "Ti.Inputs[0].txHash == Ti.PrevTxHash && Ti.Inputs[0].amount == Ti.ReferencedAmount";
    m.attr("BLOCK_TIME") = 600; // 10 minutes
    m.attr("MAX_BLOCK_SIZE") = 1048576; // 1 MB
    m.attr("COINBASE_MATURITY") = 100; // blocks
    
    // Enums
    py::enum_<LogLevel>(m, "LogLevel")
        .value("DEBUG", LogLevel::DEBUG)
        .value("INFO", LogLevel::INFO)
        .value("WARNING", LogLevel::WARNING)
        .value("ERROR", LogLevel::ERROR);
}

// Additional convenience functions for Python
extern "C" {
    // C-style API for easier integration
    
    int gxc_validate_transaction_traceability(const char* tx_json) {
        try {
            Transaction tx;
            tx.fromJson(std::string(tx_json));
            return tx.isTraceabilityValid() ? 1 : 0;
        } catch (...) {
            return -1; // Error
        }
    }
    
    const char* gxc_hash_data_c(const char* data) {
        static std::string result;
        try {
            result = HashUtils::sha256(std::string(data));
            return result.c_str();
        } catch (...) {
            return nullptr;
        }
    }
    
    double gxc_get_current_timestamp_c() {
        return static_cast<double>(Utils::getCurrentTimestamp());
    }
}