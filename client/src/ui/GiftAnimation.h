#pragma once

#include <QWidget>
#include <QLabel>
#include <QPropertyAnimation>
#include <QQueue>
#include <QTimer>

struct GiftInfo;

class GiftAnimation : public QWidget {
    Q_OBJECT

public:
    explicit GiftAnimation(QWidget* parent = nullptr);

    void showGift(const QString& username, int giftId);

private:
    void playNext();
    void startAnimation();

    QLabel* m_iconLabel;
    QLabel* m_textLabel;
    QPropertyAnimation* m_slideIn;
    QPropertyAnimation* m_fadeOut;
    QQueue<QPair<QString, int>> m_queue;
    QTimer* m_displayTimer;
    bool m_playing;
};
