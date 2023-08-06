// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"

#include <QAction>
#include <QApplication>
#include <QDesktopServices>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QListWidget>
#include <QMenuBar>
#include <QTableWidget>
#include <QUrl>
#include <fmt/core.h>
#include <physis.hpp>

#include "aboutwindow.h"
#include "mdlpart.h"

MainWindow::MainWindow(GameData* data) : data(data), cache(FileCache{*data}) {
    setWindowTitle("Model Viewer");

    auto fileMenu = menuBar()->addMenu("File");

    auto openMDLFile = fileMenu->addAction("Open MDL...");
    connect(openMDLFile, &QAction::triggered, [=] {
        auto fileName = QFileDialog::getOpenFileName(nullptr,
                                                     "Open MDL File",
                                                     "~",
                                                     "FFXIV Model File (*.mdl)");

        auto buffer = physis_read_file(fileName.toStdString().c_str());

        part->addModel(physis_mdl_parse(buffer.size, buffer.data), {}, 0);
    });

    auto helpMenu = menuBar()->addMenu("Help");

    auto donateAction = helpMenu->addAction("Donate");
    connect(donateAction, &QAction::triggered, this, [] {
        QDesktopServices::openUrl(QUrl("https://redstrate.com/fund"));
    });
    donateAction->setIcon(QIcon::fromTheme("help-donate"));

    helpMenu->addSeparator();

    auto aboutNovusAction = helpMenu->addAction("About Model Viewer");
    aboutNovusAction->setIcon(QIcon::fromTheme("help-about"));
    connect(aboutNovusAction, &QAction::triggered, this, [this] {
        auto window = new AboutWindow(this);
        window->show();
    });

    auto aboutQtAction = helpMenu->addAction("About Qt");
    aboutQtAction->setIcon(QIcon(":/qt-project.org/qmessagebox/images/qtlogo-64.png"));
    connect(aboutQtAction, &QAction::triggered, QApplication::instance(), &QApplication::aboutQt);

    auto dummyWidget = new QWidget();
    setCentralWidget(dummyWidget);

    auto layout = new QHBoxLayout();
    dummyWidget->setLayout(layout);

    part = new MDLPart(data, cache);

    const int raceCode = physis_get_race_code(Race::Hyur, Subrace::Midlander, Gender::Male);
    QString skelName = QString{"c%1b0001.skel"}.arg(raceCode, 4, 10, QLatin1Char{'0'});
    part->setSkeleton(physis_skeleton_from_skel(physis_read_file(skelName.toStdString().c_str())));

    layout->addWidget(part);
}