// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QTabWidget>
#include <QWidget>

struct GameData;

class EXDPart : public QWidget {
public:
    explicit EXDPart(GameData* data);

    void loadSheet(const QString& name);

private:
    GameData* data = nullptr;

    QTabWidget* pageTabWidget = nullptr;
};