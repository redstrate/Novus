// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

#include <QHash>
#include <physis.hpp>

/// Represents a "SCN1" section, which could be a complete level (LVB) or a prefab (SGB). These can also be infinitely nested.
class ObjectScene
{
public:
    void clear();
    void load(physis_SqPackResource *data, const physis_ScnSection &section);

    QString basePath;
    QList<physis_ScnTimeline> embeddedTimelines;
    physis_Terrain terrain = {};
    std::vector<std::pair<QString, physis_LayerGroup>> lgbFiles;
    std::vector<physis_ScnLayerGroup> embeddedLgbs;

    /// Key is the ID of the SGB instance.
    QHash<uint32_t, ObjectScene> nestedScenes;

private:
    void processSharedGroup(physis_SqPackResource *data, uint32_t instanceId, const char *path);
    void processScnLayerGroup(physis_SqPackResource *data, const physis_ScnLayerGroup &group);
};

class SceneState : public QObject
{
    Q_OBJECT

public:
    explicit SceneState(physis_SqPackResource *resource, QObject *parent = nullptr);

    void load(physis_SqPackResource *data, const physis_ScnSection &section);
    void clear();

    /// The root scene.
    ObjectScene rootScene;
    QList<uint32_t> visibleLayerIds;
    std::optional<physis_InstanceObject const *> selectedObject;
    std::optional<physis_Layer const *> selectedLayer;
    QList<uint32_t> visibleTerrainPlates;

    /**
     * @return The name for this Event NPC. If not found, then a generic one.
     */
    QString lookupENpcName(uint32_t id) const;

    /**
     * @return The name for this Event Object. If not found, then a generic one.
     */
    QString lookupEObjName(uint32_t id) const;

Q_SIGNALS:
    void mapLoaded();
    void visibleLayerIdsChanged();
    void visibleTerrainPlatesChanged();
    void selectionChanged();

private:
    physis_ExcelSheet m_enpcResidentSheet;
    physis_ExcelSheet m_eobjNameSheet;
};
