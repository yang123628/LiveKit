#include "ui/LoadingSpinner.h"
#include <QPainter>

LoadingSpinner::LoadingSpinner(QWidget* parent)
    : QWidget(parent)
    , m_rotation(0)
    , m_spinning(false)
{
    setFixedSize(36, 36);

    m_anim = new QPropertyAnimation(this, "rotation", this);
    m_anim->setDuration(1000);
    m_anim->setStartValue(0);
    m_anim->setEndValue(360);
    m_anim->setLoopCount(-1);
    m_anim->setEasingCurve(QEasingCurve::Linear);
}

LoadingSpinner::~LoadingSpinner() {
    stop();
}

void LoadingSpinner::start() {
    if (m_spinning) return;
    m_spinning = true;
    show();
    m_anim->start();
}

void LoadingSpinner::stop() {
    m_spinning = false;
    m_anim->stop();
    hide();
}

bool LoadingSpinner::isSpinning() const { return m_spinning; }

int LoadingSpinner::rotation() const { return m_rotation; }

void LoadingSpinner::setRotation(int angle) {
    m_rotation = angle;
    update();
}

void LoadingSpinner::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.translate(width() / 2.0, height() / 2.0);
    painter.rotate(m_rotation);

    QPen pen;
    pen.setWidth(3);
    pen.setCapStyle(Qt::RoundCap);

    int radius = 14;
    QRectF arcRect(-radius, -radius, radius * 2, radius * 2);

    pen.setColor(QColor(233, 69, 96, 60));
    painter.setPen(pen);
    painter.drawArc(arcRect, 0, 360 * 16);

    pen.setColor(QColor(233, 69, 96, 255));
    painter.setPen(pen);
    painter.drawArc(arcRect, 0, 120 * 16);
}
