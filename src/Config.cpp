#include "../include/Config.h"
#include "../include/Logger.h"
#include "../include/Utils.h"
#include <fstream>
#include <sstream>
#include <algorithm>

// Static members
std::map<std::string, std::string> Config::configMap;
std::mutex Config::configMutex;
bool Config::initialized = false;

Config::Config() {
    // Private constructor
}

bool Config::initialize() {
    std::lock_guard<std::mutex> lock(configMutex);
    
    if (initialized) {
        return true;
    }
    
    // Set default values
    setDefaults();
    
    initialized = true;
    LOG_CONFIG(LogLevel::INFO, "Configuration system initialized");
    
    return true;
}

void Config::shutdown() {
    std::lock_guard<std::mutex> lock(configMutex);
    
    configMap.clear();
    initialized = false;
    
    LOG_CONFIG(LogLevel::INFO, "Configuration system shutdown");
}

void Config::setDefaults() {
    // Network settings
    configMap["network_port"] = "9333";
    configMap["rpc_port"] = "8332";
    configMap["rest_port"] = "8080";
    configMap["max_peers"] = "20";
    configMap["min_peers"] = "8";
    configMap["connection_timeout"] = "30";
    
    // Blockchain settings
    configMap["data_dir"] = "./gxc_data";
    configMap["testnet"] = "false";
    configMap["difficulty_adjustment_interval"] = "10";
    configMap["target_block_time"] = "600"; // 10 minutes
    configMap["max_block_size"] = "1048576"; // 1MB
    configMap["block_reward"] = "50.0";
    
    // Mining settings
    configMap["enable_mining"] = "false";
    configMap["mining_threads"] = "1";
    configMap["mining_algorithm"] = "SHA256";
    configMap["pool_address"] = "";
    configMap["miner_address"] = "";
    
    // Wallet settings
    configMap["wallet_file"] = "wallet.dat";
    configMap["wallet_backup_interval"] = "3600"; // 1 hour
    configMap["transaction_fee"] = "0.001";
    configMap["min_confirmations"] = "6";
    
    // Dynamic fee settings
    configMap["enable_dynamic_fees"] = "true";
    configMap["base_transaction_fee"] = "0.001";
    configMap["min_tx_fee"] = "0.001";  // Minimum fee (same as base)
    configMap["max_transaction_fee"] = "0.01";
    configMap["fee_low_threshold"] = "10";
    configMap["fee_medium_threshold"] = "50";
    configMap["fee_high_threshold"] = "100";
    configMap["fee_low_multiplier"] = "1.0";
    configMap["fee_medium_multiplier"] = "1.5";
    configMap["fee_high_multiplier"] = "2.0";
    configMap["fee_very_high_multiplier"] = "3.0";
    
    // Logging settings
    configMap["log_level"] = "INFO";
    configMap["log_file"] = "gxc.log";
    configMap["log_max_size"] = "10485760"; // 10MB
    configMap["log_max_files"] = "5";
    configMap["log_to_console"] = "true";
    
    // Security settings
    configMap["enable_ssl"] = "false";
    configMap["ssl_cert_file"] = "";
    configMap["ssl_key_file"] = "";
    configMap["max_connections"] = "100";
    configMap["ban_threshold"] = "10";
    configMap["ban_duration"] = "86400"; // 24 hours
    
    // Performance settings
    configMap["cache_size"] = "268435456"; // 256MB
    configMap["mempool_size"] = "50000000"; // 50MB
    configMap["checkpoint_interval"] = "1000";
    configMap["vacuum_interval"] = "604800"; // 1 week
    
    // Traceability settings
    configMap["enable_traceability"] = "true";
    configMap["traceability_validation"] = "strict";
    configMap["trace_depth_limit"] = "10000";
    
    // API settings
    configMap["enable_rest_api"] = "true";
    configMap["enable_rpc_api"] = "true";
    configMap["api_rate_limit"] = "100";
    configMap["api_key"] = "";
    
    // Governance settings
    configMap["governance_enabled"] = "true";
    configMap["proposal_deposit"] = "100.0";
    configMap["voting_period"] = "604800"; // 1 week
    configMap["quorum_threshold"] = "0.33";
    
    // Cross-chain settings
    configMap["bridge_enabled"] = "false";
    configMap["bridge_validators"] = "3";
    configMap["bridge_threshold"] = "2";
    
    // Development settings
    configMap["debug_mode"] = "false";
    configMap["enable_profiling"] = "false";
    configMap["stats_interval"] = "300"; // 5 minutes
}

