// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "schema.h"

#include <QAbstractTableModel>

#include <physis.hpp>

class ExcelModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    ExcelModel(const physis_EXD &exd, Schema schema);

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;

    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:
    physis_EXD m_exd;
    unsigned int m_rowCount = 0;
    std::vector<std::tuple<int, int, int>> m_rowIndices;
    Schema m_schema; // TODO: don't copy
    bool m_hasSubrows = false;
};
