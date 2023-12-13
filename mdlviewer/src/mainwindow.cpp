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
#include <physis.hpp>

#include "mdlpart.h"

MainWindow::MainWindow(GameData *data)
    : NovusMainWindow()
    , data(data)
    , cache(FileCache{*data})
{
    setMinimumSize(640, 480);
    setupMenubar();

    auto dummyWidget = new QWidget();
    setCentralWidget(dummyWidget);

    auto layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    dummyWidget->setLayout(layout);

    part = new MDLPart(data, cache);

    const int raceCode = physis_get_race_code(Race::Hyur, Subrace::Midlander, Gender::Male);

    QString skelName = QStringLiteral("chara/human/c%1/skeleton/base/b0001/skl_c%1b0001.sklb").arg(raceCode, 4, 10, QLatin1Char{'0'});
    std::string skelNameStd = skelName.toStdString();
    part->setSkeleton(physis_parse_skeleton(physis_gamedata_extract_file(data, skelNameStd.c_str())));

    layout->addWidget(part);
}

void MainWindow::setupFileMenu(QMenu *menu)
{
    auto openMDLFile = menu->addAction(QStringLiteral("Open MDL..."));
    openMDLFile->setIcon(QIcon::fromTheme(QStringLiteral("document-open")));
    connect(openMDLFile, &QAction::triggered, [this] {
        auto fileName = QFileDialog::getOpenFileName(nullptr, QStringLiteral("Open MDL File"), QStringLiteral("~"), QStringLiteral("FFXIV Model File (*.mdl)"));

        auto buffer = physis_read_file(fileName.toStdString().c_str());

        part->addModel(physis_mdl_parse(buffer), QStringLiteral("mdl"), {}, 0);
    });
}

#include "moc_mainwindow.cpp"
