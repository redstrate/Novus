// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "excelresolver.h"

#include "magic_enum.hpp"
#include "schema.h"
#include "settings.h"

#include <KLocalizedString>

ScopedExelRow::ScopedExelRow(const physis_ExcelRow &row, uint32_t columnCount)
    : m_row(row)
    , m_columnCount(columnCount)
{
}

ScopedExelRow::~ScopedExelRow()
{
    physis_free_row(&m_row, m_columnCount);
}

physis_ExcelRow const &ScopedExelRow::row() const
{
    return m_row;
}

std::optional<std::pair<QString, ScopedExelRow>>
AbstractExcelResolver::resolveRow(const QStringList &sheetNames, const uint32_t row, const Language preferredLanguage)
{
    Q_UNUSED(sheetNames)
    Q_UNUSED(row)
    Q_UNUSED(preferredLanguage)
    return std::nullopt;
}

physis_Field *AbstractExcelResolver::translateSchemaColumn(const QString &sheetName, physis_ExcelRow const *row, uint32_t column)
{
    Q_UNUSED(sheetName)
    Q_UNUSED(column)
    Q_UNUSED(row)
    return nullptr;
}

CachingExcelResolver::CachingExcelResolver(physis_SqPackResource *resource)
    : m_resource(resource)
{
}

CachingExcelResolver::~CachingExcelResolver()
{
    for (const auto [_, exh] : m_cachedEXHs.asKeyValueRange()) {
        physis_exh_free(&exh);
    }
    for (const auto [_, sheet] : m_cachedSheets.asKeyValueRange()) {
        physis_sqpack_free_excel_sheet(&sheet);
    }
}

std::optional<std::pair<QString, ScopedExelRow>>
CachingExcelResolver::resolveRow(const QStringList &sheetNames, const uint32_t row, const Language preferredLanguage)
{
    for (const auto &sheetName : sheetNames) {
        const auto exh = getCachedEXH(sheetName);
        Q_ASSERT(exh.p_ptr);

        if (const auto page = hasRow(exh, row)) {
            const auto exd = getCachedSheet(exh,
                                            EXDSelector{
                                                .name = sheetName,
                                                .preferredLanguage = preferredLanguage,
                                            });
            if (exd.p_ptr) {
                const auto exdRow = physis_excel_get_row(&exd, row);
                return std::pair{sheetName, ScopedExelRow(exdRow, exh.column_count)};
            }

            qWarning() << "Failed to fetch resolved row" << sheetName << row << "???";
        }
    }
    return AbstractExcelResolver::resolveRow(sheetNames, row, preferredLanguage);
}

physis_Field *CachingExcelResolver::translateSchemaColumn(const QString &sheetName, physis_ExcelRow const *row, const uint32_t column)
{
    const auto exh = getCachedEXH(sheetName);
    Q_ASSERT(exh.p_ptr);

    // Build a list of sorted indices meant for usage with schemas
    QList<std::pair<uint16_t, uint32_t>> sortedColumns;
    for (uint32_t i = 0; i < exh.column_count; i++) {
        sortedColumns.push_back({exh.column_definitions[i].offset, i});
    }
    std::ranges::sort(sortedColumns);

    QList<uint32_t> sortedColumnIndices;

    // FIXME: is there a way to unzip, maybe with ranges?
    for (uint32_t i = 0; i < sortedColumns.count(); i++) {
        sortedColumnIndices.push_back(sortedColumns[i].second);
    }

    return &row->columns[sortedColumnIndices[column]];
}

physis_EXH &CachingExcelResolver::getCachedEXH(const QString &sheetName)
{
    if (!m_cachedEXHs.contains(sheetName)) {
        const auto path = QStringLiteral("exd/%1.exh").arg(sheetName.toLower());
        const auto pathStd = path.toStdString();

        const auto file = physis_sqpack_read(m_resource, pathStd.c_str());
        const auto exh = physis_exh_parse(m_resource->platform, file);
        m_cachedEXHs.insert(sheetName, exh);
        physis_free_file(&file);
    }

    return m_cachedEXHs[sheetName];
}

physis_ExcelSheet &CachingExcelResolver::getCachedSheet(const physis_EXH &exh, const EXDSelector &selector)
{
    if (!m_cachedSheets.contains(selector)) {
        const auto language = getSuitableLanguage(exh, selector.preferredLanguage);
        const auto exd = physis_sqpack_read_excel_sheet(m_resource, selector.name.toStdString().c_str(), &exh, language);
        if (exd.p_ptr) {
            m_cachedSheets.insert(selector, exd);
        } else {
            qWarning() << "Failed to load" << selector.name << "with language" << magic_enum::enum_name(language);
        }
    }

    return m_cachedSheets[selector];
}

std::optional<uint32_t> CachingExcelResolver::hasRow(const physis_EXH &exh, const uint32_t row) const
{
    for (uint32_t i = 0; i < exh.page_count; i++) {
        const auto page = exh.pages[i];
        if (row >= page.start_id && row < page.row_count) {
            return i;
        }
    }

    return std::nullopt;
}

Language CachingExcelResolver::getSuitableLanguage(const physis_EXH &pExh, Language preferredLanguage) const
{
    // Find the preferred language first (if not None, to handle none->localized sheet cases)
    if (preferredLanguage != Language::None) {
        for (uint32_t i = 0; i < pExh.language_count; i++) {
            if (pExh.languages[i] == preferredLanguage) {
                return preferredLanguage;
            }
        }
    }

    // Fallback to None or the default language
    for (uint32_t i = 0; i < pExh.language_count; i++) {
        if (pExh.languages[i] == getLanguage()) {
            return pExh.languages[i];
        }
    }

    // Fallback to None if that fails
    for (uint32_t i = 0; i < pExh.language_count; i++) {
        if (pExh.languages[i] == Language::None) {
            return pExh.languages[i];
        }
    }

    // This should never happen, Excel sheets *always* have a language.
    Q_UNREACHABLE();
}
