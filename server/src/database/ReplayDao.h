#pragma once

#include <string>
#include <vector>

struct ReplayInfo {
    int id;
    int room_id;
    std::string title;
    int anchor_id;
    int duration;
    std::string file_path;
    std::string cover_path;
    std::string created_at;
};

class ReplayDao {
public:
    static bool createReplay(int roomId, const std::string& title, int anchorId,
                              int duration, const std::string& filePath, const std::string& coverPath);
    static bool getReplayList(std::vector<ReplayInfo>& outReplays);
    static bool getReplayById(int replayId, ReplayInfo& outReplay);
};
