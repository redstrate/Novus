// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gearview.h"

#include <QThreadPool>
#include <QVBoxLayout>
#include <QtConcurrent>
#include <imgui.h>

#include "filecache.h"
#include "magic_enum.hpp"

GearView::GearView(physis_SqPackResource *data, FileCache &cache, QWidget *parent)
    : QFrame(parent)
    , data(data)
    , cache(cache)
{
    setFrameShape(QFrame::Shape::Panel);
    setFrameShadow(QFrame::Shadow::Sunken);

    mdlPart = new MDLPart(data, cache);
    reloadRaceDeforms();

    auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(mdlPart);
    setLayout(layout);

    mdlPart->requestUpdate = [this] {
        auto &io = ImGui::GetIO();
        if (updating) {
            if (ImGui::Begin("Loading", nullptr, ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs)) {
                ImGui::SetWindowPos(ImVec2(0, 0));
                ImGui::SetWindowSize(io.DisplaySize);

                // TODO: localize
                const char *loadingLabel{"Loading gear..."};
                ImGui::SetCursorPosX((io.DisplaySize.x - ImGui::CalcTextSize(loadingLabel).x) * 0.5f);
                ImGui::SetCursorPosY((io.DisplaySize.y - ImGui::CalcTextSize(loadingLabel).y) * 0.5f);
                ImGui::Text("%s", loadingLabel);
            }
            ImGui::End();
        }

        if (updating) {
            return;
        }

        if (needsUpdate()) {
            updating = true;

            Q_EMIT loadingChanged(true);

            Q_UNUSED(QtConcurrent::run(QThreadPool::globalInstance(), [this] {
                updatePart();
                Q_EMIT loadingChanged(false);
            }))
        }
    };
}

std::vector<std::pair<Race, Tribe>> GearView::supportedRaces() const
{
    std::vector<std::pair<Race, Tribe>> races;
    for (const auto &gear : loadedGears) {
        for (const auto &[race, race_name] : magic_enum::enum_entries<Race>()) {
            for (const auto subrace : physis_get_supported_tribes(race).subraces) {
                auto equip_path = physis_build_equipment_path(gear.info.modelInfo.primaryID, race, subrace, currentGender, gear.info.slot);

                if (cache.fileExists(QLatin1String(equip_path)))
                    races.emplace_back(race, subrace);
            }
        }
    }

    return races;
}

std::vector<Gender> GearView::supportedGenders() const
{
    std::vector<Gender> genders;
    for (const auto &gear : loadedGears) {
        for (auto [gender, gender_name] : magic_enum::enum_entries<Gender>()) {
            auto equip_path = physis_build_equipment_path(gear.info.modelInfo.primaryID, currentRace, Tribe::Midlander, currentGender, gear.info.slot);

            if (cache.fileExists(QLatin1String(equip_path)))
                genders.push_back(gender);
        }
    }

    return genders;
}

int GearView::lodCount() const
{
    return maxLod;
}

void GearView::exportModel(const QString &fileName)
{
    mdlPart->exportModel(fileName);
}

void GearView::addGear(GearInfo &gear)
{
    qDebug() << "Adding gear" << gear.name.c_str();

    queuedGearAdditions.emplace_back(gear);

    for (const auto &loadedGear : loadedGears) {
        if (loadedGear.info.slot == gear.slot) {
            queuedGearRemovals.push_back(loadedGear);
        }
    }

    gearDirty = true;

    Q_EMIT gearChanged();
}

void GearView::removeGear(GearInfo &gear)
{
    qDebug() << "Removing gear" << gear.name.c_str();

    queuedGearRemovals.emplace_back(gear);
    gearDirty = true;

    Q_EMIT gearChanged();
}

void GearView::setRace(Race race)
{
    if (currentRace == race) {
        return;
    }

    currentRace = race;

    const auto supportedTribes = physis_get_supported_tribes(race);
    if (supportedTribes.subraces[0] != currentTribe && supportedTribes.subraces[1] != currentTribe) {
        setTribe(supportedTribes.subraces[0]);
    }

    if (race == Race::AuRa || race == Race::Miqote) {
        setTail(1);
    } else {
        setTail(-1);
    }

    raceDirty = true;

    Q_EMIT raceChanged();
}

void GearView::setTribe(Tribe subrace)
{
    if (currentTribe == subrace) {
        return;
    }

    currentTribe = subrace;

    // Hyur is the only race that has two different subraces
    if (currentRace == Race::Hyur) {
        raceDirty = true;
    }

    Q_EMIT subraceChanged();
}

