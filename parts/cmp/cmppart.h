// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QWidget>
#include <physis.hpp>

class RaceTreeData : public QObject
{
    Q_OBJECT

public:
    RaceTreeData(Race race, Tribe subrace)
        : race(race)
        , subrace(subrace)
    {
    }

    Race race;
    Tribe subrace;
};

class CmpPart : public QWidget
{
    Q_OBJECT

public:
    explicit CmpPart(physis_SqPackResource *data, QWidget *parent = nullptr);

    void load(physis_Buffer file);

private:
    void loadRaceData(Race race, Tribe subrace);

    physis_SqPackResource *data = nullptr;
    physis_CMP cmp{};

    QDoubleSpinBox *maleMinSize = nullptr;
    QDoubleSpinBox *maleMaxSize = nullptr;

    QDoubleSpinBox *maleMinTail = nullptr;
    QDoubleSpinBox *maleMaxTail = nullptr;

    QDoubleSpinBox *femaleMinSize = nullptr;
    QDoubleSpinBox *femaleMaxSize = nullptr;

    QDoubleSpinBox *femaleMinTail = nullptr;
    QDoubleSpinBox *femaleMaxTail = nullptr;

    QDoubleSpinBox *bustMinX = nullptr;
    QDoubleSpinBox *bustMinY = nullptr;
    QDoubleSpinBox *bustMinZ = nullptr;

    QDoubleSpinBox *bustMaxX = nullptr;
    QDoubleSpinBox *bustMaxY = nullptr;
    QDoubleSpinBox *bustMaxZ = nullptr;

    QHBoxLayout *layout = nullptr;
};
