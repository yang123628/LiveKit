#include "business/RoomManager.h"
#include "core/Logger.h"

RoomManager& RoomManager::instance() {
    static RoomManager mgr;
    return mgr;
}

bool RoomManager::joinRoom(int roomId, int userId, const std::string& username, int avatarId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto& viewers = m_rooms[roomId];
    if (viewers.find(userId) != viewers.end()) {
        return false;
    }
    ViewerInfo info;
    info.user_id = userId;
    info.username = username;
    info.avatar_id = avatarId;
    viewers[userId] = info;
    LOG_INFO("user " << username << " joined room " << roomId << ", viewers=" << viewers.size());
    return true;
}

bool RoomManager::leaveRoom(int roomId, int userId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto roomIt = m_rooms.find(roomId);
    if (roomIt == m_rooms.end()) return false;
    auto& viewers = roomIt->second;
    auto it = viewers.find(userId);
    if (it == viewers.end()) return false;
    std::string username = it->second.username;
    viewers.erase(it);
    LOG_INFO("user " << username << " left room " << roomId << ", viewers=" << viewers.size());
    if (viewers.empty()) {
        m_rooms.erase(roomIt);
    }
    return true;
}

std::vector<ViewerInfo> RoomManager::getViewerList(int roomId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<ViewerInfo> result;
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
    m_rooms.erase(roomId);
}
