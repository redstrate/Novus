// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "schema.h"

#include <QDebug>

#define RYML_SINGLE_HDR_DEFINE_NOW
#include <QFile>
#include <rapidyaml-0.10.0.hpp>

Schema::Schema(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    auto bytes = file.readAll();
    if (bytes.isEmpty()) {
        return;
    }

    const auto tree = ryml::parse_in_place(bytes.data());
    if (tree.has_child(tree.root_id(), "fields")) {
        ryml::ConstNodeRef fields = tree["fields"];
        for (const auto &node : fields) {
            Field field;
            field.name = QString::fromLatin1(node["name"].val());

            if (node.has_child("type")) {
                ryml::ConstNodeRef typeField = node["type"];
                const QString typeName = QString::fromLatin1(typeField.val());

                if (typeName == QStringLiteral("link")) {
                    field.type = Field::Type::Link;

                    if (node.has_child("targets")) {
                        ryml::ConstNodeRef targetsField = node["targets"];
                        for (auto target : targetsField) {
                            field.targetSheets.push_back(QString::fromLatin1(target.val()));
                        }
                    }

                    // TOOD: support switch statements
                }
            }

            m_fields.push_back(field);
        }

        if (tree.has_child(tree.root_id(), "displayField")) {
            ryml::ConstNodeRef displayField = tree["displayField"];
            m_displayField = QString::fromLatin1(displayField.val());
        }
    } else {
        qWarning() << "Failed to load schema from" << path;
    }
}

QString Schema::nameForColumn(const int index) const
{
    if (static_cast<unsigned int>(index) < m_fields.size()) {
        return m_fields[index].name;
    }
    return QStringLiteral("Unknown %1").arg(index);
}

QStringList Schema::targetSheetsForColumn(const int index) const
{
    if (static_cast<unsigned int>(index) < m_fields.size()) {
        return m_fields[index].targetSheets;
    }
    return {};
}

bool Schema::isDisplayField(const QString &name) const
{
    return m_displayField == name;
}
