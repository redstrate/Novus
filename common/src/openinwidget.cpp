// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "openinwidget.h"

#include "settings.h"

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <QCoreApplication>
#include <QProcess>

OpenInWidget::OpenInWidget(QObject *target)
{
    QMenu *menu = addMenu(i18n("Open In"));

    const auto installs = getGameInstalls();
    for (const auto &install : installs) {
        if (install.uuid.toString() == getGameUUID()) {
            menu->setTitle(install.label);
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
