// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "openinwidget.h"

#include "settings.h"

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <QCoreApplication>
#include <QProcess>

OpenInWidget::OpenInWidget()
{
    QMenu *menu = addMenu(i18n("Open In"));

    const KConfig config(QStringLiteral("novusrc"));
    const KConfigGroup game = config.group(QStringLiteral("Game"));

    const auto installs = getGameInstalls();
    for (const auto &install : installs) {
        if (install.uuid.toString() == game.readEntry(QStringLiteral("CurrentInstall"))) {
            menu->setTitle(install.label);
        } else {
            QAction *action = menu->addAction(install.label);
            connect(action, &QAction::triggered, this, [] {
                // TODO: pass arguments
                // TODO: pass install
                QProcess::startDetached(QCoreApplication::applicationFilePath(), {});
            });
        }
    }
}

#include "moc_openinwidget.cpp"
