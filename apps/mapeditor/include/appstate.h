// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

#include <physis.hpp>

class AppState : public QObject
{
    Q_OBJECT

public:
    explicit AppState(physis_SqPackResource *resource, QObject *parent = nullptr);

    void clear();

    QString basePath;
    std::vector<std::pair<QString, physis_LayerGroup>> lgbFiles;
    QList<uint32_t> visibleLayerIds;
    physis_Terrain terrain;
    QList<uint32_t> visibleTerrainPlates;
    std::optional<physis_InstanceObject const *> selectedObject;
    std::optional<physis_Layer const *> selectedLayer;

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
