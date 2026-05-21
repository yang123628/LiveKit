#include "network/WebSocketFrame.h"
#include <cstring>
#include <netinet/in.h>

WebSocketFrame::WebSocketFrame()
    : fin(false), opcode(0), mask(false), payloadLength(0) {
    memset(maskingKey, 0, 4);
}

bool WebSocketFrame::parse(const std::string& data, WebSocketFrame& frame, size_t& consumed) {
    consumed = 0;
    if (data.size() < 2) return false;

    frame.fin = (data[0] & 0x80) != 0;
    frame.opcode = data[0] & 0x0F;
    frame.mask = (data[1] & 0x80) != 0;

    uint64_t len = data[1] & 0x7F;
    size_t headerSize = 2;

    if (len == 126) {
        if (data.size() < 4) return false;
        uint16_t extLen = 0;
        memcpy(&extLen, data.data() + 2, 2);
        len = ntohs(extLen);
        headerSize = 4;
    } else if (len == 127) {
        if (data.size() < 10) return false;
        uint64_t extLen = 0;
        memcpy(&extLen, data.data() + 2, 8);
        extLen = be64toh(extLen);
        len = extLen;
        headerSize = 10;
    }

    if (frame.mask) {
        if (data.size() < headerSize + 4) return false;
        memcpy(frame.maskingKey, data.data() + headerSize, 4);
        headerSize += 4;
    }

    if (data.size() < headerSize + len) return false;

    frame.payloadLength = len;
    frame.payload = data.substr(headerSize, len);

    if (frame.mask) {
        applyMask(frame.payload, frame.maskingKey);
    }

    consumed = headerSize + len;
    return true;
}

std::string WebSocketFrame::buildTextFrame(const std::string& message) {
    std::string frame;
    size_t len = message.size();

    frame.push_back(0x81);

    if (len <= 125) {
        frame.push_back(static_cast<char>(len));
    } else if (len <= 65535) {
        frame.push_back(126);
        uint16_t extLen = htons(static_cast<uint16_t>(len));
        frame.append(reinterpret_cast<const char*>(&extLen), 2);
    } else {
        frame.push_back(127);
        uint64_t extLen = htobe64(len);
        frame.append(reinterpret_cast<const char*>(&extLen), 8);
    }

    frame.append(message);
    return frame;
}

std::string WebSocketFrame::buildPongFrame(const std::string& payload) {
    std::string frame;
    frame.push_back(0x8A);

    size_t len = payload.size();
    if (len <= 125) {
        frame.push_back(static_cast<char>(len));
    } else if (len <= 65535) {
        frame.push_back(126);
        uint16_t extLen = htons(static_cast<uint16_t>(len));
        frame.append(reinterpret_cast<const char*>(&extLen), 2);
    }

    frame.append(payload);
    return frame;
}

std::string WebSocketFrame::buildPingFrame() {
    std::string frame;
    frame.push_back(0x89);
    frame.push_back(0x00);
    return frame;
}

std::string WebSocketFrame::buildCloseFrame(uint16_t code, const std::string& reason) {
    std::string frame;
    frame.push_back(0x88);

    std::string payload;
    uint16_t netCode = htons(code);
    payload.append(reinterpret_cast<const char*>(&netCode), 2);
    payload.append(reason);

    size_t len = payload.size();
    if (len <= 125) {
        frame.push_back(static_cast<char>(len));
    } else {
        frame.push_back(126);
        uint16_t extLen = htons(static_cast<uint16_t>(len));
        frame.append(reinterpret_cast<const char*>(&extLen), 2);
    }

    frame.append(payload);
    return frame;
}

void WebSocketFrame::applyMask(std::string& data, const uint8_t mask[4]) {
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] ^= mask[i % 4];
    }
}
