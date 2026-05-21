#include "ui/SettingsPage.h"
#include "app/AppConfig.h"
#include "app/Application.h"
#include "network/IHttpClient.h"
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QApplication>

ToggleSwitch::ToggleSwitch(QWidget* parent)
    : QWidget(parent)
    , m_checked(false)
    , m_handleX(4)
{
    setFixedSize(48, 26);
    setCursor(Qt::PointingHandCursor);
}

bool ToggleSwitch::isChecked() const { return m_checked; }

void ToggleSwitch::setChecked(bool checked) {
    if (m_checked != checked) {
        m_checked = checked;
        m_handleX = m_checked ? width() - 22 : 4;
        update();
        emit toggled(m_checked);
    }
}

void ToggleSwitch::mousePressEvent(QMouseEvent* event) {
    Q_UNUSED(event);
    setChecked(!m_checked);
}

void ToggleSwitch::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QColor trackColor = m_checked ? QColor("#e94560") : QColor("#3a3a4a");
    painter.setBrush(trackColor);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(0, 0, width(), height(), height() / 2, height() / 2);

    QColor handleColor = Qt::white;
    painter.setBrush(handleColor);
    int handleSize = height() - 8;
    m_handleX = m_checked ? width() - handleSize - 4 : 4;
    painter.drawEllipse(m_handleX, 4, handleSize, handleSize);
}

SettingsPage::SettingsPage(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    loadSettings();
}

void SettingsPage::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 30, 40, 30);
    mainLayout->setSpacing(20);

    auto* titleLabel = new QLabel(QStringLiteral("设置"), this);
    titleLabel->setObjectName("pageTitle");
    mainLayout->addWidget(titleLabel);

    auto* contentWidget = new QWidget(this);
    contentWidget->setObjectName("settingsContent");
    auto* contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(24, 24, 24, 24);
    contentLayout->setSpacing(24);

    auto* serverLayout = new QHBoxLayout();
    auto* serverLabel = new QLabel(QStringLiteral("服务器地址"), this);
    serverLabel->setObjectName("formLabel");
    serverLabel->setFixedWidth(100);
    m_serverInput = new QLineEdit(this);
    m_serverInput->setObjectName("settingsInput");
    m_serverInput->setPlaceholderText(QStringLiteral("例如: 192.168.124.128:8080"));
    serverLayout->addWidget(serverLabel);
    serverLayout->addWidget(m_serverInput, 1);
    contentLayout->addLayout(serverLayout);

    auto* qualityLayout = new QHBoxLayout();
    auto* qualityLabel = new QLabel(QStringLiteral("推流画质"), this);
    qualityLabel->setObjectName("formLabel");
    qualityLabel->setFixedWidth(100);
    m_qualityCombo = new QComboBox(this);
    m_qualityCombo->setObjectName("settingsCombo");
    m_qualityCombo->addItem(QStringLiteral("高清 (400kbps)"), 400);
    m_qualityCombo->addItem(QStringLiteral("标准 (200kbps)"), 200);
    m_qualityCombo->addItem(QStringLiteral("流畅 (100kbps)"), 100);
    qualityLayout->addWidget(qualityLabel);
    qualityLayout->addWidget(m_qualityCombo, 1);
    contentLayout->addLayout(qualityLayout);

    auto* themeLayout = new QHBoxLayout();
    auto* themeLabel = new QLabel(QStringLiteral("深色主题"), this);
    themeLabel->setObjectName("formLabel");
    themeLabel->setFixedWidth(100);
    m_themeToggle = new ToggleSwitch(this);
    themeLayout->addWidget(themeLabel);
    themeLayout->addStretch();
    contentLayout->addLayout(themeLayout);

    contentLayout->addSpacing(16);

    m_saveButton = new QPushButton(QStringLiteral("保存设置"), this);
    m_saveButton->setObjectName("settingsSaveButton");
    m_saveButton->setFixedSize(160, 40);
    contentLayout->addWidget(m_saveButton, 0, Qt::AlignLeft);

    contentLayout->addSpacing(24);

    auto* divider = new QFrame(this);
    divider->setObjectName("settingsDivider");
    divider->setFixedHeight(1);
    contentLayout->addWidget(divider);

    contentLayout->addSpacing(8);

    m_aboutLabel = new QLabel(this);
    m_aboutLabel->setObjectName("aboutLabel");
    QString aboutText = QStringLiteral(
        "<b>LiveKit</b><br>"
        "版本: %1<br>"
        "一款类抖音风格的桌面直播应用<br>"
        "GitHub: github.com/vskelin/LiveKit"
    ).arg(QApplication::applicationVersion());
    m_aboutLabel->setText(aboutText);
    m_aboutLabel->setWordWrap(true);
    contentLayout->addWidget(m_aboutLabel);

    mainLayout->addWidget(contentWidget);
    mainLayout->addStretch();

    connect(m_themeToggle, &ToggleSwitch::toggled, this, [this](bool dark) {
        QString theme = dark ? "dark" : "light";
        emit SIG_themeChanged(theme);
    });

    connect(m_saveButton, &QPushButton::clicked, this, &SettingsPage::saveSettings);
}

void SettingsPage::loadSettings() {
    auto& config = AppConfig::instance();
    m_serverInput->setText(config.serverAddress());
    m_themeToggle->setChecked(config.themeName() == "dark");

    int quality = config.settings().value("stream/quality", 200).toInt();
    for (int i = 0; i < m_qualityCombo->count(); ++i) {
        if (m_qualityCombo->itemData(i).toInt() == quality) {
            m_qualityCombo->setCurrentIndex(i);
            break;
        }
    }
}

void SettingsPage::saveSettings() {
    auto& config = AppConfig::instance();
    config.setServerAddress(m_serverInput->text());
    config.settings().setValue("stream/quality", m_qualityCombo->currentData());
    config.save();

    if (Application::instance().httpClient()) {
        Application::instance().httpClient()->setBaseUrl(m_serverInput->text());
    }

    emit SIG_serverChanged(m_serverInput->text());
}
