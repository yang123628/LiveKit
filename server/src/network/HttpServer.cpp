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

    WsContext* ctx = WebSocketHandler::getWsContext(conn);
    if (ctx && ctx->handshakeDone) {
        m_eventLoop->threadPool().submit([this, conn, rawData]() {
            WebSocketHandler::processFrames(conn, rawData);
        });
        return;
    }

    m_eventLoop->threadPool().submit([this, conn, rawData]() {
        handleRequest(conn, rawData);
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

void HttpServer::handleRequest(std::shared_ptr<Connection> conn, const std::string& rawData) {
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

    if (req.method() == HttpRequest::UNKNOWN) {
        resp.setStatus(405);
        resp.setJson(405, "Method Not Allowed");
        conn->send(resp.serialize());
        return;
    }

    m_router.route(req, resp);
    conn->send(resp.serialize());
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
