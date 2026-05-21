#include "core/Logger.h"
#include "network/HttpServer.h"
#include "network/HttpRequest.h"
#include "network/HttpResponse.h"
#include "network/WebSocketHandler.h"
#include "network/Connection.h"
#include "network/StaticFileHandler.h"
#include "utils/Config.h"
#include "database/Database.h"
#include "business/UserService.h"
#include "business/RoomService.h"
#include "business/RoomManager.h"
#include "business/GiftService.h"
#include "business/ReplayService.h"
#include "database/RoomDao.h"
#include "database/UserDao.h"
#include "recording/RecordingManager.h"
#include <csignal>
#include <iostream>
#include <chrono>
#include <thread>
#include <nlohmann/json.hpp>

HttpServer* g_server = nullptr;

void signalHandler(int signum) {
    LOG_INFO("received signal " << signum << ", shutting down...");
    RecordingManager::instance().stopAll();
    if (g_server) {
        g_server->stop();
    }
}

static std::string getTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto timeT = std::chrono::system_clock::to_time_t(now);
    char buf[64];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&timeT));
    return std::string(buf);
}

static std::unordered_map<int, int> g_likeCounts;
static std::mutex g_likeMutex;

void persistLikeCount(int roomId) {
    std::lock_guard<std::mutex> lock(g_likeMutex);
    auto it = g_likeCounts.find(roomId);
    if (it != g_likeCounts.end() && it->second > 0) {
        RoomDao::updateLikeCount(roomId, it->second);
    }
}

void onWsOpen(std::shared_ptr<Connection> conn, const std::string& rawData) {
    HttpRequest req;
    req.parse(rawData);
    std::string token = req.getParam("token");
    std::string roomIdStr = req.getParam("room_id");
    int roomId = 0;
    try { roomId = std::stoi(roomIdStr); } catch (...) { return; }

    int userId = 0;
    if (!UserService::verifyToken(token, userId)) {
        WebSocketHandler::sendClose(conn, 4001, "invalid token");
        return;
    }

    RoomInfo room;
    if (!RoomDao::findRoomById(roomId, room) || room.status != "live") {
        WebSocketHandler::sendClose(conn, 4002, "room not live");
        return;
    }

    UserInfo user;
    if (!UserDao::findUserById(userId, user)) {
        WebSocketHandler::sendClose(conn, 4003, "user not found");
        return;
    }

    WsContext* ctx = WebSocketHandler::getWsContext(conn);
    if (!ctx) return;
    ctx->userId = userId;
    ctx->username = user.username;
    ctx->avatarId = user.avatar_id;
    ctx->roomId = roomId;

    RoomManager::instance().joinRoom(roomId, userId, user.username, user.avatar_id, conn);
    int viewerCount = RoomManager::instance().getViewerCount(roomId);
    RoomDao::updateViewerCount(roomId, viewerCount);

    nlohmann::json joinMsg;
    joinMsg["type"] = "viewer_join";
    joinMsg["username"] = user.username;
    RoomManager::instance().broadcastToRoom(roomId, joinMsg.dump(), conn->fd());

    nlohmann::json countMsg;
    countMsg["type"] = "viewer_count";
    countMsg["count"] = viewerCount;
    RoomManager::instance().broadcastToRoom(roomId, countMsg.dump());

    LOG_INFO("ws open: user=" << user.username << " room=" << roomId);
}

void onWsMessage(std::shared_ptr<Connection> conn, const std::string& payload) {
    WsContext* ctx = WebSocketHandler::getWsContext(conn);
    if (!ctx || !ctx->handshakeDone) return;

    nlohmann::json msg;
    try {
        msg = nlohmann::json::parse(payload);
    } catch (...) {
        return;
    }

    std::string type = msg.value("type", "");
    int roomId = ctx->roomId;
    int userId = ctx->userId;
    std::string username = ctx->username;

    if (type == "danmaku") {
        std::string content = msg.value("content", "");
        if (content.empty()) return;

        nlohmann::json broadcast;
        broadcast["type"] = "danmaku";
        broadcast["username"] = username;
        broadcast["content"] = content;
        broadcast["timestamp"] = getTimestamp();
        RoomManager::instance().broadcastToRoom(roomId, broadcast.dump());

    } else if (type == "gift") {
        int giftId = msg.value("gift_id", 0);
        if (giftId <= 0) return;

        nlohmann::json giftResult = GiftService::sendGift(roomId, userId, giftId);
        if (giftResult.value("code", -1) != 0) return;

        nlohmann::json broadcast;
        broadcast["type"] = "gift";
        broadcast["username"] = username;
        broadcast["gift_id"] = giftId;
        broadcast["gift_name"] = giftResult["data"].value("gift_name", "");
        broadcast["timestamp"] = getTimestamp();
        RoomManager::instance().broadcastToRoom(roomId, broadcast.dump());

    } else if (type == "like") {
        int count = 1;
        {
            std::lock_guard<std::mutex> lock(g_likeMutex);
            g_likeCounts[roomId] += count;
            count = g_likeCounts[roomId];
        }

        RoomDao::updateLikeCount(roomId, count);

        nlohmann::json broadcast;
        broadcast["type"] = "like";
        broadcast["count"] = count;
        broadcast["timestamp"] = getTimestamp();
        RoomManager::instance().broadcastToRoom(roomId, broadcast.dump());
    }
}

