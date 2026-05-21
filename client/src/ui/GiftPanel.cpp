#include "ui/GiftPanel.h"
#include "model/GiftInfo.h"
#include <QEvent>
#include <QFile>
#include <QLabel>
#include <QSvgRenderer>
#include <QPainter>

GiftPanel::GiftPanel(QWidget* parent)
    : QWidget(parent)
    , m_visible(false)
{
    setupUI();
    createGiftButtons();
}

void GiftPanel::setupUI() {
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedWidth(280);
    setFixedHeight(220);

    m_panelContent = new QWidget(this);
    m_panelContent->setObjectName("giftPanelContent");
    m_panelContent->setGeometry(0, 0, 280, 220);

    m_gridLayout = new QGridLayout(m_panelContent);
    m_gridLayout->setContentsMargins(16, 16, 16, 16);
    m_gridLayout->setSpacing(12);
}

void GiftPanel::createGiftButtons() {
    m_gifts = GiftInfo::allGifts();

    for (int i = 0; i < m_gifts.size(); ++i) {
        const auto& gift = m_gifts[i];
        auto* btn = new QPushButton(m_panelContent);
        btn->setObjectName("giftButton");
        btn->setFixedSize(72, 80);
        btn->setProperty("giftId", gift.giftId);

        auto* btnLayout = new QVBoxLayout(btn);
        btnLayout->setContentsMargins(4, 4, 4, 4);
        btnLayout->setSpacing(2);

        auto* iconLabel = new QLabel(btn);
        iconLabel->setAlignment(Qt::AlignCenter);
        iconLabel->setFixedSize(40, 40);

        QSvgRenderer renderer(gift.iconPath);
        if (renderer.isValid()) {
            QPixmap pixmap(40, 40);
            pixmap.fill(Qt::transparent);
            QPainter painter(&pixmap);
            renderer.render(&painter);
            iconLabel->setPixmap(pixmap);
        }
        btnLayout->addWidget(iconLabel, 0, Qt::AlignCenter);

        auto* nameLabel = new QLabel(gift.name, btn);
        nameLabel->setObjectName("giftNameLabel");
        nameLabel->setAlignment(Qt::AlignCenter);
        nameLabel->setFixedHeight(20);
        btnLayout->addWidget(nameLabel, 0, Qt::AlignCenter);

        int row = i / 3;
        int col = i % 3;
        m_gridLayout->addWidget(btn, row, col);

        connect(btn, &QPushButton::clicked, [this, gift]() {
            emit SIG_giftSelected(gift.giftId);
            hidePanel();
        });
    }
}

void GiftPanel::showPanel() {
    if (m_visible) return;
    m_visible = true;

    if (m_showAnimation) {
        delete m_showAnimation;
    }

    m_panelContent->setGeometry(0, height(), width(), height());

    m_showAnimation = new QPropertyAnimation(m_panelContent, "geometry");
    m_showAnimation->setDuration(250);
    m_showAnimation->setStartValue(QRect(0, height(), width(), height()));
    m_showAnimation->setEndValue(QRect(0, 0, width(), height()));
    m_showAnimation->setEasingCurve(QEasingCurve::OutCubic);
    m_showAnimation->start(QAbstractAnimation::DeleteWhenStopped);

    show();
}

void GiftPanel::hidePanel() {
    if (!m_visible) return;
    m_visible = false;

    if (m_hideAnimation) {
        delete m_hideAnimation;
    }

    m_hideAnimation = new QPropertyAnimation(m_panelContent, "geometry");
    m_hideAnimation->setDuration(200);
    m_hideAnimation->setStartValue(m_panelContent->geometry());
    m_hideAnimation->setEndValue(QRect(0, height(), width(), height()));
    m_hideAnimation->setEasingCurve(QEasingCurve::InCubic);
    connect(m_hideAnimation, &QPropertyAnimation::finished, this, &QWidget::hide);
    m_hideAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}

bool GiftPanel::isPanelVisible() const {
    return m_visible;
}

bool GiftPanel::event(QEvent* event) {
    if (event->type() == QEvent::WindowDeactivate && m_visible) {
        hidePanel();
    }
    return QWidget::event(event);
}
