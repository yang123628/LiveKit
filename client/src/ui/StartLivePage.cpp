#include "ui/StartLivePage.h"

StartLivePage::StartLivePage(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

void StartLivePage::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 30, 40, 30);
    mainLayout->setSpacing(20);

    auto* titleLabel = new QLabel(QStringLiteral("开播设置"), this);
    titleLabel->setObjectName("pageTitle");
    mainLayout->addWidget(titleLabel);

    auto* formWidget = new QWidget(this);
    formWidget->setObjectName("startLiveForm");
    auto* formLayout = new QVBoxLayout(formWidget);
    formLayout->setSpacing(16);

    auto* titleLabel2 = new QLabel(QStringLiteral("直播标题"), this);
    titleLabel2->setObjectName("formLabel");
    formLayout->addWidget(titleLabel2);

    m_titleEdit = new QLineEdit(this);
    m_titleEdit->setObjectName("liveInput");
    m_titleEdit->setPlaceholderText(QStringLiteral("请输入直播标题（2-30字符）"));
    m_titleEdit->setMaxLength(30);
    formLayout->addWidget(m_titleEdit);

    auto* categoryLabel = new QLabel(QStringLiteral("直播分类"), this);
    categoryLabel->setObjectName("formLabel");
    formLayout->addWidget(categoryLabel);

    m_categoryCombo = new QComboBox(this);
    m_categoryCombo->setObjectName("liveCombo");
    m_categoryCombo->addItem(QStringLiteral("游戏"), QStringLiteral("游戏"));
    m_categoryCombo->addItem(QStringLiteral("聊天"), QStringLiteral("聊天"));
    m_categoryCombo->addItem(QStringLiteral("音乐"), QStringLiteral("音乐"));
    m_categoryCombo->addItem(QStringLiteral("其他"), QStringLiteral("其他"));
    formLayout->addWidget(m_categoryCombo);

    auto* modeLabel = new QLabel(QStringLiteral("直播模式"), this);
    modeLabel->setObjectName("formLabel");
    formLayout->addWidget(modeLabel);

    auto* modeLayout = new QHBoxLayout();
    modeLayout->setSpacing(12);

    m_modeGroup = new QButtonGroup(this);

    m_radioCamera = new QRadioButton(QStringLiteral("摄像头直播"), this);
    m_radioCamera->setObjectName("liveRadio");
    m_radioCamera->setChecked(true);

    m_radioDesktop = new QRadioButton(QStringLiteral("桌面直播"), this);
    m_radioDesktop->setObjectName("liveRadio");

    m_radioPip = new QRadioButton(QStringLiteral("桌面+画中画"), this);
    m_radioPip->setObjectName("liveRadio");

    m_modeGroup->addButton(m_radioCamera, 0);
    m_modeGroup->addButton(m_radioDesktop, 1);
    m_modeGroup->addButton(m_radioPip, 2);

    modeLayout->addWidget(m_radioCamera);
    modeLayout->addWidget(m_radioDesktop);
    modeLayout->addWidget(m_radioPip);
    modeLayout->addStretch();

    formLayout->addLayout(modeLayout);
    formLayout->addStretch();

    mainLayout->addWidget(formWidget, 1);

    m_startButton = new QPushButton(QStringLiteral("开始直播"), this);
    m_startButton->setObjectName("startLiveButton");
    m_startButton->setFixedHeight(48);
    mainLayout->addWidget(m_startButton);

    connect(m_startButton, &QPushButton::clicked, [this]() {
        QString title = m_titleEdit->text().trimmed();
        if (title.length() < 2) {
            m_titleEdit->setFocus();
            return;
        }
        QString category = m_categoryCombo->currentData().toString();
        int mode = m_modeGroup->checkedId();
        emit SIG_startLive(title, category, mode);
    });
}
