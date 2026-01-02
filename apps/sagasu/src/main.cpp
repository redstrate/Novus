// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QApplication>
#include <QCommandLineParser>
#include <QMessageBox>

#include <KLocalizedString>
#include <physis.hpp>

#include "aboutdata.h"
#include "mainwindow.h"
#include "settings.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    KLocalizedString::setApplicationDomain(QByteArrayLiteral("novus"));

    customizeAboutData(QStringLiteral("novus"),
                       QStringLiteral("zone.xiv.sagasu"),
                       QStringLiteral("Data Explorer"),
                       i18n("Program to explore FFXIV data archives."));

    // Default to a sensible message pattern
    if (qEnvironmentVariableIsEmpty("QT_MESSAGE_PATTERN")) {
        qputenv("QT_MESSAGE_PATTERN", "[%{time yyyy-MM-dd h:mm:ss.zzz}] %{if-category}[%{category}] %{endif}[%{type}] %{message}");
    }

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QStringLiteral("path"), i18n("Initial path to select."));

    parser.process(app);

    const QString gameDir{getGameDirectory()};
    const std::string gameDirStd{gameDir.toStdString()};
    const auto window = new MainWindow(gameDir, physis_sqpack_initialize(gameDirStd.c_str()));
    window->show();

    const QStringList args = parser.positionalArguments();
    if (!args.isEmpty()) {
        const auto path = args.value(0);
        if (!window->selectPath(path)) {
            QMessageBox::warning(window, i18n("Path Not Found"), i18n("%1 wasn't found, maybe you need to add it to the database?", path));
        }
    }

    return QApplication::exec();
}
