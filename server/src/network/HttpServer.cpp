#include "network/HttpServer.h"
#include "network/HttpRequest.h"
#include "network/HttpResponse.h"
#include "network/Connection.h"
#include "network/WebSocketHandler.h"
#include "core/Logger.h"
#include <chrono>

HttpServer::HttpServer(int port, int numThreads)
    : m_eventLoop(std::make_unique<EventLoop>(port, numThreads)) {
}

HttpServer::~HttpServer() {
    stop();
}

void HttpServer::start() {
    m_eventLoop->setNewConnectionCallback([this](std::shared_ptr<Connection> conn) {
        onNewConnection(conn);
    });
    m_eventLoop->setMessageCallback([this](std::shared_ptr<Connection> conn) {
        onMessage(conn);
    });
    m_eventLoop->setCloseCallback([this](std::shared_ptr<Connection> conn) {
        onClose(conn);
    });

    if (!m_eventLoop->start()) {
        LOG_ERROR("HttpServer start failed");
        return;
    }
    m_eventLoop->loop();
}

void HttpServer::stop() {
    m_eventLoop->stop();
}

Router& HttpServer::router() {
    return m_router;
}

void HttpServer::onNewConnection(std::shared_ptr<Connection> conn) {
    LOG_INFO("new connection: " << conn->ip() << ":" << conn->port());
}

void HttpServer::onMessage(std::shared_ptr<Connection> conn) {
    std::string rawData = conn->inputBuffer().retrieveAllAsString();
    if (rawData.empty()) return;

    LOG_INFO("onMessage fd=" << conn->fd() << " bytes=" << rawData.size() << " first=" << rawData.substr(0, std::min((size_t)50, rawData.size())));

    WsContext* ctx = WebSocketHandler::getWsContext(conn);
    if (ctx && ctx->handshakeDone) {
        m_eventLoop->threadPool().submit([this, conn, rawData]() {
            WebSocketHandler::processFrames(conn, rawData);
        });
        return;
    }

    size_t headerEnd = rawData.find("\r\n\r\n");
    if (headerEnd == std::string::npos) {
        LOG_WARN("incomplete HTTP request, no header end");
        HttpResponse resp;
        resp.setStatus(400);
        resp.setJson(400, "Bad Request");
        conn->send(resp.serialize());
        return;
    }

    bool isPost = (rawData.size() >= 4 && rawData.substr(0, 4) == "POST");
    if (isPost) {
        size_t clPos = rawData.find("Content-Length:");
        if (clPos != std::string::npos && clPos < headerEnd) {
            size_t clStart = clPos + 16;
            while (clStart < headerEnd && (rawData[clStart] == ' ' || rawData[clStart] == '\t')) clStart++;
            size_t clEnd = rawData.find("\r\n", clStart);
            if (clEnd != std::string::npos) {
                int contentLength = std::atoi(rawData.c_str() + clStart);
                size_t bodyAvailable = rawData.size() - (headerEnd + 4);
                if ((int)bodyAvailable < contentLength) {
                    LOG_WARN("incomplete POST body: have=" << bodyAvailable << " need=" << contentLength);
                    HttpResponse resp;
                    resp.setStatus(400);
                    resp.setJson(400, "Bad Request");
                    conn->send(resp.serialize());
                    return;
                }
            }
        }
    }

    if (rawData.find("Expect: 100-continue") != std::string::npos) {
        conn->send("HTTP/1.1 100 Continue\r\n\r\n");
    }

    auto startTime = std::chrono::steady_clock::now();

    m_eventLoop->threadPool().submit([this, conn, rawData, startTime]() {
        handleRequest(conn, rawData, startTime);
    });
}

void HttpServer::onClose(std::shared_ptr<Connection> conn) {
    WsContext* ctx = WebSocketHandler::getWsContext(conn);
    if (ctx && ctx->handshakeDone) {
        if (m_wsCloseCallback) {
            m_wsCloseCallback(conn);
        }
        delete ctx;
        conn->setUserData(nullptr);
    }
    LOG_INFO("connection closed: " << conn->ip() << ":" << conn->port());
}

void HttpServer::handleRequest(std::shared_ptr<Connection> conn, const std::string& rawData, std::chrono::steady_clock::time_point startTime) {
    if (WebSocketHandler::isWebSocketUpgrade(rawData)) {
        std::string response;
        if (WebSocketHandler::handshake(rawData, response)) {
            conn->send(response);
            WebSocketHandler::initWsContext(conn);
            WsContext* ctx = WebSocketHandler::getWsContext(conn);
            if (ctx) {
                ctx->handshakeDone = true;
            }
            if (m_wsOpenCallback) {
                m_wsOpenCallback(conn, rawData);
            }
        } else {
            HttpResponse resp;
            resp.setStatus(400);
            resp.setJson(400, "WebSocket handshake failed");
            conn->send(resp.serialize());
        }
        return;
    }

    if (rawData.find("Expect: 100-continue") != std::string::npos) {
        conn->send("HTTP/1.1 100 Continue\r\n\r\n");
    }

    HttpRequest req;
    if (!req.parse(rawData)) {
        HttpResponse resp;
        resp.setStatus(400);
        resp.setJson(400, "Bad Request");
        resp.setHeader("Access-Control-Allow-Origin", "*");
        conn->send(resp.serialize());
        return;
    }

    HttpResponse resp;
    resp.setHeader("Access-Control-Allow-Origin", "*");
    resp.setHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    resp.setHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
    resp.setHeader("Connection", "keep-alive");

    if (req.method() == HttpRequest::UNKNOWN) {
        resp.setStatus(405);
        resp.setJson(405, "Method Not Allowed");
        conn->send(resp.serialize());
        return;
    }

    m_router.route(req, resp);
    conn->send(resp.serialize());

    auto endTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    std::string methodStr;
    switch (req.method()) {
        case HttpRequest::GET: methodStr = "GET"; break;
        case HttpRequest::POST: methodStr = "POST"; break;
        default: methodStr = "UNKNOWN"; break;
    }

    LOG_INFO("HTTP " << methodStr << " " << req.path() << " " << resp.statusCode() << " " << duration << "ms");
}

void HttpServer::setWsOpenCallback(const WsOpenCallback& cb) {
    m_wsOpenCallback = cb;
}

void HttpServer::setWsMessageCallback(const WsMessageCallback& cb) {
    m_wsMessageCallback = cb;
}

void HttpServer::setWsCloseCallback(const WsCloseCallback& cb) {
    m_wsCloseCallback = cb;
}
