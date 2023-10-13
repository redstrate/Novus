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
    explicit FileTreeWindow(QString gamePath, GameData *data, QWidget *parent = nullptr);

Q_SIGNALS:
    void extractFile(QString path);
    void pathSelected(QString path);

private:
    void refreshModel();

    GameData *data = nullptr;
    FileTreeModel *m_fileModel;
    QSortFilterProxyModel *m_searchModel;
    QCheckBox *m_unknownCheckbox;
    QString m_gamePath;
};