#include "../include/Utils.h"
#include <random>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <thread>
#include <ctime>

namespace Utils {

uint64_t getCurrentTimestamp() {
    return static_cast<uint64_t>(std::time(nullptr));
}

uint32_t randomUint32() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<uint32_t> dis;
    return dis(gen);
}

uint64_t randomUint64() {
    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    static std::uniform_int_distribution<uint64_t> dis;
    return dis(gen);
}

std::string randomString(size_t length) {
    const std::string chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::string result;
    result.reserve(length);
    
    for (size_t i = 0; i < length; ++i) {
        result += chars[randomUint32() % chars.length()];
    }
    
    return result;
}

std::string bytesToHex(const std::vector<uint8_t>& bytes) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    
    for (const auto& byte : bytes) {
        oss << std::setw(2) << static_cast<int>(byte);
    }
    
    return oss.str();
}

std::vector<uint8_t> hexToBytes(const std::string& hex) {
    std::vector<uint8_t> bytes;
    
    for (size_t i = 0; i < hex.length(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        uint8_t byte = static_cast<uint8_t>(std::stoul(byteString, nullptr, 16));
        bytes.push_back(byte);
    }
    
    return bytes;
}

bool isValidHex(const std::string& hex) {
    if (hex.empty() || hex.length() % 2 != 0) {
        return false;
    }
    
    return std::all_of(hex.begin(), hex.end(), [](char c) {
        return std::isxdigit(c);
    });
}

std::string formatAmount(double amount, int precision) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << amount;
    return oss.str();
}

double parseAmount(const std::string& amountStr) {
    try {
        return std::stod(amountStr);
    } catch (const std::exception&) {
        return 0.0;
    }
}

bool isValidAddress(const std::string& address) {
    // Basic address validation
    if (address.empty() || address.length() < 26 || address.length() > 62) {
        return false;
    }
    
    // Check for valid characters (alphanumeric)
    return std::all_of(address.begin(), address.end(), [](char c) {
        return std::isalnum(c);
    });
}

std::string generateAddress() {
    // Generate a simplified address (in real implementation would use proper cryptography)
    return "gxc" + randomString(30);
}

bool fileExists(const std::string& filename) {
    std::ifstream file(filename);
    return file.good();
}

bool createDirectory(const std::string& path) {
    // Simplified directory creation - would use proper OS calls in real implementation
    return true;
}

std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return "";
    }
    
    std::string content;
    std::string line;
    while (std::getline(file, line)) {
        content += line + "\n";
    }
    
    return content;
}

bool writeFile(const std::string& filename, const std::string& content) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    file << content;
    return true;
}

std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) {
        return "";
    }
    
    size_t end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    
    return tokens;
}

std::string join(const std::vector<std::string>& strings, const std::string& delimiter) {
    if (strings.empty()) {
        return "";
    }
    
    std::string result = strings[0];
    for (size_t i = 1; i < strings.size(); ++i) {
        result += delimiter + strings[i];
    }
    
    return result;
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

bool startsWith(const std::string& str, const std::string& prefix) {
    if (prefix.length() > str.length()) {
        return false;
    }
    
    return str.substr(0, prefix.length()) == prefix;
}

bool endsWith(const std::string& str, const std::string& suffix) {
    if (suffix.length() > str.length()) {
        return false;
    }
    
    return str.substr(str.length() - suffix.length()) == suffix;
}

std::string formatTimestamp(uint64_t timestamp, const std::string& format) {
    std::time_t time = static_cast<std::time_t>(timestamp);
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time), format.c_str());
    return oss.str();
}

std::time_t parseTimestamp(const std::string& timestampStr) {
    // Simplified timestamp parsing
    try {
        return static_cast<std::time_t>(std::stoull(timestampStr));
    } catch (const std::exception&) {
        return 0;
    }
}

void sleep(uint32_t milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

uint64_t getMillisecondTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    return static_cast<uint64_t>(ms.count());
}

double calculatePercentage(double value, double total) {
    if (total == 0.0) {
        return 0.0;
    }
    
    return (value / total) * 100.0;
}

bool isNumeric(const std::string& str) {
    if (str.empty()) {
        return false;
    }
    
    return std::all_of(str.begin(), str.end(), [](char c) {
        return std::isdigit(c) || c == '.';
    });
}

std::string escapeJson(const std::string& str) {
    std::string escaped;
    for (char c : str) {
        switch (c) {
            case '"': escaped += "\\\""; break;
            case '\\': escaped += "\\\\"; break;
            case '\n': escaped += "\\n"; break;
            case '\r': escaped += "\\r"; break;
            case '\t': escaped += "\\t"; break;
            default: escaped += c; break;
        }
    }
    return escaped;
}

std::string generateUUID() {
    // Simplified UUID generation
    std::string uuid = randomString(8) + "-" + randomString(4) + "-" + 
                      randomString(4) + "-" + randomString(4) + "-" + randomString(12);
    return toLowerCase(uuid);
}

bool validateIPAddress(const std::string& ip) {
    std::vector<std::string> parts = split(ip, '.');
    if (parts.size() != 4) {
        return false;
    }
    
    for (const auto& part : parts) {
        if (!isNumeric(part)) {
            return false;
        }
        
        int num = std::stoi(part);
        if (num < 0 || num > 255) {
            return false;
        }
    }
    
    return true;
}

uint16_t parsePort(const std::string& portStr) {
    try {
        int port = std::stoi(portStr);
        if (port < 1 || port > 65535) {
            return 0;
        }
        return static_cast<uint16_t>(port);
    } catch (const std::exception&) {
        return 0;
    }
}

std::string formatSize(uint64_t bytes) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int unit = 0;
    double size = static_cast<double>(bytes);
    
    while (size >= 1024.0 && unit < 4) {
        size /= 1024.0;
        unit++;
    }
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << size << " " << units[unit];
    return oss.str();
}

std::string encodeBase64(const std::vector<uint8_t>& data) {
    // Simplified base64 encoding
    const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string result;
    
    for (size_t i = 0; i < data.size(); i += 3) {
        uint32_t value = data[i] << 16;
        if (i + 1 < data.size()) value |= data[i + 1] << 8;
        if (i + 2 < data.size()) value |= data[i + 2];
        
        result += chars[(value >> 18) & 0x3F];
        result += chars[(value >> 12) & 0x3F];
        result += (i + 1 < data.size()) ? chars[(value >> 6) & 0x3F] : '=';
        result += (i + 2 < data.size()) ? chars[value & 0x3F] : '=';
    }
    
    return result;
}

std::vector<uint8_t> decodeBase64(const std::string& encoded) {
    // Simplified base64 decoding
    std::vector<uint8_t> result;
    // Implementation would go here in production
    return result;
}

} // namespace Utils