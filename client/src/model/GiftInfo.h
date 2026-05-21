#pragma once

#include <QVector>
#include <QString>

struct GiftInfo {
    int giftId;
    QString name;
    QString iconPath;

    static QVector<GiftInfo> allGifts();
};
