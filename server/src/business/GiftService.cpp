#include "business/GiftService.h"
#include "database/GiftDao.h"
#include "utils/Config.h"
#include "core/Logger.h"

nlohmann::json GiftService::getGiftList() {
    nlohmann::json result;
    std::vector<GiftInfo> gifts;
    GiftDao::getGiftList(gifts);

    std::string host = Config::instance().get("server", "host", "127.0.0.1");
    int port = Config::instance().getInt("server", "port", 9090);
    std::string baseUrl = "http://" + host + ":" + std::to_string(port);

    nlohmann::json giftList = nlohmann::json::array();
    for (const auto& g : gifts) {
        nlohmann::json item;
        item["id"] = g.id;
        item["name"] = g.name;
        item["icon"] = g.icon;
        item["icon_url"] = baseUrl + "/gifts/" + g.icon;
        giftList.push_back(item);
    }

    nlohmann::json data;
    data["gifts"] = giftList;

    result["code"] = 0;
    result["msg"] = "success";
    result["data"] = data;
    return result;
}

nlohmann::json GiftService::sendGift(int roomId, int senderId, int giftId) {
    nlohmann::json result;

    GiftInfo gift;
    if (!GiftDao::findGiftById(giftId, gift)) {
        result["code"] = 3001;
        result["msg"] = "礼物不存在";
        return result;
    }

    if (!GiftDao::createGiftRecord(roomId, senderId, giftId)) {
        result["code"] = 3002;
        result["msg"] = "礼物记录失败";
        return result;
    }

    nlohmann::json data;
    data["gift_id"] = giftId;
    data["gift_name"] = gift.name;
    data["icon_url"] = gift.icon;

    result["code"] = 0;
    result["msg"] = "success";
    result["data"] = data;
    return result;
}
