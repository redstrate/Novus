// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QCheckBox>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QTreeView>
#include <physis.hpp>

#include "difftreemodel.h"

class DiffTreeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DiffTreeWidget(HashDatabase &database, physis_SqPackResource *data, QWidget *parent = nullptr);

    void refreshModel();
    void openPatch(const QString &path);

Q_SIGNALS:
    void bufferSelected(physis_Buffer buffer);

private:
    physis_SqPackResource *data = nullptr;
    DiffTreeModel *m_fileModel = nullptr;
    QSortFilterProxyModel *m_searchModel = nullptr;
    HashDatabase &m_database;
    QTreeView *m_treeWidget = nullptr;
    QLineEdit *m_searchEdit = nullptr;
};
