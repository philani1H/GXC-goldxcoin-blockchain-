#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <ctime>
#include <fstream>
#include <sstream>

/**
 * Address Registry System
 * 
 * Maintains a persistent registry of known entities (exchanges, staking pools,
 * merchants, validators) for fraud detection clean zone identification.
 * 
 * This is NOT a heuristic - it's a real registry that can be populated with
 * verified addresses from known entities.
 */

class AddressRegistry {
public:
    enum class EntityType {
        EXCHANGE,
        STAKING_POOL,
        MERCHANT,
        VALIDATOR,
        UNKNOWN
    };
    
    struct EntityInfo {
        EntityType type;
        std::string name;
        std::string address;
        std::string website;
        std::string verificationSource;
        uint64_t registeredAt;
        uint64_t lastVerified;
        bool verified;
        
        EntityInfo() : type(EntityType::UNKNOWN), registeredAt(0), 
                      lastVerified(0), verified(false) {}
    };
    
private:
    std::unordered_map<std::string, EntityInfo> registry;
    std::string registryFilePath;
    
    std::string entityTypeToString(EntityType type) const;
    EntityType stringToEntityType(const std::string& str) const;
    
public:
    AddressRegistry(const std::string& filePath = "data/address_registry.dat");
    ~AddressRegistry();
    
    // Registration methods
    void registerExchange(const std::string& address, const std::string& name, 
                         const std::string& website = "", 
                         const std::string& verificationSource = "");
    void registerStakingPool(const std::string& address, const std::string& name,
                            const std::string& website = "",
                            const std::string& verificationSource = "");
    void registerMerchant(const std::string& address, const std::string& name,
                         const std::string& website = "",
                         const std::string& verificationSource = "");
    void registerValidator(const std::string& address, const std::string& name,
                          const std::string& website = "",
                          const std::string& verificationSource = "");
    
    // Verification
    void markAsVerified(const std::string& address, const std::string& verificationSource);
    void updateLastVerified(const std::string& address);
    
    // Query methods
    bool isRegistered(const std::string& address) const;
    bool isVerified(const std::string& address) const;
    EntityType getEntityType(const std::string& address) const;
    EntityInfo getEntityInfo(const std::string& address) const;
    std::string getEntityName(const std::string& address) const;
    
    // Bulk operations
    std::vector<std::string> getAllAddresses() const;
    std::vector<std::string> getAddressesByType(EntityType type) const;
    std::vector<EntityInfo> getAllEntities() const;
    std::vector<EntityInfo> getVerifiedEntities() const;
    
    // Management
    void unregister(const std::string& address);
    void clear();
    size_t size() const;
    
    // Persistence
    bool save();
    bool load();
    
    // Import/Export
    bool importFromCSV(const std::string& csvPath);
    bool exportToCSV(const std::string& csvPath) const;
    
    // Statistics
    struct RegistryStats {
        uint32_t totalEntities;
        uint32_t verifiedEntities;
        uint32_t exchanges;
        uint32_t stakingPools;
        uint32_t merchants;
        uint32_t validators;
    };
    
    RegistryStats getStats() const;
};
