#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QList>

class RegisterPage : public QWidget {
    Q_OBJECT

public:
    explicit RegisterPage(QWidget* parent = nullptr);

signals:
    void registerSuccess();
    void switchToLogin();

private:
    void setupUI();
    void onRegisterClicked();
    void setLoading(bool loading);

    QLineEdit* m_editUsername;
    QLineEdit* m_editPassword;
    QLineEdit* m_editConfirmPassword;
    QPushButton* m_btnRegister;
    QLabel* m_labelError;
    QLabel* m_labelLoading;
    QWidget* m_avatarGrid;
    QGridLayout* m_avatarLayout;
    QList<QPushButton*> m_avatarButtons;
    int m_selectedAvatarId;
    bool m_loading;
};
