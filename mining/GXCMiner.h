#pragma once

#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <memory>
#include <chrono>
#include <functional>
#include <unordered_map>
#include <cstdint>

// Mining algorithm types
enum class MiningAlgorithm {
    SHA256,     // ASIC mining (Bitcoin-style)
    ETHASH,     // GPU mining (Ethereum-style)
    GXHASH      // CPU mining (GXC custom algorithm)
};

// Mining device types
enum class DeviceType {
    CPU,
    GPU_NVIDIA,
    GPU_AMD,
    ASIC
};

// Mining device information
struct MiningDevice {
    uint32_t deviceId;
    DeviceType type;
    std::string name;
    std::string driver;
    uint64_t memory;
    uint32_t computeUnits;
    uint32_t clockSpeed;
    double temperature;
    double powerUsage;
    bool isEnabled;
    bool isOverclocked;
    
    // Performance metrics
    double hashRate;
    uint64_t acceptedShares;
    uint64_t rejectedShares;
    uint64_t staleSahres;
    std::chrono::time_point<std::chrono::steady_clock> lastShare;
};

// Mining pool configuration
struct PoolConfig {
    std::string url;
    uint16_t port;
    std::string username;
    std::string password;
    std::string workerName;
    MiningAlgorithm algorithm;
    uint32_t difficulty;
    bool isSSL;
    uint32_t priority; // Pool priority (lower = higher priority)
    bool isBackup;
};

// Mining work package
struct MiningWork {
    std::string jobId;
    std::string blockHeader;
    std::string target;
    std::string extraNonce1;
    std::string extraNonce2;
    uint32_t difficulty;
    uint64_t height;
    std::chrono::time_point<std::chrono::steady_clock> timestamp;
    bool cleanJobs;
    
    // Algorithm-specific data
    std::string seedHash;    // For Ethash
    std::string dagHash;     // For Ethash
    uint64_t epochNumber;    // For Ethash
};

// Mining result/share
struct MiningShare {
    std::string jobId;
    std::string nonce;
    std::string extraNonce2;
    std::string mixHash;     // For Ethash
    std::string result;
    uint32_t deviceId;
    std::chrono::time_point<std::chrono::steady_clock> timestamp;
    double difficulty;
};

// Mining statistics
struct MiningStats {
    double totalHashRate;
    uint64_t totalAcceptedShares;
    uint64_t totalRejectedShares;
    uint64_t totalStaleShares;
    double efficiency; // accepted / (accepted + rejected + stale)
    std::chrono::time_point<std::chrono::steady_clock> startTime;
    std::chrono::duration<double> uptime;
    double powerConsumption;
    double profitability; // GXC per hour
    
    // Per-device stats
    std::vector<MiningStats> deviceStats;
};

// Base miner interface
class BaseMiner {
public:
    virtual ~BaseMiner() = default;
    
    virtual bool initialize() = 0;
    virtual bool start() = 0;
    virtual void stop() = 0;
    virtual bool isRunning() const = 0;
    
    virtual void setWork(const MiningWork& work) = 0;
    virtual MiningStats getStats() const = 0;
    virtual std::vector<MiningDevice> getDevices() const = 0;
    
    virtual void setHashRateCallback(std::function<void(uint32_t, double)> callback) = 0;
    virtual void setShareCallback(std::function<void(const MiningShare&)> callback) = 0;
    virtual void setErrorCallback(std::function<void(const std::string&)> callback) = 0;

protected:
    std::atomic<bool> running{false};
    std::vector<MiningDevice> devices;
    MiningWork currentWork;
    std::function<void(uint32_t, double)> hashRateCallback;
    std::function<void(const MiningShare&)> shareCallback;
    std::function<void(const std::string&)> errorCallback;
};

// SHA-256 ASIC Miner
class SHA256Miner : public BaseMiner {
public:
    SHA256Miner();
    ~SHA256Miner();
    
    bool initialize() override;
    bool start() override;
    void stop() override;
    bool isRunning() const override { return running; }
    
    void setWork(const MiningWork& work) override;
    MiningStats getStats() const override;
    std::vector<MiningDevice> getDevices() const override;
    
