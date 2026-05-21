#include "business/RoomManager.h"
#include "network/Connection.h"
#include "network/WebSocketHandler.h"
#include "core/Logger.h"
#include <algorithm>

RoomManager& RoomManager::instance() {
    static RoomManager mgr;
    return mgr;
}

bool RoomManager::joinRoom(int roomId, int userId, const std::string& username, int avatarId, std::shared_ptr<Connection> conn) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto& viewers = m_rooms[roomId];
    if (viewers.find(userId) != viewers.end()) {
        return false;
    }
    WsViewer viewer;
    viewer.user_id = userId;
    viewer.username = username;
    viewer.avatar_id = avatarId;
    viewer.conn = conn;
    viewers[userId] = viewer;
    m_fdToRoomUser[conn->fd()] = {roomId, userId};
    LOG_INFO("ws user " << username << " joined room " << roomId << ", viewers=" << viewers.size());
    return true;
}

void RoomManager::leaveRoom(int roomId, int userId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto roomIt = m_rooms.find(roomId);
    if (roomIt == m_rooms.end()) return;
    auto& viewers = roomIt->second;
    auto it = viewers.find(userId);
    if (it == viewers.end()) return;
    std::string username = it->second.username;
    int fd = -1;
    if (auto c = it->second.conn.lock()) fd = c->fd();
    viewers.erase(it);
    if (fd >= 0) m_fdToRoomUser.erase(fd);
    LOG_INFO("ws user " << username << " left room " << roomId << ", viewers=" << viewers.size());
    if (viewers.empty()) {
        m_rooms.erase(roomIt);
    }
}

void RoomManager::leaveRoomByFd(int fd) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_fdToRoomUser.find(fd);
    if (it == m_fdToRoomUser.end()) return;
    int roomId = it->second.first;
    int userId = it->second.second;
    m_fdToRoomUser.erase(it);

    auto roomIt = m_rooms.find(roomId);
    if (roomIt == m_rooms.end()) return;
    auto& viewers = roomIt->second;
    auto viewerIt = viewers.find(userId);
    if (viewerIt != viewers.end()) {
        std::string username = viewerIt->second.username;
        viewers.erase(viewerIt);
        LOG_INFO("ws user " << username << " left room " << roomId << " (fd disconnect), viewers=" << viewers.size());
    }
    if (viewers.empty()) {
        m_rooms.erase(roomIt);
    }
}

void RoomManager::broadcastToRoom(int roomId, const std::string& message, int excludeFd) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto roomIt = m_rooms.find(roomId);
    if (roomIt == m_rooms.end()) return;
    auto& viewers = roomIt->second;
    for (auto& pair : viewers) {
        if (auto conn = pair.second.conn.lock()) {
            if (conn->fd() != excludeFd) {
                WebSocketHandler::sendText(conn, message);
            }
        }
    }
}

std::vector<WsViewer> RoomManager::getViewerList(int roomId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<WsViewer> result;
    auto it = m_rooms.find(roomId);
    if (it != m_rooms.end()) {
        for (const auto& pair : it->second) {
            result.push_back(pair.second);
        }
    }
    return result;
}

int RoomManager::getViewerCount(int roomId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_rooms.find(roomId);
    if (it != m_rooms.end()) {
        return static_cast<int>(it->second.size());
    }
    return 0;
}

void RoomManager::clearRoom(int roomId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto roomIt = m_rooms.find(roomId);
    if (roomIt != m_rooms.end()) {
        for (auto& pair : roomIt->second) {
            if (auto conn = pair.second.conn.lock()) {
                m_fdToRoomUser.erase(conn->fd());
            }
        }
        m_rooms.erase(roomIt);
    }
}

int RoomManager::getRoomIdByFd(int fd) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_fdToRoomUser.find(fd);
    if (it != m_fdToRoomUser.end()) return it->second.first;
    return -1;
}

int RoomManager::getUserIdByFd(int fd) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_fdToRoomUser.find(fd);
    if (it != m_fdToRoomUser.end()) return it->second.second;
    return -1;
}

void RoomManager::cleanupExpired() {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto roomIt = m_rooms.begin(); roomIt != m_rooms.end();) {
        auto& viewers = roomIt->second;
        for (auto it = viewers.begin(); it != viewers.end();) {
            if (it->second.conn.expired()) {
                m_fdToRoomUser.erase(-1);
                it = viewers.erase(it);
            } else {
                ++it;
            }
        }
        if (viewers.empty()) {
            roomIt = m_rooms.erase(roomIt);
        } else {
            ++roomIt;
        }
    }
}
