#pragma once

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QSvgRenderer>
#include <QPainter>
#include <QPixmap>

class ReplayCard : public QWidget {
    Q_OBJECT

public:
    explicit ReplayCard(QWidget* parent = nullptr);

    void setReplayId(int id);
    int replayId() const;

    void setPlayUrl(const QString& url);
    QString playUrl() const;

    void setAnchorAvatar(int avatarId);
    void setAnchorName(const QString& name);
    void setTitle(const QString& title);
    void setDuration(const QString& duration);

    QSize sizeHint() const override;

signals:
    void clicked(int replayId, const QString& playUrl);

protected:
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    void setupUI();

    int m_replayId;
    QString m_playUrl;
    QLabel* m_coverLabel;
    QLabel* m_avatarLabel;
    QLabel* m_anchorNameLabel;
    QLabel* m_titleLabel;
    QLabel* m_durationLabel;
    QWidget* m_infoBar;

    QGraphicsDropShadowEffect* m_shadowEffect;
    bool m_hovered;
};
