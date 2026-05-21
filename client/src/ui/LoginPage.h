#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QMovie>

class LoginPage : public QWidget {
    Q_OBJECT

public:
    explicit LoginPage(QWidget* parent = nullptr);

signals:
    void loginSuccess();
    void switchToRegister();

private:
    void setupUI();
    void onLoginClicked();
    void setLoading(bool loading);

    QLineEdit* m_editUsername;
    QLineEdit* m_editPassword;
    QPushButton* m_btnLogin;
    QLabel* m_labelError;
    QLabel* m_labelLoading;
    bool m_loading;
};
