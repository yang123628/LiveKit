#include "business/RoomService.h"
#include "business/RoomManager.h"
#include "database/RoomDao.h"
#include "database/UserDao.h"
#include "business/UserService.h"
#include "utils/Crypto.h"
#include "utils/Config.h"
#include "core/Logger.h"

static std::string generateStreamKey() {
    std::string chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::string key;
    unsigned char buf[16];
    FILE* f = fopen("/dev/urandom", "rb");
    if (f) {
        fread(buf, 1, 16, f);
        fclose(f);
    } else {
        for (int i = 0; i < 16; ++i) buf[i] = static_cast<unsigned char>(rand());
    }
    for (int i = 0; i < 16; ++i) {
        key += chars[buf[i] % chars.size()];
    }
    return key;
}

static std::string getRtmpBaseUrl() {
    std::string serverIp = Config::instance().get("nginx", "server_ip", "127.0.0.1");
    return "rtmp://" + serverIp + "/live/";
}

nlohmann::json RoomService::createRoom(const std::string& token, const std::string& title,
                                         const std::string& category, const std::string& mode) {
    nlohmann::json result;

    int userId = 0;
    if (!UserService::verifyToken(token, userId)) {
        result["code"] = 2001;
        result["msg"] = "无效的token";
        return result;
    }

    if (title.empty()) {
        result["code"] = 2002;
        result["msg"] = "直播标题不能为空";
        return result;
    }

    std::vector<RoomInfo> existingRooms;
    RoomDao::findRoomsByAnchorId(userId, existingRooms);
    for (const auto& r : existingRooms) {
        if (r.status == "live") {
            result["code"] = 2003;
            result["msg"] = "已有正在直播的房间";
            return result;
        }
    }

    std::string streamKey = generateStreamKey();

    int roomId = 0;
    if (!RoomDao::createRoom(userId, title, category, mode, streamKey, roomId)) {
        result["code"] = 2004;
        result["msg"] = "创建直播间失败";
        return result;
    }

    std::string pushUrl = getRtmpBaseUrl() + streamKey;

    nlohmann::json data;
    data["room_id"] = roomId;
    data["stream_key"] = streamKey;
    data["push_url"] = pushUrl;

    result["code"] = 0;
    result["msg"] = "创建成功";
    result["data"] = data;
    return result;
}

nlohmann::json RoomService::getRoomList(const std::string& category) {
    nlohmann::json result;
    std::vector<RoomInfo> rooms;

    if (category == "all" || category.empty()) {
        RoomDao::findRoomsByStatus("live", rooms);
    } else {
        RoomDao::findRoomsByCategory(category, "live", rooms);
    }

    nlohmann::json roomList = nlohmann::json::array();
    for (const auto& room : rooms) {
        UserInfo anchor;
        if (!UserDao::findUserById(room.anchor_id, anchor)) continue;

        nlohmann::json item;
        item["room_id"] = room.id;
        item["title"] = room.title;
        item["anchor_name"] = anchor.username;
        item["anchor_avatar_id"] = anchor.avatar_id;
        item["viewer_count"] = RoomManager::instance().getViewerCount(room.id);
        item["category"] = room.category;
        item["mode"] = room.mode;
        item["status"] = room.status;

        std::string host = Config::instance().get("server", "host", "127.0.0.1");
        int port = Config::instance().getInt("server", "port", 9090);
        item["cover_url"] = "http://" + host + ":" + std::to_string(port) + "/covers/" + std::to_string(room.id) + ".jpg";

        roomList.push_back(item);
    }

    nlohmann::json data;
    data["rooms"] = roomList;

    result["code"] = 0;
    result["msg"] = "success";
    result["data"] = data;
    return result;
}

