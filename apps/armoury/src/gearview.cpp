// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gearview.h"

#include <QThreadPool>
#include <QVBoxLayout>
#include <QtConcurrent>
#include <imgui.h>

#include "filecache.h"
#include "magic_enum.hpp"

GearView::GearView(FileCache &cache, QWidget *parent)
    : QFrame(parent)
    , m_cache(cache)
{
    setFrameShape(QFrame::Shape::Panel);
    setFrameShadow(QFrame::Shadow::Sunken);

    m_mdlPart = new MDLPart(m_cache);
    reloadRaceDeforms();

    auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_mdlPart);
    setLayout(layout);

    m_mdlPart->requestUpdate = [this] {
        auto &io = ImGui::GetIO();
        if (m_updating) {
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

        if (m_updating) {
            return;
        }

        if (needsUpdate()) {
            m_updating = true;

            Q_EMIT loadingChanged(true);

            Q_UNUSED(QtConcurrent::run(QThreadPool::globalInstance(), [this] {
                updatePart();
                Q_EMIT loadingChanged(false);
            }))
        }
    };

    resetMdlPart();
}

std::vector<std::pair<Race, Tribe>> GearView::supportedRaces() const
{
    std::vector<std::pair<Race, Tribe>> races;
    for (const auto &gear : m_loadedGears) {
        for (const auto &[race, race_name] : magic_enum::enum_entries<Race>()) {
            for (const auto subrace : physis_get_supported_tribes(race).subraces) {
                auto equip_path = physis_build_equipment_path(gear.info.modelInfo.primaryID, race, subrace, currentGender, gear.info.slot);

                if (m_cache.exists(QLatin1String(equip_path)))
                    races.emplace_back(race, subrace);
            }
        }
    }

    return races;
}

std::vector<Gender> GearView::supportedGenders() const
{
    std::vector<Gender> genders;
    for (const auto &gear : m_loadedGears) {
        for (auto [gender, gender_name] : magic_enum::enum_entries<Gender>()) {
            auto equip_path = physis_build_equipment_path(gear.info.modelInfo.primaryID, currentRace, Tribe::Midlander, currentGender, gear.info.slot);

            if (m_cache.exists(QLatin1String(equip_path)))
                genders.push_back(gender);
        }
    }

    return genders;
}

int GearView::lodCount() const
{
    return m_maxLod;
}

void GearView::exportModel(const QString &fileName)
{
    m_mdlPart->exportModel(fileName);
}

void GearView::addGear(GearInfo &gear)
{
    qDebug() << "Adding gear" << gear.name;

    m_queuedGearAdditions.emplace_back(gear);

    for (const auto &loadedGear : m_loadedGears) {
        if (loadedGear.info.slot == gear.slot) {
            m_queuedGearRemovals.push_back(loadedGear);
        }
    }

    m_gearDirty = true;

    Q_EMIT gearChanged();
}

void GearView::removeGear(GearInfo &gear)
{
    qDebug() << "Removing gear" << gear.name;

    m_queuedGearRemovals.emplace_back(gear);
    m_gearDirty = true;

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

    m_raceDirty = true;

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
        m_raceDirty = true;
    }

    Q_EMIT subraceChanged();
}

void GearView::setGender(Gender gender)
{
    if (currentGender == gender) {
        return;
    }

    currentGender = gender;

    m_raceDirty = true;

    Q_EMIT genderChanged();
}

void GearView::setLevelOfDetail(int lod)
{
    if (m_currentLod == lod) {
        return;
    }

    m_currentLod = lod;

    // TODO: maybe should be gearDirty?
    m_raceDirty = true;

    Q_EMIT levelOfDetailChanged();
}

void GearView::setFace(const int faceCode)
{
    if (m_face == faceCode) {
        return;
    }

    if (faceCode == -1) {
        m_face = std::nullopt;
    } else {
        m_face = faceCode;
    }

    m_faceDirty = true;
    Q_EMIT faceChanged();
}

void GearView::setHair(int hairCode)
{
    if (m_hair == hairCode) {
        return;
    }

    if (hairCode == -1) {
        m_hair = std::nullopt;
    } else {
        m_hair = hairCode;
    }

    m_hairDirty = true;
    Q_EMIT hairChanged();
}

void GearView::setEar(const int earCode)
{
    if (m_ear == earCode) {
        return;
    }

    if (earCode == -1) {
        m_ear = std::nullopt;
    } else {
        m_ear = earCode;
    }

    m_earDirty = true;
    Q_EMIT earChanged();
}

