// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>

#include "novuscommon_export.h"

#include <QUuid>
#include <physis.hpp>

class QWidget;
class QCoreApplication;
class QCommandLineParser;

struct GameInstall {
    QUuid uuid;
    QString label;
    QString path;
    Language language;
};

NOVUSCOMMON_EXPORT QList<GameInstall> getGameInstalls();
NOVUSCOMMON_EXPORT void saveGameInstalls(QList<GameInstall> installs);
NOVUSCOMMON_EXPORT QString getGameDirectory();
NOVUSCOMMON_EXPORT QString getGameUUID();
NOVUSCOMMON_EXPORT bool addNewInstall();
NOVUSCOMMON_EXPORT Language getLanguage();

struct GameMod {
    QUuid uuid;
    QString path;
};

NOVUSCOMMON_EXPORT QList<GameMod> getGameMods();
NOVUSCOMMON_EXPORT void saveGameMods(QList<GameMod> mods);
NOVUSCOMMON_EXPORT bool addNewGameMod();
NOVUSCOMMON_EXPORT bool gameModsEnabled();
NOVUSCOMMON_EXPORT void setGameModsEnabled(bool enabled);

NOVUSCOMMON_EXPORT QString processCommandLine(QCommandLineParser &parser, QCoreApplication &app, bool prompt = true);

/**
 * @brief Identical to QFileDialog::getSaveFileName but it saves its own settings automatically.
 */
NOVUSCOMMON_EXPORT QString getSaveFileName(QWidget *parent = nullptr,
                                           const QString &configKey = QString(),
                                           const QString &caption = QString(),
                                           const QString &fileName = QString(),
                                           const QString &filter = QString());

/**
 * @brief Identical to QFileDialog::getSaveFileName but it saves its own settings automatically.
 */
NOVUSCOMMON_EXPORT QString getOpenFileName(QWidget *parent = nullptr,
                                           const QString &configKey = QString(),
                                           const QString &caption = QString(),
                                           const QString &fileName = QString(),
                                           const QString &filter = QString());
