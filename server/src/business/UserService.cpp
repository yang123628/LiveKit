#include "business/UserService.h"
#include "database/UserDao.h"
#include "utils/Crypto.h"
#include "utils/TokenGenerator.h"
#include "core/Logger.h"

nlohmann::json UserService::registerUser(const std::string& username, const std::string& password, int avatarId) {
    nlohmann::json result;

    if (username.empty() || password.empty()) {
        result["code"] = 1001;
        result["msg"] = "用户名和密码不能为空";
        return result;
    }

    if (username.length() > 32) {
        result["code"] = 1002;
        result["msg"] = "用户名过长";
        return result;
    }

    if (password.length() < 6) {
        result["code"] = 1003;
        result["msg"] = "密码长度不能少于6位";
        return result;
    }

    UserInfo existing;
    if (UserDao::findUserByUsername(username, existing)) {
        result["code"] = 1004;
        result["msg"] = "用户名已存在";
        return result;
    }

    std::string salt = Crypto::generateSalt();
    std::string passwordHash = Crypto::hashPassword(password, salt);

    int userId = 0;
    if (!UserDao::createUser(username, passwordHash, salt, avatarId, userId)) {
        result["code"] = 1005;
        result["msg"] = "注册失败";
        return result;
    }

    std::string token = TokenGenerator::generate(userId);
    if (!TokenGenerator::storeToken(userId, token)) {
        result["code"] = 1006;
        result["msg"] = "Token生成失败";
        return result;
    }

    nlohmann::json data;
    data["token"] = token;
    data["user_info"]["id"] = userId;
    data["user_info"]["username"] = username;
    data["user_info"]["avatar_id"] = avatarId;

    result["code"] = 0;
    result["msg"] = "注册成功";
    result["data"] = data;
    return result;
}

nlohmann::json UserService::loginUser(const std::string& username, const std::string& password) {
    nlohmann::json result;

    if (username.empty() || password.empty()) {
        result["code"] = 1001;
        result["msg"] = "用户名和密码不能为空";
        return result;
    }

    UserInfo user;
    if (!UserDao::findUserByUsername(username, user)) {
        result["code"] = 1010;
        result["msg"] = "用户名或密码错误";
        return result;
    }

    std::string inputHash = Crypto::hashPassword(password, user.salt);
    if (inputHash != user.password_hash) {
        result["code"] = 1010;
        result["msg"] = "用户名或密码错误";
        return result;
    }

    std::string token = TokenGenerator::generate(user.id);
    if (!TokenGenerator::storeToken(user.id, token)) {
        result["code"] = 1006;
        result["msg"] = "Token生成失败";
        return result;
    }

    UserDao::updateLastLogin(user.id);

    nlohmann::json data;
    data["token"] = token;
    data["user_info"]["id"] = user.id;
    data["user_info"]["username"] = user.username;
    data["user_info"]["avatar_id"] = user.avatar_id;

    result["code"] = 0;
    result["msg"] = "登录成功";
    result["data"] = data;
    return result;
}

bool UserService::verifyToken(const std::string& token, int& outUserId) {
    if (token.empty()) return false;
    return TokenGenerator::verify(token, outUserId);
}
