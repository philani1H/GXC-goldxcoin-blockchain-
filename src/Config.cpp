#include "../include/Config.h"
#include "../include/Logger.h"
#include <fstream>
#include <sstream>
#include <algorithm>

// Configuration constants
const std::string Config::DEFAULT_CONFIG_FILE = "gxc.conf";
const std::string Config::DEFAULT_DATA_DIR = "~/.gxc";
const uint16_t Config::DEFAULT_PORT = 8333;
const uint16_t Config::DEFAULT_RPC_PORT = 8332;
const uint16_t Config::DEFAULT_REST_PORT = 8334;
const uint32_t Config::DEFAULT_MAX_CONNECTIONS = 125;
const double Config::DEFAULT_MIN_TX_FEE = 0.00001;
const uint32_t Config::DEFAULT_BLOCK_TIME = 600; // 10 minutes

Config::Config(const std::string& configPath) : configFilePath(configPath) {
    setDefaults();
}

bool Config::loadConfig() {
    std::ifstream file(configFilePath);
    if (!file.is_open()) {
        Logger::getInstance().warning("Config file not found, using defaults: " + configFilePath);
        return saveConfig(); // Create default config file
    }
    
    std::string line;
    while (std::getline(file, line)) {
        line = trim(line);
        
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = trim(line.substr(0, pos));
            std::string value = trim(line.substr(pos + 1));
            configValues[key] = value;
        }
    }
    
    Logger::getInstance().info("Configuration loaded from: " + configFilePath);
    return true;
}

bool Config::saveConfig() {
    std::ofstream file(configFilePath);
    if (!file.is_open()) {
        Logger::getInstance().error("Failed to save config file: " + configFilePath);
        return false;
    }
    
    file << "# GXC Blockchain Configuration File\n";
    file << "# Generated automatically - edit carefully\n\n";
    
    file << "# Network Configuration\n";
    file << "listen_address=" << getString("listen_address") << "\n";
    file << "listen_port=" << getInt("listen_port") << "\n";
    file << "max_connections=" << getInt("max_connections") << "\n";
    file << "testnet=" << (getBool("testnet") ? "1" : "0") << "\n\n";
    
    file << "# Mining Configuration\n";
    file << "mining_enabled=" << (getBool("mining_enabled") ? "1" : "0") << "\n";
    file << "miner_address=" << getString("miner_address") << "\n";
    file << "mining_threads=" << getInt("mining_threads") << "\n";
    file << "mining_algorithm=" << getString("mining_algorithm") << "\n\n";
    
    file << "# Database Configuration\n";
    file << "data_directory=" << getString("data_directory") << "\n";
    file << "database_cache_size=" << getInt("database_cache_size") << "\n\n";
    
    file << "# API Configuration\n";
    file << "rpc_enabled=" << (getBool("rpc_enabled") ? "1" : "0") << "\n";
    file << "rpc_address=" << getString("rpc_address") << "\n";
    file << "rpc_port=" << getInt("rpc_port") << "\n";
    file << "rest_enabled=" << (getBool("rest_enabled") ? "1" : "0") << "\n";
    file << "rest_port=" << getInt("rest_port") << "\n\n";
    
    file << "# Logging Configuration\n";
    file << "log_level=" << getString("log_level") << "\n";
    file << "log_file=" << getString("log_file") << "\n";
    file << "log_max_size=" << getInt("log_max_size") << "\n";
    file << "log_max_files=" << getInt("log_max_files") << "\n\n";
    
    Logger::getInstance().info("Configuration saved to: " + configFilePath);
    return true;
}

void Config::setDefaults() {
    // Network defaults
    configValues["listen_address"] = "0.0.0.0";
    configValues["listen_port"] = std::to_string(DEFAULT_PORT);
    configValues["max_connections"] = std::to_string(DEFAULT_MAX_CONNECTIONS);
    configValues["testnet"] = "0";
    
    // Mining defaults
    configValues["mining_enabled"] = "0";
    configValues["miner_address"] = "";
    configValues["mining_threads"] = "0"; // Auto-detect
    configValues["mining_algorithm"] = "sha256";
    configValues["pool_address"] = "";
    configValues["gpu_mining_enabled"] = "0";
    
    // Wallet defaults
    configValues["wallet_file"] = "wallet.dat";
    configValues["wallet_encrypted"] = "0";
    configValues["wallet_backup_interval"] = "3600"; // 1 hour
    
    // Database defaults
    configValues["data_directory"] = DEFAULT_DATA_DIR;
    configValues["database_cache_size"] = "100"; // MB
    configValues["database_compression"] = "1";
    
    // Consensus defaults
    configValues["initial_difficulty"] = "4.0";
    configValues["difficulty_adjustment_interval"] = "2016";
    configValues["block_time"] = std::to_string(DEFAULT_BLOCK_TIME);
    configValues["max_block_size"] = "33554432"; // 32MB
    configValues["min_tx_fee"] = "0.00001";
    
    // Validator defaults
    configValues["validator_enabled"] = "0";
    configValues["min_stake"] = "100.0";
    configValues["validator_timeout"] = "30";
    
    // API defaults
    configValues["rpc_enabled"] = "1";
    configValues["rpc_address"] = "127.0.0.1";
    configValues["rpc_port"] = std::to_string(DEFAULT_RPC_PORT);
    configValues["rest_enabled"] = "1";
    configValues["rest_port"] = std::to_string(DEFAULT_REST_PORT);
    configValues["rpc_username"] = "";
    configValues["rpc_password"] = "";
    
    // Logging defaults
    configValues["log_level"] = "INFO";
    configValues["log_file"] = "gxc.log";
    configValues["log_max_size"] = "10485760"; // 10MB
    configValues["log_max_files"] = "10";
    
    // Security defaults
    configValues["ssl_enabled"] = "0";
    configValues["ssl_cert_file"] = "";
    configValues["ssl_key_file"] = "";
    configValues["max_login_attempts"] = "5";
    configValues["login_ban_time"] = "300"; // 5 minutes
    
    // Performance defaults
    configValues["worker_threads"] = "0"; // Auto-detect
    configValues["io_threads"] = "4";
    configValues["network_buffer_size"] = "65536"; // 64KB
    configValues["multi_threading"] = "1";
}

