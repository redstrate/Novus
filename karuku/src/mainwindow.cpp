// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"

#include <QApplication>
#include <QDesktopServices>
#include <QHBoxLayout>
#include <QListWidget>
#include <QMenuBar>
#include <QUrl>
#include <physis.hpp>

#include "exdpart.h"
#include "sheetlistwidget.h"

MainWindow::MainWindow(GameData *data)
    : NovusMainWindow()
    , data(data)
{
    setMinimumSize(1280, 720);
    setupMenubar();

    auto dummyWidget = new QWidget();
    setCentralWidget(dummyWidget);

    auto layout = new QHBoxLayout();
    dummyWidget->setLayout(layout);

    auto listWidget = new SheetListWidget(data);
    listWidget->setMaximumWidth(200);
    layout->addWidget(listWidget);

    auto exdPart = new EXDPart(data);
    layout->addWidget(exdPart);

    connect(listWidget, &SheetListWidget::sheetSelected, this, [data, exdPart](const QString &name) {
        auto path = QStringLiteral("exd/%1.exh").arg(name.toLower());
        auto pathStd = path.toStdString();

        auto file = physis_gamedata_extract_file(data, pathStd.c_str());

        exdPart->loadSheet(name, file);
    });
}

#include "moc_mainwindow.cpp"