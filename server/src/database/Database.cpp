#include "database/Database.h"
#include "core/Logger.h"
#include <fstream>
#include <sstream>

Database& Database::instance() {
    static Database db;
    return db;
}

Database::~Database() {
    close();
}

bool Database::open(const std::string& path) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_db) {
        sqlite3_close(m_db);
        m_db = nullptr;
    }
    int rc = sqlite3_open(path.c_str(), &m_db);
    if (rc != SQLITE_OK) {
        LOG_ERROR("cannot open database: " << sqlite3_errmsg(m_db));
        sqlite3_close(m_db);
        m_db = nullptr;
        return false;
    }
    sqlite3_exec(m_db, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);
    sqlite3_exec(m_db, "PRAGMA foreign_keys=ON;", nullptr, nullptr, nullptr);
    sqlite3_exec(m_db, "PRAGMA busy_timeout=5000;", nullptr, nullptr, nullptr);
    LOG_INFO("database opened: " << path);
    return true;
}

void Database::close() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_db) {
        sqlite3_close(m_db);
        m_db = nullptr;
        LOG_INFO("database closed");
    }
}

bool Database::execute(const std::string& sql) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_db) return false;
    char* errMsg = nullptr;
    int rc = sqlite3_exec(m_db, sql.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        LOG_ERROR("SQL error: " << (errMsg ? errMsg : "unknown"));
        if (errMsg) sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool Database::executeFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        LOG_ERROR("cannot open SQL file: " << filePath);
        return false;
    }
    std::stringstream ss;
    ss << file.rdbuf();
    std::string sql = ss.str();
    LOG_INFO("executing SQL file: " << filePath << " size=" << sql.size());
    bool result = execute(sql);
    if (result) {
        LOG_INFO("SQL file executed successfully: " << filePath);
    }
    return result;
}

bool Database::query(const std::string& sql, const RowCallback& callback) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_db) return false;

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR("prepare failed: " << sqlite3_errmsg(m_db));
        return false;
    }

    int colCount = sqlite3_column_count(stmt);
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        std::vector<std::string> row;
        row.reserve(colCount);
        for (int i = 0; i < colCount; ++i) {
            const char* val = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
            row.push_back(val ? val : "");
        }
        if (callback) callback(row);
    }

    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        LOG_ERROR("query step error: " << sqlite3_errmsg(m_db));
        return false;
    }
    return true;
}

bool Database::executePrepared(const std::string& sql, const std::vector<std::string>& params) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_db) return false;

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR("prepare failed: " << sqlite3_errmsg(m_db));
        return false;
    }

    for (size_t i = 0; i < params.size(); ++i) {
        sqlite3_bind_text(stmt, static_cast<int>(i + 1), params[i].c_str(), -1, SQLITE_TRANSIENT);
    }

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE && rc != SQLITE_ROW) {
        LOG_ERROR("execute prepared error: " << sqlite3_errmsg(m_db) << " sql=" << sql);
        return false;
    }
    return true;
}

bool Database::queryPrepared(const std::string& sql, const std::vector<std::string>& params, const RowCallback& callback) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_db) return false;

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR("prepare failed: " << sqlite3_errmsg(m_db));
        return false;
    }

    for (size_t i = 0; i < params.size(); ++i) {
        sqlite3_bind_text(stmt, static_cast<int>(i + 1), params[i].c_str(), -1, SQLITE_TRANSIENT);
    }

    int colCount = sqlite3_column_count(stmt);
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        std::vector<std::string> row;
        row.reserve(colCount);
        for (int i = 0; i < colCount; ++i) {
            const char* val = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
            row.push_back(val ? val : "");
        }
        if (callback) callback(row);
    }

    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        LOG_ERROR("query prepared step error: " << sqlite3_errmsg(m_db));
        return false;
    }
    return true;
}

std::string Database::escapeString(const std::string& input) {
    std::string result;
    result.reserve(input.size());
    for (char c : input) {
        if (c == '\'') result += "''";
        else result += c;
    }
    return result;
}
