// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <KLocalizedString>
#include <QApplication>
#include <physis.hpp>

#include "aboutdata.h"
#include "mainwindow.h"
#include "settings.h"

#include <QCommandLineParser>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    KLocalizedString::setApplicationDomain(QByteArrayLiteral("novus"));

    customizeAboutData(QStringLiteral("novus"), QStringLiteral("zone.xiv.novus.mapeditor"), QStringLiteral("Map Editor"), i18n("View and edit FFXIV maps."));

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();

    const QString gameDir = processCommandLine(parser, app);
    if (gameDir.isEmpty()) {
        return 0;
    }

    const std::string gameDirStd{gameDir.toStdString()};
    const auto window = new MainWindow(physis_sqpack_initialize(gameDirStd.c_str()));
    window->show();

    return QApplication::exec();
}
