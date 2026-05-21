#include "ui/RegisterPage.h"
#include "app/AppConfig.h"
#include "app/Application.h"
#include "network/MockHttpClient.h"
#include <QHBoxLayout>
#include <QPixmap>
#include <QPainter>
#include <QSvgRenderer>

RegisterPage::RegisterPage(QWidget* parent)
    : QWidget(parent)
    , m_selectedAvatarId(1)
    , m_loading(false)
{
    setupUI();
}

void RegisterPage::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignCenter);

    auto* container = new QWidget(this);
    container->setObjectName("authContainer");
    container->setFixedWidth(400);
    auto* containerLayout = new QVBoxLayout(container);
    containerLayout->setContentsMargins(40, 36, 40, 36);
    containerLayout->setSpacing(12);

    auto* titleLabel = new QLabel(QStringLiteral("创建账号"), container);
    titleLabel->setObjectName("authLogo");
    titleLabel->setAlignment(Qt::AlignCenter);
    containerLayout->addWidget(titleLabel);

    auto* subtitleLabel = new QLabel(QStringLiteral("注册一个新账号开始使用"), container);
    subtitleLabel->setObjectName("authSubtitle");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    containerLayout->addWidget(subtitleLabel);

    containerLayout->addSpacing(12);

    m_editUsername = new QLineEdit(container);
    m_editUsername->setPlaceholderText(QStringLiteral("用户名 (3-20字符)"));
    m_editUsername->setObjectName("authInput");
    m_editUsername->setFixedHeight(44);
    containerLayout->addWidget(m_editUsername);

    m_editPassword = new QLineEdit(container);
    m_editPassword->setPlaceholderText(QStringLiteral("密码 (6-20字符)"));
    m_editPassword->setObjectName("authInput");
    m_editPassword->setEchoMode(QLineEdit::Password);
    m_editPassword->setFixedHeight(44);
    containerLayout->addWidget(m_editPassword);

    m_editConfirmPassword = new QLineEdit(container);
    m_editConfirmPassword->setPlaceholderText(QStringLiteral("确认密码"));
    m_editConfirmPassword->setObjectName("authInput");
    m_editConfirmPassword->setEchoMode(QLineEdit::Password);
    m_editConfirmPassword->setFixedHeight(44);
    containerLayout->addWidget(m_editConfirmPassword);

    auto* avatarTitle = new QLabel(QStringLiteral("选择头像"), container);
    avatarTitle->setObjectName("authAvatarTitle");
    containerLayout->addWidget(avatarTitle);

    m_avatarGrid = new QWidget(container);
    m_avatarGrid->setObjectName("avatarGrid");
    m_avatarLayout = new QGridLayout(m_avatarGrid);
    m_avatarLayout->setSpacing(10);
    m_avatarLayout->setContentsMargins(0, 0, 0, 0);

    for (int i = 1; i <= 8; ++i) {
        auto* btn = new QPushButton(m_avatarGrid);
        btn->setObjectName("avatarButton");
        btn->setFixedSize(64, 64);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setProperty("avatarId", i);

        QString svgPath = QString(":/avatars/avatar_%1.svg").arg(i);
        QSvgRenderer renderer(svgPath);
        QPixmap pixmap(48, 48);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        renderer.render(&painter);
        painter.end();
        btn->setIcon(QIcon(pixmap));
        btn->setIconSize(QSize(48, 48));

        if (i == 1) {
            btn->setChecked(true);
        }

        int row = (i - 1) / 4;
        int col = (i - 1) % 4;
        m_avatarLayout->addWidget(btn, row, col);
        m_avatarButtons.append(btn);

        connect(btn, &QPushButton::clicked, [this, i]() {
            m_selectedAvatarId = i;
            for (int j = 0; j < m_avatarButtons.size(); ++j) {
                m_avatarButtons[j]->setChecked((j + 1) == i);
            }
        });
    }
    containerLayout->addWidget(m_avatarGrid);

    m_labelError = new QLabel(container);
    m_labelError->setObjectName("authError");
    m_labelError->setAlignment(Qt::AlignCenter);
    m_labelError->setWordWrap(true);
    m_labelError->hide();
    containerLayout->addWidget(m_labelError);

    m_btnRegister = new QPushButton(QStringLiteral("注 册"), container);
    m_btnRegister->setObjectName("authButton");
    m_btnRegister->setFixedHeight(44);
    m_btnRegister->setCursor(Qt::PointingHandCursor);
    containerLayout->addWidget(m_btnRegister);

    m_labelLoading = new QLabel(container);
    m_labelLoading->setObjectName("authLoading");
    m_labelLoading->setAlignment(Qt::AlignCenter);
    m_labelLoading->setText(QStringLiteral("注册中..."));
    m_labelLoading->hide();
    containerLayout->addWidget(m_labelLoading);

    containerLayout->addSpacing(6);

    auto* switchLayout = new QHBoxLayout();
    switchLayout->setAlignment(Qt::AlignCenter);
    auto* switchLabel = new QLabel(QStringLiteral("已有账号？"), container);
    switchLabel->setObjectName("authSwitchText");
    auto* switchBtn = new QPushButton(QStringLiteral("去登录"), container);
    switchBtn->setObjectName("authSwitchButton");
    switchBtn->setCursor(Qt::PointingHandCursor);
    switchBtn->setFlat(true);
    switchLayout->addWidget(switchLabel);
    switchLayout->addWidget(switchBtn);
    containerLayout->addLayout(switchLayout);

    mainLayout->addWidget(container);

    connect(m_btnRegister, &QPushButton::clicked, this, &RegisterPage::onRegisterClicked);
    connect(switchBtn, &QPushButton::clicked, this, &RegisterPage::switchToLogin);
}

