#pragma once

#include <memory>
#include <string>
#include <functional>
#include "core/EventLoop.h"
#include "network/Router.h"

class HttpRequest;
class HttpResponse;
class Connection;

class HttpServer {
public:
    using WsOpenCallback = std::function<void(std::shared_ptr<Connection>, const std::string&)>;
    using WsMessageCallback = std::function<void(std::shared_ptr<Connection>, const std::string&)>;
    using WsCloseCallback = std::function<void(std::shared_ptr<Connection>)>;

    HttpServer(int port, int numThreads);
    ~HttpServer();

    void start();
    void stop();

    Router& router();

    void setWsOpenCallback(const WsOpenCallback& cb);
    void setWsMessageCallback(const WsMessageCallback& cb);
    void setWsCloseCallback(const WsCloseCallback& cb);

private:
    void onNewConnection(std::shared_ptr<Connection> conn);
    void onMessage(std::shared_ptr<Connection> conn);
    void onClose(std::shared_ptr<Connection> conn);
    void handleRequest(std::shared_ptr<Connection> conn, const std::string& rawData);

    std::unique_ptr<EventLoop> m_eventLoop;
    Router m_router;

    WsOpenCallback m_wsOpenCallback;
    WsMessageCallback m_wsMessageCallback;
    WsCloseCallback m_wsCloseCallback;
};
