#include "business/UserService.h"
#include "database/UserDao.h"
#include "utils/Crypto.h"
#include "utils/TokenGenerator.h"
#include "utils/ErrorCode.h"
#include "core/Logger.h"

nlohmann::json UserService::registerUser(const std::string& username, const std::string& password, int avatarId) {
    nlohmann::json result;

    if (username.empty() || password.empty()) {
        result["code"] = ErrorCode::User::EMPTY_PARAMS;
        result["msg"] = "用户名和密码不能为空";
        return result;
    }

    if (username.length() > 32) {
        result["code"] = ErrorCode::User::USERNAME_TOO_LONG;
        result["msg"] = "用户名过长";
        return result;
    }

    if (password.length() < 6) {
        result["code"] = ErrorCode::User::PASSWORD_TOO_SHORT;
        result["msg"] = "密码长度不能少于6位";
        return result;
    }

    UserInfo existing;
    if (UserDao::findUserByUsername(username, existing)) {
        result["code"] = ErrorCode::User::USERNAME_EXISTS;
        result["msg"] = "用户名已存在";
        return result;
    }

    std::string salt = Crypto::generateSalt();
    std::string passwordHash = Crypto::hashPassword(password, salt);

    int userId = 0;
    if (!UserDao::createUser(username, passwordHash, salt, avatarId, userId)) {
        result["code"] = ErrorCode::User::REGISTER_FAILED;
        result["msg"] = "注册失败";
        return result;
    }

    std::string token = TokenGenerator::generate(userId);
    if (!TokenGenerator::storeToken(userId, token)) {
        result["code"] = ErrorCode::User::TOKEN_FAILED;
        result["msg"] = "Token生成失败";
        return result;
    }

    nlohmann::json data;
    data["token"] = token;
    data["user_info"]["id"] = userId;
    data["user_info"]["username"] = username;
    data["user_info"]["avatar_id"] = avatarId;

    result["code"] = ErrorCode::SUCCESS;
    result["msg"] = "注册成功";
    result["data"] = data;
    return result;
}

nlohmann::json UserService::loginUser(const std::string& username, const std::string& password) {
    nlohmann::json result;

    if (username.empty() || password.empty()) {
        result["code"] = ErrorCode::User::EMPTY_PARAMS;
        result["msg"] = "用户名和密码不能为空";
        return result;
    }

    UserInfo user;
    if (!UserDao::findUserByUsername(username, user)) {
        result["code"] = ErrorCode::User::WRONG_PASSWORD;
        result["msg"] = "用户名或密码错误";
        return result;
    }

    std::string inputHash = Crypto::hashPassword(password, user.salt);
    if (inputHash != user.password_hash) {
        result["code"] = ErrorCode::User::WRONG_PASSWORD;
        result["msg"] = "用户名或密码错误";
        return result;
    }

    std::string token = TokenGenerator::generate(user.id);
    if (!TokenGenerator::storeToken(user.id, token)) {
        result["code"] = ErrorCode::User::TOKEN_FAILED;
        result["msg"] = "Token生成失败";
        return result;
    }

    UserDao::updateLastLogin(user.id);

    nlohmann::json data;
    data["token"] = token;
    data["user_info"]["id"] = user.id;
    data["user_info"]["username"] = user.username;
    data["user_info"]["avatar_id"] = user.avatar_id;

    result["code"] = ErrorCode::SUCCESS;
    result["msg"] = "登录成功";
    result["data"] = data;
    return result;
}

UserService::TokenStatus UserService::verifyToken(const std::string& token, int& outUserId) {
    if (token.empty()) return TokenStatus::INVALID;
    int result = TokenGenerator::verifyWithDetail(token, outUserId);
    if (result == 0) return TokenStatus::VALID;
    if (result == 1) return TokenStatus::EXPIRED;
    return TokenStatus::INVALID;
}
