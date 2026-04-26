// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "openinwidget.h"

#include "settings.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <QCoreApplication>
#include <QMessageBox>
#include <QProcess>

OpenInWidget::OpenInWidget(QObject *target)
{
    QMenu *menu = addMenu(i18n("Open In"));

    auto enableModsAction = menu->addAction(i18n("Enable Mods"));
    enableModsAction->setCheckable(true);
    enableModsAction->setChecked(gameModsEnabled());
    connect(enableModsAction, &QAction::triggered, this, [this](const bool checked) {
        setGameModsEnabled(checked);

        QMessageBox::information(this, i18n("Restart Required"), i18n("Mods status changed, you need to restart the application for it to take full effect."));
    });

    const auto installs = getGameInstalls();

    if (installs.size() > 1) {
        menu->addSeparator();
    }

    for (const auto &install : installs) {
        if (install.uuid.toString() == getGameUUID()) {
            menu->setTitle(gameModsEnabled() ? i18n("%1 (Modded)").arg(install.label) : install.label);
        } else {
            QAction *action = menu->addAction(install.label);
            connect(action, &QAction::triggered, this, [target, install] {
                QString arguments;
                QMetaObject::invokeMethod(target, "getArguments", qReturnArg(arguments));
                QProcess::startDetached(QCoreApplication::applicationFilePath(), {QStringLiteral("--game"), install.uuid.toString(), arguments});
            });
        }
    }
}

#include "moc_openinwidget.cpp"
