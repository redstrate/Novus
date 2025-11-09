// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QStringList>
#include <QVariant>

#include "physis.hpp"

class Schema;
/**
 * @brief Handles resolving and caching Excel sheets. This is meant for quickly looking up sheet data.
 *
 * This is meant to be implemented by an application, and does nothing by default (e.g. returning empty sheets.)
 * It intentionally does nothing by default to reduce CPU, memory and I/O load since most applications typically don't need to cache anything.
 */
class AbstractExcelResolver
{
public:
    virtual ~AbstractExcelResolver() = default;

    /**
     * @brief Resolves a given row from multiple sheets. Returns the sheet name and the requested column data, if found.
     *
     * The first sheet that contains said row ID will be returned.
     */
    virtual std::optional<std::pair<QString, physis_ExcelRow const *>> resolveRow(const QStringList &sheetNames, uint32_t row, Language language);

    virtual physis_ColumnData &translateSchemaColumn(const QString &sheetName, physis_ExcelRow const *row, uint32_t column);
};

struct EXDSelector {
    QString name;
    Language language;
    uint32_t page;
};

inline bool operator==(const EXDSelector &a, const EXDSelector &b)
{
    return a.name == b.name && a.language == b.language && a.page == b.page;
}

inline size_t qHash(const EXDSelector &selector, const size_t seed)
{
    return qHashMulti(seed, selector.name, selector.language, selector.page);
}

class CachingExcelResolver : public AbstractExcelResolver
{
public:
    explicit CachingExcelResolver(SqPackResource *resource);

    std::optional<std::pair<QString, physis_ExcelRow const *>> resolveRow(const QStringList &sheetNames, uint32_t row, Language language) override;

    physis_ColumnData &translateSchemaColumn(const QString &sheetName, physis_ExcelRow const *row, uint32_t column) override;

private:
    /**
     * @brief Returns the EXH for a given sheet, loading and caching it as necessary.
     */
    physis_EXH *getCachedEXH(const QString &sheetName);

    /**
     * @brief Returns the EXD for a given selector, loading and caching it as necessary.
     */
    physis_EXD &getCachedEXD(physis_EXH *exh, const EXDSelector &selector);

    /**
     * @brief Checks whether this sheet contains said row ID. Returns the page it can be found on, if found.
     */
    std::optional<uint32_t> hasRow(const physis_EXH *exh, uint32_t row) const;

    SqPackResource *m_resource = nullptr;
    QHash<QString, physis_EXH *> m_cachedEXHs;
    QHash<EXDSelector, physis_EXD> m_cachedEXDs;
};
