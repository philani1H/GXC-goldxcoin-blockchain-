#include "../include/AddressRegistry.h"
#include "../include/Logger.h"
#include <algorithm>
#include <iostream>

AddressRegistry::AddressRegistry(const std::string& filePath) 
    : registryFilePath(filePath) {
    load();
    LOG_INFO("Address Registry initialized with " + std::to_string(registry.size()) + " entities");
}

AddressRegistry::~AddressRegistry() {
    save();
}

std::string AddressRegistry::entityTypeToString(EntityType type) const {
    switch (type) {
        case EntityType::EXCHANGE: return "EXCHANGE";
        case EntityType::STAKING_POOL: return "STAKING_POOL";
        case EntityType::MERCHANT: return "MERCHANT";
        case EntityType::VALIDATOR: return "VALIDATOR";
        default: return "UNKNOWN";
    }
}

AddressRegistry::EntityType AddressRegistry::stringToEntityType(const std::string& str) const {
    if (str == "EXCHANGE") return EntityType::EXCHANGE;
    if (str == "STAKING_POOL") return EntityType::STAKING_POOL;
    if (str == "MERCHANT") return EntityType::MERCHANT;
    if (str == "VALIDATOR") return EntityType::VALIDATOR;
    return EntityType::UNKNOWN;
}

// ============================================================================
// REGISTRATION METHODS
// ============================================================================

void AddressRegistry::registerExchange(const std::string& address, const std::string& name,
                                      const std::string& website,
                                      const std::string& verificationSource) {
    EntityInfo info;
    info.type = EntityType::EXCHANGE;
    info.name = name;
    info.address = address;
    info.website = website;
    info.verificationSource = verificationSource;
    info.registeredAt = std::time(nullptr);
    info.lastVerified = 0;
    info.verified = !verificationSource.empty();
    
    registry[address] = info;
    save();
    
    LOG_SECURITY(LogLevel::INFO, 
        "Exchange registered: " + name + " (" + address + ")");
}

void AddressRegistry::registerStakingPool(const std::string& address, const std::string& name,
                                         const std::string& website,
                                         const std::string& verificationSource) {
    EntityInfo info;
    info.type = EntityType::STAKING_POOL;
    info.name = name;
    info.address = address;
    info.website = website;
    info.verificationSource = verificationSource;
    info.registeredAt = std::time(nullptr);
    info.lastVerified = 0;
    info.verified = !verificationSource.empty();
    
    registry[address] = info;
    save();
    
    LOG_SECURITY(LogLevel::INFO, 
        "Staking pool registered: " + name + " (" + address + ")");
}

void AddressRegistry::registerMerchant(const std::string& address, const std::string& name,
                                      const std::string& website,
                                      const std::string& verificationSource) {
    EntityInfo info;
    info.type = EntityType::MERCHANT;
    info.name = name;
    info.address = address;
    info.website = website;
    info.verificationSource = verificationSource;
    info.registeredAt = std::time(nullptr);
    info.lastVerified = 0;
    info.verified = !verificationSource.empty();
    
    registry[address] = info;
    save();
    
    LOG_SECURITY(LogLevel::INFO, 
        "Merchant registered: " + name + " (" + address + ")");
}

void AddressRegistry::registerValidator(const std::string& address, const std::string& name,
                                       const std::string& website,
                                       const std::string& verificationSource) {
    EntityInfo info;
    info.type = EntityType::VALIDATOR;
    info.name = name;
    info.address = address;
    info.website = website;
    info.verificationSource = verificationSource;
    info.registeredAt = std::time(nullptr);
    info.lastVerified = 0;
    info.verified = !verificationSource.empty();
    
    registry[address] = info;
    save();
    
    LOG_SECURITY(LogLevel::INFO, 
        "Validator registered: " + name + " (" + address + ")");
}

// ============================================================================
// VERIFICATION
// ============================================================================

