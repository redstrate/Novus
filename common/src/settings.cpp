// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "settings.h"

#include "magic_enum.hpp"

#include <physis.hpp>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <QCoreApplication>
#include <QFileDialog>
#include <QMessageBox>

QList<GameInstall> getGameInstalls()
{
    KConfig config(QStringLiteral("novusrc"));
    auto groups = config.groupList();

    QList<GameInstall> installs;
    for (const auto &group : groups) {
        if (group.startsWith(QStringLiteral("install-"))) {
            const auto uuid = QUuid::fromString(group.split(QStringLiteral("install-")).last());

            KConfigGroup kgroup = config.group(group);

            installs.push_back(GameInstall{
                .uuid = uuid,
                .label = kgroup.readEntry(QStringLiteral("Label")),
                .path = kgroup.readEntry(QStringLiteral("Path")),
            });
        }
    }

    return installs;
}

void saveGameInstalls(QList<GameInstall> installs)
{
    KConfig config(QStringLiteral("novusrc"));

    for (const auto &install : installs) {
        auto group = config.group(QStringLiteral("install-%1").arg(install.uuid.toString()));
        group.writeEntry(QStringLiteral("Path"), install.path);
        group.writeEntry(QStringLiteral("Label"), install.label);
    }

    config.sync();
}

QString getGameDirectory(const bool prompt)
{
    KConfig config(QStringLiteral("novusrc"));

    const KConfigGroup game = config.group(QStringLiteral("Game"));
    if (game.hasKey(QStringLiteral("CurrentInstall"))) {
        const auto uuid = game.readEntry(QStringLiteral("CurrentInstall"));
        const auto installs = getGameInstalls();
        for (auto install : installs) {
            if (install.uuid == QUuid::fromString(uuid)) {
                return install.path;
            }
        }
    }

    if (prompt) {
        QMessageBox msgBox;
        msgBox.setText(i18n("The game directory has not been set. Please open the Novus SDK launcher and set it."));
        msgBox.exec();

        QCoreApplication::quit();
    }

    return {};
}

bool addNewInstall()
{
    const QString dir = QFileDialog::getExistingDirectory(nullptr,
                                                          i18nc("@title:window", "Open Game Directory"),
                                                          QStandardPaths::standardLocations(QStandardPaths::StandardLocation::HomeLocation).last(),
                                                          QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    const std::string dirStd = dir.toStdString();

    auto resource = physis_sqpack_initialize(dirStd.c_str());
    if (!resource.p_ptr)
        return false;

    KConfig config(QStringLiteral("novusrc"));
    KConfigGroup game = config.group(QStringLiteral("Game"));

    const QString label = QStringLiteral("%1 %2").arg(magic_enum::enum_name(resource.platform), magic_enum::enum_name(resource.release));

    const auto uuid = QUuid::createUuid();
    KConfigGroup newGameInstall = config.group(QStringLiteral("install-%1").arg(uuid.toString()));
    newGameInstall.writeEntry(QStringLiteral("Path"), dir);
    newGameInstall.writeEntry(QStringLiteral("Label"), label);

    config.sync();

    return true;
}
