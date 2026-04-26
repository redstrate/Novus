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
};
