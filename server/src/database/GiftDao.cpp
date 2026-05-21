#include "database/GiftDao.h"
#include "database/Database.h"
#include "core/Logger.h"

bool GiftDao::getGiftList(std::vector<GiftInfo>& outGifts) {
    std::string sql = "SELECT id, name, icon FROM gifts ORDER BY id";
    Database::instance().query(sql, [&](const std::vector<std::string>& row) {
        GiftInfo gift;
        gift.id = std::stoi(row[0]);
        gift.name = row[1];
        gift.icon = row[2];
        outGifts.push_back(gift);
    });
    return true;
}

bool GiftDao::findGiftById(int giftId, GiftInfo& outGift) {
    std::string sql = "SELECT id, name, icon FROM gifts WHERE id = ?";
    std::vector<std::string> params = {std::to_string(giftId)};
    bool found = false;
    Database::instance().queryPrepared(sql, params, [&](const std::vector<std::string>& row) {
        outGift.id = std::stoi(row[0]);
        outGift.name = row[1];
        outGift.icon = row[2];
        found = true;
    });
    return found;
}

bool GiftDao::createGiftRecord(int roomId, int senderId, int giftId) {
    std::string sql = "INSERT INTO gift_records (room_id, sender_id, gift_id) VALUES (?, ?, ?)";
    std::vector<std::string> params = {
        std::to_string(roomId),
        std::to_string(senderId),
        std::to_string(giftId)
    };
    return Database::instance().executePrepared(sql, params);
}
