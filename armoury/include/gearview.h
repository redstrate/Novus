// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "filecache.h"
#include "mdlpart.h"
#include <QComboBox>
#include <QFrame>
#include <physis.hpp>

struct ModelInfo {
    int primaryID;
    int gearVersion = 1;
};

struct GearInfo {
    std::string name;
    Slot slot;
    ModelInfo modelInfo;

    std::string getMtrlPath(const std::string_view material_name) const
    {
        return physis_build_gear_material_path(modelInfo.primaryID, modelInfo.gearVersion, material_name.data());
    }
};

inline bool operator==(const GearInfo &a, const GearInfo &b)
{
    return a.name == b.name && a.slot == b.slot;
}

struct GameData;

class GearView : public QFrame
{
    Q_OBJECT

public:
    explicit GearView(GameData *data, FileCache &cache, QWidget *parent = nullptr);

    /// Returns an inclusive list of races supported by the current gearset.
    std::vector<std::pair<Race, Subrace>> supportedRaces() const;

    /// Returns an inclusive list of genders supported by the current gearset.
    std::vector<Gender> supportedGenders() const;

    /// Returns an inclusive list of LoDs supported by the current gearset.
    int lodCount() const;

    void exportModel(const QString &fileName);

    MDLPart &part() const;

    Race currentRace = Race::Hyur;
    Subrace currentSubrace = Subrace::Midlander;
    Gender currentGender = Gender::Male;

    QString getLoadedGearPath() const;

Q_SIGNALS:
    void gearChanged();
    void modelReloaded();
    void loadingChanged(bool loading);

    void raceChanged();
    void subraceChanged();
    void genderChanged();
    void levelOfDetailChanged();

    void faceChanged();
    void hairChanged();
    void earChanged();
    void tailChanged();

public Q_SLOTS:
    void addGear(GearInfo &gear);
    void removeGear(GearInfo &gear);

    void setRace(Race race);
    void setSubrace(Subrace subrace);
    void setGender(Gender gender);
    void setLevelOfDetail(int lod);

    void setFace(int bodyVer);
    void setHair(int bodyVer);
    void setEar(int bodyVer);
    void setTail(int bodyVer);

    void reloadRaceDeforms();

protected:
    void changeEvent(QEvent *) override;

private:
    int currentLod = 0;

    uint32_t maxLod = 0;

    struct LoadedGear {
        GearInfo info;
        physis_MDL mdl{};
        QLatin1String path;
        int bodyId = 0;
    };

    std::vector<LoadedGear> loadedGears;
    std::vector<LoadedGear> queuedGearAdditions;
    std::vector<LoadedGear> queuedGearRemovals;
    bool gearDirty = false;

    std::optional<int> face = 1, hair = 1, ear = 1, tail;
    bool faceDirty = false, hairDirty = false, earDirty = false, tailDirty = false;
    bool raceDirty = false;

    MDLPart *mdlPart = nullptr;

    GameData *data;
    FileCache &cache;

    bool updating = false;
    void updatePart();
    bool needsUpdate() const;
};