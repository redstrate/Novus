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
    QString nameForColumn(uint32_t index) const;

    /**
     * @brief Returns a list of target sheets for this column.
     *
     * @note Only makes sense and returns non-empty for Links.
     */
    QStringList targetSheetsForColumn(uint32_t index) const;

    /**
     * @brief Returns true if this column name is supposed to be the main display field.
     */
    bool isDisplayField(const QString &name) const;

    /**
     * @brief Returns the index in the schema fields where the display field is located.
     *
     * Returns none if there isn't a display field set.
     */
    std::optional<int> displayFieldIndex() const;

    /**
     * @brief Returns the comment for this field. Empty if none is provided.
     */
    QString comment(uint32_t index) const;

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
        QString comment;
    };
    std::vector<Field> m_fields;
    QString m_displayField;
};
