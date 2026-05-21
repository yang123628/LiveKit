#include "ui/LiveHallPage.h"
#include "ui/RoomCard.h"
#include "ui/ReplayCard.h"
#include "app/Application.h"
#include "network/IHttpClient.h"
#include "network/ApiResponse.h"

LiveHallPage::LiveHallPage(QWidget* parent)
    : QWidget(parent)
    , m_currentMode(0)
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
    connect(m_refreshTimer, &QTimer::timeout, this, [this]() {
        if (m_currentMode == 0) {
            loadRooms();
        } else {
            loadReplays();
        }
    });
    m_refreshTimer->start();
}

void LiveHallPage::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    setupHeader();
    setupModeTabs();
    setupCategoryBar();
    setupRoomGrid();
    setupReplayGrid();

    m_contentStack = new QStackedWidget(this);
    m_contentStack->addWidget(m_roomScrollArea);
    m_contentStack->addWidget(m_replayScrollArea);

    mainLayout->addWidget(m_header);
    mainLayout->addWidget(m_liveTab->parentWidget());
    mainLayout->addWidget(m_categoryBar);
    mainLayout->addWidget(m_contentStack, 1);
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

void LiveHallPage::setupModeTabs() {
    auto* tabBar = new QWidget(this);
    tabBar->setObjectName("hallModeBar");
    tabBar->setFixedHeight(40);

    auto* tabLayout = new QHBoxLayout(tabBar);
    tabLayout->setContentsMargins(24, 4, 24, 4);
    tabLayout->setSpacing(4);

    m_modeGroup = new QButtonGroup(this);
    m_modeGroup->setExclusive(true);

    m_liveTab = new QPushButton(QStringLiteral("直播"), tabBar);
    m_liveTab->setObjectName("modeTabButton");
    m_liveTab->setCheckable(true);
    m_liveTab->setChecked(true);
    m_liveTab->setFixedHeight(30);
    m_liveTab->setMinimumWidth(60);
    m_liveTab->setCursor(Qt::PointingHandCursor);
    m_modeGroup->addButton(m_liveTab, 0);
    tabLayout->addWidget(m_liveTab);

    m_replayTab = new QPushButton(QStringLiteral("回放"), tabBar);
    m_replayTab->setObjectName("modeTabButton");
    m_replayTab->setCheckable(true);
    m_replayTab->setFixedHeight(30);
    m_replayTab->setMinimumWidth(60);
    m_replayTab->setCursor(Qt::PointingHandCursor);
    m_modeGroup->addButton(m_replayTab, 1);
    tabLayout->addWidget(m_replayTab);

    tabLayout->addStretch();

    connect(m_modeGroup, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked),
        this, &LiveHallPage::switchMode);
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
        if (m_currentMode == 0) {
            loadRooms();
        } else {
            loadReplays();
        }
    });
}

void LiveHallPage::setupRoomGrid() {
    m_roomScrollArea = new QScrollArea(this);
    m_roomScrollArea->setObjectName("hallScrollArea");
    m_roomScrollArea->setWidgetResizable(true);
    m_roomScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_roomScrollContent = new QWidget(m_roomScrollArea);
    m_roomScrollContent->setObjectName("hallScrollContent");

    m_roomGridLayout = new QGridLayout(m_roomScrollContent);
    m_roomGridLayout->setContentsMargins(24, 16, 24, 16);
    m_roomGridLayout->setSpacing(12);
    m_roomGridLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    m_roomEmptyLabel = new QLabel(QStringLiteral("暂无直播"), m_roomScrollContent);
    m_roomEmptyLabel->setObjectName("hallEmptyLabel");
    m_roomEmptyLabel->setAlignment(Qt::AlignCenter);
    m_roomEmptyLabel->hide();

    m_roomScrollArea->setWidget(m_roomScrollContent);
}

