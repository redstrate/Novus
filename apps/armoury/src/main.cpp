// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <KLocalizedString>
#include <QApplication>
#include <physis.hpp>

#include "aboutdata.h"
#include "mainwindow.h"
#include "settings.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    KLocalizedString::setApplicationDomain(QByteArrayLiteral("novus"));

    customizeAboutData(QStringLiteral("novus"), QStringLiteral("zone.xiv.armoury"), QStringLiteral("Gear Editor"), i18n("Program to view FFXIV gear."));

    // Default to a sensible message pattern
    if (qEnvironmentVariableIsEmpty("QT_MESSAGE_PATTERN")) {
        qputenv("QT_MESSAGE_PATTERN", "[%{time yyyy-MM-dd h:mm:ss.zzz}] %{if-category}[%{category}] %{endif}[%{type}] %{message}");
    }

    const QString gameDir{getGameDirectory()};
    const std::string gameDirStd{gameDir.toStdString()};
    auto w = new MainWindow(physis_gamedata_initialize(gameDirStd.c_str()));
    w->show();

    return QApplication::exec();
}
