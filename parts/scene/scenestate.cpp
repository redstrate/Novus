// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "scenestate.h"

#include <KLocalizedString>

SceneState::SceneState(physis_SqPackResource *resource, QObject *parent)
    : QObject(parent)
    , terrain({})
{
    // ENPC
    {
        const auto exhFile = physis_sqpack_read(resource, "exd/enpcresident.exh");
        if (exhFile.size == 0) {
            qWarning() << "Failed to read exd/epncresident.exh";
        } else {
            const auto exh = physis_exh_parse(resource->platform, exhFile);
            if (!exh.p_ptr) {
                qWarning() << "Failed to parse exd/enpcresident.exh";
            } else {
                m_enpcResidentSheet = physis_sqpack_read_excel_sheet(resource, "ENpcResident", &exh, Language::English);
            }
        }
    }

    // EOBJ
    {
        const auto exhFile = physis_sqpack_read(resource, "exd/eobjname.exh");
        if (exhFile.size == 0) {
            qWarning() << "Failed to read exd/eobjname.exh";
        } else {
            const auto exh = physis_exh_parse(resource->platform, exhFile);
            if (!exh.p_ptr) {
                qWarning() << "Failed to parse exd/eobjname.exh";
            } else {
                m_eobjNameSheet = physis_sqpack_read_excel_sheet(resource, "EObjName", &exh, Language::English);
            }
        }
    }
}

void SceneState::load(physis_SqPackResource *data, const physis_ScnSection &section)
{
    basePath = QString::fromLatin1(section.general.bg_path);

    QString bgPath = QStringLiteral("%1/bgplate/").arg(section.general.bg_path);
    std::string bgPathStd = bgPath.toStdString() + "terrain.tera";

    auto tera_buffer = physis_sqpack_read(data, bgPathStd.c_str());
    if (tera_buffer.size > 0) {
        terrain = physis_terrain_parse(data->platform, tera_buffer);
    } else {
        qWarning() << "Failed to load terrain" << bgPathStd;
    }

    const auto loadLgb = [this, data](const char *path) {
        const auto bg_buffer = physis_sqpack_read(data, path);
        if (bg_buffer.size > 0) {
            const auto lgb = physis_lgb_parse(data->platform, bg_buffer);
            if (lgb.num_chunks > 0) {
                lgbFiles.push_back({QString::fromLatin1(path), lgb});
            }
        }
    };

    for (uint32_t i = 0; i < section.num_lgb_paths; i++) {
        loadLgb(section.lgb_paths[i]);
    }

    for (uint32_t i = 0; i < section.num_layer_groups; i++) {
        embeddedLgbs.push_back(section.layer_groups[i]);
    }

    // Load terrain and bg by default
    for (int i = 0; i < terrain.num_plates; i++) {
        visibleTerrainPlates.push_back(i);
    }

    for (const auto &[name, lgb] : lgbFiles) {
        if (name.endsWith(QStringLiteral("bg.lgb"))) {
            for (uint32_t i = 0; i < lgb.num_chunks; i++) {
                for (uint32_t j = 0; j < lgb.chunks[i].num_layers; j++) {
                    // Skip festival-specific layers
                    if (lgb.chunks[i].layers[j].festival_id == 0) {
                        visibleLayerIds.push_back(lgb.chunks[i].layers[j].id);
                    }
                }
            }
            break;
        }
    }

    Q_EMIT mapLoaded();
}

void SceneState::clear()
{
    basePath.clear();
    lgbFiles.clear();
    visibleLayerIds.clear();
    terrain = {};
    visibleTerrainPlates.clear();
    selectedObject.reset();
    selectedLayer.reset();
    embeddedLgbs.clear();
}

QString SceneState::lookupENpcName(const uint32_t id) const
{
    auto row = physis_excel_get_row(&m_enpcResidentSheet, id);
    if (row.columns && strlen(row.columns[0].string._0) > 0) {
        return QString::fromLatin1(row.columns[0].string._0);
    }
    return i18n("Event NPC");
}

QString SceneState::lookupEObjName(const uint32_t id) const
{
    auto row = physis_excel_get_row(&m_eobjNameSheet, id);
    if (row.columns && strlen(row.columns[0].string._0) > 0) {
        return QString::fromLatin1(row.columns[0].string._0);
    }
    return i18n("Event Object");
}
