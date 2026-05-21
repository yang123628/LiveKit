#include "network/MockHttpClient.h"
#include <QDateTime>
#include <QUrl>
#include <QUrlQuery>

MockHttpClient::MockHttpClient(QObject* parent)
    : IHttpClient(parent)
    , m_nextUserId(1)
{
}

void MockHttpClient::get(const QString& path, const Callback& callback) {
    QTimer::singleShot(300, [this, path, callback]() {
        if (path == "/api/avatars") {
            handleGetAvatars(callback);
        } else if (path.startsWith("/api/live/rooms")) {
            handleGetRooms(path, callback);
        } else {
            QJsonObject resp;
            resp["code"] = -1;
            resp["msg"] = QString::fromUtf8("未知接口");
            callback(ApiResponse(resp));
        }
    });
}

void MockHttpClient::post(const QString& path, const QJsonObject& body, const Callback& callback) {
    QTimer::singleShot(500, [this, path, body, callback]() {
        if (path == "/api/login") {
            handleLogin(body, callback);
        } else if (path == "/api/register") {
            handleRegister(body, callback);
        } else {
            QJsonObject resp;
            resp["code"] = -1;
            resp["msg"] = QString::fromUtf8("未知接口");
            callback(ApiResponse(resp));
        }
    });
}

void MockHttpClient::handleGetAvatars(const Callback& callback) {
    QJsonArray avatars;
    for (int i = 1; i <= 8; ++i) {
        QJsonObject avatar;
        avatar["id"] = i;
        avatar["url"] = QString(":/avatars/avatar_%1.svg").arg(i);
        avatars.append(avatar);
    }

    QJsonObject data;
    data["avatars"] = avatars;

    QJsonObject resp;
    resp["code"] = 0;
    resp["msg"] = QString::fromUtf8("ok");
    resp["data"] = data;
    callback(ApiResponse(resp));
}

void MockHttpClient::handleGetRooms(const QString& path, const Callback& callback) {
    QString category;
    int queryPos = path.indexOf('?');
    if (queryPos >= 0) {
        QString queryStr = path.mid(queryPos + 1);
        QUrlQuery query(queryStr);
        category = query.queryItemValue("category");
    }

    QJsonArray roomsArr;

    auto addRoom = [&](int id, const QString& title, const QString& anchor,
                       int avatarId, int viewers, const QString& cat) {
        if (!category.isEmpty() && cat != category) return;
        QJsonObject obj;
        obj["room_id"] = id;
        obj["title"] = title;
        obj["anchor_name"] = anchor;
        obj["anchor_avatar_id"] = avatarId;
        obj["viewer_count"] = viewers;
        obj["category"] = cat;
        obj["cover_url"] = "";
        obj["status"] = "live";
        roomsArr.append(obj);
    };

    addRoom(1, QStringLiteral("英雄联盟排位赛"), QStringLiteral("小明"), 1, 12580, QStringLiteral("游戏"));
    addRoom(2, QStringLiteral("聊天唱歌放松一下"), QStringLiteral("小红"), 2, 8340, QStringLiteral("音乐"));
    addRoom(3, QStringLiteral("原神深渊满星挑战"), QStringLiteral("阿杰"), 3, 5620, QStringLiteral("游戏"));
    addRoom(4, QStringLiteral("深夜聊天室 来坐坐"), QStringLiteral("小美"), 4, 3210, QStringLiteral("聊天"));
    addRoom(5, QStringLiteral("吉他弹唱直播"), QStringLiteral("音乐人"), 5, 2890, QStringLiteral("音乐"));
    addRoom(6, QStringLiteral("Minecraft 建筑大赛"), QStringLiteral("方块哥"), 6, 4150, QStringLiteral("游戏"));
    addRoom(7, QStringLiteral("日常闲聊 说说心里话"), QStringLiteral("暖心姐"), 7, 1980, QStringLiteral("聊天"));
    addRoom(8, QStringLiteral("编程直播 写代码"), QStringLiteral("程序员老王"), 8, 760, QStringLiteral("其他"));

    QJsonObject data;
    data["rooms"] = roomsArr;

    QJsonObject resp;
    resp["code"] = 0;
    resp["msg"] = QString::fromUtf8("ok");
    resp["data"] = data;
    callback(ApiResponse(resp));
}

void MockHttpClient::handleLogin(const QJsonObject& body, const Callback& callback) {
    auto username = body.value("username").toString();
    auto password = body.value("password").toString();

    if (username.isEmpty() || password.isEmpty()) {
        QJsonObject resp;
        resp["code"] = 1001;
        resp["msg"] = QString::fromUtf8("用户名或密码不能为空");
        callback(ApiResponse(resp));
        return;
    }

    if (password.length() < 6) {
        QJsonObject resp;
        resp["code"] = 1002;
        resp["msg"] = QString::fromUtf8("密码错误");
        callback(ApiResponse(resp));
        return;
    }

    auto token = QString("mock_token_%1_%2").arg(username).arg(QDateTime::currentMSecsSinceEpoch());

    QJsonObject userInfo;
    userInfo["user_id"] = m_nextUserId;
    userInfo["username"] = username;
    userInfo["avatar_id"] = 1;

    QJsonObject data;
    data["token"] = token;
    data["user_info"] = userInfo;

    QJsonObject resp;
    resp["code"] = 0;
    resp["msg"] = QString::fromUtf8("ok");
    resp["data"] = data;
    callback(ApiResponse(resp));
}

void MockHttpClient::handleRegister(const QJsonObject& body, const Callback& callback) {
    auto username = body.value("username").toString();
    auto password = body.value("password").toString();
    auto avatarId = body.value("avatar_id").toInt(1);

    if (username.length() < 3 || username.length() > 20) {
        QJsonObject resp;
        resp["code"] = 2001;
        resp["msg"] = QString::fromUtf8("用户名需3-20个字符");
        callback(ApiResponse(resp));
        return;
    }

    if (password.length() < 6 || password.length() > 20) {
        QJsonObject resp;
        resp["code"] = 2002;
        resp["msg"] = QString::fromUtf8("密码需6-20个字符");
        callback(ApiResponse(resp));
        return;
    }

    auto token = QString("mock_token_%1_%2").arg(username).arg(QDateTime::currentMSecsSinceEpoch());
    int userId = m_nextUserId++;

    QJsonObject userInfo;
    userInfo["user_id"] = userId;
    userInfo["username"] = username;
    userInfo["avatar_id"] = avatarId;

    QJsonObject data;
    data["token"] = token;
    data["user_info"] = userInfo;

    QJsonObject resp;
    resp["code"] = 0;
    resp["msg"] = QString::fromUtf8("ok");
    resp["data"] = data;
    callback(ApiResponse(resp));
}
