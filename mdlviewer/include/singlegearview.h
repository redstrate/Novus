#pragma once

#include <QWidget>
#include <QPushButton>
#include "gearview.h"

struct GameData;

class SingleGearView : public QWidget {
    Q_OBJECT
public:
    explicit SingleGearView(GameData* data);

Q_SIGNALS:
    void gearChanged();

    void raceChanged();
    void genderChanged();
    void levelOfDetailChanged();

    void addToFullModelViewer(GearInfo& info);

public Q_SLOTS:
    void clear();
    void setGear(GearInfo& info);

    void setRace(Race race);
    void setGender(Gender gender);
    void setLevelOfDetail(int lod);

private Q_SLOTS:
    void reloadGear();

private:
    std::optional<GearInfo> currentGear;

    Race currentRace = Race::Hyur;
    Gender currentGender = Gender::Female;
    int currentLod = 0;

    GearView* gearView = nullptr;
    QComboBox* raceCombo, *genderCombo, *lodCombo;
    QPushButton* addToFMVButton, *exportButton;

    bool loadingComboData = false;

    GameData* data = nullptr;
};