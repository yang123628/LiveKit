#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QRadioButton>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QButtonGroup>

class StartLivePage : public QWidget {
    Q_OBJECT

public:
    explicit StartLivePage(QWidget* parent = nullptr);

signals:
    void SIG_startLive(const QString& title, const QString& category, int mode);

private:
    void setupUI();

    QLineEdit* m_titleEdit;
    QComboBox* m_categoryCombo;
    QRadioButton* m_radioCamera;
    QRadioButton* m_radioDesktop;
    QRadioButton* m_radioPip;
    QPushButton* m_startButton;
    QButtonGroup* m_modeGroup;
};
