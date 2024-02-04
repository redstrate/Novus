// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "settings.h"

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <QCoreApplication>
#include <QMessageBox>

QString getGameDirectory()
{
    KConfig config(QStringLiteral("novusrc"));
    KConfigGroup game = config.group(QStringLiteral("Game"));

    if (game.hasKey(QStringLiteral("GameDir"))) {
        return game.readEntry(QStringLiteral("GameDir"));
    } else {
        QMessageBox msgBox;
        msgBox.setText(i18n("The game directory has not been set. Please open the Novus SDK launcher and set it."));
        msgBox.exec();
        QCoreApplication::quit();
        return {};
    }
}