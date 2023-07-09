#include "gearview.h"

#include <QDebug>
#include <QVBoxLayout>

#include "magic_enum.hpp"
#include "filecache.h"

GearView::GearView(GameData* data, FileCache& cache) : data(data), cache(cache) {
    mdlPart = new MDLPart(data, cache);

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

    connect(this, &GearView::faceChanged, this, &GearView::reloadModel);
    connect(this, &GearView::hairChanged, this, &GearView::reloadModel);
    connect(this, &GearView::earChanged, this, &GearView::reloadModel);
    connect(this, &GearView::tailChanged, this, &GearView::reloadModel);
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

    if (race == Race::AuRa || race == Race::Miqote) {
        setTail(1);
    } else {
        setTail(-1);
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

void GearView::setFace(int bodyVer) {
    if (face == bodyVer) {
        return;
    }

    if (bodyVer == -1) {
        face = std::nullopt;
    } else {
        face = bodyVer;
    }

    Q_EMIT faceChanged();
}

void GearView::setHair(int bodyVer) {
    if (hair == bodyVer) {
        return;
    }

    if (bodyVer == -1) {
        hair = std::nullopt;
    } else {
        hair = bodyVer;
    }

    Q_EMIT hairChanged();
}

void GearView::setEar(int bodyVer) {
    if (ear == bodyVer) {
        return;
    }

    if (bodyVer == -1) {
        ear = std::nullopt;
    } else {
        ear = bodyVer;
    }

    Q_EMIT earChanged();
}

void GearView::setTail(int bodyVer) {
    if (tail == bodyVer) {
        return;
    }

    if (bodyVer == -1) {
        tail = std::nullopt;
    } else {
        tail = bodyVer;
    }

    Q_EMIT tailChanged();
}

void GearView::reloadModel() {
    mdlPart->clear();

    maxLod = 0;

    for (const auto& gear : gears) {
        auto mdl_data = cache.lookupFile(physis_build_equipment_path(
                gear.modelInfo.primaryID, currentRace, currentSubrace, currentGender, gear.slot));

        // attempt to load the next best race
        // currently hardcoded to hyur midlander
        Race fallbackRace = currentRace;
        Subrace fallbackSubrace = currentSubrace;
        if (mdl_data.size == 0) {
            mdl_data = cache.lookupFile(physis_build_equipment_path(
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
                    auto mat = physis_material_parse(cache.lookupFile(mtrl_path.c_str()));
                    materials.push_back(mat);
                }

                if (physis_gamedata_exists(data, skinmtrl_path.c_str())) {
                    auto mat = physis_material_parse(cache.lookupFile(skinmtrl_path.c_str()));
                    materials.push_back(mat);
                }
            }

            maxLod = std::max(mdl.num_lod, maxLod);

            mdlPart->addModel(mdl, materials, currentLod);
        }
    }

    if (face) {
        auto mdl_data = cache.lookupFile(physis_build_character_path(
                CharacterCategory::Face, *face, currentRace, currentSubrace, currentGender));

        if (mdl_data.size > 0) {
            auto mdl = physis_mdl_parse(mdl_data.size, mdl_data.data);

            std::vector<physis_Material> materials;
            for (int i = 0; i < mdl.num_material_names; i++) {
                const char* material_name = mdl.material_names[i];

                std::string skinmtrl_path = fmt::format(
                    "chara/human/c{raceCode:04d}/obj/face/f{bodyCode:04d}/"
                    "material{}",
                    material_name,
                    fmt::arg("raceCode", physis_get_race_code(currentRace, currentSubrace, currentGender)),
                    fmt::arg("bodyCode", *face));

                fmt::print("oops: {}", skinmtrl_path);

                if (physis_gamedata_exists(data, skinmtrl_path.c_str())) {
                    auto mat = physis_material_parse(cache.lookupFile(skinmtrl_path.c_str()));
                    materials.push_back(mat);
                }
            }

            mdlPart->addModel(mdl, materials, currentLod);
        }
    }

    if (hair) {
        auto mdl_data = cache.lookupFile(physis_build_character_path(
                CharacterCategory::Hair, *hair, currentRace, currentSubrace, currentGender));

        if (mdl_data.size > 0) {
            auto mdl = physis_mdl_parse(mdl_data.size, mdl_data.data);

            std::vector<physis_Material> materials;
            for (int i = 0; i < mdl.num_material_names; i++) {
                const char* material_name = mdl.material_names[i];

                std::string skinmtrl_path = fmt::format(
                    "chara/human/c{raceCode:04d}/obj/hair/h{bodyCode:04d}/"
                    "material/v0001{}",
                    material_name,
                    fmt::arg("raceCode", physis_get_race_code(currentRace, currentSubrace, currentGender)),
                    fmt::arg("bodyCode", *hair));

                fmt::print("oops: {}", skinmtrl_path);

                if (physis_gamedata_exists(data, skinmtrl_path.c_str())) {
                    auto mat = physis_material_parse(cache.lookupFile(skinmtrl_path.c_str()));
                    materials.push_back(mat);
                }
            }

            mdlPart->addModel(mdl, materials, currentLod);
        }
    }

    if (ear) {
        auto mdl_data = cache.lookupFile(physis_build_character_path(
                CharacterCategory::Hair, *ear, currentRace, currentSubrace, currentGender));

        if (mdl_data.size > 0) {
            auto mdl = physis_mdl_parse(mdl_data.size, mdl_data.data);

            std::vector<physis_Material> materials;
            for (int i = 0; i < mdl.num_material_names; i++) {
                const char* material_name = mdl.material_names[i];

                std::string skinmtrl_path = fmt::format(
                    "chara/human/c{raceCode:04d}/obj/ear/e{bodyCode:04d}/"
                    "material/v0001{}",
                    material_name,
                    fmt::arg("raceCode", physis_get_race_code(currentRace, currentSubrace, currentGender)),
                    fmt::arg("bodyCode", *ear));

                fmt::print("oops: {}", skinmtrl_path);

                if (physis_gamedata_exists(data, skinmtrl_path.c_str())) {
                    auto mat = physis_material_parse(cache.lookupFile(skinmtrl_path.c_str()));
                    materials.push_back(mat);
                }
            }

            mdlPart->addModel(mdl, materials, currentLod);
        }
    }

    if (tail) {
        auto mdl_data = cache.lookupFile(physis_build_character_path(
                CharacterCategory::Tail, *tail, currentRace, currentSubrace, currentGender));

        if (mdl_data.size > 0) {
            auto mdl = physis_mdl_parse(mdl_data.size, mdl_data.data);

            const char* material_name = mdl.material_names[0];

            std::string skinmtrl_path = fmt::format(
                "chara/human/c{raceCode:04d}/obj/tail/t{bodyCode:04d}/"
                "material/v0001{}",
                material_name,
                fmt::arg("raceCode", physis_get_race_code(currentRace, currentSubrace, currentGender)),
                fmt::arg("bodyCode", *tail));

            if (physis_gamedata_exists(data, skinmtrl_path.c_str())) {
                auto mat = physis_material_parse(cache.lookupFile(skinmtrl_path.c_str()));
                mdlPart->addModel(mdl, {mat}, currentLod);
            }
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

    // racial deforms don't work on Hyur Midlander, not needed? TODO not sure
    if (currentSubrace != Subrace::Midlander) {
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
