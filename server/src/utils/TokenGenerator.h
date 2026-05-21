#pragma once

#include <string>

class TokenGenerator {
public:
    static std::string generate(int userId);
    static bool verify(const std::string& token, int& outUserId);
    static bool storeToken(int userId, const std::string& token);
    static bool isTokenExpired(const std::string& token);
    static bool removeToken(const std::string& token);
};
