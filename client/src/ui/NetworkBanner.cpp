#include "ui/NetworkBanner.h"
#include <QTimer>

NetworkBanner::NetworkBanner(QWidget* parent)
    : QWidget(parent)
    , m_visible(false)
{
    setupUI();
    hide();

    m_opacityEffect = new QGraphicsOpacityEffect(this);
    m_opacityEffect->setOpacity(0.0);
    setGraphicsEffect(m_opacityEffect);

    m_slideIn = new QPropertyAnimation(m_opacityEffect, "opacity", this);
    m_slideIn->setDuration(300);
    m_slideIn->setStartValue(0.0);
    m_slideIn->setEndValue(1.0);
    m_slideIn->setEasingCurve(QEasingCurve::OutCubic);

    m_slideOut = new QPropertyAnimation(m_opacityEffect, "opacity", this);
    m_slideOut->setDuration(300);
    m_slideOut->setStartValue(1.0);
    m_slideOut->setEndValue(0.0);
    m_slideOut->setEasingCurve(QEasingCurve::InCubic);

    connect(m_slideOut, &QPropertyAnimation::finished, this, &QWidget::hide);
}

void NetworkBanner::setupUI() {
    setObjectName("networkBanner");
    setFixedHeight(36);

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(16, 0, 16, 0);
    layout->setSpacing(8);

    m_iconLabel = new QLabel(QStringLiteral("⚠"), this);
    m_iconLabel->setObjectName("networkBannerIcon");
    m_iconLabel->setFixedSize(20, 20);
    layout->addWidget(m_iconLabel);

    m_messageLabel = new QLabel(this);
    m_messageLabel->setObjectName("networkBannerText");
    layout->addWidget(m_messageLabel, 1);
}

void NetworkBanner::showDisconnected() {
    m_messageLabel->setText(QStringLiteral("网络连接已断开，正在重连..."));
    showBannerInternal();
}

void NetworkBanner::showReconnecting(int attempt) {
    m_messageLabel->setText(QStringLiteral("正在重连... (第%1次)").arg(attempt));
    showBannerInternal();
}

void NetworkBanner::showConnected() {
    m_messageLabel->setText(QStringLiteral("网络已恢复"));
    QTimer::singleShot(2000, this, &NetworkBanner::hideBanner);
}

void NetworkBanner::showBannerInternal() {
    if (m_visible) return;
    m_visible = true;
    show();
    raise();
    m_slideOut->stop();
    m_slideIn->start();
}

void NetworkBanner::hideBanner() {
    if (!m_visible) return;
    m_visible = false;
    m_slideIn->stop();
    m_slideOut->start();
}
