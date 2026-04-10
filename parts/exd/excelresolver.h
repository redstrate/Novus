// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QStringList>
#include <QVariant>

#include "physis.hpp"

class Schema;

/**
 * Wraps physis_ExcelRow with RAII so it can be freed.
 */
class ScopedExelRow
{
public:
    explicit ScopedExelRow(const physis_ExcelRow &row, uint32_t columnCount);
    ~ScopedExelRow();

    // force move, not copies
    ScopedExelRow(ScopedExelRow &&rhs) noexcept
    {
        m_row = std::exchange(rhs.m_row, {});
        m_columnCount = std::exchange(rhs.m_columnCount, 0);
    }

    ScopedExelRow &operator=(ScopedExelRow &&rhs) noexcept
    {
        m_row = std::exchange(rhs.m_row, {});
        m_columnCount = std::exchange(rhs.m_columnCount, 0);

        return *this;
    }

    ScopedExelRow(const ScopedExelRow &rhs) = delete;
    ScopedExelRow &operator=(const ScopedExelRow &rhs) = delete;

    physis_ExcelRow const &row() const;

private:
    physis_ExcelRow m_row{};
    uint32_t m_columnCount = 0;
};

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
    virtual std::optional<std::pair<QString, ScopedExelRow>> resolveRow(const QStringList &sheetNames, uint32_t row, Language preferredLanguage);

    virtual physis_Field *translateSchemaColumn(const QString &sheetName, physis_ExcelRow const *row, uint32_t column);
};

struct EXDSelector {
    QString name;
    Language preferredLanguage;
};

inline bool operator==(const EXDSelector &a, const EXDSelector &b)
{
    return a.name == b.name && a.preferredLanguage == b.preferredLanguage;
}

inline size_t qHash(const EXDSelector &selector, const size_t seed)
{
    return qHashMulti(seed, selector.name, selector.preferredLanguage);
}

class CachingExcelResolver : public AbstractExcelResolver
{
public:
    explicit CachingExcelResolver(physis_SqPackResource *resource);
    ~CachingExcelResolver() override;

    std::optional<std::pair<QString, ScopedExelRow>> resolveRow(const QStringList &sheetNames, uint32_t row, Language preferredLanguage) override;

    physis_Field *translateSchemaColumn(const QString &sheetName, physis_ExcelRow const *row, uint32_t column) override;

private:
    /**
     * @brief Returns the EXH for a given sheet, loading and caching it as necessary.
     */
    physis_EXH &getCachedEXH(const QString &sheetName);

    /**
     * @brief Returns the sheet for a given selector, loading and caching it as necessary.
     */
    physis_ExcelSheet &getCachedSheet(const physis_EXH &exh, const EXDSelector &selector);

    /**
     * @brief Checks whether this sheet contains said row ID. Returns the page it can be found on, if found.
     */
    std::optional<uint32_t> hasRow(const physis_EXH &exh, uint32_t row) const;

    Language getSuitableLanguage(const physis_EXH &pExh, Language preferredLanguage) const;

    physis_SqPackResource *m_resource = nullptr;
    QHash<QString, physis_EXH> m_cachedEXHs;
    QHash<EXDSelector, physis_ExcelSheet> m_cachedSheets;
};
