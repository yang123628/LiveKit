#pragma once

#include <string>
#include <nlohmann/json.hpp>

class UserService {
public:
    enum class TokenStatus { VALID = 0, EXPIRED = 1, INVALID = 2 };

    static nlohmann::json registerUser(const std::string& username, const std::string& password, int avatarId);
    static nlohmann::json loginUser(const std::string& username, const std::string& password);
    static TokenStatus verifyToken(const std::string& token, int& outUserId);
};
