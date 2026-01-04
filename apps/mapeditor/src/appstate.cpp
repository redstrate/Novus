// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appstate.h"

#include <KLocalizedString>

AppState::AppState(physis_SqPackResource *resource, QObject *parent)
    : QObject(parent)
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

void AppState::clear()
{
    basePath.clear();
    lgbFiles.clear();
    visibleLayerIds.clear();
    terrain = {};
    visibleTerrainPlates.clear();
    selectedObject.reset();
    selectedLayer.reset();
}

QString AppState::lookupENpcName(const uint32_t id) const
{
    auto row = physis_excel_get_row(&m_enpcResidentSheet, id);
    if (row.row_data && strlen(row.row_data[0].column_data[0].string._0) > 0) {
        return QString::fromLatin1(row.row_data[0].column_data[0].string._0);
    }
    return i18n("Event NPC");
}

QString AppState::lookupEObjName(const uint32_t id) const
{
    auto row = physis_excel_get_row(&m_eobjNameSheet, id);
    if (row.row_data && strlen(row.row_data[0].column_data[0].string._0) > 0) {
        return QString::fromLatin1(row.row_data[0].column_data[0].string._0);
    }
    return i18n("Event Object");
}
