#pragma once

#include <cstdint>
#include <string>
#include <vector>

/**
 * 256-bit unsigned integer for chainwork calculations
 * Based on Bitcoin's arith_uint256 implementation
 */
class arith_uint256 {
private:
    static constexpr int WIDTH = 256 / 32;
    uint32_t pn[WIDTH];

public:
    arith_uint256();
    arith_uint256(uint64_t b);
    arith_uint256(const std::string& str);
    
    // Comparison operators
    bool operator==(const arith_uint256& b) const;
    bool operator!=(const arith_uint256& b) const;
    bool operator<(const arith_uint256& b) const;
    bool operator<=(const arith_uint256& b) const;
    bool operator>(const arith_uint256& b) const;
    bool operator>=(const arith_uint256& b) const;
    
    // Arithmetic operators
    arith_uint256 operator+(const arith_uint256& b) const;
    arith_uint256 operator-(const arith_uint256& b) const;
    arith_uint256 operator*(const arith_uint256& b) const;
    arith_uint256 operator/(const arith_uint256& b) const;
    arith_uint256 operator~() const;
    
    arith_uint256& operator+=(const arith_uint256& b);
    arith_uint256& operator-=(const arith_uint256& b);
    arith_uint256& operator++();
    
    // Conversion
    std::string GetHex() const;
    void SetHex(const std::string& str);
    void SetCompact(uint32_t nCompact);
    uint32_t GetCompact() const;
    uint64_t GetLow64() const;
    
    // Utility
    bool IsZero() const;
    int bits() const;
};

// Helper function to calculate block proof (work)
arith_uint256 GetBlockProof(uint32_t nBits);
arith_uint256 GetBlockProof(double difficulty);
