#pragma once

#include "filecache.h"
#include "gearview.h"
#include <QPushButton>
#include <QWidget>

struct GameData;

class SingleGearView : public QWidget {
    Q_OBJECT
public:
    explicit SingleGearView(GameData* data, FileCache& cache);

Q_SIGNALS:
    void gearChanged();

    void raceChanged();
    void subraceChanged();
    void genderChanged();
    void levelOfDetailChanged();

    void addToFullModelViewer(GearInfo& info);

public Q_SLOTS:
    void clear();
    void setGear(const GearInfo& info);

    void setRace(Race race);
    void setSubrace(Subrace subrace);
    void setGender(Gender gender);
    void setLevelOfDetail(int lod);

private Q_SLOTS:
    void reloadGear();

private:
    std::optional<GearInfo> currentGear;

    Race currentRace = Race::Hyur;
    Subrace currentSubrace = Subrace::Midlander;
    Gender currentGender = Gender::Female;
    int currentLod = 0;

    GearView* gearView = nullptr;
    QComboBox *raceCombo, *subraceCombo, *genderCombo, *lodCombo;
    QPushButton *addToFMVButton, *exportButton;

    bool loadingComboData = false;

    GameData* data = nullptr;
};