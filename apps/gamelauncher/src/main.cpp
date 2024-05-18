// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <KLocalizedString>
#include <QApplication>
#include <physis.hpp>
#include <physis_logger.h>

#include "aboutdata.h"
#include "mainwindow.h"
#include "settings.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    KLocalizedString::setApplicationDomain(QByteArrayLiteral("novus"));

    customizeAboutData(QStringLiteral("gamelauncher"),
                       QStringLiteral("zone.xiv.gamelauncher"),
                       QStringLiteral("Game Launcher"),
                       i18n("Program to launch the game for debug purposes."));

    // Default to a sensible message pattern
    if (qEnvironmentVariableIsEmpty("QT_MESSAGE_PATTERN")) {
        qputenv("QT_MESSAGE_PATTERN", "[%{time yyyy-MM-dd h:mm:ss.zzz}] %{if-category}[%{category}] %{endif}[%{type}] %{message}");
    }

    setup_physis_logging();

    MainWindow w;
    w.show();

    return app.exec();
}