    void setHashRateCallback(std::function<void(uint32_t, double)> callback) override;
    void setShareCallback(std::function<void(const MiningShare&)> callback) override;
    void setErrorCallback(std::function<void(const std::string&)> callback) override;
    
    // ASIC-specific methods
    bool detectASICDevices();
    void setASICFrequency(uint32_t deviceId, uint32_t frequency);
    void setASICVoltage(uint32_t deviceId, double voltage);
    void resetASICDevice(uint32_t deviceId);

private:
    void minerThread(uint32_t deviceId);
    void hashWork(uint32_t deviceId, const MiningWork& work);
    std::string calculateSHA256d(const std::string& input);
    bool meetsTarget(const std::string& hash, const std::string& target);
    
    std::vector<std::thread> minerThreads;
    std::mutex workMutex;
    std::mutex statsMutex;
    MiningStats stats;
    
    // ASIC communication
    std::vector<int> asicFileDescriptors;
    bool initializeASICCommunication();
    void shutdownASICCommunication();
    bool sendASICCommand(uint32_t deviceId, const std::string& command);
    std::string receiveASICResponse(uint32_t deviceId);
};

// Ethash GPU Miner
class EthashMiner : public BaseMiner {
public:
    EthashMiner();
    ~EthashMiner();
    
    bool initialize() override;
    bool start() override;
    void stop() override;
    bool isRunning() const override { return running; }
    
    void setWork(const MiningWork& work) override;
    MiningStats getStats() const override;
    std::vector<MiningDevice> getDevices() const override;
    
    void setHashRateCallback(std::function<void(uint32_t, double)> callback) override;
    void setShareCallback(std::function<void(const MiningShare&)> callback) override;
    void setErrorCallback(std::function<void(const std::string&)> callback) override;
    
    // GPU-specific methods
    bool detectGPUDevices();
    void setGPUClockSpeed(uint32_t deviceId, uint32_t coreClock, uint32_t memoryClock);
    void setGPUFanSpeed(uint32_t deviceId, uint32_t fanSpeed);
    void setGPUPowerLimit(uint32_t deviceId, uint32_t powerLimit);

private:
    void minerThread(uint32_t deviceId);
    void hashWork(uint32_t deviceId, const MiningWork& work);
    std::string calculateEthash(const std::string& headerHash, uint64_t nonce, uint64_t epochNumber);
    bool generateDAG(uint64_t epochNumber);
    
    std::vector<std::thread> minerThreads;
    std::mutex workMutex;
    std::mutex statsMutex;
    std::mutex dagMutex;
    MiningStats stats;
    
    // Ethash-specific data
    std::unordered_map<uint64_t, std::vector<uint8_t>> dagCache;
    uint64_t currentEpoch = UINT64_MAX;
    
    // GPU context management
    struct GPUContext {
        void* context;
        void* device;
        void* queue;
        void* program;
        void* kernel;
        void* dagBuffer;
        void* headerBuffer;
        void* resultBuffer;
        size_t workGroupSize;
    };
    std::vector<GPUContext> gpuContexts;
    
    bool initializeGPUContext(uint32_t deviceId);
    void shutdownGPUContext(uint32_t deviceId);
    bool loadGPUKernel(uint32_t deviceId);
};

// GXHash CPU Miner
class GXHashMiner : public BaseMiner {
public:
    GXHashMiner();
    ~GXHashMiner();
    
    bool initialize() override;
    bool start() override;
    void stop() override;
    bool isRunning() const override { return running; }
    
    void setWork(const MiningWork& work) override;
    MiningStats getStats() const override;
    std::vector<MiningDevice> getDevices() const override;
    
    void setHashRateCallback(std::function<void(uint32_t, double)> callback) override;
    void setShareCallback(std::function<void(const MiningShare&)> callback) override;
    void setErrorCallback(std::function<void(const std::string&)> callback) override;
    
    // CPU-specific methods
    void setThreadCount(uint32_t threads);
    void setThreadAffinity(uint32_t threadId, uint32_t cpuCore);
    void enableHugePagesOptimization(bool enable);

private:
    void minerThread(uint32_t threadId);
    void hashWork(uint32_t threadId, const MiningWork& work);
    std::string calculateGXHash(const std::string& input, uint64_t nonce);
    
