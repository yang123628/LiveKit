#include "recording/FFmpegRecorder.h"
#include "utils/Config.h"
#include "core/Logger.h"
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <cstring>
#include <cerrno>

FFmpegRecorder::FFmpegRecorder()
    : m_childPid(-1), m_stdinFd(-1) {
}

FFmpegRecorder::~FFmpegRecorder() {
    stopRecording();
}

bool FFmpegRecorder::startRecording(const std::string& streamKey, const std::string& outputPath) {
    if (m_childPid > 0) {
        LOG_WARN("recording already in progress");
        return false;
    }

    std::string ffmpegPath = Config::instance().get("recording", "ffmpeg_path", "/usr/bin/ffmpeg");
    std::string rtmpHost = Config::instance().get("nginx", "rtmp_host", "127.0.0.1");
    std::string rtmpPort = Config::instance().get("nginx", "rtmp_port", "1935");
    std::string inputUrl = "rtmp://" + rtmpHost + "/live/" + streamKey;

    int pipeFd[2];
    if (pipe(pipeFd) < 0) {
        LOG_ERROR("pipe() failed: " << strerror(errno));
        return false;
    }

    pid_t pid = fork();
    if (pid < 0) {
        LOG_ERROR("fork() failed: " << strerror(errno));
        close(pipeFd[0]);
        close(pipeFd[1]);
        return false;
    }

    if (pid == 0) {
        close(pipeFd[1]);
        dup2(pipeFd[0], STDIN_FILENO);
        close(pipeFd[0]);

        int devNull = open("/dev/null", O_WRONLY);
        if (devNull >= 0) {
            dup2(devNull, STDOUT_FILENO);
            dup2(devNull, STDERR_FILENO);
            close(devNull);
        }

        execl(ffmpegPath.c_str(), "ffmpeg",
              "-i", inputUrl.c_str(),
              "-c", "copy",
              "-f", "mp4",
              "-y",
              outputPath.c_str(),
              nullptr);

        _exit(1);
    }

    close(pipeFd[0]);
    m_stdinFd = pipeFd[1];
    m_childPid = pid;
    m_outputPath = outputPath;
    m_startTime = std::chrono::steady_clock::now();

    LOG_INFO("recording started: pid=" << pid << " output=" << outputPath);
    return true;
}

bool FFmpegRecorder::stopRecording() {
    if (m_childPid <= 0) return false;

    int status = 0;
    pid_t ret = waitpid(m_childPid, &status, WNOHANG);
    if (ret == 0) {
        if (m_stdinFd >= 0) {
            const char q = 'q';
            write(m_stdinFd, &q, 1);
            close(m_stdinFd);
            m_stdinFd = -1;
        }

        for (int i = 0; i < 10; ++i) {
            usleep(200000);
            ret = waitpid(m_childPid, &status, WNOHANG);
            if (ret != 0) break;
        }

        if (ret == 0) {
            kill(m_childPid, SIGTERM);
            for (int i = 0; i < 10; ++i) {
                usleep(200000);
                ret = waitpid(m_childPid, &status, WNOHANG);
                if (ret != 0) break;
            }
            if (ret == 0) {
                kill(m_childPid, SIGKILL);
                waitpid(m_childPid, &status, 0);
            }
        }
    } else {
        if (m_stdinFd >= 0) {
            close(m_stdinFd);
            m_stdinFd = -1;
        }
    }

    LOG_INFO("recording stopped: pid=" << m_childPid);
    m_childPid = -1;
    return true;
}

bool FFmpegRecorder::isRecording() const {
    if (m_childPid <= 0) return false;
    int status = 0;
    pid_t ret = waitpid(m_childPid, &status, WNOHANG);
    if (ret == m_childPid) {
        return false;
    }
    if (ret < 0 && errno == ECHILD) {
        return false;
    }
    return true;
}

int FFmpegRecorder::childPid() const {
    return m_childPid;
}

std::string FFmpegRecorder::outputPath() const {
    return m_outputPath;
}

std::chrono::steady_clock::time_point FFmpegRecorder::startTime() const {
    return m_startTime;
}
