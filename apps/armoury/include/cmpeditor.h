// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "cmppart.h"

class CmpEditor : public CmpPart
{
    Q_OBJECT

public:
    explicit CmpEditor(GameData *data, QWidget *parent = nullptr);
};
