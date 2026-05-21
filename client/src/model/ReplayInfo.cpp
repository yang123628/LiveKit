#include "model/ReplayInfo.h"

ReplayInfo::ReplayInfo()
    : m_replayId(0)
    , m_anchorAvatarId(1)
    , m_duration(0)
{
}

int ReplayInfo::replayId() const { return m_replayId; }
void ReplayInfo::setReplayId(int id) { m_replayId = id; }

QString ReplayInfo::title() const { return m_title; }
void ReplayInfo::setTitle(const QString& title) { m_title = title; }

QString ReplayInfo::anchorName() const { return m_anchorName; }
void ReplayInfo::setAnchorName(const QString& name) { m_anchorName = name; }

int ReplayInfo::anchorAvatarId() const { return m_anchorAvatarId; }
void ReplayInfo::setAnchorAvatarId(int id) { m_anchorAvatarId = id; }

int ReplayInfo::duration() const { return m_duration; }
void ReplayInfo::setDuration(int seconds) { m_duration = seconds; }

QString ReplayInfo::coverUrl() const { return m_coverUrl; }
void ReplayInfo::setCoverUrl(const QString& url) { m_coverUrl = url; }

QString ReplayInfo::playUrl() const { return m_playUrl; }
void ReplayInfo::setPlayUrl(const QString& url) { m_playUrl = url; }

void ReplayInfo::fromJson(const QJsonObject& json) {
    m_replayId = json.value("replay_id").toInt(0);
    m_title = json.value("title").toString();
    m_anchorName = json.value("anchor_name").toString();
    m_anchorAvatarId = json.value("anchor_avatar_id").toInt(1);
    m_duration = json.value("duration").toInt(0);
    m_coverUrl = json.value("cover_url").toString();
    m_playUrl = json.value("play_url").toString();
}

QJsonObject ReplayInfo::toJson() const {
    QJsonObject obj;
    obj["replay_id"] = m_replayId;
    obj["title"] = m_title;
    obj["anchor_name"] = m_anchorName;
    obj["anchor_avatar_id"] = m_anchorAvatarId;
    obj["duration"] = m_duration;
    obj["cover_url"] = m_coverUrl;
    obj["play_url"] = m_playUrl;
    return obj;
}

QVector<ReplayInfo> ReplayInfo::fromJsonArray(const QJsonArray& arr) {
    QVector<ReplayInfo> result;
    result.reserve(arr.size());
    for (const auto& item : arr) {
        ReplayInfo info;
        info.fromJson(item.toObject());
        result.append(info);
    }
    return result;
}

QString ReplayInfo::anchorAvatarPath() const {
    return QString(":/avatars/avatar_%1.svg").arg(m_anchorAvatarId);
}

QString ReplayInfo::durationText() const {
    int h = m_duration / 3600;
    int m = (m_duration % 3600) / 60;
    int s = m_duration % 60;
    if (h > 0) {
        return QString("%1:%2:%3")
            .arg(h)
            .arg(m, 2, 10, QChar('0'))
            .arg(s, 2, 10, QChar('0'));
    }
    return QString("%1:%2")
        .arg(m, 2, 10, QChar('0'))
        .arg(s, 2, 10, QChar('0'));
}
