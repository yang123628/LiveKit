#include "ui/ProfilePage.h"
#include "app/Application.h"
#include "app/AppConfig.h"
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>

ProfilePage::ProfilePage(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    loadUserInfo();
}

void ProfilePage::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 30, 40, 30);
    mainLayout->setSpacing(20);

    auto* headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(20);

    m_avatarLabel = new QLabel(this);
    m_avatarLabel->setObjectName("profileAvatar");
    m_avatarLabel->setFixedSize(64, 64);
    headerLayout->addWidget(m_avatarLabel);

    auto* infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(4);

    m_usernameLabel = new QLabel(this);
    m_usernameLabel->setObjectName("profileUsername");

    m_userIdLabel = new QLabel(this);
    m_userIdLabel->setObjectName("profileUserId");

    infoLayout->addWidget(m_usernameLabel);
    infoLayout->addWidget(m_userIdLabel);
    infoLayout->addStretch();

    headerLayout->addLayout(infoLayout, 1);

    m_settingsButton = new QPushButton(QStringLiteral("设置"), this);
    m_settingsButton->setObjectName("profileSettingsButton");
    m_settingsButton->setFixedSize(80, 36);
    headerLayout->addWidget(m_settingsButton);

    mainLayout->addLayout(headerLayout);

    auto* divider1 = new QFrame(this);
    divider1->setObjectName("settingsDivider");
    divider1->setFixedHeight(1);
    mainLayout->addWidget(divider1);

    auto* historyTitle = new QLabel(QStringLiteral("直播历史"), this);
    historyTitle->setObjectName("formLabel");
    mainLayout->addWidget(historyTitle);

    m_historyList = new QListWidget(this);
    m_historyList->setObjectName("profileHistoryList");
    mainLayout->addWidget(m_historyList, 1);

    m_logoutButton = new QPushButton(QStringLiteral("退出登录"), this);
    m_logoutButton->setObjectName("logoutButton");
    m_logoutButton->setFixedSize(140, 40);
    mainLayout->addWidget(m_logoutButton, 0, Qt::AlignLeft);

    connect(m_settingsButton, &QPushButton::clicked, this, &ProfilePage::SIG_openSettings);
    connect(m_logoutButton, &QPushButton::clicked, this, &ProfilePage::SIG_logout);
}

void ProfilePage::loadUserInfo() {
    auto& user = Application::instance().currentUser();

    QString svgPath = user.avatarResourcePath();
    QSvgRenderer renderer(svgPath);
    QPixmap pixmap(64, 64);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    renderer.render(&painter);
    painter.end();

    QPixmap rounded(64, 64);
    rounded.fill(Qt::transparent);
    QPainter rp(&rounded);
    rp.setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    path.addEllipse(0, 0, 64, 64);
    rp.setClipPath(path);
    rp.drawPixmap(0, 0, 64, 64, pixmap);
    m_avatarLabel->setPixmap(rounded);

    m_usernameLabel->setText(user.username().isEmpty() ? QStringLiteral("未登录") : user.username());
    m_userIdLabel->setText(QStringLiteral("ID: %1").arg(user.userId()));
}

void ProfilePage::refreshProfile() {
    loadUserInfo();
}