bool Config::loadFromFile(const std::string& filename) {
    std::lock_guard<std::mutex> lock(configMutex);
    
    LOG_CONFIG(LogLevel::INFO, "Loading configuration from: " + filename);
    
    std::ifstream file(filename);
    if (!file.is_open()) {
        LOG_CONFIG(LogLevel::ERROR, "Failed to open config file: " + filename);
        return false;
    }
    
    std::string line;
    int lineNumber = 0;
    
    while (std::getline(file, line)) {
        lineNumber++;
        
        // Remove leading/trailing whitespace
        line = Utils::trim(line);
        
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }
        
        // Parse key=value pairs
        size_t equalPos = line.find('=');
        if (equalPos == std::string::npos) {
            LOG_CONFIG(LogLevel::WARNING, "Invalid config line " + std::to_string(lineNumber) + ": " + line);
            continue;
        }
        
        std::string key = Utils::trim(line.substr(0, equalPos));
        std::string value = Utils::trim(line.substr(equalPos + 1));
        
        // Remove quotes if present
        if (value.length() >= 2 && 
            ((value.front() == '"' && value.back() == '"') ||
             (value.front() == '\'' && value.back() == '\''))) {
            value = value.substr(1, value.length() - 2);
        }
        
        configMap[key] = value;
        LOG_CONFIG(LogLevel::DEBUG, "Loaded config: " + key + " = " + value);
    }
    
    file.close();
    
    LOG_CONFIG(LogLevel::INFO, "Configuration loaded successfully from " + filename);
    return true;
}

bool Config::saveToFile(const std::string& filename) {
    std::lock_guard<std::mutex> lock(configMutex);
    
    LOG_CONFIG(LogLevel::INFO, "Saving configuration to: " + filename);
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        LOG_CONFIG(LogLevel::ERROR, "Failed to create config file: " + filename);
        return false;
    }
    
    // Write header
    file << "# GXC Blockchain Configuration File\n";
    file << "# Generated on " << Utils::formatTimestamp(Utils::getCurrentTimestamp()) << "\n";
    file << "\n";
    
    // Group settings by category
    std::vector<std::string> categories = {
        "network", "blockchain", "mining", "wallet", "logging", 
        "security", "performance", "traceability", "api", 
        "governance", "bridge", "debug"
    };
    
    for (const auto& category : categories) {
        // Capitalize first letter
        std::string capCategory = category;
        if (!capCategory.empty()) {
            capCategory[0] = std::toupper(capCategory[0]);
        }
        file << "# " << capCategory << " Settings\n";
        
        for (const auto& [key, value] : configMap) {
            if (key.find(category) == 0 || 
                (category == "blockchain" && (key == "data_dir" || key == "testnet")) ||
                (category == "debug" && key == "debug_mode")) {
                file << key << " = " << value << "\n";
            }
        }
        
        file << "\n";
    }
    
    file.close();
    
    LOG_CONFIG(LogLevel::INFO, "Configuration saved successfully to " + filename);
    return true;
}

std::string Config::get(const std::string& key, const std::string& defaultValue) {
    std::lock_guard<std::mutex> lock(configMutex);
    
    auto it = configMap.find(key);
    if (it != configMap.end()) {
        return it->second;
    }
    
    LOG_CONFIG(LogLevel::DEBUG, "Config key not found, using default: " + key + " = " + defaultValue);
    return defaultValue;
}

