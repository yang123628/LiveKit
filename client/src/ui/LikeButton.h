#pragma once

#include <QPushButton>
#include <QPropertyAnimation>

class LikeButton : public QPushButton {
    Q_OBJECT

public:
    explicit LikeButton(QWidget* parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    void animatePress();
    void animateRelease();

    QPropertyAnimation* m_pressAnim;
    QPropertyAnimation* m_releaseAnim;
};