void GearView::setGender(Gender gender)
{
    if (currentGender == gender) {
        return;
    }

    currentGender = gender;

    raceDirty = true;

    Q_EMIT genderChanged();
}

void GearView::setLevelOfDetail(int lod)
{
    if (currentLod == lod) {
        return;
    }

    currentLod = lod;

    // TODO: maybe should be gearDirty?
    raceDirty = true;

    Q_EMIT levelOfDetailChanged();
}

void GearView::setFace(const int faceCode)
{
    if (face == faceCode) {
        return;
    }

    if (faceCode == -1) {
        face = std::nullopt;
    } else {
        face = faceCode;
    }

    faceDirty = true;
    Q_EMIT faceChanged();
}

void GearView::setHair(int hairCode)
{
    if (hair == hairCode) {
        return;
    }

    if (hairCode == -1) {
        hair = std::nullopt;
    } else {
        hair = hairCode;
    }

    hairDirty = true;
    Q_EMIT hairChanged();
}

void GearView::setEar(const int earCode)
{
    if (ear == earCode) {
        return;
    }

    if (earCode == -1) {
        ear = std::nullopt;
    } else {
        ear = earCode;
    }

    earDirty = true;
    Q_EMIT earChanged();
}

void GearView::setTail(const int tailCode)
{
    if (tail == tailCode) {
        return;
    }

    if (tailCode == -1) {
        tail = std::nullopt;
    } else {
        tail = tailCode;
    }

    tailDirty = true;
    Q_EMIT tailChanged();
}

void GearView::reloadRaceDeforms()
{
    qDebug() << "Loading race deform matrices for " << magic_enum::enum_name(currentRace).data() << magic_enum::enum_name(currentTribe).data()
             << magic_enum::enum_name(currentGender).data();
    const int raceCode = physis_get_race_code(currentRace, currentTribe, currentGender);

    QString skelName = QStringLiteral("chara/human/c%1/skeleton/base/b0001/skl_c%1b0001.sklb").arg(raceCode, 4, 10, QLatin1Char{'0'});
    std::string skelNameStd = skelName.toStdString();
    mdlPart->setSkeleton(physis_skeleton_parse(data->platform, physis_sqpack_read(data, skelNameStd.c_str())));
}

MDLPart &GearView::part() const
{
    return *mdlPart;
}

