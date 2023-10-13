// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QTreeView>
#include <QWidget>
#include <physis.hpp>

#include "gearview.h"

class GearListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GearListWidget(GameData *data, QWidget *parent = nullptr);

Q_SIGNALS:
    void gearSelected(const GearInfo &gear);

private:
    QTreeView *listWidget = nullptr;

    GameData *data = nullptr;
};
