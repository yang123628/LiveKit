#include "ui/LoginPage.h"
#include "app/AppConfig.h"
#include "app/Application.h"
#include "network/HttpClient.h"
#include <QHBoxLayout>
#include <QSpacerItem>

LoginPage::LoginPage(QWidget* parent)
    : QWidget(parent)
    , m_loading(false)
{
    setupUI();
}

void LoginPage::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignCenter);

    auto* container = new QWidget(this);
    container->setObjectName("authContainer");
    container->setFixedWidth(400);
    auto* containerLayout = new QVBoxLayout(container);
    containerLayout->setContentsMargins(40, 40, 40, 40);
    containerLayout->setSpacing(16);

    auto* logoLabel = new QLabel(QStringLiteral("LiveKit"), container);
    logoLabel->setObjectName("authLogo");
    logoLabel->setAlignment(Qt::AlignCenter);
    containerLayout->addWidget(logoLabel);

    auto* subtitleLabel = new QLabel(QStringLiteral("欢迎回来，请登录你的账号"), container);
    subtitleLabel->setObjectName("authSubtitle");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    containerLayout->addWidget(subtitleLabel);

    containerLayout->addSpacing(20);

    m_editUsername = new QLineEdit(container);
    m_editUsername->setPlaceholderText(QStringLiteral("用户名"));
    m_editUsername->setObjectName("authInput");
    m_editUsername->setFixedHeight(44);
    containerLayout->addWidget(m_editUsername);

    m_editPassword = new QLineEdit(container);
    m_editPassword->setPlaceholderText(QStringLiteral("密码"));
    m_editPassword->setObjectName("authInput");
    m_editPassword->setEchoMode(QLineEdit::Password);
    m_editPassword->setFixedHeight(44);
    containerLayout->addWidget(m_editPassword);

    m_labelError = new QLabel(container);
    m_labelError->setObjectName("authError");
    m_labelError->setAlignment(Qt::AlignCenter);
    m_labelError->setWordWrap(true);
    m_labelError->hide();
    containerLayout->addWidget(m_labelError);

    m_btnLogin = new QPushButton(QStringLiteral("登 录"), container);
    m_btnLogin->setObjectName("authButton");
    m_btnLogin->setFixedHeight(44);
    m_btnLogin->setCursor(Qt::PointingHandCursor);
    containerLayout->addWidget(m_btnLogin);

    m_labelLoading = new QLabel(container);
    m_labelLoading->setObjectName("authLoading");
    m_labelLoading->setAlignment(Qt::AlignCenter);
    m_labelLoading->setText(QStringLiteral("登录中..."));
    m_labelLoading->hide();
    containerLayout->addWidget(m_labelLoading);

    containerLayout->addSpacing(10);

    auto* switchLayout = new QHBoxLayout();
    switchLayout->setAlignment(Qt::AlignCenter);
    auto* switchLabel = new QLabel(QStringLiteral("没有账号？"), container);
    switchLabel->setObjectName("authSwitchText");
    auto* switchBtn = new QPushButton(QStringLiteral("去注册"), container);
    switchBtn->setObjectName("authSwitchButton");
    switchBtn->setCursor(Qt::PointingHandCursor);
    switchBtn->setFlat(true);
    switchLayout->addWidget(switchLabel);
    switchLayout->addWidget(switchBtn);
    containerLayout->addLayout(switchLayout);

    mainLayout->addWidget(container);

    connect(m_btnLogin, &QPushButton::clicked, this, &LoginPage::onLoginClicked);
    connect(switchBtn, &QPushButton::clicked, this, &LoginPage::switchToRegister);
    connect(m_editPassword, &QLineEdit::returnPressed, this, &LoginPage::onLoginClicked);
}

void LoginPage::onLoginClicked() {
    if (m_loading) return;

    auto username = m_editUsername->text().trimmed();
    auto password = m_editPassword->text();

    m_labelError->hide();

    if (username.isEmpty() || password.isEmpty()) {
        m_labelError->setText(QStringLiteral("请输入用户名和密码"));
        m_labelError->show();
        return;
    }

    setLoading(true);

    auto* client = Application::instance().httpClient();
    QJsonObject body;
    body["username"] = username;
    body["password"] = password;

    client->post("/api/login", body, [this](const ApiResponse& resp) {
        setLoading(false);

        if (resp.isSuccess()) {
            auto data = resp.data();
            auto token = data.value("token").toString();
            auto userInfo = data.value("user_info").toObject();

            auto& user = Application::instance().currentUser();
            user.fromJson(userInfo);
            user.setToken(token);

            auto& config = AppConfig::instance();
            config.setToken(token);
            config.save();

            emit loginSuccess();
        } else {
            m_labelError->setText(resp.errorMessage());
            m_labelError->show();
        }
    });
}

void LoginPage::setLoading(bool loading) {
    m_loading = loading;
    m_btnLogin->setEnabled(!loading);
    m_editUsername->setEnabled(!loading);
    m_editPassword->setEnabled(!loading);
    m_labelLoading->setVisible(loading);

    if (loading) {
        m_btnLogin->setText(QStringLiteral("登录中..."));
    } else {
        m_btnLogin->setText(QStringLiteral("登 录"));
    }
}
