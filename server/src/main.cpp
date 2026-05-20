#include "core/Logger.h"
#include "network/HttpServer.h"
#include "network/HttpRequest.h"
#include "network/HttpResponse.h"
#include "utils/Config.h"
#include <csignal>
#include <iostream>

HttpServer* g_server = nullptr;

void signalHandler(int signum) {
    LOG_INFO("received signal " << signum << ", shutting down...");
    if (g_server) {
        g_server->stop();
    }
}

int main(int argc, char* argv[]) {
    std::string configPath = "../config/server.conf";
    if (argc > 1) {
        configPath = argv[1];
    }

    Config::instance().load(configPath);

    int port = Config::instance().getInt("server", "port", 9090);
    int threadCount = Config::instance().getInt("server", "thread_count", 4);
    std::string logLevel = Config::instance().get("log", "level", "info");
    std::string logPath = Config::instance().get("log", "path", "./logs/livekit.log");

    if (logLevel == "debug") Logger::instance().setLevel(LogLevel::DEBUG);
    else if (logLevel == "info") Logger::instance().setLevel(LogLevel::INFO);
    else if (logLevel == "warn") Logger::instance().setLevel(LogLevel::WARN);
    else if (logLevel == "error") Logger::instance().setLevel(LogLevel::ERROR);

    Logger::instance().setLogFile(logPath);

    LOG_INFO("LiveKit Server starting...");
    LOG_INFO("port=" << port << " thread_count=" << threadCount);

    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    HttpServer server(port, threadCount);
    g_server = &server;

    server.router().get("/", [](HttpRequest& req, HttpResponse& resp) {
        resp.setJson(0, "Welcome to LiveKit Server");
    });

    server.router().get("/api/ping", [](HttpRequest& req, HttpResponse& resp) {
        resp.setJson(0, "pong");
    });

    server.start();

    LOG_INFO("LiveKit Server stopped");
    return 0;
}
