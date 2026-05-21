#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QLabel>

class DanmakuWidget : public QWidget {
    Q_OBJECT

public:
    explicit DanmakuWidget(QWidget* parent = nullptr);

    void addDanmaku(const QString& username, const QString& content);
    void clearDanmaku();

private:
    void setupUI();

    QListWidget* m_listWidget;
    int m_maxCount;
};
