#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>

class ToggleSwitch;

class SettingsPage : public QWidget {
    Q_OBJECT

public:
    explicit SettingsPage(QWidget* parent = nullptr);

signals:
    void SIG_themeChanged(const QString& themeName);
    void SIG_serverChanged(const QString& address);

private:
    void setupUI();
    void loadSettings();
    void saveSettings();

    QLineEdit* m_serverInput;
    QComboBox* m_qualityCombo;
    ToggleSwitch* m_themeToggle;
    QPushButton* m_saveButton;
    QLabel* m_aboutLabel;
};

class ToggleSwitch : public QWidget {
    Q_OBJECT
    Q_PROPERTY(bool checked READ isChecked WRITE setChecked NOTIFY toggled)

public:
    explicit ToggleSwitch(QWidget* parent = nullptr);

    bool isChecked() const;
    void setChecked(bool checked);

signals:
    void toggled(bool checked);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    bool m_checked;
    int m_handleX;
};