    std::vector<std::thread> minerThreads;
    std::mutex workMutex;
    std::mutex statsMutex;
    MiningStats stats;
    uint32_t threadCount;
    
    // GXHash algorithm implementation
    struct GXHashContext {
        uint8_t state[64];
        uint64_t counter[2];
        uint8_t buffer[128];
        size_t bufferSize;
    };
    
    void gxHashInit(GXHashContext* ctx);
    void gxHashUpdate(GXHashContext* ctx, const uint8_t* data, size_t length);
    void gxHashFinalize(GXHashContext* ctx, uint8_t* hash);
    void gxHashRound(uint8_t* state, const uint8_t* block);
};

// Pool connection manager
class PoolManager {
public:
    PoolManager();
    ~PoolManager();
    
    // Pool management
    void addPool(const PoolConfig& config);
    void removePool(const std::string& url);
    void setPoolPriority(const std::string& url, uint32_t priority);
    std::vector<PoolConfig> getPools() const;
    
    // Connection management
    bool connectToPool(const std::string& url = "");
    void disconnectFromPool();
    bool isConnected() const;
    std::string getCurrentPool() const;
    
    // Mining work
    void requestWork();
    void submitShare(const MiningShare& share);
    MiningWork getCurrentWork() const;
    
    // Statistics
    double getPoolHashRate() const;
    uint32_t getPoolDifficulty() const;
    uint64_t getPoolShares() const;
    std::chrono::time_point<std::chrono::steady_clock> getLastWorkTime() const;
    
    // Callbacks
    void setWorkCallback(std::function<void(const MiningWork&)> callback);
    void setDifficultyCallback(std::function<void(uint32_t)> callback);
    void setConnectionCallback(std::function<void(bool)> callback);

private:
    void connectionThread();
    void handleStratumMessage(const std::string& message);
    void sendStratumMessage(const std::string& message);
    bool parseStratumWork(const std::string& message, MiningWork& work);
    void processShareResponse(const std::string& message);
    
    std::vector<PoolConfig> pools;
    std::string currentPoolUrl;
    int socketFd = -1;
    std::atomic<bool> connected{false};
    std::atomic<bool> shouldStop{false};
    
    std::thread connectionThread_;
    std::mutex poolsMutex;
    std::mutex workMutex;
    std::mutex socketMutex;
    
    MiningWork currentWork;
    uint32_t extraNonce1;
    uint32_t extraNonce2Size;
    
    // Pool statistics
    double poolHashRate = 0.0;
    uint32_t poolDifficulty = 1;
    uint64_t poolShares = 0;
    std::chrono::time_point<std::chrono::steady_clock> lastWorkTime;
    
    // Callbacks
    std::function<void(const MiningWork&)> workCallback;
    std::function<void(uint32_t)> difficultyCallback;
    std::function<void(bool)> connectionCallback;
};

// Main miner application
class GXCMiner {
public:
    GXCMiner();
    ~GXCMiner();
    
    // Initialization
    bool initialize(MiningAlgorithm algorithm);
    void shutdown();
    
    // Mining control
    bool startMining();
    void stopMining();
    bool isMining() const;
    
    // Configuration
    void setAlgorithm(MiningAlgorithm algorithm);
    void setThreadCount(uint32_t threads); // For CPU mining
    void setDeviceList(const std::vector<uint32_t>& deviceIds);
    void enableDevice(uint32_t deviceId, bool enable);
    
    // Pool management
    void addPool(const PoolConfig& config);
    void removePool(const std::string& url);
    bool connectToPool(const std::string& url = "");
    void disconnectFromPool();
    
    // Statistics and monitoring
    MiningStats getOverallStats() const;
    MiningStats getDeviceStats(uint32_t deviceId) const;
    std::vector<MiningDevice> getDevices() const;
    PoolConfig getCurrentPool() const;
    
    // Events and callbacks
    void setHashRateCallback(std::function<void(double)> callback);
    void setShareCallback(std::function<void(bool, const std::string&)> callback);
    void setErrorCallback(std::function<void(const std::string&)> callback);
    void setStatusCallback(std::function<void(const std::string&)> callback);
    
