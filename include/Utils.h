#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <chrono>

namespace Utils {
    // String utilities
    std::string trim(const std::string& str);
    std::string toLowerCase(const std::string& str);
    std::string toUpperCase(const std::string& str);
    std::vector<std::string> split(const std::string& str, char delimiter);
    std::string join(const std::vector<std::string>& vec, const std::string& delimiter);
    bool startsWith(const std::string& str, const std::string& prefix);
    bool endsWith(const std::string& str, const std::string& suffix);
    std::string replace(const std::string& str, const std::string& from, const std::string& to);
    
    // Encoding/Decoding
    std::string toHex(const std::vector<uint8_t>& data);
    std::string toHex(const uint8_t* data, size_t length);
    std::vector<uint8_t> fromHex(const std::string& hex);
    std::string toBase64(const std::vector<uint8_t>& data);
    std::vector<uint8_t> fromBase64(const std::string& base64);
    std::string toBase58(const std::vector<uint8_t>& data);
    std::vector<uint8_t> fromBase58(const std::string& base58);
    
    // Cryptographic utilities
    std::vector<uint8_t> sha256(const std::vector<uint8_t>& data);
    std::vector<uint8_t> sha256(const std::string& data);
    std::vector<uint8_t> doubleSha256(const std::vector<uint8_t>& data);
    std::vector<uint8_t> ripemd160(const std::vector<uint8_t>& data);
    std::vector<uint8_t> hash160(const std::vector<uint8_t>& data); // SHA256 + RIPEMD160
    std::vector<uint8_t> generateRandomBytes(size_t length);
    uint32_t calculateChecksum(const std::vector<uint8_t>& data);
    
    // Time utilities
    uint64_t getCurrentTimestamp();
    uint64_t getCurrentTimestampMs();
    std::string formatTimestamp(uint64_t timestamp, const std::string& format = "%Y-%m-%d %H:%M:%S");
    uint64_t parseTimestamp(const std::string& timeStr, const std::string& format = "%Y-%m-%d %H:%M:%S");
    bool isValidTimestamp(uint64_t timestamp);
    double getTimeDifferenceSeconds(uint64_t start, uint64_t end);
    
    // File utilities
    bool fileExists(const std::string& path);
    bool directoryExists(const std::string& path);
    bool createDirectory(const std::string& path);
    bool deleteFile(const std::string& path);
    bool copyFile(const std::string& source, const std::string& destination);
    std::string readFileContent(const std::string& path);
    bool writeFileContent(const std::string& path, const std::string& content);
    uint64_t getFileSize(const std::string& path);
    std::vector<std::string> listFiles(const std::string& directory);
    std::string getFileExtension(const std::string& path);
    std::string getFileName(const std::string& path);
    std::string getDirectoryPath(const std::string& path);
    
    // Network utilities
    bool isValidIPAddress(const std::string& ip);
    bool isValidPort(uint16_t port);
    std::string getLocalIPAddress();
    bool isLocalAddress(const std::string& address);
    bool isPrivateAddress(const std::string& address);
    std::vector<std::string> resolveHostname(const std::string& hostname);
    uint16_t getAvailablePort(uint16_t startPort = 8000);
    
    // Bitcoin address utilities
    std::string generateAddress(const std::vector<uint8_t>& publicKey, uint8_t version = 0);
    bool isValidAddress(const std::string& address);
    std::vector<uint8_t> addressToHash160(const std::string& address);
    std::string hash160ToAddress(const std::vector<uint8_t>& hash160, uint8_t version = 0);
    
    // Math utilities
    template<typename T>
    T clamp(T value, T min, T max);
    
    double calculatePercentage(double value, double total);
    uint64_t roundToNearest(double value);
    bool isPowerOfTwo(uint64_t value);
    uint32_t countBits(uint64_t value);
    uint64_t nextPowerOfTwo(uint64_t value);
    
    // Validation utilities
    bool isNumeric(const std::string& str);
    bool isValidHex(const std::string& hex);
    bool isValidBase64(const std::string& base64);
    bool isValidAmount(double amount);
    bool isValidBlockHeight(uint32_t height);
    bool isValidNonce(uint64_t nonce);
    
    // System utilities
    uint64_t getAvailableMemory();
    uint64_t getTotalMemory();
    uint32_t getCpuCoreCount();
    double getCpuUsage();
    std::string getOperatingSystem();
    std::string getArchitecture();
    std::string getUserHomeDirectory();
    std::string getTempDirectory();
    
    // Performance utilities
    class Timer {
    private:
        std::chrono::high_resolution_clock::time_point startTime;
        
    public:
        Timer();
        void start();
        double elapsedSeconds() const;
        uint64_t elapsedMilliseconds() const;
        uint64_t elapsedMicroseconds() const;
        void reset();
    };
    
    // Memory utilities
    template<typename T>
    void secureZeroMemory(T* ptr, size_t count);
    
    template<typename T>
    bool constantTimeEquals(const T* a, const T* b, size_t length);
    
    // Serialization utilities
    std::vector<uint8_t> serializeUint32(uint32_t value);
    std::vector<uint8_t> serializeUint64(uint64_t value);
    std::vector<uint8_t> serializeDouble(double value);
    std::vector<uint8_t> serializeString(const std::string& str);
    uint32_t deserializeUint32(const std::vector<uint8_t>& data, size_t& offset);
    uint64_t deserializeUint64(const std::vector<uint8_t>& data, size_t& offset);
    double deserializeDouble(const std::vector<uint8_t>& data, size_t& offset);
    std::string deserializeString(const std::vector<uint8_t>& data, size_t& offset);
    
    // Configuration utilities
    std::string expandPath(const std::string& path);
    std::string getConfigDirectory();
    std::string getDataDirectory();
    std::string getLogDirectory();
    
    // Error handling utilities
    std::string getLastSystemError();
    std::string formatError(int errorCode);
    void printStackTrace();
    
    // Random utilities
    uint32_t randomUint32();
    uint64_t randomUint64();
    double randomDouble(double min = 0.0, double max = 1.0);
    std::string randomString(size_t length);
    std::vector<uint8_t> randomBytes(size_t length);
    
    // Conversion utilities
    std::string doubleToString(double value, int precision = 8);
    double stringToDouble(const std::string& str);
    uint64_t satoshisToAmount(uint64_t satoshis);
    uint64_t amountToSatoshis(double amount);
    std::string formatAmount(uint64_t satoshis, const std::string& unit = "GXC");
    
    // Progress tracking
    class ProgressTracker {
    private:
        uint64_t total;
        uint64_t current;
        std::chrono::high_resolution_clock::time_point startTime;
        
    public:
        ProgressTracker(uint64_t total);
        void update(uint64_t current);
        double getPercentage() const;
        uint64_t getETA() const; // Estimated time of arrival in seconds
        double getSpeed() const; // Items per second
        std::string getFormattedProgress() const;
    };
    
    // Constants
    static const uint8_t BITCOIN_ADDRESS_VERSION = 0x00;
    static const uint8_t TESTNET_ADDRESS_VERSION = 0x6F;
    static const uint64_t SATOSHIS_PER_COIN = 100000000;
    static const double MAX_MONEY = 31000000.0; // 31 million GXC
    static const uint32_t COINBASE_MATURITY = 100; // Blocks
    static const uint32_t MAX_BLOCK_SIZE = 32 * 1024 * 1024; // 32MB
}