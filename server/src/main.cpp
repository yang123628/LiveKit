#include "core/Logger.h"
#include "network/HttpServer.h"
#include "network/HttpRequest.h"
#include "network/HttpResponse.h"
#include "utils/Config.h"
#include "database/Database.h"
#include "business/UserService.h"
#include "business/RoomService.h"
#include <csignal>
#include <iostream>
#include <nlohmann/json.hpp>

HttpServer* g_server = nullptr;

void signalHandler(int signum) {
    LOG_INFO("received signal " << signum << ", shutting down...");
    if (g_server) {
        g_server->stop();
    }
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

        nlohmann::json result = RoomService::endRoom(token, roomId);
        int code = result.value("code", -1);
        std::string msg = result.value("msg", "");
        nlohmann::json data = result.value("data", nlohmann::json::object());
        resp.setJson(code, msg, data);
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

    HttpServer server(port, threadCount);
    g_server = &server;

    registerRoutes(server);

    server.start();

    Database::instance().close();
    LOG_INFO("LiveKit Server stopped");
    return 0;
}
