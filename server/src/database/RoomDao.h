#pragma once

#include <string>
#include <vector>

struct RoomInfo {
    int id;
    int anchor_id;
    std::string title;
    std::string category;
    std::string mode;
    std::string stream_key;
    std::string status;
    int viewer_count;
    int like_count;
    std::string created_at;
    std::string ended_at;
};

class RoomDao {
public:
    static bool createRoom(int anchorId, const std::string& title,
                           const std::string& category, const std::string& mode,
                           const std::string& streamKey, int& outId);
    static bool findRoomById(int roomId, RoomInfo& outRoom);
    static bool findRoomsByStatus(const std::string& status, std::vector<RoomInfo>& outRooms);
    static bool findRoomsByCategory(const std::string& category, const std::string& status,
                                     std::vector<RoomInfo>& outRooms);
    static bool updateRoomStatus(int roomId, const std::string& status);
    static bool updateViewerCount(int roomId, int count);
    static bool updateLikeCount(int roomId, int count);
    static bool findRoomsByAnchorId(int anchorId, std::vector<RoomInfo>& outRooms);
};
