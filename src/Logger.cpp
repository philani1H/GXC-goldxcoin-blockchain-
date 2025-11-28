#include "../include/Logger.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <ctime>
#include <condition_variable>

// Static member definitions
std::unique_ptr<Logger> Logger::instance = nullptr;
std::mutex Logger::instanceMutex;

// Static const members are defined inline in C++17
const std::string Logger::DEFAULT_LOG_FILE = "gxc.log";
const std::string Logger::LOG_DATE_FORMAT = "%Y-%m-%d %H:%M:%S";

Logger::Logger() 
    : minLogLevel(LogLevel::INFO), maxFileSize(DEFAULT_MAX_FILE_SIZE), 
      maxFiles(DEFAULT_MAX_FILES), consoleOutput(true), asyncLogging(false),
      shutdownFlag(false) {
}

Logger::~Logger() {
    if (asyncLogging && logThread.joinable()) {
        shutdownFlag = true;
        queueCondition.notify_all();
        logThread.join();
    }
    
    if (logFile.is_open()) {
        logFile.close();
    }
}

Logger& Logger::getInstance() {
    std::lock_guard<std::mutex> lock(instanceMutex);
    if (!instance) {
        instance = std::unique_ptr<Logger>(new Logger());
    }
    return *instance;
}

void Logger::cleanup() {
    std::lock_guard<std::mutex> lock(instanceMutex);
    instance.reset();
}

bool Logger::initialize() {
    return getInstance().initialize(DEFAULT_LOG_FILE, LogLevel::INFO);
}

bool Logger::initialize(const std::string& logPath, LogLevel minLevel) {
    Logger& logger = getInstance();
    logger.logFilePath = logPath;
    logger.minLogLevel = minLevel;
    
    // Create log directory if it doesn't exist
    std::filesystem::path path(logger.logFilePath);
    std::filesystem::path dir = path.parent_path();
    
    if (!dir.empty() && !std::filesystem::exists(dir)) {
        std::filesystem::create_directories(dir);
    }
    
    // Open log file
    logger.logFile.open(logger.logFilePath, std::ios::app);
    if (!logger.logFile.is_open()) {
        std::cerr << "Failed to open log file: " << logger.logFilePath << std::endl;
        return false;
    }
    
    // Start async logging thread if enabled
    if (logger.asyncLogging) {
        logger.logThread = std::thread(&Logger::processLogQueue, &logger);
    }
    
    logger.info("Logger initialized: " + logger.logFilePath);
    return true;
}

void Logger::setLogLevel(LogLevel level) {
    minLogLevel = level;
    info("Log level set to: " + logLevelToString(level));
}

void Logger::setConsoleOutput(bool enabled) {
    consoleOutput = enabled;
}

void Logger::setAsyncLogging(bool enabled) {
    if (asyncLogging != enabled) {
        if (asyncLogging && logThread.joinable()) {
            shutdownFlag = true;
            queueCondition.notify_all();
            logThread.join();
            shutdownFlag = false;
        }
        
        asyncLogging = enabled;
        
        if (asyncLogging) {
            logThread = std::thread(&Logger::processLogQueue, this);
        }
    }
}

void Logger::setFileRotation(uint32_t maxSize, uint32_t maxFiles) {
    this->maxFileSize = maxSize;
    this->maxFiles = maxFiles;
}

void Logger::debug(const std::string& message, const std::string& category) {
    writeLog(LogLevel::DEBUG, message, category);
}

void Logger::info(const std::string& message, const std::string& category) {
    writeLog(LogLevel::INFO, message, category);
}

void Logger::warning(const std::string& message, const std::string& category) {
    writeLog(LogLevel::WARNING, message, category);
}

void Logger::error(const std::string& message, const std::string& category) {
    writeLog(LogLevel::ERROR, message, category);
}

void Logger::critical(const std::string& message, const std::string& category) {
    writeLog(LogLevel::CRITICAL, message, category);
}

