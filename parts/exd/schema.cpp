// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "schema.h"

#include <QDebug>
#include <KLocalizedString>

#define RYML_SINGLE_HDR_DEFINE_NOW
#include <QFile>
#include <rapidyaml-0.10.0.hpp>

Schema::Schema(const QString &path)
{
    QFile file(path);
    file.open(QIODevice::ReadOnly);
    auto bytes = file.readAll();

    const auto tree = ryml::parse_in_place(bytes.data());
    if (tree.has_child(tree.root_id(), "fields")) {
        ryml::ConstNodeRef fields = tree["fields"];
        for (const auto &node : fields) {
            Field field;
            field.name = QString::fromLatin1(node["name"].val());

            m_fields.push_back(field);
        }
    } else {
        qWarning() << "Failed to load schema from" << path;
    }
}

QString Schema::nameForColumn(const int index) const
{
    if (static_cast<unsigned int>(index) < m_fields.size()) {
        return m_fields[index].name;
    } else {
        return QStringLiteral("Unknown %1").arg(index);
    }
}

QVariant Schema::displayForData(const physis_ColumnData &data) const
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
