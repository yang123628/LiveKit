#pragma once

#include <string>
#include <functional>
#include <memory>
#include <chrono>

class Connection;

struct WsContext {
    bool isWebSocket = false;
    bool handshakeDone = false;
    int userId = 0;
    std::string username;
    int avatarId = 0;
    int roomId = 0;
    std::chrono::steady_clock::time_point lastPingTime;
    std::chrono::steady_clock::time_point lastPongTime;
};

class WebSocketHandler {
public:
    using MessageCallback = std::function<void(std::shared_ptr<Connection>, const std::string&)>;
    using CloseCallback = std::function<void(std::shared_ptr<Connection>)>;

    static bool isWebSocketUpgrade(const std::string& rawData);
    static bool handshake(const std::string& rawData, std::string& response);
    static void processFrames(std::shared_ptr<Connection> conn, const std::string& data);
    static void sendText(std::shared_ptr<Connection> conn, const std::string& message);
    static void sendClose(std::shared_ptr<Connection> conn, uint16_t code = 1000, const std::string& reason = "");
    static void sendPing(std::shared_ptr<Connection> conn);

    static void setMessageCallback(const MessageCallback& cb);
    static void setCloseCallback(const CloseCallback& cb);

    static WsContext* getWsContext(std::shared_ptr<Connection> conn);
    static void initWsContext(std::shared_ptr<Connection> conn);

private:
    static std::string computeAcceptKey(const std::string& clientKey);
    static std::string base64Encode(const unsigned char* data, size_t len);

    static MessageCallback m_messageCallback;
    static CloseCallback m_closeCallback;
};