void Logger::writeLog(LogLevel level, const std::string& message, const std::string& category) {
    if (level < minLogLevel) {
        return;
    }
    
    std::string formattedMessage = formatLogMessage(level, message, category);
    
    if (asyncLogging) {
        std::lock_guard<std::mutex> lock(queueMutex);
        logQueue.push(formattedMessage);
        queueCondition.notify_one();
    } else {
        // Synchronous logging
        if (logFile.is_open()) {
            logFile << formattedMessage << std::endl;
            logFile.flush();
            
            // Check if file rotation is needed
            if (static_cast<uint32_t>(logFile.tellp()) > maxFileSize) {
                rotateLogFile();
            }
        }
        
        if (consoleOutput) {
            std::cout << formattedMessage << std::endl;
        }
    }
}

void Logger::processLogQueue() {
    while (!shutdownFlag) {
        std::unique_lock<std::mutex> lock(queueMutex);
        queueCondition.wait(lock, [this] { return !logQueue.empty() || shutdownFlag; });
        
        while (!logQueue.empty()) {
            std::string message = logQueue.front();
            logQueue.pop();
            lock.unlock();
            
            if (logFile.is_open()) {
                logFile << message << std::endl;
                logFile.flush();
                
                // Check if file rotation is needed
                if (static_cast<uint32_t>(logFile.tellp()) > maxFileSize) {
                    rotateLogFile();
                }
            }
            
            if (consoleOutput) {
                std::cout << message << std::endl;
            }
            
            lock.lock();
        }
    }
}

std::string Logger::formatLogMessage(LogLevel level, const std::string& message, const std::string& category) const {
    std::ostringstream oss;
    
    // Timestamp
    oss << "[" << getCurrentTimestamp() << "] ";
    
    // Level
    oss << "[" << logLevelToString(level) << "] ";
    
    // Category
    if (!category.empty()) {
        oss << "[" << category << "] ";
    }
    
    // Message
    oss << message;
    
    return oss.str();
}

std::string Logger::getCurrentTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    oss << "." << std::setfill('0') << std::setw(3) << ms.count();
    
    return oss.str();
}

std::string Logger::logLevelToString(LogLevel level) const {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARNING: return "WARN";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::CRITICAL: return "CRIT";
        default: return "UNKNOWN";
    }
}

void Logger::rotateLogFile() {
    if (!logFile.is_open()) {
        return;
    }
    
    logFile.close();
    
    // Delete oldest log file if we've reached the limit
    std::string oldestFile = getRotatedFileName(maxFiles - 1);
    if (std::filesystem::exists(oldestFile)) {
        std::filesystem::remove(oldestFile);
    }
    
    // Rotate existing files
    for (uint32_t i = maxFiles - 1; i > 0; --i) {
        std::string currentFile = getRotatedFileName(i - 1);
        std::string nextFile = getRotatedFileName(i);
        
        if (std::filesystem::exists(currentFile)) {
            std::filesystem::rename(currentFile, nextFile);
        }
    }
    
    // Rename current log file
    std::string firstRotated = getRotatedFileName(0);
    if (std::filesystem::exists(logFilePath)) {
        std::filesystem::rename(logFilePath, firstRotated);
    }
    
    // Open new log file
    logFile.open(logFilePath, std::ios::app);
    
    info("Log file rotated");
}

void Logger::deleteOldLogFiles() {
    for (uint32_t i = maxFiles; i < 100; ++i) { // Check for files beyond the limit
        std::string fileName = getRotatedFileName(i);
        if (std::filesystem::exists(fileName)) {
            std::filesystem::remove(fileName);
        } else {
            break; // No more files to delete
        }
    }
}

std::string Logger::getRotatedFileName(uint32_t index) const {
    std::filesystem::path path(logFilePath);
    std::string stem = path.stem().string();
    std::string extension = path.extension().string();
    std::string directory = path.parent_path().string();
    
    if (!directory.empty()) {
        directory += "/";
    }
    
    return directory + stem + "." + std::to_string(index) + extension;
}

// Category-specific logging methods
void Logger::blockchain(LogLevel level, const std::string& message) {
    writeLog(level, message, "BLOCKCHAIN");
}

void Logger::network(LogLevel level, const std::string& message) {
    writeLog(level, message, "NETWORK");
}

void Logger::mining(LogLevel level, const std::string& message) {
    writeLog(level, message, "MINING");
}

