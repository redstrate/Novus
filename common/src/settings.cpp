// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "settings.h"

#include "magic_enum.hpp"

#include <physis.hpp>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QFileDialog>
#include <QMessageBox>

Q_GLOBAL_STATIC(QString, currentGameUuid)

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
                .language = static_cast<Language>(kgroup.readEntry(QStringLiteral("Language")).toInt()),
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
        group.writeEntry(QStringLiteral("Language"), static_cast<int>(install.language));
    }

    config.sync();
}

QString getGameDirectory()
{
    KConfig config(QStringLiteral("novusrc"));

    const KConfigGroup game = config.group(QStringLiteral("Game"));
    const auto installs = getGameInstalls();
    for (auto install : installs) {
        if (install.uuid == QUuid::fromString(*currentGameUuid)) {
            return install.path;
        }
    }

    return {};
}

QString getGameUUID()
{
    return *currentGameUuid;
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

    if (!physis_sqpack_exists(&resource, "exd/root.exl"))
        return false;

    KConfig config(QStringLiteral("novusrc"));
    KConfigGroup game = config.group(QStringLiteral("Game"));

    const QString label = QStringLiteral("%1 %2").arg(magic_enum::enum_name(resource.platform), magic_enum::enum_name(resource.release));

    const auto uuid = QUuid::createUuid();
    KConfigGroup newGameInstall = config.group(QStringLiteral("install-%1").arg(uuid.toString()));
    newGameInstall.writeEntry(QStringLiteral("Path"), dir);
    newGameInstall.writeEntry(QStringLiteral("Label"), label);
    newGameInstall.writeEntry(QStringLiteral("Language"), static_cast<int>(Language::English)); // TODO: don't assume English

    config.sync();

    return true;
}

Language getLanguage()
{
    KConfig config(QStringLiteral("novusrc"));

    const KConfigGroup game = config.group(QStringLiteral("Game"));
    if (game.hasKey(QStringLiteral("CurrentInstall"))) {
        const auto installs = getGameInstalls();
        for (auto install : installs) {
            if (install.uuid == QUuid::fromString(*currentGameUuid)) {
                return install.language;
            }
        }
    }

    Q_UNREACHABLE(); // if you hit this, you did something wrong
}

QList<GameMod> getGameMods()
{
    KConfig config(QStringLiteral("novusrc"));
    auto groups = config.groupList();

    QList<GameMod> mods;
    for (const auto &group : groups) {
        if (group.startsWith(QStringLiteral("mod-"))) {
            const auto uuid = QUuid::fromString(group.split(QStringLiteral("mod-")).last());

            KConfigGroup kgroup = config.group(group);

            mods.push_back(GameMod{
                .uuid = uuid,
                .path = kgroup.readEntry(QStringLiteral("Path")),
            });
        }
    }

    return mods;
}

void saveGameMods(QList<GameMod> mods)
{
    KConfig config(QStringLiteral("novusrc"));

    for (const auto &mod : mods) {
        auto group = config.group(QStringLiteral("mod-%1").arg(mod.uuid.toString()));
        group.writeEntry(QStringLiteral("Path"), mod.path);
    }

    config.sync();
}

bool addNewGameMod()
{
    const QString dir = QFileDialog::getExistingDirectory(nullptr,
                                                          i18nc("@title:window", "Open Game Mod Directory"),
                                                          QStandardPaths::standardLocations(QStandardPaths::StandardLocation::HomeLocation).last(),
                                                          QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!QDir(dir).exists(QStringLiteral("meta.json"))) {
        return false;
    }

    KConfig config(QStringLiteral("novusrc"));
    KConfigGroup game = config.group(QStringLiteral("Mod"));

    const auto uuid = QUuid::createUuid();
    KConfigGroup newGameInstall = config.group(QStringLiteral("mod-%1").arg(uuid.toString()));
    newGameInstall.writeEntry(QStringLiteral("Path"), dir);

    config.sync();

    return true;
}

bool gameModsEnabled()
{
    KConfig config(QStringLiteral("novusrc"));

    const KConfigGroup game = config.group(QStringLiteral("Mod"));
    return game.readEntry(QStringLiteral("Enabled"), false);
}

void setGameModsEnabled(const bool enabled)
{
    KConfig config(QStringLiteral("novusrc"));

    KConfigGroup game = config.group(QStringLiteral("Mod"));
    game.writeEntry(QStringLiteral("Enabled"), enabled);
}

QString processCommandLine(QCommandLineParser &parser, QCoreApplication &app, bool prompt)
{
    QCommandLineOption gameInstallOption(QStringLiteral("game"), i18n("Which installation to use"), QStringLiteral("uuid"));
    parser.addOption(gameInstallOption);

    parser.process(app);

    if (parser.isSet(gameInstallOption)) {
        // Load override
        QString uuid = parser.value(gameInstallOption);
        *currentGameUuid = uuid;
    } else {
        // Load default
        KConfig config(QStringLiteral("novusrc"));

        const KConfigGroup game = config.group(QStringLiteral("Game"));
        if (game.hasKey(QStringLiteral("CurrentInstall"))) {
            const auto uuid = game.readEntry(QStringLiteral("CurrentInstall"));
            *currentGameUuid = uuid;
        }
    }

    const auto installs = getGameInstalls();
    for (auto install : installs) {
        if (install.uuid == QUuid::fromString(*currentGameUuid)) {
            return install.path;
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
