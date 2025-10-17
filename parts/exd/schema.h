// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <physis.hpp>

#include <QString>

class Schema
{
public:
    Schema(const QString &path);

    QString nameForColumn(int index) const;
    QVariant displayForData(const physis_ColumnData &data) const;

private:
    struct Field
    {
        QString name;
    };
    std::vector<Field> m_fields;
};
