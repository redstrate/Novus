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
#include <QSplitter>
#include <magic_enum.hpp>

#include "cmpeditor.h"
#include "filecache.h"
#include "gearlistwidget.h"
#include "penumbraapi.h"
#include "settingswindow.h"

MainWindow::MainWindow(GameData *in_data)
    : NovusMainWindow()
    , data(*in_data)
    , cache(FileCache{*in_data})
    , m_api(new PenumbraApi(this))
{
    setMinimumSize(QSize(800, 600));
    setupMenubar();

    auto dummyWidget = new QSplitter();
    setCentralWidget(dummyWidget);

    auto gearListWidget = new GearListWidget(&data);
    gearListWidget->setMaximumWidth(350);
    connect(gearListWidget, &GearListWidget::gearSelected, this, [this](const GearInfo &gear) {
        gearView->setGear(gear);
    });
    dummyWidget->addWidget(gearListWidget);

    gearView = new SingleGearView(&data, cache);
    connect(gearView, &SingleGearView::addToFullModelViewer, this, [this](GearInfo &info) {
        fullModelViewer->addGear(info);
    });
    connect(gearView, &SingleGearView::importedModel, m_api, &PenumbraApi::redrawAll);

    auto tabWidget = new QTabWidget();
    tabWidget->addTab(gearView, QStringLiteral("Models"));
    tabWidget->setDocumentMode(true); // Don't draw the borders
    tabWidget->tabBar()->setExpanding(true);
    dummyWidget->addWidget(tabWidget);

    fullModelViewer = new FullModelViewer(&data, cache);
    connect(fullModelViewer, &FullModelViewer::loadingChanged, this, [this](const bool loading) {
        gearView->setFMVAvailable(!loading);
    });
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

    auto windowMenu = menuBar->addMenu(QStringLiteral("Window"));

    auto fmvMenu = windowMenu->addAction(QStringLiteral("Full Model viewer"));
    fmvMenu->setCheckable(true);
    fmvMenu->setIcon(QIcon::fromTheme(QStringLiteral("user-symbolic")));
    connect(fmvMenu, &QAction::toggled, [this](bool toggled) {
        if (toggled) {
            fullModelViewer->show();
        } else {
            fullModelViewer->hide();
        }
    });
    connect(fullModelViewer, &FullModelViewer::visibleChanged, this, [this, fmvMenu] {
        fmvMenu->setChecked(fullModelViewer->isVisible());
    });

    auto settingsMenu = menuBar->addMenu(QStringLiteral("Settings"));

    auto settingsAction = settingsMenu->addAction(QStringLiteral("Configure Armoury..."));
    settingsAction->setIcon(QIcon::fromTheme(QStringLiteral("configure-symbolic")));
    connect(settingsAction, &QAction::triggered, [this] {
        auto settingsWindow = new SettingsWindow();
        settingsWindow->show();
    });
}

#include "moc_mainwindow.cpp"
