#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QGridLayout>
#include <QTimer>
#include <QVector>
#include <QButtonGroup>
#include <QStackedWidget>

#include "model/RoomInfo.h"
#include "model/ReplayInfo.h"

class RoomCard;

class LiveHallPage : public QWidget {
    Q_OBJECT

public:
    explicit LiveHallPage(QWidget* parent = nullptr);
    ~LiveHallPage() = default;

    void refreshRooms();
    void setCategoryFilter(const QString& category);

signals:
    void roomClicked(int roomId);
    void replayClicked(const QString& playUrl);

private:
    void setupUI();
    void setupHeader();
    void setupModeTabs();
    void setupCategoryBar();
    void setupRoomGrid();
    void setupReplayGrid();
    void loadRooms();
    void loadReplays();
    void clearRoomCards();
    void clearReplayCards();
    void populateRoomCards(const QVector<RoomInfo>& rooms);
    void populateReplayCards(const QVector<ReplayInfo>& replays);
    void switchMode(int mode);

    QWidget* m_header;
    QLabel* m_logoLabel;
    QHBoxLayout* m_headerLayout;

    QPushButton* m_liveTab;
    QPushButton* m_replayTab;
    QButtonGroup* m_modeGroup;
    int m_currentMode;

    QWidget* m_categoryBar;
    QHBoxLayout* m_categoryLayout;
    QButtonGroup* m_categoryGroup;
    QStringList m_categories;
    int m_selectedCategory;

    QStackedWidget* m_contentStack;

    QScrollArea* m_roomScrollArea;
    QWidget* m_roomScrollContent;
    QGridLayout* m_roomGridLayout;
    QList<RoomCard*> m_roomCards;
    QLabel* m_roomEmptyLabel;

    QScrollArea* m_replayScrollArea;
    QWidget* m_replayScrollContent;
    QGridLayout* m_replayGridLayout;
    QList<QWidget*> m_replayCards;
    QLabel* m_replayEmptyLabel;

    QTimer* m_refreshTimer;
    QString m_currentCategory;
};
