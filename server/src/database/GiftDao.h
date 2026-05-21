#pragma once

#include <string>
#include <vector>

struct GiftInfo {
    int id;
    std::string name;
    std::string icon;
};

class GiftDao {
public:
    static bool getGiftList(std::vector<GiftInfo>& outGifts);
    static bool findGiftById(int giftId, GiftInfo& outGift);
    static bool createGiftRecord(int roomId, int senderId, int giftId);
};
