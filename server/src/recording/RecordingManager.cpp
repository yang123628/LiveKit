#include "recording/RecordingManager.h"
#include "recording/FFmpegRecorder.h"
#include "utils/Config.h"
#include "core/Logger.h"
#include <chrono>
#include <ctime>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

RecordingManager& RecordingManager::instance() {
    static RecordingManager mgr;
    return mgr;
}

std::string RecordingManager::generateOutputPath(int roomId) {
    auto now = std::chrono::system_clock::now();
    auto timeT = std::chrono::system_clock::to_time_t(now);
    char timeBuf[32];
    std::strftime(timeBuf, sizeof(timeBuf), "%Y%m%d%H%M%S", std::localtime(&timeT));

    std::string recPath = Config::instance().get("recording", "path", "./static/recordings");
    mkdir(recPath.c_str(), 0755);

    return recPath + "/" + std::to_string(roomId) + "_" + timeBuf + ".mp4";
}

std::string RecordingManager::generateCoverPath(int roomId) {
    return "static/covers/" + std::to_string(roomId) + ".jpg";
}

bool RecordingManager::startRecording(int roomId, const std::string& streamKey) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_recorders.find(roomId);
    if (it != m_recorders.end() && it->second->isRecording()) {
        LOG_WARN("room " << roomId << " already recording");
        return false;
    }

    auto recorder = std::make_shared<FFmpegRecorder>();
    std::string outputPath = generateOutputPath(roomId);

    if (!recorder->startRecording(streamKey, outputPath)) {
        LOG_ERROR("failed to start recording for room " << roomId);
        return false;
    }

    m_recorders[roomId] = recorder;
    LOG_INFO("recording started for room " << roomId);
    return true;
}

bool RecordingManager::stopRecording(int roomId) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_recorders.find(roomId);
    if (it == m_recorders.end()) {
        LOG_WARN("no recording found for room " << roomId);
        return false;
    }

    bool ok = it->second->stopRecording();
    m_recorders.erase(it);
    LOG_INFO("recording stopped for room " << roomId);
    return ok;
}

bool RecordingManager::isRecording(int roomId) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_recorders.find(roomId);
    if (it == m_recorders.end()) return false;
    return it->second->isRecording();
}

bool RecordingManager::hasRecorder(int roomId) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_recorders.find(roomId) != m_recorders.end();
}

std::string RecordingManager::getOutputPath(int roomId) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_recorders.find(roomId);
    if (it == m_recorders.end()) return "";
    return it->second->outputPath();
}

void RecordingManager::stopAll() {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& pair : m_recorders) {
        pair.second->stopRecording();
    }
    m_recorders.clear();
}

void RecordingManager::takeScreenshot(int roomId, const std::string& streamKey) {
    std::string ffmpegPath = Config::instance().get("recording", "ffmpeg_path", "/usr/bin/ffmpeg");
    std::string rtmpHost = Config::instance().get("nginx", "rtmp_host", "127.0.0.1");
    std::string inputUrl = "rtmp://" + rtmpHost + "/live/" + streamKey;
    std::string coverPath = generateCoverPath(roomId);

    mkdir("static/covers", 0755);

    pid_t pid = fork();
    if (pid == 0) {
        int devNull = open("/dev/null", O_WRONLY);
        if (devNull >= 0) {
            dup2(devNull, STDOUT_FILENO);
            dup2(devNull, STDERR_FILENO);
            close(devNull);
        }

        execl(ffmpegPath.c_str(), "ffmpeg",
              "-i", inputUrl.c_str(),
              "-frames:v", "1",
              "-y",
              coverPath.c_str(),
              nullptr);

        _exit(1);
    } else if (pid > 0) {
        LOG_INFO("screenshot started for room " << roomId << " pid=" << pid);
    }
}
