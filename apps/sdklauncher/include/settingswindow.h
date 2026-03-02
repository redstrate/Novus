// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "enumedit.h"
#include "settings.h"

#include <QLineEdit>
#include <QWidget>

class QPushButton;
class QListWidget;

class SettingsWindow : public QWidget
{
public:
    explicit SettingsWindow(QWidget *parent = nullptr);

private:
    void applySettings();
    void refreshList();
    void refreshConfigureWidget(QUuid uuid);
    void addInstall();
    void removeInstall();

    GameInstall &currentInstall();

    QListWidget *m_installWidget;
    QLineEdit *m_labelEdit;
    QLineEdit *m_pathEdit;
    EnumEdit<Language> *m_languageEdit;
    QList<GameInstall> m_installs;
    QPushButton *m_addButton;
    QPushButton *m_removeButton;
};
