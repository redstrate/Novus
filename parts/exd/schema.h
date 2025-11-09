// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <physis.hpp>

#include <QString>
#include <QStringList>

class Schema
{
public:
    explicit Schema(const QString &path);

    /**
     * @brief Returns a human-readable name for the given column.
     *
     * @note This is the index into the *schema* and is sorted by column offset.
     */
    QString nameForColumn(int index) const;

    /**
     * @brief Returns a list of target sheets for this column.
     *
     * @note Only makes sense and returns non-empty for Links.
     */
    QStringList targetSheetsForColumn(int index) const;

    /**
     * @brief Returns true if this column name is supposed to be the main display field.
     */
    bool isDisplayField(const QString &name) const;

private:
    struct Field
    {
        QString name;

        enum class Type {
            Single,
            Link,
        };
        Type type = Type::Single;

        QStringList targetSheets;
    };
    std::vector<Field> m_fields;
    QString m_displayField;
};
