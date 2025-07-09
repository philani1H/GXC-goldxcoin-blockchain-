#include "Config.h"
#include "Logger.h"
#include <fstream>
#include <sstream>
#include <algorithm>

Config::Config(const std::string& configPath) : configPath_(configPath) {
    loadDefaults();
    loadFromFile();
}

void Config::loadDefaults() {
    // Network settings
    settings_["network.port"] = "8333";
    settings_["network.max_peers"] = "50";
    settings_["network.listen"] = "true";
    settings_["network.bind_address"] = "0.0.0.0";
    
    // API settings
    settings_["api.rpc_port"] = "8332";
    settings_["api.rest_port"] = "8080";
    settings_["api.websocket_port"] = "8334";
    settings_["api.enable_cors"] = "true";
    
    // Database settings
    settings_["database.path"] = "data/gxc.db";
    settings_["database.wal_mode"] = "true";
    settings_["database.cache_size"] = "10000";
    
    // Mining settings
    settings_["mining.enabled"] = "false";
    settings_["mining.threads"] = "0"; // Auto-detect
    settings_["mining.algorithm"] = "GXHash";
    settings_["mining.address"] = "";
    
    // Logging settings
    settings_["logging.level"] = "info";
    settings_["logging.file"] = "logs/gxc.log";
    settings_["logging.max_size"] = "100MB";
    settings_["logging.rotate_count"] = "5";
    
    // Performance settings
    settings_["performance.metrics_retention_days"] = "30";
    settings_["performance.block_cache_size"] = "1000";
    settings_["performance.tx_cache_size"] = "10000";
    
    // Security settings
    settings_["security.ban_duration_hours"] = "24";
    settings_["security.max_connection_attempts"] = "5";
    settings_["security.enable_whitelist"] = "false";
    
    Logger::debug("Default configuration loaded");
}

void Config::loadFromFile() {
    std::ifstream file(configPath_);
    if (!file.is_open()) {
        Logger::warning("Configuration file not found: " + configPath_ + ", using defaults");
        return;
    }
    
    std::string line;
    int lineNumber = 0;
    
    while (std::getline(file, line)) {
        lineNumber++;
        
        // Skip empty lines and comments
        line = trim(line);
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        // Parse key=value pairs
        size_t equalPos = line.find('=');
        if (equalPos == std::string::npos) {
            Logger::warning("Invalid configuration line " + std::to_string(lineNumber) + ": " + line);
            continue;
        }
        
        std::string key = trim(line.substr(0, equalPos));
        std::string value = trim(line.substr(equalPos + 1));
        
        // Remove quotes from value if present
        if (value.size() >= 2 && value[0] == '"' && value.back() == '"') {
            value = value.substr(1, value.size() - 2);
        }
        
        settings_[key] = value;
    }
    
    file.close();
    Logger::info("Configuration loaded from: " + configPath_);
}

void Config::saveToFile() const {
    std::ofstream file(configPath_);
    if (!file.is_open()) {
        Logger::error("Failed to open configuration file for writing: " + configPath_);
        return;
    }
    
    file << "# GXC Blockchain Configuration File\n";
    file << "# Generated automatically - edit with care\n\n";
    
    // Group settings by category
    std::map<std::string, std::map<std::string, std::string>> groupedSettings;
    
    for (const auto& [key, value] : settings_) {
        size_t dotPos = key.find('.');
        if (dotPos != std::string::npos) {
            std::string category = key.substr(0, dotPos);
            std::string setting = key.substr(dotPos + 1);
            groupedSettings[category][setting] = value;
        } else {
            groupedSettings["general"][key] = value;
        }
    }
    
    for (const auto& [category, categorySettings] : groupedSettings) {
        file << "# " << category << " settings\n";
        for (const auto& [setting, value] : categorySettings) {
            file << category << "." << setting << "=" << value << "\n";
        }
        file << "\n";
    }
    
    file.close();
    Logger::info("Configuration saved to: " + configPath_);
}

std::string Config::getString(const std::string& key, const std::string& defaultValue) const {
    auto it = settings_.find(key);
    return (it != settings_.end()) ? it->second : defaultValue;
}

int Config::getInt(const std::string& key, int defaultValue) const {
    auto it = settings_.find(key);
    if (it != settings_.end()) {
        try {
            return std::stoi(it->second);
        } catch (const std::exception& e) {
            Logger::warning("Invalid integer value for " + key + ": " + it->second);
        }
    }
    return defaultValue;
}

bool Config::getBool(const std::string& key, bool defaultValue) const {
    auto it = settings_.find(key);
    if (it != settings_.end()) {
        std::string value = it->second;
        std::transform(value.begin(), value.end(), value.begin(), ::tolower);
        return (value == "true" || value == "1" || value == "yes" || value == "on");
    }
    return defaultValue;
}

double Config::getDouble(const std::string& key, double defaultValue) const {
    auto it = settings_.find(key);
    if (it != settings_.end()) {
        try {
            return std::stod(it->second);
        } catch (const std::exception& e) {
            Logger::warning("Invalid double value for " + key + ": " + it->second);
        }
    }
    return defaultValue;
}

void Config::set(const std::string& key, const std::string& value) {
    settings_[key] = value;
}

