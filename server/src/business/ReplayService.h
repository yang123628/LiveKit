#pragma once

#include <string>
#include <nlohmann/json.hpp>

class ReplayService {
public:
    static nlohmann::json getReplayList();
    static nlohmann::json getReplayById(int replayId);
};
