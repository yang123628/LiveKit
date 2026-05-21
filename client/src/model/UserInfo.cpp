#include "model/UserInfo.h"

UserInfo::UserInfo()
    : m_userId(0)
    , m_avatarId(1)
{
}

int UserInfo::userId() const {
    return m_userId;
}

void UserInfo::setUserId(int id) {
    m_userId = id;
}

QString UserInfo::username() const {
    return m_username;
}

void UserInfo::setUsername(const QString& name) {
    m_username = name;
}

int UserInfo::avatarId() const {
    return m_avatarId;
}

void UserInfo::setAvatarId(int id) {
    m_avatarId = id;
}

QString UserInfo::token() const {
    return m_token;
}

void UserInfo::setToken(const QString& token) {
    m_token = token;
}

bool UserInfo::isLoggedIn() const {
    return !m_token.isEmpty() && m_userId > 0;
}

void UserInfo::fromJson(const QJsonObject& json) {
    m_userId = json.value("user_id").toInt(0);
    m_username = json.value("username").toString();
    m_avatarId = json.value("avatar_id").toInt(1);
}

QJsonObject UserInfo::toJson() const {
    QJsonObject obj;
    obj["user_id"] = m_userId;
    obj["username"] = m_username;
    obj["avatar_id"] = m_avatarId;
    return obj;
}

void UserInfo::clear() {
    m_userId = 0;
    m_username.clear();
    m_avatarId = 1;
    m_token.clear();
}

QString UserInfo::avatarResourcePath() const {
    return QString(":/avatars/avatar_%1.svg").arg(m_avatarId);
}
