// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QApplication>

#include <QStyle>
#include <physis.hpp>

#include "aboutdata.h"
#include "mainwindow.h"
#include "settings.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    customizeAboutData(
        QStringLiteral("explorer"), QStringLiteral("Explorer"), QStringLiteral("Program to explore FFXIV dat files."));

    physis_initialize_logging();

    app.setStyle(QStringLiteral("Windows"));

    const QString gameDir{getGameDirectory()};
    const std::string gameDirStd{gameDir.toStdString()};
    MainWindow w(physis_gamedata_initialize(gameDirStd.c_str()));
    w.show();

    return app.exec();
}