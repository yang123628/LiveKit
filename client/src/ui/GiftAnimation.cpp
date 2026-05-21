#include "ui/GiftAnimation.h"
#include "model/GiftInfo.h"
#include <QVBoxLayout>
#include <QGraphicsOpacityEffect>
#include <QSvgRenderer>
#include <QPainter>

GiftAnimation::GiftAnimation(QWidget* parent)
    : QWidget(parent)
    , m_playing(false)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(300, 80);

    auto* effect = new QGraphicsOpacityEffect(this);
    effect->setOpacity(0.0);
    setGraphicsEffect(effect);

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 5, 10, 5);
    layout->setSpacing(8);

    m_iconLabel = new QLabel(this);
    m_iconLabel->setFixedSize(50, 50);
    layout->addWidget(m_iconLabel);

    m_textLabel = new QLabel(this);
    m_textLabel->setObjectName("giftAnimText");
    m_textLabel->setWordWrap(true);
    layout->addWidget(m_textLabel, 1);

    m_displayTimer = new QTimer(this);
    m_displayTimer->setSingleShot(true);
    connect(m_displayTimer, &QTimer::timeout, this, [this]() {
        auto* effect = qobject_cast<QGraphicsOpacityEffect*>(graphicsEffect());
        if (!effect) return;

        if (m_fadeOut) {
            delete m_fadeOut;
        }

        m_fadeOut = new QPropertyAnimation(effect, "opacity");
        m_fadeOut->setDuration(500);
        m_fadeOut->setStartValue(1.0);
        m_fadeOut->setEndValue(0.0);
        connect(m_fadeOut, &QPropertyAnimation::finished, this, [this]() {
            m_playing = false;
            if (!m_queue.isEmpty()) {
                playNext();
            }
        });
        m_fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
    });
}

void GiftAnimation::showGift(const QString& username, int giftId) {
    m_queue.enqueue(qMakePair(username, giftId));
    if (!m_playing) {
        playNext();
    }
}

void GiftAnimation::playNext() {
    if (m_queue.isEmpty()) return;

    auto item = m_queue.dequeue();
    m_playing = true;

    auto gifts = GiftInfo::allGifts();
    QString giftName;
    QString iconPath;

    for (const auto& g : gifts) {
        if (g.giftId == item.second) {
            giftName = g.name;
            iconPath = g.iconPath;
            break;
        }
    }

    if (giftName.isEmpty()) {
        giftName = QStringLiteral("礼物");
    }

    QSvgRenderer renderer(iconPath);
    if (renderer.isValid()) {
        QPixmap pixmap(50, 50);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        renderer.render(&painter);
        m_iconLabel->setPixmap(pixmap);
    }

    m_textLabel->setText(QStringLiteral("<b style='color:#e94560;'>%1</b><br>送出了 <b>%2</b>")
        .arg(item.first).arg(giftName));

    auto* effect = qobject_cast<QGraphicsOpacityEffect*>(graphicsEffect());
    if (effect) {
        effect->setOpacity(1.0);
    }

    move(parentWidget() ? parentWidget()->width() - width() - 20 : 300, 20);

    if (m_slideIn) {
        delete m_slideIn;
    }

    int startX = parentWidget() ? parentWidget()->width() : 600;
    int endX = parentWidget() ? parentWidget()->width() - width() - 20 : 300;

    m_slideIn = new QPropertyAnimation(this, "geometry");
    m_slideIn->setDuration(400);
    m_slideIn->setStartValue(QRect(startX, y(), width(), height()));
    m_slideIn->setEndValue(QRect(endX, y(), width(), height()));
    m_slideIn->setEasingCurve(QEasingCurve::OutBack);
    m_slideIn->start(QAbstractAnimation::DeleteWhenStopped);

    m_displayTimer->start(3000);
}
