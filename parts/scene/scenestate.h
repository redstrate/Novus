// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "animation.h"

#include <QObject>

#include <QHash>
#include <glm/vec3.hpp>
#include <physis.hpp>

class FileCache;
class Animation;

struct DropInGatheringPoint {
    uint32_t baseId;
};

struct DropInBattleNpc {
    uint32_t baseId;
    uint32_t nameId;
    uint32_t hp;
    uint32_t level;
    bool nonpop;
    uint32_t aggressionMode;
    uint32_t gimmickId;
    uint32_t maxLinks;
    uint32_t linkFamily;
    uint32_t linkRange;
};

struct DropInObject {
    uint32_t instanceId;
    glm::vec3 position;
    float rotation;

    std::variant<DropInGatheringPoint, DropInBattleNpc, std::monostate> data;
};

struct DropInLayer {
    QString name;
    QList<DropInObject> objects;
};

struct DropIn {
    QString appends;
    QList<DropInLayer> layers;
};

/// Represents a "SCN1" section, which could be a complete level (LVB) or a prefab (SGB). These can also be infinitely nested.
class ObjectScene
{
public:
    ~ObjectScene();

    void load(FileCache &cache, const physis_ScnSection &section);

    /// Transformation to apply to all subsequent children.
    Transformation transformation = {
        .translation = {0, 0, 0},
        .rotation = {0, 0, 0},
        .scale = {1, 1, 1},
    };
    Transformation combinedTransformation = {
        .translation = {0, 0, 0},
        .rotation = {0, 0, 0},
        .scale = {1, 1, 1},
    };
    QString basePath;
    QList<physis_ScnTimeline> embeddedTimelines;
    physis_Terrain terrain = {};
    QString terrainPath;
    std::vector<std::pair<QString, physis_LayerGroup>> lgbFiles;
    std::vector<physis_ScnLayerGroup> embeddedLgbs;
    Animation animation;
    std::vector<ScnSGActionControllerDescriptor> actionDescriptors;
    std::vector<std::pair<QString, DropIn>> dropIns;
    physis_Sgb sgb{}; // so it can be freed later
    uint32_t originatingSgbLayerId = 0;
    std::unordered_map<std::string, physis_Material> cachedMaterials;

    /// Key is the ID of the SGB instance.
    std::unordered_map<uint32_t, ObjectScene> nestedScenes;

    Transformation locateGameObject(uint32_t instanceId) const;
    Transformation locateGameObjectByBaseId(uint32_t baseId) const;

    bool isSgb() const;

private:
    void processSharedGroup(FileCache &cache, uint32_t instanceId, uint32_t layerId, const Transformation &transformation, const char *path);
    void processScnLayerGroup(FileCache &cache, const physis_ScnLayerGroup &group);
};

class SceneState : public QObject
{
    Q_OBJECT

public:
    explicit SceneState(FileCache &cache, QObject *parent = nullptr);
    ~SceneState() override;

    void load(FileCache &cache, const physis_ScnSection &section);
    void loadDropIn(const QString &path);
    void saveDropIns();
    void clear();
    void showAll();

    /// The root scene.
    ObjectScene rootScene;
    QList<uint32_t> visibleLayerIds;
    std::optional<physis_InstanceObject *> selectedObject;
    std::optional<physis_Layer const *> selectedLayer;
    std::optional<physis_ScnTimeline const *> selectedTimeline;
    std::optional<ScnSGActionControllerDescriptor const *> selectedAction;
    std::optional<QString> selectedLgb;
    std::optional<QString> selectedTera;
    std::optional<DropInObject *> selectedDropInObject;
    QList<uint32_t> visibleTerrainPlates;

    /**
     * @return The name for this Event NPC. If not found, then a generic one.
     */
    QString lookupENpcName(uint32_t id) const;

    /**
     * @return The name for this Event Object. If not found, then a generic one.
     */
    QString lookupEObjName(uint32_t id) const;

    /**
     * @return The name for this Battle NPC. If not found, then a generic one.
     */
    QString lookupBNpcName(uint32_t id) const;

    float longestAnimationTime() const;

    void updateAllAnimations(float time);

    FileCache &cache() const;

Q_SIGNALS:
    void mapLoaded();
    void visibleLayerIdsChanged();
    void visibleTerrainPlatesChanged();
    void selectionChanged();
    void selectObject(uint32_t objectId);

private:
    void processLongestAnimationTime(const ObjectScene &scene);
    void processUpdateAnimation(ObjectScene &scene, float time);
    void showAllInScene(const ObjectScene &scene);

    physis_ExcelSheet m_enpcResidentSheet;
    physis_ExcelSheet m_eobjNameSheet;
    physis_ExcelSheet m_bnpcNameSheet;
    float m_longestAnimationTime = 0.0f;
    FileCache &m_cache;
};
