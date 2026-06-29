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
#include "openinwidget.h"
#include "penumbraapi.h"
#include "settingswindow.h"

MainWindow::MainWindow(physis_SqPackResource data)
    : m_cache(data)
    , m_api(new PenumbraApi(this))
{
    setMinimumSize(QSize(800, 600));

    auto dummyWidget = new QSplitter();
    dummyWidget->setChildrenCollapsible(false);
    setCentralWidget(dummyWidget);

    auto gearListWidget = new GearListWidget(m_cache);
    gearListWidget->setMaximumWidth(350);
    dummyWidget->addWidget(gearListWidget);

    m_gearView = new SingleGearView(m_cache);
    connect(m_gearView, &SingleGearView::importedModel, m_api, &PenumbraApi::redrawAll);
    connect(gearListWidget, &GearListWidget::gearSelected, m_gearView, &SingleGearView::setGear);

    m_materialsView = new QTabWidget();

    auto tabWidget = new QTabWidget();
    tabWidget->addTab(m_gearView, i18nc("@title:tab", "Models"));
    tabWidget->addTab(m_materialsView, i18nc("@title:tab", "Materials"));
    tabWidget->setDocumentMode(true); // Don't draw the borders
    tabWidget->tabBar()->setExpanding(true);
    dummyWidget->addWidget(tabWidget);

    m_fullModelViewer = new FullModelViewer(m_cache);
    connect(m_fullModelViewer, &FullModelViewer::loadingChanged, this, [this](const bool loading) {
        m_gearView->setFMVAvailable(!loading);
    });
    connect(m_gearView, &SingleGearView::addToFullModelViewer, m_fullModelViewer, &FullModelViewer::addGear);
    connect(m_gearView, &SingleGearView::gearChanged, this, &KXmlGuiWindow::setPlainCaption);

    connect(m_gearView, &SingleGearView::doneLoadingModel, this, [this] {
        m_materialsView->clear();

        int i = 0;
        for (auto material : m_gearView->getLoadedMaterials()) {
            auto materialView = new MtrlPart(m_cache);
            materialView->load(material);
            m_materialsView->addTab(materialView, i18n("Material %1", i)); // TODO: it would be nice to get the actual material name here

            i++;
        }
    });

    setupActions();
    setupGUI(ToolBar | Keys | StatusBar | Save | Create, QStringLiteral("geareditor.rc"));

    // We don't provide help (yet)
    actionCollection()->removeAction(actionCollection()->action(KStandardAction::name(KStandardAction::HelpContents)));
    // This isn't KDE software
    actionCollection()->removeAction(actionCollection()->action(KStandardAction::name(KStandardAction::AboutKDE)));

    auto openInWidget = new OpenInWidget(this);
    menuBar()->setCornerWidget(openInWidget);
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
            m_fullModelViewer->show();
        } else {
            m_fullModelViewer->hide();
        }
    });
    connect(m_fullModelViewer, &FullModelViewer::visibleChanged, this, [this, showFMVAction] {
        showFMVAction->setChecked(m_fullModelViewer->isVisible());
    });
    actionCollection()->addAction(QStringLiteral("show_fmv"), showFMVAction);

    auto cmpEditorAction = new QAction(this);
    cmpEditorAction->setText(i18n("&CMP Editor"));
    cmpEditorAction->setIcon(QIcon::fromTheme(QStringLiteral("document-edit")));
    connect(cmpEditorAction, &QAction::triggered, [this] {
        auto cmpEditor = new CmpEditor(m_cache);
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
