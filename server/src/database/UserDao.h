#pragma once

#include <string>
#include <vector>

struct UserInfo {
    int id;
    std::string username;
    std::string password_hash;
    std::string salt;
    int avatar_id;
    std::string created_at;
    std::string last_login;
};

class UserDao {
public:
    static bool createUser(const std::string& username, const std::string& passwordHash,
                           const std::string& salt, int avatarId, int& outId);
    static bool findUserByUsername(const std::string& username, UserInfo& outUser);
    static bool findUserById(int id, UserInfo& outUser);
    static bool updateLastLogin(int id);
};
