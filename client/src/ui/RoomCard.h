#pragma once

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QSvgRenderer>
#include <QPainter>
#include <QPixmap>

class RoomCard : public QWidget {
    Q_OBJECT

public:
    explicit RoomCard(QWidget* parent = nullptr);

    void setRoomId(int id);
    int roomId() const;

    void setCoverPixmap(const QPixmap& pixmap);
    void setAnchorAvatar(int avatarId);
    void setAnchorName(const QString& name);
    void setTitle(const QString& title);
    void setViewerCount(int count);
    void setCategory(const QString& category);

    QSize sizeHint() const override;

signals:
    void clicked(int roomId);

protected:
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    void setupUI();
    QPixmap createRoundedPixmap(const QPixmap& src, int size) const;

    int m_roomId;
    QLabel* m_coverLabel;
    QLabel* m_avatarLabel;
    QLabel* m_anchorNameLabel;
    QLabel* m_titleLabel;
    QLabel* m_viewerCountLabel;
    QLabel* m_categoryLabel;
    QWidget* m_infoBar;

    QGraphicsDropShadowEffect* m_shadowEffect;
    QPropertyAnimation* m_hoverAnimation;
    bool m_hovered;
};
