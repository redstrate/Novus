// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QObject>

#include <physis.hpp>

class AppState : public QObject
{
    Q_OBJECT

public:
    explicit AppState(SqPackResource *resource, QObject *parent = nullptr);

    QString basePath;
    std::vector<std::pair<QString, physis_LayerGroup>> lgbFiles;
    QList<uint32_t> visibleLayerIds;
    std::optional<physis_InstanceObject const *> selectedObject;
    std::optional<physis_Layer const *> selectedLayer;

    /**
     * @return The name for this Event NPC. If not found, then a generic one.
     */
    QString lookupENpcName(uint32_t id);

    /**
     * @return The name for this Event Object. If not found, then a generic one.
     */
    QString lookupEObjName(uint32_t id);

Q_SIGNALS:
    void mapLoaded();
    void visibleLayerIdsChanged();
    void selectionChanged();

private:
    physis_EXD m_enpcResidentPage;
    physis_EXD m_eobjNamePage;
};
