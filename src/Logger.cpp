#include "Logger.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <chrono>
#include <filesystem>
#include <thread>

// Static member definitions
Logger::LogLevel Logger::currentLevel_ = Logger::LogLevel::INFO;
std::string Logger::logFile_;
std::ofstream Logger::fileStream_;
std::mutex Logger::logMutex_;
bool Logger::initialized_ = false;
size_t Logger::maxFileSize_ = 100 * 1024 * 1024; // 100MB
int Logger::maxRotateFiles_ = 5;

void Logger::initialize(const std::string& logFile, LogLevel level) {
    std::lock_guard<std::mutex> lock(logMutex_);
    
    logFile_ = logFile;
    currentLevel_ = level;
    
    // Create log directory if it doesn't exist
    std::filesystem::path logPath(logFile_);
    std::filesystem::create_directories(logPath.parent_path());
    
    // Open log file
    fileStream_.open(logFile_, std::ios::app);
    if (!fileStream_.is_open()) {
        std::cerr << "Warning: Could not open log file: " << logFile_ << std::endl;
    }
    
    initialized_ = true;
    
    // Log initialization message
    info("Logger initialized - File: " + logFile_ + ", Level: " + levelToString(level));
}

void Logger::setLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(logMutex_);
    currentLevel_ = level;
}

void Logger::setLevel(const std::string& levelStr) {
    LogLevel level = stringToLevel(levelStr);
    setLevel(level);
}

void Logger::setMaxFileSize(size_t sizeBytes) {
    std::lock_guard<std::mutex> lock(logMutex_);
    maxFileSize_ = sizeBytes;
}

void Logger::setMaxRotateFiles(int count) {
    std::lock_guard<std::mutex> lock(logMutex_);
    maxRotateFiles_ = count;
}

void Logger::debug(const std::string& message) {
    log(LogLevel::DEBUG, message);
}

void Logger::info(const std::string& message) {
    log(LogLevel::INFO, message);
}

void Logger::warning(const std::string& message) {
    log(LogLevel::WARNING, message);
}

void Logger::error(const std::string& message) {
    log(LogLevel::ERROR, message);
}

void Logger::critical(const std::string& message) {
    log(LogLevel::CRITICAL, message);
}

void Logger::log(LogLevel level, const std::string& message) {
    if (!shouldLog(level)) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(logMutex_);
    
    std::string timestamp = getCurrentTimestamp();
    std::string levelStr = levelToString(level);
    std::string threadId = getThreadId();
    
    std::string logEntry = "[" + timestamp + "] [" + levelStr + "] [" + threadId + "] " + message;
    
    // Write to console
    if (level >= LogLevel::WARNING) {
        std::cerr << logEntry << std::endl;
    } else {
        std::cout << logEntry << std::endl;
    }
    
    // Write to file if initialized
    if (initialized_ && fileStream_.is_open()) {
        fileStream_ << logEntry << std::endl;
        fileStream_.flush();
        
        // Check if we need to rotate the log file
        checkAndRotateFile();
    }
}

bool Logger::shouldLog(LogLevel level) {
    return level >= currentLevel_;
}

std::string Logger::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    
    return ss.str();
}

std::string Logger::getThreadId() {
    std::stringstream ss;
    ss << std::this_thread::get_id();
    return ss.str();
}

std::string Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:    return "DEBUG";
        case LogLevel::INFO:     return "INFO ";
        case LogLevel::WARNING:  return "WARN ";
        case LogLevel::ERROR:    return "ERROR";
        case LogLevel::CRITICAL: return "CRIT ";
        default:                 return "UNKNOWN";
    }
}

Logger::LogLevel Logger::stringToLevel(const std::string& levelStr) {
    std::string lower = levelStr;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    
    if (lower == "debug") return LogLevel::DEBUG;
    if (lower == "info") return LogLevel::INFO;
    if (lower == "warning" || lower == "warn") return LogLevel::WARNING;
    if (lower == "error") return LogLevel::ERROR;
    if (lower == "critical" || lower == "crit") return LogLevel::CRITICAL;
    
    return LogLevel::INFO; // Default
}

void Logger::checkAndRotateFile() {
    if (!fileStream_.is_open() || logFile_.empty()) {
        return;
    }
    
    // Get current file size
    fileStream_.seekp(0, std::ios::end);
    size_t currentSize = fileStream_.tellp();
    
    if (currentSize >= maxFileSize_) {
        rotateLogFiles();
    }
}