void GearView::setTail(const int tailCode)
{
    if (m_tail == tailCode) {
        return;
    }

    if (tailCode == -1) {
        m_tail = std::nullopt;
    } else {
        m_tail = tailCode;
    }

    m_tailDirty = true;
    Q_EMIT tailChanged();
}

void GearView::reloadRaceDeforms()
{
    qDebug() << "Loading race deform matrices for " << magic_enum::enum_name(currentRace).data() << magic_enum::enum_name(currentTribe).data()
             << magic_enum::enum_name(currentGender).data();
    QString skelName = QString::fromStdString(physis_skeleton_path(currentRace, currentTribe, currentGender));
    m_mdlPart->setSkeleton(physis_skeleton_parse(m_cache.platform(), m_cache.read(skelName)));
}

MDLPart &GearView::part() const
{
    return *m_mdlPart;
}

void GearView::updatePart()
{
    if (m_raceDirty) {
        // if race changes, all of the models need to be reloaded.
        // TODO: in the future, we can be a bit smarter about this, lots of races use the same model (hyur)
        resetMdlPart();
        m_queuedGearAdditions = m_loadedGears;
        m_loadedGears.clear();
        m_gearDirty = true;
    }

    const auto sanitizeMdlPath = [](const QLatin1String mdlPath) -> QString {
        return QString(mdlPath).section(QLatin1Char('/'), -1).remove(QStringLiteral(".mdl"));
    };

    if (m_gearDirty) {
        for (auto &gearAddition : m_queuedGearAdditions) {
            auto mdlPath = QLatin1String(
                physis_build_equipment_path(gearAddition.info.modelInfo.primaryID, currentRace, currentTribe, currentGender, gearAddition.info.slot));

            auto mdl_data = m_cache.read(mdlPath);

            // attempt to load the next best race
            // currently hardcoded to hyur midlander
            Race fallbackRace = currentRace;
            Tribe fallbackTribe = currentTribe;
            if (mdl_data.size == 0) {
                mdlPath = QLatin1String(
                    physis_build_equipment_path(gearAddition.info.modelInfo.primaryID, Race::Hyur, Tribe::Midlander, currentGender, gearAddition.info.slot));
                mdl_data = m_cache.read(mdlPath);
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
                auto mdl = physis_mdl_parse(m_cache.platform(), mdl_data);
                if (mdl.p_ptr != nullptr) {
                    std::vector<std::pair<std::string, physis_Material>> materials;
                    for (uint32_t i = 0; i < mdl.num_material_names; i++) {
                        const char *material_name = mdl.material_names[i];

                        const std::string mtrl_path = gearAddition.info.getMtrlPath(material_name);
                        const std::string skinmtrl_path =
                            physis_build_skin_material_path(physis_get_race_code(fallbackRace, fallbackTribe, currentGender), 1, material_name);

                        if (m_cache.exists(QLatin1String(mtrl_path.c_str()))) {
                            auto mat = physis_material_parse(m_cache.platform(), m_cache.read(QLatin1String(mtrl_path.c_str())));
                            materials.push_back(std::make_pair(mtrl_path, mat));
                        }

                        if (m_cache.exists(QLatin1String(skinmtrl_path.c_str()))) {
                            auto mat = physis_material_parse(m_cache.platform(), m_cache.read(QLatin1String(skinmtrl_path.c_str())));
                            materials.push_back(std::make_pair(mtrl_path, mat));
                        }
                    }

                    m_maxLod = std::max(mdl.num_lod, m_maxLod);

                    gearAddition.bodyId = physis_get_race_code(fallbackRace, fallbackTribe, currentGender);

                    Transformation transformation{};
                    transformation.scale[0] = 1;
                    transformation.scale[1] = 1;
                    transformation.scale[2] = 1;

                    m_mdlPart->addModel(mdl,
                                        true,
                                        transformation,
                                        sanitizeMdlPath(mdlPath),
                                        materials,
                                        m_currentLod,
                                        physis_get_race_code(currentRace, currentTribe, currentGender),
                                        gearAddition.bodyId);
                    gearAddition.mdl = mdl;
                    gearAddition.path = mdlPath;
                    m_loadedGears.push_back(gearAddition);
                } else {
                    qWarning() << "Failed to parse" << mdlPath;
                }
            } else {
                qWarning() << "Failed to load" << mdlPath;
            }
        }

        for (auto &queuedRemoval : m_queuedGearRemovals) {
            auto it = std::find_if(m_loadedGears.cbegin(), m_loadedGears.cend(), [queuedRemoval](const LoadedGear &other) {
                return queuedRemoval.info == other.info;
            });

            if (it != m_loadedGears.cend()) {
                m_mdlPart->removeModel((*it).mdl);
                m_loadedGears.erase(std::remove_if(m_loadedGears.begin(),
                                                   m_loadedGears.end(),
                                                   [queuedRemoval](const LoadedGear &other) {
                                                       return queuedRemoval.info == other.info;
                                                   }),
                                    m_loadedGears.end());
            }
        }

        m_queuedGearAdditions.clear();
        m_queuedGearRemovals.clear();
    }

    const auto loadBodyPart = [this, &sanitizeMdlPath](int index, CharacterCategory category, auto build_material_path_func) {
        const auto mdlPath = QLatin1String(physis_build_character_path(category, index, currentRace, currentTribe, currentGender));
        auto mdl_data = m_cache.read(mdlPath);

        if (mdl_data.size > 0) {
            auto mdl = physis_mdl_parse(m_cache.platform(), mdl_data);
            if (mdl.p_ptr != nullptr) {
                std::vector<std::pair<std::string, physis_Material>> materials;
                for (uint32_t i = 0; i < mdl.num_material_names; i++) {
                    const char *material_name = mdl.material_names[i];
                    const std::string skinmtrl_path =
                        build_material_path_func(physis_get_race_code(currentRace, currentTribe, currentGender), index, material_name);

                    if (m_cache.exists(QLatin1String(skinmtrl_path.c_str()))) {
                        auto mat = physis_material_parse(m_cache.platform(), m_cache.read(QLatin1String(skinmtrl_path.c_str())));
                        materials.push_back(std::make_pair(skinmtrl_path, mat));
                    }
                }

                Transformation transformation{};
                transformation.scale[0] = 1;
                transformation.scale[1] = 1;
                transformation.scale[2] = 1;

                m_mdlPart->addModel(mdl, true, transformation, sanitizeMdlPath(mdlPath), materials, m_currentLod);
            }
        }
    };

    if (m_face) {
        loadBodyPart(*m_face, CharacterCategory::Face, physis_build_face_material_path);
    }

    if (m_hair) {
        loadBodyPart(*m_hair, CharacterCategory::Hair, physis_build_hair_material_path);
    }

    if (m_ear) {
        loadBodyPart(*m_ear, CharacterCategory::Ear, physis_build_ear_material_path);
    }

    if (m_tail) {
        loadBodyPart(*m_tail, CharacterCategory::Tail, physis_build_tail_material_path);
    }

    m_raceDirty = false;
    m_gearDirty = false;
    m_updating = false;
    m_faceDirty = false;
    m_hairDirty = false;
    m_earDirty = false;
    m_tailDirty = false;
}

