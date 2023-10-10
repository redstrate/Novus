// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QMdiSubWindow>
#include <physis.hpp>

class FilePropertiesWindow : public QWidget
{
    Q_OBJECT
public:
    explicit FilePropertiesWindow(GameData *data, QString path, QWidget *parent = nullptr);

private:
    GameData *data = nullptr;
};