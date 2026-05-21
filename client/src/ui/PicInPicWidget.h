#pragma once

#include <QWidget>
#include <QLabel>
#include <QImage>
#include <QPoint>

class PicInPicWidget : public QWidget {
    Q_OBJECT

public:
    explicit PicInPicWidget(QWidget* parent = nullptr);
    ~PicInPicWidget() = default;

    void updateFrame(const QImage& frame);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private:
    QLabel* m_closeButton;
    QImage m_frame;
    bool m_dragging;
    QPoint m_dragPos;
};
