#pragma once

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

class NetworkBanner : public QWidget {
    Q_OBJECT

public:
    explicit NetworkBanner(QWidget* parent = nullptr);

    void showDisconnected();
    void showReconnecting(int attempt);
    void showConnected();
    void hideBanner();

private:
    void setupUI();
    void showBannerInternal();

    QLabel* m_iconLabel;
    QLabel* m_messageLabel;
    QPropertyAnimation* m_slideIn;
    QPropertyAnimation* m_slideOut;
    QGraphicsOpacityEffect* m_opacityEffect;
    bool m_visible;
};
