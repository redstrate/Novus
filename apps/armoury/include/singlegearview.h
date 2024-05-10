// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "filecache.h"
#include "gearview.h"
#include <QPushButton>
#include <QWidget>

struct GameData;

class SingleGearView : public QWidget
{
    Q_OBJECT

public:
    explicit SingleGearView(GameData *data, FileCache &cache, QWidget *parent = nullptr);

    QString getLoadedGearPath() const;
    QList<physis_Material> getLoadedMaterials() const;

Q_SIGNALS:
    void gearChanged();
    void gotMDLPath();

    void raceChanged();
    void subraceChanged();
    void genderChanged();
    void levelOfDetailChanged();

    void addToFullModelViewer(GearInfo &info);
    void importedModel();

    void doneLoadingModel();

public Q_SLOTS:
    void clear();
    void setGear(const GearInfo &info);

    void setRace(Race race);
    void setSubrace(Subrace subrace);
    void setGender(Gender gender);
    void setLevelOfDetail(int lod);

    void setFMVAvailable(bool available);

private Q_SLOTS:
    void reloadGear();

private:
    void importModel(const QString &filename);

    std::optional<GearInfo> currentGear;

    Race currentRace = Race::Hyur;
    Subrace currentSubrace = Subrace::Midlander;
    Gender currentGender = Gender::Male;
    int currentLod = 0;

    GearView *gearView = nullptr;
    QComboBox *raceCombo, *subraceCombo, *genderCombo, *lodCombo;
    QPushButton *addToFMVButton, *editButton, *importButton, *exportButton;

    bool loadingComboData = false;
    bool fmvAvailable = false;

    GameData *data = nullptr;
};