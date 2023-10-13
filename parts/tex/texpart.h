// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QWidget>
#include <physis.hpp>

#include "imagelabel.h"

class TexPart : public QWidget
{
    Q_OBJECT

public:
    explicit TexPart(GameData *data);

    void load(physis_Buffer file);

private:
    GameData *data = nullptr;

    ImageLabel *m_label = nullptr;
};