void LiveHallPage::setupReplayGrid() {
    m_replayScrollArea = new QScrollArea(this);
    m_replayScrollArea->setObjectName("hallScrollArea");
    m_replayScrollArea->setWidgetResizable(true);
    m_replayScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_replayScrollContent = new QWidget(m_replayScrollArea);
    m_replayScrollContent->setObjectName("hallScrollContent");

    m_replayGridLayout = new QGridLayout(m_replayScrollContent);
    m_replayGridLayout->setContentsMargins(24, 16, 24, 16);
    m_replayGridLayout->setSpacing(12);
    m_replayGridLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    m_replayEmptyLabel = new QLabel(QStringLiteral("暂无回放"), m_replayScrollContent);
    m_replayEmptyLabel->setObjectName("hallEmptyLabel");
    m_replayEmptyLabel->setAlignment(Qt::AlignCenter);
    m_replayEmptyLabel->hide();

    m_replayScrollArea->setWidget(m_replayScrollContent);
}

void LiveHallPage::switchMode(int mode) {
    m_currentMode = mode;
    if (mode == 0) {
        m_contentStack->setCurrentWidget(m_roomScrollArea);
        loadRooms();
    } else {
        m_contentStack->setCurrentWidget(m_replayScrollArea);
        loadReplays();
    }
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

void LiveHallPage::loadReplays() {
    auto* client = Application::instance().httpClient();
    client->get("/api/live/replays", [this](const ApiResponse& resp) {
        if (resp.isSuccess()) {
            auto replaysArr = resp.data().value("replays").toArray();
            auto replays = ReplayInfo::fromJsonArray(replaysArr);
            populateReplayCards(replays);
        }
    });
}

void LiveHallPage::clearRoomCards() {
    for (auto* card : m_roomCards) {
        m_roomGridLayout->removeWidget(card);
        delete card;
    }
    m_roomCards.clear();
}

void LiveHallPage::clearReplayCards() {
    for (auto* card : m_replayCards) {
        m_replayGridLayout->removeWidget(card);
        delete card;
    }
    m_replayCards.clear();
}

void LiveHallPage::populateRoomCards(const QVector<RoomInfo>& rooms) {
    clearRoomCards();

    if (rooms.isEmpty()) {
        m_roomEmptyLabel->show();
        m_roomGridLayout->addWidget(m_roomEmptyLabel, 0, 0, 1, 4);
        return;
    }

    m_roomEmptyLabel->hide();

    int col = 0;
    int row = 0;
    const int maxCols = 4;

    for (const auto& room : rooms) {
        auto* card = new RoomCard(m_roomScrollContent);
        card->setRoomId(room.roomId());
        card->setAnchorAvatar(room.anchorAvatarId());
        card->setAnchorName(room.anchorName());
        card->setTitle(room.title());
        card->setViewerCount(room.viewerCount());
        card->setCategory(room.category());

        connect(card, &RoomCard::clicked, this, &LiveHallPage::roomClicked);

        m_roomGridLayout->addWidget(card, row, col);
        m_roomCards.append(card);

        col++;
        if (col >= maxCols) {
            col = 0;
            row++;
        }
    }
}

void LiveHallPage::populateReplayCards(const QVector<ReplayInfo>& replays) {
    clearReplayCards();

    if (replays.isEmpty()) {
        m_replayEmptyLabel->show();
        m_replayGridLayout->addWidget(m_replayEmptyLabel, 0, 0, 1, 4);
        return;
    }

    m_replayEmptyLabel->hide();

    int col = 0;
    int row = 0;
    const int maxCols = 4;

    for (const auto& replay : replays) {
        auto* card = new ReplayCard(m_replayScrollContent);
        card->setReplayId(replay.replayId());
        card->setPlayUrl(replay.playUrl());
        card->setAnchorAvatar(replay.anchorAvatarId());
        card->setAnchorName(replay.anchorName());
        card->setTitle(replay.title());
        card->setDuration(replay.durationText());

        connect(card, &ReplayCard::clicked, this, [this](int replayId, const QString& playUrl) {
            Q_UNUSED(replayId)
            emit replayClicked(playUrl);
        });

        m_replayGridLayout->addWidget(card, row, col);
        m_replayCards.append(card);

        col++;
        if (col >= maxCols) {
            col = 0;
            row++;
        }
    }
}

void LiveHallPage::refreshRooms() {
    if (m_currentMode == 0) {
        loadRooms();
    } else {
        loadReplays();
    }
}

void LiveHallPage::setCategoryFilter(const QString& category) {
    m_currentCategory = category;
    if (m_currentMode == 0) {
        loadRooms();
    } else {
        loadReplays();
    }
}
