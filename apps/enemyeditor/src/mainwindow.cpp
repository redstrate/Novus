// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"

#include "enemyinfowindow.h"
#include "enemymodel.h"

#include <KActionCollection>
#include <KActionMenu>
#include <KColorSchemeManager>
#include <KColorSchemeMenu>
#include <KLocalizedString>
#include <QApplication>
#include <QDesktopServices>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QListWidget>
#include <QTableView>
#include <physis.hpp>

#include "kawariapi.h"
#include "mdlpart.h"
#include "openinwidget.h"

MainWindow::MainWindow(const physis_SqPackResource data)
    : m_cache(data)
{
    const auto dummyWidget = new QWidget();
    setCentralWidget(dummyWidget);

    const auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    dummyWidget->setLayout(layout);

    const auto model = new EnemyModel(m_cache);

    m_tableView = new QTableView();
    m_tableView->setModel(model);
    m_tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    m_tableView->verticalHeader()->setDefaultSectionSize(128);
    m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    m_tableView->horizontalHeader()->setDefaultSectionSize(128);
    m_tableView->horizontalHeader()->setVisible(false);
    m_tableView->verticalHeader()->setVisible(false);
    layout->addWidget(m_tableView);

    connect(m_tableView, &QTableView::activated, this, [this](const QModelIndex &index) {
        const auto ids = index.data(EnemyModel::CustomRole::IdsRole).value<QList<uint32_t>>();
        const auto mdlPath = index.data(EnemyModel::CustomRole::MdlPath).value<QString>();
        const auto mtrlPaths = index.data(EnemyModel::CustomRole::MtrlPaths).value<QList<QString>>();

        const auto window = new EnemyInfoWindow(m_cache, m_kawariApi, ids, mdlPath, mtrlPaths, this);
        window->open();
    });

    setupActions();
    setupGUI(QSize(640, 480), Keys | Save | Create, QStringLiteral("enemyeditor.rc"));

    // We don't provide help (yet)
    actionCollection()->removeAction(actionCollection()->action(KStandardAction::name(KStandardAction::HelpContents)));
    // This isn't KDE software
    actionCollection()->removeAction(actionCollection()->action(KStandardAction::name(KStandardAction::AboutKDE)));
    // We don't use this well enough
    actionCollection()->removeAction(actionCollection()->action(KStandardAction::name(KStandardAction::WhatsThis)));

    const auto openInWidget = new OpenInWidget(this);
    menuBar()->setCornerWidget(openInWidget);

    m_kawariApi = new KawariApi(this);
}

void MainWindow::setupActions()
{
    KStandardAction::quit(qApp, &QCoreApplication::quit, actionCollection());

    // Window color scheme menu
    const auto manager = KColorSchemeManager::instance();
    const auto selectionMenu = KColorSchemeMenu::createMenu(manager, this);
    const auto windowColorSchemeMenu = new QAction(this);
    windowColorSchemeMenu->setMenu(selectionMenu->menu());
    windowColorSchemeMenu->menu()->setIcon(QIcon::fromTheme(QStringLiteral("preferences-desktop-color")));
    windowColorSchemeMenu->menu()->setTitle(i18n("&Window Color Scheme"));
    actionCollection()->addAction(QStringLiteral("window_color_scheme"), windowColorSchemeMenu);
}

#include "moc_mainwindow.cpp"
