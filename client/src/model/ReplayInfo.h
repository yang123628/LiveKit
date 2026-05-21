#pragma once

#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QVector>

class ReplayInfo {
public:
    ReplayInfo();

    int replayId() const;
    void setReplayId(int id);

    QString title() const;
    void setTitle(const QString& title);

    QString anchorName() const;
    void setAnchorName(const QString& name);

    int anchorAvatarId() const;
    void setAnchorAvatarId(int id);

    int duration() const;
    void setDuration(int seconds);

    QString coverUrl() const;
    void setCoverUrl(const QString& url);

    QString playUrl() const;
    void setPlayUrl(const QString& url);

    void fromJson(const QJsonObject& json);
    QJsonObject toJson() const;

    static QVector<ReplayInfo> fromJsonArray(const QJsonArray& arr);

    QString anchorAvatarPath() const;
    QString durationText() const;

private:
    int m_replayId;
    QString m_title;
    QString m_anchorName;
    int m_anchorAvatarId;
    int m_duration;
    QString m_coverUrl;
    QString m_playUrl;
};
