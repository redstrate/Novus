#pragma once

#include <QWidget>
#include <physis.hpp>
#include <fmt/format.h>
#include <QComboBox>
#include "mdlpart.h"

struct ModelInfo {
    int primaryID;
    int gearVersion = 1;
};

struct GearInfo {
    std::string name;
    Slot slot;
    ModelInfo modelInfo;

    std::string getMtrlPath(int raceID) {
        return fmt::format("chara/equipment/e{gearId:04d}/material/v{gearVersion:04d}/mt_c{raceId:04d}e{gearId:04d}_{slot}_a.mtrl",
                           fmt::arg("gearId", modelInfo.primaryID),
                           fmt::arg("gearVersion", modelInfo.gearVersion),
                           fmt::arg("raceId", raceID),
                           fmt::arg("slot", physis_get_slot_name(slot)));
    }
};

struct GameData;

class GearView : public QWidget {
    Q_OBJECT
public:
    explicit GearView(GameData* data);

    /// Returns an inclusive list of races supported by the current gearset.
    std::vector<Race> supportedRaces() const;

    /// Returns an inclusive list of genders supported by the current gearset.
    std::vector<Gender> supportedGenders() const;

    /// Returns an inclusive list of LoDs supported by the current gearset.
    int lodCount() const;

    void exportModel(const QString& fileName);

    MDLPart& part() const;

Q_SIGNALS:
    void gearChanged();
    void modelReloaded();

    void raceChanged();
    void genderChanged();
    void levelOfDetailChanged();

public Q_SLOTS:
    void clear();
    void addGear(GearInfo& gear);

    void setRace(Race race);
    void setGender(Gender gender);
    void setLevelOfDetail(int lod);

    void reloadModel();

private:
    Race currentRace = Race::Hyur;
    Gender currentGender = Gender::Female;
    int currentLod = 0;

    uint32_t maxLod = 0;

    std::vector<GearInfo> gears;

    MDLPart* mdlPart = nullptr;

    GameData* data;
};