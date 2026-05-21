#pragma once

#include <QWidget>
#include <QTimer>
#include <QPropertyAnimation>

class LoadingSpinner : public QWidget {
    Q_OBJECT
    Q_PROPERTY(int rotation READ rotation WRITE setRotation)

public:
    explicit LoadingSpinner(QWidget* parent = nullptr);
    ~LoadingSpinner();

    void start();
    void stop();
    bool isSpinning() const;

    int rotation() const;
    void setRotation(int angle);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    int m_rotation;
    bool m_spinning;
    QTimer* m_timer;
    QPropertyAnimation* m_anim;
};
