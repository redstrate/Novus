// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <KConfig>
#include <KConfigGroup>
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <physis.hpp>

#include "aboutdata.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    customizeAboutData(QStringLiteral("launcher"),
                       QStringLiteral("SDK Launcher"),
                       QStringLiteral("Handles setting up and launching various Novus SDK components."));

    // Default to a sensible message pattern
    if (qEnvironmentVariableIsEmpty("QT_MESSAGE_PATTERN")) {
        qputenv("QT_MESSAGE_PATTERN", "[%{time yyyy-MM-dd h:mm:ss.zzz}] %{if-category}[%{category}] %{endif}[%{type}] %{message}");
    }

    KConfig config(QStringLiteral("novusrc"));
    KConfigGroup game = config.group(QStringLiteral("Game"));

    if (!game.hasKey("GameDir")) {
        while (true) {
            QMessageBox msgBox;
            msgBox.setText(QStringLiteral("The game directory has not been set, please select it now. Select the 'game' folder."));
            msgBox.exec();

            const QString dir = QFileDialog::getExistingDirectory(nullptr,
                                                                  QStringLiteral("Open Game Directory"),
                                                                  QStandardPaths::standardLocations(QStandardPaths::StandardLocation::HomeLocation).last(),
                                                                  QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

            const std::string dirStd = dir.toStdString();

            if (!physis_gamedata_initialize(dirStd.c_str()))
                continue;

            game.writeEntry("GameDir", dir);
            config.sync();

            break;
        }
    }

    MainWindow window;
    window.show();

    return app.exec();
}
