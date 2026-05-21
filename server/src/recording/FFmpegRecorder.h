#pragma once

#include <string>
#include <chrono>

class FFmpegRecorder {
public:
    FFmpegRecorder();
    ~FFmpegRecorder();

    bool startRecording(const std::string& streamKey, const std::string& outputPath);
    bool stopRecording();
    bool isRecording() const;
    int childPid() const;
    std::string outputPath() const;
    std::chrono::steady_clock::time_point startTime() const;

private:
    pid_t m_childPid;
    int m_stdinFd;
    std::string m_outputPath;
    std::chrono::steady_clock::time_point m_startTime;
};
