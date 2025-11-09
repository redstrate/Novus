// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <physis.hpp>

#include <QString>

class Schema
{
public:
    Schema(const QString &path);

    /**
     * @brief Returns a human-readable name for the given column.
     *
     * @note This is the index into the *schema* and is sorted by column offset.
     */
    QString nameForColumn(int index) const;

    /**
     * @brief Returns a nice display for a given column data.
     */
    QVariant displayForData(const physis_ColumnData &data) const;

    /**
     * @brief Returns true if this column name is supposed to be the main display field.
     */
    bool isDisplayField(const QString &name) const;

private:
    struct Field
    {
        QString name;
    };
    std::vector<Field> m_fields;
    QString m_displayField;
};
