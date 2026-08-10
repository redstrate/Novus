// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <KXmlGuiWindow>
#include <QMdiArea>
#include <QNetworkAccessManager>

#include "filecache.h"
#include "filetreewindow.h"
#include "hashdatabase.h"

#include <QLabel>

class QFileInfo;
class AbstractExcelResolver;

class MainWindow : public KXmlGuiWindow
{
    Q_OBJECT
public:
    MainWindow(const QString &gamePath, physis_SqPackResource data);

    bool selectPath(const QString &path) const;

public Q_SLOTS:
    QString getArguments() const;
    void back();
    void forward();

private:
    void setupActions();
    void updateNavigationActions() const;

    QTabWidget *m_partHolder = nullptr;
    FileCache m_cache;
    HashDatabase m_database;
    QNetworkAccessManager *m_mgr = nullptr;
    FileTreeWindow *m_tree = nullptr;
    AbstractExcelResolver *m_excelResolver = nullptr;
    QAction *m_fileActions = nullptr;
    QMenu *m_fileActionsMenu = nullptr;
    QString m_currentPath;

    void refreshParts(const QString &indexPath, Hash hash, const QString &path);
    void loadPart(physis_Buffer file, const QFileInfo &info);

    QList<QString> m_navigationStack;
    qsizetype m_navigationPointer = 0;
    QAction *m_backAction = nullptr;
    QAction *m_forwardAction = nullptr;
    bool m_navigatingStack = false;
    QLineEdit *m_urlEdit = nullptr;
};
