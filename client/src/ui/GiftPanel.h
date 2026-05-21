#pragma once

#include <QWidget>
#include <QGridLayout>
#include <QPushButton>
#include <QPropertyAnimation>
#include <QVector>
#include "model/GiftInfo.h"

class GiftPanel : public QWidget {
    Q_OBJECT

public:
    explicit GiftPanel(QWidget* parent = nullptr);

    void showPanel();
    void hidePanel();
    bool isPanelVisible() const;

signals:
    void SIG_giftSelected(int giftId);

protected:
    bool event(QEvent* event) override;

private:
    void setupUI();
    void createGiftButtons();

    QGridLayout* m_gridLayout;
    QWidget* m_panelContent;
    QPropertyAnimation* m_showAnimation;
    QPropertyAnimation* m_hideAnimation;
    bool m_visible;
    QVector<GiftInfo> m_gifts;
};
