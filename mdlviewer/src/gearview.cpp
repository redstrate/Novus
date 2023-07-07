#include "gearview.h"
#include "magic_enum.hpp"

#include <QDebug>
#include <QVBoxLayout>

GearView::GearView(GameData* data) : data(data) {
    mdlPart = new MDLPart(data);

    reloadRaceDeforms();

    auto layout = new QVBoxLayout();
    layout->addWidget(mdlPart);
    setLayout(layout);

    connect(this, &GearView::gearChanged, this, [=] {
        reloadModel();
    });
    connect(this, &GearView::raceChanged, this, [=] {
        reloadRaceDeforms();
        reloadModel();
    });
    connect(this, &GearView::subraceChanged, this, [=] {
        reloadRaceDeforms();
        reloadModel();
    });
    connect(this, &GearView::genderChanged, this, [=] {
        reloadRaceDeforms();
        reloadModel();
    });
    connect(this, &GearView::levelOfDetailChanged, this, &GearView::reloadModel);
}

std::vector<std::pair<Race, Subrace>> GearView::supportedRaces() const {
    std::vector<std::pair<Race, Subrace>> races;
    for (const auto& gear : gears) {
        for (auto [race, race_name] : magic_enum::enum_entries<Race>()) {
            for (auto subrace : physis_get_supported_subraces(race).subraces) {
                auto equip_path =
                    physis_build_equipment_path(gear.modelInfo.primaryID, race, subrace, currentGender, gear.slot);

                if (physis_gamedata_exists(data, equip_path))
                    races.emplace_back(race, subrace);
            }
        }
    }

    return races;
}

std::vector<Gender> GearView::supportedGenders() const {
    std::vector<Gender> genders;
    for (const auto& gear : gears) {
        for (auto [gender, gender_name] : magic_enum::enum_entries<Gender>()) {
            auto equip_path = physis_build_equipment_path(
                gear.modelInfo.primaryID, currentRace, Subrace::Midlander, currentGender, gear.slot);

            if (physis_gamedata_exists(data, equip_path))
                genders.push_back(gender);
        }
    }

    return genders;
}

int GearView::lodCount() const {
    return maxLod;
}

void GearView::exportModel(const QString& fileName) {
    mdlPart->exportModel(fileName);
}

void GearView::clear() {
    gears.clear();

    Q_EMIT gearChanged();
}

void GearView::addGear(GearInfo& gear) {
    qDebug() << "Adding gear" << gear.name.c_str();

    gears.push_back(gear);

    Q_EMIT gearChanged();
}

void GearView::setRace(Race race) {
    if (currentRace == race) {
        return;
    }

    currentRace = race;

    auto supportedSubraces = physis_get_supported_subraces(race);
    if (supportedSubraces.subraces[0] == currentSubrace || supportedSubraces.subraces[1] == currentSubrace) {
    } else {
        setSubrace(supportedSubraces.subraces[0]);
    }

    Q_EMIT raceChanged();
}

void GearView::setSubrace(Subrace subrace) {
    if (currentSubrace == subrace) {
        return;
    }

    currentSubrace = subrace;
    Q_EMIT subraceChanged();
}

void GearView::setGender(Gender gender) {
    if (currentGender == gender) {
        return;
    }

    currentGender = gender;
    Q_EMIT genderChanged();
}

void GearView::setLevelOfDetail(int lod) {
    if (currentLod == lod) {
        return;
    }

    currentLod = lod;
    Q_EMIT levelOfDetailChanged();
}

void GearView::reloadModel() {
    mdlPart->clear();

    maxLod = 0;

    for (const auto& gear : gears) {
        auto mdl_data = physis_gamedata_extract_file(
            data,
            physis_build_equipment_path(
                gear.modelInfo.primaryID, currentRace, currentSubrace, currentGender, gear.slot));

        // attempt to load the next best race
        // currently hardcoded to hyur midlander
        Race fallbackRace = currentRace;
        Subrace fallbackSubrace = currentSubrace;
        if (mdl_data.size == 0) {
            mdl_data = physis_gamedata_extract_file(
                data,
                physis_build_equipment_path(
                    gear.modelInfo.primaryID, Race::Hyur, Subrace::Midlander, currentGender, gear.slot));
            fallbackRace = Race::Hyur;
            fallbackSubrace = Subrace::Midlander;
        }

        if (mdl_data.size > 0) {
            auto mdl = physis_mdl_parse(mdl_data.size, mdl_data.data);

            std::vector<physis_Material> materials;
            for (int i = 0; i < mdl.num_material_names; i++) {
                const char* material_name = mdl.material_names[i];

                // std::string mtrl_path =
                // loadedGear.gearInfo->getMtrlPath(201);
                std::string mtrl_path = fmt::format(
                    "chara/equipment/e{gearId:04d}/material/"
                    "v{gearVersion:04d}{}",
                    material_name,
                    fmt::arg("gearId", gear.modelInfo.primaryID),
                    fmt::arg("gearVersion", gear.modelInfo.gearVersion));

                int bodyCode = 1;

                // skin path
                std::string skinmtrl_path = fmt::format(
                    "chara/human/c{raceCode:04d}/obj/body/b{bodyCode:04d}/"
                    "material/v0001{}",
                    material_name,
                    fmt::arg("raceCode", physis_get_race_code(fallbackRace, fallbackSubrace, currentGender)),
                    fmt::arg("bodyCode", bodyCode));

                if (physis_gamedata_exists(data, mtrl_path.c_str())) {
                    auto mat = physis_material_parse(physis_gamedata_extract_file(data, mtrl_path.c_str()));
                    materials.push_back(mat);
                }

                if (physis_gamedata_exists(data, skinmtrl_path.c_str())) {
                    auto mat = physis_material_parse(physis_gamedata_extract_file(data, skinmtrl_path.c_str()));
                    materials.push_back(mat);
                }
            }

            maxLod = std::max(mdl.num_lod, maxLod);

            mdlPart->addModel(mdl, materials, currentLod);
        }
    }

    Q_EMIT modelReloaded();
}

void GearView::reloadRaceDeforms() {
    qDebug() << "Loading race deform matrices for " << magic_enum::enum_name(currentRace).data()
             << magic_enum::enum_name(currentSubrace).data() << magic_enum::enum_name(currentGender).data();
    const int raceCode = physis_get_race_code(currentRace, currentSubrace, currentGender);
    qDebug() << "Race code: " << raceCode;

    QString skelName = QString{"c%1b0001.skel"}.arg(raceCode, 4, 10, QLatin1Char{'0'});
    mdlPart->setSkeleton(physis_skeleton_from_skel(physis_read_file(skelName.toStdString().c_str())));

    // racial deforms don't work on Hyur, not needed? TODO not sure
    if (currentRace != Race::Hyur) {
        QString deformName = QString{"c%1_deform.json"}.arg(raceCode, 4, 10, QLatin1Char{'0'});
        mdlPart->loadRaceDeformMatrices(physis_read_file(deformName.toStdString().c_str()));
    } else {
        for (auto& data : mdlPart->boneData) {
            data.deformRaceMatrix = glm::mat4(1.0f);
        }
    }
}

MDLPart& GearView::part() const {
    return *mdlPart;
}

#include "moc_gearview.cpp"
