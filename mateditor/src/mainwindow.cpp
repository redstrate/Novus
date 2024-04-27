// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"

#include <QApplication>
#include <QListWidget>
#include <QMenuBar>
#include <QSplitter>
#include <physis.hpp>

#include "materialpropertyedit.h"
#include "materialview.h"

MainWindow::MainWindow(GameData *data)
    : NovusMainWindow()
    , data(data)
    , cache(*data)
{
    setMinimumSize(1280, 720);
    setupMenubar();

    auto matFile = physis_gamedata_extract_file(data, "chara/equipment/e0028/material/v0001/mt_c0101e0028_top_a.mtrl");
    m_material = physis_material_parse(matFile);

    auto dummyWidget = new QSplitter();
    dummyWidget->setChildrenCollapsible(false);
    setCentralWidget(dummyWidget);

    auto materialProperty = new MaterialPropertyEdit(data);
    materialProperty->setMaximumWidth(400);
    materialProperty->setMaterial(m_material);
    dummyWidget->addWidget(materialProperty);

    auto matView = new MaterialView(data, cache);
    matView->addSphere(m_material);
    dummyWidget->addWidget(matView);
}

#include "moc_mainwindow.cpp"