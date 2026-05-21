#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QSvgRenderer>

class ProfilePage : public QWidget {
    Q_OBJECT

public:
    explicit ProfilePage(QWidget* parent = nullptr);
    void refreshProfile();

signals:
    void SIG_logout();
    void SIG_openSettings();

private:
    void setupUI();
    void loadUserInfo();

    QLabel* m_avatarLabel;
    QLabel* m_usernameLabel;
    QLabel* m_userIdLabel;
    QListWidget* m_historyList;
    QPushButton* m_settingsButton;
    QPushButton* m_logoutButton;
};
