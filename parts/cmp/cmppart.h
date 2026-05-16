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
    explicit CmpPart(QWidget *parent = nullptr);

    void load(Platform platform, physis_Buffer file);

private:
    void loadRaceData(Race race, Tribe subrace);

    physis_CMP m_cmp{};

    QDoubleSpinBox *m_maleMinSize = nullptr;
    QDoubleSpinBox *m_maleMaxSize = nullptr;

    QDoubleSpinBox *m_maleMinTail = nullptr;
    QDoubleSpinBox *m_maleMaxTail = nullptr;

    QDoubleSpinBox *m_femaleMinSize = nullptr;
    QDoubleSpinBox *m_femaleMaxSize = nullptr;

    QDoubleSpinBox *m_femaleMinTail = nullptr;
    QDoubleSpinBox *m_femaleMaxTail = nullptr;

    QDoubleSpinBox *m_bustMinX = nullptr;
    QDoubleSpinBox *m_bustMinY = nullptr;
    QDoubleSpinBox *m_bustMinZ = nullptr;

    QDoubleSpinBox *m_bustMaxX = nullptr;
    QDoubleSpinBox *m_bustMaxY = nullptr;
    QDoubleSpinBox *m_bustMaxZ = nullptr;

    QHBoxLayout *m_layout = nullptr;
};
