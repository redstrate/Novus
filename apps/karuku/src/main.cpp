// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
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

    customizeAboutData(QStringLiteral("novus"), QStringLiteral("zone.xiv.karaku"), QStringLiteral("Excel Editor"), i18n("Program to view FFXIV Excel files."));

    // Default to a sensible message pattern
    if (qEnvironmentVariableIsEmpty("QT_MESSAGE_PATTERN")) {
        qputenv("QT_MESSAGE_PATTERN", "[%{time yyyy-MM-dd h:mm:ss.zzz}] %{if-category}[%{category}] %{endif}[%{type}] %{message}");
    }

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QStringLiteral("query"), i18n("Initial sheet and row to open."));

    const QString gameDir = processCommandLine(parser, app);
    if (gameDir.isEmpty()) {
        return 0;
    }
    const std::string gameDirStd{gameDir.toStdString()};
    const auto window = new MainWindow(physis_sqpack_initialize(gameDirStd.c_str()));
    window->show();

    const QStringList args = parser.positionalArguments();
    if (!args.isEmpty()) {
        if (args.value(0).contains(QLatin1Char('#'))) {
            // Sheet + row query
            const auto query = args.value(0).split(QLatin1Char('#'));
            window->jumpToSheetAndRow(query.constFirst(), query.constLast());
        } else {
            // Only sheet
            window->jumpToSheet(args.value(0));
        }
    }

    return QApplication::exec();
}
