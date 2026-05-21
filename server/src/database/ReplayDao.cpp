#include "database/ReplayDao.h"
#include "database/Database.h"
#include "core/Logger.h"

bool ReplayDao::createReplay(int roomId, const std::string& title, int anchorId,
                               int duration, const std::string& filePath, const std::string& coverPath) {
    std::string sql = "INSERT INTO replays (room_id, title, anchor_id, duration, file_path, cover_path) VALUES (?, ?, ?, ?, ?, ?)";
    std::vector<std::string> params = {
        std::to_string(roomId), title, std::to_string(anchorId),
        std::to_string(duration), filePath, coverPath
    };

    bool ok = Database::instance().executePrepared(sql, params);
    if (ok) {
        LOG_INFO("replay created: room_id=" << roomId << " duration=" << duration);
    } else {
        LOG_WARN("createReplay failed for room_id=" << roomId);
    }
    return ok;
}

bool ReplayDao::getReplayList(std::vector<ReplayInfo>& outReplays) {
    std::string sql = "SELECT id, room_id, title, anchor_id, duration, file_path, cover_path, created_at FROM replays ORDER BY id DESC";
    Database::instance().query(sql, [&](const std::vector<std::string>& row) {
        ReplayInfo replay;
        replay.id = std::stoi(row[0]);
        replay.room_id = std::stoi(row[1]);
        replay.title = row[2];
        replay.anchor_id = std::stoi(row[3]);
        replay.duration = std::stoi(row[4]);
        replay.file_path = row[5];
        replay.cover_path = row[6];
        replay.created_at = row[7];
        outReplays.push_back(replay);
    });
    return true;
}

bool ReplayDao::getReplayById(int replayId, ReplayInfo& outReplay) {
    std::string sql = "SELECT id, room_id, title, anchor_id, duration, file_path, cover_path, created_at FROM replays WHERE id = ?";
    std::vector<std::string> params = {std::to_string(replayId)};
    bool found = false;
    Database::instance().queryPrepared(sql, params, [&](const std::vector<std::string>& row) {
        outReplay.id = std::stoi(row[0]);
        outReplay.room_id = std::stoi(row[1]);
        outReplay.title = row[2];
        outReplay.anchor_id = std::stoi(row[3]);
        outReplay.duration = std::stoi(row[4]);
        outReplay.file_path = row[5];
        outReplay.cover_path = row[6];
        outReplay.created_at = row[7];
        found = true;
    });
    return found;
}
