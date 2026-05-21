#include "ui/FloatingHeartsWidget.h"
#include <QGraphicsOpacityEffect>
#include <QRandomGenerator>

FloatingHeartsWidget::FloatingHeartsWidget(QWidget* parent)
    : QWidget(parent)
    , m_maxHearts(10)
{
    setAttribute(Qt::WA_TranslucentBackground);
}

void FloatingHeartsWidget::addHeart() {
    if (m_activeHearts.size() >= m_maxHearts) {
        auto* oldest = m_activeHearts.takeFirst();
        oldest->deleteLater();
    }
    createHeart();
}

void FloatingHeartsWidget::createHeart() {
    int size = QRandomGenerator::global()->bounded(16, 29);
    int offsetX = QRandomGenerator::global()->bounded(-30, 31);

    auto* heart = new QLabel(this);
    heart->setText(QStringLiteral("❤"));
    heart->setStyleSheet(
        QString("QLabel { color: #e94560; font-size: %1px; background: transparent; }")
            .arg(size));
    heart->setAlignment(Qt::AlignCenter);
    heart->setFixedSize(size + 10, size + 10);

    int startX = width() / 2 + offsetX - heart->width() / 2;
    int startY = height() - heart->height();

    heart->move(startX, startY);
    heart->show();

    auto* opacityEffect = new QGraphicsOpacityEffect(heart);
    opacityEffect->setOpacity(1.0);
    heart->setGraphicsEffect(opacityEffect);

    auto* moveAnim = new QPropertyAnimation(heart, "pos");
    moveAnim->setDuration(1500);
    moveAnim->setStartValue(QPoint(startX, startY));
    int endX = startX + QRandomGenerator::global()->bounded(-20, 21);
    int endY = -heart->height();
    moveAnim->setEndValue(QPoint(endX, endY));
    moveAnim->setEasingCurve(QEasingCurve::OutQuad);

    auto* fadeAnim = new QPropertyAnimation(opacityEffect, "opacity");
    fadeAnim->setDuration(1500);
    fadeAnim->setStartValue(1.0);
    fadeAnim->setEndValue(0.0);

    connect(moveAnim, &QPropertyAnimation::finished, this, [this, heart]() {
        m_activeHearts.removeOne(heart);
        heart->deleteLater();
    });

    m_activeHearts.append(heart);

    moveAnim->start(QAbstractAnimation::DeleteWhenStopped);
    fadeAnim->start(QAbstractAnimation::DeleteWhenStopped);
}
