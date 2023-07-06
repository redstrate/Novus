#pragma once

#include <QWidget>
#include <physis.hpp>
#include <QDoubleSpinBox>

class RaceTreeData : public QObject {
    Q_OBJECT
public:
    RaceTreeData(Race race, Subrace subrace) : race(race), subrace(subrace) {}

    Race race;
    Subrace subrace;
};

class CmpEditor : public QWidget {
    Q_OBJECT

public:
    explicit CmpEditor(GameData* data);

private:
    void loadRaceData(Race race, Subrace subrace);

    GameData* data;
    physis_CMP cmp;

    QDoubleSpinBox* maleMinSize;
    QDoubleSpinBox* maleMaxSize;

    QDoubleSpinBox* maleMinTail;
    QDoubleSpinBox* maleMaxTail;

    QDoubleSpinBox* femaleMinSize;
    QDoubleSpinBox* femaleMaxSize;

    QDoubleSpinBox* femaleMinTail;
    QDoubleSpinBox* femaleMaxTail;

    QDoubleSpinBox* bustMinX;
    QDoubleSpinBox* bustMinY;
    QDoubleSpinBox* bustMinZ;

    QDoubleSpinBox* bustMaxX;
    QDoubleSpinBox* bustMaxY;
    QDoubleSpinBox* bustMaxZ;
};
