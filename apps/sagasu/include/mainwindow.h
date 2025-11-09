// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <KXmlGuiWindow>
#include <QMap>
#include <QMdiArea>
#include <QNetworkAccessManager>
#include <QTreeWidget>

#include "filecache.h"
#include "filetreewindow.h"
#include "hashdatabase.h"

#include <QLabel>

class AbstractExcelResolver;
struct SqPackResource;

class MainWindow : public KXmlGuiWindow
{
public:
    MainWindow(const QString &gamePath, SqPackResource *data);

    bool selectPath(const QString &path);

private:
    void setupActions();

    SqPackResource *data = nullptr;
    QTabWidget *partHolder = nullptr;
    FileCache fileCache;
    HashDatabase m_database;
    QNetworkAccessManager *m_mgr = nullptr;
    FileTreeWindow *m_tree = nullptr;
    QLabel *m_offsetLabel = nullptr;
    QLabel *m_hashLabel = nullptr;
    QLabel *m_fileTypeLabel = nullptr;
    AbstractExcelResolver *m_excelResolver = nullptr;

    void refreshParts(const QString &indexPath, Hash hash, const QString &path);
};