void Logger::rotateLogFiles() {
    if (logFile_.empty()) {
        return;
    }
    
    // Close current file
    fileStream_.close();
    
    try {
        // Rotate existing files
        for (int i = maxRotateFiles_ - 1; i >= 1; i--) {
            std::string oldFile = logFile_ + "." + std::to_string(i);
            std::string newFile = logFile_ + "." + std::to_string(i + 1);
            
            if (std::filesystem::exists(oldFile)) {
                if (i == maxRotateFiles_ - 1) {
                    // Delete the oldest file
                    std::filesystem::remove(newFile);
                }
                std::filesystem::rename(oldFile, newFile);
            }
        }
        
        // Move current log file to .1
        std::string firstRotated = logFile_ + ".1";
        if (std::filesystem::exists(logFile_)) {
            std::filesystem::rename(logFile_, firstRotated);
        }
        
        // Open new log file
        fileStream_.open(logFile_, std::ios::out | std::ios::trunc);
        
        if (fileStream_.is_open()) {
            info("Log file rotated successfully");
        } else {
            std::cerr << "Error: Could not reopen log file after rotation: " << logFile_ << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error during log rotation: " << e.what() << std::endl;
        
        // Try to reopen the original file
        fileStream_.open(logFile_, std::ios::app);
    }
}

void Logger::flush() {
    std::lock_guard<std::mutex> lock(logMutex_);
    
    if (fileStream_.is_open()) {
        fileStream_.flush();
    }
    
    std::cout.flush();
    std::cerr.flush();
}

void Logger::shutdown() {
    std::lock_guard<std::mutex> lock(logMutex_);
    
    if (initialized_) {
        info("Logger shutting down");
        
        if (fileStream_.is_open()) {
            fileStream_.close();
        }
        
        initialized_ = false;
    }
}

// Convenience methods for common log patterns
void Logger::logPerformance(const std::string& operation, double durationMs) {
    info("PERF: " + operation + " completed in " + std::to_string(durationMs) + "ms");
}

void Logger::logNetwork(const std::string& event, const std::string& peer) {
    info("NET: " + event + " - " + peer);
}

void Logger::logMining(const std::string& event, const std::string& details) {
    info("MINING: " + event + " - " + details);
}

void Logger::logSecurity(const std::string& event, const std::string& details) {
    warning("SECURITY: " + event + " - " + details);
}

void Logger::logTransaction(const std::string& txHash, const std::string& event) {
    debug("TX: " + event + " - " + txHash.substr(0, 16) + "...");
}

void Logger::logBlock(int height, const std::string& hash, const std::string& event) {
    info("BLOCK: " + event + " - Height: " + std::to_string(height) + 
         ", Hash: " + hash.substr(0, 16) + "...");
}

// Exception logging helpers
void Logger::logException(const std::string& context, const std::exception& e) {
    error("EXCEPTION in " + context + ": " + e.what());
}

void Logger::logCriticalException(const std::string& context, const std::exception& e) {
    critical("CRITICAL EXCEPTION in " + context + ": " + e.what());
}

// Debugging helpers
void Logger::hexDump(const std::string& label, const std::vector<uint8_t>& data) {
    if (!shouldLog(LogLevel::DEBUG)) {
        return;
    }
    
    std::stringstream ss;
    ss << label << " (" << data.size() << " bytes):\n";
    
    for (size_t i = 0; i < data.size(); i += 16) {
        // Address
        ss << std::setfill('0') << std::setw(8) << std::hex << i << ": ";
        
        // Hex values
        for (size_t j = 0; j < 16; j++) {
            if (i + j < data.size()) {
                ss << std::setfill('0') << std::setw(2) << std::hex << 
                      static_cast<unsigned int>(data[i + j]) << " ";
            } else {
                ss << "   ";
            }
            
            if (j == 7) ss << " ";
        }
        
        // ASCII representation
        ss << " |";
        for (size_t j = 0; j < 16 && (i + j) < data.size(); j++) {
            char c = static_cast<char>(data[i + j]);
            ss << (std::isprint(c) ? c : '.');
        }
        ss << "|";
        
        if (i + 16 < data.size()) ss << "\n";
    }
    
    debug(ss.str());
}

// Statistics and monitoring
std::map<std::string, size_t> Logger::getLogStatistics() {
    std::lock_guard<std::mutex> lock(logMutex_);
    
    std::map<std::string, size_t> stats;
    
    // These would be populated if we were tracking log counts
    stats["debug_count"] = 0;
    stats["info_count"] = 0;
    stats["warning_count"] = 0;
    stats["error_count"] = 0;
    stats["critical_count"] = 0;
    
    // File size
    if (fileStream_.is_open()) {
        auto pos = fileStream_.tellp();
        fileStream_.seekp(0, std::ios::end);
        stats["file_size"] = fileStream_.tellp();
        fileStream_.seekp(pos);
    } else {
        stats["file_size"] = 0;
    }
    
    return stats;
}

// Configuration helpers
Logger::LogLevel Logger::getCurrentLevel() {
    std::lock_guard<std::mutex> lock(logMutex_);
    return currentLevel_;
}

std::string Logger::getCurrentLevelString() {
    return levelToString(getCurrentLevel());
}

bool Logger::isInitialized() {
    std::lock_guard<std::mutex> lock(logMutex_);
    return initialized_;
}

std::string Logger::getLogFile() {
    std::lock_guard<std::mutex> lock(logMutex_);
    return logFile_;
}