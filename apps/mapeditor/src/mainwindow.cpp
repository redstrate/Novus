// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"

#include <QApplication>
#include <QDesktopServices>
#include <QHBoxLayout>
#include <QListWidget>
#include <QMenuBar>
#include <QSplitter>
#include <QUrl>
#include <physis.hpp>

#include "maplistwidget.h"
#include "mapview.h"

MainWindow::MainWindow(GameData *data)
    : NovusMainWindow()
    , data(data)
    , cache(*data)
{
    setMinimumSize(1280, 720);
    setupMenubar();

    auto dummyWidget = new QSplitter();
    dummyWidget->setChildrenCollapsible(false);
    setCentralWidget(dummyWidget);

    auto listWidget = new MapListWidget(data);
    listWidget->setMaximumWidth(400);
    dummyWidget->addWidget(listWidget);

    auto mapView = new MapView(data, cache);
    dummyWidget->addWidget(mapView);

    connect(listWidget, &MapListWidget::mapSelected, this, [data, mapView](const QString &basePath) {
        QString base2Path = basePath.left(basePath.lastIndexOf(QStringLiteral("/level/")));
        QString bgPath = QStringLiteral("bg/%1/bgplate/").arg(base2Path);

        std::string bgPathStd = bgPath.toStdString() + "terrain.tera";

        auto tera_buffer = physis_gamedata_extract_file(data, bgPathStd.c_str());

        auto tera = physis_parse_tera(tera_buffer);
        mapView->addTerrain(bgPath, tera);
    });
}

#include "moc_mainwindow.cpp"