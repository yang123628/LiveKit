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

#include "model/RoomInfo.h"

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

private:
    void setupUI();
    void setupHeader();
    void setupCategoryBar();
    void setupRoomGrid();
    void loadRooms();
    void clearRoomCards();
    void populateRoomCards(const QVector<RoomInfo>& rooms);

    QWidget* m_header;
    QLabel* m_logoLabel;
    QHBoxLayout* m_headerLayout;

    QWidget* m_categoryBar;
    QHBoxLayout* m_categoryLayout;
    QButtonGroup* m_categoryGroup;
    QStringList m_categories;
    int m_selectedCategory;

    QScrollArea* m_scrollArea;
    QWidget* m_scrollContent;
    QGridLayout* m_gridLayout;
    QList<RoomCard*> m_roomCards;

    QLabel* m_emptyLabel;

    QTimer* m_refreshTimer;
    QString m_currentCategory;
};
