// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"

#include <KAboutApplicationDialog>
#include <KAboutData>
#include <QAction>
#include <QApplication>
#include <QDesktopServices>
#include <QHBoxLayout>
#include <QListWidget>
#include <QMenuBar>
#include <QTableWidget>
#include <QUrl>
#include <physis.hpp>

#include "exdpart.h"

MainWindow::MainWindow(GameData *data)
    : data(data)
{
    setWindowTitle(QStringLiteral("Karuku"));
    setMinimumSize(1280, 720);

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

    auto aboutNovusAction = helpMenu->addAction(QStringLiteral("About Karuku"));
    aboutNovusAction->setIcon(QIcon::fromTheme(QStringLiteral("help-about")));
    connect(aboutNovusAction, &QAction::triggered, this, [this] {
        auto window = new KAboutApplicationDialog(KAboutData::applicationData(), this);
        window->show();
    });

    auto aboutQtAction = helpMenu->addAction(QStringLiteral("About Qt"));
    aboutQtAction->setIcon(QIcon(QStringLiteral(":/qt-project.org/qmessagebox/images/qtlogo-64.png")));
    connect(aboutQtAction, &QAction::triggered, QApplication::instance(), &QApplication::aboutQt);

    auto dummyWidget = new QWidget();
    setCentralWidget(dummyWidget);

    auto layout = new QHBoxLayout();
    dummyWidget->setLayout(layout);

    auto listWidget = new QListWidget();

    auto names = physis_gamedata_get_all_sheet_names(data);
    for (int i = 0; i < names.name_count; i++) {
        listWidget->addItem(QString::fromStdString(names.names[i]));
    }

    listWidget->setMaximumWidth(200);
    listWidget->sortItems();
    layout->addWidget(listWidget);

    auto exdPart = new EXDPart(data);
    layout->addWidget(exdPart);

    connect(listWidget, &QListWidget::itemClicked, this, [exdPart](QListWidgetItem *item) {
        auto name = item->text().toStdString();
        auto nameLowercase = item->text().toLower().toStdString();

        exdPart->loadSheet(QString::fromStdString(name.c_str()));
    });
}