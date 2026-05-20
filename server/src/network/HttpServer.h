#pragma once

#include <memory>
#include <string>
#include "core/EventLoop.h"
#include "network/Router.h"

class HttpRequest;
class HttpResponse;

class HttpServer {
public:
    HttpServer(int port, int numThreads);
    ~HttpServer();

    void start();
    void stop();

    Router& router();

private:
    void onNewConnection(std::shared_ptr<Connection> conn);
    void onMessage(std::shared_ptr<Connection> conn);
    void onClose(std::shared_ptr<Connection> conn);
    void handleRequest(std::shared_ptr<Connection> conn, const std::string& rawData);

    std::unique_ptr<EventLoop> m_eventLoop;
    Router m_router;
};
