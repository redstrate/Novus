// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"

#include <KAboutApplicationDialog>
#include <KAboutData>
#include <QAction>
#include <QApplication>
#include <QDesktopServices>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QListWidget>
#include <QMenuBar>
#include <QTableWidget>
#include <QUrl>
#include <physis.hpp>

#include "mdlpart.h"

MainWindow::MainWindow(GameData* data) : data(data), cache(FileCache{*data}) {
    setWindowTitle(QStringLiteral("Model Viewer"));
    setMinimumSize(640, 480);

    auto fileMenu = menuBar()->addMenu(QStringLiteral("File"));

    auto openMDLFile = fileMenu->addAction(QStringLiteral("Open MDL..."));
    openMDLFile->setIcon(QIcon::fromTheme(QStringLiteral("document-open")));
    connect(openMDLFile, &QAction::triggered, [=] {
        auto fileName = QFileDialog::getOpenFileName(nullptr, QStringLiteral("Open MDL File"), QStringLiteral("~"), QStringLiteral("FFXIV Model File (*.mdl)"));

        auto buffer = physis_read_file(fileName.toStdString().c_str());

        part->addModel(physis_mdl_parse(buffer.size, buffer.data), {}, 0);
    });

    fileMenu->addSeparator();

    auto quitAction = fileMenu->addAction(QStringLiteral("Quit"));
    quitAction->setIcon(QIcon::fromTheme(QStringLiteral("gtk-quit")));
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

    auto helpMenu = menuBar()->addMenu(QStringLiteral("Help"));

    auto donateAction = helpMenu->addAction(QStringLiteral("Donate"));
    connect(donateAction, &QAction::triggered, this, [] {
        QDesktopServices::openUrl(QUrl(QStringLiteral("https://redstrate.com/fund")));
    });
    donateAction->setIcon(QIcon::fromTheme(QStringLiteral("help-donate")));

    helpMenu->addSeparator();

    auto aboutNovusAction = helpMenu->addAction(QStringLiteral("About Model Viewer"));
    aboutNovusAction->setIcon(QIcon::fromTheme(QStringLiteral("help-about")));
    connect(aboutNovusAction, &QAction::triggered, this, [this] {
        auto window = new KAboutApplicationDialog(KAboutData::applicationData(), this);
        window->show();
    });

    auto aboutQtAction = helpMenu->addAction(QStringLiteral("About Qt"));
    aboutQtAction->setIcon(QIcon(QStringLiteral(":/qt-project.org/qmessagebox/images/qtlogo-64.png")));
    connect(aboutQtAction, &QAction::triggered, QApplication::instance(), &QApplication::aboutQt);

    auto dummyWidget = new QWidget();
    setCentralWidget(dummyWidget);

    auto layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    dummyWidget->setLayout(layout);

    part = new MDLPart(data, cache);

    const int raceCode = physis_get_race_code(Race::Hyur, Subrace::Midlander, Gender::Male);
    QString skelName = QStringLiteral("c%1b0001.skel").arg(raceCode, 4, 10, QLatin1Char{'0'});
    part->setSkeleton(physis_skeleton_from_skel(physis_read_file(skelName.toStdString().c_str())));

    layout->addWidget(part);
}