void AddressRegistry::markAsVerified(const std::string& address, 
                                     const std::string& verificationSource) {
    auto it = registry.find(address);
    if (it != registry.end()) {
        it->second.verified = true;
        it->second.verificationSource = verificationSource;
        it->second.lastVerified = std::time(nullptr);
        save();
        
        LOG_SECURITY(LogLevel::INFO, 
            "Entity verified: " + it->second.name + " via " + verificationSource);
    }
}

void AddressRegistry::updateLastVerified(const std::string& address) {
    auto it = registry.find(address);
    if (it != registry.end()) {
        it->second.lastVerified = std::time(nullptr);
        save();
    }
}

// ============================================================================
// QUERY METHODS
// ============================================================================

bool AddressRegistry::isRegistered(const std::string& address) const {
    return registry.find(address) != registry.end();
}

bool AddressRegistry::isVerified(const std::string& address) const {
    auto it = registry.find(address);
    return (it != registry.end() && it->second.verified);
}

AddressRegistry::EntityType AddressRegistry::getEntityType(const std::string& address) const {
    auto it = registry.find(address);
    return (it != registry.end()) ? it->second.type : EntityType::UNKNOWN;
}

AddressRegistry::EntityInfo AddressRegistry::getEntityInfo(const std::string& address) const {
    auto it = registry.find(address);
    return (it != registry.end()) ? it->second : EntityInfo();
}

std::string AddressRegistry::getEntityName(const std::string& address) const {
    auto it = registry.find(address);
    return (it != registry.end()) ? it->second.name : "";
}

// ============================================================================
// BULK OPERATIONS
// ============================================================================

std::vector<std::string> AddressRegistry::getAllAddresses() const {
    std::vector<std::string> addresses;
    addresses.reserve(registry.size());
    
    for (const auto& pair : registry) {
        addresses.push_back(pair.first);
    }
    
    return addresses;
}

std::vector<std::string> AddressRegistry::getAddressesByType(EntityType type) const {
    std::vector<std::string> addresses;
    
    for (const auto& pair : registry) {
        if (pair.second.type == type) {
            addresses.push_back(pair.first);
        }
    }
    
    return addresses;
}

std::vector<AddressRegistry::EntityInfo> AddressRegistry::getAllEntities() const {
    std::vector<EntityInfo> entities;
    entities.reserve(registry.size());
    
    for (const auto& pair : registry) {
        entities.push_back(pair.second);
    }
    
    return entities;
}

std::vector<AddressRegistry::EntityInfo> AddressRegistry::getVerifiedEntities() const {
    std::vector<EntityInfo> entities;
    
    for (const auto& pair : registry) {
        if (pair.second.verified) {
            entities.push_back(pair.second);
        }
    }
    
    return entities;
}

// ============================================================================
// MANAGEMENT
// ============================================================================

void AddressRegistry::unregister(const std::string& address) {
    auto it = registry.find(address);
    if (it != registry.end()) {
        std::string name = it->second.name;
        registry.erase(it);
        save();
        
        LOG_SECURITY(LogLevel::INFO, 
            "Entity unregistered: " + name + " (" + address + ")");
    }
}

void AddressRegistry::clear() {
    registry.clear();
    save();
    LOG_INFO("Address registry cleared");
}

size_t AddressRegistry::size() const {
    return registry.size();
}

// ============================================================================
// PERSISTENCE
// ============================================================================

bool AddressRegistry::save() {
    std::ofstream file(registryFilePath);
    if (!file.is_open()) {
        LOG_ERROR("Failed to save address registry to " + registryFilePath);
        return false;
    }
    
    for (const auto& pair : registry) {
        const EntityInfo& info = pair.second;
        file << info.address << "|"
             << entityTypeToString(info.type) << "|"
             << info.name << "|"
             << info.website << "|"
             << info.verificationSource << "|"
             << info.registeredAt << "|"
             << info.lastVerified << "|"
             << (info.verified ? "1" : "0") << "\n";
    }
    
    file.close();
    return true;
}

