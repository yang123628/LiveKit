#include "core/Logger.h"
#include <iostream>
#include <sys/time.h>

Logger& Logger::instance() {
    static Logger logger;
    return logger;
}

void Logger::setLevel(LogLevel level) {
    m_level = level;
}

void Logger::setLogFile(const std::string& path) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_fileStream.is_open()) {
        m_fileStream.close();
    }
    m_fileStream.open(path, std::ios::app);
}

void Logger::log(LogLevel level, const char* file, int line, const std::string& msg) {
    if (level < m_level) return;

    struct timeval tv;
    gettimeofday(&tv, nullptr);
    struct tm tm;
    localtime_r(&tv.tv_sec, &tm);

    char timeBuf[64];
    strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", &tm);

    std::ostringstream oss;
    oss << "[" << timeBuf << "." << std::setfill('0') << std::setw(3) << (tv.tv_usec / 1000) << "] "
        << "[" << levelToString(level) << "] "
        << "[" << file << ":" << line << "] "
        << msg;

    std::string logLine = oss.str();

    std::lock_guard<std::mutex> lock(m_mutex);
    std::cout << logLine << std::endl;
    if (m_fileStream.is_open()) {
        m_fileStream << logLine << std::endl;
        m_fileStream.flush();
    }
}

std::string Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARN: return "WARN";
        case LogLevel::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}
