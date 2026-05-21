#include "model/GiftInfo.h"

QVector<GiftInfo> GiftInfo::allGifts() {
    QVector<GiftInfo> gifts;

    GiftInfo g1;
    g1.giftId = 1;
    g1.name = QStringLiteral("小花");
    g1.iconPath = ":/gifts/gift_1.svg";
    gifts.append(g1);

    GiftInfo g2;
    g2.giftId = 2;
    g2.name = QStringLiteral("鼓掌");
    g2.iconPath = ":/gifts/gift_2.svg";
    gifts.append(g2);

    GiftInfo g3;
    g3.giftId = 3;
    g3.name = QStringLiteral("比心");
    g3.iconPath = ":/gifts/gift_3.svg";
    gifts.append(g3);

    GiftInfo g4;
    g4.giftId = 4;
    g4.name = QStringLiteral("火箭");
    g4.iconPath = ":/gifts/gift_4.svg";
    gifts.append(g4);

    GiftInfo g5;
    g5.giftId = 5;
    g5.name = QStringLiteral("皇冠");
    g5.iconPath = ":/gifts/gift_5.svg";
    gifts.append(g5);

    GiftInfo g6;
    g6.giftId = 6;
    g6.name = QStringLiteral("烟花");
    g6.iconPath = ":/gifts/gift_6.svg";
    gifts.append(g6);

    return gifts;
}
