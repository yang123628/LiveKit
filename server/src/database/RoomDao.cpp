#include "database/RoomDao.h"
#include "database/Database.h"
#include "core/Logger.h"

bool RoomDao::createRoom(int anchorId, const std::string& title,
                          const std::string& category, const std::string& mode,
                          const std::string& streamKey, int& outId) {
    std::string sql = "INSERT INTO rooms (anchor_id, title, category, mode, stream_key) VALUES (?, ?, ?, ?, ?)";
    std::vector<std::string> params = {
        std::to_string(anchorId), title, category, mode, streamKey
    };

    bool ok = Database::instance().executePrepared(sql, params);
    if (!ok) {
        LOG_WARN("createRoom failed for anchor_id=" << anchorId);
        return false;
    }

    Database::instance().query("SELECT last_insert_rowid()", [&](const std::vector<std::string>& row) {
        outId = std::stoi(row[0]);
    });
    LOG_INFO("room created: id=" << outId << " anchor_id=" << anchorId);
    return true;
}

bool RoomDao::findRoomById(int roomId, RoomInfo& outRoom) {
    std::string sql = "SELECT id, anchor_id, title, category, mode, stream_key, status, viewer_count, created_at, ended_at FROM rooms WHERE id = ?";
    std::vector<std::string> params = {std::to_string(roomId)};
    bool found = false;

    Database::instance().queryPrepared(sql, params, [&](const std::vector<std::string>& row) {
        outRoom.id = std::stoi(row[0]);
        outRoom.anchor_id = std::stoi(row[1]);
        outRoom.title = row[2];
        outRoom.category = row[3];
        outRoom.mode = row[4];
        outRoom.stream_key = row[5];
        outRoom.status = row[6];
        outRoom.viewer_count = std::stoi(row[7]);
        outRoom.created_at = row[8];
        outRoom.ended_at = row[9];
        found = true;
    });
    return found;
}

bool RoomDao::findRoomsByStatus(const std::string& status, std::vector<RoomInfo>& outRooms) {
    std::string sql = "SELECT id, anchor_id, title, category, mode, stream_key, status, viewer_count, created_at, ended_at FROM rooms WHERE status = ? ORDER BY id DESC";
    std::vector<std::string> params = {status};

    Database::instance().queryPrepared(sql, params, [&](const std::vector<std::string>& row) {
        RoomInfo room;
        room.id = std::stoi(row[0]);
        room.anchor_id = std::stoi(row[1]);
        room.title = row[2];
        room.category = row[3];
        room.mode = row[4];
        room.stream_key = row[5];
        room.status = row[6];
        room.viewer_count = std::stoi(row[7]);
        room.created_at = row[8];
        room.ended_at = row[9];
        outRooms.push_back(room);
    });
    return true;
}

bool RoomDao::findRoomsByCategory(const std::string& category, const std::string& status,
                                   std::vector<RoomInfo>& outRooms) {
    std::string sql = "SELECT id, anchor_id, title, category, mode, stream_key, status, viewer_count, created_at, ended_at FROM rooms WHERE category = ? AND status = ? ORDER BY id DESC";
    std::vector<std::string> params = {category, status};

    Database::instance().queryPrepared(sql, params, [&](const std::vector<std::string>& row) {
        RoomInfo room;
        room.id = std::stoi(row[0]);
        room.anchor_id = std::stoi(row[1]);
        room.title = row[2];
        room.category = row[3];
        room.mode = row[4];
        room.stream_key = row[5];
        room.status = row[6];
        room.viewer_count = std::stoi(row[7]);
        room.created_at = row[8];
        room.ended_at = row[9];
        outRooms.push_back(room);
    });
    return true;
}

bool RoomDao::updateRoomStatus(int roomId, const std::string& status) {
    std::string sql = "UPDATE rooms SET status = ? WHERE id = ?";
    std::vector<std::string> params = {status, std::to_string(roomId)};
    return Database::instance().executePrepared(sql, params);
}

bool RoomDao::updateViewerCount(int roomId, int count) {
    std::string sql = "UPDATE rooms SET viewer_count = ? WHERE id = ?";
    std::vector<std::string> params = {std::to_string(count), std::to_string(roomId)};
    return Database::instance().executePrepared(sql, params);
}

bool RoomDao::findRoomsByAnchorId(int anchorId, std::vector<RoomInfo>& outRooms) {
    std::string sql = "SELECT id, anchor_id, title, category, mode, stream_key, status, viewer_count, created_at, ended_at FROM rooms WHERE anchor_id = ? ORDER BY id DESC";
    std::vector<std::string> params = {std::to_string(anchorId)};

    Database::instance().queryPrepared(sql, params, [&](const std::vector<std::string>& row) {
        RoomInfo room;
        room.id = std::stoi(row[0]);
        room.anchor_id = std::stoi(row[1]);
        room.title = row[2];
        room.category = row[3];
        room.mode = row[4];
        room.stream_key = row[5];
        room.status = row[6];
        room.viewer_count = std::stoi(row[7]);
        room.created_at = row[8];
        room.ended_at = row[9];
        outRooms.push_back(room);
    });
    return true;
}