void Config::set(const std::string& key, int value) {
    settings_[key] = std::to_string(value);
}

void Config::set(const std::string& key, bool value) {
    settings_[key] = value ? "true" : "false";
}

void Config::set(const std::string& key, double value) {
    settings_[key] = std::to_string(value);
}

bool Config::has(const std::string& key) const {
    return settings_.find(key) != settings_.end();
}

std::vector<std::string> Config::getKeys() const {
    std::vector<std::string> keys;
    for (const auto& [key, _] : settings_) {
        keys.push_back(key);
    }
    return keys;
}

// Network configuration getters
int Config::getNetworkPort() const {
    return getInt("network.port", 8333);
}

int Config::getMaxPeers() const {
    return getInt("network.max_peers", 50);
}

bool Config::getNetworkListen() const {
    return getBool("network.listen", true);
}

std::string Config::getBindAddress() const {
    return getString("network.bind_address", "0.0.0.0");
}

// API configuration getters
int Config::getRPCPort() const {
    return getInt("api.rpc_port", 8332);
}

int Config::getRESTPort() const {
    return getInt("api.rest_port", 8080);
}

int Config::getWebSocketPort() const {
    return getInt("api.websocket_port", 8334);
}

bool Config::getCORSEnabled() const {
    return getBool("api.enable_cors", true);
}

// Database configuration getters
std::string Config::getDatabasePath() const {
    return getString("database.path", "data/gxc.db");
}

bool Config::getDatabaseWALMode() const {
    return getBool("database.wal_mode", true);
}

int Config::getDatabaseCacheSize() const {
    return getInt("database.cache_size", 10000);
}

// Mining configuration getters
bool Config::getMiningEnabled() const {
    return getBool("mining.enabled", false);
}

int Config::getMiningThreads() const {
    return getInt("mining.threads", 0);
}

std::string Config::getMiningAlgorithm() const {
    return getString("mining.algorithm", "GXHash");
}

std::string Config::getMiningAddress() const {
    return getString("mining.address", "");
}

// Logging configuration getters
std::string Config::getLogLevel() const {
    return getString("logging.level", "info");
}

std::string Config::getLogFile() const {
    return getString("logging.file", "logs/gxc.log");
}

std::string Config::getLogMaxSize() const {
    return getString("logging.max_size", "100MB");
}

int Config::getLogRotateCount() const {
    return getInt("logging.rotate_count", 5);
}

// Performance configuration getters
int Config::getMetricsRetentionDays() const {
    return getInt("performance.metrics_retention_days", 30);
}

int Config::getBlockCacheSize() const {
    return getInt("performance.block_cache_size", 1000);
}

int Config::getTransactionCacheSize() const {
    return getInt("performance.tx_cache_size", 10000);
}

// Security configuration getters
int Config::getBanDurationHours() const {
    return getInt("security.ban_duration_hours", 24);
}

int Config::getMaxConnectionAttempts() const {
    return getInt("security.max_connection_attempts", 5);
}

bool Config::getWhitelistEnabled() const {
    return getBool("security.enable_whitelist", false);
}

std::string Config::trim(const std::string& str) const {
    const std::string whitespace = " \t\r\n";
    size_t start = str.find_first_not_of(whitespace);
    if (start == std::string::npos) {
        return "";
    }
    size_t end = str.find_last_not_of(whitespace);
    return str.substr(start, end - start + 1);
}

void Config::printConfiguration() const {
    Logger::info("=== Configuration Settings ===");
    
    // Print network settings
    Logger::info("Network:");
    Logger::info("  Port: " + std::to_string(getNetworkPort()));
    Logger::info("  Max Peers: " + std::to_string(getMaxPeers()));
    Logger::info("  Listen: " + std::string(getNetworkListen() ? "true" : "false"));
    Logger::info("  Bind Address: " + getBindAddress());
    
    // Print API settings
    Logger::info("API:");
    Logger::info("  RPC Port: " + std::to_string(getRPCPort()));
    Logger::info("  REST Port: " + std::to_string(getRESTPort()));
    Logger::info("  WebSocket Port: " + std::to_string(getWebSocketPort()));
    Logger::info("  CORS Enabled: " + std::string(getCORSEnabled() ? "true" : "false"));
    
    // Print database settings
    Logger::info("Database:");
    Logger::info("  Path: " + getDatabasePath());
    Logger::info("  WAL Mode: " + std::string(getDatabaseWALMode() ? "true" : "false"));
    Logger::info("  Cache Size: " + std::to_string(getDatabaseCacheSize()));
    
    // Print mining settings
    Logger::info("Mining:");
    Logger::info("  Enabled: " + std::string(getMiningEnabled() ? "true" : "false"));
    Logger::info("  Threads: " + std::to_string(getMiningThreads()));
    Logger::info("  Algorithm: " + getMiningAlgorithm());
    Logger::info("  Address: " + getMiningAddress());
    
    // Print logging settings
    Logger::info("Logging:");
    Logger::info("  Level: " + getLogLevel());
    Logger::info("  File: " + getLogFile());
    Logger::info("  Max Size: " + getLogMaxSize());
    Logger::info("  Rotate Count: " + std::to_string(getLogRotateCount()));
    
    Logger::info("==============================");
}