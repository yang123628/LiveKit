#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>

struct ViewerInfo {
    int user_id;
    std::string username;
    int avatar_id;
};

class RoomManager {
public:
    static RoomManager& instance();

    bool joinRoom(int roomId, int userId, const std::string& username, int avatarId);
    bool leaveRoom(int roomId, int userId);
    std::vector<ViewerInfo> getViewerList(int roomId);
    int getViewerCount(int roomId);
    void clearRoom(int roomId);

private:
    RoomManager() = default;
    ~RoomManager() = default;
    RoomManager(const RoomManager&) = delete;
    RoomManager& operator=(const RoomManager&) = delete;

    std::mutex m_mutex;
    std::unordered_map<int, std::unordered_map<int, ViewerInfo>> m_rooms;
};
