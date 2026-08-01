// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QVariant>

#ifndef _RYML_SINGLE_HEADER_AMALGAMATED_HPP_
#include <rapidyaml-0.10.0.hpp>
#endif

class Schema
{
public:
    explicit Schema(const QString &path);

    /**
     * @return The path to the schema for a given sheet name.
     */
    static QString getPath(const QString &name);

    /**
     * @brief Returns a human-readable name for the given column.
     *
     * @note This is the index into the *schema* and is sorted by column offset.
     */
    QString nameForColumn(uint32_t index) const;

    /**
     * @brief Reverse of nameForColumn.
     */
    std::optional<uint32_t> indexForName(const QString &name) const;

    /**
     * @brief Returns a list of target sheets for this column.
     *
     * @note Only makes sense and returns non-empty for Links.
     */
    QStringList targetSheetsForColumn(uint32_t index, const std::optional<QVariant> &context) const;

    /**
     * @brief Returns the name of the column needed to resolve the sheet for targetSheetsForColumn.
     */
    QString neededContextForColumn(uint32_t index) const;

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
    struct Condition {
        QString switchColumn;
        QHash<int, QList<QString>> cases; // TODO: is it only ints supported in cases? I think so...
    };

    struct Field {
        enum class Type {
            Single,
            Link,
            Array,
        };

        QString name;
        Type type = Type::Single;
        QString comment;

        // link
        QStringList targetSheets;
        std::optional<Condition> condition;
    };

    Field parseField(ryml::ConstNodeRef node);

    std::vector<Field> m_fields;
    QString m_displayField;
};
