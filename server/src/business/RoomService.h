#pragma once

#include <string>
#include <nlohmann/json.hpp>

class RoomService {
public:
    static nlohmann::json createRoom(const std::string& token, const std::string& title,
                                      const std::string& category, const std::string& mode);
    static nlohmann::json getRoomList(const std::string& category);
    static nlohmann::json getRoomInfo(int roomId);
    static nlohmann::json endRoom(const std::string& token, int roomId);
};
