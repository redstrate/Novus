// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appstate.h"

#include <KLocalizedString>

AppState::AppState(SqPackResource *resource, QObject *parent)
    : QObject(parent)
{
    auto exh = physis_parse_excel_sheet_header(physis_gamedata_extract_file(resource, "exd/enpcresident.exh"));
    Q_ASSERT(exh->page_count == 1);

    m_enpcResidentPage = physis_gamedata_read_excel_sheet(resource, "ENpcResident", exh, Language::English, 0);
}

QString AppState::lookupENpcName(const uint32_t id)
{
    auto row = physis_exd_get_row(&m_enpcResidentPage, id);
    if (row->column_data && strlen(row->column_data[0].string._0) > 0) {
        return QString::fromLatin1(row->column_data[0].string._0);
    }
    return i18n("Event NPC");
}
