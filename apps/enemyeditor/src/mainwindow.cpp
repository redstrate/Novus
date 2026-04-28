// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"

#include "enemymodel.h"

#include <KActionCollection>
#include <KLocalizedString>
#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QDesktopServices>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMenuBar>
#include <QTableView>
#include <physis.hpp>

#include "mdlpart.h"
#include "openinwidget.h"

#include <QHeaderView>
#include <QInputDialog>

MainWindow::MainWindow(physis_SqPackResource data)
    : m_data(data)
    , cache(FileCache{m_data})
{
    setMinimumSize(640, 480);

    auto dummyWidget = new QWidget();
    setCentralWidget(dummyWidget);

    auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    dummyWidget->setLayout(layout);

    auto model = new EnemyModel(&m_data);

    part = new MDLPart(&m_data, cache);
    part->minimumCameraDistance = 0.05f;

    auto skelName = physis_skeleton_path(Race::Hyur, Tribe::Midlander, Gender::Male);
    part->setSkeleton(physis_skeleton_parse(m_data.platform, physis_sqpack_read(&m_data, skelName)));

    m_tableView = new QTableView();
    m_tableView->setModel(model);
    m_tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    m_tableView->verticalHeader()->setDefaultSectionSize(128);
    m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    m_tableView->horizontalHeader()->setDefaultSectionSize(128);
    layout->addWidget(m_tableView);

    setupActions();
    setupGUI(Keys | Save | Create, QStringLiteral("enemyeditor.rc"));

    // We don't provide help (yet)
    actionCollection()->removeAction(actionCollection()->action(KStandardAction::name(KStandardAction::HelpContents)));
    // This isn't KDE software
    actionCollection()->removeAction(actionCollection()->action(KStandardAction::name(KStandardAction::AboutKDE)));

    auto openInWidget = new OpenInWidget(this);
    menuBar()->setCornerWidget(openInWidget);
}

void MainWindow::setupActions()
{
    KStandardAction::quit(qApp, &QCoreApplication::quit, actionCollection());
}

#include "moc_mainwindow.cpp"
