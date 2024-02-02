// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QCheckBox>
#include <QSortFilterProxyModel>
#include <physis.hpp>

#include "filetreemodel.h"

class FileTreeWindow : public QWidget
{
    Q_OBJECT

public:
    explicit FileTreeWindow(HashDatabase &database, const QString &gamePath, GameData *data, QWidget *parent = nullptr);

Q_SIGNALS:
    void extractFile(const QString &path);
    void pathSelected(const QString &path);

private:
    void refreshModel();

    GameData *data = nullptr;
    FileTreeModel *m_fileModel = nullptr;
    QSortFilterProxyModel *m_searchModel = nullptr;
    QCheckBox *m_unknownCheckbox = nullptr;
    QString m_gamePath;
    HashDatabase &m_database;
};