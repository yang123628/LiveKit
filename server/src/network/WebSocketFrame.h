#pragma once

#include <string>
#include <vector>
#include <cstdint>

class WebSocketFrame {
public:
    static constexpr uint8_t OPCODE_CONTINUATION = 0x0;
    static constexpr uint8_t OPCODE_TEXT = 0x1;
    static constexpr uint8_t OPCODE_BINARY = 0x2;
    static constexpr uint8_t OPCODE_CLOSE = 0x8;
    static constexpr uint8_t OPCODE_PING = 0x9;
    static constexpr uint8_t OPCODE_PONG = 0xA;

    bool fin;
    uint8_t opcode;
    bool mask;
    uint64_t payloadLength;
    uint8_t maskingKey[4];
    std::string payload;

    WebSocketFrame();

    static bool parse(const std::string& data, WebSocketFrame& frame, size_t& consumed);
    static std::string buildTextFrame(const std::string& message);
    static std::string buildPongFrame(const std::string& payload);
    static std::string buildPingFrame();
    static std::string buildCloseFrame(uint16_t code = 1000, const std::string& reason = "");

private:
    static void applyMask(std::string& data, const uint8_t mask[4]);
};
