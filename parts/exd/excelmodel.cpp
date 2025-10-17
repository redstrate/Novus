// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "excelmodel.h"

ExcelModel::ExcelModel(const physis_EXD &exd, Schema schema) : m_exd(exd), m_schema(schema)
{
    // We need to count the subrows to get the "true" row count.
    unsigned int regularRowCount = 0;
    for (unsigned int i = 0; i < exd.row_count; i++) {
        const auto &row = exd.rows[i];

        for (unsigned int j = 0; j < row.row_count; j++) {
            // Push a "true" row index for the given subrow. This is just for faster lookups.
            m_rowIndices.push_back({regularRowCount, row.row_id, j});
        }

        m_rowCount += row.row_count;
        regularRowCount++;
    }

    Q_ASSERT(m_rowIndices.size() == m_rowCount);
}

int ExcelModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_rowCount;
}

int ExcelModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_exd.column_count;
}

QVariant ExcelModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        // Look up the original row id, regardless of subrow
        const auto [row_index, _, subrow_id] = m_rowIndices[index.row()];
        const auto &row = m_exd.rows[row_index];
        const auto &subrow = row.row_data[subrow_id];

        const auto &column = subrow.column_data[index.column()];
        return m_schema.displayForData(column);
    }

    return {};
}

QVariant ExcelModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Vertical) {
            const auto [_, row_id, subrow_id] = m_rowIndices[section];
            return QStringLiteral("%1.%2").arg(row_id).arg(subrow_id);
        }

        return m_schema.nameForColumn(section);
    }

    return {};
}

#include "moc_excelmodel.cpp"