std::string Config::getString(const std::string& key, const std::string& defaultValue) const {
    auto it = configValues.find(key);
    return (it != configValues.end()) ? it->second : defaultValue;
}

int Config::getInt(const std::string& key, int defaultValue) const {
    auto it = configValues.find(key);
    if (it != configValues.end()) {
        try {
            return std::stoi(it->second);
        } catch (const std::exception&) {
            return defaultValue;
        }
    }
    return defaultValue;
}

double Config::getDouble(const std::string& key, double defaultValue) const {
    auto it = configValues.find(key);
    if (it != configValues.end()) {
        try {
            return std::stod(it->second);
        } catch (const std::exception&) {
            return defaultValue;
        }
    }
    return defaultValue;
}

bool Config::getBool(const std::string& key, bool defaultValue) const {
    auto it = configValues.find(key);
    if (it != configValues.end()) {
        const std::string& value = it->second;
        return (value == "1" || value == "true" || value == "TRUE" || value == "yes" || value == "YES");
    }
    return defaultValue;
}

std::vector<std::string> Config::getStringList(const std::string& key) const {
    std::vector<std::string> result;
    std::string value = getString(key);
    
    if (!value.empty()) {
        std::istringstream iss(value);
        std::string item;
        
        while (std::getline(iss, item, ',')) {
            result.push_back(trim(item));
        }
    }
    
    return result;
}

void Config::setString(const std::string& key, const std::string& value) {
    configValues[key] = value;
}

void Config::setInt(const std::string& key, int value) {
    configValues[key] = std::to_string(value);
}

void Config::setDouble(const std::string& key, double value) {
    configValues[key] = std::to_string(value);
}

void Config::setBool(const std::string& key, bool value) {
    configValues[key] = value ? "1" : "0";
}

void Config::setStringList(const std::string& key, const std::vector<std::string>& values) {
    std::string result;
    for (size_t i = 0; i < values.size(); ++i) {
        if (i > 0) result += ",";
        result += values[i];
    }
    configValues[key] = result;
}

bool Config::hasKey(const std::string& key) const {
    return configValues.find(key) != configValues.end();
}

bool Config::validateConfig() const {
    // Validate network settings
    if (getListenPort() == 0 || getListenPort() > 65535) {
        return false;
    }
    
    if (getMaxConnections() == 0 || getMaxConnections() > 1000) {
        return false;
    }
    
    // Validate mining settings
    if (isMiningEnabled() && getMinerAddress().empty()) {
        return false;
    }
    
    // Validate API settings
    if (isRpcEnabled() && (getRpcPort() == 0 || getRpcPort() > 65535)) {
        return false;
    }
    
    if (isRestEnabled() && (getRestPort() == 0 || getRestPort() > 65535)) {
        return false;
    }
    
    // Validate consensus settings
    if (getInitialDifficulty() <= 0.0 || getInitialDifficulty() > 1000.0) {
        return false;
    }
    
    if (getBlockTime() < 60 || getBlockTime() > 3600) { // 1 minute to 1 hour
        return false;
    }
    
    if (getMinTxFee() < 0.0) {
        return false;
    }
    
    return true;
}

std::vector<std::string> Config::getValidationErrors() const {
    std::vector<std::string> errors;
    
    if (getListenPort() == 0 || getListenPort() > 65535) {
        errors.push_back("Invalid listen port: " + std::to_string(getListenPort()));
    }
    
    if (getMaxConnections() == 0 || getMaxConnections() > 1000) {
        errors.push_back("Invalid max connections: " + std::to_string(getMaxConnections()));
    }
    
    if (isMiningEnabled() && getMinerAddress().empty()) {
        errors.push_back("Mining enabled but no miner address specified");
    }
    
    if (isRpcEnabled() && (getRpcPort() == 0 || getRpcPort() > 65535)) {
        errors.push_back("Invalid RPC port: " + std::to_string(getRpcPort()));
    }
    
    if (isRestEnabled() && (getRestPort() == 0 || getRestPort() > 65535)) {
        errors.push_back("Invalid REST port: " + std::to_string(getRestPort()));
    }
    
    return errors;
}

