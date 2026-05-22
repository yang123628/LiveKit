#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <memory>
#include <functional>

class Connection;

struct WsViewer {
    int user_id;
    std::string username;
    int avatar_id;
    std::weak_ptr<Connection> conn;
    bool has_ws_conn = false;
};

class RoomManager {
public:
    static RoomManager& instance();

    bool joinRoom(int roomId, int userId, const std::string& username, int avatarId, std::shared_ptr<Connection> conn);
    void leaveRoom(int roomId, int userId);
    void leaveRoomByFd(int fd);
    void broadcastToRoom(int roomId, const std::string& message, int excludeFd = -1);
    std::vector<WsViewer> getViewerList(int roomId);
    int getViewerCount(int roomId);
    void clearRoom(int roomId);
    int getRoomIdByFd(int fd);
    int getUserIdByFd(int fd);

    void cleanupExpired();

    void forEachConnection(std::function<void(std::shared_ptr<Connection>)> callback);

private:
    RoomManager() = default;
    ~RoomManager() = default;
    RoomManager(const RoomManager&) = delete;
    RoomManager& operator=(const RoomManager&) = delete;

    std::mutex m_mutex;
    std::unordered_map<int, std::unordered_map<int, WsViewer>> m_rooms;
    std::unordered_map<int, std::pair<int, int>> m_fdToRoomUser;
};
