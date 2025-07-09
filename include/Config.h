#pragma once

#include <string>
#include <map>
#include <vector>

class Config {
private:
    std::map<std::string, std::string> configValues;
    std::string configFilePath;
    
    // Internal helpers
    void parseConfigFile();
    void writeConfigFile();
    std::string trim(const std::string& str);

public:
    // Constructor
    Config(const std::string& configPath = "gxc.conf");
    
    // Configuration management
    bool loadConfig();
    bool saveConfig();
    void setDefaults();
    
    // Value accessors
    std::string getString(const std::string& key, const std::string& defaultValue = "") const;
    int getInt(const std::string& key, int defaultValue = 0) const;
    double getDouble(const std::string& key, double defaultValue = 0.0) const;
    bool getBool(const std::string& key, bool defaultValue = false) const;
    std::vector<std::string> getStringList(const std::string& key) const;
    
    // Value setters
    void setString(const std::string& key, const std::string& value);
    void setInt(const std::string& key, int value);
    void setDouble(const std::string& key, double value);
    void setBool(const std::string& key, bool value);
    void setStringList(const std::string& key, const std::vector<std::string>& values);
    
    // Validation
    bool hasKey(const std::string& key) const;
    bool validateConfig() const;
    std::vector<std::string> getValidationErrors() const;
    
    // Network configuration
    std::string getListenAddress() const;
    uint16_t getListenPort() const;
    uint32_t getMaxConnections() const;
    std::vector<std::string> getBootstrapNodes() const;
    bool isTestNet() const;
    
    // Mining configuration
    bool isMiningEnabled() const;
    std::string getMinerAddress() const;
    uint32_t getMiningThreads() const;
    std::string getMiningAlgorithm() const;
    std::string getPoolAddress() const;
    bool isGpuMiningEnabled() const;
    
    // Wallet configuration
    std::string getWalletFile() const;
    bool isWalletEncrypted() const;
    uint32_t getWalletBackupInterval() const;
    
    // Database configuration
    std::string getDataDirectory() const;
    uint32_t getDatabaseCacheSize() const;
    bool isDatabaseCompressionEnabled() const;
    
    // Consensus configuration
    double getInitialDifficulty() const;
    uint32_t getDifficultyAdjustmentInterval() const;
    uint32_t getBlockTime() const;
    uint64_t getMaxBlockSize() const;
    double getMinTxFee() const;
    
    // Validator configuration
    bool isValidatorEnabled() const;
    double getMinStake() const;
    uint32_t getValidatorTimeout() const;
    
    // API configuration
    bool isRpcEnabled() const;
    std::string getRpcAddress() const;
    uint16_t getRpcPort() const;
    bool isRestEnabled() const;
    uint16_t getRestPort() const;
    std::string getRpcUsername() const;
    std::string getRpcPassword() const;
    
    // Logging configuration
    std::string getLogLevel() const;
    std::string getLogFile() const;
    uint32_t getLogMaxSize() const;
    uint32_t getLogMaxFiles() const;
    
    // Security configuration
    bool isSslEnabled() const;
    std::string getSslCertFile() const;
    std::string getSslKeyFile() const;
    uint32_t getMaxLoginAttempts() const;
    uint32_t getLoginBanTime() const;
    
    // Performance configuration
    uint32_t getWorkerThreads() const;
    uint32_t getIoThreads() const;
    uint32_t getNetworkBufferSize() const;
    bool isMultiThreadingEnabled() const;
    
    // Configuration constants
    static const std::string DEFAULT_CONFIG_FILE;
    static const std::string DEFAULT_DATA_DIR;
    static const uint16_t DEFAULT_PORT;
    static const uint16_t DEFAULT_RPC_PORT;
    static const uint16_t DEFAULT_REST_PORT;
    static const uint32_t DEFAULT_MAX_CONNECTIONS;
    static const double DEFAULT_MIN_TX_FEE;
    static const uint32_t DEFAULT_BLOCK_TIME;
};