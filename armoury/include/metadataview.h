// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "filecache.h"
#include "gearview.h"

#include <QWidget>

struct GameData;

class MetadataView : public QWidget
{
    Q_OBJECT

public:
    explicit MetadataView(GameData *data, QWidget *parent = nullptr);

private:
    GameData *data = nullptr;
};