#include "ui/PicInPicWidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QHBoxLayout>

PicInPicWidget::PicInPicWidget(QWidget* parent)
    : QWidget(parent, Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint)
    , m_dragging(false)
{
    setFixedSize(320, 240);
    setAttribute(Qt::WA_TranslucentBackground, false);

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_closeButton = new QLabel(QStringLiteral("×"), this);
    m_closeButton->setObjectName("pipCloseButton");
    m_closeButton->setFixedSize(20, 20);
    m_closeButton->setAlignment(Qt::AlignCenter);
    m_closeButton->setStyleSheet(
        "QLabel#pipCloseButton { background: rgba(0,0,0,160); color: white; border-radius: 10px; font-size: 14px; font-weight: bold; }"
        "QLabel#pipCloseButton:hover { background: rgba(233,69,96,220); }");
    m_closeButton->installEventFilter(this);

    layout->addStretch();
    layout->addWidget(m_closeButton);
    layout->setAlignment(m_closeButton, Qt::AlignTop | Qt::AlignRight);
}

void PicInPicWidget::updateFrame(const QImage& frame) {
    m_frame = frame.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    update();
}

void PicInPicWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    if (!m_frame.isNull()) {
        painter.drawImage(rect(), m_frame);
    } else {
        painter.fillRect(rect(), QColor(30, 30, 30));
        painter.setPen(Qt::white);
        painter.drawText(rect(), Qt::AlignCenter, QStringLiteral("摄像头"));
    }

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 80));
    painter.drawRect(0, 0, width(), 3);
}

void PicInPicWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        QRect closeRect = m_closeButton->geometry();
        if (closeRect.contains(event->pos())) {
            close();
            return;
        }
        m_dragging = true;
        m_dragPos = event->globalPos() - frameGeometry().topLeft();
    }
}

void PicInPicWidget::mouseMoveEvent(QMouseEvent* event) {
    if (m_dragging) {
        move(event->globalPos() - m_dragPos);
    }
}

void PicInPicWidget::mouseReleaseEvent(QMouseEvent* event) {
    Q_UNUSED(event)
    m_dragging = false;
}

void PicInPicWidget::closeEvent(QCloseEvent* event) {
    QWidget::closeEvent(event);
}