bool AddressRegistry::load() {
    std::ifstream file(registryFilePath);
    if (!file.is_open()) {
        LOG_INFO("No existing registry file found, starting fresh");
        return false;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string token;
        EntityInfo info;
        
        // Parse pipe-delimited format
        std::getline(iss, info.address, '|');
        std::getline(iss, token, '|');
        info.type = stringToEntityType(token);
        std::getline(iss, info.name, '|');
        std::getline(iss, info.website, '|');
        std::getline(iss, info.verificationSource, '|');
        std::getline(iss, token, '|');
        info.registeredAt = std::stoull(token);
        std::getline(iss, token, '|');
        info.lastVerified = std::stoull(token);
        std::getline(iss, token, '|');
        info.verified = (token == "1");
        
        registry[info.address] = info;
    }
    
    file.close();
    LOG_INFO("Loaded " + std::to_string(registry.size()) + " entities from registry");
    return true;
}

// ============================================================================
// IMPORT/EXPORT
// ============================================================================

bool AddressRegistry::importFromCSV(const std::string& csvPath) {
    std::ifstream file(csvPath);
    if (!file.is_open()) {
        LOG_ERROR("Failed to open CSV file: " + csvPath);
        return false;
    }
    
    std::string line;
    // Skip header
    std::getline(file, line);
    
    uint32_t imported = 0;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string address, typeStr, name, website, verificationSource;
        
        std::getline(iss, address, ',');
        std::getline(iss, typeStr, ',');
        std::getline(iss, name, ',');
        std::getline(iss, website, ',');
        std::getline(iss, verificationSource, ',');
        
        EntityType type = stringToEntityType(typeStr);
        
        switch (type) {
            case EntityType::EXCHANGE:
                registerExchange(address, name, website, verificationSource);
                break;
            case EntityType::STAKING_POOL:
                registerStakingPool(address, name, website, verificationSource);
                break;
            case EntityType::MERCHANT:
                registerMerchant(address, name, website, verificationSource);
                break;
            case EntityType::VALIDATOR:
                registerValidator(address, name, website, verificationSource);
                break;
            default:
                continue;
        }
        
        imported++;
    }
    
    file.close();
    LOG_INFO("Imported " + std::to_string(imported) + " entities from CSV");
    return true;
}

bool AddressRegistry::exportToCSV(const std::string& csvPath) const {
    std::ofstream file(csvPath);
    if (!file.is_open()) {
        LOG_ERROR("Failed to create CSV file: " + csvPath);
        return false;
    }
    
    // Write header
    file << "Address,Type,Name,Website,VerificationSource,RegisteredAt,LastVerified,Verified\n";
    
    for (const auto& pair : registry) {
        const EntityInfo& info = pair.second;
        file << info.address << ","
             << entityTypeToString(info.type) << ","
             << info.name << ","
             << info.website << ","
             << info.verificationSource << ","
             << info.registeredAt << ","
             << info.lastVerified << ","
             << (info.verified ? "true" : "false") << "\n";
    }
    
    file.close();
    LOG_INFO("Exported " + std::to_string(registry.size()) + " entities to CSV");
    return true;
}

// ============================================================================
// STATISTICS
// ============================================================================

AddressRegistry::RegistryStats AddressRegistry::getStats() const {
    RegistryStats stats;
    stats.totalEntities = registry.size();
    stats.verifiedEntities = 0;
    stats.exchanges = 0;
    stats.stakingPools = 0;
    stats.merchants = 0;
    stats.validators = 0;
    
    for (const auto& pair : registry) {
        if (pair.second.verified) {
            stats.verifiedEntities++;
        }
        
        switch (pair.second.type) {
            case EntityType::EXCHANGE:
                stats.exchanges++;
                break;
            case EntityType::STAKING_POOL:
                stats.stakingPools++;
                break;
            case EntityType::MERCHANT:
                stats.merchants++;
                break;
            case EntityType::VALIDATOR:
                stats.validators++;
                break;
            default:
                break;
        }
    }
    
    return stats;
}