int Config::getInt(const std::string& key, int defaultValue) {
    std::string value = get(key, std::to_string(defaultValue));
    
    try {
        return std::stoi(value);
    } catch (const std::exception& e) {
        LOG_CONFIG(LogLevel::WARNING, "Invalid integer value for " + key + ": " + value);
        return defaultValue;
    }
}

double Config::getDouble(const std::string& key, double defaultValue) {
    std::string value = get(key, std::to_string(defaultValue));
    
    try {
        return std::stod(value);
    } catch (const std::exception& e) {
        LOG_CONFIG(LogLevel::WARNING, "Invalid double value for " + key + ": " + value);
        return defaultValue;
    }
}

bool Config::getBool(const std::string& key, bool defaultValue) {
    std::string value = get(key, defaultValue ? "true" : "false");
    
    // Convert to lowercase for comparison
    std::transform(value.begin(), value.end(), value.begin(), ::tolower);
    
    if (value == "true" || value == "1" || value == "yes" || value == "on") {
        return true;
    } else if (value == "false" || value == "0" || value == "no" || value == "off") {
        return false;
    } else {
        LOG_CONFIG(LogLevel::WARNING, "Invalid boolean value for " + key + ": " + value);
        return defaultValue;
    }
}

void Config::set(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(configMutex);
    
    configMap[key] = value;
    LOG_CONFIG(LogLevel::DEBUG, "Config updated: " + key + " = " + value);
}

void Config::setInt(const std::string& key, int value) {
    set(key, std::to_string(value));
}

void Config::setDouble(const std::string& key, double value) {
    set(key, std::to_string(value));
}

void Config::setBool(const std::string& key, bool value) {
    set(key, value ? "true" : "false");
}

bool Config::has(const std::string& key) {
    std::lock_guard<std::mutex> lock(configMutex);
    
    return configMap.find(key) != configMap.end();
}

void Config::remove(const std::string& key) {
    std::lock_guard<std::mutex> lock(configMutex);
    
    auto it = configMap.find(key);
    if (it != configMap.end()) {
        configMap.erase(it);
        LOG_CONFIG(LogLevel::DEBUG, "Config removed: " + key);
    }
}

std::vector<std::string> Config::getKeys() {
    std::lock_guard<std::mutex> lock(configMutex);
    
    std::vector<std::string> keys;
    keys.reserve(configMap.size());
    
    for (const auto& [key, value] : configMap) {
        keys.push_back(key);
    }
    
    return keys;
}

std::map<std::string, std::string> Config::getAll() {
    std::lock_guard<std::mutex> lock(configMutex);
    
    return configMap;
}

void Config::clear() {
    std::lock_guard<std::mutex> lock(configMutex);
    
    configMap.clear();
    LOG_CONFIG(LogLevel::INFO, "Configuration cleared");
}

