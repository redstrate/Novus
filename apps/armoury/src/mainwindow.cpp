// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"

#include <QHBoxLayout>
#include <QListWidget>
#include <QTableWidget>
#include <QTimer>

#include <KActionCollection>
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
    : KXmlGuiWindow()
    , data(*in_data)
    , cache(FileCache{*in_data})
    , m_api(new PenumbraApi(this))
{
    setMinimumSize(QSize(800, 600));

    auto dummyWidget = new QSplitter();
    dummyWidget->setChildrenCollapsible(false);
    setCentralWidget(dummyWidget);

    auto gearListWidget = new GearListWidget(&data);
    gearListWidget->setMaximumWidth(350);
    dummyWidget->addWidget(gearListWidget);

    gearView = new SingleGearView(&data, cache);
    connect(gearView, &SingleGearView::importedModel, m_api, &PenumbraApi::redrawAll);
    connect(gearListWidget, &GearListWidget::gearSelected, gearView, &SingleGearView::setGear);

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
    connect(gearView, &SingleGearView::addToFullModelViewer, fullModelViewer, &FullModelViewer::addGear);

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

    setupActions();
    setupGUI(Keys | Save | Create, QStringLiteral("geareditor.rc"));

    // We don't provide help (yet)
    actionCollection()->removeAction(actionCollection()->action(KStandardAction::name(KStandardAction::HelpContents)));
    // This isn't KDE software
    actionCollection()->removeAction(actionCollection()->action(KStandardAction::name(KStandardAction::AboutKDE)));
}

void MainWindow::configure()
{
    auto settingsWindow = new SettingsWindow();
    settingsWindow->show();
}

void MainWindow::setupActions()
{
    KStandardAction::preferences(this, &MainWindow::configure, actionCollection());
    KStandardAction::quit(qApp, &QCoreApplication::quit, actionCollection());

    auto showFMVAction = new QAction(this);
    showFMVAction->setText(i18n("&Full Model Viewer"));
    showFMVAction->setCheckable(true);
    showFMVAction->setIcon(QIcon::fromTheme(QStringLiteral("user-symbolic")));
    connect(showFMVAction, &QAction::toggled, [this](bool toggled) {
        if (toggled) {
            fullModelViewer->show();
        } else {
            fullModelViewer->hide();
        }
    });
    connect(fullModelViewer, &FullModelViewer::visibleChanged, this, [this, showFMVAction] {
        showFMVAction->setChecked(fullModelViewer->isVisible());
    });
    actionCollection()->addAction(QStringLiteral("show_fmv"), showFMVAction);

    auto cmpEditorAction = new QAction(this);
    cmpEditorAction->setText(i18n("&CMP Editor"));
    cmpEditorAction->setIcon(QIcon::fromTheme(QStringLiteral("document-edit")));
    connect(cmpEditorAction, &QAction::triggered, [this] {
        auto cmpEditor = new CmpEditor(&data);
        cmpEditor->show();
    });
    actionCollection()->addAction(QStringLiteral("cmp_editor"), cmpEditorAction);

    auto redrawAction = new QAction(i18nc("@action:inmenu", "Redraw All"));
    connect(redrawAction, &QAction::triggered, m_api, &PenumbraApi::redrawAll);
    actionCollection()->addAction(QStringLiteral("redraw_all"), redrawAction);

    auto openWindowAction = new QAction(i18nc("@action:inmenu", "Open Window"));
    connect(openWindowAction, &QAction::triggered, m_api, &PenumbraApi::openWindow);
    actionCollection()->addAction(QStringLiteral("open_window"), openWindowAction);
}

#include "moc_mainwindow.cpp"