nlohmann::json RoomService::getRoomInfo(int roomId) {
    nlohmann::json result;

    RoomInfo room;
    if (!RoomDao::findRoomById(roomId, room)) {
        result["code"] = 2005;
        result["msg"] = "直播间不存在";
        return result;
    }

    UserInfo anchor;
    UserDao::findUserById(room.anchor_id, anchor);

    nlohmann::json roomJson;
    roomJson["room_id"] = room.id;
    roomJson["title"] = room.title;
    roomJson["anchor_name"] = anchor.username;
    roomJson["anchor_avatar_id"] = anchor.avatar_id;
    roomJson["viewer_count"] = RoomManager::instance().getViewerCount(room.id);
    roomJson["category"] = room.category;
    roomJson["mode"] = room.mode;
    roomJson["status"] = room.status;
    roomJson["stream_key"] = room.stream_key;
    roomJson["play_url"] = getRtmpBaseUrl() + room.stream_key;

    nlohmann::json viewerList = nlohmann::json::array();
    auto viewers = RoomManager::instance().getViewerList(room.id);
    for (const auto& v : viewers) {
        nlohmann::json viewer;
        viewer["id"] = v.user_id;
        viewer["username"] = v.username;
        viewer["avatar_id"] = v.avatar_id;
        viewerList.push_back(viewer);
    }

    nlohmann::json data;
    data["room_info"] = roomJson;
    data["viewer_list"] = viewerList;

    result["code"] = 0;
    result["msg"] = "success";
    result["data"] = data;
    return result;
}

nlohmann::json RoomService::endRoom(const std::string& token, int roomId) {
    nlohmann::json result;

    int userId = 0;
    if (!UserService::verifyToken(token, userId)) {
        result["code"] = 2001;
        result["msg"] = "无效的token";
        return result;
    }

    RoomInfo room;
    if (!RoomDao::findRoomById(roomId, room)) {
        result["code"] = 2005;
        result["msg"] = "直播间不存在";
        return result;
    }

    if (room.anchor_id != userId) {
        result["code"] = 2006;
        result["msg"] = "无权结束该直播间";
        return result;
    }

    if (room.status != "live") {
        result["code"] = 2007;
        result["msg"] = "直播间已结束";
        return result;
    }

    if (!RoomDao::updateRoomStatus(roomId, "ended")) {
        result["code"] = 2008;
        result["msg"] = "结束直播失败";
        return result;
    }

    RoomManager::instance().clearRoom(roomId);

    result["code"] = 0;
    result["msg"] = "直播已结束";
    return result;
}

nlohmann::json RoomService::joinRoom(const std::string& token, int roomId) {
    nlohmann::json result;

    int userId = 0;
    if (!UserService::verifyToken(token, userId)) {
        result["code"] = 2001;
        result["msg"] = "无效的token";
        return result;
    }

    RoomInfo room;
    if (!RoomDao::findRoomById(roomId, room)) {
        result["code"] = 2005;
        result["msg"] = "直播间不存在";
        return result;
    }

    if (room.status != "live") {
        result["code"] = 2007;
        result["msg"] = "直播已结束";
        return result;
    }

    UserInfo user;
    if (!UserDao::findUserById(userId, user)) {
        result["code"] = 2009;
        result["msg"] = "用户不存在";
        return result;
    }

    if (!RoomManager::instance().joinRoom(roomId, userId, user.username, user.avatar_id)) {
        result["code"] = 2010;
        result["msg"] = "已在直播间中";
        return result;
    }

    int viewerCount = RoomManager::instance().getViewerCount(roomId);
    RoomDao::updateViewerCount(roomId, viewerCount);

    nlohmann::json data;
    data["room_id"] = roomId;
    data["viewer_count"] = viewerCount;
    data["play_url"] = getRtmpBaseUrl() + room.stream_key;

    result["code"] = 0;
    result["msg"] = "加入成功";
    result["data"] = data;
    return result;
}

nlohmann::json RoomService::leaveRoom(const std::string& token, int roomId) {
    nlohmann::json result;

    int userId = 0;
    if (!UserService::verifyToken(token, userId)) {
        result["code"] = 2001;
        result["msg"] = "无效的token";
        return result;
    }

    if (!RoomManager::instance().leaveRoom(roomId, userId)) {
        result["code"] = 2011;
        result["msg"] = "不在该直播间中";
        return result;
    }

    int viewerCount = RoomManager::instance().getViewerCount(roomId);
    RoomDao::updateViewerCount(roomId, viewerCount);

    nlohmann::json data;
    data["room_id"] = roomId;
    data["viewer_count"] = viewerCount;

    result["code"] = 0;
    result["msg"] = "离开成功";
    result["data"] = data;
    return result;
}
