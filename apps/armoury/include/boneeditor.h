// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "sklbpart.h"

class GearView;

class BoneEditor : public SklbPart
{
    Q_OBJECT

public:
    explicit BoneEditor(GearView *gearView, QWidget *parent = nullptr);

private:
    GearView *gearView = nullptr;
};
