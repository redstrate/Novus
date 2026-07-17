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
    QString name;
    EquipSlotCategory slot;
    ModelInfo modelInfo;
    uint16_t icon;

    std::string getMtrlPath(const std::string_view material_name) const
    {
        return physis_build_gear_material_path(modelInfo.primaryID, modelInfo.gearVersion, material_name.data());
    }
};

inline bool operator==(const GearInfo &a, const GearInfo &b)
{
    return a.name == b.name && a.slot == b.slot;
}

class GearView : public QFrame
{
    Q_OBJECT

public:
    explicit GearView(FileCache &cache, QWidget *parent = nullptr);

    /// Returns an inclusive list of races supported by the current gearset.
    std::vector<std::pair<Race, Tribe>> supportedRaces() const;

    /// Returns an inclusive list of genders supported by the current gearset.
    std::vector<Gender> supportedGenders() const;

    /// Returns an inclusive list of LoDs supported by the current gearset.
    int lodCount() const;

    void exportModel(const QString &fileName) const;

    MDLPart &part() const;

    Race currentRace = Race::Hyur;
    Tribe currentTribe = Tribe::Midlander;
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
    void setTribe(Tribe subrace);
    void setGender(Gender gender);
    void setLevelOfDetail(int lod);

    void setFace(int faceCode);
    void setHair(int hairCode);
    void setEar(int earCode);
    void setTail(int tailCode);

    void reloadRaceDeforms() const;

protected:
    void changeEvent(QEvent *) override;

private:
    void resetMdlPart() const;

    int m_currentLod = 0;

    uint32_t m_maxLod = 0;

    struct LoadedGear {
        GearInfo info;
        physis_MDL mdl{};
        QLatin1String path;
        int bodyId = 0;
    };

    std::vector<LoadedGear> m_loadedGears;
    std::vector<LoadedGear> m_queuedGearAdditions;
    std::vector<LoadedGear> m_queuedGearRemovals;
    bool m_gearDirty = false;

    std::optional<int> m_face = 1, m_hair = 1, m_ear = 1, m_tail;
    bool m_faceDirty = false, m_hairDirty = false, m_earDirty = false, m_tailDirty = false;
    bool m_raceDirty = false;

    MDLPart *m_mdlPart = nullptr;

    FileCache &m_cache;

    bool m_updating = false;

    void updatePart();
    bool needsUpdate() const;
};
