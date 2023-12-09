#pragma once

#include <QLineEdit>
#include <QWidget>

class SettingsWindow : public QWidget
{
public:
    explicit SettingsWindow(QWidget *parent = nullptr);

private:
    void applySettings();

    QLineEdit *m_outputLineEdit = nullptr;
};