void onWsClose(std::shared_ptr<Connection> conn) {
    WsContext* ctx = WebSocketHandler::getWsContext(conn);
    if (!ctx) return;

    int roomId = ctx->roomId;
    int userId = ctx->userId;
    std::string username = ctx->username;

    RoomManager::instance().leaveRoomByFd(conn->fd());

    int viewerCount = RoomManager::instance().getViewerCount(roomId);
    RoomDao::updateViewerCount(roomId, viewerCount);

    nlohmann::json leaveMsg;
    leaveMsg["type"] = "viewer_leave";
    leaveMsg["username"] = username;
    RoomManager::instance().broadcastToRoom(roomId, leaveMsg.dump());

    nlohmann::json countMsg;
    countMsg["type"] = "viewer_count";
    countMsg["count"] = viewerCount;
    RoomManager::instance().broadcastToRoom(roomId, countMsg.dump());

    LOG_INFO("ws close: user=" << username << " room=" << roomId);
}

void registerRoutes(HttpServer& server) {
    server.router().get("/", [](HttpRequest& req, HttpResponse& resp) {
        resp.setJson(0, "Welcome to LiveKit Server");
    });

    server.router().get("/api/ping", [](HttpRequest& req, HttpResponse& resp) {
        resp.setJson(0, "pong");
    });

    server.router().post("/api/register", [](HttpRequest& req, HttpResponse& resp) {
        auto json = req.getJson();
        std::string username = json.value("username", "");
        std::string password = json.value("password", "");
        int avatarId = json.value("avatar_id", 1);

        nlohmann::json result = UserService::registerUser(username, password, avatarId);
        int code = result.value("code", -1);
        std::string msg = result.value("msg", "");
        nlohmann::json data = result.value("data", nlohmann::json::object());
        resp.setJson(code, msg, data);
    });

    server.router().post("/api/login", [](HttpRequest& req, HttpResponse& resp) {
        auto json = req.getJson();
        std::string username = json.value("username", "");
        std::string password = json.value("password", "");

        nlohmann::json result = UserService::loginUser(username, password);
        int code = result.value("code", -1);
        std::string msg = result.value("msg", "");
        nlohmann::json data = result.value("data", nlohmann::json::object());
        resp.setJson(code, msg, data);
    });

    server.router().get("/api/avatars", [](HttpRequest& req, HttpResponse& resp) {
        nlohmann::json avatars = nlohmann::json::array();
        for (int i = 1; i <= 8; ++i) {
            nlohmann::json avatar;
            avatar["id"] = i;
            avatar["url"] = "http://" + Config::instance().get("server", "host", "127.0.0.1") +
                           ":" + std::to_string(Config::instance().getInt("server", "port", 9090)) +
                           "/avatars/" + std::to_string(i) + ".png";
            avatars.push_back(avatar);
        }
        nlohmann::json data;
        data["avatars"] = avatars;
        resp.setJson(0, "success", data);
    });

    server.router().post("/api/live/create", [](HttpRequest& req, HttpResponse& resp) {
        auto json = req.getJson();
        std::string token = json.value("token", "");
        std::string title = json.value("title", "");
        std::string category = json.value("category", "other");
        std::string mode = json.value("mode", "camera");

        nlohmann::json result = RoomService::createRoom(token, title, category, mode);
        int code = result.value("code", -1);
        std::string msg = result.value("msg", "");
        nlohmann::json data = result.value("data", nlohmann::json::object());
        resp.setJson(code, msg, data);
    });

    server.router().get("/api/live/rooms", [](HttpRequest& req, HttpResponse& resp) {
        std::string category = req.getParam("category");
        if (category.empty()) category = "all";

        nlohmann::json result = RoomService::getRoomList(category);
        int code = result.value("code", -1);
        std::string msg = result.value("msg", "");
        nlohmann::json data = result.value("data", nlohmann::json::object());
        resp.setJson(code, msg, data);
    });

    server.router().get("/api/live/room/{room_id}", [](HttpRequest& req, HttpResponse& resp) {
        std::string roomIdStr = req.getPathParam("room_id");
        int roomId = 0;
        try {
            roomId = std::stoi(roomIdStr);
        } catch (...) {
            resp.setJson(2005, "无效的房间ID");
            return;
        }

        nlohmann::json result = RoomService::getRoomInfo(roomId);
        int code = result.value("code", -1);
        std::string msg = result.value("msg", "");
        nlohmann::json data = result.value("data", nlohmann::json::object());
        resp.setJson(code, msg, data);
    });

    server.router().post("/api/live/end", [](HttpRequest& req, HttpResponse& resp) {
        auto json = req.getJson();
        std::string token = json.value("token", "");
        int roomId = json.value("room_id", 0);

        persistLikeCount(roomId);

        nlohmann::json result = RoomService::endRoom(token, roomId);
        int code = result.value("code", -1);
        std::string msg = result.value("msg", "");
        nlohmann::json data = result.value("data", nlohmann::json::object());
        resp.setJson(code, msg, data);
    });

    server.router().post("/api/live/join", [](HttpRequest& req, HttpResponse& resp) {
        auto json = req.getJson();
        std::string token = json.value("token", "");
        int roomId = json.value("room_id", 0);

        nlohmann::json result = RoomService::joinRoom(token, roomId);
        int code = result.value("code", -1);
        std::string msg = result.value("msg", "");
        nlohmann::json data = result.value("data", nlohmann::json::object());
        resp.setJson(code, msg, data);
    });

    server.router().post("/api/live/leave", [](HttpRequest& req, HttpResponse& resp) {
        auto json = req.getJson();
        std::string token = json.value("token", "");
        int roomId = json.value("room_id", 0);

        nlohmann::json result = RoomService::leaveRoom(token, roomId);
        int code = result.value("code", -1);
        std::string msg = result.value("msg", "");
        nlohmann::json data = result.value("data", nlohmann::json::object());
        resp.setJson(code, msg, data);
    });

    server.router().get("/api/gifts", [](HttpRequest& req, HttpResponse& resp) {
        nlohmann::json result = GiftService::getGiftList();
        int code = result.value("code", -1);
        std::string msg = result.value("msg", "");
        nlohmann::json data = result.value("data", nlohmann::json::object());
        resp.setJson(code, msg, data);
    });

    server.router().get("/api/live/replays", [](HttpRequest& req, HttpResponse& resp) {
        nlohmann::json result = ReplayService::getReplayList();
        int code = result.value("code", -1);
        std::string msg = result.value("msg", "");
        nlohmann::json data = result.value("data", nlohmann::json::object());
        resp.setJson(code, msg, data);
    });

    server.router().get("/avatars/{file}", [](HttpRequest& req, HttpResponse& resp) {
        StaticFileHandler::handle(req, resp);
    });

    server.router().get("/gifts/{file}", [](HttpRequest& req, HttpResponse& resp) {
        StaticFileHandler::handle(req, resp);
    });

    server.router().get("/covers/{file}", [](HttpRequest& req, HttpResponse& resp) {
        StaticFileHandler::handle(req, resp);
    });

    server.router().get("/recordings/{file}", [](HttpRequest& req, HttpResponse& resp) {
        StaticFileHandler::handle(req, resp);
    });
}

