#include "model/RoomInfo.h"

RoomInfo::RoomInfo()
    : m_roomId(0)
    , m_anchorAvatarId(1)
    , m_viewerCount(0)
{
}

int RoomInfo::roomId() const { return m_roomId; }
void RoomInfo::setRoomId(int id) { m_roomId = id; }

QString RoomInfo::title() const { return m_title; }
void RoomInfo::setTitle(const QString& title) { m_title = title; }

QString RoomInfo::anchorName() const { return m_anchorName; }
void RoomInfo::setAnchorName(const QString& name) { m_anchorName = name; }

int RoomInfo::anchorAvatarId() const { return m_anchorAvatarId; }
void RoomInfo::setAnchorAvatarId(int id) { m_anchorAvatarId = id; }

int RoomInfo::viewerCount() const { return m_viewerCount; }
void RoomInfo::setViewerCount(int count) { m_viewerCount = count; }

QString RoomInfo::category() const { return m_category; }
void RoomInfo::setCategory(const QString& category) { m_category = category; }

QString RoomInfo::coverUrl() const { return m_coverUrl; }
void RoomInfo::setCoverUrl(const QString& url) { m_coverUrl = url; }

QString RoomInfo::status() const { return m_status; }
void RoomInfo::setStatus(const QString& status) { m_status = status; }

void RoomInfo::fromJson(const QJsonObject& json) {
    m_roomId = json.value("room_id").toInt(0);
    m_title = json.value("title").toString();
    m_anchorName = json.value("anchor_name").toString();
    m_anchorAvatarId = json.value("anchor_avatar_id").toInt(1);
    m_viewerCount = json.value("viewer_count").toInt(0);
    m_category = json.value("category").toString();
    m_coverUrl = json.value("cover_url").toString();
    m_status = json.value("status").toString("live");
}

QJsonObject RoomInfo::toJson() const {
    QJsonObject obj;
    obj["room_id"] = m_roomId;
    obj["title"] = m_title;
    obj["anchor_name"] = m_anchorName;
    obj["anchor_avatar_id"] = m_anchorAvatarId;
    obj["viewer_count"] = m_viewerCount;
    obj["category"] = m_category;
    obj["cover_url"] = m_coverUrl;
    obj["status"] = m_status;
    return obj;
}

QVector<RoomInfo> RoomInfo::fromJsonArray(const QJsonArray& arr) {
    QVector<RoomInfo> result;
    result.reserve(arr.size());
    for (const auto& item : arr) {
        RoomInfo info;
        info.fromJson(item.toObject());
        result.append(info);
    }
    return result;
}

QString RoomInfo::anchorAvatarPath() const {
    return QString(":/avatars/avatar_%1.svg").arg(m_anchorAvatarId);
}
