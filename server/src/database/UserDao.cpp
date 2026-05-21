#include "database/UserDao.h"
#include "database/Database.h"
#include "core/Logger.h"

bool UserDao::createUser(const std::string& username, const std::string& passwordHash,
                         const std::string& salt, int avatarId, int& outId) {
    std::string sql = "INSERT INTO users (username, password_hash, salt, avatar_id) VALUES (?, ?, ?, ?)";
    std::vector<std::string> params = {username, passwordHash, salt, std::to_string(avatarId)};

    bool ok = Database::instance().executePrepared(sql, params);
    if (!ok) {
        LOG_WARN("createUser failed for username=" << username);
        return false;
    }

    Database::instance().query("SELECT last_insert_rowid()", [&](const std::vector<std::string>& row) {
        outId = std::stoi(row[0]);
    });
    LOG_INFO("user created: id=" << outId << " username=" << username);
    return true;
}

bool UserDao::findUserByUsername(const std::string& username, UserInfo& outUser) {
    std::string sql = "SELECT id, username, password_hash, salt, avatar_id, created_at, last_login FROM users WHERE username = ?";
    std::vector<std::string> params = {username};
    bool found = false;

    Database::instance().queryPrepared(sql, params, [&](const std::vector<std::string>& row) {
        outUser.id = std::stoi(row[0]);
        outUser.username = row[1];
        outUser.password_hash = row[2];
        outUser.salt = row[3];
        outUser.avatar_id = std::stoi(row[4]);
        outUser.created_at = row[5];
        outUser.last_login = row[6];
        found = true;
    });
    return found;
}

bool UserDao::findUserById(int id, UserInfo& outUser) {
    std::string sql = "SELECT id, username, password_hash, salt, avatar_id, created_at, last_login FROM users WHERE id = ?";
    std::vector<std::string> params = {std::to_string(id)};
    bool found = false;

    Database::instance().queryPrepared(sql, params, [&](const std::vector<std::string>& row) {
        outUser.id = std::stoi(row[0]);
        outUser.username = row[1];
        outUser.password_hash = row[2];
        outUser.salt = row[3];
        outUser.avatar_id = std::stoi(row[4]);
        outUser.created_at = row[5];
        outUser.last_login = row[6];
        found = true;
    });
    return found;
}

bool UserDao::updateLastLogin(int id) {
    std::string sql = "UPDATE users SET last_login = datetime('now', 'localtime') WHERE id = ?";
    std::vector<std::string> params = {std::to_string(id)};
    return Database::instance().executePrepared(sql, params);
}