    // Overclocking and device management
    bool setDeviceClockSpeed(uint32_t deviceId, uint32_t coreClock, uint32_t memoryClock = 0);
    bool setDevicePowerLimit(uint32_t deviceId, uint32_t powerLimit);
    bool setDeviceFanSpeed(uint32_t deviceId, uint32_t fanSpeed);
    void resetDevice(uint32_t deviceId);
    
    // Configuration save/load
    bool saveConfiguration(const std::string& filename) const;
    bool loadConfiguration(const std::string& filename);
    
    // Solo mining support
    void setSoloMining(bool solo, const std::string& nodeUrl = "127.0.0.1:8332");
    bool isSoloMining() const;

private:
    void onNewWork(const MiningWork& work);
    void onNewShare(const MiningShare& share);
    void onHashRateUpdate(uint32_t deviceId, double hashRate);
    void onMiningError(const std::string& error);
    
    MiningAlgorithm currentAlgorithm;
    std::unique_ptr<BaseMiner> miner;
    std::unique_ptr<PoolManager> poolManager;
    
    std::atomic<bool> initialized{false};
    std::atomic<bool> mining{false};
    std::atomic<bool> soloMining{false};
    
    // Configuration
    uint32_t threadCount = 0;
    std::vector<uint32_t> enabledDevices;
    std::string soloNodeUrl;
    
    // Statistics
    MiningStats overallStats;
    std::mutex statsMutex;
    
    // Callbacks
    std::function<void(double)> hashRateCallback;
    std::function<void(bool, const std::string&)> shareCallback;
    std::function<void(const std::string&)> errorCallback;
    std::function<void(const std::string&)> statusCallback;
    
    // Solo mining
    class SoloMiningManager* soloManager;
};

// Performance optimizer
class MiningOptimizer {
public:
    static void optimizeSystem();
    static void setCPUAffinity(const std::vector<uint32_t>& cores);
    static void setPriority(int priority);
    static void enableLargePagesSupport();
    static void disableWindowsDefender();
    static void configureGPUSettings();
    static void optimizeNetworkSettings();
    
    // Auto-tuning
    static std::vector<uint32_t> findOptimalClockSpeeds(uint32_t deviceId);
    static uint32_t findOptimalThreadCount();
    static uint32_t findOptimalIntensity(uint32_t deviceId);
};

// Hardware detection utilities
class HardwareDetector {
public:
    static std::vector<MiningDevice> detectCPUs();
    static std::vector<MiningDevice> detectNVIDIAGPUs();
    static std::vector<MiningDevice> detectAMDGPUs();
    static std::vector<MiningDevice> detectASICDevices();
    static std::vector<MiningDevice> detectAllDevices();
    
    static std::string getCPUInfo();
    static std::string getGPUInfo(uint32_t deviceId);
    static std::string getASICInfo(uint32_t deviceId);
    static bool isDeviceSupported(const MiningDevice& device, MiningAlgorithm algorithm);
};

// Configuration manager
class ConfigManager {
public:
    struct MinerConfig {
        MiningAlgorithm algorithm = MiningAlgorithm::GXHASH;
        uint32_t threadCount = 0;
        std::vector<uint32_t> enabledDevices;
        std::vector<PoolConfig> pools;
        bool soloMining = false;
        std::string soloNodeUrl = "127.0.0.1:8332";
        bool autoStart = false;
        bool minimizeToTray = false;
        std::string logLevel = "INFO";
        std::string logFile = "gxcminer.log";
    };
    
    static bool saveConfig(const MinerConfig& config, const std::string& filename = "config.json");
    static bool loadConfig(MinerConfig& config, const std::string& filename = "config.json");
    static MinerConfig getDefaultConfig();
    static std::string getConfigPath();
};

// Constants
constexpr uint32_t DEFAULT_STRATUM_PORT = 3333;
constexpr uint32_t DEFAULT_DIFFICULTY = 1;
constexpr uint32_t MAX_EXTRANONCE2_SIZE = 8;
constexpr uint32_t HASHRATE_UPDATE_INTERVAL = 5; // seconds
constexpr uint32_t STATS_UPDATE_INTERVAL = 30; // seconds
constexpr double MIN_SHARE_DIFFICULTY = 0.000001;