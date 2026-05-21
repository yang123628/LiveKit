#include "ui/DanmakuWidget.h"

DanmakuWidget::DanmakuWidget(QWidget* parent)
    : QWidget(parent)
    , m_maxCount(50)
{
    setupUI();
}

void DanmakuWidget::setupUI() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_listWidget = new QListWidget(this);
    m_listWidget->setObjectName("danmakuList");
    m_listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_listWidget->setSelectionMode(QAbstractItemView::NoSelection);
    m_listWidget->setFocusPolicy(Qt::NoFocus);
    m_listWidget->setFrameShape(QFrame::NoFrame);
    m_listWidget->setStyleSheet(
        "QListWidget#danmakuList { background: transparent; border: none; }"
        "QListWidget#danmakuList::item { background: transparent; padding: 2px 4px; }");

    layout->addWidget(m_listWidget);
}

void DanmakuWidget::addDanmaku(const QString& username, const QString& content) {
    while (m_listWidget->count() >= m_maxCount) {
        QListWidgetItem* item = m_listWidget->takeItem(0);
        delete item;
    }

    QString text = QString("<span style='color:#e94560;font-weight:bold;'>%1</span><span style='color:#cccccc;'>: %2</span>")
        .arg(username)
        .arg(content);

    auto* item = new QListWidgetItem(m_listWidget);
    auto* label = new QLabel(this);
    label->setObjectName("danmakuItem");
    label->setTextFormat(Qt::RichText);
    label->setText(text);
    label->setWordWrap(true);
    label->setStyleSheet(
        "QLabel#danmakuItem {"
        "  background: rgba(0, 0, 0, 140);"
        "  color: #cccccc;"
        "  border-radius: 8px;"
        "  padding: 4px 10px;"
        "  font-size: 13px;"
        "}");

    item->setSizeHint(label->sizeHint());
    m_listWidget->addItem(item);
    m_listWidget->setItemWidget(item, label);

    auto* opacityEffect = new QGraphicsOpacityEffect(label);
    opacityEffect->setOpacity(0.0);
    label->setGraphicsEffect(opacityEffect);

    auto* fadeIn = new QPropertyAnimation(opacityEffect, "opacity");
    fadeIn->setDuration(300);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    fadeIn->start(QAbstractAnimation::DeleteWhenStopped);

    m_listWidget->scrollToBottom();
}

void DanmakuWidget::clearDanmaku() {
    m_listWidget->clear();
}
