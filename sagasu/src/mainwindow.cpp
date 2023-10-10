// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"

#include <KAboutApplicationDialog>
#include <KAboutData>
#include <QAction>
#include <QApplication>
#include <QDebug>
#include <QDesktopServices>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QTableWidget>
#include <QTreeWidget>
#include <QUrl>

#include "filepropertieswindow.h"
#include "filetreewindow.h"

MainWindow::MainWindow(GameData *data)
    : NovusMainWindow()
    , data(data)
{
    setupMenubar();

    mdiArea = new QMdiArea();
    setCentralWidget(mdiArea);

    auto tree = new FileTreeWindow(data);
    connect(tree, &FileTreeWindow::openFileProperties, this, [=](QString path) {
        qInfo() << "opening properties window for " << path;
        auto window = mdiArea->addSubWindow(new FilePropertiesWindow(data, path));
        window->show();
    });

    mdiArea->addSubWindow(tree);
}
