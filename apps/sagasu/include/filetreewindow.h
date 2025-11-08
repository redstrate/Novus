// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QCheckBox>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QTreeView>
#include <physis.hpp>

#include "filetreemodel.h"

class FileTreeWindow : public QWidget
{
    Q_OBJECT

public:
    explicit FileTreeWindow(HashDatabase &database, const QString &gamePath, SqPackResource *data, QWidget *parent = nullptr);

    void refreshModel();
    void setShowUnknown(bool show);
    bool selectPath(const QString &path);
    void focusSearchField();

Q_SIGNALS:
    void extractFile(const QString &path);
    void pathSelected(const QString &indexPath, Hash hash, const QString &path);

private:
    SqPackResource *data = nullptr;
    FileTreeModel *m_fileModel = nullptr;
    QSortFilterProxyModel *m_searchModel = nullptr;
    QCheckBox *m_unknownCheckbox = nullptr;
    QString m_gamePath;
    HashDatabase &m_database;
    bool m_showUnknown = false;
    QTreeView *m_treeWidget = nullptr;
    QLineEdit *m_searchEdit = nullptr;
};