void RegisterPage::onRegisterClicked() {
    if (m_loading) return;

    auto username = m_editUsername->text().trimmed();
    auto password = m_editPassword->text();
    auto confirm = m_editConfirmPassword->text();

    m_labelError->hide();

    if (username.isEmpty() || password.isEmpty() || confirm.isEmpty()) {
        m_labelError->setText(QStringLiteral("请填写所有字段"));
        m_labelError->show();
        return;
    }

    if (username.length() < 3 || username.length() > 20) {
        m_labelError->setText(QStringLiteral("用户名需3-20个字符"));
        m_labelError->show();
        return;
    }

    if (password.length() < 6 || password.length() > 20) {
        m_labelError->setText(QStringLiteral("密码需6-20个字符"));
        m_labelError->show();
        return;
    }

    if (password != confirm) {
        m_labelError->setText(QStringLiteral("两次密码输入不一致"));
        m_labelError->show();
        return;
    }

    setLoading(true);

    auto* client = new MockHttpClient(this);
    QJsonObject body;
    body["username"] = username;
    body["password"] = password;
    body["avatar_id"] = m_selectedAvatarId;

    client->post("/api/register", body, [this, client](const ApiResponse& resp) {
        setLoading(false);
        client->deleteLater();

        if (resp.isSuccess()) {
            auto data = resp.data();
            auto token = data.value("token").toString();
            auto userInfo = data.value("user_info").toObject();

            auto& app = Application::instance();
            app.currentUser().fromJson(userInfo);
            app.currentUser().setToken(token);

            auto& config = AppConfig::instance();
            config.setToken(token);
            config.save();

            emit registerSuccess();
        } else {
            m_labelError->setText(resp.errorMessage());
            m_labelError->show();
        }
    });
}

void RegisterPage::setLoading(bool loading) {
    m_loading = loading;
    m_btnRegister->setEnabled(!loading);
    m_editUsername->setEnabled(!loading);
    m_editPassword->setEnabled(!loading);
    m_editConfirmPassword->setEnabled(!loading);
    m_labelLoading->setVisible(loading);

    if (loading) {
        m_btnRegister->setText(QStringLiteral("注册中..."));
    } else {
        m_btnRegister->setText(QStringLiteral("注 册"));
    }
}
