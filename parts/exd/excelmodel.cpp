// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "excelmodel.h"

#include "excelresolver.h"

#include <KLocalizedString>
#include <QDir>
#include <QFont>
#include <QIcon>
#include <QStandardPaths>
#include <magic_enum.hpp>

ExcelModel::ExcelModel(const physis_EXH &exh, const physis_EXD &exd, Schema schema, AbstractExcelResolver *resolver, Language language)
    : m_exh(exh)
    , m_exd(exd)
    , m_schema(schema)
    , m_resolver(resolver)
    , m_language(language)
{
    // We need to count the subrows to get the "true" row count.
    unsigned int regularRowCount = 0;
    for (unsigned int i = 0; i < exd.row_count; i++) {
        const auto &row = exd.rows[i];

        for (unsigned int j = 0; j < row.row_count; j++) {
            // Push a "true" row index for the given subrow. This is just for faster lookups.
            m_rowIndices.emplace_back(regularRowCount, row.row_id, j);
        }

        if (row.row_count > 1) {
            m_hasSubrows = true;
        }

        m_rowCount += row.row_count;
        regularRowCount++;
    }

    // TODO: make this look-up more sensible to reduce calls to indexOf
    // Build a list of sorted indices meant for usage with schemas
    QList<std::pair<uint16_t, uint32_t>> sortedColumns;
    for (uint32_t i = 0; i < exh.column_count; i++) {
        sortedColumns.push_back({exh.column_definitions[i].offset, i});
    }
    std::ranges::sort(sortedColumns);
    // FIXME: is there a way to unzip, maybe with ranges?
    for (uint32_t i = 0; i < sortedColumns.count(); i++) {
        m_sortedColumnIndices.push_back(sortedColumns[i].second);
    }

    Q_ASSERT(m_rowIndices.size() == m_rowCount);
    Q_ASSERT(m_sortedColumnIndices.size() == m_exd.column_count);
}

int ExcelModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return static_cast<int>(m_rowCount);
}

int ExcelModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return static_cast<int>(m_exd.column_count);
}

QVariant ExcelModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        const auto column = m_sortedColumnIndices.indexOf(index.column());
        const auto &data = dataForIndex(index);

        return displayForColumn(column, data);
    }
    if (role == Qt::FontRole) {
        const auto mappedIndex = m_sortedColumnIndices.indexOf(index.column());
        const auto columnName = m_schema.nameForColumn(mappedIndex);

        QFont font;

        // Make font bold to make the display field more obvious.
        font.setBold(m_schema.isDisplayField(columnName));

        // Show an underline to make the resolved column more obvious.
        if (!m_schema.targetSheetsForColumn(mappedIndex).isEmpty()) {
            font.setUnderline(true);
        }

        return font;
    }
    if (role == Qt::ToolTipRole) {
        const auto mappedIndex = m_sortedColumnIndices.indexOf(index.column());
        if (!m_schema.targetSheetsForColumn(mappedIndex).isEmpty()) {
            // Get the resolved sheet and its row id.
            // TODO: use multiData for this
            const auto resolvedSheet = data(index, ExcelRoles::ResolvedSheetRole).toString();
            if (resolvedSheet.isEmpty()) {
                return {};
            }

            const auto resolvedRow = data(index, ExcelRoles::ResolvedRowRole).toInt();

            return i18n("Links to %1#%2").arg(resolvedSheet).arg(resolvedRow);
        }
    }
    if (role == ResolvedSheetRole) {
        const auto mappedIndex = m_sortedColumnIndices.indexOf(index.column());
        const auto targetSheets = m_schema.targetSheetsForColumn(mappedIndex);
        if (targetSheets.isEmpty()) {
            return {};
        }

        const auto targetRowId = data(index, ResolvedRowRole).toInt();

        if (const auto value = m_resolver->resolveRow(targetSheets, targetRowId, m_language)) {
            return value->first;
        }
    }
    if (role == ResolvedRowRole) {
        const auto &data = dataForIndex(index);

        // TODO: de-duplicate with the identical switch below
        uint32_t targetRowId;
        switch (data.tag) {
        case physis_ColumnData::Tag::Int8:
            targetRowId = data.int8._0;
            break;
        case physis_ColumnData::Tag::UInt8:
            targetRowId = data.u_int8._0;
            break;
        case physis_ColumnData::Tag::Int16:
            targetRowId = data.int16._0;
            break;
        case physis_ColumnData::Tag::UInt16:
            targetRowId = data.u_int16._0;
            break;
        case physis_ColumnData::Tag::Int32:
            targetRowId = data.int32._0;
            break;
        case physis_ColumnData::Tag::UInt32:
            targetRowId = data.u_int32._0;
            break;
        case physis_ColumnData::Tag::Int64:
            targetRowId = data.int64._0;
            break;
        case physis_ColumnData::Tag::UInt64:
            targetRowId = data.u_int64._0;
            break;
        default:
            return {};
        }

        return targetRowId;
    }

    return {};
}

