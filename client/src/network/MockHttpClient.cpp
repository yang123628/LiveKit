#include "network/MockHttpClient.h"
#include <QDateTime>

MockHttpClient::MockHttpClient(QObject* parent)
    : IHttpClient(parent)
    , m_nextUserId(1)
{
}

void MockHttpClient::get(const QString& path, const Callback& callback) {
    QTimer::singleShot(300, [this, path, callback]() {
        if (path == "/api/avatars") {
            handleGetAvatars(callback);
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
