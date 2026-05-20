#include "network/HttpServer.h"
#include "network/HttpRequest.h"
#include "network/HttpResponse.h"
#include "network/Connection.h"
#include "core/Logger.h"

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
    LOG_INFO("new http connection: " << conn->ip() << ":" << conn->port());
}

void HttpServer::onMessage(std::shared_ptr<Connection> conn) {
    std::string rawData = conn->inputBuffer().retrieveAllAsString();
    if (rawData.empty()) return;
    m_eventLoop->threadPool().submit([this, conn, rawData]() {
        handleRequest(conn, rawData);
    });
}

void HttpServer::onClose(std::shared_ptr<Connection> conn) {
    LOG_INFO("http connection closed: " << conn->ip() << ":" << conn->port());
}

void HttpServer::handleRequest(std::shared_ptr<Connection> conn, const std::string& rawData) {
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
