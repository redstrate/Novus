// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"

#include <QHBoxLayout>
#include <QListWidget>
#include <QTableWidget>
#include <QTimer>

#include <KLocalizedString>
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
    dummyWidget->setChildrenCollapsible(false);
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

    materialsView = new QTabWidget();

    metadataView = new MetadataView(&data);

    auto tabWidget = new QTabWidget();
    tabWidget->addTab(gearView, i18nc("@title:tab", "Models"));
    tabWidget->addTab(materialsView, i18nc("@title:tab", "Materials"));
    tabWidget->addTab(metadataView, i18nc("@title:tab", "Metadata"));
    tabWidget->setDocumentMode(true); // Don't draw the borders
    tabWidget->tabBar()->setExpanding(true);
    dummyWidget->addWidget(tabWidget);

    fullModelViewer = new FullModelViewer(&data, cache);
    connect(fullModelViewer, &FullModelViewer::loadingChanged, this, [this](const bool loading) {
        gearView->setFMVAvailable(!loading);
    });

    connect(gearView, &SingleGearView::doneLoadingModel, this, [this, in_data] {
        materialsView->clear();

        int i = 0;
        for (auto material : gearView->getLoadedMaterials()) {
            auto materialView = new MtrlPart(in_data);
            materialView->load(material);
            materialsView->addTab(materialView, i18n("Material %1", i)); // TODO: it would be nice to get the actual material name here

            i++;
        }
    });
}

void MainWindow::setupAdditionalMenus(QMenuBar *menuBar)
{
    auto toolsMenu = menuBar->addMenu(i18nc("@title:menu", "Tools"));

    auto cmpEditorMenu = toolsMenu->addAction(i18nc("@action:inmenu CMP is an abbreviation", "CMP Editor"));
    cmpEditorMenu->setIcon(QIcon::fromTheme(QStringLiteral("document-edit")));
    connect(cmpEditorMenu, &QAction::triggered, [this] {
        auto cmpEditor = new CmpEditor(&data);
        cmpEditor->show();
    });

    auto windowMenu = menuBar->addMenu(i18nc("@title:menu", "Window"));

    auto fmvMenu = windowMenu->addAction(i18nc("@action:inmenu", "Full Model Viewer"));
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

    auto penumbraMenu = menuBar->addMenu(i18nc("@title:menu", "Penumbra"));

    auto redrawAction = penumbraMenu->addAction(i18nc("@action:inmenu", "Redraw All"));
    connect(redrawAction, &QAction::triggered, [this] {
        m_api->redrawAll();
    });

    auto openWindowAction = penumbraMenu->addAction(i18nc("@action:inmenu", "Open Window"));
    connect(openWindowAction, &QAction::triggered, [this] {
        m_api->openWindow();
    });

    auto settingsMenu = menuBar->addMenu(i18nc("@title:menu", "Settings"));

    auto settingsAction = settingsMenu->addAction(i18nc("@action:inmenu", "Configure Armoury…"));
    settingsAction->setIcon(QIcon::fromTheme(QStringLiteral("configure-symbolic")));
    connect(settingsAction, &QAction::triggered, [this] {
        auto settingsWindow = new SettingsWindow();
        settingsWindow->show();
    });
}

#include "moc_mainwindow.cpp"
