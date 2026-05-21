#pragma once

#include <nlohmann/json.hpp>
#include <string>

class GiftService {
public:
    static nlohmann::json getGiftList();
    static nlohmann::json sendGift(int roomId, int senderId, int giftId);
};
