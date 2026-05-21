#pragma once

#include <string>
#include <functional>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <sqlite3.h>

class Database {
public:
    static Database& instance();

    bool open(const std::string& path);
    void close();

    bool execute(const std::string& sql);
    bool executeFile(const std::string& filePath);

    using RowCallback = std::function<void(const std::vector<std::string>&)>;
    bool query(const std::string& sql, const RowCallback& callback);

    bool executePrepared(const std::string& sql, const std::vector<std::string>& params);
    bool queryPrepared(const std::string& sql, const std::vector<std::string>& params, const RowCallback& callback);

    std::string escapeString(const std::string& input);

private:
    Database() = default;
    ~Database();
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    sqlite3* m_db = nullptr;
    std::mutex m_mutex;
};
