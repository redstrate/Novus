// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <physis.hpp>

#include "aboutdata.h"
#include "mainwindow.h"
#include "settings.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    KLocalizedString::setApplicationDomain(QByteArrayLiteral("novus"));

    customizeAboutData(QStringLiteral("novus"),
                       QStringLiteral("zone.xiv.novus"),
                       QStringLiteral("Novus SDK"),
                       i18n("Handles setting up and launching various Novus SDK components."));

    // Default to a sensible message pattern
    if (qEnvironmentVariableIsEmpty("QT_MESSAGE_PATTERN")) {
        qputenv("QT_MESSAGE_PATTERN", "[%{time yyyy-MM-dd h:mm:ss.zzz}] %{if-category}[%{category}] %{endif}[%{type}] %{message}");
    }

    if (getGameDirectory(false).isEmpty()) {
        while (true) {
            QMessageBox msgBox;
            msgBox.setText(i18n("The game directory has not been set, please select it now. Select the 'game' folder."));
            msgBox.exec();

            if (!addNewInstall())
                continue;

            KConfig config(QStringLiteral("novusrc"));
            KConfigGroup game = config.group(QStringLiteral("Game"));

            auto gameInstalls = getGameInstalls();

            game.writeEntry("CurrentInstall", gameInstalls.constFirst().uuid.toString());
            config.sync();

            break;
        }
    }

    const auto window = new MainWindow();
    window->show();

    return QApplication::exec();
}
