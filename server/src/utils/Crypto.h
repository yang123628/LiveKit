#pragma once

#include <string>
#include <vector>

class Crypto {
public:
    static std::string sha256(const std::string& input);
    static std::string generateSalt(size_t length = 32);
    static std::string hashPassword(const std::string& password, const std::string& salt);
    static std::string bytesToHex(const unsigned char* data, size_t len);
};
