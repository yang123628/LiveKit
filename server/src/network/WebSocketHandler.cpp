#include "network/WebSocketHandler.h"
#include "network/WebSocketFrame.h"
#include "network/Connection.h"
#include "core/Logger.h"
#include <openssl/sha.h>
#include <sstream>
#include <algorithm>

WebSocketHandler::MessageCallback WebSocketHandler::m_messageCallback;
WebSocketHandler::CloseCallback WebSocketHandler::m_closeCallback;

bool WebSocketHandler::isWebSocketUpgrade(const std::string& rawData) {
    return rawData.find("Upgrade: websocket") != std::string::npos ||
           rawData.find("Upgrade: WebSocket") != std::string::npos ||
           rawData.find("upgrade: websocket") != std::string::npos;
}

bool WebSocketHandler::handshake(const std::string& rawData, std::string& response) {
    std::string clientKey;
    std::istringstream iss(rawData);
    std::string line;
    while (std::getline(iss, line)) {
        if (line.size() > 0 && line.back() == '\r') line.pop_back();
        size_t colon = line.find(':');
        if (colon != std::string::npos) {
            std::string key = line.substr(0, colon);
            std::string value = line.substr(colon + 1);
            while (!value.empty() && value[0] == ' ') value.erase(0, 1);
            if (key == "Sec-WebSocket-Key" || key == "sec-websocket-key") {
                clientKey = value;
            }
        }
    }

    if (clientKey.empty()) return false;

    std::string acceptKey = computeAcceptKey(clientKey);

    std::ostringstream resp;
    resp << "HTTP/1.1 101 Switching Protocols\r\n";
    resp << "Upgrade: websocket\r\n";
    resp << "Connection: Upgrade\r\n";
    resp << "Sec-WebSocket-Accept: " << acceptKey << "\r\n";
    resp << "\r\n";

    response = resp.str();
    return true;
}

void WebSocketHandler::processFrames(std::shared_ptr<Connection> conn, const std::string& data) {
    WsContext* ctx = getWsContext(conn);
    if (!ctx || !ctx->handshakeDone) return;

    ctx->lastPongTime = std::chrono::steady_clock::now();

    std::string remaining = data;
    while (!remaining.empty()) {
        WebSocketFrame frame;
        size_t consumed = 0;
        if (!WebSocketFrame::parse(remaining, frame, consumed)) {
            break;
        }
        remaining = remaining.substr(consumed);

        switch (frame.opcode) {
            case WebSocketFrame::OPCODE_TEXT:
                if (m_messageCallback) {
                    m_messageCallback(conn, frame.payload);
                }
                break;

            case WebSocketFrame::OPCODE_PING:
                conn->send(WebSocketFrame::buildPongFrame(frame.payload));
                break;

            case WebSocketFrame::OPCODE_PONG:
                ctx->lastPongTime = std::chrono::steady_clock::now();
                break;

            case WebSocketFrame::OPCODE_CLOSE:
                sendClose(conn, 1000, "bye");
                if (m_closeCallback) {
                    m_closeCallback(conn);
                }
                return;

            default:
                break;
        }
    }
}

void WebSocketHandler::sendText(std::shared_ptr<Connection> conn, const std::string& message) {
    std::string frame = WebSocketFrame::buildTextFrame(message);
    conn->send(frame);
}

void WebSocketHandler::sendClose(std::shared_ptr<Connection> conn, uint16_t code, const std::string& reason) {
    std::string frame = WebSocketFrame::buildCloseFrame(code, reason);
    conn->send(frame);
}

void WebSocketHandler::sendPing(std::shared_ptr<Connection> conn) {
    WsContext* ctx = getWsContext(conn);
    if (ctx) {
        ctx->lastPingTime = std::chrono::steady_clock::now();
    }
    conn->send(WebSocketFrame::buildPingFrame());
}

void WebSocketHandler::setMessageCallback(const MessageCallback& cb) {
    m_messageCallback = cb;
}

void WebSocketHandler::setCloseCallback(const CloseCallback& cb) {
    m_closeCallback = cb;
}

WsContext* WebSocketHandler::getWsContext(std::shared_ptr<Connection> conn) {
    void* data = conn->userData();
    if (!data) return nullptr;
    return static_cast<WsContext*>(data);
}

void WebSocketHandler::initWsContext(std::shared_ptr<Connection> conn) {
    WsContext* ctx = new WsContext();
    ctx->isWebSocket = true;
    ctx->handshakeDone = false;
    ctx->lastPingTime = std::chrono::steady_clock::now();
    ctx->lastPongTime = std::chrono::steady_clock::now();
    conn->setUserData(ctx);
}

std::string WebSocketHandler::computeAcceptKey(const std::string& clientKey) {
    static const std::string magic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    std::string combined = clientKey + magic;

    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(combined.c_str()), combined.size(), hash);

    return base64Encode(hash, SHA_DIGEST_LENGTH);
}

std::string WebSocketHandler::base64Encode(const unsigned char* data, size_t len) {
    static const char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string result;
    result.reserve((len + 2) / 3 * 4);

    for (size_t i = 0; i < len; i += 3) {
        unsigned int val = data[i] << 16;
        if (i + 1 < len) val |= data[i + 1] << 8;
        if (i + 2 < len) val |= data[i + 2];

        result.push_back(table[(val >> 18) & 0x3F]);
        result.push_back(table[(val >> 12) & 0x3F]);
        result.push_back((i + 1 < len) ? table[(val >> 6) & 0x3F] : '=');
        result.push_back((i + 2 < len) ? table[val & 0x3F] : '=');
    }

    return result;
}