bool GearView::needsUpdate() const
{
    return m_gearDirty || m_raceDirty || m_faceDirty || m_hairDirty || m_earDirty || m_tailDirty;
}

QString GearView::getLoadedGearPath() const
{
    if (m_loadedGears.empty()) {
        return {};
    }
    return m_loadedGears[0].path;
}

void GearView::changeEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::EnabledChange:
        m_mdlPart->setEnabled(isEnabled());
        break;
    default:
        break;
    }
    QFrame::changeEvent(event);
}

void GearView::resetMdlPart()
{
    m_mdlPart->clear();

    // Setup some basic three-point lighting
    m_mdlPart->manager()->scene.lights.clear();

    SceneLight keyLight;
    keyLight.type = LightType::Point;
    keyLight.position = glm::vec3(-5, 5, 5);
    keyLight.intensity = 5.0f;
    m_mdlPart->manager()->scene.lights.push_back(keyLight);

    SceneLight backLight;
    backLight.type = LightType::Point;
    backLight.position = glm::vec3(-5, 5, -5);
    backLight.color = glm::vec3(1, 0, 0);
    backLight.intensity = 2.0f;
    m_mdlPart->manager()->scene.lights.push_back(backLight);

    SceneLight fillLight;
    fillLight.type = LightType::Point;
    fillLight.position = glm::vec3(5, 5, 5);
    fillLight.color = glm::vec3(0, 0, 1);
    fillLight.intensity = 3.0f;
    m_mdlPart->manager()->scene.lights.push_back(fillLight);
}

#include "moc_gearview.cpp"