void GearView::updatePart()
{
    if (raceDirty) {
        // if race changes, all of the models need to be reloaded.
        // TODO: in the future, we can be a bit smarter about this, lots of races use the same model (hyur)
        mdlPart->clear();
        queuedGearAdditions = loadedGears;
        loadedGears.clear();
        gearDirty = true;
    }

    const auto sanitizeMdlPath = [](const QLatin1String mdlPath) -> QString {
        return QString(mdlPath).section(QLatin1Char('/'), -1).remove(QStringLiteral(".mdl"));
    };

    if (gearDirty) {
        for (auto &gearAddition : queuedGearAdditions) {
            auto mdlPath = QLatin1String(
                physis_build_equipment_path(gearAddition.info.modelInfo.primaryID, currentRace, currentTribe, currentGender, gearAddition.info.slot));

            auto mdl_data = cache.lookupFile(mdlPath);

            // attempt to load the next best race
            // currently hardcoded to hyur midlander
            Race fallbackRace = currentRace;
            Tribe fallbackTribe = currentTribe;
            if (mdl_data.size == 0) {
                mdlPath = QLatin1String(
                    physis_build_equipment_path(gearAddition.info.modelInfo.primaryID, Race::Hyur, Tribe::Midlander, currentGender, gearAddition.info.slot));
                mdl_data = cache.lookupFile(mdlPath);
                fallbackRace = Race::Hyur;
                fallbackTribe = Tribe::Midlander;
            }

            if (fallbackRace != currentRace) {
                qDebug() << "Fell back to hyur race for" << mdlPath;
            }

            if (fallbackTribe != currentTribe) {
                qDebug() << "Fell back to midlander subrace for" << mdlPath;
            }

            if (mdl_data.size > 0) {
                auto mdl = physis_mdl_parse(data->platform, mdl_data);
                if (mdl.p_ptr != nullptr) {
                    std::vector<physis_Material> materials;
                    for (uint32_t i = 0; i < mdl.num_material_names; i++) {
                        const char *material_name = mdl.material_names[i];

                        const std::string mtrl_path = gearAddition.info.getMtrlPath(material_name);
                        const std::string skinmtrl_path =
                            physis_build_skin_material_path(physis_get_race_code(fallbackRace, fallbackTribe, currentGender), 1, material_name);

                        if (cache.fileExists(QLatin1String(mtrl_path.c_str()))) {
                            auto mat = physis_material_parse(data->platform, cache.lookupFile(QLatin1String(mtrl_path.c_str())));
                            materials.push_back(mat);
                        }

                        if (cache.fileExists(QLatin1String(skinmtrl_path.c_str()))) {
                            auto mat = physis_material_parse(data->platform, cache.lookupFile(QLatin1String(skinmtrl_path.c_str())));
                            materials.push_back(mat);
                        }
                    }

                    maxLod = std::max(mdl.num_lod, maxLod);

                    gearAddition.bodyId = physis_get_race_code(fallbackRace, fallbackTribe, currentGender);

                    Transformation transformation{};
                    transformation.scale[0] = 1;
                    transformation.scale[1] = 1;
                    transformation.scale[2] = 1;

                    mdlPart->addModel(mdl,
                                      true,
                                      transformation,
                                      sanitizeMdlPath(mdlPath),
                                      materials,
                                      currentLod,
                                      physis_get_race_code(currentRace, currentTribe, currentGender),
                                      gearAddition.bodyId);
                    gearAddition.mdl = mdl;
                    gearAddition.path = mdlPath;
                    loadedGears.push_back(gearAddition);
                } else {
                    qWarning() << "Failed to parse" << mdlPath;
                }
            } else {
                qWarning() << "Failed to load" << mdlPath;
            }
        }

        for (auto &queuedRemoval : queuedGearRemovals) {
            auto it = std::find_if(loadedGears.cbegin(), loadedGears.cend(), [queuedRemoval](const LoadedGear &other) {
                return queuedRemoval.info == other.info;
            });

            if (it != loadedGears.cend()) {
                mdlPart->removeModel((*it).mdl);
                loadedGears.erase(std::remove_if(loadedGears.begin(),
                                                 loadedGears.end(),
                                                 [queuedRemoval](const LoadedGear &other) {
                                                     return queuedRemoval.info == other.info;
                                                 }),
                                  loadedGears.end());
            }
        }

        queuedGearAdditions.clear();
        queuedGearRemovals.clear();
    }

    const auto loadBodyPart = [this, &sanitizeMdlPath](int index, CharacterCategory category, auto build_material_path_func) {
        const auto mdlPath = QLatin1String(physis_build_character_path(category, index, currentRace, currentTribe, currentGender));
        auto mdl_data = cache.lookupFile(mdlPath);

        if (mdl_data.size > 0) {
            auto mdl = physis_mdl_parse(data->platform, mdl_data);
            if (mdl.p_ptr != nullptr) {
                std::vector<physis_Material> materials;
                for (uint32_t i = 0; i < mdl.num_material_names; i++) {
                    const char *material_name = mdl.material_names[i];
                    const std::string skinmtrl_path =
                        build_material_path_func(physis_get_race_code(currentRace, currentTribe, currentGender), index, material_name);

                    if (cache.fileExists(QLatin1String(skinmtrl_path.c_str()))) {
                        auto mat = physis_material_parse(data->platform, cache.lookupFile(QLatin1String(skinmtrl_path.c_str())));
                        materials.push_back(mat);
                    }
                }

                Transformation transformation{};
                transformation.scale[0] = 1;
                transformation.scale[1] = 1;
                transformation.scale[2] = 1;

                mdlPart->addModel(mdl, true, transformation, sanitizeMdlPath(mdlPath), materials, currentLod);
            }
        }
    };

    if (face) {
        loadBodyPart(*face, CharacterCategory::Face, physis_build_face_material_path);
    }

    if (hair) {
        loadBodyPart(*hair, CharacterCategory::Hair, physis_build_hair_material_path);
    }

    if (ear) {
        loadBodyPart(*ear, CharacterCategory::Ear, physis_build_ear_material_path);
    }

    if (tail) {
        loadBodyPart(*tail, CharacterCategory::Tail, physis_build_tail_material_path);
    }

    raceDirty = false;
    gearDirty = false;
    updating = false;
    faceDirty = false;
    hairDirty = false;
    earDirty = false;
    tailDirty = false;
}

bool GearView::needsUpdate() const
{
    return gearDirty || raceDirty || faceDirty || hairDirty || earDirty || tailDirty;
}

QString GearView::getLoadedGearPath() const
{
    if (loadedGears.empty()) {
        return {};
    }
    return loadedGears[0].path;
}

void GearView::changeEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::EnabledChange:
        mdlPart->setEnabled(isEnabled());
        break;
    default:
        break;
    }
    QFrame::changeEvent(event);
}

#include "moc_gearview.cpp"