void Logger::wallet(LogLevel level, const std::string& message) {
    writeLog(level, message, "WALLET");
}

void Logger::consensus(LogLevel level, const std::string& message) {
    writeLog(level, message, "CONSENSUS");
}

void Logger::api(LogLevel level, const std::string& message) {
    writeLog(level, message, "API");
}

void Logger::database(LogLevel level, const std::string& message) {
    writeLog(level, message, "DATABASE");
}

// Performance logging
void Logger::performance(const std::string& operation, double duration) {
    std::ostringstream oss;
    oss << operation << " completed in " << std::fixed << std::setprecision(3) << duration << "s";
    writeLog(LogLevel::INFO, oss.str(), "PERFORMANCE");
}

void Logger::throughput(const std::string& metric, uint64_t value) {
    std::ostringstream oss;
    oss << metric << ": " << value;
    writeLog(LogLevel::INFO, oss.str(), "THROUGHPUT");
}

void Logger::memory(const std::string& component, uint64_t bytes) {
    std::ostringstream oss;
    oss << component << " memory usage: " << (bytes / 1024 / 1024) << " MB";
    writeLog(LogLevel::INFO, oss.str(), "MEMORY");
}

// Security logging
void Logger::security(LogLevel level, const std::string& event, const std::string& source) {
    std::string message = event;
    if (!source.empty()) {
        message += " from " + source;
    }
    writeLog(level, message, "SECURITY");
}

void Logger::audit(const std::string& action, const std::string& user, const std::string& details) {
    std::string message = "User '" + user + "' performed action: " + action;
    if (!details.empty()) {
        message += " (" + details + ")";
    }
    writeLog(LogLevel::INFO, message, "AUDIT");
}

// File management
void Logger::flush() {
    if (logFile.is_open()) {
        logFile.flush();
    }
}

void Logger::close() {
    if (asyncLogging && logThread.joinable()) {
        shutdownFlag = true;
        queueCondition.notify_all();
        logThread.join();
    }
    
    if (logFile.is_open()) {
        logFile.close();
    }
}

std::string Logger::getLogFilePath() const {
    return logFilePath;
}

uint64_t Logger::getLogFileSize() const {
    if (std::filesystem::exists(logFilePath)) {
        return std::filesystem::file_size(logFilePath);
    }
    return 0;
}

std::vector<std::string> Logger::getLogFiles() const {
    std::vector<std::string> files;
    
    if (std::filesystem::exists(logFilePath)) {
        files.push_back(logFilePath);
    }
    
    for (uint32_t i = 0; i < maxFiles; ++i) {
        std::string rotatedFile = getRotatedFileName(i);
        if (std::filesystem::exists(rotatedFile)) {
            files.push_back(rotatedFile);
        }
    }
    
    return files;
}

// Statistics (simplified implementation)
uint64_t Logger::getTotalLogEntries() const {
    // This would require keeping a counter in a real implementation
    return 0;
}

uint64_t Logger::getLogEntriesByLevel(LogLevel level) const {
    // This would require level-specific counters in a real implementation
    return 0;
}

std::string Logger::getLogStatistics() const {
    std::ostringstream oss;
    oss << "Log Statistics:\n";
    oss << "  Current log file: " << logFilePath << "\n";
    oss << "  File size: " << getLogFileSize() << " bytes\n";
    oss << "  Log level: " << logLevelToString(minLogLevel) << "\n";
    oss << "  Console output: " << (consoleOutput ? "enabled" : "disabled") << "\n";
    oss << "  Async logging: " << (asyncLogging ? "enabled" : "disabled") << "\n";
    return oss.str();
}

// Search and filtering (simplified implementations)
std::vector<std::string> Logger::searchLogs(const std::string& pattern) const {
    // This would require implementing actual log file searching
    return {};
}

std::vector<std::string> Logger::getLogsByCategory(const std::string& category) const {
    // This would require implementing category filtering
    return {};
}

std::vector<std::string> Logger::getLogsByLevel(LogLevel level) const {
    // This would require implementing level filtering
    return {};
}

std::vector<std::string> Logger::getLogsByTimeRange(uint64_t startTime, uint64_t endTime) const {
    // This would require implementing time range filtering
    return {};
}