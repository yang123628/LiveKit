#include "ui/LiveHallPage.h"
#include "ui/RoomCard.h"
#include "app/Application.h"
#include "network/IHttpClient.h"
#include "network/ApiResponse.h"
#include <QSvgRenderer>

LiveHallPage::LiveHallPage(QWidget* parent)
    : QWidget(parent)
    , m_selectedCategory(0)
{
    m_categories = QStringList{
        QStringLiteral("全部"),
        QStringLiteral("游戏"),
        QStringLiteral("聊天"),
        QStringLiteral("音乐"),
        QStringLiteral("其他")
    };
    setupUI();
    loadRooms();

    m_refreshTimer = new QTimer(this);
    m_refreshTimer->setInterval(30000);
    connect(m_refreshTimer, &QTimer::timeout, this, &LiveHallPage::loadRooms);
    m_refreshTimer->start();
}

void LiveHallPage::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    setupHeader();
    setupCategoryBar();
    setupRoomGrid();

    mainLayout->addWidget(m_header);
    mainLayout->addWidget(m_categoryBar);
    mainLayout->addWidget(m_scrollArea, 1);
}

void LiveHallPage::setupHeader() {
    m_header = new QWidget(this);
    m_header->setObjectName("hallHeader");
    m_header->setFixedHeight(56);

    m_headerLayout = new QHBoxLayout(m_header);
    m_headerLayout->setContentsMargins(24, 8, 24, 8);

    m_logoLabel = new QLabel(QStringLiteral("LiveKit"), m_header);
    m_logoLabel->setObjectName("hallLogo");

    m_headerLayout->addWidget(m_logoLabel);
    m_headerLayout->addStretch();
}

void LiveHallPage::setupCategoryBar() {
    m_categoryBar = new QWidget(this);
    m_categoryBar->setObjectName("hallCategoryBar");
    m_categoryBar->setFixedHeight(44);

    m_categoryLayout = new QHBoxLayout(m_categoryBar);
    m_categoryLayout->setContentsMargins(24, 4, 24, 4);
    m_categoryLayout->setSpacing(8);

    m_categoryGroup = new QButtonGroup(this);
    m_categoryGroup->setExclusive(true);

    for (int i = 0; i < m_categories.size(); ++i) {
        auto* btn = new QPushButton(m_categories[i], m_categoryBar);
        btn->setObjectName("categoryButton");
        btn->setCheckable(true);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFixedHeight(32);
        btn->setMinimumWidth(60);
        if (i == 0) btn->setChecked(true);
        m_categoryGroup->addButton(btn, i);
        m_categoryLayout->addWidget(btn);
    }

    m_categoryLayout->addStretch();

    connect(m_categoryGroup, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), [this](int id) {
        m_selectedCategory = id;
        m_currentCategory = (id == 0) ? "" : m_categories[id];
        loadRooms();
    });
}

void LiveHallPage::setupRoomGrid() {
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setObjectName("hallScrollArea");
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_scrollContent = new QWidget(m_scrollArea);
    m_scrollContent->setObjectName("hallScrollContent");

    m_gridLayout = new QGridLayout(m_scrollContent);
    m_gridLayout->setContentsMargins(24, 16, 24, 16);
    m_gridLayout->setSpacing(12);
    m_gridLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    m_emptyLabel = new QLabel(QStringLiteral("暂无直播"), m_scrollContent);
    m_emptyLabel->setObjectName("hallEmptyLabel");
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->hide();

    m_scrollArea->setWidget(m_scrollContent);
}

void LiveHallPage::loadRooms() {
    auto* client = Application::instance().httpClient();
    QString path = "/api/live/rooms";
    if (!m_currentCategory.isEmpty()) {
        path += QString("?category=%1").arg(m_currentCategory);
    }

    client->get(path, [this](const ApiResponse& resp) {
        if (resp.isSuccess()) {
            auto roomsArr = resp.data().value("rooms").toArray();
            auto rooms = RoomInfo::fromJsonArray(roomsArr);
            populateRoomCards(rooms);
        }
    });
}

void LiveHallPage::clearRoomCards() {
    for (auto* card : m_roomCards) {
        m_gridLayout->removeWidget(card);
        delete card;
    }
    m_roomCards.clear();
}

void LiveHallPage::populateRoomCards(const QVector<RoomInfo>& rooms) {
    clearRoomCards();

    if (rooms.isEmpty()) {
        m_emptyLabel->show();
        m_gridLayout->addWidget(m_emptyLabel, 0, 0, 1, 4);
        return;
    }

    m_emptyLabel->hide();

    int col = 0;
    int row = 0;
    const int maxCols = 4;

    for (const auto& room : rooms) {
        auto* card = new RoomCard(m_scrollContent);
        card->setRoomId(room.roomId());
        card->setAnchorAvatar(room.anchorAvatarId());
        card->setAnchorName(room.anchorName());
        card->setTitle(room.title());
        card->setViewerCount(room.viewerCount());
        card->setCategory(room.category());

        connect(card, &RoomCard::clicked, this, &LiveHallPage::roomClicked);

        m_gridLayout->addWidget(card, row, col);
        m_roomCards.append(card);

        col++;
        if (col >= maxCols) {
            col = 0;
            row++;
        }
    }
}

void LiveHallPage::refreshRooms() {
    loadRooms();
}

void LiveHallPage::setCategoryFilter(const QString& category) {
    m_currentCategory = category;
    loadRooms();
}
