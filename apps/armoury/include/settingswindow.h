// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QLineEdit>
#include <QWidget>

class SettingsWindow : public QWidget
{
public:
    explicit SettingsWindow(QWidget *parent = nullptr);

private:
    void applySettings();

    QLineEdit *m_sourcesLineEdit = nullptr;
    QLineEdit *m_outputLineEdit = nullptr;
    QLineEdit *m_blenderPath = nullptr;
};