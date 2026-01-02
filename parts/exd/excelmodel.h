// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "schema.h"

#include <QAbstractTableModel>

#include <physis.hpp>

class AbstractExcelResolver;

class ExcelModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    ExcelModel(const physis_EXH &exh, const physis_ExcelSheetPage &page, Schema schema, AbstractExcelResolver *resolver, Language language);

    enum ExcelRoles {
        ResolvedSheetRole = Qt::UserRole,
        ResolvedRowRole,
    };
    Q_ENUM(ExcelRoles);

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;

    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    /**
     * @brief Returns a nice display for a given column data, including resolving other sheets..
     */
    QVariant displayForColumn(uint32_t column, const physis_ColumnData &data) const;

    /**
     * @brief Returns a nice display for a given column data.
     */
    static QVariant displayForData(const physis_ColumnData &data);

    /**
     * @brief Returns the column data for a given QModelIndex.
     */
    physis_ColumnData &dataForIndex(const QModelIndex &index) const;

    physis_EXH m_exh;
    physis_ExcelSheetPage m_page;
    uint32_t m_rowCount = 0;
    std::vector<std::tuple<int, int, int>> m_rowIndices;
    Schema m_schema; // TODO: don't copy
    bool m_hasSubrows = false;
    // Mapping from a regular index to a list of columns that were sorted by offset
    QList<uint32_t> m_sortedColumnIndices;
    AbstractExcelResolver *m_resolver;
    Language m_language;
};
