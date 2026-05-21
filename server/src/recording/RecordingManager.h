#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <mutex>

class FFmpegRecorder;

class RecordingManager {
public:
    static RecordingManager& instance();

    bool startRecording(int roomId, const std::string& streamKey);
    bool stopRecording(int roomId);
    bool isRecording(int roomId) const;
    bool hasRecorder(int roomId) const;
    std::string getOutputPath(int roomId) const;
    void stopAll();

    void takeScreenshot(int roomId, const std::string& streamKey);

private:
    RecordingManager() = default;
    ~RecordingManager() = default;
    RecordingManager(const RecordingManager&) = delete;
    RecordingManager& operator=(const RecordingManager&) = delete;

    std::string generateOutputPath(int roomId);
    std::string generateCoverPath(int roomId);

    mutable std::mutex m_mutex;
    std::unordered_map<int, std::shared_ptr<FFmpegRecorder>> m_recorders;
};
