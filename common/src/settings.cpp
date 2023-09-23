// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "settings.h"

#include <KConfig>
#include <KConfigGroup>
#include <QCoreApplication>
#include <QMessageBox>

QString getGameDirectory() {
    KConfig config("novusrc");
    KConfigGroup game = config.group("Game");

    if (game.hasKey("GameDir")) {
        return game.readEntry("GameDir");
    } else {
        QMessageBox msgBox;
        msgBox.setText("The game directory has not been set. Please open the Novus SDK launcher and set it.");
        msgBox.exec();
        QCoreApplication::quit();
        return {};
    }
}