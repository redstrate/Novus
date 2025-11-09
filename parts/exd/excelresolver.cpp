// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "excelresolver.h"

#include "schema.h"

#include <KLocalizedString>

std::optional<std::pair<QString, physis_ExcelRow const *>>
AbstractExcelResolver::resolveRow(const QStringList &sheetNames, const uint32_t row, const Language language)
{
    Q_UNUSED(sheetNames)
    Q_UNUSED(row)
    Q_UNUSED(language)
    return std::nullopt;
}

CachingExcelResolver::CachingExcelResolver(SqPackResource *resource)
    : m_resource(resource)
{
}

std::optional<std::pair<QString, physis_ExcelRow const *>>
CachingExcelResolver::resolveRow(const QStringList &sheetNames, const uint32_t row, const Language language)
{
    for (const auto &sheetName : sheetNames) {
        const auto exh = getCachedEXH(sheetName);
        Q_ASSERT(exh);

        if (const auto page = hasRow(exh, row)) {
            const auto exd = getCachedEXD(exh,
                                          EXDSelector{
                                              .name = sheetName,
                                              .language = language,
                                              .page = *page,
                                          });
            if (exd.p_ptr) {
                const auto exdRow = physis_exd_get_row(&exd, row);
                return std::pair{sheetName, exdRow};
            }

            qWarning() << "Failed to fetch resolved row" << sheetName << row << "???";
        }
    }
    return AbstractExcelResolver::resolveRow(sheetNames, row, language);
}

physis_EXH *CachingExcelResolver::getCachedEXH(const QString &sheetName)
{
    if (!m_cachedEXHs.contains(sheetName)) {
        const auto path = QStringLiteral("exd/%1.exh").arg(sheetName.toLower());
        const auto pathStd = path.toStdString();

        const auto file = physis_gamedata_extract_file(m_resource, pathStd.c_str());
        const auto exh = physis_parse_excel_sheet_header(file);
        m_cachedEXHs.insert(sheetName, exh);
    }

    return m_cachedEXHs.value(sheetName);
}

physis_EXD &CachingExcelResolver::getCachedEXD(physis_EXH *exh, const EXDSelector &selector)
{
    if (!m_cachedEXDs.contains(selector)) {
        auto exd = physis_gamedata_read_excel_sheet(m_resource, selector.name.toStdString().c_str(), exh, selector.language, selector.page);
        m_cachedEXDs.insert(selector, exd);
    }

    return m_cachedEXDs[selector];
}

std::optional<uint32_t> CachingExcelResolver::hasRow(const physis_EXH *exh, const uint32_t row) const
{
    for (uint32_t i = 0; i < exh->page_count; i++) {
        const auto page = exh->pages[i];
        if (row >= page.start_id && row < page.row_count) {
            return i;
        }
    }

    return std::nullopt;
}