std::string Config::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

// Accessor implementations
std::string Config::getListenAddress() const { return getString("listen_address", "0.0.0.0"); }
uint16_t Config::getListenPort() const { return static_cast<uint16_t>(getInt("listen_port", DEFAULT_PORT)); }
uint32_t Config::getMaxConnections() const { return static_cast<uint32_t>(getInt("max_connections", DEFAULT_MAX_CONNECTIONS)); }
std::vector<std::string> Config::getBootstrapNodes() const { return getStringList("bootstrap_nodes"); }
bool Config::isTestNet() const { return getBool("testnet", false); }

bool Config::isMiningEnabled() const { return getBool("mining_enabled", false); }
std::string Config::getMinerAddress() const { return getString("miner_address"); }
uint32_t Config::getMiningThreads() const { return static_cast<uint32_t>(getInt("mining_threads", 0)); }
std::string Config::getMiningAlgorithm() const { return getString("mining_algorithm", "sha256"); }
std::string Config::getPoolAddress() const { return getString("pool_address"); }
bool Config::isGpuMiningEnabled() const { return getBool("gpu_mining_enabled", false); }

std::string Config::getWalletFile() const { return getString("wallet_file", "wallet.dat"); }
bool Config::isWalletEncrypted() const { return getBool("wallet_encrypted", false); }
uint32_t Config::getWalletBackupInterval() const { return static_cast<uint32_t>(getInt("wallet_backup_interval", 3600)); }

std::string Config::getDataDirectory() const { return getString("data_directory", DEFAULT_DATA_DIR); }
uint32_t Config::getDatabaseCacheSize() const { return static_cast<uint32_t>(getInt("database_cache_size", 100)); }
bool Config::isDatabaseCompressionEnabled() const { return getBool("database_compression", true); }

double Config::getInitialDifficulty() const { return getDouble("initial_difficulty", 4.0); }
uint32_t Config::getDifficultyAdjustmentInterval() const { return static_cast<uint32_t>(getInt("difficulty_adjustment_interval", 2016)); }
uint32_t Config::getBlockTime() const { return static_cast<uint32_t>(getInt("block_time", DEFAULT_BLOCK_TIME)); }
uint64_t Config::getMaxBlockSize() const { return static_cast<uint64_t>(getInt("max_block_size", 33554432)); }
double Config::getMinTxFee() const { return getDouble("min_tx_fee", DEFAULT_MIN_TX_FEE); }

bool Config::isValidatorEnabled() const { return getBool("validator_enabled", false); }
double Config::getMinStake() const { return getDouble("min_stake", 100.0); }
uint32_t Config::getValidatorTimeout() const { return static_cast<uint32_t>(getInt("validator_timeout", 30)); }

bool Config::isRpcEnabled() const { return getBool("rpc_enabled", true); }
std::string Config::getRpcAddress() const { return getString("rpc_address", "127.0.0.1"); }
uint16_t Config::getRpcPort() const { return static_cast<uint16_t>(getInt("rpc_port", DEFAULT_RPC_PORT)); }
bool Config::isRestEnabled() const { return getBool("rest_enabled", true); }
uint16_t Config::getRestPort() const { return static_cast<uint16_t>(getInt("rest_port", DEFAULT_REST_PORT)); }
std::string Config::getRpcUsername() const { return getString("rpc_username"); }
std::string Config::getRpcPassword() const { return getString("rpc_password"); }

std::string Config::getLogLevel() const { return getString("log_level", "INFO"); }
std::string Config::getLogFile() const { return getString("log_file", "gxc.log"); }
uint32_t Config::getLogMaxSize() const { return static_cast<uint32_t>(getInt("log_max_size", 10485760)); }
uint32_t Config::getLogMaxFiles() const { return static_cast<uint32_t>(getInt("log_max_files", 10)); }

bool Config::isSslEnabled() const { return getBool("ssl_enabled", false); }
std::string Config::getSslCertFile() const { return getString("ssl_cert_file"); }
std::string Config::getSslKeyFile() const { return getString("ssl_key_file"); }
uint32_t Config::getMaxLoginAttempts() const { return static_cast<uint32_t>(getInt("max_login_attempts", 5)); }
uint32_t Config::getLoginBanTime() const { return static_cast<uint32_t>(getInt("login_ban_time", 300)); }

uint32_t Config::getWorkerThreads() const { return static_cast<uint32_t>(getInt("worker_threads", 0)); }
uint32_t Config::getIoThreads() const { return static_cast<uint32_t>(getInt("io_threads", 4)); }
uint32_t Config::getNetworkBufferSize() const { return static_cast<uint32_t>(getInt("network_buffer_size", 65536)); }
bool Config::isMultiThreadingEnabled() const { return getBool("multi_threading", true); }