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

MainWindow::MainWindow(GameData* data) : data(data) {
    setWindowTitle(QStringLiteral("explorer"));

    auto fileMenu = menuBar()->addMenu(QStringLiteral("File"));

    auto quitAction = fileMenu->addAction(QStringLiteral("Quit"));
    quitAction->setIcon(QIcon::fromTheme(QStringLiteral("gtk-quit")));
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

    auto helpMenu = menuBar()->addMenu(QStringLiteral("Help"));

    auto donateAction = helpMenu->addAction(QStringLiteral("Donate"));
    connect(donateAction, &QAction::triggered, this, [] {
        QDesktopServices::openUrl(QUrl(QStringLiteral("https://redstrate.com/fund")));
    });
    donateAction->setIcon(QIcon::fromTheme(QStringLiteral("help-donate")));

    helpMenu->addSeparator();

    auto aboutNovusAction = helpMenu->addAction(QStringLiteral("About explorer"));
    aboutNovusAction->setIcon(QIcon::fromTheme(QStringLiteral("help-about")));
    connect(aboutNovusAction, &QAction::triggered, this, [this] {
        auto window = new KAboutApplicationDialog(KAboutData::applicationData(), this);
        window->show();
    });

    auto aboutQtAction = helpMenu->addAction(QStringLiteral("About Qt"));
    aboutQtAction->setIcon(QIcon(QStringLiteral(":/qt-project.org/qmessagebox/images/qtlogo-64.png")));
    connect(aboutQtAction, &QAction::triggered, QApplication::instance(), &QApplication::aboutQt);

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

