#pragma once

#include <QString>
#include <QJsonObject>

class UserInfo {
public:
    UserInfo();

    int userId() const;
    void setUserId(int id);

    QString username() const;
    void setUsername(const QString& name);

    int avatarId() const;
    void setAvatarId(int id);

    QString token() const;
    void setToken(const QString& token);

    bool isLoggedIn() const;

    void fromJson(const QJsonObject& json);
    QJsonObject toJson() const;

    void clear();

    QString avatarResourcePath() const;

private:
    int m_userId;
    QString m_username;
    int m_avatarId;
    QString m_token;
};
