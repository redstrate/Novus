// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#define RYML_SINGLE_HDR_DEFINE_NOW
#include "schema.h"

#include <QDebug>
#include <QFile>

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
        const ryml::ConstNodeRef fields = tree["fields"];
        for (const auto &node : fields) {
            const auto field = parseField(node);
            if (field.type != Field::Type::Array) {
                m_fields.push_back(field);
            }
        }

        if (tree.has_child(tree.root_id(), "displayField")) {
            const ryml::ConstNodeRef displayField = tree["displayField"];
            m_displayField = QString::fromLatin1(displayField.val());
        }
    } else {
        qWarning() << "Failed to load schema from" << path;
    }
}

QString Schema::nameForColumn(const uint32_t index) const
{
    if (index < m_fields.size()) {
        return m_fields[index].name;
    }
    return QStringLiteral("Unknown %1").arg(index);
}

std::optional<uint32_t> Schema::indexForName(const QString &name) const
{
    for (size_t i = 0; i < m_fields.size(); i++) {
        if (m_fields[i].name == name) {
            return i;
        }
    }

    return std::nullopt;
}

QStringList Schema::targetSheetsForColumn(const uint32_t index, const std::optional<QVariant> &context) const
{
    if (index < m_fields.size()) {
        const auto &field = m_fields[index];
        if (const auto condition = field.condition) {
            Q_ASSERT(context.has_value()); // We need context here to resolve sheets!

            const int contextValue = context.value().toInt();
            if (condition->cases.contains(contextValue)) {
                return condition->cases[contextValue];
            }
        }
        return field.targetSheets;
    }
    return {};
}

QString Schema::neededContextForColumn(const uint32_t index) const
{
    if (index < m_fields.size()) {
        const auto &field = m_fields[index];
        if (const auto condition = field.condition) {
            return condition->switchColumn;
        }
    }
    return {};
}

bool Schema::isDisplayField(const QString &name) const
{
    return m_displayField == name;
}

std::optional<int> Schema::displayFieldIndex() const
{
    for (size_t i = 0; i < m_fields.size(); i++) {
        if (m_fields[i].name == m_displayField) {
            return i;
        }
    }

    return std::nullopt;
}

QString Schema::comment(const uint32_t index) const
{
    if (index < m_fields.size()) {
        return m_fields[index].comment;
    }
    return {};
}

Schema::Field Schema::parseField(ryml::ConstNodeRef node)
{
    Field field;
    if (node.has_child("name")) {
        field.name = QString::fromLatin1(node["name"].val());
    }

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
            if (node.has_child("condition")) {
                ryml::ConstNodeRef conditionField = node["condition"];

                Condition condition;
                condition.switchColumn = QString::fromLatin1(conditionField["switch"].val());

                ryml::ConstNodeRef casesField = conditionField["cases"];
                for (const auto &switchCase : casesField) {
                    const int caseValue = std::atoi(switchCase.key().data());
                    for (const auto &targetSheet : casesField[switchCase.key()]) {
                        condition.cases[caseValue].push_back(QString::fromLatin1(targetSheet.val()));
                    }
                }

                field.condition = condition;
            }
        } else if (typeName == QStringLiteral("array")) {
            field.type = Field::Type::Array;

            const int count = std::atoi(node["count"].val().data());

            if (node.has_child("fields")) {
                ryml::ConstNodeRef fieldsNode = node["fields"];

                for (int i = 0; i < count; i++) {
                    for (const auto &childFieldNode : fieldsNode) {
                        auto childField = parseField(childFieldNode);

                        // There can be single-field arrays that have no name for their only field
                        if (fieldsNode.num_children() == 1) {
                            childField.name = QStringLiteral("%1[%2]").arg(field.name).arg(i);
                        } else {
                            childField.name = QStringLiteral("%1[%2].%3").arg(field.name).arg(i).arg(childField.name);
                        }

                        m_fields.push_back(childField);
                    }
                }
            } else {
                for (int i = 0; i < count; i++) {
                    auto childField = field; // clone original field
                    childField.name = QStringLiteral("%1[%2]").arg(field.name).arg(i);

                    m_fields.push_back(childField);
                }
            }
        }
    }

    if (node.has_child("comment")) {
        ryml::ConstNodeRef commentField = node["comment"];
        field.comment = QString::fromLatin1(commentField.val());
    }

    return field;
}
