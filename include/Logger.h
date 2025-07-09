#pragma once

#include <string>
#include <fstream>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <atomic>

enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3,
    CRITICAL = 4
};

class Logger {
private:
    static std::unique_ptr<Logger> instance;
    static std::mutex instanceMutex;
    
    std::ofstream logFile;
    std::string logFilePath;
    LogLevel minLogLevel;
    uint32_t maxFileSize;
    uint32_t maxFiles;
    bool consoleOutput;
    bool asyncLogging;
    
    // Async logging
    std::queue<std::string> logQueue;
    std::mutex queueMutex;
    std::condition_variable queueCondition;
    std::thread logThread;
    std::atomic<bool> shutdownFlag;
    
    // File rotation
    void rotateLogFile();
    void deleteOldLogFiles();
    std::string getRotatedFileName(uint32_t index) const;
    
    // Internal logging
    void writeLog(LogLevel level, const std::string& message, const std::string& category = "");
    void processLogQueue();
    std::string formatLogMessage(LogLevel level, const std::string& message, const std::string& category) const;
    std::string getCurrentTimestamp() const;
    std::string logLevelToString(LogLevel level) const;
    
    // Constructor/Destructor (private for singleton)
    Logger();
    ~Logger();

public:
    // Singleton access
    static Logger& getInstance();
    static void cleanup();
    
    // Configuration
    bool initialize(const std::string& logPath, LogLevel minLevel = LogLevel::INFO);
    void setLogLevel(LogLevel level);
    void setConsoleOutput(bool enabled);
    void setAsyncLogging(bool enabled);
    void setFileRotation(uint32_t maxSize, uint32_t maxFiles);
    
    // Logging methods
    void debug(const std::string& message, const std::string& category = "");
    void info(const std::string& message, const std::string& category = "");
    void warning(const std::string& message, const std::string& category = "");
    void error(const std::string& message, const std::string& category = "");
    void critical(const std::string& message, const std::string& category = "");
    
    // Formatted logging
    template<typename... Args>
    void debugf(const std::string& format, Args... args);
    
    template<typename... Args>
    void infof(const std::string& format, Args... args);
    
    template<typename... Args>
    void warningf(const std::string& format, Args... args);
    
    template<typename... Args>
    void errorf(const std::string& format, Args... args);
    
    template<typename... Args>
    void criticalf(const std::string& format, Args... args);
    
    // Category-specific logging
    void blockchain(LogLevel level, const std::string& message);
    void network(LogLevel level, const std::string& message);
    void mining(LogLevel level, const std::string& message);
    void wallet(LogLevel level, const std::string& message);
    void consensus(LogLevel level, const std::string& message);
    void api(LogLevel level, const std::string& message);
    void database(LogLevel level, const std::string& message);
    
    // Performance logging
    void performance(const std::string& operation, double duration);
    void throughput(const std::string& metric, uint64_t value);
    void memory(const std::string& component, uint64_t bytes);
    
    // Security logging
    void security(LogLevel level, const std::string& event, const std::string& source = "");
    void audit(const std::string& action, const std::string& user, const std::string& details = "");
    
    // Log file management
    void flush();
    void close();
    std::string getLogFilePath() const;
    uint64_t getLogFileSize() const;
    std::vector<std::string> getLogFiles() const;
    
    // Statistics
    uint64_t getTotalLogEntries() const;
    uint64_t getLogEntriesByLevel(LogLevel level) const;
    std::string getLogStatistics() const;
    
    // Log filtering and searching
    std::vector<std::string> searchLogs(const std::string& pattern) const;
    std::vector<std::string> getLogsByCategory(const std::string& category) const;
    std::vector<std::string> getLogsByLevel(LogLevel level) const;
    std::vector<std::string> getLogsByTimeRange(uint64_t startTime, uint64_t endTime) const;
    
    // Configuration getters
    LogLevel getLogLevel() const { return minLogLevel; }
    bool isConsoleOutputEnabled() const { return consoleOutput; }
    bool isAsyncLoggingEnabled() const { return asyncLogging; }
    uint32_t getMaxFileSize() const { return maxFileSize; }
    uint32_t getMaxFiles() const { return maxFiles; }
    
    // Constants
    static const uint32_t DEFAULT_MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB
    static const uint32_t DEFAULT_MAX_FILES = 10;
    static const std::string DEFAULT_LOG_FILE;
    static const std::string LOG_DATE_FORMAT;
};

// Convenience macros
#define LOG_DEBUG(msg) Logger::getInstance().debug(msg)
#define LOG_INFO(msg) Logger::getInstance().info(msg)
#define LOG_WARNING(msg) Logger::getInstance().warning(msg)
#define LOG_ERROR(msg) Logger::getInstance().error(msg)
#define LOG_CRITICAL(msg) Logger::getInstance().critical(msg)

#define LOG_BLOCKCHAIN(level, msg) Logger::getInstance().blockchain(level, msg)
#define LOG_NETWORK(level, msg) Logger::getInstance().network(level, msg)
#define LOG_MINING(level, msg) Logger::getInstance().mining(level, msg)
#define LOG_WALLET(level, msg) Logger::getInstance().wallet(level, msg)
#define LOG_CONSENSUS(level, msg) Logger::getInstance().consensus(level, msg)
#define LOG_API(level, msg) Logger::getInstance().api(level, msg)
#define LOG_DATABASE(level, msg) Logger::getInstance().database(level, msg)

#define LOG_SECURITY(level, event, source) Logger::getInstance().security(level, event, source)
#define LOG_AUDIT(action, user, details) Logger::getInstance().audit(action, user, details)