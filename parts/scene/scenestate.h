// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

#include <QHash>
#include <physis.hpp>

class SceneState : public QObject
{
    Q_OBJECT

public:
    explicit SceneState(physis_SqPackResource *resource, QObject *parent = nullptr);

    void load(physis_SqPackResource *data, const physis_ScnSection &section);
    void clear();

    QString basePath;
    std::vector<std::pair<QString, physis_LayerGroup>> lgbFiles;
    std::vector<physis_ScnLayerGroup> embeddedLgbs;
    QList<uint32_t> visibleLayerIds;
    physis_Terrain terrain;
    QList<uint32_t> visibleTerrainPlates;
    std::optional<physis_InstanceObject const *> selectedObject;
    std::optional<physis_Layer const *> selectedLayer;
    QHash<QString, physis_Sgb> nestedSharedGroups;

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
    void processSharedGroup(physis_SqPackResource *data, const char *path);
    void processScnLayerGroup(physis_SqPackResource *data, const physis_ScnLayerGroup &group);

    physis_ExcelSheet m_enpcResidentSheet;
    physis_ExcelSheet m_eobjNameSheet;
};
