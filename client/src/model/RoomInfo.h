#pragma once

#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QVector>

class RoomInfo {
public:
    RoomInfo();

    int roomId() const;
    void setRoomId(int id);

    QString title() const;
    void setTitle(const QString& title);

    QString anchorName() const;
    void setAnchorName(const QString& name);

    int anchorAvatarId() const;
    void setAnchorAvatarId(int id);

    int viewerCount() const;
    void setViewerCount(int count);

    QString category() const;
    void setCategory(const QString& category);

    QString coverUrl() const;
    void setCoverUrl(const QString& url);

    QString status() const;
    void setStatus(const QString& status);

    void fromJson(const QJsonObject& json);
    QJsonObject toJson() const;

    static QVector<RoomInfo> fromJsonArray(const QJsonArray& arr);

    QString anchorAvatarPath() const;

private:
    int m_roomId;
    QString m_title;
    QString m_anchorName;
    int m_anchorAvatarId;
    int m_viewerCount;
    QString m_category;
    QString m_coverUrl;
    QString m_status;
};
