// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"

#include "enemyinfowindow.h"
#include "enemymodel.h"

#include <KActionCollection>
#include <QApplication>
#include <QDesktopServices>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QListWidget>
#include <QTableView>
#include <physis.hpp>

#include "mdlpart.h"
#include "openinwidget.h"

#include <QHeaderView>

MainWindow::MainWindow(const physis_SqPackResource data)
    : m_cache(data)
{
    setMinimumSize(640, 480);

    const auto dummyWidget = new QWidget();
    setCentralWidget(dummyWidget);

    const auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    dummyWidget->setLayout(layout);

    const auto model = new EnemyModel(m_cache);

    m_part = new MDLPart(m_cache);
    m_part->minimumCameraDistance = 0.05f;

    const auto skelName = physis_skeleton_path(Race::Hyur, Tribe::Midlander, Gender::Male);
    m_part->setSkeleton(physis_skeleton_parse(m_cache.platform(), m_cache.read(QString::fromUtf8(skelName))));

    m_tableView = new QTableView();
    m_tableView->setModel(model);
    m_tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    m_tableView->verticalHeader()->setDefaultSectionSize(128);
    m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    m_tableView->horizontalHeader()->setDefaultSectionSize(128);
    layout->addWidget(m_tableView);

    connect(m_tableView, &QTableView::activated, this, [this](const QModelIndex &index) {
        const auto id = index.data(EnemyModel::CustomRole::IdRole).value<uint32_t>();
        const auto mdlPath = index.data(EnemyModel::CustomRole::MdlPath).value<QString>();
        const auto mtrlPath = index.data(EnemyModel::CustomRole::MtrlPath).value<QString>();

        const auto window = new EnemyInfoWindow(id, mdlPath, mtrlPath, this);
        window->open();
    });

    setupActions();
    setupGUI(ToolBar | Keys | StatusBar | Save | Create, QStringLiteral("enemyeditor.rc"));

    // We don't provide help (yet)
    actionCollection()->removeAction(actionCollection()->action(KStandardAction::name(KStandardAction::HelpContents)));
    // This isn't KDE software
    actionCollection()->removeAction(actionCollection()->action(KStandardAction::name(KStandardAction::AboutKDE)));

    const auto openInWidget = new OpenInWidget(this);
    menuBar()->setCornerWidget(openInWidget);
}

void MainWindow::setupActions() const
{
    KStandardAction::quit(qApp, &QCoreApplication::quit, actionCollection());
}

#include "moc_mainwindow.cpp"
