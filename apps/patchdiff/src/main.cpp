// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
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

    customizeAboutData(QStringLiteral("novus"),
                       QStringLiteral("zone.xiv.novus.patchdiff"),
                       QStringLiteral("Patch Diff"),
                       i18n("Program to view FFXIV patches."));

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
