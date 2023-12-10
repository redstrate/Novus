// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"

#include <QHBoxLayout>
#include <QListWidget>
#include <QTableWidget>
#include <QTimer>

#include <QAction>
#include <QApplication>
#include <QDesktopServices>
#include <QFileDialog>
#include <QMenuBar>
#include <magic_enum.hpp>

#include "cmpeditor.h"
#include "filecache.h"
#include "gearlistwidget.h"
#include "settingswindow.h"

MainWindow::MainWindow(GameData *in_data)
    : NovusMainWindow()
    , data(*in_data)
    , cache(FileCache{*in_data})
{
    setMinimumSize(QSize(800, 600));
    setupMenubar();

    auto dummyWidget = new QWidget();
    setCentralWidget(dummyWidget);

    auto layout = new QHBoxLayout();
    dummyWidget->setLayout(layout);

    auto gearListWidget = new GearListWidget(&data);
    gearListWidget->setMaximumWidth(350);
    connect(gearListWidget, &GearListWidget::gearSelected, this, [=](const GearInfo &gear) {
        gearView->setGear(gear);
    });
    layout->addWidget(gearListWidget);

    gearView = new SingleGearView(&data, cache);
    connect(gearView, &SingleGearView::addToFullModelViewer, this, [=](GearInfo &info) {
        fullModelViewer->addGear(info);
    });

    auto tabWidget = new QTabWidget();
    tabWidget->addTab(gearView, QStringLiteral("Models"));
    layout->addWidget(tabWidget);

    fullModelViewer = new FullModelViewer(&data, cache);
    connect(fullModelViewer, &FullModelViewer::loadingChanged, this, [=](const bool loading) {
        gearView->setFMVAvailable(!loading);
    });
    fullModelViewer->show();
}

void MainWindow::setupAdditionalMenus(QMenuBar *menuBar)
{
    auto toolsMenu = menuBar->addMenu(QStringLiteral("Tools"));

    auto cmpEditorMenu = toolsMenu->addAction(QStringLiteral("CMP Editor"));
    cmpEditorMenu->setIcon(QIcon::fromTheme(QStringLiteral("document-edit")));
    connect(cmpEditorMenu, &QAction::triggered, [this] {
        auto cmpEditor = new CmpEditor(&data);
        cmpEditor->show();
    });

    auto settingsMenu = toolsMenu->addAction(QStringLiteral("Settings..."));
    connect(settingsMenu, &QAction::triggered, [this] {
        auto settingsWindow = new SettingsWindow();
        settingsWindow->show();
    });
}

#include "moc_mainwindow.cpp"
