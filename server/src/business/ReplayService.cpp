#include "business/ReplayService.h"
#include "database/ReplayDao.h"
#include "database/UserDao.h"
#include "utils/Config.h"
#include "core/Logger.h"

nlohmann::json ReplayService::getReplayList() {
    nlohmann::json result;

    std::vector<ReplayInfo> replays;
    ReplayDao::getReplayList(replays);

    std::string host = Config::instance().get("server", "host", "127.0.0.1");
    int port = Config::instance().getInt("server", "port", 9090);
    std::string baseUrl = "http://" + host + ":" + std::to_string(port);

    nlohmann::json replayList = nlohmann::json::array();
    for (const auto& r : replays) {
        UserInfo anchor;
        std::string anchorName = "unknown";
        if (UserDao::findUserById(r.anchor_id, anchor)) {
            anchorName = anchor.username;
        }

        std::string playUrl = baseUrl + "/recordings/" + r.file_path.substr(r.file_path.find_last_of('/') + 1);
        std::string coverUrl = baseUrl + "/covers/" + std::to_string(r.room_id) + ".jpg";

        nlohmann::json item;
        item["replay_id"] = r.id;
        item["room_id"] = r.room_id;
        item["title"] = r.title;
        item["anchor_name"] = anchorName;
        item["duration"] = r.duration;
        item["cover_url"] = coverUrl;
        item["play_url"] = playUrl;
        item["created_at"] = r.created_at;
        replayList.push_back(item);
    }

    nlohmann::json data;
    data["replays"] = replayList;

    result["code"] = 0;
    result["msg"] = "success";
    result["data"] = data;
    return result;
}

nlohmann::json ReplayService::getReplayById(int replayId) {
    nlohmann::json result;

    ReplayInfo replay;
    if (!ReplayDao::getReplayById(replayId, replay)) {
        result["code"] = 4001;
        result["msg"] = "回放不存在";
        return result;
    }

    UserInfo anchor;
    std::string anchorName = "unknown";
    if (UserDao::findUserById(replay.anchor_id, anchor)) {
        anchorName = anchor.username;
    }

    std::string host = Config::instance().get("server", "host", "127.0.0.1");
    int port = Config::instance().getInt("server", "port", 9090);
    std::string baseUrl = "http://" + host + ":" + std::to_string(port);

    std::string playUrl = baseUrl + "/recordings/" + replay.file_path.substr(replay.file_path.find_last_of('/') + 1);
    std::string coverUrl = baseUrl + "/covers/" + std::to_string(replay.room_id) + ".jpg";

    nlohmann::json data;
    data["replay_id"] = replay.id;
    data["room_id"] = replay.room_id;
    data["title"] = replay.title;
    data["anchor_name"] = anchorName;
    data["duration"] = replay.duration;
    data["cover_url"] = coverUrl;
    data["play_url"] = playUrl;
    data["created_at"] = replay.created_at;

    result["code"] = 0;
    result["msg"] = "success";
    result["data"] = data;
    return result;
}
