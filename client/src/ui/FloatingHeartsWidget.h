#pragma once

#include <QWidget>
#include <QLabel>
#include <QPropertyAnimation>
#include <QList>
#include <QTimer>

class FloatingHeartsWidget : public QWidget {
    Q_OBJECT

public:
    explicit FloatingHeartsWidget(QWidget* parent = nullptr);

    void addHeart();

private:
    void createHeart();

    QList<QLabel*> m_activeHearts;
    int m_maxHearts;
};
