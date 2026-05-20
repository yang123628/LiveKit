#pragma once

#include <string>
#include <mutex>
#include <fstream>
#include <sstream>
#include <ctime>
#include <iomanip>

enum class LogLevel { DEBUG, INFO, WARN, ERROR };

class Logger {
public:
    static Logger& instance();
    void setLevel(LogLevel level);
    void setLogFile(const std::string& path);
    void log(LogLevel level, const char* file, int line, const std::string& msg);

private:
    Logger() = default;
    ~Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    LogLevel m_level = LogLevel::INFO;
    std::mutex m_mutex;
    std::ofstream m_fileStream;
    std::string levelToString(LogLevel level);
};

#define LOG_DEBUG(msg) do { std::ostringstream _oss; _oss << msg; Logger::instance().log(LogLevel::DEBUG, __FILE__, __LINE__, _oss.str()); } while(0)
#define LOG_INFO(msg) do { std::ostringstream _oss; _oss << msg; Logger::instance().log(LogLevel::INFO, __FILE__, __LINE__, _oss.str()); } while(0)
#define LOG_WARN(msg) do { std::ostringstream _oss; _oss << msg; Logger::instance().log(LogLevel::WARN, __FILE__, __LINE__, _oss.str()); } while(0)
#define LOG_ERROR(msg) do { std::ostringstream _oss; _oss << msg; Logger::instance().log(LogLevel::ERROR, __FILE__, __LINE__, _oss.str()); } while(0)
