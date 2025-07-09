#include "../include/Utils.h"
#include <openssl/sha.h>
#include <openssl/ripemd.h>
#include <openssl/rand.h>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <cstring>
#include <cctype>
#include <random>
#include <fstream>
#include <thread>

namespace Utils {

// String utilities
std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

std::string toLowerCase(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

std::string toUpperCase(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(trim(token));
    }
    
    return tokens;
}

std::string join(const std::vector<std::string>& vec, const std::string& delimiter) {
    if (vec.empty()) return "";
    
    std::ostringstream oss;
    for (size_t i = 0; i < vec.size(); ++i) {
        if (i > 0) oss << delimiter;
        oss << vec[i];
    }
    return oss.str();
}

bool startsWith(const std::string& str, const std::string& prefix) {
    return str.size() >= prefix.size() && 
           str.compare(0, prefix.size(), prefix) == 0;
}

bool endsWith(const std::string& str, const std::string& suffix) {
    return str.size() >= suffix.size() && 
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

std::string replace(const std::string& str, const std::string& from, const std::string& to) {
    std::string result = str;
    size_t pos = 0;
    while ((pos = result.find(from, pos)) != std::string::npos) {
        result.replace(pos, from.length(), to);
        pos += to.length();
    }
    return result;
}

// Encoding/Decoding
std::string toHex(const std::vector<uint8_t>& data) {
    return toHex(data.data(), data.size());
}

std::string toHex(const uint8_t* data, size_t length) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (size_t i = 0; i < length; ++i) {
        oss << std::setw(2) << static_cast<int>(data[i]);
    }
    return oss.str();
}

std::vector<uint8_t> fromHex(const std::string& hex) {
    std::vector<uint8_t> result;
    
    if (hex.length() % 2 != 0) {
        return result; // Invalid hex string
    }
    
    for (size_t i = 0; i < hex.length(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        uint8_t byte = static_cast<uint8_t>(std::strtoul(byteString.c_str(), nullptr, 16));
        result.push_back(byte);
    }
    
    return result;
}

// Base64 encoding (simplified implementation)
std::string toBase64(const std::vector<uint8_t>& data) {
    const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string result;
    
    int val = 0, valb = -6;
    for (uint8_t c : data) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            result.push_back(chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) result.push_back(chars[((val << 8) >> (valb + 8)) & 0x3F]);
    while (result.size() % 4) result.push_back('=');
    
    return result;
}

std::vector<uint8_t> fromBase64(const std::string& base64) {
    const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::vector<uint8_t> result;
    
    int val = 0, valb = -8;
    for (char c : base64) {
        if (c == '=') break;
        auto pos = chars.find(c);
        if (pos == std::string::npos) continue;
        
        val = (val << 6) + pos;
        valb += 6;
        if (valb >= 0) {
            result.push_back(static_cast<uint8_t>((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    
    return result;
}

// Base58 encoding (simplified implementation for Bitcoin addresses)
std::string toBase58(const std::vector<uint8_t>& data) {
    const std::string alphabet = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
    
    // Count leading zeros
    int zeros = 0;
    for (uint8_t byte : data) {
        if (byte == 0) zeros++;
        else break;
    }
    
    // Convert to base58
    std::vector<uint8_t> digits;
    for (size_t i = zeros; i < data.size(); ++i) {
        int carry = data[i];
        for (size_t j = 0; j < digits.size(); ++j) {
            carry += 256 * digits[j];
            digits[j] = carry % 58;
            carry /= 58;
        }
        while (carry) {
            digits.push_back(carry % 58);
            carry /= 58;
        }
    }
    
    // Convert to string
    std::string result(zeros, '1');
    for (int i = digits.size() - 1; i >= 0; --i) {
        result += alphabet[digits[i]];
    }
    
    return result;
}

std::vector<uint8_t> fromBase58(const std::string& base58) {
    // Simplified implementation - would need full implementation for production
    return {};
}

// Cryptographic utilities
std::vector<uint8_t> sha256(const std::vector<uint8_t>& data) {
    return sha256(std::string(data.begin(), data.end()));
}

std::vector<uint8_t> sha256(const std::string& data) {
    std::vector<uint8_t> hash(SHA256_DIGEST_LENGTH);
    SHA256(reinterpret_cast<const unsigned char*>(data.c_str()), data.length(), hash.data());
    return hash;
}

std::vector<uint8_t> doubleSha256(const std::vector<uint8_t>& data) {
    auto firstHash = sha256(data);
    return sha256(firstHash);
}

std::vector<uint8_t> ripemd160(const std::vector<uint8_t>& data) {
    std::vector<uint8_t> hash(RIPEMD160_DIGEST_LENGTH);
    RIPEMD160(data.data(), data.size(), hash.data());
    return hash;
}

std::vector<uint8_t> hash160(const std::vector<uint8_t>& data) {
    auto sha = sha256(data);
    return ripemd160(sha);
}

std::vector<uint8_t> generateRandomBytes(size_t length) {
    std::vector<uint8_t> bytes(length);
    if (RAND_bytes(bytes.data(), static_cast<int>(length)) != 1) {
        // Fallback to C++ random if OpenSSL fails
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 255);
        
        for (size_t i = 0; i < length; ++i) {
            bytes[i] = static_cast<uint8_t>(dis(gen));
        }
    }
    return bytes;
}

uint32_t calculateChecksum(const std::vector<uint8_t>& data) {
    auto hash = doubleSha256(data);
    return (hash[0] << 24) | (hash[1] << 16) | (hash[2] << 8) | hash[3];
}

// Time utilities
uint64_t getCurrentTimestamp() {
    return std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

uint64_t getCurrentTimestampMs() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

std::string formatTimestamp(uint64_t timestamp, const std::string& format) {
    std::time_t time = static_cast<std::time_t>(timestamp);
    std::tm* tm = std::localtime(&time);
    
    std::ostringstream oss;
    oss << std::put_time(tm, format.c_str());
    return oss.str();
}

uint64_t parseTimestamp(const std::string& timeStr, const std::string& format) {
    std::tm tm = {};
    std::istringstream ss(timeStr);
    ss >> std::get_time(&tm, format.c_str());
    return static_cast<uint64_t>(std::mktime(&tm));
}

bool isValidTimestamp(uint64_t timestamp) {
    uint64_t now = getCurrentTimestamp();
    return timestamp <= now && timestamp > 0;
}

double getTimeDifferenceSeconds(uint64_t start, uint64_t end) {
    return static_cast<double>(end - start);
}

// File utilities
bool fileExists(const std::string& path) {
    return std::filesystem::exists(path);
}

bool directoryExists(const std::string& path) {
    return std::filesystem::exists(path) && std::filesystem::is_directory(path);
}

bool createDirectory(const std::string& path) {
    try {
        return std::filesystem::create_directories(path);
    } catch (const std::filesystem::filesystem_error&) {
        return false;
    }
}

bool deleteFile(const std::string& path) {
    try {
        return std::filesystem::remove(path);
    } catch (const std::filesystem::filesystem_error&) {
        return false;
    }
}

bool copyFile(const std::string& source, const std::string& destination) {
    try {
        std::filesystem::copy_file(source, destination);
        return true;
    } catch (const std::filesystem::filesystem_error&) {
        return false;
    }
}

std::string readFileContent(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return "";
    
    std::ostringstream oss;
    oss << file.rdbuf();
    return oss.str();
}

bool writeFileContent(const std::string& path, const std::string& content) {
    std::ofstream file(path);
    if (!file.is_open()) return false;
    
    file << content;
    return file.good();
}

uint64_t getFileSize(const std::string& path) {
    try {
        return std::filesystem::file_size(path);
    } catch (const std::filesystem::filesystem_error&) {
        return 0;
    }
}

std::vector<std::string> listFiles(const std::string& directory) {
    std::vector<std::string> files;
    
    try {
        for (const auto& entry : std::filesystem::directory_iterator(directory)) {
            if (entry.is_regular_file()) {
                files.push_back(entry.path().filename().string());
            }
        }
    } catch (const std::filesystem::filesystem_error&) {
        // Return empty vector on error
    }
    
    return files;
}

std::string getFileExtension(const std::string& path) {
    std::filesystem::path p(path);
    return p.extension().string();
}

std::string getFileName(const std::string& path) {
    std::filesystem::path p(path);
    return p.filename().string();
}

std::string getDirectoryPath(const std::string& path) {
    std::filesystem::path p(path);
    return p.parent_path().string();
}

// Math utilities
template<typename T>
T clamp(T value, T min, T max) {
    return std::max(min, std::min(value, max));
}

// Explicit instantiations
template int clamp<int>(int, int, int);
template double clamp<double>(double, double, double);
template float clamp<float>(float, float, float);

double calculatePercentage(double value, double total) {
    if (total == 0.0) return 0.0;
    return (value / total) * 100.0;
}

uint64_t roundToNearest(double value) {
    return static_cast<uint64_t>(value + 0.5);
}

bool isPowerOfTwo(uint64_t value) {
    return value > 0 && (value & (value - 1)) == 0;
}

uint32_t countBits(uint64_t value) {
    uint32_t count = 0;
    while (value) {
        count += value & 1;
        value >>= 1;
    }
    return count;
}

uint64_t nextPowerOfTwo(uint64_t value) {
    if (value == 0) return 1;
    value--;
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    value |= value >> 32;
    return value + 1;
}

// Validation utilities
bool isValidHex(const std::string& hex) {
    if (hex.empty() || hex.length() % 2 != 0) return false;
    
    for (char c : hex) {
        if (!std::isxdigit(c)) return false;
    }
    return true;
}

bool isValidBase64(const std::string& base64) {
    const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
    
    for (char c : base64) {
        if (chars.find(c) == std::string::npos) return false;
    }
    return true;
}

bool isValidAmount(double amount) {
    return amount >= 0.0 && amount <= MAX_MONEY;
}

bool isValidBlockHeight(uint32_t height) {
    return height >= 0; // All heights are valid
}

bool isValidNonce(uint64_t nonce) {
    return true; // All nonces are valid
}

// Bitcoin address utilities
std::string generateAddress(const std::vector<uint8_t>& publicKey, uint8_t version) {
    auto hash = hash160(publicKey);
    
    std::vector<uint8_t> payload;
    payload.push_back(version);
    payload.insert(payload.end(), hash.begin(), hash.end());
    
    auto checksum = doubleSha256(payload);
    payload.insert(payload.end(), checksum.begin(), checksum.begin() + 4);
    
    return toBase58(payload);
}

bool isValidAddress(const std::string& address) {
    // Simplified validation - would need full implementation for production
    return address.length() >= 26 && address.length() <= 35;
}

std::vector<uint8_t> addressToHash160(const std::string& address) {
    // Simplified implementation - would need full implementation for production
    return {};
}

std::string hash160ToAddress(const std::vector<uint8_t>& hash160, uint8_t version) {
    return generateAddress(hash160, version);
}

// System utilities  
uint64_t getAvailableMemory() {
    // Platform-specific implementation would be needed
    return 0;
}

uint64_t getTotalMemory() {
    // Platform-specific implementation would be needed
    return 0;
}

uint32_t getCpuCoreCount() {
    return std::thread::hardware_concurrency();
}

double getCpuUsage() {
    // Platform-specific implementation would be needed
    return 0.0;
}

std::string getOperatingSystem() {
#ifdef _WIN32
    return "Windows";
#elif __APPLE__
    return "macOS";
#elif __linux__
    return "Linux";
#else
    return "Unknown";
#endif
}

std::string getArchitecture() {
#ifdef _M_X64 || __x86_64__
    return "x64";
#elif _M_IX86 || __i386__
    return "x86";
#elif __aarch64__
    return "ARM64";
#else
    return "Unknown";
#endif
}

std::string getUserHomeDirectory() {
    const char* home = std::getenv("HOME");
    if (!home) {
        home = std::getenv("USERPROFILE");
    }
    return home ? std::string(home) : "";
}

std::string getTempDirectory() {
    return std::filesystem::temp_directory_path().string();
}

// Conversion utilities
std::string doubleToString(double value, int precision) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << value;
    return oss.str();
}

double stringToDouble(const std::string& str) {
    try {
        return std::stod(str);
    } catch (const std::exception&) {
        return 0.0;
    }
}

uint64_t satoshisToAmount(uint64_t satoshis) {
    return satoshis;
}

uint64_t amountToSatoshis(double amount) {
    return static_cast<uint64_t>(amount * SATOSHIS_PER_COIN);
}

std::string formatAmount(uint64_t satoshis, const std::string& unit) {
    double amount = static_cast<double>(satoshis) / SATOSHIS_PER_COIN;
    return doubleToString(amount, 8) + " " + unit;
}

// Timer implementation
Timer::Timer() {
    start();
}

void Timer::start() {
    startTime = std::chrono::high_resolution_clock::now();
}

double Timer::elapsedSeconds() const {
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime);
    return duration.count() / 1000000.0;
}

uint64_t Timer::elapsedMilliseconds() const {
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime);
    return duration.count();
}

uint64_t Timer::elapsedMicroseconds() const {
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime);
    return duration.count();
}

void Timer::reset() {
    start();
}

// Progress tracker implementation
ProgressTracker::ProgressTracker(uint64_t total) : total(total), current(0) {
    startTime = std::chrono::high_resolution_clock::now();
}

void ProgressTracker::update(uint64_t current) {
    this->current = current;
}

double ProgressTracker::getPercentage() const {
    if (total == 0) return 0.0;
    return (static_cast<double>(current) / total) * 100.0;
}

uint64_t ProgressTracker::getETA() const {
    if (current == 0) return 0;
    
    auto now = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();
    
    if (elapsed == 0) return 0;
    
    double rate = static_cast<double>(current) / elapsed;
    if (rate == 0) return 0;
    
    uint64_t remaining = total - current;
    return static_cast<uint64_t>(remaining / rate);
}

double ProgressTracker::getSpeed() const {
    auto now = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();
    
    if (elapsed == 0) return 0.0;
    return static_cast<double>(current) / elapsed;
}

std::string ProgressTracker::getFormattedProgress() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << getPercentage() << "% ";
    oss << "(" << current << "/" << total << ") ";
    oss << "ETA: " << getETA() << "s ";
    oss << "Speed: " << std::setprecision(2) << getSpeed() << " items/s";
    return oss.str();
}

// Network utilities (simplified implementations)
bool isValidIPAddress(const std::string& ip) {
    // Simplified validation
    auto parts = split(ip, '.');
    if (parts.size() != 4) return false;
    
    for (const auto& part : parts) {
        try {
            int value = std::stoi(part);
            if (value < 0 || value > 255) return false;
        } catch (const std::exception&) {
            return false;
        }
    }
    return true;
}

bool isValidPort(uint16_t port) {
    return port > 0 && port <= 65535;
}

std::string getLocalIPAddress() {
    return "127.0.0.1"; // Simplified
}

bool isLocalAddress(const std::string& address) {
    return address == "127.0.0.1" || address == "localhost" || startsWith(address, "192.168.");
}

bool isPrivateAddress(const std::string& address) {
    return startsWith(address, "10.") || startsWith(address, "172.") || startsWith(address, "192.168.");
}

std::vector<std::string> resolveHostname(const std::string& hostname) {
    // Would need platform-specific DNS resolution
    return {hostname};
}

uint16_t getAvailablePort(uint16_t startPort) {
    // Simplified implementation
    return startPort;
}

// Random utilities
uint32_t randomUint32() {
    auto bytes = generateRandomBytes(4);
    return (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];
}

uint64_t randomUint64() {
    auto bytes = generateRandomBytes(8);
    uint64_t result = 0;
    for (int i = 0; i < 8; ++i) {
        result = (result << 8) | bytes[i];
    }
    return result;
}

double randomDouble(double min, double max) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(min, max);
    return dis(gen);
}

std::string randomString(size_t length) {
    const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    std::string result;
    
    for (size_t i = 0; i < length; ++i) {
        result += chars[randomUint32() % chars.length()];
    }
    
    return result;
}

std::vector<uint8_t> randomBytes(size_t length) {
    return generateRandomBytes(length);
}

// Configuration utilities
std::string expandPath(const std::string& path) {
    if (startsWith(path, "~/")) {
        return getUserHomeDirectory() + path.substr(1);
    }
    return path;
}

std::string getConfigDirectory() {
    return expandPath("~/.gxc");
}

std::string getDataDirectory() {
    return expandPath("~/.gxc/data");
}

std::string getLogDirectory() {
    return expandPath("~/.gxc/logs");
}

// Error handling utilities
std::string getLastSystemError() {
    return std::strerror(errno);
}

std::string formatError(int errorCode) {
    return std::strerror(errorCode);
}

void printStackTrace() {
    // Would need platform-specific implementation
}

// Serialization utilities (simplified implementations)
std::vector<uint8_t> serializeUint32(uint32_t value) {
    return {
        static_cast<uint8_t>(value >> 24),
        static_cast<uint8_t>(value >> 16),
        static_cast<uint8_t>(value >> 8),
        static_cast<uint8_t>(value)
    };
}

std::vector<uint8_t> serializeUint64(uint64_t value) {
    std::vector<uint8_t> result;
    for (int i = 7; i >= 0; --i) {
        result.push_back(static_cast<uint8_t>(value >> (i * 8)));
    }
    return result;
}

std::vector<uint8_t> serializeDouble(double value) {
    std::vector<uint8_t> result(sizeof(double));
    std::memcpy(result.data(), &value, sizeof(double));
    return result;
}

std::vector<uint8_t> serializeString(const std::string& str) {
    auto lengthBytes = serializeUint32(static_cast<uint32_t>(str.length()));
    std::vector<uint8_t> result = lengthBytes;
    result.insert(result.end(), str.begin(), str.end());
    return result;
}

uint32_t deserializeUint32(const std::vector<uint8_t>& data, size_t& offset) {
    if (offset + 4 > data.size()) return 0;
    
    uint32_t value = (data[offset] << 24) | (data[offset + 1] << 16) | 
                     (data[offset + 2] << 8) | data[offset + 3];
    offset += 4;
    return value;
}

uint64_t deserializeUint64(const std::vector<uint8_t>& data, size_t& offset) {
    if (offset + 8 > data.size()) return 0;
    
    uint64_t value = 0;
    for (int i = 0; i < 8; ++i) {
        value = (value << 8) | data[offset + i];
    }
    offset += 8;
    return value;
}

double deserializeDouble(const std::vector<uint8_t>& data, size_t& offset) {
    if (offset + sizeof(double) > data.size()) return 0.0;
    
    double value;
    std::memcpy(&value, &data[offset], sizeof(double));
    offset += sizeof(double);
    return value;
}

std::string deserializeString(const std::vector<uint8_t>& data, size_t& offset) {
    uint32_t length = deserializeUint32(data, offset);
    if (offset + length > data.size()) return "";
    
    std::string result(data.begin() + offset, data.begin() + offset + length);
    offset += length;
    return result;
}

// Template implementations for memory utilities
template<typename T>
void secureZeroMemory(T* ptr, size_t count) {
    volatile T* vptr = ptr;
    for (size_t i = 0; i < count; ++i) {
        vptr[i] = T{};
    }
}

template<typename T>
bool constantTimeEquals(const T* a, const T* b, size_t length) {
    T result = 0;
    for (size_t i = 0; i < length; ++i) {
        result |= a[i] ^ b[i];
    }
    return result == 0;
}

// Explicit instantiations
template void secureZeroMemory<uint8_t>(uint8_t*, size_t);
template void secureZeroMemory<char>(char*, size_t);
template bool constantTimeEquals<uint8_t>(const uint8_t*, const uint8_t*, size_t);
template bool constantTimeEquals<char>(const char*, const char*, size_t);

} // namespace Utils