QVariant ExcelModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Vertical) {
            const auto [_, row_id, subrow_id] = m_rowIndices[section];
            if (m_hasSubrows) {
                return QStringLiteral("%1.%2").arg(row_id).arg(subrow_id);
            }
            return QString::number(row_id);
        }

        const auto mappedIndex = m_sortedColumnIndices.indexOf(section);
        return m_schema.nameForColumn(mappedIndex);
    }
    if (role == Qt::ToolTipRole && orientation == Qt::Horizontal) {
        const auto column = m_exh.column_definitions[section];

        QString toolTip;
        toolTip.append(i18n("Index: %1").arg(section));
        toolTip.append(i18n("\nSchema Index: %1").arg(m_sortedColumnIndices.indexOf(section)));
        toolTip.append(i18n("\nOffset: %1 (0x%2)").arg(column.offset).arg(QString::number(column.offset, 16)));
        toolTip.append(i18n("\nType: %1").arg(magic_enum::enum_name(column.data_type)));

        const QString comment = m_schema.comment(m_sortedColumnIndices.indexOf(section));
        if (!comment.isEmpty()) {
            toolTip.append(QStringLiteral("\n\n%1").arg(comment));
        }

        return toolTip;
    }
    if (role == Qt::FontRole && orientation == Qt::Horizontal) {
        // Make font bold to make the display field more obvious.
        const auto mappedIndex = m_sortedColumnIndices.indexOf(section);
        const auto columnName = m_schema.nameForColumn(mappedIndex);

        QFont font;
        font.setBold(m_schema.isDisplayField(columnName));

        return font;
    }
    if (role == Qt::DecorationRole && orientation == Qt::Horizontal) {
        if (m_schema.displayFieldIndex().value_or(-1) == section) {
            return QIcon::fromTheme(QStringLiteral("favorite-symbolic"));
        }

        const QString comment = m_schema.comment(m_sortedColumnIndices.indexOf(section));
        if (!comment.isEmpty()) {
            return QIcon::fromTheme(QStringLiteral("comment-symbolic"));
        }
    }

    return {};
}

QVariant ExcelModel::displayForColumn(const uint32_t column, const physis_ColumnData &data) const
{
    // Check to see if there's any targets
    const auto targetSheets = m_schema.targetSheetsForColumn(column);
    if (!targetSheets.isEmpty()) {
        uint32_t targetRowId;
        switch (data.tag) {
        case physis_ColumnData::Tag::Int8:
            targetRowId = data.int8._0;
            break;
        case physis_ColumnData::Tag::UInt8:
            targetRowId = data.u_int8._0;
            break;
        case physis_ColumnData::Tag::Int16:
            targetRowId = data.int16._0;
            break;
        case physis_ColumnData::Tag::UInt16:
            targetRowId = data.u_int16._0;
            break;
        case physis_ColumnData::Tag::Int32:
            targetRowId = data.int32._0;
            break;
        case physis_ColumnData::Tag::UInt32:
            targetRowId = data.u_int32._0;
            break;
        case physis_ColumnData::Tag::Int64:
            targetRowId = data.int64._0;
            break;
        case physis_ColumnData::Tag::UInt64:
            targetRowId = data.u_int64._0;
            break;
        default:
            // There can't be columns that point to another Excel row with something like a string, so something went wrong somewhere
            return i18n("Unknown Target?");
        }

        if (const auto value = m_resolver->resolveRow(targetSheets, targetRowId, m_language)) {
            const auto [sheetName, data] = *value;

            // TODO: de-duplicate with the code in EXDPart
            const QDir dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
            const QDir schemaDir = dataDir.absoluteFilePath(QStringLiteral("schema"));

            Schema schema(schemaDir.absoluteFilePath(QStringLiteral("%1.yml").arg(sheetName)));

            if (auto displayFieldIndex = schema.displayFieldIndex()) {
                return displayForData(data->column_data[0]); // TODO: use display field
            }
        }
    }

    // Normal data
    return displayForData(data);
}

QVariant ExcelModel::displayForData(const physis_ColumnData &data)
{
    QString columnString;
    switch (data.tag) {
    case physis_ColumnData::Tag::String:
        columnString = QString::fromStdString(data.string._0);
        break;
    case physis_ColumnData::Tag::Bool:
        columnString = data.bool_._0 ? i18nc("Value is true", "True") : i18nc("Value is false", "False");
        break;
    case physis_ColumnData::Tag::Int8:
        columnString = QString::number(data.int8._0);
        break;
    case physis_ColumnData::Tag::UInt8:
        columnString = QString::number(data.u_int8._0);
        break;
    case physis_ColumnData::Tag::Int16:
        columnString = QString::number(data.int16._0);
        break;
    case physis_ColumnData::Tag::UInt16:
        columnString = QString::number(data.u_int16._0);
        break;
    case physis_ColumnData::Tag::Int32:
        columnString = QString::number(data.int32._0);
        break;
    case physis_ColumnData::Tag::UInt32:
        columnString = QString::number(data.u_int32._0);
        break;
    case physis_ColumnData::Tag::Float32:
        columnString = QString::number(data.float32._0);
        break;
    case physis_ColumnData::Tag::Int64:
        columnString = QString::number(data.int64._0);
        break;
    case physis_ColumnData::Tag::UInt64:
        columnString = QString::number(data.u_int64._0);
        break;
    }

    return columnString;
}

physis_ColumnData &ExcelModel::dataForIndex(const QModelIndex &index) const
{
    const auto [row_index, _, subrow_id] = m_rowIndices[index.row()];
    const auto &row = m_exd.rows[row_index];
    const auto &subrow = row.row_data[subrow_id];

    const auto column = m_sortedColumnIndices.indexOf(index.column());
    return subrow.column_data[index.column()];
}

#include "moc_excelmodel.cpp"
