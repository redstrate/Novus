// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QApplication>

#include <physis.hpp>
#include <physis_logger.h>

#include "aboutdata.h"
#include "mainwindow.h"
#include "settings.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    customizeAboutData(QStringLiteral("sagasu"),
                       QStringLiteral("zone.xiv.sagasu"),
                       QStringLiteral("Data Explorer"),
                       QStringLiteral("Program to explore FFXIV data archives."));

    // Default to a sensible message pattern
    if (qEnvironmentVariableIsEmpty("QT_MESSAGE_PATTERN")) {
        qputenv("QT_MESSAGE_PATTERN", "[%{time yyyy-MM-dd h:mm:ss.zzz}] %{if-category}[%{category}] %{endif}[%{type}] %{message}");
    }

    setup_physis_logging();

    const QString gameDir{getGameDirectory()};
    const std::string gameDirStd{gameDir.toStdString()};
    MainWindow w(gameDir, physis_gamedata_initialize(gameDirStd.c_str()));
    w.show();

    return app.exec();
}