int main(int argc, char* argv[]) {
    std::string configPath = "config/server.conf";
    if (argc > 1) {
        configPath = argv[1];
    }

    Config::instance().load(configPath);

    int port = Config::instance().getInt("server", "port", 9090);
    int threadCount = Config::instance().getInt("server", "thread_count", 4);
    std::string logLevel = Config::instance().get("log", "level", "info");
    std::string logPath = Config::instance().get("log", "path", "./logs/livekit.log");
    std::string dbPath = Config::instance().get("database", "path", "./data/livekit.db");

    if (logLevel == "debug") Logger::instance().setLevel(LogLevel::DEBUG);
    else if (logLevel == "info") Logger::instance().setLevel(LogLevel::INFO);
    else if (logLevel == "warn") Logger::instance().setLevel(LogLevel::WARN);
    else if (logLevel == "error") Logger::instance().setLevel(LogLevel::ERROR);

    Logger::instance().setLogFile(logPath);

    LOG_INFO("LiveKit Server starting...");
    LOG_INFO("port=" << port << " thread_count=" << threadCount);

    if (!Database::instance().open(dbPath)) {
        LOG_ERROR("failed to open database");
        return 1;
    }

    std::string sqlPath = "scripts/init_db.sql";
    if (!Database::instance().executeFile(sqlPath)) {
        LOG_WARN("failed to execute init_db.sql (may already exist)");
    }

    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGPIPE, SIG_IGN);

    HttpServer server(port, threadCount);
    g_server = &server;

    server.setWsOpenCallback(onWsOpen);
    WebSocketHandler::setMessageCallback(onWsMessage);
    server.setWsCloseCallback(onWsClose);

    registerRoutes(server);

    server.start();

    RecordingManager::instance().stopAll();
    Database::instance().close();
    LOG_INFO("LiveKit Server stopped");
    return 0;
}
