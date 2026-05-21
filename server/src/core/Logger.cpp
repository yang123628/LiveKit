#include "core/Logger.h"
#include <iostream>
#include <sys/time.h>
#include <chrono>

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
    m_logBasePath = path;
    auto now = std::chrono::system_clock::now();
    auto timeT = std::chrono::system_clock::to_time_t(now);
    struct tm tm;
    localtime_r(&timeT, &tm);
    char dateBuf[16];
    strftime(dateBuf, sizeof(dateBuf), "%Y-%m-%d", &tm);
    m_currentLogDate = dateBuf;

    size_t dotPos = path.rfind('.');
    std::string rotatedPath;
    if (dotPos != std::string::npos) {
        rotatedPath = path.substr(0, dotPos) + "_" + m_currentLogDate + path.substr(dotPos);
    } else {
        rotatedPath = path + "_" + m_currentLogDate;
    }
    m_fileStream.open(rotatedPath, std::ios::app);
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
    checkLogRotation();
    std::cout << logLine << std::endl;
    if (m_fileStream.is_open()) {
        m_fileStream << logLine << std::endl;
        m_fileStream.flush();
    }
}

void Logger::checkLogRotation() {
    auto now = std::chrono::system_clock::now();
    auto timeT = std::chrono::system_clock::to_time_t(now);
    struct tm tm;
    localtime_r(&timeT, &tm);
    char dateBuf[16];
    strftime(dateBuf, sizeof(dateBuf), "%Y-%m-%d", &tm);
    std::string today(dateBuf);

    if (today != m_currentLogDate) {
        m_currentLogDate = today;
        if (m_fileStream.is_open()) {
            m_fileStream.close();
        }
        size_t dotPos = m_logBasePath.rfind('.');
        std::string rotatedPath;
        if (dotPos != std::string::npos) {
            rotatedPath = m_logBasePath.substr(0, dotPos) + "_" + today + m_logBasePath.substr(dotPos);
        } else {
            rotatedPath = m_logBasePath + "_" + today;
        }
        m_fileStream.open(rotatedPath, std::ios::app);
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
