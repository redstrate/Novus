// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"

#include <KActionCollection>
#include <QApplication>
#include <QListWidget>
#include <QMenuBar>
#include <QSplitter>
#include <physis.hpp>

#include "materialview.h"
#include "mtrlpart.h"

MainWindow::MainWindow(GameData *data)
    : KXmlGuiWindow()
    , data(data)
    , cache(*data)
{
    setMinimumSize(1280, 720);

    auto matFile = physis_gamedata_extract_file(data, "chara/equipment/e0028/material/v0001/mt_c0101e0028_top_a.mtrl");
    m_material = physis_material_parse(matFile);

    auto dummyWidget = new QSplitter();
    dummyWidget->setChildrenCollapsible(false);
    setCentralWidget(dummyWidget);

    auto materialProperty = new MtrlPart(data);
    materialProperty->setMaximumWidth(400);
    materialProperty->load(m_material);
    dummyWidget->addWidget(materialProperty);

    auto matView = new MaterialView(data, cache);
    matView->addSphere(m_material);
    dummyWidget->addWidget(matView);

    setupActions();
    setupGUI(Keys | Save | Create);

    // We don't provide help (yet)
    actionCollection()->removeAction(actionCollection()->action(KStandardAction::name(KStandardAction::HelpContents)));
    // This isn't KDE software
    actionCollection()->removeAction(actionCollection()->action(KStandardAction::name(KStandardAction::AboutKDE)));
}

void MainWindow::setupActions()
{
    KStandardAction::quit(qApp, &QCoreApplication::quit, actionCollection());
}

#include "moc_mainwindow.cpp"
