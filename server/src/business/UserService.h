#pragma once

#include <string>
#include <nlohmann/json.hpp>

class UserService {
public:
    static nlohmann::json registerUser(const std::string& username, const std::string& password, int avatarId);
    static nlohmann::json loginUser(const std::string& username, const std::string& password);
    static bool verifyToken(const std::string& token, int& outUserId);
};
