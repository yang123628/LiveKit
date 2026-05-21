#include "ui/LikeButton.h"
#include <QMouseEvent>

LikeButton::LikeButton(QWidget* parent)
    : QPushButton(parent)
    , m_pressAnim(nullptr)
    , m_releaseAnim(nullptr)
{
    setObjectName("likeButton");
    setFixedSize(44, 44);
    setText(QStringLiteral("♥"));
    setToolTip(QStringLiteral("点赞"));
}

void LikeButton::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        animatePress();
    }
    QPushButton::mousePressEvent(event);
}

void LikeButton::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        animateRelease();
    }
    QPushButton::mouseReleaseEvent(event);
}

void LikeButton::animatePress() {
    if (m_pressAnim) {
        delete m_pressAnim;
    }

    m_pressAnim = new QPropertyAnimation(this, "geometry");
    m_pressAnim->setDuration(100);
    QRect geo = geometry();
    int shrink = 4;
    m_pressAnim->setStartValue(geo);
    m_pressAnim->setEndValue(QRect(geo.x() + shrink, geo.y() + shrink,
        geo.width() - shrink * 2, geo.height() - shrink * 2));
    m_pressAnim->setEasingCurve(QEasingCurve::InQuad);
    m_pressAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

void LikeButton::animateRelease() {
    if (m_releaseAnim) {
        delete m_releaseAnim;
    }

    m_releaseAnim = new QPropertyAnimation(this, "geometry");
    m_releaseAnim->setDuration(150);
    QRect geo = geometry();
    int expand = 4;
    m_releaseAnim->setStartValue(geo);
    m_releaseAnim->setEndValue(QRect(geo.x() - expand, geo.y() - expand,
        geo.width() + expand * 2, geo.height() + expand * 2));
    m_releaseAnim->setEasingCurve(QEasingCurve::OutBack);
    m_releaseAnim->start(QAbstractAnimation::DeleteWhenStopped);
}
