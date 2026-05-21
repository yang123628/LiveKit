#include "utils/TokenGenerator.h"
#include "utils/Crypto.h"
#include "database/Database.h"
#include "core/Logger.h"
#include <sstream>
#include <ctime>
#include <cstring>

std::string TokenGenerator::generate(int userId) {
    std::ostringstream oss;
    oss << userId << "_" << time(nullptr) << "_" << Crypto::generateSalt(16);
    return Crypto::sha256(oss.str());
}

bool TokenGenerator::verify(const std::string& token, int& outUserId) {
    if (isTokenExpired(token)) return false;

    std::string sql = "SELECT user_id FROM tokens WHERE token = ?";
    std::vector<std::string> params = {token};
    bool found = false;

    Database::instance().queryPrepared(sql, params, [&](const std::vector<std::string>& row) {
        outUserId = std::stoi(row[0]);
        found = true;
    });
    return found;
}

bool TokenGenerator::storeToken(int userId, const std::string& token) {
    std::string sql = "INSERT INTO tokens (user_id, token, expires_at) VALUES (?, ?, datetime('now', 'localtime', '+24 hours'))";
    std::vector<std::string> params = {std::to_string(userId), token};
    bool ok = Database::instance().executePrepared(sql, params);
    if (ok) {
        LOG_INFO("token stored for user_id=" << userId);
    }
    return ok;
}

bool TokenGenerator::isTokenExpired(const std::string& token) {
    std::string sql = "SELECT expires_at FROM tokens WHERE token = ?";
    std::vector<std::string> params = {token};
    bool expired = true;

    Database::instance().queryPrepared(sql, params, [&](const std::vector<std::string>& row) {
        std::string expiresAt = row[0];
        time_t now = time(nullptr);
        struct tm tm;
        memset(&tm, 0, sizeof(tm));
        strptime(expiresAt.c_str(), "%Y-%m-%d %H:%M:%S", &tm);
        time_t expiresTime = mktime(&tm);
        expired = (now > expiresTime);
    });
    return expired;
}

bool TokenGenerator::removeToken(const std::string& token) {
    std::string sql = "DELETE FROM tokens WHERE token = ?";
    std::vector<std::string> params = {token};
    return Database::instance().executePrepared(sql, params);
}

int TokenGenerator::verifyWithDetail(const std::string& token, int& outUserId) {
    if (token.empty()) return 2;

    std::string sql = "SELECT user_id, expires_at FROM tokens WHERE token = ?";
    std::vector<std::string> params = {token};
    bool found = false;
    std::string expiresAt;

    Database::instance().queryPrepared(sql, params, [&](const std::vector<std::string>& row) {
        outUserId = std::stoi(row[0]);
        expiresAt = row[1];
        found = true;
    });

    if (!found) return 2;

    time_t now = time(nullptr);
    struct tm tm;
    memset(&tm, 0, sizeof(tm));
    strptime(expiresAt.c_str(), "%Y-%m-%d %H:%M:%S", &tm);
    time_t expiresTime = mktime(&tm);

    if (now > expiresTime) return 1;

    return 0;
}

bool TokenGenerator::cleanExpiredTokens() {
    std::string sql = "DELETE FROM tokens WHERE expires_at < datetime('now', 'localtime')";
    bool ok = Database::instance().execute(sql);
    if (ok) {
        LOG_INFO("expired tokens cleaned");
    }
    return ok;
}
