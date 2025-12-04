#pragma once

#include <string>
#include <map>
#include <vector>
#include <mutex>

class Config {
private:
    // Static members for singleton pattern
    static std::map<std::string, std::string> configMap;
    static std::mutex configMutex;
    static bool initialized;

    // Private constructor for singleton
    Config();

public:
    // Static singleton methods
    static bool initialize();
    static void shutdown();
    static void setDefaults();

    // Configuration file operations
    static bool loadFromFile(const std::string& filename);
    static bool saveToFile(const std::string& filename);

    // Value accessors (static)
    static std::string get(const std::string& key, const std::string& defaultValue = "");
    static int getInt(const std::string& key, int defaultValue = 0);
    static double getDouble(const std::string& key, double defaultValue = 0.0);
    static bool getBool(const std::string& key, bool defaultValue = false);
    static std::vector<std::string> getStringList(const std::string& key);

    // Value setters (static)
    static void set(const std::string& key, const std::string& value);
    static void setInt(const std::string& key, int value);
    static void setDouble(const std::string& key, double value);
    static void setBool(const std::string& key, bool value);
    static void setStringList(const std::string& key, const std::vector<std::string>& values);

    // Configuration management
    static bool has(const std::string& key);
    static void remove(const std::string& key);
    static std::vector<std::string> getKeys();
    static std::map<std::string, std::string> getAll();
    static void clear();
    static bool validate();

    // Utility methods
    static void printConfig();
    static std::string getConfigSummary();
    static void setNetworkMode(bool testnet);

    // Network configuration
    static std::string getListenAddress();
    static uint16_t getListenPort();
    static uint32_t getMaxConnections();
    static std::vector<std::string> getBootstrapNodes();
    static bool isTestNet();
    static bool isTestnet();
    static int getNetworkPort();
    static int getRPCPort();
    static int getRESTPort();
    static bool isInitialized();
    static bool isTraceabilityEnabled();

    // Mining configuration
    static bool isMiningEnabled();
    static std::string getMinerAddress();
    static uint32_t getMiningThreads();
    static std::string getMiningAlgorithm();
    static std::string getPoolAddress();
    static bool isGpuMiningEnabled();

    // Wallet configuration
    static std::string getWalletFile();
    static bool isWalletEncrypted();
    static uint32_t getWalletBackupInterval();

    // Database configuration
    static std::string getDataDirectory();
    static uint32_t getDatabaseCacheSize();
    static bool isDatabaseCompressionEnabled();

    // Consensus configuration
    static double getInitialDifficulty();
    static uint32_t getDifficultyAdjustmentInterval();
    static uint32_t getBlockTime();
    static uint64_t getMaxBlockSize();
    static double getMinTxFee();

    // Validator configuration
    static bool isValidatorEnabled();
    static double getMinStake();
    static uint32_t getValidatorTimeout();

    // API configuration
    static bool isRpcEnabled();
    static std::string getRpcAddress();
    static uint16_t getRpcPort();
    static bool isRestEnabled();
    static uint16_t getRestPort();
    static std::string getRpcUsername();
    static std::string getRpcPassword();

    // Logging configuration
    static std::string getLogLevel();
    static std::string getLogFile();
    static uint32_t getLogMaxSize();
    static uint32_t getLogMaxFiles();

    // Security configuration
    static bool isSslEnabled();
    static std::string getSslCertFile();
    static std::string getSslKeyFile();
    static uint32_t getMaxLoginAttempts();
    static uint32_t getLoginBanTime();

    // Performance configuration
    static uint32_t getWorkerThreads();
    static uint32_t getIoThreads();
    static uint32_t getNetworkBufferSize();
    static bool isMultiThreadingEnabled();

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