bool Config::validate() {
    std::lock_guard<std::mutex> lock(configMutex);
    
    LOG_CONFIG(LogLevel::INFO, "Validating configuration");
    
    bool isValid = true;
    
    // Validate network ports
    int networkPort = getInt("network_port", 9333);
    int rpcPort = getInt("rpc_port", 8332);
    int restPort = getInt("rest_port", 8080);
    
    if (networkPort < 1024 || networkPort > 65535) {
        LOG_CONFIG(LogLevel::ERROR, "Invalid network port: " + std::to_string(networkPort));
        isValid = false;
    }
    
    if (rpcPort < 1024 || rpcPort > 65535) {
        LOG_CONFIG(LogLevel::ERROR, "Invalid RPC port: " + std::to_string(rpcPort));
        isValid = false;
    }
    
    if (restPort < 1024 || restPort > 65535) {
        LOG_CONFIG(LogLevel::ERROR, "Invalid REST port: " + std::to_string(restPort));
        isValid = false;
    }
    
    // Check for port conflicts
    if (networkPort == rpcPort || networkPort == restPort || rpcPort == restPort) {
        LOG_CONFIG(LogLevel::ERROR, "Port conflict detected");
        isValid = false;
    }
    
    // Validate data directory
    std::string dataDir = get("data_dir", "./gxc_data");
    if (dataDir.empty()) {
        LOG_CONFIG(LogLevel::ERROR, "Data directory cannot be empty");
        isValid = false;
    }
    
    // Validate block reward
    double blockReward = getDouble("block_reward", 50.0);
    if (blockReward < 0) {
        LOG_CONFIG(LogLevel::ERROR, "Block reward cannot be negative");
        isValid = false;
    }
    
    // Validate transaction fee
    double txFee = getDouble("transaction_fee", 0.001);
    if (txFee < 0) {
        LOG_CONFIG(LogLevel::ERROR, "Transaction fee cannot be negative");
        isValid = false;
    }
    
    // Validate mining threads
    int miningThreads = getInt("mining_threads", 1);
    if (miningThreads < 1 || miningThreads > 64) {
        LOG_CONFIG(LogLevel::ERROR, "Invalid mining threads count: " + std::to_string(miningThreads));
        isValid = false;
    }
    
    // Validate log level
    std::string logLevel = get("log_level", "INFO");
    if (logLevel != "DEBUG" && logLevel != "INFO" && logLevel != "WARNING" && logLevel != "ERROR") {
        LOG_CONFIG(LogLevel::ERROR, "Invalid log level: " + logLevel);
        isValid = false;
    }
    
    if (isValid) {
        LOG_CONFIG(LogLevel::INFO, "Configuration validation passed");
    } else {
        LOG_CONFIG(LogLevel::ERROR, "Configuration validation failed");
    }
    
    return isValid;
}

void Config::printConfig() {
    std::lock_guard<std::mutex> lock(configMutex);
    
    LOG_CONFIG(LogLevel::INFO, "Current configuration:");
    
    for (const auto& [key, value] : configMap) {
        // Hide sensitive values
        std::string displayValue = value;
        if (key.find("password") != std::string::npos || 
            key.find("private") != std::string::npos ||
            key.find("secret") != std::string::npos ||
            key.find("key") != std::string::npos) {
            displayValue = "***";
        }
        
        LOG_CONFIG(LogLevel::INFO, "  " + key + " = " + displayValue);
    }
}

std::string Config::getConfigSummary() {
    std::lock_guard<std::mutex> lock(configMutex);
    
    std::ostringstream oss;
    oss << "GXC Configuration Summary:\n";
    oss << "  Network Port: " << get("network_port") << "\n";
    oss << "  RPC Port: " << get("rpc_port") << "\n";
    oss << "  REST Port: " << get("rest_port") << "\n";
    oss << "  Data Directory: " << get("data_dir") << "\n";
    oss << "  Testnet: " << (getBool("testnet") ? "YES" : "NO") << "\n";
    oss << "  Mining Enabled: " << (getBool("enable_mining") ? "YES" : "NO") << "\n";
    oss << "  Traceability: " << (getBool("enable_traceability") ? "ENABLED" : "DISABLED") << "\n";
    oss << "  Log Level: " << get("log_level") << "\n";
    
    return oss.str();
}

bool Config::isTestnet() {
    return getBool("testnet", false);
}

bool Config::isMiningEnabled() {
    return getBool("enable_mining", false);
}

bool Config::isTraceabilityEnabled() {
    return getBool("enable_traceability", true);
}

std::string Config::getDataDirectory() {
    return get("data_dir", "./gxc_data");
}

int Config::getNetworkPort() {
    return getInt("network_port", 9333);
}

int Config::getRPCPort() {
    return getInt("rpc_port", 8332);
}

int Config::getRESTPort() {
    return getInt("rest_port", 8080);
}

void Config::setNetworkMode(bool testnet) {
    setBool("testnet", testnet);
    
    if (testnet) {
        // Adjust default ports for testnet
        if (getInt("network_port") == 9333) {
            setInt("network_port", 19333);
        }
        if (getInt("rpc_port") == 8332) {
            setInt("rpc_port", 18332);
        }
        if (getInt("rest_port") == 8080) {
            setInt("rest_port", 18080);
        }
    }
}

bool Config::isInitialized() {
    std::lock_guard<std::mutex> lock(configMutex);
    return initialized;
}